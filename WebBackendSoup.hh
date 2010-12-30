#ifndef WEBBACKENDSOUP_HH
#define WEBBACKENDSOUP_HH

#include <string>
#include <map>

#ifdef HAVE_GNOME
#include <libsoup/soup-gnome.h>
#else
#include <libsoup/soup.h>
#endif
#include <json-glib/json-glib.h>

#include "IWebBackend.hh"

class OAuth;

class WebBackendSoup : public IWebBackend
{
public:
 	WebBackendSoup();
  virtual ~WebBackendSoup();
  
  virtual std::string request(std::string http_method, std::string uri, std::string body, std::string oauth_header);
  virtual void request_async(std::string http_method, std::string uri, std::string body, RequestCallback callback, std::string oauth_header);
  virtual void listen(ListenCallback callback, std::string path, int &port);
  
private:
  static void soup_callback(SoupSession *session, SoupMessage *message, gpointer user_data);
  static void print_headers(const char *name, const char *value, gpointer user_data);
  static void server_callback (SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query, SoupClientContext *context, gpointer data);

  void server_callback(SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query, SoupClientContext *context);

private:
  SoupSession *session;
  SoupURI *proxy;
  SoupServer *server;

  ListenCallback listen_cb;
};

#endif
