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

#ifndef UBUNTUONESSO_HH
#define UBUNTUONESSO_HH

#include <string>
#include <map>
#include <gio/gio.h>
#include <boost/function.hpp>

class OAuth;
class IWebBackend;

#include "Exception.hh"

class UbuntuOneSSO
{
public:
  typedef boost::function<void (const std::string &, const std::string &, const std::string &, const std::string &) > PairingSuccessCallback;
  typedef boost::function<void () > PairingFailedCallback;

 	UbuntuOneSSO();
  virtual ~UbuntuOneSSO();
  
  void init(PairingSuccessCallback success_cb, PairingFailedCallback failed_cb);
  
private:
  void init_sso();
  void pair_sso();
  void pair_oauth();
  
  static void on_signal_static(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters, gpointer user_data);
  void on_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters);
  
  void on_credentials_success(const std::string &app_name, const std::map<std::string, std::string> &credentials);
  void on_credentials_failed(const std::string &app_name);
  void on_oauth_success();
  void on_oauth_failed();

  GDBusProxy *proxy;
  OAuth *oauth;
  IWebBackend *backend;
  PairingSuccessCallback success_cb;
  PairingFailedCallback failed_cb;
};
  
#endif
