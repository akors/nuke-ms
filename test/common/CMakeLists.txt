# CMakeLists.txt file for the testing directory.
# Should not be called directly, use parent level cmake file in test
# directory instead.


add_custom_target(testsuite-common DEPENDS
    testownership
    testmsglayer
    testbyteorder
)

# Add top level include directory
include_directories(${nuke-ms_SOURCE_DIR}/include)



add_executable(testownership testownership.cpp)
target_link_libraries(testownership nuke-ms-common)
add_test(ownership testownership)

add_executable(testmsglayer testmsglayer.cpp byteprinter.cpp)
target_link_libraries(testmsglayer nuke-ms-common)
add_test(msglayer testmsglayer)

add_executable(testbyteorder testbyteorder.cpp byteprinter.cpp)
add_test(byteorder testbyteorder)

