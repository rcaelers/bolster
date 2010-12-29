#ifndef OAUTHCLIENT_HH
#define OAUTHCLIENT_HH

#include <string>
#include <map>

#ifdef HAVE_GNOME
#include <libsoup/soup-gnome.h>
#else
#include <libsoup/soup.h>
#endif
#include <json-glib/json-glib.h>

class OAuth;

class OAuthClient
{
private:
  typedef std::map<std::string, std::string> RequestParams;
public:
 	OAuthClient(const OAuth *oauth);

  void request_temporary_credentials(const std::string &http_method,
                                     const std::string &uri
                                     );
  
  void request_resource_owner_authorization(const std::string &http_method,
                                            const std::string &uri);
  
  void perform_request(const std::string &http_method,
                       const std::string &uri,
                       const std::string &header,
                       const std::string &body,
                       bool async = false);

  void finished(SoupSession *session, SoupMessage *message);
  
private:
  static void soup_callback(SoupSession *session, SoupMessage *message, gpointer user_data);
  static void print_headers(const char *name, const char *value, gpointer user_data);
  bool parse_json(SoupMessage *message, JsonParser *json);
  bool parse_query(SoupMessage *message, RequestParams &params);

private:
 	std::string temp_token_key;
 	std::string temp_token_secret;

  std::string token_key;
 	std::string token_secret;

  const OAuth *oauth;
  SoupURI *proxy;
};

#endif
