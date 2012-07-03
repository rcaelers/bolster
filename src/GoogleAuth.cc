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
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "OAuth2.hh"
#include "OAuth2Filter.hh"
#include "HttpBackendSoup.hh"
#include "Uri.hh"

using namespace std;

const SecretSchema *
google_get_schema (void)
{
    static const SecretSchema the_schema = {
        "org.krandor.Password", SECRET_SCHEMA_NONE,
        {
            {  "number", SECRET_SCHEMA_ATTRIBUTE_INTEGER },
            {  "string", SECRET_SCHEMA_ATTRIBUTE_STRING },
            {  "even", SECRET_SCHEMA_ATTRIBUTE_BOOLEAN },
            {  "NULL", (SecretSchemaAttributeType)0 },
        }
    };
    return &the_schema;
}


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
GoogleAuth::on_auth_result(OAuth2::AuthResult result)
{
  string access_token;
  string refresh_token;
  time_t valid_until;
  
  workflow->get_tokens(access_token, refresh_token, valid_until);
  
  string password = boost::str(boost::format("%1%:%2%:%3%") % access_token % refresh_token % valid_until);
  g_debug("on_auth_result: %s", password.c_str());

  secret_password_store(GOOGLE_SCHEMA, SECRET_COLLECTION_DEFAULT, "OAuth2",
                        password.c_str(), NULL, on_password_stored, this,
                        NULL);

  // TODO:
  callback(true);
}

void
GoogleAuth::init(AsyncAuthResult callback)
{
  g_debug("GoogleAuth::init");

  this->callback = callback;

  HttpBackendSoup::Ptr backend_soup = HttpBackendSoup::create();
  backend_soup->init("Bolster");

  backend = backend_soup;
  
  workflow = OAuth2::create(backend, oauth_settings);

  secret_password_lookup(GOOGLE_SCHEMA, NULL, on_password_lookup, this, NULL);
}
 

void
GoogleAuth::on_password_lookup(GObject *source, GAsyncResult *result, gpointer data)
{
  GoogleAuth *self = (GoogleAuth *)data;
  GError *error = NULL;

  bool success = false;
  gchar *password = secret_password_lookup_finish(result, &error);

  if (error != NULL)
    {
      g_debug("secret_password_lookup: %s", error->message);
      g_error_free(error);
    }

  else if (password != NULL)
    {
      g_debug("secret_password_lookup: passwd=%s", password);

      vector<string> elements;
      boost::split(elements, password, boost::is_any_of(":"));
  
      if (elements.size() == 3 &&
          elements[0].length() > 0 &&
          elements[1].length() > 0)
        {
          time_t valid_until = 0;

          try
            {
              valid_until = boost::lexical_cast<int>(elements[2]);
            }
          catch(boost::bad_lexical_cast &) {}
          g_debug("secret_password_lookup: valid=%d", (int)valid_until);
          
          self->workflow->init(elements[0], elements[1], valid_until, boost::bind(&GoogleAuth::on_auth_result, self, _1));
          success = true;
        }
      secret_password_free(password);
    }

  if (!success)
    {
      g_debug("secret_password_lookup: obtain access");
      self->workflow->init(boost::bind(&GoogleAuth::on_auth_result, self, _1));
    }
}


void
GoogleAuth::on_password_stored(GObject *source, GAsyncResult *result, gpointer data)
{
  GoogleAuth *self = (GoogleAuth *)data;
  GError *error = NULL;

  g_debug("secret_password_store");
  
  secret_password_store_finish(result, &error);
  if (error != NULL)
    {
      g_debug("secret_password_store: %s", error->message);
      g_error_free(error);
    }
}
