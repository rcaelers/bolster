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
#include "TestUtils.hh"

BOOST_AUTO_TEST_SUITE(Datebase_Tests)

using namespace std;

static bool couch_is_ready = false;

BOOST_AUTO_TEST_CASE(database1)
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
