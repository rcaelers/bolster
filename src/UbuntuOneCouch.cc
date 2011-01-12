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

#include "UbuntuOneCouch.hh"

#include <glib.h>
#include <glib-object.h>
#include <json-glib/json-glib.h>
#include "boost/bind.hpp"

#include "OAuth.hh"
#include "OAuthException.hh"
#include "WebBackendSoup.hh"
#include "WebBackendException.hh"
#include "StringUtil.hh"
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
  OAuth::RequestParams parameters;
  oauth->init(consumer_key, consumer_secret, token_key, token_secret, parameters);

  string account_uri = "https://one.ubuntu.com/api/account/";

  string response;
  int resp = oauth->request("GET", account_uri, "", response);

  g_debug("response: %d %s", resp, response.c_str());

	GError *error;
	JsonParser *parser = json_parser_new();

  couch_uri = "";
  if (!json_parser_load_from_data(parser, response.c_str(), response.length(), &error))
    {
      g_error_free(error);
    }
  else
    {
      // TODO: rewrite using Json class
      JsonNode *root_node = json_parser_get_root(parser);

      if (json_node_get_node_type(root_node) == JSON_NODE_OBJECT)
        {
          JsonObject *root_obj = json_node_get_object(root_node);

          if (root_obj != NULL)
            {
              JsonObject *couchdb_obj = json_object_get_object_member(root_obj, "couchdb");

              if (couchdb_obj != NULL)
                {
                  const gchar *host = json_object_get_string_member(couchdb_obj, "host");
                  const gchar *dbpath = json_object_get_string_member(couchdb_obj, "dbpath");

                  couch_uri = string(host) + "/" + g_uri_escape_string(dbpath, NULL, TRUE) + "%2F";
                  g_debug("root %s", couch_uri.c_str());
                }
            }
        }
    }

  if (couch_uri != "")
    {
      complete();
    }
  else
    {
      failure();
    }
  
  g_object_unref(G_OBJECT(parser));
}

void
UbuntuOneCouch::on_pairing_failed()
{
  failure();
}
