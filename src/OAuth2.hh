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

#ifndef OAUTH2_HH
#define OAUTH2_HH

#include <string>
#include <map>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include "IHttpBackend.hh"
#include "OAuth2Filter.hh"

class OAuth2
{
public:
  typedef boost::shared_ptr<OAuth2> Ptr;

  typedef boost::function<void (bool) > AsyncOAuth2Result;

  struct Settings
  {
    Settings()
    {
    }
    
    Settings(const std::string &auth_endpoint,
             const std::string &token_endpoint,
             const std::string &client_id,
             const std::string &client_secret,
             const std::string &scope,
             const std::string &success_html,
             const std::string &failure_html)
      : auth_endpoint(auth_endpoint),
        token_endpoint(token_endpoint),
        client_id(client_id),
        client_secret(client_secret),
        scope(scope),
        success_html(success_html),
        failure_html(failure_html)
    {
    }
    
    std::string auth_endpoint;
    std::string token_endpoint;
    std::string client_id;
    std::string client_secret;
    std::string scope;
    std::string success_html;
    std::string failure_html;
  };
   
public:
  static Ptr create(IHttpBackend::Ptr backend, const Settings &settings);

 	OAuth2(IHttpBackend::Ptr backend, const Settings &settings);
  ~OAuth2();
  
  void init(AsyncOAuth2Result callback);
  void init(std::string access_token, std::string refresh_token);

  
private:
  typedef std::map<std::string, std::string> RequestParams;

  void request_authorization_grant();
  void on_authorization_grant_ready(const std::string &method, const std::string &query, const std::string &body,
                                    std::string &response_content_type, std::string &response_body);

  void request_access_token(const std::string &code);
  void on_access_token_ready(HttpReply::Ptr reply);


  void request_refresh_token(bool sync = false);
  void on_refresh_token_ready(HttpReply::Ptr reply);
 
  void parse_query(const std::string &query, RequestParams &params) const;
  const std::string parameters_to_string(const RequestParams &parameters) const;
  const std::string create_login_url(const std::string &redirect_uri, const RequestParams &parameters);

  void report_result(bool result);
  
private:  
  IHttpBackend::Ptr backend;
  OAuth2Filter::Ptr oauth;
  Settings settings;

  std::string callback_uri;
  std::string access_token;
  std::string refresh_token;
  
  AsyncOAuth2Result callback;
  
  std::string verified_path;
};

#endif