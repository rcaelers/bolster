// main.cc --- OAuth test app
//
// Copyright (C) 2010 Rob Caelers <robc@krandor.org>
// All rights reserved.

#include <string.h>

#include <string>
#include <iostream>

#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */

#include <libsoup/soup-logger.h>
#include <libsoup/soup-session-sync.h>
#include <libsoup/soup-message.h>
#include <libsoup/soup-message-headers.h>
#include <gnome-keyring.h>

  
#ifdef __cplusplus
}
#endif  /* __cplusplus */

#include "OAuth.hh"
#include "WebBackendSoup.hh"

using namespace std;

// static void
// debug_print_headers (const char *name, const char *value, gpointer user_data)
// {
//   (void) user_data;
//   g_debug ("\t%s: %s\n", name, value);
// }

// void
// couch_init()
// {
//   char *uri = NULL;
//   DBusGConnection *bus;
//   DBusGProxy *proxy;
//   gint port;
//   GError *error;
//   gboolean success;
//   SoupMessage *http_message;
//   guint status;

//   error = NULL;
//   bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
//   if (error)
//     {
//       g_warning("Couldn't get session bus: %s", error->message);
//       g_error_free(error);
//       return;
//     }

//   proxy = dbus_g_proxy_new_for_name(bus,
//                                     "org.desktopcouch.CouchDB",
//                                     "/",
//                                     "org.desktopcouch.CouchDB");

//   error = NULL;
//   success = dbus_g_proxy_call(proxy, "getPort", &error,
//                               G_TYPE_INVALID,
//                               G_TYPE_INT, &port, G_TYPE_INVALID);

//   g_object_unref(G_OBJECT(proxy));
//   dbus_g_connection_unref(bus);

//   if (success)
//     {
//       GnomeKeyringAttributeList *attrs;
//       GnomeKeyringResult result;
//       GList *items_found;

//       uri = g_strdup_printf ("http://127.0.0.1:%d/_all_dbs", port);

//       /* Get OAuth tokens from GnomeKeyring */
//       attrs = gnome_keyring_attribute_list_new ();
//       gnome_keyring_attribute_list_append_string (attrs, "desktopcouch", "oauth");

//       result = gnome_keyring_find_items_sync (GNOME_KEYRING_ITEM_GENERIC_SECRET,
//                                               attrs, &items_found);
//       if (result == GNOME_KEYRING_RESULT_OK && items_found != NULL)
//         {
//           gchar **items;
//           char *oauth_c_key = NULL, *oauth_c_secret = NULL, *oauth_t_key = NULL, *oauth_t_secret = NULL;
//           //CouchdbCredentials *credentials;
//           GnomeKeyringFound *first_item = (GnomeKeyringFound *) items_found->data;

//           items = g_strsplit (first_item->secret, ":", 4);
//           if (items)
//             {
//               oauth_c_key = g_strdup (items[0]);
//               oauth_c_secret = g_strdup (items[1]);
//               oauth_t_key = g_strdup (items[2]);
//               oauth_t_secret = g_strdup (items[3]);

//               g_strfreev (items);
//             }

//           gnome_keyring_found_list_free (items_found);


//           SoupSession *http_session = soup_session_sync_new();

          
//           http_message = soup_message_new(SOUP_METHOD_GET, uri);
//           char *body = NULL;
//           if (body != NULL)
//             {
//               soup_message_set_request (http_message, "application/json", SOUP_MEMORY_COPY,
//                                         body, strlen (body));
//             }

//           OAuth oauth(oauth_c_key, oauth_c_secret, "oob", "HMAC-SHA1");
//           string h = "OAuth " + oauth.request("GET", uri, oauth_t_key, oauth_t_secret);

          
//           soup_message_headers_append(http_message->request_headers, "Authorization", h.c_str());
          

//           g_debug ("Sending %s to %s... with headers: ", SOUP_METHOD_GET, uri);
//           soup_message_headers_foreach (http_message->request_headers,
//                                         (SoupMessageHeadersForeachFunc) debug_print_headers,
//                                         NULL);

//           status = soup_session_send_message (http_session, http_message);
//           if (SOUP_STATUS_IS_SUCCESSFUL (status))
//             {
//               g_debug ("Resp: %s\n", http_message->response_body->data);
//               soup_message_headers_foreach (http_message->response_headers,
//                                             (SoupMessageHeadersForeachFunc) debug_print_headers,
//                                             NULL);


//               // if (output != NULL)
//               //   parse_json_response (session, output, http_message, real_error);
//               g_object_unref (G_OBJECT (http_message));
//             }
//           else
//             {
//               // if (error != NULL)
//               //   g_set_error (error, COUCHDB_ERROR, status, "%s",
//               //   http_message->reason_phrase);
              
//               g_debug ("Error: %d %s\n", http_message->status_code, http_message->reason_phrase);

//               g_object_unref (G_OBJECT (http_message));
//             }

//           /* Free memory */
//           g_free (oauth_c_key);
//           g_free (oauth_c_secret);
//           g_free (oauth_t_key);
//           g_free (oauth_t_secret);
//           g_free (uri);
//         }
//       else
//         {
//           g_warning ("Could not get OAuth tokens from keyring: %s",
//                      gnome_keyring_result_to_message (result));
//         }

//       /* Free memory */
//       gnome_keyring_attribute_list_free (attrs);
//     }
//   else
//     {
//       g_warning ("Couldn't get port for desktopcouch: %s", error->message);
//       g_error_free (error);
//     }
// }

static OAuth *web = NULL;

void result(int status, string msg)
{
  cout << status << " " << msg << endl;

  string response;
  cout << web->request("GET", "http://localhost:8888/oauth/photo/", "", response) << endl;
  cout << response << endl;
}

int main(int argc, char **argv)
{
  (void) argc;
  (void) argv;

  g_type_init();

  GMainLoop *loop = g_main_loop_new(NULL, TRUE);

  IWebBackend *backend = new WebBackendSoup();
  web = new OAuth(backend,
                  "http://127.0.0.1:8888/oauth/request_token/",
                  "http://127.0.0.1:8888/oauth/authorize/",
                  "http://127.0.0.1:8888/oauth/access_token/",
                  "<html><head><title>Authorization Ok</title></head><body><div><h1>Authorization Ok</h1>OK</div></body></html>",
                  "<html><head><title>Failed to authorize</title></head><body><div><h1>Failed to authorize</h1>Sorry</div></body></html>");

  // Ubuntu One:
  // web = new OAuth(backend,
  //                 "https://one.ubuntu.com/oauth/request/",
  //                 "https://one.ubuntu.com/oauth/authorize/",
  //                 "https://one.ubuntu.com/oauth/access/", 
  //                 "<html><head><title>Authorization Ok</title></head><body><div><h1>Authorization Ok</h1>OK</div></body></html>",
  //                 "<html><head><title>Failed to authorize</title></head><body><div><h1>Failed to authorize</h1>Sorry</div></body></html>");
  
  
  OAuth::RequestParams parameters;
  web->init("Hello", "World", parameters, &result);
  // Ubuntu One
  web->init("anyone", "anyone", parameters, &result);


  g_main_loop_run(loop);
  g_main_loop_unref(loop);
}
