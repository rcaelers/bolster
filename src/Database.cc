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

#include "Database.hh"

#include <boost/format.hpp>
#include <glib.h>

#include "json/json.h"

#include "ICouchDB.hh"
#include "Document.hh"
#include "Uri.hh"
#include "Registry.hh"

using namespace std;

Database::Database(ICouchDB *couch, const std::string &database_name)
  : database_name(database_name),
    couch(couch)
{
}


Database::~Database()
{
}


void
Database::create()
{
  string out;
  couch->request("PUT",
                 boost::str(boost::format("/%1%") % Uri::escape(database_name)),
                 "", out);
}


void
Database::destroy()
{
  string out;
  couch->request("DELETE",
                 boost::str(boost::format("/%1%") % Uri::escape(database_name)),
                 "", out);
}


void
Database::put(Document *doc)
{
  string id = doc->get_id();
  string out;

  if (id != "")
    {
      couch->request("PUT",
                     boost::str(boost::format("/%1%/%2%") % Uri::escape(database_name) % Uri::escape(id)),
                     doc->str(), out);
    }
  else
    {
      couch->request("POST",
                     boost::str(boost::format("/%1%/") % Uri::escape(database_name)),
                     doc->str(), out);

    }

  Json::Value root;
  Json::Reader reader;
  bool ok = reader.parse(out, root);
  
  if (ok && root.isMember("ok"))
    {
      doc->set_id(root["id"].asString());
      doc->set_revision(root["rev"].asString());
    }
  else
    {
      g_debug("Error: %s (%s)", root["error"].asString().c_str(), root["reason"].asString().c_str());
    }
}


void
Database::remove(Document *doc)
{
  string id = doc->get_id();
  string rev = doc->get_revision();
  string out;
  
  couch->request("DELETE",
                 boost::str(boost::format("/%1%/%2%?rev=%3%")
                            % Uri::escape(database_name)
                            % Uri::escape(id)
                            % Uri::escape(rev)), "", out);
}


Document *
Database::get(const std::string &id)
{
  string out;

  couch->request("GET",
                 boost::str(boost::format("%1%/%2%") % Uri::escape(database_name) % Uri::escape(id)),
                 "", out);

  Document *doc = NULL;
  Json::Value root;
  Json::Reader reader;
  bool ok = reader.parse(out, root);

  if (ok && !root.isMember("error") && root.isMember("_id"))
    {
      string type = root["record_type"].asString();
      doc = bolster::Registry<Document>::instance().create(type);
      doc->init(database_name,  root);
    }

  return doc;
}


std::string
Database::get_database_name() const
{
  return database_name;
}
