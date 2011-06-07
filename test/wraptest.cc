#include <string>
#include <iostream>

#include "FunctionWrappers.hh"

using namespace std;

typedef int (* callback_t)(int, const char *, int, void *, char);

class Bar
{
public:
  int foo(int a, const char *b, int c, char d, string extra1, string extra2)
  {
    cout << a << " " << b << " " << c << " " << d << " " << extra1 << " " << extra2 << endl;
    return 42;
   }
};
  
void perform_c_callback(callback_t cb, void *userdata)
{
  int ret = cb(1, "pietje", 2, userdata, 'x');
  cout << "ret = " << ret << endl;
}

int main(int argc, char **argv)
{
  (void) argc;
  (void) argv;
  
  Bar bar;

  typedef FunctionForwarder<Bar, callback_t, 4, std::string, std::string> BarFooWrapper;
  
  BarFooWrapper wrap(&bar, &Bar::foo, string("hello"), string("world"));
  
  perform_c_callback(BarFooWrapper::dispatch, (void *)&wrap);

  return 0;
}
