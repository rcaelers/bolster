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
