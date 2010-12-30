#include "WebBackendSoup.hh"

#include <glib.h>

#include "WebBackendException.hh"
#include "OAuth.hh"
#include "StringUtil.hh"

using namespace std;

WebBackendSoup::WebBackendSoup()
  : sync_session(NULL),
    async_session(NULL),
    proxy(NULL),
    server(NULL)
{
}


WebBackendSoup::~WebBackendSoup()
{
  if (sync_session != NULL)
    {
      g_object_unref(sync_session);
    }

  if (async_session != NULL)
    {
      g_object_unref(async_session);
    }

  if (server != NULL)
    {
      g_object_unref(server);
    }
}


int
WebBackendSoup::request(const string &http_method, const string &uri, const string &body, const string &oauth_header,
                        string &response_body)
{
  if (sync_session == NULL)
    {
      init_sync();
    }

  SoupMessage *message = create_soup_message(http_method, uri, body, oauth_header);
  soup_session_send_message(sync_session, message);

  response_body = (message->response_body->length > 0) ? message->response_body->data : "";
  int ret = message->status_code;
  
	g_object_unref(message);
  
  return ret;
}


void
WebBackendSoup::request(const string &http_method, const string &uri, const string &body, const string &oauth_header, WebReplyCallback callback)
{
  AsyncRequestData *data = new AsyncRequestData(this, callback);

  if (async_session == NULL)
    {
      init_async();
    }

  SoupMessage *message = create_soup_message(http_method, uri, body, oauth_header);
  soup_session_queue_message(async_session, message, AsyncRequestData::cb, data);
}


void
WebBackendSoup::listen(WebRequestCallback callback, const string &path, int &port)
{
  AsyncServerData *data = new AsyncServerData(this, callback);

  if (server != NULL)
    {
      g_object_unref(server);
      server = NULL;
    }

  SoupAddress *addr = soup_address_new("127.0.0.1", SOUP_ADDRESS_ANY_PORT);
  soup_address_resolve_sync(addr, NULL);

  server = soup_server_new(SOUP_SERVER_SERVER_HEADER, "OAuth",
                           SOUP_SERVER_INTERFACE, addr,
                           NULL);
	g_object_unref (addr);

	if (server == NULL)
    {
      throw WebBackendException("Cannot receive incoming connections.");
    }

  port = soup_server_get_port(server);
  g_debug("Listening on %d", port);

  soup_server_add_handler(server, path.c_str(), AsyncServerData::cb, data, NULL);
	soup_server_run_async(server);
}

void
WebBackendSoup::init_async()
{
  async_session = soup_session_async_new_with_options(
#ifdef HAVE_GNOME
			SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_GNOME_FEATURES_2_26,
#endif
			SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_CONTENT_DECODER,
			SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_COOKIE_JAR,
			SOUP_SESSION_USER_AGENT, "Workrave",
			SOUP_SESSION_ACCEPT_LANGUAGE_AUTO, TRUE,
			NULL);
  
  if (async_session == NULL)
    {
      throw WebBackendException("Cannot create session.");
    }
  
  if (proxy)
    {
      g_object_set(G_OBJECT(async_session), SOUP_SESSION_PROXY_URI, proxy, NULL);
    }
}


void
WebBackendSoup::init_sync()
{
  sync_session = soup_session_sync_new_with_options (
#ifdef HAVE_GNOME
			SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_GNOME_FEATURES_2_26,
#endif
			SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_CONTENT_DECODER,
			SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_COOKIE_JAR,
			SOUP_SESSION_USER_AGENT, "Workrave",
			SOUP_SESSION_ACCEPT_LANGUAGE_AUTO, TRUE,
			NULL);

  if (sync_session == NULL)
    {
      throw WebBackendException("Cannot create session.");
    }
  
  if (proxy)
    {
      g_object_set(G_OBJECT(sync_session), SOUP_SESSION_PROXY_URI, proxy, NULL);
    }
}


SoupMessage *
WebBackendSoup::create_soup_message(const string &http_method,
                                    const string &uri,
                                    const string &body,
                                    const string &oauth_header)
{
  SoupMessage *message = soup_message_new(http_method.c_str(), uri.c_str());
  if (message == NULL)
    {
      throw WebBackendException("Cannot create HTTP request.");
    }

  if (body != "")
    {
      soup_message_set_request(message, "application/json", SOUP_MEMORY_COPY,
                               body.c_str(), body.size());
    }

  if (oauth_header != "")
    {
      soup_message_headers_append(message->request_headers, "Authorization", oauth_header.c_str());
    }
  
	soup_message_set_flags(message, SOUP_MESSAGE_NO_REDIRECT);

  return message;
}
  


void
WebBackendSoup::AsyncServerData::cb(SoupServer *server, SoupMessage *message, const char *path,
                                    GHashTable *query, SoupClientContext *context, gpointer data)
{
  AsyncServerData *d = (AsyncServerData *)data;
  d->backend->server_callback(server, message, path, query, context, d);
  delete d;
}

void
WebBackendSoup::server_callback(SoupServer *, SoupMessage *message, const char *path,
                                GHashTable *query, SoupClientContext *context,
                                AsyncServerData *data)
{
  
  (void) path;
  (void) context;
  (void) query;

  SoupURI *uri = soup_message_get_uri(message);
  string response_query = (uri != NULL && uri->query != NULL) ? uri->query : "";
  string response_body = (message->response_body->length > 0) ? message->response_body->data : "";

  string content_type;
  string reply;

  data->callback(message->method, response_query, response_body, content_type, reply);

  g_debug("reply %s %s", reply.c_str(), content_type.c_str());
  soup_message_set_status(message, SOUP_STATUS_OK);
  soup_message_set_response(message, content_type.c_str(), SOUP_MEMORY_COPY, reply.c_str(), reply.length());
}


void
WebBackendSoup::AsyncRequestData::cb(SoupSession *session, SoupMessage *message, gpointer user_data)
{
  AsyncRequestData *d = (AsyncRequestData *)user_data;
  d->backend->client_callback(session, message, d);
  delete d;
}

void
WebBackendSoup::client_callback(SoupSession *session, SoupMessage *message, AsyncRequestData *data)
{
  string response_body = (message->response_body->length > 0) ? message->response_body->data : "";

  (void)session;
  
  data->callback(message->status_code, response_body);
}

