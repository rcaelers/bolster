// Copyright (C) 2010, 2011, 2012 by Rob Caelers <robc@krandor.nl>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef IHTTPFILTER_HH
#define IHTTPFILTER_HH

#include <string>
#include <map>
#include <boost/shared_ptr.hpp>

#include "HttpRequest.hh"

class IHttpFilter
{
public:
  virtual ~IHttpFilter() {};
  
  typedef boost::shared_ptr<IHttpFilter> Ptr;
};

class IHttpRequestFilter : public IHttpFilter
{
public:
  typedef boost::shared_ptr<IHttpRequestFilter> Ptr;

public:
  virtual bool filter_http_request(HttpRequest::Ptr request) = 0;
};


class IHttpErrorFilter : public IHttpFilter
{
public:
  typedef boost::shared_ptr<IHttpErrorFilter> Ptr;

public:
  virtual bool filter_http_request(int error, const std::string &http_method, std::string &uri, std::string &body,
                                   std::map<std::string, std::string> &headers) = 0;
};


#endif
