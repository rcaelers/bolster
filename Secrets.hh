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

#ifndef SECRETS_HH
#define SECRETS_HH

#include <string>
#include <list>
#include <map>
#include <gio/gio.h>
#include <boost/function.hpp>

class OAuth;
class IWebBackend;

namespace Secrets
{
  typedef std::map<std::string, std::string> Attributes;
  typedef std::list<std::string> ItemList;

  typedef boost::function<void (const ItemList&) > UnlockedCallback;

  
  class DBusObject
  {
  public:
    DBusObject(const std::string &service_name, const std::string &object_path, const std::string &interface_name);
    ~DBusObject();

    virtual void init();
    virtual void on_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters);

    std::string get_path() const { return object_path; }
    
  private:    
    static void on_signal_static(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters, gpointer user_data);
    
  protected:
    GDBusProxy *proxy;

  private:
    std::string service_name;
    std::string object_path;
    std::string interface_name;
  };
    
  class Item : public DBusObject
  {
  public:
    Item(const std::string &object_path);

    std::string get_secret(const std::string &session_path);
  };

  
  class Prompt : public DBusObject
  {
  public:
    
  public:
    Prompt(const std::string &object_path, UnlockedCallback callback);
    
    void prompt();

  private:
    void on_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters);

    UnlockedCallback callback;
  };

  
  class Session : public DBusObject
  {
  public:
    Session(const std::string &object_path);

    void close();
  };

  class Service : public DBusObject
  {
  public:
    Service();

    void open(Session **session);
    void search(const Attributes &attributes, ItemList &locked, ItemList &unlocked);
    void unlock(const ItemList &locked, ItemList &unlocked_secrets, UnlockedCallback callback);
    void lock(const ItemList &unlocked);
  };
  
  class Secrets
  {
  public:
    typedef boost::function<void (const std::string&) > SuccessCallback;
    typedef boost::function<void () > FailedCallback;
  
    Secrets();
    virtual ~Secrets();
  
    void init(const Attributes &attributes, SuccessCallback success_cb, FailedCallback failed_cb);

  private:
    void on_unlocked(const ItemList &unlocked_secrets);

  private:
    SuccessCallback success_cb;
    FailedCallback failed_cb;

    Service *service;
    Session *session;
    Item *item;
  };
}
#endif
