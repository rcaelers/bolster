// main.cc --- OAuth test app
//
// Copyright (C) 2010, 2011 Rob Caelers <robc@krandor.org>
// All rights reserved.

#include <string>
#include <iostream>

#include "DesktopCouch.hh"

using namespace std;

static GMainLoop *loop = NULL;

int main(int argc, char **argv)
{
  (void) argc;
  (void) argv;

  g_type_init();

  loop = g_main_loop_new(NULL, TRUE);

  // IWebBackend *backend = new WebBackendSoup();
  // web = new OAuth(backend,
  //                 "http://127.0.0.1:8888/oauth/request_token/",
  //                 "http://127.0.0.1:8888/oauth/authorize/",
  //                 "http://127.0.0.1:8888/oauth/access_token/",
  //                 "<html><head><title>Authorization Ok</title></head><body><div><h1>Authorization Ok</h1>OK</div></body></html>",
  //                 "<html><head><title>Failed to authorize</title></head><body><div><h1>Failed to authorize</h1>Sorry</div></body></html>");
  // 
  // OAuth::RequestParams parameters;
  // web->init("Hello", "World", parameters, &result_oauth);
  
  DesktopCouch c;
  c.init();

  g_main_loop_run(loop);
  g_main_loop_unref(loop);
}
