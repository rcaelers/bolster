//
// Copyright (C) 2011 Rob Caelers <robc@krandor.nl>
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

#include "Secrets.hh"

#include <glib.h>

#include "OAuth.hh"
#include "OAuthException.hh"
#include "WebBackendSoup.hh"
#include "WebBackendException.hh"
#include "StringUtil.hh"


using namespace std;

Secrets::Secrets()
  : secrets_service(NULL),
    secrets_session(NULL),
    secrets_item(NULL),
    secrets_prompt(NULL)
{
}


Secrets::~Secrets()
{
  close_session();
}


void
Secrets::init(const Attributes &attributes, SuccessCallback success_cb, FailedCallback failed_cb)
{
  try
    {
      this->success_cb = success_cb;
      this->failed_cb = failed_cb;
      
      open_session();

      ItemList locked_secrets;
      ItemList unlocked_secrets;
      search(attributes, locked_secrets, unlocked_secrets);

      if (unlocked_secrets.size() == 0 && locked_secrets.size() > 0)
        {
          unlock(locked_secrets, unlocked_secrets);
        }
      
      if (unlocked_secrets.size() > 0)
        {
          string secret = get_secret(unlocked_secrets.front());
          success_cb(secret);
          close_session();
        }
    }
  catch (Exception)
    {
      failed_cb();
      close_session();
    }
}


void
Secrets::on_signal_static(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters, gpointer user_data)
{
  Secrets *u = (Secrets *)user_data;
  u->on_signal(proxy, sender_name, signal_name, parameters);
}


void
Secrets::on_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters)
{
  (void) proxy;
  (void) sender_name;
  
  bool dismissed;
  GVariant *result = NULL;
  
  if (string(signal_name) == "Completed")
    {
      g_variant_get(parameters, "(bv)", &dismissed, &result);

      if (!dismissed)
        {
          GVariantIter *unlocked_iter = NULL;
          g_variant_get(result, "ao", &unlocked_iter);
      
          ItemList unlocked_secrets;
          char *path;
          while (g_variant_iter_loop(unlocked_iter, "o", &path))
            {
              unlocked_secrets.push_back(path);
            }

          string secret = get_secret(unlocked_secrets.front());

          g_variant_iter_free(unlocked_iter);

          success_cb(secret);
        }
      else
        {
          failed_cb();
        }

      close_session();
      g_variant_unref(result);
    }
}


void
Secrets::open_session()
{
  init_secrets_service();
      
  GError *error = NULL;
  GVariant *result = g_dbus_proxy_call_sync(secrets_service,
                                            "OpenSession",
                                            g_variant_new("(sv)",
                                                          "plain",
                                                          g_variant_new("s", "")),
                                            G_DBUS_CALL_FLAGS_NONE,
                                            -1,
                                            NULL,
                                            &error);

  if (error != NULL)
    {
      string error_msg = error->message;
      g_error_free(error);
      throw Exception("Cannot open secrets service session: " + error_msg);
    }
      
  GVariant *var = NULL;
  char *path = NULL;
  g_variant_get(result, "(vo)", &var, &path);

  init_secrets_session(path);

  g_variant_unref(result);
  g_variant_unref(var);
  g_free(path);
}


void
Secrets::close_session()
{
  if (secrets_session != NULL)
    {
      GError *error = NULL;
      GVariant *result = g_dbus_proxy_call_sync(secrets_session,
                                                "Close",
                                                NULL,
                                                G_DBUS_CALL_FLAGS_NONE,
                                                -1,
                                                NULL,
                                                &error);
      if (error != NULL)
        {
          string error_msg = error->message;
          g_error_free(error);
        }
      else
        {
          g_variant_unref(result);
        }
    }

  if (secrets_service != NULL)
    {
      g_object_unref(secrets_service);
      secrets_service = NULL;
    }

  if (secrets_session != NULL)
    {
      g_object_unref(secrets_session);
      secrets_session = NULL;
      secrets_session_object_path = "";
    }
  
  if (secrets_item != NULL)
    {
      g_object_unref(secrets_item);
      secrets_item = NULL;
      secrets_item_object_path = "";
    }

  if (secrets_prompt != NULL)
    {
      g_object_unref(secrets_prompt);
      secrets_prompt = NULL;
      secrets_prompt_object_path = "";
    }
}

void
Secrets::search(const Attributes &attributes, ItemList &locked, ItemList &unlocked)
{
  GVariantBuilder b;
  g_variant_builder_init(&b, G_VARIANT_TYPE("a{ss}"));

  for (Attributes::const_iterator i = attributes.begin(); i != attributes.end(); i++)
    {
      g_variant_builder_add(&b, "{ss}", i->first.c_str(), i->second.c_str());
    }
          
  GError *error = NULL;
  GVariant *result = g_dbus_proxy_call_sync(secrets_service,
                                            "SearchItems",
                                            g_variant_new("(a{ss})", &b),
                                            G_DBUS_CALL_FLAGS_NONE,
                                            -1,
                                            NULL,
                                            &error);
  if (error != NULL)
    {
      string error_msg = error->message;
      g_error_free(error);
      throw Exception("Cannot open search for secrets: " + error_msg);
    }

  GVariantIter *unlocked_iter = NULL;
  GVariantIter *locked_iter = NULL;
  g_variant_get(result, "(aoao)", &unlocked_iter, &locked_iter);

  char *path;
  while (g_variant_iter_loop(unlocked_iter, "o", &path))
    {
      unlocked.push_back(path);
    }
  while (g_variant_iter_loop(locked_iter, "o", &path))
    {
      locked.push_back(path);
    }
          
  g_variant_iter_free(unlocked_iter);
  g_variant_iter_free(locked_iter);
  g_variant_unref(result);
}


void
Secrets::unlock(const ItemList &locked, ItemList &unlocked)
{
  GVariantBuilder b;
  g_variant_builder_init(&b, G_VARIANT_TYPE("ao"));

  for (ItemList::const_iterator i = locked.begin(); i != locked.end(); i++)
    {
      g_variant_builder_add(&b, "o", i->c_str());
    }

          
  GError *error = NULL;
  GVariant *result = g_dbus_proxy_call_sync(secrets_service,
                                            "Unlock",
                                            g_variant_new("(ao)", &b),
                                            G_DBUS_CALL_FLAGS_NONE,
                                            -1,
                                            NULL,
                                            &error);

  if (error != NULL)
    {
      string error_msg = error->message;
      g_error_free(error);
      throw Exception("Cannot unlock secrets: " + error_msg);
    }

  GVariantIter *unlocked_iter = NULL;
  char *prompt_path = NULL;
  g_variant_get(result, "(aoo)", &unlocked_iter, &prompt_path);

  char *path;
  while (g_variant_iter_loop(unlocked_iter, "o", &path))
    {
      unlocked.push_back(path);
    }

  if (unlocked.size() == 0 && string(prompt_path) != "/")
    {
      prompt(prompt_path);
    }

  g_variant_unref(result);
}


void
Secrets::prompt(const string &path)
{
  init_secrets_prompt(path);
  
  GError *error = NULL;
  GVariant *result = g_dbus_proxy_call_sync(secrets_prompt,
                                            "Prompt",
                                            g_variant_new("(s)", ""),
                                            G_DBUS_CALL_FLAGS_NONE,
                                            -1,
                                            NULL,
                                            &error);

  if (error != NULL)
    {
      string error_msg = error->message;
      g_error_free(error);
      throw Exception("Cannot lock secrets: " + error_msg);
    }
  
  g_variant_unref(result);
}


string
Secrets::get_secret(const string &item_path)
{
  init_secrets_item(item_path);
  
  GError *error = NULL;
  GVariant *result = g_dbus_proxy_call_sync(secrets_item,
                                            "GetSecret",
                                            g_variant_new("(o)", secrets_session_object_path.c_str()),
                                            G_DBUS_CALL_FLAGS_NONE,
                                            -1,
                                            NULL,
                                            &error);
  if (error != NULL)
    {
      string error_msg = error->message;
      g_error_free(error);
      throw Exception("Cannot get secret: " + error_msg);
    }

  GVariantIter *item_parameters = NULL;
  GVariantIter *item_secret = NULL;
  char *dummy = NULL;
  g_variant_get(result, "((oayay))", &dummy, &item_parameters, &item_secret);

  int s;
  string secret;
  while (g_variant_iter_loop(item_secret, "y", &s))
    {
      secret += char(s);
    }

  g_variant_iter_free(item_parameters);
  g_variant_iter_free(item_secret);
  g_free(dummy);
  g_variant_unref(result);

  return secret;
}


GDBusProxy *
Secrets::get_secrets_proxy(const string &object_path, const string &interface_name)
{
  GError *error = NULL;
  GDBusProxy *proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
                                                    G_DBUS_PROXY_FLAGS_NONE,
                                                    NULL,
                                                    "org.freedesktop.secrets",
                                                    object_path.c_str(),
                                                    interface_name.c_str(),
                                                    NULL,
                                                    &error);
  if (error != NULL)
    {
      string error_msg = error->message;
      g_error_free(error);
      throw Exception("Cannot open secrets: " + error_msg);
    }

  return proxy;
}


void
Secrets::init_secrets_service()
{
  if (secrets_service == NULL)
    {
      secrets_service = get_secrets_proxy("/org/freedesktop/secrets", "org.freedesktop.Secret.Service");
    }
}


void
Secrets::init_secrets_session(const string &path)
{
  if (secrets_session_object_path != path && secrets_session != NULL)
    {
      g_object_unref(secrets_session);
      secrets_session = NULL;
    }

  if (secrets_session == NULL)
    {
      secrets_session = get_secrets_proxy(path, "org.freedesktop.Secret.Session");
      if (secrets_session != NULL)
        {
          secrets_session_object_path = path;
        }
    }
}


void
Secrets::init_secrets_item(const string &path)
{
  if (secrets_item_object_path != path && secrets_item != NULL)
    {
      g_object_unref(secrets_item);
      secrets_item = NULL;
    }
  
  if (secrets_item == NULL)
    {
      secrets_item = get_secrets_proxy(path, "org.freedesktop.Secret.Item");
      if (secrets_item != NULL)
        {
          secrets_item_object_path = path;
        }
    }
}


void
Secrets::init_secrets_prompt(const string &path)
{
  if (secrets_prompt_object_path != path && secrets_prompt != NULL)
    {
      g_object_unref(secrets_prompt);
      secrets_prompt = NULL;
    }
  
  if (secrets_prompt == NULL)
    {
      secrets_prompt = get_secrets_proxy(path, "org.freedesktop.Secret.Prompt");

      if (secrets_prompt != NULL)
        {
          secrets_prompt_object_path = path;
          
          g_signal_connect(secrets_prompt,
                           "g-signal",
                           G_CALLBACK(&Secrets::on_signal_static),
                           this);
        }
    }
}

