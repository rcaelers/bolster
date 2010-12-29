#include <iostream>
#include <sstream>
#include <algorithm>
#include <string.h>
#include <list>
#include <stdint.h>

#include <glib.h>

#include <crypto++/cryptlib.h>
#include <crypto++/sha.h>
#include <crypto++/hmac.h>
#include <crypto++/base64.h>

#include <libsoup/soup.h>

#include "OAuth.hh"
#include "IWebBackend.hh"
#include "StringUtil.hh"

using namespace std;

OAuth::OAuth(IWebBackend *backend)
  : backend(backend)
{
  oauth_version = "1.0";
  signature_method = "HMAC-SHA1";
  callback = "oob";
}


void
OAuth::init(std::string consumer_key, std::string consumer_secret)
{
  this->consumer_key = consumer_key;
  this->consumer_secret = consumer_secret;

  request_temporary_credentials();
  
}


void
OAuth::init(std::string consumer_key, std::string consumer_secret, std::string token_key, std::string token_secret)
{
  this->consumer_key = consumer_key;
  this->consumer_secret = consumer_secret;
  this->token_key = token_key;
  this->token_secret = token_secret;
}


void
OAuth::set_callback(std::string callback)
{
  this->callback = callback;
}


void
OAuth::set_verifier(std::string verifier)
{
  this->verifier = verifier;
}


void
OAuth::request_temporary_credentials()
{
  RequestParams parameters;

  IWebBackend::ListenCallback cb;

  int port;
  string path = "/oauth-verfied";
  backend->listen(cb, path, port);

  stringstream ss;
  ss << "http://127.0.0.1:" << port << path;
  
  parameters["oauth_callback"] = ss.str();

  string http_method = "POST";
  string uri = "http://127.0.0.1:8888/oauth/request_token/";
  
  string oauth_header = create_headers(http_method, uri, parameters);

  string response = backend->request(http_method, uri, "", oauth_header);

  RequestParams response_parameters;
  parse_query(response, response_parameters);

  token_key = response_parameters["oauth_token"];
  token_secret = response_parameters["oauth_token_secret"];

  if (response_parameters["oauth_callback_confirmed"] != "true")
    {
      g_debug("oauth_callback_confirmed error");
    }

  request_resource_owner_authorization();
}


void
OAuth::request_resource_owner_authorization()
{
  gchar *program = g_find_program_in_path("xdg-open");
  if (program != NULL)
    {
      string command = ( string(program) +
                         " http://127.0.0.1:8888/oauth/authorize/?oauth_token="
                         + escape_uri(token_key)
                         );
  
      
      gint exit_code;
      GError *error = NULL;
      if (!g_spawn_command_line_sync(command.c_str(), NULL, NULL, &exit_code, &error) )
        {
          g_error_free(error);
        }
    }
}


string
OAuth::get_request_header(const std::string &http_method, const std::string &uri) const
{
  RequestParams parameters;

  return create_headers(http_method, uri, parameters);
}


string
OAuth::escape_uri(const string &uri) const
{
  return g_uri_escape_string(uri.c_str(), NULL, TRUE);
}


string
OAuth::unescape_uri(const string &uri) const
{
  return g_uri_unescape_string(uri.c_str(), NULL);
}

string
OAuth::get_timestamp() const
{
  time_t now = time (NULL);

  stringstream ss;
  ss << now;
  return ss.str();
}

string
OAuth::get_nonce() const
{
  static bool random_seeded = 1;
  const int nonce_size = 32;
  const char *valid_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  const int valid_chars_count = strlen(valid_chars);
  char nonce[nonce_size + 1] = { 0, };
  
  if (!random_seeded)
    {
      g_random_set_seed(time(NULL));
      random_seeded = true;
    }

  for (int i = 0; i < nonce_size; i++)
    {
      nonce[i] = valid_chars[g_random_int_range(0, valid_chars_count)];
    }

  return nonce;
}

const string
OAuth::normalize_uri(const string &uri, RequestParams &parameters) const
{
  SoupURI *u = soup_uri_new(uri.c_str());
  string ret = uri;
    
  if (u != NULL)
    {
      if (u->query != NULL)
        {
          vector<string> query_params;
          StringUtil::split(unescape_uri(u->query), '&', query_params);

          for (size_t i = 0; i < query_params.size(); ++i)
            {
              vector<string> param_elements;
              StringUtil::split(query_params[i], '=', param_elements);

              if (param_elements.size() == 2)
                {
                  parameters[param_elements[0]] = param_elements[1];
                }
            }

          g_free(u->query);
          u->query = NULL;
        }
      
      char *new_uri = soup_uri_to_string(u, FALSE);
      ret = new_uri;
      g_free(new_uri);
    }
 
  return ret;
}
  

const string
OAuth::parameters_to_string(const RequestParams &parameters, ParameterMode mode) const
{
  list<string> sorted;
  string sep;
  string quotes;
  bool only_oauth;
  
  switch (mode)
    {
    case ParameterModeRequest:
      quotes = "";
      sep = "&";
      only_oauth = false;
      break;
      
    case ParameterModeHeader:
      quotes = "\"";
      sep = ",";
      only_oauth = true;
      break;
      
    case ParameterModeSignatureBase:
      quotes = "";
      sep = "&";
      only_oauth = false;
      break;
    }
  
  for(RequestParams::const_iterator it = parameters.begin(); it != parameters.end(); it++)
    {
      string key = it->first;
      if (!only_oauth || key.find("oauth_") == 0)
        {
          string param = key + "=" + quotes + escape_uri(it->second) +  quotes;
          sorted.push_back(param);
        }
    }
  sorted.sort();

  string norm;
  for(list<string>::iterator it = sorted.begin(); it != sorted.end(); it++)
    {
      if (norm.size() > 0)
        {
          norm += sep;
        }
      norm += *it;
    }

  return norm;
}


std::string
OAuth::encrypt(const std::string &input, const std::string &key) const
{
  uint8_t digest[CryptoPP::HMAC<CryptoPP::SHA1>::DIGESTSIZE];

  CryptoPP::HMAC<CryptoPP::SHA1> hmac((uint8_t*)key.c_str(), key.size());
  hmac.Update((uint8_t*)input.c_str(), input.size());
  hmac.Final(digest);

  CryptoPP::Base64Encoder base64; 
  base64.Put(digest, sizeof(digest));
  base64.MessageEnd();
  base64.MessageSeriesEnd();
  
  unsigned int size = (sizeof(digest) + 2 - ((sizeof(digest) + 2) % 3)) * 4 / 3;
  uint8_t* encodedValues = new uint8_t[size + 1];
  
  base64.Get(encodedValues, size);
  encodedValues[size] = 0;
  std::string encodedString = (char*)encodedValues;

  return encodedString;
}


string
OAuth::create_headers(const string &http_method,
                      const string &uri,
                      RequestParams &parameters) const
{
  string timestamp = get_timestamp();
  string nonce = get_nonce();

  parameters["oauth_consumer_key"] = consumer_key;
  parameters["oauth_signature_method"] = signature_method;
  parameters["oauth_timestamp"] = timestamp;
  parameters["oauth_nonce"] = nonce;
  parameters["oauth_version"] = oauth_version;

  if (token_key != "")
    {
      parameters["oauth_token"] = token_key;
    }

  string key = consumer_secret + "&";
  if (token_secret != "")
    {
      key += token_secret;
    }
  
  string normalized_uri = normalize_uri(uri, parameters);
  string normalized_parameters = parameters_to_string(parameters, ParameterModeSignatureBase);
 
  string signature_base_string = ( http_method + "&" +
                                   escape_uri(normalized_uri) + "&" +
                                   escape_uri(normalized_parameters)
                                   );

  cout << "BASE " << signature_base_string << endl;
  cout << "KEY " << key << endl;
  
  string signature = encrypt(signature_base_string, key);

  parameters["oauth_signature"] = signature;
  
  string header = "OAuth " + parameters_to_string(parameters, ParameterModeHeader);

  return header;
}

void
OAuth::parse_query(string query, RequestParams &params) const
{
  g_debug("Response body: %s", query.c_str());

  if (query != "")
    {
      vector<string> query_params;
      StringUtil::split(query, '&', query_params);

      for (size_t i = 0; i < query_params.size(); ++i)
        {
          vector<string> param_elements;
          StringUtil::split(query_params[i], '=', param_elements);
      
          if (param_elements.size() == 2)
            {
              params[param_elements[0]] = param_elements[1];
            }
        }
    }
}

