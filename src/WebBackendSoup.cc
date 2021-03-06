// Copyright (C) 2010, 2011 by Rob Caelers <robc@krandor.nl>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "WebBackendSoup.hh"

#include <glib.h>

#include "WebBackendException.hh"
#include "OAuth.hh"
#include "Uri.hh"

using namespace std;

WebBackendSoup::WebBackendSoup()
  : sync_session(NULL),
    async_session(NULL),
    proxy(NULL)
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

  for (ServerList::iterator i = servers.begin(); i != servers.end(); i++)
    {
      g_object_unref(i->second);
    }
}

void
WebBackendSoup::add_filter(IHttpRequestFilter *filter)
{
  filters.push_back(filter);
}


void
WebBackendSoup::remove_filter(IHttpRequestFilter *filter)
{
  filters.remove(filter);
}


int
WebBackendSoup::request(const string &http_method,
                        const string &uri,
                        const string &body,
                        string &response_body)
{
  if (sync_session == NULL)
    {
      init_sync();
    }

  SoupMessage *message = create_soup_message(http_method, uri, body);
  soup_session_send_message(sync_session, message);

  response_body = (message->response_body->length > 0) ? message->response_body->data : "";
  int ret = message->status_code;
  
	g_object_unref(message);
  
  return ret;
}


void
WebBackendSoup::request(const string &http_method, const string &uri, const string &body, WebReplyCallback callback)
{
  if (async_session == NULL)
    {
      init_async();
    }

  SoupMessage *message = create_soup_message(http_method, uri, body);
  AsyncReplyForwarder *forwarder = new AsyncReplyForwarder(this, &WebBackendSoup::client_callback, callback);
  forwarder->set_once();
  soup_session_queue_message(async_session, message, AsyncReplyForwarder::dispatch, forwarder);
}


void
WebBackendSoup::listen(WebRequestCallback callback, const string &path, int &port)
{
  SoupAddress *addr = soup_address_new("127.0.0.1", SOUP_ADDRESS_ANY_PORT);
  soup_address_resolve_sync(addr, NULL);

  SoupServer *server = soup_server_new(SOUP_SERVER_SERVER_HEADER, "OAuth",
                                       SOUP_SERVER_INTERFACE, addr,
                                       NULL);
	g_object_unref (addr);

	if (server == NULL)
    {
      throw WebBackendException("Cannot receive incoming connections.");
    }
  
  port = soup_server_get_port(server);
  g_debug("Listening on %d", port);

  AsyncRequestForwarder *forwarder = new AsyncRequestForwarder(this, &WebBackendSoup::server_callback, callback);

  ServerData *data = new ServerData(server, forwarder);
  servers[path] = data;
  
  soup_server_add_handler(server, path.c_str(), AsyncRequestForwarder::dispatch, forwarder, NULL);
	soup_server_run_async(server);
}


void
WebBackendSoup::stop_listen(const std::string &path)
{
  if (servers.find(path) != servers.end())
    {
      ServerData *data = servers[path];
      delete data;
      servers.erase(path);
    }
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
                                    const string &body)
{
  SoupMessage *message = soup_message_new(http_method.c_str(), uri.c_str());
  if (message == NULL)
    {
      throw WebBackendException("Cannot create HTTP request: " + http_method + " " + uri);
    }

  if (body != "")
    {
      soup_message_set_request(message, "application/json", SOUP_MEMORY_COPY,
                               body.c_str(), body.size());
    }

  map<string, string> headers;

  for (FilterList::iterator i = filters.begin(); i != filters.end(); i++)
    {
      string u = uri;
      string b = body;
      (*i)->filter_http_request(http_method, u, b, headers);
    }
  
  for (map<string, string>::const_iterator i = headers.begin(); i != headers.end(); i++)
    {
      soup_message_headers_append(message->request_headers, i->first.c_str(), i->second.c_str());
    }
  
	soup_message_set_flags(message, SOUP_MESSAGE_NO_REDIRECT);

  return message;
}


void
WebBackendSoup::server_callback(SoupServer *, SoupMessage *message, const char *path,
                                GHashTable *query, SoupClientContext *context,
                                WebRequestCallback callback)
{
  (void) path;
  (void) context;
  (void) query;

  SoupURI *uri = soup_message_get_uri(message);
  string response_query = (uri != NULL && uri->query != NULL) ? uri->query : "";
  string response_body = (message->response_body->length > 0) ? message->response_body->data : "";
  string content_type;
  string reply;

  callback(message->method, response_query, response_body, content_type, reply);

  soup_message_set_status(message, SOUP_STATUS_OK);
  soup_message_set_response(message, content_type.c_str(), SOUP_MEMORY_COPY, reply.c_str(), reply.length());
}


void
WebBackendSoup::client_callback(SoupSession *session, SoupMessage *message, WebReplyCallback callback)
{
  (void)session;

  string response_body = (message->response_body->length > 0) ? message->response_body->data : "";
  callback(message->status_code, response_body);
	g_object_unref(message);
}
