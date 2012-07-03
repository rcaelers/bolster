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
on_reply(HttpReply::Ptr reply)
{
  g_debug("google async : %d %s", reply->status, reply->body.c_str());
}

void
on_auth_ready(bool success, GoogleAuth::Ptr auth)
{
  IHttpBackend::Ptr backend = auth->get_backend();

  HttpRequest::Ptr request = HttpRequest::create();
  request->uri = "https://docs.google.com/feeds/metadata/default?v=3";
  request->method = "GET";

  HttpReply::Ptr reply = backend->request(request, boost::bind(on_reply, _1));
}

int
main(int argc, char **argv)
{
  (void) argc;
  (void) argv;

  g_type_init();

  loop = g_main_loop_new(NULL, TRUE);

  GoogleAuth::Ptr auth = GoogleAuth::create();
  auth->init(boost::bind(on_auth_ready, _1, auth));

  g_main_loop_run(loop);
  g_main_loop_unref(loop);
}
