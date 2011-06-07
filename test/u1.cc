// main.cc --- OAuth test app
//
// Copyright (C) 2010, 2011 Rob Caelers <robc@krandor.org>
// All rights reserved.

#include <string>
#include <iostream>

#include <glib.h>
#include <glib-object.h>

#include "ICouchDB.hh"
#include "CouchDBFactory.hh"

#include "Database.hh"
#include "Settings.hh"
#include "IWebBackend.hh"
#include "WebBackendSoup.hh"
#include "OAuth.hh"
#include "OAuthWorkflow.hh"

using namespace std;

static GMainLoop *loop = NULL;

void run1(ICouchDB *couch)
{
  string in;
  string out;

  couch->request("GET", "_all_dbs", "", out);
  g_debug("all %s:", out.c_str());

  g_main_loop_quit(loop);
}

void result_ok()
{
}

void result_nok()
{
}

int main(int argc, char **argv)
{
  (void) argc;
  (void) argv;

  g_type_init();

  loop = g_main_loop_new(NULL, TRUE);
  
  ICouchDB *c = CouchDBFactory::create(CouchDBFactory::UbuntuOne);
  c->signal_ready.connect(boost::bind(run1, c));
  c->init();
      
  g_main_loop_run(loop);
  g_main_loop_unref(loop);

  delete c;
}
