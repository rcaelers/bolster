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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "GoogleAuth.hh"

#include <glib.h>
#include <boost/bind.hpp>

#include "OAuth2.hh"
#include "OAuth2Filter.hh"
#include "HttpBackendSoup.hh"
#include "Uri.hh"

using namespace std;

GoogleAuth::Ptr
GoogleAuth::create()
{
  return Ptr(new GoogleAuth());
}


GoogleAuth::GoogleAuth()
{
  // TODO: Make HTML response customisable
  oauth_settings.success_html = 
    "<html>"
    "<head><title>Authorization</title></head>"
    "<body><h1>Authorization OK</h1></body>"
    "</html>";
  
  oauth_settings.failure_html =
    "<html>"
    "<head><title>Authorization</title></head>"
    "<body><h1>Authorization FAILED</h1></body>"
    "</html>";

  // TODO: 
  oauth_settings.auth_endpoint = "https://accounts.google.com/o/oauth2/auth";
  oauth_settings.token_endpoint = "https://accounts.google.com/o/oauth2/token";
  oauth_settings.client_id = "TODO";
  oauth_settings.client_secret = "TODO";
  oauth_settings.scope = "https://docs.google.com/feeds/ https://www.googleapis.com/auth/userinfo#email";
}


GoogleAuth::~GoogleAuth()
{
}


void
GoogleAuth::on_auth_result(bool success)
{
  callback(success);
}


void
GoogleAuth::init(AsyncAuthResult callback)
{
  this->callback = callback;;
  
  try
    {
      backend = HttpBackendSoup::create();
      workflow = OAuth2::create(backend, oauth_settings);

      workflow->init(boost::bind(&GoogleAuth::on_auth_result, this, _1));
    }
  catch (Exception &e)
    {
      callback(false);
    }
}


void
GoogleAuth::init(string access_token, string refresh_token)
{
  try
    {
      backend = HttpBackendSoup::create();

      workflow = OAuth2::create(backend, oauth_settings);
      workflow->init(access_token, refresh_token);
    }
  catch (Exception &e)
    {
    }
}
