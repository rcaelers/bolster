// Copyright (C) 2010, 2011, 2012 by Rob Caelers <robc@krandor.nl>
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

#include "HttpBackendSoup.hh"

#include <glib.h>

#include "HttpBackendException.hh"
#include "Uri.hh"

#include "HttpRequest.hh"
#include "HttpReplySoup.hh"

using namespace std;

HttpBackendSoup::Ptr
HttpBackendSoup::create()
{
  return Ptr(new HttpBackendSoup());
}


HttpBackendSoup::HttpBackendSoup()
  : sync_session(NULL),
    async_session(NULL),
    proxy(NULL)
{
}


HttpBackendSoup::~HttpBackendSoup()
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
HttpBackendSoup::add_filter(IHttpFilter::Ptr filter)
{
  filters.push_back(filter);
}


void
HttpBackendSoup::remove_filter(IHttpFilter::Ptr filter)
{
  filters.remove(filter);
}


HttpReply::Ptr
HttpBackendSoup::request(HttpRequest::Ptr request)
{
  if (sync_session == NULL)
    {
      init_sync();
    }

  apply_request_filters(request);

  HttpReplySoup::Ptr reply = HttpReplySoup::create(request);
  reply->init(sync_session);

  return reply;
}


HttpReply::Ptr
HttpBackendSoup::request(HttpRequest::Ptr request, HttpReplyCallback callback)
{
  if (async_session == NULL)
    {
      init_async();
    }

  apply_request_filters(request);
 
  HttpReplySoup::Ptr reply = HttpReplySoup::create(request);
  reply->init(async_session, callback);

  return reply;
}


void
HttpBackendSoup::listen(HttpRequestCallback callback, const string &path, int &port)
{
  AsyncServerData *data = new AsyncServerData(this, callback);

  SoupAddress *addr = soup_address_new("127.0.0.1", SOUP_ADDRESS_ANY_PORT);
  soup_address_resolve_sync(addr, NULL);

  SoupServer *server = soup_server_new(SOUP_SERVER_SERVER_HEADER, "OAuth",
                                       SOUP_SERVER_INTERFACE, addr,
                                       NULL);
	g_object_unref (addr);

	if (server == NULL)
    {
      throw HttpBackendException("Cannot receive incoming connections.");
    }
  
  port = soup_server_get_port(server);
  g_debug("Listening on %d", port);

  servers[path] = server;
  
  soup_server_add_handler(server, path.c_str(), AsyncServerData::cb, data, NULL);
	soup_server_run_async(server);
}


void
HttpBackendSoup::stop_listen(const std::string &path)
{
  if (servers.find(path) != servers.end())
    {
      SoupServer *server = servers[path];
      g_object_unref(server);
      servers.erase(path);
    }
}


void
HttpBackendSoup::init_async()
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
      throw HttpBackendException("Cannot create session.");
    }
  
  if (proxy)
    {
      g_object_set(G_OBJECT(async_session), SOUP_SESSION_PROXY_URI, proxy, NULL);
    }
}


void
HttpBackendSoup::init_sync()
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
      throw HttpBackendException("Cannot create session.");
    }
  
  if (proxy)
    {
      g_object_set(G_OBJECT(sync_session), SOUP_SESSION_PROXY_URI, proxy, NULL);
    }
}


void
HttpBackendSoup::apply_request_filters(HttpRequest::Ptr request)
{
  for (FilterList::iterator i = filters.begin(); i != filters.end(); i++)
    {
      IHttpRequestFilter::Ptr f = boost::dynamic_pointer_cast<IHttpRequestFilter>(*i);
      if (f)
        {
          f->filter_http_request(request);
        }
    }
}


void
HttpBackendSoup::AsyncServerData::cb(SoupServer *server, SoupMessage *message, const char *path,
                                    GHashTable *query, SoupClientContext *context, gpointer data)
{
  AsyncServerData *d = (AsyncServerData *)data;
  d->backend->server_callback(server, message, path, query, context, d);
  delete d;
}

void
HttpBackendSoup::server_callback(SoupServer *, SoupMessage *message, const char *path,
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

  soup_message_set_status(message, SOUP_STATUS_OK);
  soup_message_set_response(message, content_type.c_str(), SOUP_MEMORY_COPY, reply.c_str(), reply.length());
}
