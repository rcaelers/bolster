//
// Copyright (C) 2010, 2011 Rob Caelers <robc@krandor.nl>
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef WEBBACKENDSOUP_HH
#define WEBBACKENDSOUP_HH

#include <string>
#include <map>
#include <list>

#ifdef HAVE_GNOME
#include <libsoup/soup-gnome.h>
#else
#include <libsoup/soup.h>
#endif

#include "IWebBackend.hh"
#include "FunctionWrappers.hh"

class OAuth;
class IHttpRequestFilter;

class WebBackendSoup : public IWebBackend
{
public:
 	WebBackendSoup();
  virtual ~WebBackendSoup();

  virtual void add_filter(IHttpRequestFilter *filter);
  virtual void remove_filter(IHttpRequestFilter *filter);
  
  virtual int request(const std::string &http_method,
                      const std::string &uri,
                      const std::string &body,
                      std::string &response_body);
  
  virtual void request(const std::string &http_method,
                       const std::string &uri,
                       const std::string &body,
                       const WebReplyCallback callback);
  
  virtual void listen(const WebRequestCallback callback,
                      const std::string &path,
                      int &port);

  virtual void stop_listen(const std::string &path);
  
private:
  typedef void (WebBackendSoup::*AsyncRequest)(SoupServer *, SoupMessage *, const char *, GHashTable *, SoupClientContext *, WebRequestCallback);
  typedef void (WebBackendSoup::*AsyncReply)(SoupSession *, SoupMessage *, WebReplyCallback);
  
  typedef FunctionForwarder<AsyncReply, SoupSessionCallback, 3, WebReplyCallback> AsyncReplyForwarder;
  typedef FunctionForwarder<AsyncRequest, SoupServerCallback, 6, WebRequestCallback> AsyncRequestForwarder;

  class ServerData
  {
  public:
    ServerData(SoupServer *server, AsyncRequestForwarder *forwarder) : server(server), forwarder(forwarder)
    {
    }

    ~ServerData()
    {
      g_object_unref(server);
      delete forwarder;
    }

  private:
    SoupServer *server;
    AsyncRequestForwarder *forwarder;
  };
  
  void server_callback(SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query, SoupClientContext *context, WebRequestCallback cb);
  void client_callback(SoupSession *session, SoupMessage *message, WebReplyCallback cb);

  void init_async();
  void init_sync();

  SoupMessage *create_soup_message(const std::string &http_method,
                                   const std::string &uri,
                                   const std::string &body);

private:
  SoupSession *sync_session;
  SoupSession *async_session;
  SoupURI *proxy;

  typedef std::map<std::string, ServerData*> ServerList;
  typedef std::list<IHttpRequestFilter*> FilterList;
  
  ServerList servers;
  FilterList filters;
};

#endif
