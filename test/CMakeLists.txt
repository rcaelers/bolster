include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}
    ${GLIB_INCLUDE_DIRS}
    ${SOUP_INCLUDE_DIRS}
    ${KEYRING_INCLUDE_DIRS}
    ${JASON_INCLUDE_DIRS}
)

set(ALLLIBS ${GLIB_LIBRARIES} ${LIBGCRYPT_LIBS} ${SOUP_LIBRARIES} ${KEYRING_LIBRARIES} ${JASON_LIBRARIES})

add_definitions(-W -Wall -Wextra -g -O0)
add_definitions(-DHAVE_CONFIG_H=1)

add_executable(wrap_test wraptest.cc)
#target_link_libraries(wrap_test ${ALLLIBS} couch)

add_executable(dropbox dropbox.cc)
target_link_libraries(dropbox ${ALLLIBS} couch)

add_executable(google google.cc)
target_link_libraries(google ${ALLLIBS} couch)

#add_executable(variant variant.cc)
#target_link_libraries(variant ${ALLLIBS} )

include_directories(../src)
