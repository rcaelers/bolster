#include <iostream>
#include <sstream>
#include <algorithm>
#include <string.h>
#include <list>
#include <stdint.h>

#include <glib.h>


#include "OAuthClient.hh"
#include "OAuth.hh"
#include "StringUtil.hh"

using namespace std;

OAuthClient::OAuthClient(const OAuth *oauth)
  : oauth(oauth)
{
  proxy = NULL;
}


void
OAuthClient::request_temporary_credentials(const string &http_method,
                                           const string &uri
                                           )
{
  const string &header = oauth->request_temporary_credentials(http_method, uri);

  perform_request(http_method, uri, header, "");
}


void
OAuthClient::request_resource_owner_authorization(const string &http_method,
                                                  const string &uri
                                                  )
{
  const string &header = oauth->request_resource_owner_authorization(http_method, uri, temp_token_key, temp_token_secret);

  perform_request(http_method, uri, header, "");
}


void
OAuthClient::perform_request(const string &http_method,
                             const string &uri,
                             const string &header,
                             const string &body,
                             bool async)
{
  SoupSession *session;
  
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

  g_debug("Auth = %s", header.c_str());
	soup_message_set_flags(message, SOUP_MESSAGE_NO_REDIRECT);
  soup_message_headers_append(message->request_headers, "Authorization", header.c_str());
  soup_session_queue_message(session, message, soup_callback, this);
}


void
OAuthClient::print_headers(const char *name, const char *value, gpointer user_data)
{
  (void) user_data;
  g_debug("%s: %s", name, value);
}

bool
OAuthClient::parse_query(SoupMessage *message, RequestParams &params)
{
  g_debug("Response len: %d", (int) message->response_body->length);
  
	if (message->response_body->length > 0)
    {
      g_debug("Response body: %s", message->response_body->data);

      vector<string> query_params;
      StringUtil::split(message->response_body->data, '&', query_params);

      for (size_t i = 0; i < query_params.size(); ++i)
        {
          vector<string> param_elements;
          StringUtil::split(query_params[i], '=', param_elements);
      
          if (param_elements.size() == 2)
            {
              params[param_elements[0]] = param_elements[1];
            }
        }

    }

  return true;
}

bool
OAuthClient::parse_json(SoupMessage *message, JsonParser *json)
{
  GString *str = NULL;
  bool success = true;
  GError *error = NULL;

  g_debug("Response len: %d", (int) message->response_body->length);
  
	if (message->response_body->length > 0)
    {
      g_debug("Response body: %s", message->response_body->data);

      if (!json_parser_load_from_data (json,
                                       (const gchar *) message->response_body->data,
                                       message->response_body->length,
                                       &error))
        {
          g_object_unref (G_OBJECT(json));
          success = false;
        }

      g_string_free (str, TRUE);
    }

	return success;
}

void
OAuthClient::finished(SoupSession *session, SoupMessage *message)
{
  (void) session;
	soup_message_headers_foreach(message->response_headers,
                               (SoupMessageHeadersForeachFunc) print_headers, NULL);

  g_debug("Result %d %s", message->status_code, message->reason_phrase);
  
  if (SOUP_STATUS_IS_TRANSPORT_ERROR(message->status_code))
    {
      g_error("ERROR: %d %s", message->status_code, message->reason_phrase);
    }
  else if (SOUP_STATUS_IS_SUCCESSFUL (message->status_code))
    {
      RequestParams parameters;
      parse_query(message, parameters);

      temp_token_key = parameters["oauth_token"];
      temp_token_secret = parameters["oauth_token_secret"];

      if (parameters["oauth_callback_confirmed"] != "true")
        {
          g_debug("oauth_callback_confirmed error");

        }
      else
        {
          request_resource_owner_authorization("POST", 
                                               "http://127.0.0.1:8888/oauth/authorize/?oauth_token=" + oauth->escape_uri(temp_token_key));

          
        }
      g_debug("Result %s %s", temp_token_key.c_str(), temp_token_secret.c_str());
    }

  //  g_object_unref(G_OBJECT(message));
 
}

void
OAuthClient::soup_callback(SoupSession *session, SoupMessage *message, gpointer user_data)
{
  OAuthClient *oath = (OAuthClient *) user_data;
  oath->finished(session, message);
}
