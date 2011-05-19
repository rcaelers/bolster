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


#include "PlainCouch.hh"

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

PlainCouch::PlainCouch(ICouchDB::Params params)
  : CouchDB(),
    couch_port(0)
{
  if (params.count("uri") > 0)
    {
      couch_uri = params["uri"];
    }
  else
    {
      couch_uri = "http://127.0.0.1:5984/";
    }
}


PlainCouch::~PlainCouch()
{
}


void
PlainCouch::init()
{
  CouchDB::init();
  complete();
}
