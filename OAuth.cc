//
// Copyright (C) 2010, 2011 Rob Caelers <robc@krandor.nl>
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "OAuth.hh"

#include <sstream>
#include <string.h>
#include <list>
#include "boost/bind.hpp"

#include <glib.h>

#ifdef HAVE_CRYPTOPP  
#include <crypto++/cryptlib.h>
#include <crypto++/sha.h>
#include <crypto++/hmac.h>
#include <crypto++/base64.h>
#endif

#include <gcrypt.h>
#include <libsoup/soup.h>

#include "IWebBackend.hh"
#include "OAuthException.hh"
#include "WebBackendException.hh"
#include "StringUtil.hh"

using namespace std;

OAuth::OAuth(IWebBackend *backend,
             const string &temporary_request_uri,
             const string &authorize_uri,
             const string &token_request_uri,
             const string &success_html,
             const string &failure_html)
  : backend(backend),
    temporary_request_uri(temporary_request_uri),
    authorize_uri(authorize_uri),
    token_request_uri(token_request_uri),
    success_html(success_html),
    failure_html(failure_html)
{
  oauth_version = "1.0";
  signature_method = "HMAC-SHA1";
}

void
OAuth::init(const string &consumer_key, const string &consumer_secret, const RequestParams &custom_headers,
            SuccessCallback success_cb, FailedCallback failure_cb)
{
  this->consumer_key = consumer_key;
  this->consumer_secret = consumer_secret;
  this->custom_headers = custom_headers;
  this->success_cb = success_cb;
  this->failure_cb = failure_cb;
  
  request_temporary_credentials();
}


void
OAuth::init(const string &consumer_key, const string &consumer_secret, const string &token_key, const string &token_secret,
            const RequestParams &custom_headers)
{
  this->consumer_key = consumer_key;
  this->consumer_secret = consumer_secret;
  this->token_key = token_key;
  this->token_secret = token_secret;
  this->custom_headers = custom_headers;
}


int
OAuth::request(const string &http_method,
               const string &uri,
               const string &body,
               string &response_body)
{
  RequestParams parameters;
  string oauth_header = create_oauth_header(http_method, uri, parameters);

  return backend->request(http_method, uri, body, oauth_header, response_body);
}
  
void
OAuth::request(const string &http_method,
               const string &uri,
               const string &body,
               const WebReplyCallback callback)
{
  RequestParams parameters;
  string oauth_header = create_oauth_header(http_method, uri, parameters);

  backend->request(http_method, uri, body, oauth_header, callback);
}


bool
OAuth::has_credentials() const
{
  return consumer_key != "" &&
    consumer_secret != "" &&
    token_key != "" &&
    token_secret != "";
}

void
OAuth::get_credentials(string &consumer_key, string &consumer_secret, string &token_key, string &token_secret)
{
  consumer_key = this->consumer_key;
  consumer_secret = this->consumer_secret;
  token_key = this->token_key;
  token_secret = this->token_secret;
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
      if (!only_oauth || key.find("oauth_") == 0 || custom_headers.find(key) != custom_headers.end())
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


const string
OAuth::encrypt(const string &input, const string &key) const
{
#ifdef HAVE_CRYPTOPP  
  uint8_t digest[CryptoPP::HMAC<CryptoPP::SHA1>::DIGESTSIZE];

  CryptoPP::HMAC<CryptoPP::SHA1> hmac((uint8_t*)key.c_str(), key.size());
  hmac.Update((uint8_t*)input.c_str(), input.size());
  hmac.Final(digest);

  CryptoPP::Base64Encoder base64; 
  base64.Put(digest, sizeof(digest));
  base64.MessageEnd();
  base64.MessageSeriesEnd();
  
  unsigned int size = (sizeof(digest) + 2 - ((sizeof(digest) + 2) % 3)) * 4 / 3;
  uint8_t* encoded_values = new uint8_t[size + 1];
  
  base64.Get(encoded_values, size);
  encoded_values[size] = 0;
  return string((char *)encoded_values);

#else

  string ret;
  gcry_md_hd_t hd = NULL;
  gcry_error_t err = 0;
  try
    {
      err = gcry_md_open(&hd, GCRY_MD_SHA1, GCRY_MD_FLAG_SECURE | GCRY_MD_FLAG_HMAC);
      if (err)
        {
          throw OAuthException();
        }

      err = gcry_md_setkey(hd, key.c_str(), key.length());
      if (err)
        {
          throw OAuthException();
        }

      gcry_md_write(hd, input.c_str(), input.length());

      size_t digest_length = gcry_md_get_algo_dlen(GCRY_MD_SHA1);
      const guchar *digest = (const guchar *)gcry_md_read(hd, 0);
      const gchar *digest64 = g_base64_encode(digest, digest_length);
 
      ret = digest64;
    }
  catch (OAuthException &e)
    {
      g_printerr("Failed to encrypt: %s", gcry_strerror(err));
    }

  if (hd != NULL)
    {
      gcry_md_close(hd);
    }

  return ret;
#endif  
}


const string
OAuth::create_oauth_header(const string &http_method,
                           const string &uri,
                           RequestParams &parameters) const
{
  parameters.insert(custom_headers.begin(), custom_headers.end());
  
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
      backend->listen(boost::bind(&OAuth::on_resource_owner_authorization_ready, this, _1, _2, _3, _4, _5),
                      path, port);

      stringstream ss;
      ss << "http://127.0.0.1:" << port << path;
  
      RequestParams parameters;
      parameters["oauth_callback"] = ss.str();

      string http_method = "POST";
      string oauth_header = create_oauth_header(http_method, temporary_request_uri, parameters);

      backend->request(http_method, temporary_request_uri, "", oauth_header,
                       boost::bind(&OAuth::on_temporary_credentials_ready, this, _1, _2));
    }
  catch(OAuthException &oe)
    {
      failure_cb();
    }
  catch(WebBackendException &we)
    {
      failure_cb();
    }
}


void
OAuth::on_temporary_credentials_ready(int status, const string &response)
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
      failure_cb();
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
      failure_cb();
    }
}


void
OAuth::on_resource_owner_authorization_ready(const string &method, const string &query, const string &body,
                                             string &response_content_type, string &response_body)
{
  (void) body;

  try
    {
      response_content_type = "text/html";
      response_body = failure_html;
      (void) body;
      
      if (method != "GET")
        {
          g_debug("Resource owner authorization only supports GET callback");
          throw OAuthException();
        }          

      if (query == "")
        {
          g_debug("Empty response for resource owner authorization");
          throw OAuthException();
        }          

      RequestParams response_parameters;
      parse_query(query, response_parameters);

      string token = response_parameters["oauth_token"];
      string verifier = response_parameters["oauth_verifier"];

      if (token == "" || verifier == "")
        {
          throw OAuthException("Token en Verifier must be set");
        }

      response_body = success_html;
      request_token(token, verifier);
    }
  catch(WebBackendException &we)
    {
      failure_cb();
    }
  catch(OAuthException &oe)
    {
      failure_cb();
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
                       boost::bind(&OAuth::on_token_ready, this, _1, _2));
    }
  catch(WebBackendException &we)
    {
      failure_cb();
    }
  catch(OAuthException &oe)
    {
      failure_cb();
    }
  
}

void
OAuth::on_token_ready(int status, const string &response)
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

      success_cb();
    }
  catch(OAuthException &oe)
    {
      failure_cb();
    }
}
