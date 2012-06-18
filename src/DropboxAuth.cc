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

#include "DropboxAuth.hh"

#include <glib.h>
#include <boost/bind.hpp>

#include "OAuth.hh"
#include "OAuthFilter.hh"
#include "HttpBackendSoup.hh"
#include "Uri.hh"

using namespace std;

DropboxAuth::DropboxAuth()
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

  // TODO: retrieve OAuth URLs from Ubuntu.
  oauth_settings.temporary_request_uri = "https://api.dropbox.com/1/oauth/request_token";
  oauth_settings.authorize_uri = "https://www.dropbox.com/1/oauth/authorize";
  oauth_settings.token_request_uri = "https://api.dropbox.com/1/oauth/access_token";

  // TODO: Customize help text.
  sso_help_text = "Workrave wants to access you Dropbox account";
}


DropboxAuth::~DropboxAuth()
{
}


void
DropboxAuth::on_oauth_success()
{
  string consumer_key;
  string consumer_secret;
  string token_key;
  string token_secret;
  
  oauth->get_credentials(consumer_key, consumer_secret, token_key, token_secret);
  success_cb(consumer_key, consumer_secret, token_key, token_secret);
}

void
DropboxAuth::on_oauth_failed()
{
  failed_cb();
}

void
DropboxAuth::init(PairingSuccessCallback success_cb, PairingFailedCallback failed_cb)
{
  this->success_cb = success_cb;
  this->failed_cb = failed_cb;
  
  try
    {
      backend = HttpBackendSoup::create();
      workflow = OAuth::create(backend, oauth_settings);

      workflow->init("xxx", "xxx",
                     boost::bind(&DropboxAuth::on_oauth_success, this),
                     boost::bind(&DropboxAuth::on_oauth_failed, this));
    }
  catch (Exception &e)
    {
      failed_cb();
    }
}


void
DropboxAuth::init(string key, string secret)
{
  try
    {
      backend = HttpBackendSoup::create();
      oauth = OAuthFilter::create();

      backend->add_filter(oauth);
      
      oauth->set_consumer("xxx", "xxx");
      oauth->set_token(key, secret);
    }
  catch (Exception &e)
    {
    }
}
