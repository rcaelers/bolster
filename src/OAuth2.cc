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

#include "OAuth2.hh"

#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include <glib.h>

#include "json/json.h"

#include "IHttpBackend.hh"
#include "Exception.hh"
#include "OAuth2.hh"
#include "Uri.hh"

using namespace std;

OAuth2::Ptr
OAuth2::create(IHttpBackend::Ptr backend, const Settings &settings)
{
  return Ptr(new OAuth2(backend, settings));
}


OAuth2::OAuth2(IHttpBackend::Ptr backend, const OAuth2::Settings &settings)
  : backend(backend),
    settings(settings)
{
  verified_path = "/oauth-verfied";
  oauth = OAuth2Filter::create();
  backend->add_filter(oauth);
}


OAuth2::~OAuth2()
{
  backend->stop_listen(verified_path);
}


void
OAuth2::init(AsyncOAuth2Result callback)
{
  this->callback = callback;
  
  request_authorization_grant();
}


void
OAuth2::init(std::string access_token, std::string refresh_token)
{
  this->access_token = access_token;
  this->refresh_token = refresh_token;

  //request_refresh_token();
  oauth->set_access_token(access_token);
}


void
OAuth2::request_authorization_grant()
{
  try
    {
      int port = 0;
      backend->listen(boost::bind(&OAuth2::on_authorization_grant_ready, this, _1, _2, _3, _4, _5),
                      verified_path, port);

      callback_uri = boost::str(boost::format("http://127.0.0.1:%1%%2%") % port % verified_path);
      
      gchar *program = g_find_program_in_path("xdg-open");
      if (program == NULL)
        {
          g_debug("Cannot find xdg-open");
          throw Exception();
        }

      RequestParams parameters;
      string command = string(program) + " " + create_login_url(callback_uri, parameters);

      gint exit_code;
      GError *error = NULL;
      if (!g_spawn_command_line_sync(command.c_str(), NULL, NULL, &exit_code, &error))
        {
          g_debug("Failed to execute xdg-open: %s %s", command.c_str(), error->message);
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
      report_result(false);
    }
}


void
OAuth2::on_authorization_grant_ready(const string &method, const string &query, const string &body,
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

      RequestParams response_parameters;
      parse_query(query, response_parameters);

      // TODO: handle error
      string code = response_parameters["code"];

      if (code == "")
        {
          g_debug("Code must be set");
          throw Exception();
        }
      
      response_body = settings.success_html;
      request_access_token(code);
    }
  catch(Exception &we)
    {
      report_result(false);
    }
}


void
OAuth2::request_access_token(const string &code)
{
  g_debug("request_access_token");

  try
    {
      string body = boost::str(boost::format("code=%1%&client_id=%2%&client_secret=%3%&redirect_uri=%4%&grant_type=authorization_code")
                               % code
                               % settings.client_id
                               % settings.client_secret
                               % Uri::escape(callback_uri)
                               );
      g_debug("body %s", body.c_str());

      HttpRequest::Ptr request = HttpRequest::create();
      request->uri = settings.token_endpoint;
      request->method = "POST";
      request->content_type = "application/x-www-form-urlencoded";
      request->body = body;

      backend->request(request, boost::bind(&OAuth2::on_access_token_ready, this, _1));
    }
  catch(Exception &we)
    {
      report_result(false);
    }
}


void
OAuth2::on_access_token_ready(HttpReply::Ptr reply)
{
  try
    {
      g_debug("status = %d, resp = %s", reply->status, reply->body.c_str());
      
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
      
      Json::Value root;
      Json::Reader reader;
      bool ok = reader.parse(reply->body, root);

      if (ok && !root.isMember("error"))
        {
          string access_token = root["access_token"].asString();

          g_debug("access_token : %s", access_token.c_str());
          oauth->set_access_token(access_token);
        }

      report_result(true);
    }
  catch(Exception &oe)
    {
      report_result(false);
    }
}


void
OAuth2::request_refresh_token(bool sync)
{
  g_debug("request_refresh_token");

  try
    {
      RequestParams parameters;

      string body = boost::str(boost::format("client_id=%1%&client_secret=%2%&refresh_token=%3%&grant_type=refresh_token")
                               % settings.client_id
                               % settings.client_secret
                               % refresh_token
                               );
      g_debug("body %s", body.c_str());

      HttpRequest::Ptr request = HttpRequest::create();
      request->uri = settings.token_endpoint;
      request->method = "POST";
      request->content_type = "application/x-www-form-urlencoded";
      request->body = body;
          
      if (sync)
        {
          HttpReply::Ptr reply = backend->request(request);
          on_refresh_token_ready(reply);
        }
      else
        {
          backend->request(request, boost::bind(&OAuth2::on_access_token_ready, this, _1));
        }
    }
  catch(Exception &we)
    {
      report_result(false);
    }
}



void
OAuth2::on_refresh_token_ready(HttpReply::Ptr reply)
{
  try
    {
      g_debug("status = %d, resp = %s", reply->status, reply->body.c_str());
      
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
      
      Json::Value root;
      Json::Reader reader;
      bool ok = reader.parse(reply->body, root);

      if (ok && !root.isMember("error"))
        {
          string access_token = root["access_token"].asString();

          g_debug("access_token : %s", access_token.c_str());
          oauth->set_access_token(access_token);
        }

      report_result(true);
    }
  catch(Exception &oe)
    {
      report_result(false);
    }
}


const string
OAuth2::parameters_to_string(const RequestParams &parameters) const
{
  string ret;

  for(RequestParams::const_iterator it = parameters.begin(); it != parameters.end(); it++)
    {
      string param = Uri::escape(it->first) + "=" + Uri::escape(it->second);
      if (ret != "")
        {
          ret += "&";
        }
      ret += param;
    }
  
  return ret;
}


const string
OAuth2::create_login_url(const string &redirect_uri, const RequestParams &parameters)
{
  string uri = boost::str(boost::format("%1%?response_type=code&client_id=%2%&redirect_uri=%3%&scope=%4%")
                          % settings.auth_endpoint
                          % Uri::escape(settings.client_id)
                          % Uri::escape(redirect_uri)
                          % Uri::escape(settings.scope));

  string extra_parameters = parameters_to_string(parameters);
  if (extra_parameters != "")
    {
      uri += "&" + extra_parameters;
    }

  return uri;
}


void
OAuth2::parse_query(const string &query, RequestParams &params) const
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


void
OAuth2::report_result(bool success)
{
  backend->stop_listen(verified_path);
  callback(success);
}
