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

#ifndef VARIANT_HH
#define VARIANT_HH

#include <string>
#include <iostream>

#include <vector>
#include <map>

#include <glib.h>
#include <glib-object.h>

using namespace std;

class VariantBase
{
public:
  VariantBase()
    : gobject(0)
  {
  }

  VariantBase(GVariant *gobject)
    : gobject(gobject)
  {
  }

  ~VariantBase()
  {
    if (gobject != NULL)
      {
        g_variant_unref(gobject);
      }
  }

  void init(const GVariant *obj)
  {
    if (gobject != NULL)
      {
        g_variant_unref(gobject);
      }

    gobject = const_cast<GVariant*>(obj);
    g_variant_ref_sink(gobject);
  }
  
  GVariant *gobj()
  {
    return gobject;
  }
  
  const GVariant *gobj() const
  {
    return gobject;
  }

  virtual const GVariantType *type() = 0;
  
protected:
  GVariant *gobject;
};

template<class T>
class Variant : public VariantBase
{
};


template<>
class Variant<std::string> : public VariantBase
{
public:
  Variant<std::string>();
  
  explicit Variant<std::string>(GVariant *gobject)
  : VariantBase(gobject)
  {
  }
  
  explicit Variant<std::string>(const std::string& data)
  {
    init(g_variant_new_string(data.c_str()));
  }
  
  const GVariantType *type()
  {
    return G_VARIANT_TYPE_STRING;
  };
  
  std::string get() const
  {
    const char *str = g_variant_get_string(gobject, 0);
    return (str != NULL ? std::string(str) : std::string());
  }
};


template<>
class Variant<bool> : public VariantBase
{
public:
  Variant<bool>();
  
  explicit Variant<bool>(GVariant *gobject)
  : VariantBase(gobject)
  {
  }

  explicit Variant<bool>(const bool &data)
  {
    init(g_variant_new_boolean(data));
  }
  
  const GVariantType *type()
  {
    return G_VARIANT_TYPE_BOOLEAN;
  };
  
  bool get() const
  {
    return g_variant_get_boolean(gobject);
  }
};


template<>
class Variant<gint32> : public VariantBase
{
public:
  Variant<gint32>();
  
  explicit Variant<gint32>(GVariant *gobject)
  : VariantBase(gobject)
  {
  }
  
  explicit Variant<gint32>(const gint32& data)
  {
    init(g_variant_new_int32(data));
  }
  
  const GVariantType *type()
  {
    return G_VARIANT_TYPE_INT32;
  };
  
  gint32 get() const
  {
    return g_variant_get_int32(gobject);
  }
};


template<class K, class V>
class Variant<std::map<K, V> > : public VariantBase
{
public:
  Variant<std::map<K, V> >();
    
  explicit Variant<std::map<K, V> >(GVariant *gobject)
    : VariantBase(gobject)
  {
  }

  explicit Variant<std::map<K, V> >(const std::map<K, V> &data)
  {
    GVariantBuilder *builder = g_variant_builder_new(type());
    for(typename std::map<K, V>::const_iterator iter = data.begin(); iter != data.end(); iter++)
      {
        Variant<K> key(iter->first);
        Variant<V> value(iter->second);

        GVariant *entry = g_variant_new_dict_entry(key.gobj(), value.gobj());
        g_variant_ref_sink(entry);
        g_variant_builder_add_value(builder, entry);
      }

    gchar *type_str = g_variant_type_dup_string(type());
    init(g_variant_new(type_str, builder));
    g_free(type_str);
  }
    
  const GVariantType *type()
  {
    static GVariantType *t = g_variant_type_new_array(g_variant_type_new_dict_entry(Variant<K>().type(), Variant<V>().type()));
    return t;
  }

  std::map<K, V> get() const
  {
    std::map<K, V> result;
    GVariantIter iter;
    
    g_variant_iter_init(&iter, const_cast<GVariant*>(gobj()));
    while (GVariant *child = g_variant_iter_next_value(&iter))
      {
        const Variant<K> key_variant(g_variant_get_child_value(child, 0));
        const Variant<V> value_variant(g_variant_get_child_value(child, 1));
        result[key_variant.get()] = value_variant.get();
      }

    return result;
  }
};


template<class V>
class Variant<std::vector<V> > : public VariantBase
{
public:
  Variant<std::vector<V> >();
    
  explicit Variant<std::vector<V> >(GVariant *gobject)
    : VariantBase(gobject)
  {
  }

  explicit Variant<std::vector<V> >(const std::vector<V> &data)
  {
    GVariantBuilder *builder = g_variant_builder_new(type());
    for(typename std::vector<V>::const_iterator iter = data.begin(); iter != data.end(); iter++)
      {
        Variant<V> value(*iter);
        g_variant_builder_add_value(builder, value.gobj());
      }

    gchar *type_str = g_variant_type_dup_string(type());
    init(g_variant_new(type_str, builder));
    g_free(type_str);
  }
    
  const GVariantType *type()
  {
    static GVariantType *t = g_variant_type_new_array(Variant<V>().type());
    return t;
  }

  std::vector<V> get() const
  {
    std::vector<V> result;
    GVariantIter iter;
    
    g_variant_iter_init(&iter, const_cast<GVariant*>(gobj()));
    while (GVariant *child = g_variant_iter_next_value(&iter))
      {
        Variant<V> value(child);
        result.push_back(value.get());
      }

    return result;
  }
};


class FunctionVariantBase : public VariantBase
{
public:
  virtual GVariant *dispatch(GVariant *variant) = 0;
};

template<typename T, typename F>
class FunctionVariant : public FunctionVariantBase
{
};

template<typename T, typename Ret, typename Arg1>
class FunctionVariant<T, Ret (T::*) (Arg1)>  : public FunctionVariantBase
{
public:
  typedef Ret (T::*FuncType)(Arg1);

  FunctionVariant<T, FuncType >();
    
  explicit FunctionVariant<T, Ret (T::*) (Arg1) >(T *object, FuncType func)
  : object(object), func(func)
  {
  }

  GVariant *dispatch(GVariant *variant)
  {
    Variant<Arg1> arg1(g_variant_get_child_value(variant, 0));
    Ret ret = (object->*func)(arg1.get());
    GVariant *v = NULL;

    Variant<Ret> var(ret);

    return v;
  }
  
  const GVariantType *type()
  {
    static const GVariantType *items[1] = { Variant<Arg1>().type() };
    static const GVariantType *t = g_variant_type_new_tuple(items, 1);
    return t;
  }

private:
  T *object;
  FuncType func;
};


template<typename T, typename Ret, typename Arg1>
static FunctionVariantBase * create(T *object, Ret (T::*func)(Arg1))
{
  typedef Ret (T::*FuncType)(Arg1);

  return new FunctionVariant<T, FuncType>(object, func);
}

// class FunctionVariant2 : FunctionVariantBase
// {
// public:

//   FunctionVariant2();
    
  
//   // Ret dispatch(GVariant *variant)
//   // {
//   //   Variant<Arg1> arg1(g_variant_get_child_value(variant, 0));
//   //   return (object->*func)(arg1.get());
//   // }
  
//   // static const GVariantType *type()
//   // {
//   //   static const GVariantType *items[1] = { Variant<Arg1>::type() };
//   //   static const GVariantType *t = g_variant_type_new_tuple(items, 1);
//   //   return t;
//   // }

// private:
//   T *object;
//   //FuncType func;
// };

#endif
