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
