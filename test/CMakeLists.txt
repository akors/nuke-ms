# CMakeLists.txt file for the testing directory.
# Should not be called directly, use parent level cmake file in project
# directory instead.


# we do not want our test executables to end up in the bin/ path.
unset(CMAKE_RUNTIME_OUTPUT_DIRECTORY)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Target to only build the testsuite
add_custom_target(testsuite)

# Target to build and run the testsuite
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS testsuite)

# Add component directories
add_subdirectory(common)
add_subdirectory(servnode)

