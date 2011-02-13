// Registry.hh --- Registry
//
// Copyright (C) 2007, 2011 Rob Caelers <robc@krandor.nl>
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
// Based on ideas from libs11n and boost
//

#ifndef REGISTRY_HH
#define REGISTRY_HH

#include <string>
#include <map>

namespace bolster {

  template <class T>
  class Creator
  {
  public:
    typedef T * (*Type)();

    template<class SubT>
    static T *create()
    {
      return new SubT;
    }
  };

  template <class Type, class CreatorType = bolster::Creator<Type> >
  class Registry
  {
  public:
    Registry() {}
    virtual ~Registry() {}

    typedef std::map<std::string, typename CreatorType::Type> Creators;

    virtual Type *create(const std::string &key)
    {
      typename CreatorType::Type creator = get_creator(key);
      return creator();
    }
    
    virtual bool add_type(const std::string &key, typename CreatorType::Type fc)
    {
      map.insert(typename Creators::value_type(key, fc));
      return true;
    }

    static Registry<Type, CreatorType>  &instance()
    {
      static Registry<Type, CreatorType> *me = NULL;

      if (me == NULL)
        {
          me = new Registry<Type, CreatorType>;
        }

      return *me;
    }

  protected:
    virtual typename CreatorType::Type get_creator(const std::string &key)
    {
      typename Creators::const_iterator it = map.find(key);
      if (it != map.end())
        {
          return it->second;
        }
      return NULL;
    }
    
  private:
    Creators map;
  };

  template <typename Type>
  struct Global
  {
    Global()
    {
      global = false;
    }

    static bool global;
  };

}

#define REGISTER_TYPE(BaseType, Type, Name) \
template<> \
 bool bolster::Global<Type>::global = bolster::Registry<BaseType>::instance().add_type(std::string(Name), bolster::Creator<BaseType>::create<Type>);

#endif // REGISTRY_HH
