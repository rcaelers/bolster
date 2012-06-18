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

#ifndef UBUNTUONESSO_HH
#define UBUNTUONESSO_HH

#include <string>
#include <map>
#include <gio/gio.h>
#include <boost/function.hpp>

#include "OAuth.hh"
#include "Exception.hh"

class DropboxAuth
{
public:
  typedef boost::function<void (const std::string &, const std::string &, const std::string &, const std::string &) > PairingSuccessCallback;
  typedef boost::function<void () > PairingFailedCallback;

 	DropboxAuth();
  virtual ~DropboxAuth();
  
  void init(std::string key, std::string secret);
  void init(PairingSuccessCallback success_cb, PairingFailedCallback failed_cb);

  IHttpBackend::Ptr get_backend() const
  {
    return backend;
  }
  
private:
  void on_oauth_success();
  void on_oauth_failed();

private:  
  OAuthFilter::Ptr oauth;
  OAuth::Ptr workflow;
  IHttpBackend::Ptr backend;
  PairingSuccessCallback success_cb;
  PairingFailedCallback failed_cb;
  OAuth::Settings oauth_settings;
  std::string sso_help_text;
};

  
#endif
