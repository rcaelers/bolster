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

#ifndef DATABASE_HH
#define DATABASE_HH

#include <string>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>


class ICouchDB;
class Document;

class Database
{
public:
  Database(ICouchDB *couch, const std::string &database_name);
  virtual ~Database();

  void create();
  void destroy();
  
  void put(Document *doc);
  void remove(Document *doc);
  Document *get(const std::string id);

  std::string get_database_name() const;
  
private:
  std::string database_name;
  ICouchDB *couch;
};

  
#endif
