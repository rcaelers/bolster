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
#include <json-glib/json-glib.h>

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

static void
foreach_object_cb (JsonObject *object,
		  const char *member_name,
		  JsonNode *member_node,
		  gpointer user_data)
{
  g_debug("name: %s", member_name);
}
  
void
CouchDB::complete()
{
  string in;
  string out;

  // oauth->request("GET", couch_uri + "_all_dbs", "", out);
  // g_debug("all %s:", out.c_str());
  
  oauth->request("PUT", couch_uri + "/test", "", out);
  g_debug("Create: %s:", out.c_str());

  oauth->request("GET", couch_uri + "/test", "", out);
  g_debug("Get %s:", out.c_str());

  in = "{\"a\":\"b\"}";
  oauth->request("PUT", couch_uri + "/test/foo", in, out);
  g_debug("Add %s:", out.c_str());

  GError *error;
	JsonParser *parser = json_parser_new();

  const gchar *rev;
  
  if (!json_parser_load_from_data(parser, out.c_str(), out.length(), &error))
    {
      g_debug("JSON error: %s", error->message);
      g_error_free(error);
    }
  else
    {
      JsonNode *root_node = json_parser_get_root(parser);

      g_debug("Type: %d", json_node_get_node_type(root_node));
      
      if (json_node_get_node_type(root_node) == JSON_NODE_OBJECT)
        {
          JsonObject *root_obj = json_node_get_object(root_node);

          json_object_foreach_member(root_obj,
                                     (JsonObjectForeach) foreach_object_cb,
                                     NULL);
          
          rev = json_object_get_string_member(root_obj, "rev");
          
          g_debug("Rev: %s", rev);
        }
    }

  
  oauth->request("GET", couch_uri + "/test/foo", "", out);
  g_debug("Get %s", out.c_str());


  in = "{\"a\":\"c\", \"_rev\" : \"" + string(rev) + "\"}";
  g_debug("in: %s", in.c_str());
  oauth->request("PUT", couch_uri + "/test/foo", in, out);
  g_debug("Add %s", out.c_str());
  
  oauth->request("DELETE", couch_uri + "/test", "", out);
}
