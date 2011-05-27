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

#include "DesktopCouch.hh"

#include <sstream>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>
#include <glib.h>

#include "OAuth.hh"
#include "Secrets.hh"
#include "Uri.hh"

using namespace std;

DesktopCouch::DesktopCouch()
  : CouchDB(),
    couch_port(0)
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
  boost::split(elements, secret, boost::is_any_of(":"));
  
  if (elements.size() == 4)
    {
      oauth->set_consumer(elements[0], elements[1]);
      oauth->set_token(elements[2], elements[3]);
      check_readiness();
    }
  else
    {
      failure();
    }
}


void
DesktopCouch::on_secret_failed()
{
  failure();
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
      // FIXME: workaround for intermittent problems with DesktoCouch DBUS service
      couch_dbus->get_port(boost::bind(&DesktopCouch::on_port, this, _1));
      // FIXME: should do: failure();
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

      complete();
    }
}
