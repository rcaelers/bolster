#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "DesktopCouch.hh"

using namespace std;

static bool couch_is_ready = false;

static void ready()
{
  couch_is_ready = true;
}

CouchDB *get_couch()
{
  g_type_init();
  couch_is_ready = false;
  
  DesktopCouch *c =  new DesktopCouch();;
  c->signal_ready.connect(ready);
  c->init();

  // TODO: use blocking iterate
  while (!couch_is_ready)
    {
      g_main_context_iteration(NULL, FALSE);
    }
  
  return c;
}
