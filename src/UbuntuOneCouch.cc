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
UbuntuOneCouch::cleanup()
{
  delete sso;
  sso = NULL;
}

void
UbuntuOneCouch::on_pairing_success(const string &consumer_key, const string &consumer_secret,
                                   const string &token_key, const string &token_secret)
{
  cleanup();
  
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
  cleanup();
  failure();
}
