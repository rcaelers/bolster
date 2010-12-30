#include "OAuth.hh"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <string.h>
#include <list>
#include <stdint.h>
#include "boost/bind.hpp"

#include <glib.h>

#include <crypto++/cryptlib.h>
#include <crypto++/sha.h>
#include <crypto++/hmac.h>
#include <crypto++/base64.h>

#include <libsoup/soup.h>

#include "IWebBackend.hh"
#include "OAuthException.hh"
#include "WebBackendException.hh"
#include "StringUtil.hh"


using namespace std;

OAuth::OAuth(IWebBackend *backend,
             const string &temporary_request_uri,
             const string &authorize_uri,
             const string &token_request_uri)
  : backend(backend),
    temporary_request_uri(temporary_request_uri),
    authorize_uri(authorize_uri),
    token_request_uri(token_request_uri)
{
  oauth_version = "1.0";
  signature_method = "HMAC-SHA1";
}


void
OAuth::init(const std::string &consumer_key, const std::string &consumer_secret, OAuthResult callback)
{
  this->consumer_key = consumer_key;
  this->consumer_secret = consumer_secret;
  this->oauth_result_callback = callback;

  request_temporary_credentials();
}


void
OAuth::init(const std::string &consumer_key, const std::string &consumer_secret, const std::string &token_key, const std::string &token_secret)
{
  this->consumer_key = consumer_key;
  this->consumer_secret = consumer_secret;
  this->token_key = token_key;
  this->token_secret = token_secret;
}


const string
OAuth::get_timestamp() const
{
  time_t now = time (NULL);

  stringstream ss;
  ss << now;
  return ss.str();
}


const string
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
OAuth::escape_uri(const string &uri) const
{
  return g_uri_escape_string(uri.c_str(), NULL, TRUE);
}


const string
OAuth::unescape_uri(const string &uri) const
{
  return g_uri_unescape_string(uri.c_str(), NULL);
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


const std::string
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


const string
OAuth::create_oauth_header(const string &http_method,
                           const string &uri,
                           RequestParams &parameters) const
{
  parameters["oauth_consumer_key"] = consumer_key;
  parameters["oauth_signature_method"] = signature_method;
  parameters["oauth_timestamp"] = get_timestamp();
  parameters["oauth_nonce"] = get_nonce();
  parameters["oauth_version"] = oauth_version;

  if (token_key != "")
    {
      parameters["oauth_token"] = token_key;
    }

  string key = consumer_secret + "&" + token_secret;
  string normalized_uri = normalize_uri(uri, parameters);
  string normalized_parameters = parameters_to_string(parameters, ParameterModeSignatureBase);

  string signature_base_string = ( http_method + "&" +
                                   escape_uri(normalized_uri) + "&" +
                                   escape_uri(normalized_parameters)
                                   );

  g_debug("BASE %s", signature_base_string.c_str());
  g_debug("KEY %s", key.c_str());
  
  string signature = encrypt(signature_base_string, key);

  parameters["oauth_signature"] = signature;
  
  return  "OAuth " + parameters_to_string(parameters, ParameterModeHeader);
}


void
OAuth::parse_query(const string &query, RequestParams &params) const
{
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

void
OAuth::request_temporary_credentials()
{
  try
    {
      int port;
      string path = "/oauth-verfied";
      backend->listen(boost::bind(&OAuth::ready_resource_owner_authorization, this, _1),
                      path, port);

      stringstream ss;
      ss << "http://127.0.0.1:" << port << path;
  
      RequestParams parameters;
      parameters["oauth_callback"] = ss.str();

      string http_method = "POST";
      string oauth_header = create_oauth_header(http_method, temporary_request_uri, parameters);

      backend->request(http_method, temporary_request_uri, "", oauth_header,
                       boost::bind(&OAuth::ready_temporary_credentials, this, _1, _2));
    }
  catch(OAuthException &oe)
    {
      oauth_result_callback(false, string("OAuth failure") + oe.what());
    }
  catch(WebBackendException &we)
    {
      oauth_result_callback(false, string("OAuth failure") + we.what());
    }
}


void
OAuth::ready_temporary_credentials(int status, const std::string &response)
{
  try
    {
      if (status != 200)
        {
          g_debug("Invalid response for temporary credentials %d", status);
          throw OAuthException("Invalid response for temporary credentials");
        }
      if (response == "")
        {
          g_debug("Empty response for temporary credentials");
          throw OAuthException("Empty response for temporary credentials");
        }          

      RequestParams response_parameters;
      parse_query(response, response_parameters);

      if (response_parameters["oauth_callback_confirmed"] != "true")
        {
          g_debug("Callback not confirmed");
          throw OAuthException("Callback not confirmed");
        }
      
      string key = response_parameters["oauth_token"];
      string secret = response_parameters["oauth_token_secret"];

      token_key = key;
      token_secret = secret;

      request_resource_owner_authorization();
    }
  catch(OAuthException &oe)
    {
      oauth_result_callback(false, string("OAuth failure ") + oe.what());
    }
}

void
OAuth::request_resource_owner_authorization()
{
  try
    {
      gchar *program = g_find_program_in_path("xdg-open");
      if (program == NULL)
        {
          throw OAuthException("Cannot find xdg-open");
        }
      
      string command = ( string(program) + " " +
                         authorize_uri + "?oauth_token="
                         + escape_uri(token_key)
                         );
  
      gint exit_code;
      GError *error = NULL;
      if (!g_spawn_command_line_sync(command.c_str(), NULL, NULL, &exit_code, &error))
        {
          g_error_free(error);
          throw OAuthException("xdg-open failed");
        }

      if (WEXITSTATUS(exit_code) != 0)
        {
          throw OAuthException("xdg-open returned != 0");
        }
    }
  catch(OAuthException &oe)
    {
      oauth_result_callback(false, string("OAuth failure") + oe.what());
    }
}


void
OAuth::ready_resource_owner_authorization(const std::string &response)
{
  try
    {
      if (response == "")
        {
          g_debug("Empty response for resource owner authorization");
          throw OAuthException();
        }          

      RequestParams response_parameters;
      parse_query(response, response_parameters);

      string token = response_parameters["oauth_token"];
      string verifier = response_parameters["oauth_verifier"];

      if (token == "" || verifier == "")
        {
          throw OAuthException();
        }
      
      request_token(token, verifier);
    }
  catch(WebBackendException &we)
    {
      oauth_result_callback(false, string("OAuth failure") + we.what());
    }
  catch(OAuthException &oe)
    {
      oauth_result_callback(false, string("OAuth failure") + oe.what());
    }
}

void
OAuth::request_token(const string &token, const string &verifier)
{
  try
    {
      if (token != token_key)
        {
          throw OAuthException();
        }
      
      RequestParams parameters;
      parameters["oauth_verifier"] = verifier;

      string http_method = "POST";
      string oauth_header = create_oauth_header(http_method, token_request_uri, parameters);

      backend->request(http_method, token_request_uri, "", oauth_header,
                       boost::bind(&OAuth::ready_token, this, _1, _2));
    }
  catch(WebBackendException &we)
    {
      oauth_result_callback(false, string("OAuth failure") + we.what());
    }
  catch(OAuthException &oe)
    {
      oauth_result_callback(false, string("OAuth failure") + oe.what());
    }
}

void
OAuth::ready_token(int status, const std::string &response)
{
  try
    {
      if (status != 200)
        {
          g_debug("Invalid response for temporary credentials %d", status);
          throw OAuthException();
        }
      if (response == "")
        {
          g_debug("Empty response for temporary credentials");
          throw OAuthException();
        }          

      RequestParams response_parameters;
      parse_query(response, response_parameters);

      string key = response_parameters["oauth_token"];
      string secret = response_parameters["oauth_token_secret"];

      if (key == "" || secret == "")
        {
          throw OAuthException();
        }

      token_key = key;
      token_secret = secret;

      oauth_result_callback(true, "OAuth successful");
    }
  catch(OAuthException &oe)
    {
      oauth_result_callback(false, string("OAuth failure") + oe.what());
    }
}
