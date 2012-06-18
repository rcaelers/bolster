// Copyright (C) 2010, 2011, 2012 by Rob Caelers <robc@krandor.nl>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "OAuth.hh"

#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include <glib.h>

#ifdef HAVE_CRYPTOPP  
#include <crypto++/cryptlib.h>
#include <crypto++/sha.h>
#include <crypto++/hmac.h>
#include <crypto++/base64.h>
#endif

#include "IHttpBackend.hh"
#include "Exception.hh"
#include "OAuth.hh"
#include "Uri.hh"

using namespace std;

OAuth::Ptr
OAuth::create(IHttpBackend::Ptr backend, const Settings &settings)
{
  return Ptr(new OAuth(backend, settings));
}

OAuth::OAuth(IHttpBackend::Ptr backend, const OAuth::Settings &settings)
  : backend(backend),
    settings(settings)
{
  verified_path = "/oauth-verfied";
  oauth = OAuthFilter::create();
  backend->add_filter(oauth);
}

OAuth::~OAuth()
{
  backend->stop_listen(verified_path);
  oauth->set_custom_headers();
}

void
OAuth::init(const string &consumer_key, const string &consumer_secret,
                    SuccessCallback success_cb, FailedCallback failure_cb)
{
  oauth->set_consumer(consumer_key, consumer_secret);

  this->success_cb = success_cb;
  this->failure_cb = failure_cb;
  
  request_temporary_credentials();
}


void
OAuth::request_temporary_credentials()
{
  try
    {
      int port = 0;
      backend->listen(boost::bind(&OAuth::on_resource_owner_authorization_ready, this, _1, _2, _3, _4, _5),
                      verified_path, port);

      oauth_callback = boost::str(boost::format("http://127.0.0.1:%1%%2%") % port % verified_path);
      
      OAuthFilter::RequestParams parameters;
      parameters["oauth_callback"] = oauth_callback;
      oauth->set_custom_headers(parameters);

      HttpRequest::Ptr request = HttpRequest::create();
      request->uri = settings.temporary_request_uri;
      request->method = "POST";
      
      backend->request(request, boost::bind(&OAuth::on_temporary_credentials_ready, this, _1));
    }
  catch(Exception)
    {
      failure();
    }
}


void
OAuth::on_temporary_credentials_ready(HttpReply::Ptr reply)
{
  try
    {
      g_debug("Resp : %s", reply->body.c_str());
      if (reply->status != 200)
        {
          g_debug("Invalid response for temporary credentials %d", reply->status);
          throw Exception();
        }

      if (reply->body == "")
        {
          g_debug("Empty response for temporary credentials");
          throw Exception();
        }          
      
      OAuthFilter::RequestParams response_parameters;
      parse_query(reply->body, response_parameters);

#ifdef DROPBOX
      if (response_parameters["oauth_callback_confirmed"] != "true")
        {
          g_debug("Callback not confirmed:%s", response_parameters["oauth_callback_confirmed"].c_str());
          throw Exception();
        }
#endif
      
      token = response_parameters["oauth_token"];
      string secret = response_parameters["oauth_token_secret"];

      oauth->set_token(token, secret);

      request_resource_owner_authorization();
    }
  catch(Exception)
    {
      failure();
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
          g_debug("Cannot find xdg-open");
          throw Exception();
        }

      string command = boost::str(boost::format("%1% %2%?oauth_token=%3%&oauth_callback=%4%")
                                  % program % settings.authorize_uri % Uri::escape(token) % Uri::escape(oauth_callback));
 
      gint exit_code;
      GError *error = NULL;
      if (!g_spawn_command_line_sync(command.c_str(), NULL, NULL, &exit_code, &error))
        {
          g_debug("Failed to execute xdg-open");
          throw Exception();
        }

      if (WEXITSTATUS(exit_code) != 0)
        {
          g_debug("xdg-open returned an error exit-code");
          throw Exception();
        }
    }
  catch(Exception)
    {
      failure();
    }
}


void
OAuth::on_resource_owner_authorization_ready(const string &method, const string &query, const string &body,
                                                     string &response_content_type, string &response_body)
{
  (void) body;

  g_debug("Body : %s", body.c_str());
  g_debug("Q : %s", query.c_str());
  g_debug("M : %s", method.c_str());
  
  try
    {
      response_content_type = "text/html";
      response_body = settings.failure_html;
      
      if (method != "GET")
        {
          g_debug("Resource owner authorization only supports GET callback");
          throw Exception();
        }          

      if (query == "")
        {
          g_debug("Empty response for resource owner authorization");
          throw Exception();
        }          

      OAuthFilter::RequestParams response_parameters;
      parse_query(query, response_parameters);

      string token = response_parameters["oauth_token"];
      string verifier = response_parameters["oauth_verifier"];

      if (token == "")
        {
          g_debug("Token must be set");
          throw Exception();
        }

      if (token != this->token)
        {
          g_debug("Reply for incorrect");
          throw Exception();
        }
      
      response_body = settings.success_html;
      request_token(verifier);
    }
  catch(Exception &we)
    {
      failure();
    }
}


void
OAuth::request_token(const string &verifier)
{
  g_debug("request_token %s", verifier.c_str());

  try
    {
      OAuthFilter::RequestParams parameters;
      if (verifier != "")
        {
          parameters["oauth_verifier"] = verifier;
        }
      oauth->set_custom_headers(parameters);

      HttpRequest::Ptr request = HttpRequest::create();
      request->uri = settings.token_request_uri;
      request->method = "POST";
      
      backend->request(request, boost::bind(&OAuth::on_token_ready, this, _1));
    }
  catch(Exception &we)
    {
      failure();
    }
}


void
OAuth::on_token_ready(HttpReply::Ptr reply)
{
  try
    {
      if (reply->status != 200)
        {
          g_debug("Invalid response for token %d", reply->status);
          throw Exception();
        }

      if (reply->body == "")
        {
          g_debug("Empty response for token");
          throw Exception();
        }          
      
      OAuthFilter::RequestParams response_parameters;
      parse_query(reply->body, response_parameters);

      string key = response_parameters["oauth_token"];
      string secret = response_parameters["oauth_token_secret"];

      if (key == "" || secret == "")
        {
          g_debug("No token/secret received");
          throw Exception();
        }

      g_debug("key %s", key.c_str());
      g_debug("Secret %s", secret.c_str());
      
      oauth->set_token(key, secret);
      success();
    }
  catch(Exception &oe)
    {
      failure();
    }
}


void
OAuth::parse_query(const string &query, OAuthFilter::RequestParams &params) const
{
  if (query != "")
    {
      g_debug("Query: %s", query.c_str());
      vector<string> query_params;
      boost::split(query_params, query, boost::is_any_of("&"));

      for (size_t i = 0; i < query_params.size(); ++i)
        {
          vector<string> param_elements;
          boost::split(param_elements, query_params[i], boost::is_any_of("="));
      
          if (param_elements.size() == 2)
            {
              params[param_elements[0]] = param_elements[1];
            }
        }
    }
}


void OAuth::failure()
{
  backend->stop_listen(verified_path);
  oauth->set_custom_headers();

  failure_cb();
}


void OAuth::success()
{
  backend->stop_listen(verified_path);
  oauth->set_custom_headers();

  success_cb();
}
