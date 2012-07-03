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
#include <boost/shared_ptr.hpp>

#include <secret/secret.h>

#include "OAuth2Filter.hh"
#include "OAuth2.hh"

#ifdef __cplusplus
extern "C" {
#endif

  const SecretSchema * google_get_schema(void) G_GNUC_CONST;

#define GOOGLE_SCHEMA  google_get_schema()

#ifdef __cplusplus
}
#endif
  
class GoogleAuth
{
public:
  typedef boost::shared_ptr<GoogleAuth> Ptr;
  typedef boost::function<void (bool) > AsyncAuthResult;

  static Ptr create();
  
 	GoogleAuth();
  virtual ~GoogleAuth();
  
  void init(std::string access_token, std::string refresh_token);
  void init(AsyncAuthResult callback);

  IHttpBackend::Ptr get_backend() const
  {
    return backend;
  }
  
private:
  void on_auth_result(OAuth2::AuthResult result);

  static void on_password_lookup(GObject *source, GAsyncResult *result, gpointer data);
  static void on_password_stored(GObject *source, GAsyncResult *result, gpointer data);
  
private:  
  OAuth2::Ptr workflow;
  IHttpBackend::Ptr backend;
  AsyncAuthResult callback;
  OAuth2::Settings oauth_settings;
};

  
#endif
