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
#include <json-glib/json-glib.h>

class Json
{
public:
 	Json(const std::string &json);
  virtual ~Json();

  std::string get_string_value(const std::string &path);
  int get_int_value(const std::string &path);
  bool get_bool_value(const std::string &path);

private:
  JsonNode *get_node(const std::string &path);

private:
  JsonParser *parser;
  JsonNode *root_node;
};
  
#endif
