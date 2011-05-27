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

#ifndef DESKTOPCOUCHDBUS_HH
#define DESKTOPCOUCHDBUS_HH

#include <gio/gio.h>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include "GDBusWrapper.hh"

class DesktopCouchDBus : public DBusObject
{
public:
  typedef boost::shared_ptr<DesktopCouchDBus> Ptr;
  typedef boost::function<void (int) > GetPortCallback;

public:
  DesktopCouchDBus();
  void get_port(GetPortCallback callback);

private:
  void on_get_port_reply(GVariant *var, GError *error, GetPortCallback callback); 
};

#endif
