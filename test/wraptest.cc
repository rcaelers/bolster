#include <string>
#include <iostream>

#include "Wrap.hh"

using namespace std;

typedef void (* callback_t)(int, const char *, int, void *);

class Foo : public CWrapper<int, const char *, int>
{
  void operator()(int a, const char *b, int c)
  {
    cout << a << " " << b << " " << c << " " << endl;
  }
};

void perform_c_callback(callback_t x, void *userdata)
{
  x(1, "pietje", 2, userdata);
}

int main(int argc, char **argv)
{
  (void) argc;
  (void) argv;
  
  Foo *foo = new Foo();
  perform_c_callback(Foo::Dispatch<4, callback_t>::dispatch, (void *)foo);

  return 0;
}
