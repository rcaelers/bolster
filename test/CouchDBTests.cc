#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <algorithm>
#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>

#include "DesktopCouch.hh"
#include "Json.hh"
#include "JsonException.hh"

BOOST_AUTO_TEST_SUITE(CouchDB_Tests)

using namespace std;

static bool couch_is_ready = false;

static void ready()
{
  couch_is_ready = true;
}

static CouchDB *get_couch()
{
  g_type_init();
  couch_is_ready = false;
  
  DesktopCouch *c =  new DesktopCouch();;
  c->signal_ready.connect(ready);
  c->init();

  // TODO: use blocking iterate
  while (!couch_is_ready)
    {
      g_main_context_iteration(NULL, FALSE);
    }
  
  return c;
}

BOOST_AUTO_TEST_CASE(couch1)
{
  try
    {
      string in;
      string out;
      CouchDB *couch = get_couch();

      couch->request("DELETE", "/test1", "", out);
      couch->request("DELETE", "/test2", "", out);
      
      delete couch;
    }
  catch(Exception &e)
    {
      BOOST_FAIL("Exception: " + e.details());
    }
}

BOOST_AUTO_TEST_SUITE_END()
