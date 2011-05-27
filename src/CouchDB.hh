// Copyright (C) 2010, 2011 by Rob Caelers <robc@krandor.nl>
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

#ifndef COUCHDB_HH
#define COUCHDB_HH

#include <string>
#include <boost/signals2.hpp>

#include "ICouchDB.hh"

class OAuth;
class IWebBackend;

class CouchDB : public ICouchDB
{
public:
 	CouchDB();
  virtual ~CouchDB();
  
  virtual void init();

  int request(const std::string &http_method,
              const std::string &uri,
              const std::string &body,
              std::string &response_body);

  bool is_ready() const;
  std::string get_couch_uri() const;
  
private:
  void init_oauth();
  
protected:
  void complete();
  void failure();
  
protected:
  std::string couch_uri;
  OAuth *oauth;
  IWebBackend *backend;

private:
  bool ready;
};
  
#endif
