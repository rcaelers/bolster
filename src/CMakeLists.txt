project(Couch)

include(FindPkgConfig)

#link_directories(
#    ${GLIB_LIBRARY_DIRS}
#    ${SOUP_LIBRARY_DIRS}
#    ${KEYRING_LIBRARY_DIRS}
#    ${JASON_LIBRARY_DIRS}
#)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}
    ${GLIB_INCLUDE_DIRS}
    ${SOUP_INCLUDE_DIRS}
    ${KEYRING_INCLUDE_DIRS}
    ${JASON_INCLUDE_DIRS}
)

set(ALLLIBS ${GLIB_LIBRARIES} ${LIBGCRYPT_LIBS} ${SOUP_LIBRARIES} ${KEYRING_LIBRARIES} ${JASON_LIBRARIES})

add_definitions(-W -Wall -g -O0)
add_definitions(-DHAVE_CONFIG_H=1)

#add_library(couch CouchDBFactory.cc HttpBackendSoup.cc OAuth.cc OAuthWorkflow.cc OAuth2.cc OAuth2Workflow.cc UbuntuOneSSO.cc Secrets.cc CouchDB.cc DesktopCouch.cc DesktopCouchDBus.cc UbuntuOneCouch.cc PlainCouch.cc Uri.cc GDBusWrapper.cc jsoncpp.cpp Document.cc Database.cc DropboxAuth.cc GoogleAuth.cc)

add_library(couch HttpRequest.cc HttpReply.cc HttpExecuteSoup.cc HttpBackendSoup.cc OAuth.cc OAuthFilter.cc OAuth2.cc OAuth2Filter.cc Uri.cc jsoncpp.cpp DropboxAuth.cc GoogleAuth.cc)

target_link_libraries(couch ${ALLLIBS})

