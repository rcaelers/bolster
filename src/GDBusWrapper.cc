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
