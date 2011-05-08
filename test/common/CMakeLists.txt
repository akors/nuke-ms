# CMakeLists.txt file for the testing directory.
# Should not be called directly, use parent level cmake file in test
# directory instead.


add_custom_target(testsuite-common DEPENDS
    test_ownership
    test_msglayer
    test_byteorder
)

# Add top level include directory
include_directories(${nuke-ms_SOURCE_DIR}/include)



add_executable(test_ownership test_ownership.cpp)
target_link_libraries(test_ownership nuke-ms-common)
add_test(ownership test_ownership)

add_executable(test_msglayer test_msglayer.cpp byteprinter.cpp)
target_link_libraries(test_msglayer nuke-ms-common)
add_test(msglayer test_msglayer)

add_executable(test_byteorder test_byteorder.cpp byteprinter.cpp)
add_test(byteorder test_byteorder)

