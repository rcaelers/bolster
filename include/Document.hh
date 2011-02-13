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

#ifndef DOCUMENT_HH
#define DOCUMENT_HH

#include <string>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <json-glib/json-glib.h>
#include "Registry.hh"

class ICouchDB;
class Json;

class Document
{
public:
  typedef boost::shared_ptr<Document> Ptr;

public:
  Document();
  virtual ~Document();

  void init(const std::string &database, Json *json);
  
  std::string str() const;

  std::string get_id() const;
  std::string get_revision() const;
  std::string get_type() const;

  void set_id(const std::string &id);
  void set_revision(const std::string &rev);
  void set_type(const std::string &type);
  
protected:
  std::string database;
  Json *json;
};

/*
class DocumentCreator
{
public:
  typedef Document * (*Type)(Json *json);

  template <class T>
  static Document *create(Json *json)
  {
    return new T(json);
  }
};

class DocumentRegistry : public bolster::Registry<Document, DocumentCreator, DocumentRegistry>
{
public:
  virtual Document *create(const std::string &key, Json *json)
    {
      DocumentCreator::Type creator = get_creator(key);
      return creator(json);
    }
};
  
#define REGISTER_DOCUMENT_TYPE(Type, Name) \
  template<>  \
  bool bolster::Global<Type>::global = DocumentRegistry::instance().add_type(std::string(Name), DocumentCreator::create<Type>);
*/

#define REGISTER_DOCUMENT_TYPE(Type, Name) REGISTER_TYPE(Document, Type, Name)

#endif
