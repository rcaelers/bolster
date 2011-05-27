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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "DesktopCouchDBus.hh"

#include <glib.h>
#include <boost/bind.hpp>

using namespace std;

DesktopCouchDBus::DesktopCouchDBus()
  : DBusObject("org.desktopcouch.CouchDB", "/", "org.desktopcouch.CouchDB")
{
}
    

void
DesktopCouchDBus::get_port(GetPortCallback callback)
{
  GDBusMethodReply *w = new GDBusMethodReply(boost::bind(&DesktopCouchDBus::on_get_port_reply, this, _1, _2, callback));
  
  g_dbus_proxy_call(proxy,
                    "getPort",
                    NULL,
                    G_DBUS_CALL_FLAGS_NONE,
                    -1,
                    NULL,
                    GDBusMethodReply::cb,
                    w);
}


void
DesktopCouchDBus::on_get_port_reply(GVariant *var, GError *error, GetPortCallback callback)
{
  int port = 0;
  
  if (error == NULL && var != NULL)
    {
      g_variant_get(var, "(i)", &port);
    }

  callback(port);
}
