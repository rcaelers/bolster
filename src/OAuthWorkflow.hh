// Copyright (C) 2010, 2011 by Rob Caelers <robc@krandor.nl>
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

#ifndef OAUTHWORKFLOW_HH
#define OAUTHWORKFLOW_HH

#include <string>
#include <map>
#include <boost/function.hpp>

#include "IWebBackend.hh"
#include "OAuth.hh"

class OAuthWorkflow
{
public:
  typedef boost::function<void () > SuccessCallback;
  typedef boost::function<void () > FailedCallback;

  typedef IWebBackend::WebReplyCallback WebReplyCallback;
  
public:
 	OAuthWorkflow(IWebBackend *backend,
                OAuth *oauth,
                const std::string &temporary_request_uri = "",
                const std::string &authorize_uri = "",
                const std::string &token_request_uri = "",
                const std::string &success_html = "",
                const std::string &failure_html= "");
  ~OAuthWorkflow();
  
  void init(const std::string &consumer_key,
            const std::string &consumer_secret,
            SuccessCallback success_cb,
            FailedCallback failure_cb);
  
private:
  void request_temporary_credentials();
  void request_resource_owner_authorization();
  void request_token(const std::string &verifier);

  void on_temporary_credentials_ready(int status, const std::string &response);
  void on_resource_owner_authorization_ready(const std::string &method, const std::string &query, const std::string &body,
                                             std::string &response_content_type, std::string &response_body);
  void on_token_ready(int status, const std::string &response);

  void parse_query(const std::string &query, OAuth::RequestParams &params) const;

  void failure();
  void success();
  
private:  
  IWebBackend *backend;
  OAuth *oauth;

  std::string token;
  
  SuccessCallback success_cb;
  FailedCallback failure_cb;
  
  std::string verified_path;
  std::string temporary_request_uri;
  std::string authorize_uri;
  std::string token_request_uri;
  std::string success_html;
  std::string failure_html;
};

#endif
