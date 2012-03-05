# CMakeLists.txt file for the testing directory.
# Should not be called directly, use parent level cmake file in test
# directory instead.

set(COMPONENT "common")

add_dependencies(testsuite
    byteorder
    packedstruct
    stringwraplayer
    segmentationlayer
    neartypes
)

# Add top level include directory
include_directories(${nuke-ms_SOURCE_DIR}/include)


add_executable(byteorder test_byteorder.cpp byteprinter.cpp)
add_test(${COMPONENT}/byteorder byteorder)

add_executable(stringwraplayer test_stringwraplayer.cpp byteprinter.cpp)
target_link_libraries(stringwraplayer nuke-ms-common)
add_test(${COMPONENT}/stringwraplayer stringwraplayer)

add_executable(packedstruct test_packedstruct.cpp)
add_test(${COMPONENT}/packedstruct packedstruct)

add_executable(segmentationlayer test_segmentationlayer.cpp byteprinter.cpp)
target_link_libraries(segmentationlayer nuke-ms-common)
add_test(${COMPONENT}/segmentationlayer segmentationlayer)

add_executable(neartypes test_neartypes.cpp)
target_link_libraries(neartypes nuke-ms-common)
add_test(${COMPONENT}/neartypes neartypes)


