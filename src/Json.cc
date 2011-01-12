//
// Copyright (C) 2010, 2011 Rob Caelers <robc@krandor.nl>
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Json.hh"

#include <string>
#include <vector>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include "JsonException.hh"
#include "StringUtil.hh"



using namespace std;

Json::Json(const std::string &json)
{
  try
    {
      parser = json_parser_new();

      GError *error = NULL;
      json_parser_load_from_data(parser, json.c_str(), json.length(), &error);

      if (error != NULL)
        {
          string msg = error->message;
          g_error_free(error);
          throw JsonException("Failed to parse JSON: " + msg);
        }
      
      root_node = json_parser_get_root(parser);
    }
  catch(...)
    {
      g_object_unref(G_OBJECT(parser));
      parser = NULL;
      throw;
    }
}


Json::~Json()
{
  if (parser != NULL)
    {
      g_object_unref(G_OBJECT(parser));
    }
}


JsonNode *
Json::get_node(const string &path) const
{
  vector<string> elements;
  StringUtil::split(path, '.', elements);

  JsonNode *node = root_node;
  for (size_t i = 0; i < elements.size(); ++i)
    {
      const char *name = elements[i].c_str();

      if (node == NULL)
        {
          throw JsonException(string("Path element ") + name + " does not exist");
        }
      
      switch(json_node_get_node_type(node))
        {
        case JSON_NODE_OBJECT:
          {
            JsonObject *obj = json_node_get_object(node);
            if (obj != NULL && json_object_has_member(obj, name))
              {
                node = json_object_get_member(obj, name);
              }
            else
              {
                throw JsonException(string("Path element ") + name + " not found");
              }
          }
          break;
          
        case JSON_NODE_ARRAY:
          {
            try
              {
                int index = boost::lexical_cast<int>(name);

                JsonArray *array = json_node_get_array(node);
                if (array == NULL)
                  {
                    throw JsonException("Invalid Json");
                  }

                guint array_length = json_array_get_length(array);
                if (index < 0 || (guint)index >= array_length)
                  {
                    throw JsonException(string("Path element ") + name + " is out-of-bounds array index");
                  }
                
                node = json_array_get_element(array, index);
              }
            catch(boost::bad_lexical_cast &)
              {
                throw JsonException(string("Path element ") + name + " is invalid array index");
              }
          }
          break;
          
        case JSON_NODE_VALUE:
          {
            if (i != elements.size())
              {
                throw JsonException("More path elements after value");
              }
          }
          break;
      
        case JSON_NODE_NULL:
          break;
        }
    }

  g_assert(node != NULL);
  
  return node;
}


bool
Json::exists(const string &path) const
{
  try
    {
      return get_node(path) != NULL;
    }
  catch(...)
    {
    }
  return false;
}


Json::Type
Json::get_type(const string &path) const
{
  Type ret = Json::None;
  
  JsonNode *node = get_node(path);
  
  switch(json_node_get_node_type(node))
    {
    case JSON_NODE_OBJECT:
      ret = Json::Object;
      break;
          
    case JSON_NODE_ARRAY:
      ret = Json::Array;
      break;
          
    case JSON_NODE_VALUE:
      {
        GValue value = {0,};
        json_node_get_value(node, &value);
          
        switch (G_VALUE_TYPE(&value))
          {
          case G_TYPE_BOOLEAN:
            ret = Json::Bool;
            break;
          case G_TYPE_STRING:
            ret = Json::String;
            break;
          case G_TYPE_INT64:
            ret = Json::Int;
            break;

          default:
            break;
          }
      }
      break;
      
    default:
    case JSON_NODE_NULL:
      ret = Json::None;
      break;
    }

  return ret;
}


int
Json::get_array_size(const std::string &path) const
{
  int ret = 0;
  JsonNode *node = get_node(path);
  
  if (json_node_get_node_type(node) == JSON_NODE_ARRAY)
    {
      JsonArray *array = json_node_get_array(node);
      if (array == NULL)
        {
          throw JsonException("Invalid Json");
        }

      ret =  json_array_get_length(array);
    }
  else
    {
      throw JsonException("Path " + path + " is not an array");
    }

  return ret;
}


void
Json::get_members(const std::string &path, std::list<std::string> &result) const
{
  JsonNode *node = get_node(path);
  
  if (json_node_get_node_type(node) == JSON_NODE_OBJECT)
    {
      JsonObject *obj = json_node_get_object(node);
      if (obj == NULL)
        {
          throw JsonException("Invalid Json");
        }
      
      GList *members = json_object_get_members(obj);
      while (members)
        {
          result.push_back(string((char *)members->data));
          members = members->next;
        }
    }
  else
    {
      throw JsonException("Path " + path + " is not an object");
    }
}


string
Json::get_string(const string &path) const
{
  string ret;
  JsonNode *node = get_node(path);

  if (node == NULL)
    {
      throw JsonException("Path " +  path + " does not exist");
    }

  GValue value = {0};
  json_node_get_value(node, &value);

  if (G_VALUE_TYPE(&value) == G_TYPE_STRING)
    {
      ret = g_value_get_string(&value);
    }
  else
    {
      throw JsonException("Incorrect type for " +  path);
    }

  g_value_unset(&value);

  return ret;
}


int
Json::get_int(const string &path) const
{
  int ret;
  JsonNode *node = get_node(path);

  if (node == NULL)
    {
      throw JsonException("Path " +  path + " does not exist");
    }

  GValue value = {0,};
  json_node_get_value(node, &value);

  if (G_VALUE_TYPE(&value) == G_TYPE_INT64)
    {
      ret = g_value_get_int64(&value);
    }
  else
    {
      throw JsonException("Incorrect type for " +  path);
    }

  g_value_unset(&value);

  return ret;
}


bool
Json::get_bool(const string &path) const
{
  bool ret;
  JsonNode *node = get_node(path);

  if (node == NULL)
    {
      throw JsonException("Path " +  path + " does not exist");
    }

  GValue value = {0,};
  json_node_get_value(node, &value);

  if (G_VALUE_TYPE(&value) == G_TYPE_BOOLEAN)
    {
      ret = g_value_get_boolean(&value);
    }
  else
    {
      throw JsonException("Incorrect type for " +  path);
    }

  g_value_unset(&value);

  return ret;
}

