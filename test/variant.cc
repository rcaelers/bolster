// main.cc --- OAuth test app
//
// Copyright (C) 2010, 2011 Rob Caelers <robc@krandor.org>
// All rights reserved.

#include <string>
#include <iostream>

#include <vector>
#include <map>

#include "Variant.hh"

class Foo
{
public:
  int foo(std::string s)
  {
    return s.length();
  }
};

int main(int argc, char **argv)
{
  (void) argc;
  (void) argv;

  g_type_init();

  typedef std::map<std::string, bool> map_type;
  typedef std::vector<std::string> vec_type;
  
  map_type m;
  m["a"] = true;
  m["b"] = false;

  vec_type v;
  v.push_back("27");
  v.push_back("2");
  v.push_back("73");
  
  Variant<map_type > var_m(m);
  Variant<vec_type > var_v(v);

  //cout << (char *)var_m.type() << endl;

  map_type r = var_m.get();
  for (map_type::iterator i = r.begin(); i != r.end(); i++)
    {
      cout << i->first << " " << i->second << std::endl;
    }

  vec_type s = var_v.get();
  for (vec_type::iterator i = s.begin(); i != s.end(); i++)
    {
      cout << *i << std::endl;
    }

  Foo foo;
  
  FunctionVariant<Foo, int (Foo::*)(std::string) > f(&foo, &Foo::foo);

  FunctionVariantBase *f2 = create(&foo, &Foo::foo);
  
  GVariant *arg1 = g_variant_new_string("foobar");
  GVariant *args[1] = { arg1 };
  
  GVariant *params = g_variant_new_tuple(args, 1);
  gchar *type_str = g_variant_type_dup_string(f2->type());
  cout << type_str << endl;
  g_free(type_str);
  cout << f2->dispatch(params) << endl;
}
