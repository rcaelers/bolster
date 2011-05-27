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

#include "CouchDB.hh"

#include <boost/bind.hpp>

#include "OAuth.hh"
#include "WebBackendSoup.hh"
#include "Uri.hh"

using namespace std;

CouchDB::CouchDB()
  : oauth(NULL),
    backend(NULL),
    ready(false)
{
}


CouchDB::~CouchDB()
{
  signal_ready.disconnect_all_slots();
  signal_failure.disconnect_all_slots();

  if (oauth != NULL)
    {
      delete oauth;
    }

  if (backend != NULL)
    {
      delete backend;
    }
}


void
CouchDB::init()
{
  init_oauth();
}


void
CouchDB::init_oauth()
{
  backend = new WebBackendSoup();
  oauth = new OAuth();

  backend->add_filter(oauth);
}


void
CouchDB::complete()
{
  ready = true;
  signal_ready();
}


void
CouchDB::failure()
{
  ready = false;
  couch_uri = "";
  signal_failure();
}


bool
CouchDB::is_ready() const
{
  return ready;
}


std::string
CouchDB::get_couch_uri() const
{
  return couch_uri;
}


int
CouchDB::request(const std::string &http_method,
                 const std::string &uri,
                 const std::string &body,
                 std::string &response_body)
{
  return backend->request(http_method, couch_uri + uri, body, response_body);
}
