# CMakeLists.txt file for the testing directory.
# Should not be called directly, use parent level cmake file in test
# directory instead.


add_custom_target(testsuite-common DEPENDS
    test_byteorder
    test_ownership
    test_stringwraplayer
    test_segmentationlayer
    test_neartypes
)

# Add top level include directory
include_directories(${nuke-ms_SOURCE_DIR}/include)


add_executable(test_byteorder test_byteorder.cpp byteprinter.cpp)
add_test(byteorder test_byteorder)

add_executable(test_ownership test_ownership.cpp)
target_link_libraries(test_ownership nuke-ms-common)
add_test(ownership test_ownership)

add_executable(test_stringwraplayer test_stringwraplayer.cpp byteprinter.cpp)
target_link_libraries(test_stringwraplayer nuke-ms-common)
add_test(stringwraplayer test_stringwraplayer)

add_executable(test_segmentationlayer test_segmentationlayer.cpp byteprinter.cpp)
target_link_libraries(test_segmentationlayer nuke-ms-common)
add_test(segmentationlayer test_segmentationlayer)

add_executable(test_neartypes test_neartypes.cpp)
target_link_libraries(test_neartypes nuke-ms-common)
add_test(neartypes test_neartypes)


