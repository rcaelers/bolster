#ifndef OAUTH_HH
#define OAUTH_HH

#include <string>
#include <map>
#include <boost/function.hpp>

class IWebBackend;

class OAuth
{
public:
  typedef boost::function<void (bool success, const std::string &) > OAuthResult;
  
public:
 	OAuth(IWebBackend *backend);

  void init(const std::string &consumer_key, const std::string &consumer_secret, OAuthResult callback);
  void init(const std::string &consumer_key, const std::string &consumer_secret, const std::string &token_key, std::string const &token_secret);
  
private:
  typedef std::map<std::string, std::string> RequestParams;
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
  void ready_resource_owner_authorization(const std::string &response);
  void ready_token(int status, const std::string &response);

private:  
  IWebBackend *backend;
  OAuthResult oauth_result_callback;
  
 	std::string consumer_key;
 	std::string consumer_secret;
 	std::string token_key;
 	std::string token_secret;
 	std::string signature_method;
  std::string oauth_version;
};

#endif
