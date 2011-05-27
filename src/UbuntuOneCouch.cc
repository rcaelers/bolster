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

#include "UbuntuOneCouch.hh"

#include <boost/bind.hpp>
#include "json/json.h"

#include "IWebBackend.hh"
#include "OAuth.hh"
#include "UbuntuOneSSO.hh"

using namespace std;

UbuntuOneCouch::UbuntuOneCouch()
  : CouchDB(),
    sso(NULL)
{
}


UbuntuOneCouch::~UbuntuOneCouch()
{
  if (sso != NULL)
    {
      delete sso;
    }
}


void
UbuntuOneCouch::init()
{
  CouchDB::init();
  
  sso = new UbuntuOneSSO();
  sso->init(boost::bind(&UbuntuOneCouch::on_pairing_success, this, _1, _2, _3, _4),
            boost::bind(&UbuntuOneCouch::on_pairing_failed, this));
}


void
UbuntuOneCouch::on_pairing_success(const string &consumer_key, const string &consumer_secret,
                                   const string &token_key, const string &token_secret)
{
  oauth->set_consumer(consumer_key, consumer_secret);
  oauth->set_token(token_key, token_secret);

  // TODO: use 'resp'
  string response;
  int resp = backend->request("GET", "https://one.ubuntu.com/api/account/", "", response);
  
  Json::Value root;
  Json::Reader reader;
  bool json_ok = reader.parse(response, root);

  if (json_ok)
    {
      // TODO: handle incorrect response.
      Json::Value &couchdb_obj = root["couchdb"];

      string host = couchdb_obj["host"].asString();
      string dbpath = couchdb_obj["dbpath"].asString();

      couch_uri = host + "/" + g_uri_escape_string(dbpath.c_str(), NULL, TRUE) + "%2F";
    }

  if (couch_uri != "")
    {
      complete();
    }
  else
    {
      failure();
    }
}


void
UbuntuOneCouch::on_pairing_failed()
{
  failure();
}
