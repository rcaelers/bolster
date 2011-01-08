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
#include "boost/bind.hpp"

#include "OAuth.hh"
#include "OAuthException.hh"
#include "WebBackendSoup.hh"
#include "WebBackendException.hh"
#include "StringUtil.hh"


using namespace std;

Secrets::Secrets::Secrets()
  : service(NULL),
    session(NULL),
    item(NULL)
{
}


Secrets::Secrets::~Secrets()
{
  if (service != NULL)
    {
      delete service;
    }

  if (session != NULL)
    {
      delete session;
    }

  if (item != NULL)
    {
      delete item;
    }
}


void
Secrets::Secrets::init(const Attributes &attributes, SuccessCallback success_cb, FailedCallback failed_cb)
{
  try
    {
      this->success_cb = success_cb;
      this->failed_cb = failed_cb;

      service = new Service();
      service->init();
      
      session = NULL;
      service->open(&session);

      ItemList locked_secrets;
      ItemList unlocked_secrets;
      service->search(attributes, locked_secrets, unlocked_secrets);

      if (unlocked_secrets.size() == 0 && locked_secrets.size() > 0)
        {
          service->unlock(locked_secrets, unlocked_secrets,
                          boost::bind(&Secrets::on_unlocked, this, _1));
        }
      
      if (unlocked_secrets.size() > 0)
        {
          item = new Item(unlocked_secrets.front());
          item->init();
          
          string secret = item->get_secret(session->get_path());
          success_cb(secret);

          service->lock(unlocked_secrets);
          
          session->close();
        }
    }
  catch (Exception)
    {
      failed_cb();
      session->close();
    }
}

void
Secrets::Secrets::on_unlocked(const ItemList &unlocked_secrets)
{
  try
    {
      item = new Item(unlocked_secrets.front());
      item->init();
          
      string secret = item->get_secret(session->get_path());
      success_cb(secret);

      session->close();
    }
  catch (Exception)
    {
      failed_cb();
      session->close();
    }
}



Secrets::DBusObject::DBusObject(const std::string &service_name, const std::string &object_path, const std::string &interface_name)
  : service_name(service_name),
    object_path(object_path),
    interface_name(interface_name)
{
}


Secrets::DBusObject::~DBusObject()
{
  if (proxy != NULL)
    {
      g_object_unref(proxy);
    }
}


void
Secrets::DBusObject::init()
{
  GError *error = NULL;
  proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
                                        G_DBUS_PROXY_FLAGS_NONE,
                                        NULL,
                                        service_name.c_str(),
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

  if (error == NULL && proxy != NULL)
    {
      g_signal_connect(proxy,
                       "g-signal",
                       G_CALLBACK(&DBusObject::on_signal_static),
                       this);

    }
}


void
Secrets::DBusObject::on_signal_static(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters, gpointer user_data)
{
  DBusObject *obj = (DBusObject *)user_data;
  obj->on_signal(proxy, sender_name, signal_name, parameters);
  g_debug("on_signal static");

}


void
Secrets::DBusObject::on_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters)
{
  (void) proxy;
  (void) sender_name;
  (void) signal_name;
  (void) parameters;
  g_debug("on_signal empty");

}

  
Secrets::Service::Service()
  : DBusObject("org.freedesktop.secrets", "/org/freedesktop/secrets" , "org.freedesktop.Secret.Service") 
{
}


void
Secrets::Service::open(Session **session)
{
  GError *error = NULL;
  GVariant *result = g_dbus_proxy_call_sync(proxy,
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

  *session = new Session(path);
  (*session)->init();

  g_variant_unref(result);
  g_variant_unref(var);
  g_free(path);
}


void
Secrets::Service::search(const Attributes &attributes, ItemList &locked, ItemList &unlocked)
{
  GVariantBuilder b;
  g_variant_builder_init(&b, G_VARIANT_TYPE("a{ss}"));

  for (Attributes::const_iterator i = attributes.begin(); i != attributes.end(); i++)
    {
      g_variant_builder_add(&b, "{ss}", i->first.c_str(), i->second.c_str());
    }
          
  GError *error = NULL;
  GVariant *result = g_dbus_proxy_call_sync(proxy,
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
Secrets::Service::unlock(const ItemList &locked, ItemList &unlocked, UnlockedCallback callback)
{
  GVariantBuilder b;
  g_variant_builder_init(&b, G_VARIANT_TYPE("ao"));

  for (ItemList::const_iterator i = locked.begin(); i != locked.end(); i++)
    {
      g_variant_builder_add(&b, "o", i->c_str());
    }

          
  GError *error = NULL;
  GVariant *result = g_dbus_proxy_call_sync(proxy,
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
      Prompt *p = new Prompt(prompt_path, callback);
      p->init();
      p->prompt();
    }
  else
    {
      callback(unlocked);
    }
  
  g_variant_unref(result);
}


void
Secrets::Service::lock(const ItemList &unlocked)
{
  GVariantBuilder b;
  g_variant_builder_init(&b, G_VARIANT_TYPE("ao"));

  for (ItemList::const_iterator i = unlocked.begin(); i != unlocked.end(); i++)
    {
      g_variant_builder_add(&b, "o", i->c_str());
    }

          
  GError *error = NULL;
  GVariant *result = g_dbus_proxy_call_sync(proxy,
                                            "Lock",
                                            g_variant_new("(ao)", &b),
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

  GVariantIter *locked_iter = NULL;
  char *prompt_path = NULL;
  g_variant_get(result, "(aoo)", &locked_iter, &prompt_path);

  char *path;
  ItemList locked;
  while (g_variant_iter_loop(locked_iter, "o", &path))
    {
      locked.push_back(path);
    }

  if (locked.size() == 0 && string(prompt_path) != "/")
    {
      // Prompt *p = new Prompt(prompt_path, callback);
      // p->init();
      // p->prompt();
    }
  
  g_variant_unref(result);
}


Secrets::Session::Session(const std::string &object_path)
  : DBusObject("org.freedesktop.secrets", object_path, "org.freedesktop.Secret.Session")
{
}


void
Secrets::Session::close()
{
  if (proxy != NULL)
    {
      GError *error = NULL;
      GVariant *result = g_dbus_proxy_call_sync(proxy,
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
}



Secrets::Item::Item(const std::string &object_path)
  : DBusObject("org.freedesktop.secrets", object_path, "org.freedesktop.Secret.Item")
{
}


string
Secrets::Item::get_secret(const string &session_path)
{
  GError *error = NULL;
  GVariant *result = g_dbus_proxy_call_sync(proxy,
                                            "GetSecret",
                                            g_variant_new("(o)", session_path.c_str()),
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



Secrets::Prompt::Prompt(const std::string &object_path, UnlockedCallback callback)
  : DBusObject("org.freedesktop.secrets", object_path, "org.freedesktop.Secret.Prompt"),
    callback(callback)
{
}


void
Secrets::Prompt::prompt()
{
  GError *error = NULL;
  GVariant *result = g_dbus_proxy_call_sync(proxy,
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


void
Secrets::Prompt::on_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters)
{
  (void) proxy;
  (void) sender_name;
  
  bool dismissed;
  GVariant *result = NULL;
  ItemList unlocked_secrets;

  g_debug("on_signal");
  if (string(signal_name) == "Completed")
    {
      g_variant_get(parameters, "(bv)", &dismissed, &result);

      if (!dismissed)
        {
          GVariantIter *unlocked_iter = NULL;
          g_variant_get(result, "ao", &unlocked_iter);
      
          char *path;
          while (g_variant_iter_loop(unlocked_iter, "o", &path))
            {
              unlocked_secrets.push_back(path);
            }

          callback(unlocked_secrets);
         
          g_variant_iter_free(unlocked_iter);
        }
      else
        {
          callback(unlocked_secrets);
        }

      g_variant_unref(result);
    }
}



