// main.cc --- OAuth test app
//
// Copyright (C) 2010, 2011 Rob Caelers <robc@krandor.org>
// All rights reserved.

#include <string>
#include <iostream>

#include "ICouchDB.hh"
#include "CouchDBFactory.hh"
#include "Json.hh"

using namespace std;

static GMainLoop *loop = NULL;

void run(ICouchDB *couch)
{
  string in;
  string out;
  Json *j;

  // couch->request("GET", "_all_dbs", "", out);
  // g_debug("all %s:", out.c_str());
  
  couch->request("DELETE", "/test", "", out);
  couch->request("DELETE", "/test2", "", out);

  couch->request("PUT", "/test", "", out);
  g_debug("Create DB: %s:", out.c_str());
  
  couch->request("PUT", "/test2", "", out);
  g_debug("Create DB: %s:", out.c_str());

  
  in = "{\"a\":\"b\"}";
  couch->request("PUT", "/test/foo", in, out);
  g_debug("Add %s:", out.c_str());

  j = new Json(out);
  string rev = j->get_string("rev");
  delete j;

  // in = string("{ \"source\" : \"test\",  \"target\" : \"test2\" }");
  // g_debug("Replicate %s:", in.c_str());
  // couch->request("POST", "_replicate", in, out);
  // g_debug("Replicate %s:", out.c_str());

  g_debug("Rev: %s", rev.c_str());
  
  // couch->request("GET", "/test/foo", "", out);
  // g_debug("Get %s", out.c_str());


  in = "{\"a\":\"c\", \"_rev\" : \"" + rev + "\"}";
  g_debug("in: %s", in.c_str());
  couch->request("PUT", "/test/foo", in, out);
  g_debug("Add %s", out.c_str());

  in = "{\"a\":\"d\", \"_rev\" : \"" + rev + "\"}";
  g_debug("in: %s", in.c_str());
  couch->request("PUT", "/test2/foo", in, out);
  g_debug("Add %s", out.c_str());

  in = string("{ \"source\" : \"test\",  \"target\" : \"test2\" }");
  g_debug("Replicate %s:", in.c_str());
  couch->request("POST", "_replicate", in, out);
  g_debug("Replicate %s:", out.c_str());

  in = string("{ \"source\" : \"test2\",  \"target\" : \"test\" }");
  g_debug("Replicate %s:", in.c_str());
  couch->request("POST", "_replicate", in, out);
  g_debug("Replicate %s:", out.c_str());
  
  couch->request("GET", "/test/foo?revs=true", "", out);
  g_debug("Get1 rev  %s", out.c_str());

  couch->request("GET", "/test/foo?conflicts=true", "", out);
  g_debug("Get1 c %s", out.c_str());

  j = new Json(out);
  string crev = j->get_string("_conflicts.0");
  delete j;
  g_debug("crev %s", crev.c_str());

  couch->request("GET", "/test/foo?deleted_conflicts=true", "", out);
  g_debug("Get1 dc %s", out.c_str());

  couch->request("GET", "/test2/foo?revs=true", "", out);
  g_debug("Get2 rev %s", out.c_str());

  couch->request("GET", "/test2/foo?conflicts=true", "", out);
  g_debug("Get2 c %s", out.c_str());
  
  couch->request("GET", "/test2/foo?deleted_conflicts=true", "", out);
  g_debug("Get2 dc %s", out.c_str());
  
  couch->request("GET", "/test/foo?deleted=true&rev="+ crev, "", out);
  g_debug("Get1 %s", out.c_str());

  couch->request("GET", "/test2/foo?rev="+ crev, "", out);
  g_debug("Get2 %s", out.c_str());

  g_main_loop_quit(loop);

}

int main(int argc, char **argv)
{
  (void) argc;
  (void) argv;

  g_type_init();


  // IWebBackend *backend = new WebBackendSoup();
  // web = new OAuth(backend,
  //                 "http://127.0.0.1:8888/couch/request_token/",
  //                 "http://127.0.0.1:8888/couch/authorize/",
  //                 "http://127.0.0.1:8888/couch/access_token/",
  //                 "<html><head><title>Authorization Ok</title></head><body><div><h1>Authorization Ok</h1>OK</div></body></html>",
  //                 "<html><head><title>Failed to authorize</title></head><body><div><h1>Failed to authorize</h1>Sorry</div></body></html>");
  // 
  // OAuth::RequestParams parameters;
  // web->init("Hello", "World", parameters, &result);

  for (int i = 0; i < 1000; i ++)
    {
      loop = g_main_loop_new(NULL, TRUE);
      
      // UbuntuOneCouch c;
      ICouchDB *c = CouchDBFactory::create(CouchDBFactory::Desktop);
      c->signal_ready.connect(boost::bind(run, c));
      c->init();
      
      g_main_loop_run(loop);
      g_main_loop_unref(loop);
      
      delete c;
    }
}
