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
#include "boost/bind.hpp"

#include "OAuth.hh"
#include "OAuthException.hh"
#include "WebBackendSoup.hh"
#include "WebBackendException.hh"
#include "StringUtil.hh"


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
  UbuntuOneSSO *u = (UbuntuOneSSO *)user_data;
  u->on_signal(proxy, sender_name, signal_name, parameters);
}


void
UbuntuOneSSO::on_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters)
{
  (void) proxy;
  (void) sender_name;
  
  gchar *parameters_str = g_variant_print(parameters, TRUE);
  g_debug("Received Signal: %s: %s\n", signal_name, parameters_str);
  g_free(parameters_str);

  GVariant *dict = NULL;
  char *app_name = NULL;
  map<string, string> info;
  
  if (string(signal_name) == "CredentialsFound" ||
      string(signal_name) == "CredentialsError")
    {
      g_variant_get(parameters, "(&s@a{ss})",
                    &app_name,
                    &dict);

      gchar *key;
      gchar *value;
      GVariantIter iter;
      g_variant_iter_init(&iter, dict);
      while (g_variant_iter_next(&iter, "{ss}", &key, &value))
        {
          info[key] = value;
        }
    }
  else
    {
      g_variant_get(parameters, "(&s)", &app_name);
    }

  if (string(signal_name) == "CredentialsFound")
    {
      on_credentials_success(app_name, info);
    }
  else if ((string(signal_name) == "CredentialsNotFound") ||
           (string(signal_name) == "CredentialsError") ||
           (string(signal_name) == "AuthorizationDenied"))
    {
      on_credentials_failed(app_name);
    }

  // FIXME: free dict & app_name?
}


void
UbuntuOneSSO::on_credentials_success(const std::string &app_name, const std::map<std::string, std::string> &credentials)
{
  (void) app_name;

  try
    {
      string consumer_key = map_get(credentials, string("consumer_key"));
      string consumer_secret = map_get(credentials, string("consumer_secret"));
      string token_key = map_get(credentials, string("token"));
      string token_secret = map_get(credentials, string("token_secret"));

      OAuth::RequestParams parameters;
      oauth->init(consumer_key, consumer_secret, token_key, token_secret, parameters);

      callback(true);
    }
  catch(Exception &e)
    {
      // Fallback to OAuth
      g_debug("on_credentials_found: exception %s", e.details().c_str());
      pair_oauth();
    }
}


void
UbuntuOneSSO::on_credentials_failed(const std::string &app_name)
{
  (void) app_name;

  // Fallback to OAuth
  g_debug("on_credentials_not_found: Trying OAuth");
  pair_oauth();
}


void
UbuntuOneSSO::on_oauth_result(bool success, const string &msg)
{
  (void) msg;
  callback(success);
}

void
UbuntuOneSSO::init(PairResult callback)
{
  this->callback = callback;

  init_oauth();
  init_sso();
  
  if (proxy != NULL)
    {
      pair_sso();
    }
  else
    {
      callback(false);
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
UbuntuOneSSO::init_oauth()
{
  // FIXME: retrieve OAuth URL from Ubuntu.
  // FIXME: Customize html
  backend = new WebBackendSoup();
  oauth = new OAuth(backend,
                  "https://one.ubuntu.com/oauth/request/",
                  "https://one.ubuntu.com/oauth/authorize/",
                  "https://one.ubuntu.com/oauth/access/", 
                  "<html><head><title>Authorization Ok</title></head><body><div><h1>Authorization Ok</h1>OK</div></body></html>",
                  "<html><head><title>Failed to authorize</title></head><body><div><h1>Failed to authorize</h1>Sorry</div></body></html>");
}


void
UbuntuOneSSO::pair_sso()
{
  GError *error = NULL;

  GVariantBuilder b;
  g_variant_builder_init(&b, G_VARIANT_TYPE("a{ss}"));
  g_variant_builder_add(&b, "{ss}", "help_text", "Workrave wants to access you Ubuntu One account");

  g_debug("1");
  g_dbus_proxy_call_sync(proxy,
                         "register",
                         g_variant_new("(sa{ss})", "Workrave", &b),
                         G_DBUS_CALL_FLAGS_NONE,
                         -1,
                         NULL,
                         &error);
  g_debug("2");
      
  if (error != NULL)
    {
      g_debug("Failed to register");
      g_error_free(error);
      pair_oauth();
    }
}


void
UbuntuOneSSO::pair_oauth()
{
  try
    {
      OAuth::RequestParams parameters;
      oauth->init("anyone", "anyone", parameters, boost::bind(&UbuntuOneSSO::on_oauth_result, this, _1, _2));
    }
  catch (OAuthException &e)
    {
      on_oauth_result(false, e.details());
    }
}
