cmake_minimum_required (VERSION 2.6)
project (OAuth)

include(FindPkgConfig)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(CRYPTO REQUIRED libcrypto++)
pkg_check_modules(SOUP REQUIRED libsoup-2.4)
pkg_check_modules(DBUS REQUIRED dbus-glib-1)
pkg_check_modules(KEYRING REQUIRED gnome-keyring-1)

link_directories(
    ${GLIB_LIBRARY_DIRS}
    ${CRYPTO_LIBRARY_DIRS}
    ${SOUP_LIBRARY_DIRS}
    ${DBUS_LIBRARY_DIRS}
    ${KEYRING_LIBRARY_DIRS}
)

include_directories(
    ${GLIB_INCLUDE_DIRS}
    ${CRYPTO_INCLUDE_DIRS}
    ${SOUP_INCLUDE_DIRS}
    ${DBUS_INCLUDE_DIRS}
    ${KEYRING_INCLUDE_DIRS}
)

add_definitions(-W -Wall -Wextra -g -O0)
add_executable(oauth OAuth.cc StringUtil.cc main.cc)
target_link_libraries(oauth ${GLIB_LIBRARIES} ${CRYPTO_LIBRARIES} ${SOUP_LIBRARIES} ${DBUS_LIBRARIES} ${KEYRING_LIBRARIES})