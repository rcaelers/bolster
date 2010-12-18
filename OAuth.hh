#ifndef OAUTH_HH
#define OAUTH_HH

#include <string>
#include <map>
#include <vector>

class OAuth
{
private:
  typedef std::map<std::string, std::string> RequestParams;
  
public:
 	OAuth(const std::string &consumer_key,
        const std::string &consumer_secret,
        const std::string &callback_uri,
        const std::string &signature_method);

  std::string request_temporary_credentials(const std::string &http_method,
                                            const std::string &uri
                                            );

  std::string request_token_credentials(const std::string &http_method,
                                        const std::string &uri,
                                        const std::string &token,
                                        const std::string &token_secret,
                                        const std::string &pin_code);

  std::string request(const std::string &http_method, const std::string &uri, const std::string &token, const std::string &token_secret);
  
private:
  std::string escape_uri(const std::string &uri) const;
  std::string unescape_uri(const std::string &uri) const;
  std::string get_timestamp();
  std::string get_nonce();
  std::string encrypt(const std::string &input, const std::string &key) const;

  enum ParameterMode { ParameterModeHeader, ParameterModeRequest, ParameterModeSignatureBase };
  std::string create_headers(const std::string &http_method, const std::string &uri, const std::string &key, RequestParams &parameters);

  const std::string parameters_to_string(const RequestParams &parameters, ParameterMode mode) const;
  const std::string normalize_uri(const std::string &uri, RequestParams &parameters) const;

 	std::string consumer_key;
 	std::string consumer_secret;
 	std::string callback_uri;
 	std::string signature_method;
  std::string oauth_version;
};

#endif
