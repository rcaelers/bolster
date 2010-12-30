#ifndef IWEBBACKEND_HH
#define IWEBBACKEND_HH

#include <string>
#include <boost/function.hpp>

class OAuth;

class IWebBackend
{
public:
  typedef boost::function<void (std::string) > RequestCallback;  
  typedef boost::function<void (const std::string &) > ListenCallback;

public:
  virtual ~IWebBackend() {}
  
  virtual std::string request(std::string http_method, std::string uri, std::string body, std::string header) = 0;
  virtual void request_async(std::string http_method, std::string uri, std::string body, RequestCallback callback, std::string header) = 0;

  virtual void listen(ListenCallback callback, std::string path, int &port) = 0;
};

#endif
