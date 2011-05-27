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

#include "CouchDB.hh"

#include <boost/bind.hpp>

#include "OAuth.hh"
#include "WebBackendSoup.hh"
#include "Uri.hh"

using namespace std;

CouchDB::CouchDB()
  : oauth(NULL),
    backend(NULL),
    ready(false)
{
}


CouchDB::~CouchDB()
{
  signal_ready.disconnect_all_slots();
  signal_failure.disconnect_all_slots();

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
  oauth = new OAuth();

  backend->add_filter(oauth);
}


void
CouchDB::complete()
{
  ready = true;
  signal_ready();
}


void
CouchDB::failure()
{
  ready = false;
  couch_uri = "";
  signal_failure();
}


bool
CouchDB::is_ready() const
{
  return ready;
}


std::string
CouchDB::get_couch_uri() const
{
  return couch_uri;
}


int
CouchDB::request(const std::string &http_method,
                 const std::string &uri,
                 const std::string &body,
                 std::string &response_body)
{
  return backend->request(http_method, couch_uri + uri, body, response_body);
}
