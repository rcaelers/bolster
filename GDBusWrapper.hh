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

#ifndef GDBUSWRAPPER_HH
#define GDBUSWRAPPER_HH

#include <string>
#include <map>
#include <gio/gio.h>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

class OAuth;
class IWebBackend;

#include "Exception.hh"

class DBusObject
{
public:
  DBusObject(const std::string &service_name, const std::string &object_path, const std::string &interface_name);
  ~DBusObject();

  virtual void init();
  virtual void on_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters);

  std::string get_path() const { return object_path; }
    
private:    
  //typedef boost::function<void (GVariant *, GError *) > MethodCallback;

  static void on_signal_static(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters, gpointer user_data);
  // static void on_method_reply_static(GObject *source_object, GAsyncResult *res, gpointer user_data)
    
protected:
  GDBusProxy *proxy;

private:
  std::string service_name;
  std::string object_path;
  std::string interface_name;
};
    

class GDBusMethodReply : boost::noncopyable
{
public:
  typedef boost::function<void (GVariant *, GError *) > MethodCallback;

 	GDBusMethodReply(MethodCallback callback):
    callback(callback),
    count(1)
  {
  }

  static void cb(GObject *source_object, GAsyncResult *res, gpointer user_data)
  {
    GError *error = NULL;
    GDBusMethodReply *w = (GDBusMethodReply *)user_data;
    GVariant *var = g_dbus_proxy_call_finish((GDBusProxy*)source_object,  res, &error);

    w->callback(var, error);

    if (var != NULL)
      {
        g_variant_unref(var);
      }
    if (error != NULL)
      {
        g_error_free(error);
      }

    w->unref();
  }

private:
  void ref()
  {
    count++;
    g_debug("ref method wrapper cnt = %d", count);
  }

  void unref()
  {
    g_debug("unref method wrapper cnt = %d", count);
    if (--count == 0)
      {
        g_debug("unref method wrapper delete");
        delete this;
      }
  }
  
  MethodCallback callback;
  int count;
};


template<class K, class V>
V map_get(const std::map<K, V> m, K key)
{
  V ret;
  typename std::map<K,V>::const_iterator it;
  
  if ((it = m.find(key)) != m.end())
    {
      ret = it->second;
    }
  else
    {
      throw Exception(std::string("key ") + key + " not found");
    }
  return ret;
}
  
#endif
