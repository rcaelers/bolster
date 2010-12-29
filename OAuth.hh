#ifndef OAUTH_HH
#define OAUTH_HH

#include <string>
#include <map>
#include <vector>

#include <iostream>

class IWebBackend;

class OAuth
{
private:
  typedef std::map<std::string, std::string> RequestParams;
  
public:
 	OAuth(IWebBackend *backend);

  void init(std::string consumer_key, std::string consumer_secret);
  void init(std::string consumer_key, std::string consumer_secret, std::string token_key, std::string token_secret);
  void set_callback(std::string callback);
  void set_verifier(std::string verifier);
  
  std::string escape_uri(const std::string &uri) const;
  std::string unescape_uri(const std::string &uri) const;

  std::string get_request_header(const std::string &http_method, const std::string &uri) const;
  
private:
  std::string get_timestamp() const;
  std::string get_nonce() const;
  std::string encrypt(const std::string &input, const std::string &key) const;
  
  enum ParameterMode { ParameterModeHeader, ParameterModeRequest, ParameterModeSignatureBase };
  const std::string parameters_to_string(const RequestParams &parameters, ParameterMode mode) const;

  void request_temporary_credentials();
  void request_resource_owner_authorization();
  
  std::string create_headers(const std::string &http_method, const std::string &uri, RequestParams &parameters) const;
  const std::string normalize_uri(const std::string &uri, RequestParams &parameters) const;
  void parse_query(std::string query, RequestParams &params) const;

  IWebBackend *backend;
  
 	std::string consumer_key;
 	std::string consumer_secret;
 	std::string token_key;
 	std::string token_secret;
 	std::string callback;
  std::string verifier;
 	std::string signature_method;
  std::string oauth_version;
};

#endif
