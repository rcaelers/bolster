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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "UbuntuOneSSO.hh"

#include <glib.h>
#include <boost/bind.hpp>

#include "OAuth.hh"
#include "OAuthWorkflow.hh"
#include "WebBackendSoup.hh"
#include "Uri.hh"
#include "GDBusWrapper.hh"

using namespace std;

UbuntuOneSSO::UbuntuOneSSO()
  : proxy(NULL),
    handler_id(0),
    oauth(NULL),
    workflow(NULL),
    backend(NULL)
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
  oauth_settings.temporary_request_uri = "https://one.ubuntu.com/oauth/request/";
  oauth_settings.authorize_uri = "https://one.ubuntu.com/oauth/authorize/";
  oauth_settings.token_request_uri = "https://one.ubuntu.com/oauth/access/";

  // TODO: Customize help text.
  sso_help_text = "Workrave wants to access you Ubuntu One account";
}


UbuntuOneSSO::~UbuntuOneSSO()
{
  if (proxy != NULL)
    {
      g_object_unref(proxy);
    }

  if (oauth != NULL)
    {
      delete oauth;
    }

  if (workflow != NULL)
    {
      delete workflow;
    }

  if (backend != NULL)
    {
      delete backend;
    }
}


void
UbuntuOneSSO::on_signal_static(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters, gpointer user_data)
{
  UbuntuOneSSO *sso = (UbuntuOneSSO *)user_data;
  sso->on_signal(proxy, sender_name, signal_name, parameters);
}


void
UbuntuOneSSO::on_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters)
{
  (void) proxy;
  (void) sender_name;
  
  char *app_name = NULL;
  map<string, string> info;
  
  if (string(signal_name) == "CredentialsFound" ||
      string(signal_name) == "CredentialsError")
    {
      GVariant *dict = NULL;
      g_variant_get(parameters, "(&s@a{ss})", &app_name, &dict);

      if (dict != NULL)
        {
          gchar *key;
          gchar *value;
          GVariantIter iter;
          g_variant_iter_init(&iter, dict);
          while (g_variant_iter_next(&iter, "{ss}", &key, &value))
            {
              info[key] = value;
            }
        }
    }
  if (string(signal_name) == "CredentialsFound")
    {
      on_credentials_success(info);
    }
  else
    {
      on_credentials_failed();
    }

  // FIXME: free dict & app_name?
}


void
UbuntuOneSSO::on_credentials_success(const map<string, string> &credentials)
{
  try
    {
      string consumer_key = map_get(credentials, string("consumer_key"));
      string consumer_secret = map_get(credentials, string("consumer_secret"));
      string token_key = map_get(credentials, string("token"));
      string token_secret = map_get(credentials, string("token_secret"));

      success_cb(consumer_key, consumer_secret, token_key, token_secret);
    }
  catch(Exception &e)
    {
      // Fallback to OAuth
      pair_oauth();
    }
}


void
UbuntuOneSSO::on_credentials_failed()
{
  // Fallback to OAuth
  pair_oauth();
}


void
UbuntuOneSSO::on_oauth_success()
{
  string consumer_key;
  string consumer_secret;
  string token_key;
  string token_secret;
  
  oauth->get_credentials(consumer_key, consumer_secret, token_key, token_secret);
  success_cb(consumer_key, consumer_secret, token_key, token_secret);

  delete oauth;
  oauth = NULL;
}

void
UbuntuOneSSO::on_oauth_failed()
{
  failed_cb();

  delete oauth;
  oauth = NULL;
}

void
UbuntuOneSSO::init(PairingSuccessCallback success_cb, PairingFailedCallback failed_cb)
{
  this->success_cb = success_cb;
  this->failed_cb = failed_cb;
  
  init_sso();
  
  if (proxy != NULL)
    {
      pair_sso();
    }
  else
    {
      pair_oauth();
    }
}


void
UbuntuOneSSO::init_sso()
{
  GError *error = NULL;

  proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
                                        G_DBUS_PROXY_FLAGS_NONE,
                                        NULL,
                                        "com.ubuntu.sso",
                                        "/com/ubuntu/sso/credentials",
                                        "com.ubuntu.sso.CredentialsManagement",
                                        NULL,
                                        &error);
  if (proxy != NULL)
    {
      g_signal_connect (proxy,
                        "g-signal",
                        G_CALLBACK(&UbuntuOneSSO::on_signal_static),
                        this);
    }
}


void
UbuntuOneSSO::pair_sso()
{
  GError *error = NULL;
  GVariantBuilder b;
  
  g_variant_builder_init(&b, G_VARIANT_TYPE("a{ss}"));
  g_variant_builder_add(&b, "{ss}", "help_text", sso_help_text.c_str());

  g_dbus_proxy_call_sync(proxy,
                         "register",
                         g_variant_new("(sa{ss})", "Ubuntu One", &b),
                         G_DBUS_CALL_FLAGS_NONE,
                         -1,
                         NULL,
                         &error);
  if (error != NULL)
    {
      g_error_free(error);
      pair_oauth();
    }
}


void
UbuntuOneSSO::pair_oauth()
{
  try
    {
      backend = new WebBackendSoup();
      oauth = new OAuth();
      workflow = new OAuthWorkflow(backend, oauth, oauth_settings);

      backend->add_filter(oauth);
      
      OAuth::RequestParams parameters;
      workflow->init("anyone", "anyone",
                     boost::bind(&UbuntuOneSSO::on_oauth_success, this),
                     boost::bind(&UbuntuOneSSO::on_oauth_failed, this));
    }
  catch (Exception &e)
    {
      failed_cb();
    }
}
