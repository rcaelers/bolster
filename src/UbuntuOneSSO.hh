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

#ifndef UBUNTUONESSO_HH
#define UBUNTUONESSO_HH

#include <string>
#include <map>
#include <gio/gio.h>
#include <boost/function.hpp>

class OAuth;
class IWebBackend;

#include "Exception.hh"

class UbuntuOneSSO
{
public:
  typedef boost::function<void (const std::string &, const std::string &, const std::string &, const std::string &) > PairingSuccessCallback;
  typedef boost::function<void () > PairingFailedCallback;

 	UbuntuOneSSO();
  virtual ~UbuntuOneSSO();
  
  void init(PairingSuccessCallback success_cb, PairingFailedCallback failed_cb);
  
private:
  void init_sso();
  void pair_sso();
  void pair_oauth();
  
  static void on_signal_static(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters, gpointer user_data);
  void on_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters);
  
  void on_credentials_success(const std::map<std::string, std::string> &credentials);
  void on_credentials_failed();
  void on_oauth_success();
  void on_oauth_failed();

private:  
  GDBusProxy *proxy;
  OAuth *oauth;
  IWebBackend *backend;
  PairingSuccessCallback success_cb;
  PairingFailedCallback failed_cb;
};
  
#endif
