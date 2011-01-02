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

#include "Exception.hh"

class Secrets
{
public:
  typedef boost::function<void (bool success, const std::string&) > SecretsResult;
  typedef std::map<std::string, std::string> Attributes;
  typedef std::list<std::string> ItemList;
  
 	Secrets();
  virtual ~Secrets();
  
  void init(const Attributes &attributes, SecretsResult callback);

private:
  void open_session();
  void close_session();
  void search(const Attributes &attributes, ItemList &locked, ItemList &unlocked);
  void unlock(const ItemList &locked, ItemList &unlocked_secrets);
  void prompt(const std::string &path);
  std::string get_secret(const std::string &item_path);

  GDBusProxy *get_secrets_proxy(const std::string &object_path, const std::string &interface_name);

  void init_secrets_service();
  void init_secrets_session(const std::string &path);
  void init_secrets_item(const std::string &path);
  void init_secrets_prompt(const std::string &path);

  
  static void on_signal_static(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters, gpointer user_data);
  void on_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters);

  GDBusProxy *secrets_service;
  GDBusProxy *secrets_session;
  GDBusProxy *secrets_item;
  GDBusProxy *secrets_prompt;

  std::string secrets_item_object_path;
  std::string secrets_session_object_path;
  std::string secrets_prompt_object_path;
  
  SecretsResult callback;
};
  
#endif
