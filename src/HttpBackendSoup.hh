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

#ifndef HTTPBACKENDSOUP_HH
#define HTTPBACKENDSOUP_HH

#include <string>
#include <map>
#include <list>
#include <boost/shared_ptr.hpp>

#ifdef HAVE_GNOME
#include <libsoup/soup-gnome.h>
#else
#include <libsoup/soup.h>
#endif

#include "IHttpBackend.hh"

class HttpBackendSoup : public IHttpBackend
{
public:
  typedef boost::shared_ptr<HttpBackendSoup> Ptr;

  static Ptr create();

public:
 	HttpBackendSoup();
  virtual ~HttpBackendSoup();

  virtual void add_filter(IHttpFilter::Ptr filter);
  virtual void remove_filter(IHttpFilter::Ptr filter);

  virtual HttpReply::Ptr request(HttpRequest::Ptr request);
  virtual HttpReply::Ptr request(HttpRequest::Ptr request, const HttpReplyCallback callback);
  
  virtual void listen(const HttpRequestCallback callback,
                      const std::string &path,
                      int &port);

  virtual void stop_listen(const std::string &path);
  
private:

  class AsyncServerData
  {
  public:
    AsyncServerData(HttpBackendSoup *backend, HttpRequestCallback callback)
      : callback(callback), backend(backend)
    {
    }

    static void cb(SoupServer *, SoupMessage *, const char *, GHashTable *, SoupClientContext *, gpointer);

    HttpRequestCallback callback;
    
  private:
    HttpBackendSoup *backend;
  };
  
  void server_callback(SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query, SoupClientContext *context, AsyncServerData *data);
  
  void init_async();
  void init_sync();


  void apply_request_filters(HttpRequest::Ptr request);
  
private:
  SoupSession *sync_session;
  SoupSession *async_session;
  SoupURI *proxy;

  typedef std::map<std::string, SoupServer*> ServerList;
  typedef std::list<IHttpFilter::Ptr> FilterList;
  
  ServerList servers;
  FilterList filters;
};

#endif