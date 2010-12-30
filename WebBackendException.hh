// WebBackendException.hh
//
// Copyright (C) 2010 Rob Caelers <robc@krandor.org>
// All rights reserved.
//

#ifndef WEBBACKENDEXCEPTION_HH
#define WEBBACKENDEXCEPTION_HH

#include <string>
#include <exception>

class WebBackendException : public std::exception

{
public:
  explicit WebBackendException(const std::string &detail)
    : detailed_information(detail)
  {
  }

  virtual ~WebBackendException() throw()
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

#endif // WEBBACKENDEXCEPTION_HH
