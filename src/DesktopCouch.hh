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

#ifndef DESKTOPCOUCH_HH
#define DESKTOPCOUCH_HH

#include <string>

#include "CouchDB.hh"
#include "DesktopCouchDBus.hh"
#include "Secrets.hh"

class DesktopCouch : public CouchDB
{
public:
 	DesktopCouch();

  void init();
  
private:
  void init_port();
  void init_secrets();

  void check_readiness();
  
  void on_secret_success(const std::string &secret);
  void on_secret_failed();
  void on_port(int port); 

private:  
  DesktopCouchDBus::Ptr couch_dbus;
  Secrets::Secrets::Ptr secrets;
  int couch_port;
};
  
#endif
