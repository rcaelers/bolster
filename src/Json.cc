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
  : root_node(NULL)
{
  parse(json);
}


Json::Json()
{
  root_node = create_node(JSON_NODE_OBJECT);
}


Json::~Json()
{
}


void
Json::parse(const std::string &json)
{
  GError *error = NULL;

  JsonParser *parser = json_parser_new();
  json_parser_load_from_data(parser, json.c_str(), json.length(), &error);

  if (error == NULL)
    {
      root_node = json_parser_get_root(parser);
      // g_object_unref(G_OBJECT(parser)); FIXME: leak
    }
  else
    {
      string msg = error->message;
      g_error_free(error);
      // g_object_unref(G_OBJECT(parser)); FIXME: leak
      throw JsonException("Failed to parse Json: " + msg);
    }
}

std::string
Json::str() const
{
  string ret;
  
	if (root_node != NULL)
    {
      JsonGenerator *generator = json_generator_new();
      json_generator_set_root(generator, root_node);

      gsize size;
      char *str = json_generator_to_data(generator, &size);

      if (str != NULL)
        {
          ret = str;
        }
      
      g_object_unref(G_OBJECT(generator));
    }

  return ret;
}


JsonNode *
Json::get_node(const string &path, bool create) const
{
  vector<string> elements;
  StringUtil::split(path, '.', elements);

  JsonNode *node = root_node;
  for (size_t i = 0; i < elements.size(); ++i)
    {
      const char *name = elements[i].c_str();

      switch(json_node_get_node_type(node))
        {
        case JSON_NODE_OBJECT:
          {
            JsonObject *obj = json_node_get_object(node);
            g_assert(obj != NULL);

            if (json_object_has_member(obj, name))
              {
                node = json_object_get_member(obj, name);
                g_assert(node != NULL);
              }
            else if (create)
              {
                node = create_node(obj, name, (i == elements.size() - 1) ? JSON_NODE_VALUE : JSON_NODE_OBJECT);
                g_assert(node != NULL);
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
                int index = -1;

                JsonArray *array = json_node_get_array(node);
                g_assert(array != NULL);
                guint array_length = json_array_get_length(array);
                
                if (create && name[0] == '*' && name[1] == 0)
                  {
                    node = create_node(array, name, (i == elements.size() - 1) ? JSON_NODE_VALUE : JSON_NODE_OBJECT);
                    g_assert(node != NULL);
                  }
                else
                  {
                    index = boost::lexical_cast<int>(name);
                    if (index < 0 || (guint)index >= array_length)
                      {
                        throw JsonException(string("Path element ") + name + " is out-of-bounds array index");
                      }
                    else
                      {
                        node = json_array_get_element(array, index);
                        g_assert(node != NULL);
                      }
                  }
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


JsonNode *
Json::create_node(JsonNodeType type) const
{
  JsonNode *node = NULL;
  
  switch (type)
    {
    case JSON_NODE_OBJECT:
      {
        JsonObject *new_obj = json_object_new();
        node = json_node_new(JSON_NODE_OBJECT);
        json_node_take_object(node, new_obj);
      }
      break;

    case JSON_NODE_VALUE:
      {
        node = json_node_new(JSON_NODE_VALUE);
      }
      break;

    case JSON_NODE_ARRAY:
    case JSON_NODE_NULL:
      g_assert(false);
    }

  return node;
}

JsonNode *
Json::create_node(JsonObject *obj, const std::string &name, JsonNodeType type) const
{
  JsonNode *node = create_node(type);
  json_object_set_member(obj, name.c_str(), node);
  return node;
}


JsonNode *
Json::create_node(JsonArray *arr, const std::string &name, JsonNodeType type) const
{
  (void) name;
  
  JsonNode *node = create_node(type);
  json_array_add_element(arr, node);
  guint len = json_array_get_length(arr);
  g_assert(len > 0);
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
  try
    {
      JsonNode *node = get_node(path);
      if (node != NULL)
        {
          GValue value = {0};
          json_node_get_value(node, &value);

          if (G_VALUE_TYPE(&value) == G_TYPE_STRING)
            {
              const gchar *s = g_value_get_string(&value);
              if (s != NULL)
                {
                  ret = s;
                }
            }
          g_value_unset(&value);
        }
    }
  catch (JsonException e)
    {
    }

  return ret;
}


int
Json::get_int(const string &path) const
{
  int ret = 0;
  JsonNode *node = get_node(path);

  try
    {
      if (node != NULL)
        {
          GValue value = {0,};
          json_node_get_value(node, &value);

          if (G_VALUE_TYPE(&value) == G_TYPE_INT64)
            {
              ret = g_value_get_int64(&value);
            }
      
          g_value_unset(&value);
        }
    }
  catch (JsonException e)
    {
    }

  return ret;
}


bool
Json::get_bool(const string &path) const
{
  bool ret = false;

  try
    {
      JsonNode *node = get_node(path);
      if (node != NULL)
        {
          GValue value = {0,};
          json_node_get_value(node, &value);

          if (G_VALUE_TYPE(&value) == G_TYPE_BOOLEAN)
            {
              ret = g_value_get_boolean(&value);
            }
      
          g_value_unset(&value);
        }
    }
  catch (JsonException e)
    {
    }
  return ret;
}


void
Json::set_string(const string &path, const string &value)
{
  try
    {
      JsonNode *node = get_node(path, true);
      g_assert(node != NULL);

      json_node_set_string(node, value.c_str());
    }
  catch (JsonException e)
    {
    }
}


void
Json::set_int(const string &path, int value)
{
  try
    {
      JsonNode *node = get_node(path, true);
      g_assert(node != NULL);

      json_node_set_int(node, value);
    }
  catch (JsonException e)
    {
    }
}


void
Json::set_bool(const string &path, bool value)
{
  try
    {
      JsonNode *node = get_node(path, true);
      g_assert(node != NULL);

      json_node_set_boolean(node, value);
    }
  catch (JsonException e)
    {
    }
}
