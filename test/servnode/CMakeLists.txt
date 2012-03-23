# CMakeLists.txt file for the testing directory.
# Should not be called directly, use parent level cmake file in test
# directory instead.

set(COMPONENT "servnode")

add_dependencies(testsuite
    connected-client
)

# Add top level include directory
include_directories(${nuke-ms_SOURCE_DIR}/include)


add_executable(connected-client test_connected-client.cpp)
target_link_libraries(connected-client nuke-ms-servnode)
add_test(${COMPONENT}/connected-client connected-client)

# set timeout for tests using networking
set_tests_properties(${COMPONENT}/connected-client PROPERTIES TIMEOUT 3)

