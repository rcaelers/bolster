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

#ifndef COUCHDB_HH
#define COUCHDB_HH

#include <string>
#include <boost/signals2.hpp>

#include "ICouchDB.hh"

class OAuth;
class IWebBackend;

class CouchDB : public ICouchDB
{
  
public:
 	CouchDB();
  virtual ~CouchDB();
  
  virtual void init();

  int request(const std::string &http_method,
              const std::string &uri,
              const std::string &body,
              std::string &response_body);

  bool is_ready() const;
  std::string get_couch_uri() const;
  
private:
  void init_oauth();
  
protected:
  void complete();
  void failure();
  
protected:
  std::string couch_uri;
  OAuth *oauth;
  IWebBackend *backend;

private:
  bool ready;
};
  
#endif