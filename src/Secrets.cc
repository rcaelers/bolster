// Copyright (C) 2011 by Rob Caelers <robc@krandor.nl>
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

#include "Secrets.hh"

#include <glib.h>
#include <boost/bind.hpp>

#include "Exception.hh"

using namespace std;

Secrets::Secrets::Secrets()
{
}


Secrets::Secrets::~Secrets()
{
}


void
Secrets::Secrets::request_secret(const Attributes &attributes, SuccessCallback success_cb, FailureCallback failure_cb)
{
  try
    {
      Request::Ptr request(new Request(success_cb, failure_cb));

      if (!service)
        {
          service = Service::Ptr(new Service());
          service->init();
        }

      if (!session)
        {
          service->open(session);
        }

      ItemList locked_secrets;
      ItemList unlocked_secrets;
      service->search(attributes, locked_secrets, unlocked_secrets);

      if (unlocked_secrets.size() == 0 && locked_secrets.size() > 0)
        {
          service->unlock(locked_secrets, unlocked_secrets,
                          boost::bind(&Secrets::on_unlocked, this, request, _1));
        }
      
      if (unlocked_secrets.size() > 0)
        {
          on_unlocked(request, unlocked_secrets);
        }
    }
  catch (Exception)
    {
      failure_cb();
    }
}


void
Secrets::Secrets::on_unlocked(Request::Ptr request, const ItemList &unlocked_secrets)
{
  try
    {
      Item::Ptr item(new Item(unlocked_secrets.front()));
      item->init();
          
      string secret = item->get_secret(session->get_path());
      request->success()(secret);
    }
  catch (Exception)
    {
      request->failure()();
    }
}



Secrets::Service::Service()
  : DBusObject("org.freedesktop.secrets", "/org/freedesktop/secrets" , "org.freedesktop.Secret.Service") 
{
}


void
Secrets::Service::open(Session::Ptr &session)
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

  session = Session::Ptr(new Session(path));
  session->init();

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
  
  g_variant_unref(result);
}


Secrets::Session::Session(const std::string &object_path)
  : DBusObject("org.freedesktop.secrets", object_path, "org.freedesktop.Secret.Session")
{
}


Secrets::Session::~Session()
{
  try
    {
      close();
    }
  catch(...)
    {
      // Ignore.
    }
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
