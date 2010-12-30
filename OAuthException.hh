// OAuthException.hh
//
// Copyright (C) 2010 Rob Caelers <robc@krandor.org>
// All rights reserved.
//

#ifndef OAUTHEXCEPTION_HH
#define OAUTHEXCEPTION_HH

#include <string>
#include <exception>

class OAuthException : public std::exception

{
public:
  explicit OAuthException()
  {
  }

  explicit OAuthException(const std::string &detail)
    : detailed_information(detail)
  {
  }

  virtual ~OAuthException() throw()
  {
  }

  virtual std::string details() const throw()
  {
    return detailed_information;
  }

private:
  //
  std::string detailed_information;
};

#endif // OAUTHEXCEPTION_HH
