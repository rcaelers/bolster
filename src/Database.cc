// Copyright (C) 2010, 2011 by Rob Caelers <robc@krandor.nl>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

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
