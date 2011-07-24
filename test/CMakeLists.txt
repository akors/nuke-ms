# CMakeLists.txt file for the testing directory.
# Should not be called directly, use parent level cmake file in project
# directory instead.


# we do not want our test executables to end up in the bin/ path.
unset(CMAKE_RUNTIME_OUTPUT_DIRECTORY)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-DNUKE_MS_TESTSUITE)

add_custom_target(testsuite COMMAND ${CMAKE_CTEST_COMMAND})
add_subdirectory(common)
