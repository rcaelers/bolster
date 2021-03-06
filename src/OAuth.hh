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

  bool has_credentials() const;
  void get_credentials(std::string &consumer_key,
                       std::string &consumer_secret,
                       std::string &token_key,
                       std::string &token_secret);

  virtual bool filter_http_request(const std::string &http_method, std::string &uri, std::string &body,
                                   std::map<std::string, std::string> &headers);
  
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
