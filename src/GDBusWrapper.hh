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
  }

  void unref()
  {
    if (--count == 0)
      {
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
