// Copyright (C) 2011 by Rob Caelers <robc@krandor.nl>
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

#ifndef DOCUMENT_HH
#define DOCUMENT_HH

#include <string>
#include <boost/shared_ptr.hpp>

#include "json/json.h"

#include "Registry.hh"

class ICouchDB;

class Document
{
public:
  typedef boost::shared_ptr<Document> Ptr;

public:
  Document();
  virtual ~Document();

  void init(const std::string &database, Json::Value &root);
  
  std::string str() const;
  std::string get_id() const;
  std::string get_revision() const;
  std::string get_type() const;

  void set_id(const std::string &id);
  void set_revision(const std::string &rev);
  void set_type(const std::string &type);
  
protected:
  std::string database;
  Json::Value root;
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
