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
