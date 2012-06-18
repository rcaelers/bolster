// main.cc --- OAuth test app
//
// Copyright (C) 2010, 2011, 2012 Rob Caelers <robc@krandor.org>
// All rights reserved.

#include <string>
#include <iostream>

#include <boost/bind.hpp>

#include <glib.h>
#include <glib-object.h>

#include "DropboxAuth.hh"
#include "IHttpBackend.hh"
#include "HttpBackendSoup.hh"
#include "OAuth.hh"
#include "OAuthFilter.hh"

using namespace std;

static GMainLoop *loop = NULL;

void
on_pairing_success(const string &consumer_key, const string &consumer_secret,
                   const string &token_key, const string &token_secret)
{
}

void
on_pairing_failed()
{
}

int main(int argc, char **argv)
{
  (void) argc;
  (void) argv;

  g_type_init();

  loop = g_main_loop_new(NULL, TRUE);

  DropboxAuth *sso = new DropboxAuth();
  sso->init(string("xxx"), string("xxx"));
  // sso->init(boost::bind(on_pairing_success, _1, _2, _3, _4),
  //           boost::bind(on_pairing_failed));

  IHttpBackend::Ptr backend = sso->get_backend();

  HttpRequest::Ptr request = HttpRequest::create();
  request->uri = "https://api-content.dropbox.com/1/files_put/sandbox/test.txt?parent_rev=2081bd5a4";
  request->method = "POST";
  request->body = "Hello World 4";
  
  HttpReply::Ptr reply = backend->request(request);
  g_debug("files_put : %d %s", reply->status, reply->body.c_str());

  g_main_loop_run(loop);
  g_main_loop_unref(loop);
}
