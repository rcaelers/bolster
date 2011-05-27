//
// Copyright (C) 2010, 2011 Rob Caelers <robc@krandor.nl>
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

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
    oauth(NULL)
{
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
  // TODO: Customize help text.
  g_variant_builder_add(&b, "{ss}", "help_text", "Workrave wants to access you Ubuntu One account");

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
      // TODO: retrieve OAuth URL from Ubuntu.
      // TODO: Customize html
      backend = new WebBackendSoup();
      oauth = new OAuth();

      backend->add_filter(oauth);
      
      OAuthWorkflow *workflow = new OAuthWorkflow(backend, oauth,
                                                  "https://one.ubuntu.com/oauth/request/",
                                                  "https://one.ubuntu.com/oauth/authorize/",
                                                  "https://one.ubuntu.com/oauth/access/", 
                                                  "<html><head><title>Authorization Ok</title></head><body><div><h1>Authorization Ok</h1>OK</div></body></html>",
                                                  "<html><head><title>Failed to authorize</title></head><body><div><h1>Failed to authorize</h1>Sorry</div></body></html>");

      
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
