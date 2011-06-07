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

add_executable(test tests.cc CouchDBTests.cc TestUtils.cc)
target_link_libraries(test ${ALLLIBS} couch)
set_target_properties(test PROPERTIES COMPILE_FLAGS -I${CMAKE_SOURCE_DIR}/src)

add_executable(couch_test main.cc Settings.cc)
target_link_libraries(couch_test ${ALLLIBS} couch)
#set_target_properties(couch_test PROPERTIES COMPILE_FLAGS -I..)

add_executable(wrap_test wraptest.cc)
#target_link_libraries(wrap_test ${ALLLIBS} couch)

add_executable(u1_test u1.cc)
target_link_libraries(u1_test ${ALLLIBS} couch)

include_directories(../src)
