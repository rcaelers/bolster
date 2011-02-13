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

#include <sstream>

#include "Document.hh"

#include "ICouchDB.hh"
#include "Json.hh"
#include "JsonException.hh"

using namespace std;

Document::Document()
{
  json = new Json();
}


void
Document::init(const std::string &database, Json *json)
{
  delete this->json;

  this->database = database;
  this->json = json;
}


Document::~Document()
{
  delete json;
}


std::string
Document::str() const
{
  return json->str();
}


std::string
Document::get_id() const
{
  return json->get_string("_id");
} 


std::string
Document::get_revision() const
{
  return json->get_string("_rev");
}


std::string
Document::get_type() const
{
  return json->get_string("record_type");
} 


void
Document::set_id(const std::string &id)
{
  json->set_string("_id", id);
} 


void
Document::set_revision(const std::string &rev)
{
  json->set_string("_rev", rev);
}


void
Document::set_type(const std::string &type)
{
  json->set_string("record_type", type);
} 
