#ifndef IWEBBACKEND_HH
#define IWEBBACKEND_HH

#include <string>
#include <boost/function.hpp>

class OAuth;

class IWebBackend
{
public:
  typedef boost::function<void (int status, const std::string &) > WebReplyCallback;
  typedef boost::function<void (const std::string &) > WebRequestCallback;

public:
  virtual ~IWebBackend() {}
  
  virtual int request(const std::string &http_method,
                      const std::string &uri,
                      const std::string &body,
                      const std::string &oauth_header,
                      std::string &response_body) = 0;
  
  virtual void request(const std::string &http_method,
                       const std::string &uri,
                       const std::string &body,
                       const std::string &oauth_header,
                       const WebReplyCallback callback) = 0;
  
  virtual void listen(const WebRequestCallback callback, const std::string &path, int &port) = 0;
};

#endif
