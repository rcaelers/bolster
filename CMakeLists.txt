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

set(ALLLIBS ${GLIB_LIBRARIES} ${LIBGCRYPT_LIBS} ${SOUP_LIBRARIES} ${KEYRING_LIBRARIES} ${JASON_LIBRARIES})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

set (LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR})
link_directories(${LIBRARY_OUTPUT_PATH})

add_subdirectory(src)
add_subdirectory(test)
