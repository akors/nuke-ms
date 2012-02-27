# CMakeLists.txt file for the testing directory.
# Should not be called directly, use parent level cmake file in test
# directory instead.

set(COMPONENT "servnode")

add_dependencies(testsuite
    test_connected-client
)

# Add top level include directory
include_directories(${nuke-ms_SOURCE_DIR}/include)


add_executable(test_connected-client test_connected-client.cpp)
target_link_libraries(test_connected-client nuke-ms-servnode)
add_test(${COMPONENT}/connected-client test_connected-client)
