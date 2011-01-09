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
#include <boost/shared_ptr.hpp>

#include "GDBusWrapper.hh"

class OAuth;
class IWebBackend;

namespace Secrets
{
  typedef std::map<std::string, std::string> Attributes;
  typedef std::list<std::string> ItemList;

  typedef boost::function<void (const std::string&) > SuccessCallback;
  typedef boost::function<void () > FailureCallback;
  typedef boost::function<void (const ItemList&) > UnlockedCallback;

  
  class Item : public DBusObject
  {
  public:
    typedef boost::shared_ptr<Item> Ptr;
    
  public:
    Item(const std::string &object_path);

    std::string get_secret(const std::string &session_path);
  };

  
  class Prompt : public DBusObject
  {
  public:
    typedef boost::shared_ptr<Prompt> Ptr;
    
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
    typedef boost::shared_ptr<Session> Ptr;

  public:
    Session(const std::string &object_path);
    ~Session();

    void close();
  };

  class Service : public DBusObject
  {
  public:
    typedef boost::shared_ptr<Service> Ptr;

  public:
    Service();

    void open(Session::Ptr &session);
    void search(const Attributes &attributes, ItemList &locked, ItemList &unlocked);
    void unlock(const ItemList &locked, ItemList &unlocked_secrets, UnlockedCallback callback);
    void lock(const ItemList &unlocked);
  };


  class Request
  {
  public:
    typedef boost::shared_ptr<Request> Ptr;
    
    Request(SuccessCallback success_cb, FailureCallback failure_cb)
      : success_cb(success_cb), failure_cb(failure_cb) {}

    SuccessCallback success() const { return success_cb; }
    FailureCallback failure() const { return failure_cb; }
    
  private:
    SuccessCallback success_cb;
    FailureCallback failure_cb;
  };
    
  class Secrets
  {
  public:
    typedef boost::shared_ptr<Secrets> Ptr;

  public:
    Secrets();
    virtual ~Secrets();
  
    void request_secret(const Attributes &attributes, SuccessCallback success_cb, FailureCallback failure_cb);

  private:
    void on_unlocked(Request::Ptr request, const ItemList &unlocked_secrets);
    void close();
    
  private:
    Service::Ptr service;
    Session::Ptr session;
  };
}
#endif
