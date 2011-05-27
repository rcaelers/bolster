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
