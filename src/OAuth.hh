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

#include "IHttpRequestFilter.hh"

class OAuth : public IHttpRequestFilter
{
public:
  typedef std::map<std::string, std::string> RequestParams;

public:
 	OAuth();

  void set_consumer(const std::string &consumer_key, const std::string &consumer_secret);
  void set_token(const std::string &token_key, const std::string &token_secret);
  void set_custom_headers(const RequestParams &custom_headers = RequestParams());

  virtual bool filter_http_request(const std::string &http_method, std::string &uri, std::string &body,
                                   std::map<std::string, std::string> &headers);
  
  bool has_credentials() const;
  void get_credentials(std::string &consumer_key,
                       std::string &consumer_secret,
                       std::string &token_key,
                       std::string &token_secret);
  
private:
  enum ParameterMode { ParameterModeHeader, ParameterModeRequest, ParameterModeSignatureBase };

private:
  const std::string get_timestamp() const;
  const std::string get_nonce() const;
  const std::string normalize_uri(const std::string &uri, RequestParams &parameters) const;
  const std::string parameters_to_string(const RequestParams &parameters, ParameterMode mode) const;
  const std::string encrypt(const std::string &input, const std::string &key) const;
  const std::string create_oauth_header(const std::string &http_method, const std::string &uri, RequestParams &parameters) const;

private:  
  RequestParams custom_headers;

 	std::string consumer_key;
 	std::string consumer_secret;
 	std::string token_key;
 	std::string token_secret;
 	std::string signature_method;
  std::string oauth_version;
};

#endif
