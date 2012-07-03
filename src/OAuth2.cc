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

#include "Uri.hh"

using namespace std;

OAuth2::Ptr
OAuth2::create(IHttpBackend::Ptr backend, const Settings &settings)
{
  return Ptr(new OAuth2(backend, settings));
}


OAuth2::OAuth2(IHttpBackend::Ptr backend, const OAuth2::Settings &settings)
  : backend(backend),
    settings(settings),
    valid_until(0)
{
  callback_uri_path = "/oauth2";
}


OAuth2::~OAuth2()
{
  if (server)
    {
      server->stop();
    }
}


void
OAuth2::init(AuthReadyCallback callback)
{
  this->callback = callback;
  
  backend->set_decorator_factory(shared_from_this());
  request_authorization_grant();
}


void
OAuth2::init(std::string access_token, std::string refresh_token, time_t valid_until, AuthReadyCallback callback)
{
  this->callback = callback;
  
  this->access_token = access_token;
  this->refresh_token = refresh_token;
  this->valid_until = valid_until;
  
  backend->set_decorator_factory(shared_from_this());
  
  if (valid_until + 60 < time(NULL))
    {
      g_debug("OAuth2::init: token expired");
      request_refresh_token(false);
    }
  else
    {
      report_async_result(Ok);
    }
}


void
OAuth2::get_tokens(std::string &access_token, std::string &refresh_token, time_t &valid_until)
{
  access_token = this->access_token;
  refresh_token = this->refresh_token;
  valid_until = this->valid_until;
}


void
OAuth2::request_authorization_grant()
{
  try
    {
      int port = 0;
      server = backend->listen(callback_uri_path, port, boost::bind(&OAuth2::on_authorization_grant_ready, this, _1));

      callback_uri = boost::str(boost::format("http://127.0.0.1:%1%%2%") % port % callback_uri_path);
      
      gchar *program = g_find_program_in_path("xdg-open");
      if (program == NULL)
        {
          g_debug("Cannot find xdg-open");
          throw std::exception();
        }

      RequestParams parameters;
      string command = string(program) + " " + create_login_url(callback_uri, parameters);

      gint exit_code = 0;
      GError *error = NULL;
      if (!g_spawn_command_line_sync(command.c_str(), NULL, NULL, &exit_code, &error))
        {
          g_debug("Failed to execute xdg-open: %s %s", command.c_str(), error->message);
          throw std::exception();
        }

      if (WEXITSTATUS(exit_code) != 0)
        {
          g_debug("xdg-open returned an error exit-code");
          throw std::exception();
        }
    }
  catch(...)
    {
      report_async_result(Failed);
    }
}


HttpReply::Ptr
OAuth2::on_authorization_grant_ready(HttpRequest::Ptr request)
{
  HttpReply::Ptr reply = HttpReply::create(request);

  try
    {
      reply->content_type = "text/html";
      reply->body = settings.failure_html;
      
      if (request->method != "GET")
        {
          g_debug("Resource owner authorization only supports GET callback");
          throw std::exception();
        }          

      if (request->uri == "")
        {
          g_debug("Empty response for authorization grant");
          throw std::exception();
        }          

      RequestParams response_parameters;
      parse_query(request->uri, response_parameters);

      // TODO: handle error
      string code = response_parameters["code"];

      if (code == "")
        {
          g_debug("Code must be set");
          throw std::exception();
        }
      
      reply->body = settings.success_html;
      request_access_token(code);
    }
  catch(...)
    {
      report_async_result(Failed);
    }

  return reply;
}


void
OAuth2::request_access_token(const string &code)
{
  try
    {
      string body = boost::str(boost::format("code=%1%&client_id=%2%&client_secret=%3%&redirect_uri=%4%&grant_type=authorization_code")
                               % code
                               % settings.client_id
                               % settings.client_secret
                               % Uri::escape(callback_uri)
                               );

      HttpRequest::Ptr request = HttpRequest::create();
      request->uri = settings.token_endpoint;
      request->method = "POST";
      request->content_type = "application/x-www-form-urlencoded";
      request->body = body;

      backend->request(request, boost::bind(&OAuth2::on_access_token_ready, this, _1));
    }
  catch(...)
    {
      report_async_result(Failed);
    }
}


void
OAuth2::on_access_token_ready(HttpReply::Ptr reply)
{
  AuthResult result = Failed;
  
  try
    {
      if (reply->status != 200)
        {
          g_debug("Invalid response for token %d", reply->status);
          throw std::exception();
        }

      if (reply->body == "")
        {
          g_debug("Empty response for token");
          throw std::exception();
        }          
      
      Json::Value root;
      Json::Reader reader;
      bool ok = reader.parse(reply->body, root);

      if (ok && !root.isMember("error"))
        {
          access_token = root["access_token"].asString();
          refresh_token = root["refresh_token"].asString();
          valid_until = root["expires_in"].asInt() + time(NULL);
          
          g_debug("access_token : %s", access_token.c_str());

          result = Ok;
        }
    }
  catch(...)
    {
    }

  report_async_result(result);
}


void
OAuth2::request_refresh_token(bool sync)
{
  g_debug("request_refresh_token");
  access_token = "";

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
          backend->request(request, boost::bind(&OAuth2::on_refresh_token_ready, this, _1));
        }
    }
  catch(...)
    {
      report_async_result(Failed);
    }
}


void
OAuth2::on_refresh_token_ready(HttpReply::Ptr reply)
{
  bool success = false;
  
  try
    {
      g_debug("status = %d, resp = %s", reply->status, reply->body.c_str());
      
      if (reply->status != 200)
        {
          g_debug("Invalid response for token %d", reply->status);
          throw std::exception();
        }

      if (reply->body == "")
        {
          g_debug("Empty response for token");
          throw std::exception();
        }          
      
      Json::Value root;
      Json::Reader reader;
      bool ok = reader.parse(reply->body, root);

      if (ok && !root.isMember("error"))
        {
          access_token = root["access_token"].asString();
          valid_until = root["expires_in"].asInt() + time(NULL);
          
          g_debug("access_token : %s", access_token.c_str());

          for(list<OAuth2Filter::Ptr>::iterator it = waiting_for_refresh.begin(); it != waiting_for_refresh.end(); it++)
            {
              (*it)->set_access_token(access_token);
            }
          waiting_for_refresh.clear();
        }

      report_async_result(Ok);
    }
  catch(...)
    {
      report_async_result(Failed);
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
OAuth2::report_async_result(AuthResult result)
{
  if (server)
    {
      server->stop();
    }
  if (!callback.empty())
    {
      callback(result);
    }
}


IHttpExecute::Ptr
OAuth2::create_decorator(IHttpExecute::Ptr execute)
{
  g_debug("OAuth2::create_decorator");
  if (access_token != "")
    {
      OAuth2Filter::Ptr filter = OAuth2Filter::create(execute);
      filter->signal_refresh_request().connect(boost::bind(&OAuth2::on_refresh_request, this, filter));
      filter->set_access_token(access_token);
      return filter;
    }
  else
    {
      g_debug("OAuth2::create_decorator: no token");
      return execute;
    }
}


void
OAuth2::on_refresh_request(OAuth2Filter::Ptr filter)
{
  waiting_for_refresh.push_back(filter);
  if (waiting_for_refresh.size() == 1)
    {
      // TODO: handle sync / and mix sync+async
      request_refresh_token(filter->is_sync());
    }
}
