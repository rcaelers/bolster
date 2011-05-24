#include <string>
#include <iostream>

#include "FunctionWrappers.hh"

using namespace std;

typedef int (* callback_t)(int, const char *, int, void *);

class Foo : public FunctionWrapper<int, int, const char *, int>
{
  int operator()(int a, const char *b, int c)
  {
    cout << a << " " << b << " " << c << " " << endl;
    return 73;
  }
};

class Bar
{
public:
  int foo(int a, const char *b, int c, string extra1, string extra2)
  {
    cout << a << " " << b << " " << c << " " << extra1 << " " << extra2 << endl;
    return 42;
   }
};
  
void perform_c_callback(callback_t cb, void *userdata)
{
  int ret = cb(1, "pietje", 2, userdata);
  cout << "ret = " << ret << endl;
}

int main(int argc, char **argv)
{
  (void) argc;
  (void) argv;
  
  Foo foo;
  Bar bar;

  typedef FunctionForwarder<decltype(&Bar::foo), std::string,std::string> BarfooWrapper;
  
  BarfooWrapper w(&bar, &Bar::foo, string("hello"), string("world"));
  
  perform_c_callback(Foo::Dispatch<4, callback_t>::dispatch, (void *)&foo);

  perform_c_callback(BarfooWrapper::Dispatch<4, callback_t>::dispatch, (void *)&w);
  
  return 0;
}
