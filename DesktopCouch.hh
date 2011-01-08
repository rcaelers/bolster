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

#ifndef DESKTOPCOUCH_HH
#define DESKTOPCOUCH_HH

#include <string>
#include <gio/gio.h>

#include "CouchDB.hh"

class Secrets;

#include "Exception.hh"

class DesktopCouch : public CouchDB
{
public:
 	DesktopCouch();
  virtual ~DesktopCouch();
  
  void init();
  
private:
  void init_secrets();
  void init_dbus();

  void check_readiness();
  
  void on_secret(bool ok, const std::string &secret);
  void on_port(GVariant *var, GError *error, GDBusProxy *proxy); 
 
  Secrets *secrets;
  int couch_port;
};
  
#endif
