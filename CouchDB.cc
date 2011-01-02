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

#include "boost/bind.hpp"

#include "OAuth.hh"
#include "OAuthException.hh"
#include "WebBackendSoup.hh"
#include "WebBackendException.hh"
#include "StringUtil.hh"

using namespace std;

CouchDB::CouchDB()
 : oauth(NULL),
   backend(NULL)
{
}


CouchDB::~CouchDB()
{
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
  oauth = new OAuth(backend);
}
