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


#include "UbuntuCouch.hh"

#include <glib.h>
#include "boost/bind.hpp"

#include "OAuth.hh"
#include "OAuthException.hh"
#include "WebBackendSoup.hh"
#include "WebBackendException.hh"
#include "StringUtil.hh"
#include "UbuntuOneSSO.hh"

using namespace std;

UbuntuCouch::UbuntuCouch()
  : CouchDB(),
    sso(NULL)
{
}


UbuntuCouch::~UbuntuCouch()
{
  if (sso != NULL)
    {
      delete sso;
    }
}


void
UbuntuCouch::init()
{
  sso = new UbuntuOneSSO();
  sso->init(boost::bind(&UbuntuCouch::on_pairing_success, this, _1, _2, _3, _4),
            boost::bind(&UbuntuCouch::on_pairing_failed, this));
}


void
UbuntuCouch::on_pairing_success(const string &consumer_key, const string &consumer_secret,
                                const string &token_key, const string &token_secret)
{
  OAuth::RequestParams parameters;
  oauth->init(consumer_key, consumer_secret, token_key, token_secret, parameters);

  // Retrieve prefix
}

void
UbuntuCouch::on_pairing_failed()
{
}



