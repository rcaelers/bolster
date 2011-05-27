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

#include "Document.hh"

using namespace std;

Document::Document()
{
}


void
Document::init(const std::string &database, Json::Value &root)
{
  this->database = database;
  this->root = root;
}


Document::~Document()
{
}


std::string
Document::str() const
{
  Json::StyledWriter writer;
  return writer.write(root);
}


std::string
Document::get_id() const
{
    return root["_id"].asString();
} 


std::string
Document::get_revision() const
{
  return root["_rev"].asString();
}


std::string
Document::get_type() const
{
  return root["record_type"].asString();
} 


void
Document::set_id(const std::string &id)
{
  root["_id"] = id;
} 


void
Document::set_revision(const std::string &rev)
{
  root["_rev"] = rev;
}


void
Document::set_type(const std::string &type)
{
  root["record_type"] = type;
} 
