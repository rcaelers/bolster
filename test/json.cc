#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>

#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>

#include "Json.hh"
#include "JsonException.hh"

BOOST_AUTO_TEST_SUITE(Json_Tests)

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

BOOST_AUTO_TEST_CASE(json_valid_string_values)
{
  try
    {
      g_type_init();

      Json j(json1);

      BOOST_CHECK_EQUAL("John", j.get_string_value("firstName"));
      BOOST_CHECK_EQUAL("New York", j.get_string_value("address.city"));
      BOOST_CHECK_EQUAL("e", j.get_string_value("a.b.c.d"));
      BOOST_CHECK_EQUAL("home", j.get_string_value("phoneNumber.0.type"));
      BOOST_CHECK_EQUAL("fax", j.get_string_value("phoneNumber.1.type"));
      BOOST_CHECK_EQUAL("646 555-4567", j.get_string_value("phoneNumber.1.number"));
    }
  catch(Exception &e)
    {
      BOOST_FAIL("Exception: " + e.details());
    }
}

BOOST_AUTO_TEST_CASE(json_valid_int_values)
{
  try
    {
      g_type_init();

      Json j(json1);

      BOOST_CHECK_EQUAL(25, j.get_int_value("age"));
      BOOST_CHECK_EQUAL(12, j.get_int_value("a.b.c.e"));
      BOOST_CHECK_EQUAL(13, j.get_int_value("phoneNumber.1.foo"));
    }
  catch(Exception &e)
    {
      BOOST_FAIL("Exception: " + e.details());
    }
}

BOOST_AUTO_TEST_CASE(json_valid_bool_values)
{
  try
    {
      g_type_init();

      Json j(json1);

      BOOST_CHECK_EQUAL(true, j.get_bool_value("a.b.c.t"));
      BOOST_CHECK_EQUAL(false, j.get_bool_value("a.b.c.f"));
      BOOST_CHECK_EQUAL(true, j.get_bool_value("phoneNumber.1.t"));
      BOOST_CHECK_EQUAL(false, j.get_bool_value("phoneNumber.1.f"));
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

      BOOST_CHECK_THROW(j.get_string_value("a.z"), JsonException);
      BOOST_CHECK_THROW(j.get_string_value("a.0"), JsonException);
      BOOST_CHECK_THROW(j.get_string_value("firstName.x"), JsonException);
      BOOST_CHECK_THROW(j.get_string_value("phoneNumber.2.type"), JsonException);
      BOOST_CHECK_THROW(j.get_string_value("phoneNumber.x.type"), JsonException);
      BOOST_CHECK_THROW(j.get_string_value("firstName.x"), JsonException);
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

      BOOST_CHECK_THROW(j.get_int_value("firstName"), JsonException);
      BOOST_CHECK_THROW(j.get_bool_value("address.city"), JsonException);
      BOOST_CHECK_THROW(j.get_string_value("age"), JsonException);
      BOOST_CHECK_THROW(j.get_bool_value("age"), JsonException);
      BOOST_CHECK_THROW(j.get_string_value("phoneNumber.1.t"), JsonException);
      BOOST_CHECK_THROW(j.get_int_value("phoneNumber.1.f"), JsonException);
    }
  catch(Exception &e)
    {
      BOOST_FAIL("Exception: " + e.details());
    }
}

// BOOST_CHECK_THROW

BOOST_AUTO_TEST_SUITE_END()
