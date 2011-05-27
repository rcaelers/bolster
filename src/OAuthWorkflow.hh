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
