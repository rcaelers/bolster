// main.cc --- OAuth test app
//
// Copyright (C) 2010, 2011, 2012 Rob Caelers <robc@krandor.org>
// All rights reserved.

#include <string>
#include <iostream>

#include <boost/bind.hpp>

#include <glib.h>
#include <glib-object.h>

#include "GoogleAuth.hh"
#include "IHttpBackend.hh"
#include "HttpBackendSoup.hh"
#include "OAuth2.hh"

using namespace std;

static GMainLoop *loop = NULL;

void
on_pairing_result(bool success)
{
}

void
on_reply(HttpReply::Ptr reply)
{
  g_debug("google async : %d %s", reply->status, reply->body.c_str());
}

int main(int argc, char **argv)
{
  (void) argc;
  (void) argv;

  g_type_init();

  loop = g_main_loop_new(NULL, TRUE);

  GoogleAuth *sso = new GoogleAuth();
  sso->init(string("xxxx"),
            string("xxxx"));
  //sso->init(boost::bind(on_pairing_result, _1));

  IHttpBackend::Ptr backend = sso->get_backend();

  // string response_body;

  //IHttpBackend::Ptr backend = HttpBackendSoup::create();
  
  HttpRequest::Ptr request = HttpRequest::create();
  //request->uri = "http://www.google.com/";
  request->uri = "https://docs.google.com/feeds/metadata/default?v=3";
  request->method = "GET";

  HttpReply::Ptr reply = backend->request(request, boost::bind(on_reply, _1));
  //g_debug("google : %d %s", reply->status, reply->body.c_str());
  
  g_main_loop_run(loop);
  g_main_loop_unref(loop);
}
