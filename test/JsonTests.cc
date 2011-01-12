#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <algorithm>
#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>

#include "Json.hh"
#include "JsonException.hh"

BOOST_AUTO_TEST_SUITE(Json_Tests)

using namespace std;

static const char *json1 =
  "  {\n"
  "     \"firstName\": \"John\",\n"
  "     \"lastName\": \"Smith\",\n"
  "     \"age\": 25,\n"
  "     \"address\":\n"
  "     {\n"
  "         \"streetAddress\": \"21 2nd Street\",\n"
  "         \"city\": \"New York\",\n"
  "         \"state\": \"NY\",\n"
  "         \"postalCode\": \"10021\"\n"
  "     },\n"
  "     \"phoneNumber\":\n"
  "     [\n"
  "         {\n"
  "           \"type\": \"home\",\n"
  "           \"number\": \"212 555-1234\"\n"
  "         },\n"
  "         {\n"
  "           \"type\": \"fax\",\n"
  "           \"number\": \"646 555-4567\",\n"
  "           \"foo\": 13,\n"
  "           \"t\": true,\n"
  "           \"f\": false\n"
  "         }\n"
  "     ],\n"
  "     \"a\":\n"
  "     {\n"
  "         \"b\":\n"
  "         {\n"
  "             \"c\":\n"
  "             {\n"
  "                 \"d\": \"e\",\n"
  "                 \"e\": 12,\n"
  "                 \"t\": true,\n"
  "                 \"f\": false\n"
  "             }\n"
  "         }\n"
  "     }\n"
  " }\n";


BOOST_AUTO_TEST_CASE(json_path_get_type)
{
  try
    {
      g_type_init();

      Json j(json1);

      BOOST_CHECK_EQUAL(Json::String, j.get_type("firstName"));
      BOOST_CHECK_EQUAL(Json::Int, j.get_type("age"));
      BOOST_CHECK_EQUAL(Json::Bool, j.get_type("phoneNumber.1.t"));
      BOOST_CHECK_EQUAL(Json::Array, j.get_type("phoneNumber"));
      BOOST_CHECK_EQUAL(Json::Object, j.get_type("phoneNumber.0"));
    }
  catch(Exception &e)
    {
      BOOST_FAIL("Exception: " + e.details());
    }
}

BOOST_AUTO_TEST_CASE(json_path_get_array_size)
{
  try
    {
      g_type_init();

      Json j(json1);

      BOOST_CHECK_EQUAL(2, j.get_array_size("phoneNumber"));
      BOOST_CHECK_THROW(j.get_array_size("age"), JsonException);
    }
  catch(Exception &e)
    {
      BOOST_FAIL("Exception: " + e.details());
    }
}


BOOST_AUTO_TEST_CASE(json_path_get_members)
{
  try
    {
      g_type_init();

      Json j(json1);

      list<string> members;
      members.push_back("streetAddress");
      members.push_back("city");
      members.push_back("state");
      members.push_back("postalCode");
                     
      list<string> result;
      j.get_members("address", result);

      BOOST_CHECK_EQUAL(members.size(), result.size());
      BOOST_CHECK(equal(members.begin(), members.end(), result.begin()));

      BOOST_CHECK_THROW(j.get_members("age", result), JsonException);
    }
  catch(Exception &e)
    {
      BOOST_FAIL("Exception: " + e.details());
    }
}


BOOST_AUTO_TEST_CASE(json_path_exists)
{
  try
    {
      g_type_init();

      Json j(json1);

      BOOST_CHECK_EQUAL(true, j.exists("firstName"));
      BOOST_CHECK_EQUAL(true, j.exists("phoneNumber.0.type"));
      BOOST_CHECK_EQUAL(true, j.exists("a.b.c.d"));
      BOOST_CHECK_EQUAL(false, j.exists("middleName"));
      BOOST_CHECK_EQUAL(false, j.exists("phoneNumber.2.type"));
      BOOST_CHECK_EQUAL(false, j.exists("a.b.c.d.e.f"));
    }
  catch(Exception &e)
    {
      BOOST_FAIL("Exception: " + e.details());
    }
}

BOOST_AUTO_TEST_CASE(json_valid_strings)
{
  try
    {
      g_type_init();

      Json j(json1);

      BOOST_CHECK_EQUAL("John", j.get_string("firstName"));
      BOOST_CHECK_EQUAL("New York", j.get_string("address.city"));
      BOOST_CHECK_EQUAL("e", j.get_string("a.b.c.d"));
      BOOST_CHECK_EQUAL("home", j.get_string("phoneNumber.0.type"));
      BOOST_CHECK_EQUAL("fax", j.get_string("phoneNumber.1.type"));
      BOOST_CHECK_EQUAL("646 555-4567", j.get_string("phoneNumber.1.number"));
    }
  catch(Exception &e)
    {
      BOOST_FAIL("Exception: " + e.details());
    }
}

BOOST_AUTO_TEST_CASE(json_valid_ints)
{
  try
    {
      g_type_init();

      Json j(json1);

      BOOST_CHECK_EQUAL(25, j.get_int("age"));
      BOOST_CHECK_EQUAL(12, j.get_int("a.b.c.e"));
      BOOST_CHECK_EQUAL(13, j.get_int("phoneNumber.1.foo"));
    }
  catch(Exception &e)
    {
      BOOST_FAIL("Exception: " + e.details());
    }
}

BOOST_AUTO_TEST_CASE(json_valid_bools)
{
  try
    {
      g_type_init();

      Json j(json1);

      BOOST_CHECK_EQUAL(true, j.get_bool("a.b.c.t"));
      BOOST_CHECK_EQUAL(false, j.get_bool("a.b.c.f"));
      BOOST_CHECK_EQUAL(true, j.get_bool("phoneNumber.1.t"));
      BOOST_CHECK_EQUAL(false, j.get_bool("phoneNumber.1.f"));
    }
  catch(Exception &e)
    {
      BOOST_FAIL("Exception: " + e.details());
    }
}

BOOST_AUTO_TEST_CASE(json_invalid_path)
{
  try
    {
      g_type_init();

      Json j(json1);

      BOOST_CHECK_THROW(j.get_string("a.z"), JsonException);
      BOOST_CHECK_THROW(j.get_string("a.0"), JsonException);
      BOOST_CHECK_THROW(j.get_string("firstName.x"), JsonException);
      BOOST_CHECK_THROW(j.get_string("phoneNumber.2.type"), JsonException);
      BOOST_CHECK_THROW(j.get_string("phoneNumber.x.type"), JsonException);
      BOOST_CHECK_THROW(j.get_string("firstName.x"), JsonException);
    }
  catch(Exception &e)
    {
      BOOST_FAIL("Exception: " + e.details());
    }
}

BOOST_AUTO_TEST_CASE(json_invalid_type)
{
  try
    {
      g_type_init();

      Json j(json1);

      BOOST_CHECK_THROW(j.get_int("firstName"), JsonException);
      BOOST_CHECK_THROW(j.get_bool("address.city"), JsonException);
      BOOST_CHECK_THROW(j.get_string("age"), JsonException);
      BOOST_CHECK_THROW(j.get_bool("age"), JsonException);
      BOOST_CHECK_THROW(j.get_string("phoneNumber.1.t"), JsonException);
      BOOST_CHECK_THROW(j.get_int("phoneNumber.1.f"), JsonException);
    }
  catch(Exception &e)
    {
      BOOST_FAIL("Exception: " + e.details());
    }
}

// BOOST_CHECK_THROW

BOOST_AUTO_TEST_SUITE_END()
