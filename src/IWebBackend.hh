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

#ifndef IWEBBACKEND_HH
#define IWEBBACKEND_HH

#include <string>
#include <boost/function.hpp>

class OAuth;

class IWebBackend
{
public:
  typedef boost::function<void (int status, const std::string &response) > WebReplyCallback;
  typedef boost::function<void (const std::string &method, const std::string &query, const std::string &body,
                                std::string &response_content_type, std::string &response_body) > WebRequestCallback;

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
