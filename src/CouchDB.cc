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
#include "Json.hh"
#include "JsonException.hh"

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

// static void
// foreach_object_cb (JsonObject *object,
// 		  const char *member_name,
// 		  JsonNode *member_node,
// 		  gpointer user_data)
// {
//   (void) object;
//   (void) member_node;
//   (void) user_data;
//   g_debug("name: %s", member_name);
// }
  
void
CouchDB::complete()
{
  string in;
  string out;
  Json *j;

  string consumer_key;
  string consumer_secret;
  string token_key;
  string token_secret;
  
  oauth->get_credentials(consumer_key, consumer_secret, token_key, token_secret);
  
  // oauth->request("GET", couch_uri + "_all_dbs", "", out);
  // g_debug("all %s:", out.c_str());
  
  oauth->request("DELETE", couch_uri + "/test", "", out);
  oauth->request("DELETE", couch_uri + "/test2", "", out);

  oauth->request("PUT", couch_uri + "/test", "", out);
  g_debug("Create DB: %s:", out.c_str());
  
  oauth->request("PUT", couch_uri + "/test2", "", out);
  g_debug("Create DB: %s:", out.c_str());

  
  in = "{\"a\":\"b\"}";
  oauth->request("PUT", couch_uri + "/test/foo", in, out);
  g_debug("Add %s:", out.c_str());

  j = new Json(out);
  string rev = j->get_string("rev");
  delete j;

  // in = string("{ \"source\" : \"test\",  \"target\" : \"test2\" }");
  // g_debug("Replicate %s:", in.c_str());
  // oauth->request("POST", couch_uri + "_replicate", in, out);
  // g_debug("Replicate %s:", out.c_str());

  g_debug("Rev: %s", rev.c_str());
  
  // oauth->request("GET", couch_uri + "/test/foo", "", out);
  // g_debug("Get %s", out.c_str());


  in = "{\"a\":\"c\", \"_rev\" : \"" + rev + "\"}";
  g_debug("in: %s", in.c_str());
  oauth->request("PUT", couch_uri + "/test/foo", in, out);
  g_debug("Add %s", out.c_str());

  in = "{\"a\":\"d\", \"_rev\" : \"" + rev + "\"}";
  g_debug("in: %s", in.c_str());
  oauth->request("PUT", couch_uri + "/test2/foo", in, out);
  g_debug("Add %s", out.c_str());

  in = string("{ \"source\" : \"test\",  \"target\" : \"test2\" }");
  g_debug("Replicate %s:", in.c_str());
  oauth->request("POST", couch_uri + "_replicate", in, out);
  g_debug("Replicate %s:", out.c_str());

  in = string("{ \"source\" : \"test2\",  \"target\" : \"test\" }");
  g_debug("Replicate %s:", in.c_str());
  oauth->request("POST", couch_uri + "_replicate", in, out);
  g_debug("Replicate %s:", out.c_str());
  
  oauth->request("GET", couch_uri + "/test/foo?revs=true", "", out);
  g_debug("Get1 rev  %s", out.c_str());

  oauth->request("GET", couch_uri + "/test/foo?conflicts=true", "", out);
  g_debug("Get1 c %s", out.c_str());

  j = new Json(out);
  string crev = j->get_string("_conflicts.0");
  delete j;
  g_debug("crev %s", crev.c_str());

  oauth->request("GET", couch_uri + "/test/foo?deleted_conflicts=true", "", out);
  g_debug("Get1 dc %s", out.c_str());

  oauth->request("GET", couch_uri + "/test2/foo?revs=true", "", out);
  g_debug("Get2 rev %s", out.c_str());

  oauth->request("GET", couch_uri + "/test2/foo?conflicts=true", "", out);
  g_debug("Get2 c %s", out.c_str());
  
  oauth->request("GET", couch_uri + "/test2/foo?deleted_conflicts=true", "", out);
  g_debug("Get2 dc %s", out.c_str());
  
  oauth->request("GET", couch_uri + "/test/foo?deleted=true&rev="+ crev, "", out);
  g_debug("Get1 %s", out.c_str());

  oauth->request("GET", couch_uri + "/test2/foo?rev="+ crev, "", out);
  g_debug("Get2 %s", out.c_str());

}
