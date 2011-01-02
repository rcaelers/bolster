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

#ifndef OAUTH_HH
#define OAUTH_HH

#include <string>
#include <map>
#include <boost/function.hpp>

#include "IWebBackend.hh"

class OAuth
{
public:
  typedef std::map<std::string, std::string> RequestParams;
  typedef boost::function<void (bool success, const std::string &) > OAuthResult;
  typedef IWebBackend::WebReplyCallback WebReplyCallback;
  
public:
 	OAuth(IWebBackend *backend,
        const std::string &temporary_request_uri = "",
        const std::string &authorize_uri = "",
        const std::string &token_request_uri = "",
        const std::string &success_html = "",
        const std::string &failure_html= "");

  void init(const std::string &consumer_key, const std::string &consumer_secret, const RequestParams &custom_headers, OAuthResult callback);
  void init(const std::string &consumer_key, const std::string &consumer_secret, const std::string &token_key, std::string const &token_secret, const RequestParams &custom_headers);
  
  int request(const std::string &http_method,
              const std::string &uri,
              const std::string &body,
              std::string &response_body);
  
  void request(const std::string &http_method,
               const std::string &uri,
               const std::string &body,
               const WebReplyCallback callback);

  bool has_credentials() const;
  void get_credentials(std::string &consumer_key, std::string &consumer_secret, std::string &token_key, std::string &token_secret);
  
private:
  enum ParameterMode { ParameterModeHeader, ParameterModeRequest, ParameterModeSignatureBase };

private:
  const std::string get_timestamp() const;
  const std::string get_nonce() const;
  const std::string escape_uri(const std::string &uri) const;
  const std::string unescape_uri(const std::string &uri) const;
  const std::string normalize_uri(const std::string &uri, RequestParams &parameters) const;
  const std::string parameters_to_string(const RequestParams &parameters, ParameterMode mode) const;
  const std::string encrypt(const std::string &input, const std::string &key) const;
  const std::string create_oauth_header(const std::string &http_method, const std::string &uri, RequestParams &parameters) const;
  void parse_query(const std::string &query, RequestParams &params) const;

  void request_temporary_credentials();
  void request_resource_owner_authorization();
  void request_token(const std::string &token, const std::string &verifier);
  void ready_temporary_credentials(int status, const std::string &response);
  void ready_resource_owner_authorization(const std::string &method, const std::string &query, const std::string &body,
                                          std::string &response_content_type, std::string &response_body);
  void ready_token(int status, const std::string &response);

private:  
  IWebBackend *backend;
  OAuthResult oauth_result_callback;
  RequestParams custom_headers;
  
  std::string temporary_request_uri;
  std::string authorize_uri;
  std::string token_request_uri;
 	std::string consumer_key;
 	std::string consumer_secret;
 	std::string token_key;
 	std::string token_secret;
 	std::string signature_method;
  std::string oauth_version;
  std::string success_html;
  std::string failure_html;
};

#endif
