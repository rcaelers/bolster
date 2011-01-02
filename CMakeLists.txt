cmake_minimum_required (VERSION 2.6)
project (OAuth)

include(FindPkgConfig)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(SOUP REQUIRED libsoup-2.4)
pkg_check_modules(KEYRING REQUIRED gnome-keyring-1)
pkg_check_modules(JASON REQUIRED json-glib-1.0)

include(CheckIncludeFiles)
check_include_files(gcrypt.h HAVE_GCRYPT_H)
if (HAVE_GCRYPT_H)
   find_library(LIBGCRYPT_LIBS NAMES gcrypt)
endif (HAVE_GCRYPT_H)
if (LIBGCRYPT_LIBS)
   set(LIBGCRYPT_FOUND TRUE)
   message(STATUS "Libgcrypt found: ${LIBGCRYPT_LIBS}")
elseif (Libgcrypt_FIND_REQUIRED)
   message(FATAL_ERROR "Could not find Libgcrypt")
endif (LIBGCRYPT_LIBS)


link_directories(
    ${GLIB_LIBRARY_DIRS}
    ${SOUP_LIBRARY_DIRS}
    ${KEYRING_LIBRARY_DIRS}
    ${JASON_LIBRARY_DIRS}
)

include_directories(
    ${GLIB_INCLUDE_DIRS}
    ${SOUP_INCLUDE_DIRS}
    ${KEYRING_INCLUDE_DIRS}
    ${JASON_INCLUDE_DIRS}
)

add_definitions(-W -Wall -Wextra -g -O0)
add_executable(oauth WebBackendSoup.cc OAuth.cc UbuntuOneSSO.cc Secrets.cc CouchDB.cc DesktopCouch.cc UbuntuOneCouch.cc StringUtil.cc main.cc)
target_link_libraries(oauth ${GLIB_LIBRARIES} ${LIBGCRYPT_LIBS} ${SOUP_LIBRARIES} ${KEYRING_LIBRARIES} ${JASON_LIBRARIES})

add_definitions(-DHAVE_CONFIG_H=1)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

# for including config.h
include_directories(${CMAKE_BINARY_DIR})
