#include <iostream>
#include <sstream>
#include <algorithm>
#include <string.h>
#include <list>
#include <stdint.h>

#include <glib.h>


#include "WebBackendSoup.hh"
#include "OAuth.hh"
#include "StringUtil.hh"

using namespace std;

WebBackendSoup::WebBackendSoup()
{
  session = NULL;
  proxy = NULL;
  server = NULL;
}


WebBackendSoup::~WebBackendSoup()
{
  if (session != NULL)
    {
      g_object_unref(session);
    }
  if (server != NULL)
    {
      g_object_unref(server);
    }
}


string
WebBackendSoup::request(string http_method, string uri, string body, string oauth_header)
{
  if (session != NULL)
    {
      g_debug("FIXME");
      return "";
    }
  
  session = soup_session_sync_new_with_options (
#ifdef HAVE_GNOME
			SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_GNOME_FEATURES_2_26,
#endif
			SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_CONTENT_DECODER,
			SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_COOKIE_JAR,
			SOUP_SESSION_USER_AGENT, "Workrave ",
			SOUP_SESSION_ACCEPT_LANGUAGE_AUTO, TRUE,
			NULL);
  
  if (proxy)
    {
      g_object_set(G_OBJECT(session), SOUP_SESSION_PROXY_URI, proxy, NULL);
    }

  SoupMessage *message = soup_message_new(http_method.c_str(), uri.c_str());

  if (body != "")
    {
      soup_message_set_request(message, "application/json", SOUP_MEMORY_COPY,
                               body.c_str(), body.size());
    }

  g_debug("Auth = %s", oauth_header.c_str());
	soup_message_set_flags(message, SOUP_MESSAGE_NO_REDIRECT);
  soup_message_headers_append(message->request_headers, "Authorization", oauth_header.c_str());
  soup_session_send_message(session, message);

  g_debug("Response len: %d", (int) message->response_body->length);

  string ret = "";

  if (message->response_body->length > 0)
    {
      ret = message->response_body->data;
    }

	g_object_unref(message);
  g_object_unref(session);
  session = NULL;
  
  return ret;
}


void
WebBackendSoup::request_async(string http_method, string uri, string body, RequestCallback callback, string oauth_header)
{
  if (session != NULL)
    {
      g_debug("FIXME");
      return;
    }

  (void) callback;
  
  session = soup_session_async_new_with_options(
#ifdef HAVE_GNOME
			SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_GNOME_FEATURES_2_26,
#endif
			SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_CONTENT_DECODER,
			SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_COOKIE_JAR,
			SOUP_SESSION_USER_AGENT, "Workrave ",
			SOUP_SESSION_ACCEPT_LANGUAGE_AUTO, TRUE,
			NULL);
  
  if (proxy)
    {
      g_object_set(G_OBJECT(session), SOUP_SESSION_PROXY_URI, proxy, NULL);
    }

  SoupMessage *message = soup_message_new(http_method.c_str(), uri.c_str());


  if (body != "")
    {
      soup_message_set_request(message, "application/json", SOUP_MEMORY_COPY,
                               body.c_str(), body.size());
    }

  g_debug("Auth = %s", oauth_header.c_str());
	soup_message_set_flags(message, SOUP_MESSAGE_NO_REDIRECT);
  soup_message_headers_append(message->request_headers, "Authorization", oauth_header.c_str());
  //soup_session_queue_message(session, message, soup_callback, this);
}


void
WebBackendSoup::listen(ListenCallback callback, string path, int &port)
{
  if (server != NULL)
    {
      g_object_unref(server);
      server = NULL;
    }
  
  (void) callback;
  
  SoupAddress *addr = soup_address_new("127.0.0.1", SOUP_ADDRESS_ANY_PORT);
  soup_address_resolve_sync(addr, NULL);

  server = soup_server_new(SOUP_SERVER_SERVER_HEADER, "OAuth",
                           SOUP_SERVER_INTERFACE, addr,
                           NULL);
	g_object_unref (addr);

	if (server == NULL)
    {
      g_debug("FIXME: Cannot listen");
    }

  port = soup_server_get_port(server);
  g_debug("Listening on %d", port);

  soup_server_add_handler(server, path.c_str(), server_callback, this, NULL);
	soup_server_run_async(server);
}

void
WebBackendSoup::print_headers(const char *name, const char *value, gpointer user_data)
{
  (void) user_data;
  g_debug("%s: %s", name, value);
}



void
WebBackendSoup::server_callback(SoupServer *server, SoupMessage *message, const char *path, GHashTable *query, SoupClientContext *context, gpointer data)
{
  WebBackendSoup *backend = (WebBackendSoup *)data;
  backend->server_callback(server, message, path, query, context);
}

void
WebBackendSoup::server_callback(SoupServer *, SoupMessage *message, const char *path, GHashTable *query, SoupClientContext *context)
{
  (void) path;
  (void) context;

  g_debug("server_callback %s", path);
  
  if (message->method != SOUP_METHOD_GET)
    {
      soup_message_set_status(message, SOUP_STATUS_NOT_IMPLEMENTED);
      return;
    }

  char *token = NULL;
  char *verifier = NULL;

	if (query != NULL)
    {
      token = (char *)g_hash_table_lookup(query, "oauth_token");
      verifier = (char *)g_hash_table_lookup(query, "oauth_verifier");
    }

  g_debug("%s %s", token, verifier);
  
	soup_message_set_response(message, "text/plain",
				   SOUP_MEMORY_STATIC,
				   "OK\r\n", 4);
	soup_message_set_status (message, SOUP_STATUS_OK);

  g_object_unref(server);
}


// void
// WebBackendSoup::soup_callback(SoupSession *session, SoupMessage *message, gpointer user_data)
// {
//   WebBackendSoup *oath = (WebBackendSoup *) user_data;
//   oath->finished(session, message);
// }
