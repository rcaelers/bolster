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
#include "StringUtil.hh"

using namespace std;

OAuth::OAuth(const string &consumer_key,
             const string &consumer_secret,
             const string &callback_uri,
             const string &signature_method)
  : consumer_key(consumer_key),
    consumer_secret(consumer_secret),
    callback_uri(callback_uri),
    signature_method(signature_method)
{
  oauth_version = "1.0";
}


string
OAuth::request_temporary_credentials(const string &http_method,
                                     const string &uri
                                     )
{
	RequestParams parameters;

	parameters["oauth_callback"] = callback_uri;

  string key = consumer_key + "&";
	string headers = create_headers(http_method, uri, key, parameters);
	
  cout << headers << endl;
  
  return headers;
}


string
OAuth::request_token_credentials(const string &http_method, const string &uri, const string &token, const string &token_secret, const string &pin_code)
{
	RequestParams parameters;

	string requestUrl;
	string key;

	parameters["oauth_token"] = token;
  parameters["oauth_verifier"] = pin_code;
  
	key = consumer_secret + "&" + token_secret;
	
	string headers = create_headers(http_method, requestUrl, key, parameters);

  cout << headers << endl;

  return headers;
}

string
OAuth::request(const string &http_method, const string &uri, const string &token, const string &token_secret)
{
	RequestParams parameters;

	string requestUrl;
	string key;

	parameters["oauth_token"] = token;
	key = consumer_secret + "&" + token_secret;
	
	string headers = create_headers(http_method, uri, key, parameters);

  cout << headers << endl;

  return headers;
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
OAuth::get_timestamp()
{
	time_t now = time (NULL);

	stringstream ss;
	ss << now;
	return ss.str();
}

string
OAuth::get_nonce()
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
      nonce[i] = valid_chars[g_random_int_range(0, valid_chars_count + 1)];
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

              cout << param_elements.size() << endl;
              if (param_elements.size() == 2)
                {
                  parameters[param_elements[0]] = param_elements[1];

                  cout << param_elements[0] << " " <<  param_elements[1] << endl;
                }
            }


          g_free(u->query);
          u->query = NULL;
        }
      char *new_uri = soup_uri_to_string(u, FALSE);
      
      ret = new_uri;

      cout << "new uro " << ret << endl;
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
  string assign;
  bool only_oauth;
  
  switch (mode)
    {
    case ParameterModeRequest:
      quotes = "";
      sep = "&";
      assign = "=";
      only_oauth = false;
      break;
      
    case ParameterModeHeader:
      quotes = "\"";
      sep = ",";
      assign = "=";
      only_oauth = true;
      break;
      
    case ParameterModeSignatureBase:
      quotes = "";
      sep = "&"; // %26";
      assign = "="; // %3D";
      only_oauth = false;
      break;
    }
  
  for(RequestParams::const_iterator it = parameters.begin(); it != parameters.end(); it++)
    {
      string key = it->first;
      if (!only_oauth || key.find("oauth_") == 0)
        {
          string param = key + assign + quotes + escape_uri(it->second) +  quotes;
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
	base64.Put(digest, sizeof(digest) ); //use the digest to base64 encode
	base64.MessageEnd();
	base64.MessageSeriesEnd();
  
	unsigned int size = (sizeof(digest) + 2 - ((sizeof(digest) + 2) % 3)) * 4 / 3;
	uint8_t* encodedValues = new uint8_t[size + 1];
  
	base64.Get( encodedValues, size );
  encodedValues[size] = 0;
	std::string encodedString = (char*)encodedValues;

	return encodedString;
}


string
OAuth::create_headers(const string &http_method, const string &uri, const string &key, RequestParams &parameters)
{
	string timestamp = get_timestamp();
	string nonce = get_nonce();

  parameters["oauth_consumer_key"] = consumer_key;
	parameters["oauth_signature_method"] = signature_method;
	parameters["oauth_timestamp"] = timestamp;
	parameters["oauth_nonce"] = nonce;
	parameters["oauth_version"] = oauth_version;

  string normalized_uri = normalize_uri(uri, parameters);
  string normalized_parameters = parameters_to_string(parameters, ParameterModeSignatureBase);
 
  string signature_base_string = ( http_method + "&" +
                                   escape_uri(normalized_uri) + "&" +
                                   escape_uri(normalized_parameters)
                                   );

  cout << "BASE " << signature_base_string << endl;
  
  string signature = encrypt(signature_base_string, key);

	parameters["oauth_signature"] = signature;
  
  string header = parameters_to_string(parameters, ParameterModeHeader);

  cout << "URL " << normalized_uri << "&" << parameters_to_string(parameters, ParameterModeRequest) << endl;
    
  return header;
}





