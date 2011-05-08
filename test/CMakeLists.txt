# CMakeLists.txt file for the testing directory.
# Should not be called directly, use parent level cmake file in project
# directory instead.


# we do not want our test executables to end up in the bin/ path.
unset(CMAKE_RUNTIME_OUTPUT_DIRECTORY)

add_subdirectory(common)

add_custom_target(testsuite DEPENDS
    testsuite-common
)

