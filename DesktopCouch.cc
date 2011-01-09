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

#include <sstream>


#include "DesktopCouch.hh"

#include <glib.h>
#include "boost/bind.hpp"

#include "OAuth.hh"
#include "OAuthException.hh"
#include "WebBackendSoup.hh"
#include "WebBackendException.hh"
#include "Secrets.hh"
#include "StringUtil.hh"
#include "GDBusWrapper.hh"

using namespace std;

DesktopCouch::DesktopCouch()
  : CouchDB(),
    couch_port(0)
{
}


DesktopCouch::~DesktopCouch()
{
}


void
DesktopCouch::init()
{
  CouchDB::init();
  
  init_port();
  init_secrets();
}


void
DesktopCouch::init_port()
{
  couch_dbus = DesktopCouchDBus::Ptr(new DesktopCouchDBus());
  couch_dbus->init();
  couch_dbus->get_port(boost::bind(&DesktopCouch::on_port, this, _1));
}

void
DesktopCouch::init_secrets()
{
  secrets = Secrets::Secrets::Ptr(new Secrets::Secrets());

  map<string, string> attributes;
  attributes["desktopcouch"] = "oauth";
  
  secrets->request_secret(attributes,
                          boost::bind(&DesktopCouch::on_secret_success, this, _1),
                          boost::bind(&DesktopCouch::on_secret_failed, this));
}

  
void
DesktopCouch::on_secret_success(const string &secret)
{
  vector<string> elements;
  StringUtil::split(secret, ':', elements);

  if (elements.size() == 4)
    {
      OAuth::RequestParams parameters;
      oauth->init(elements[0], elements[1], elements[2], elements[3], parameters);

      g_debug("OK %s", secret.c_str());
    }
  check_readiness();
}


void
DesktopCouch::on_secret_failed()
{
}

void
DesktopCouch::on_port(int port)
{
  if (port != 0)
    {
      g_debug("CouchDB is on port %d", port);
      couch_port = port;
    }
  else
    {
      couch_dbus->get_port(boost::bind(&DesktopCouch::on_port, this, _1));
    }

  check_readiness();
}


void
DesktopCouch::check_readiness()
{
  if (couch_port != 0 && oauth->has_credentials())
    {
      g_debug("Couch Ready");

      stringstream ss;
      ss << "http://127.0.0.1:" << couch_port << "/";

      couch_uri = ss.str();

      string out;
      oauth->request("GET", couch_uri + "_all_dbs", "", out);

      g_debug("all %s:", out.c_str());
    }
}

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
      g_debug("CouchDB is on port %d", port);
    }

  callback(port);
}
