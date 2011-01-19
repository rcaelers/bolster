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
#include "Exception.hh"

class JsonException : public Exception

{
public:
  explicit JsonException()
    : Exception()
  {
  }

  explicit JsonException(const std::string &detail)
    : Exception(detail)
  {
  }

  virtual ~JsonException() throw()
  {
  }
};

#endif // OAUTHEXCEPTION_HH