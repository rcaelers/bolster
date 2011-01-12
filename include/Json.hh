//
// Copyright (C) 2011 Rob Caelers <robc@krandor.nl>
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

#ifndef JSON_HH
#define JSON_HH

#include <string>
#include <list>
#include <json-glib/json-glib.h>

class Json
{
public:
  enum Type { Object, Array, String, Int, Bool, None };
  
public:
 	Json(const std::string &json);
  virtual ~Json();

  
  bool exists(const std::string &path) const;
  
  Type get_type(const std::string &path) const;
  int get_array_size(const std::string &path) const;
  void get_members(const std::string &path, std::list<std::string> &result) const;
  
  std::string get_string(const std::string &path) const;
  int get_int(const std::string &path) const;
  bool get_bool(const std::string &path) const;

private:
  JsonNode *get_node(const std::string &path) const;

private:
  JsonParser *parser;
  JsonNode *root_node;
};
  
#endif
