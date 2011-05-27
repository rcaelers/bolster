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

#include "GDBusWrapper.hh"

#include <glib.h>

#include "Exception.hh"

using namespace std;

DBusObject::DBusObject(const std::string &service_name, const std::string &object_path, const std::string &interface_name)
  : service_name(service_name),
    object_path(object_path),
    interface_name(interface_name)
{
}


DBusObject::~DBusObject()
{
  if (proxy != NULL)
    {
      g_object_unref(proxy);
    }
}


void
DBusObject::init()
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
      throw Exception("Cannot create DBus proxy: " + error_msg);
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
DBusObject::on_signal_static(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters, gpointer user_data)
{
  DBusObject *obj = (DBusObject *)user_data;
  obj->on_signal(proxy, sender_name, signal_name, parameters);
}


void
DBusObject::on_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters)
{
  (void) proxy;
  (void) sender_name;
  (void) signal_name;
  (void) parameters;
}
