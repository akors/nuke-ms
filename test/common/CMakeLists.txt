# CMakeLists.txt file for the testing directory.
# Should not be called directly, use parent level cmake file in test
# directory instead.


add_custom_target(testsuite-common DEPENDS
    ownership
    msglayer
    byteorder
)

# Add top level include directory
include_directories(${nuke-ms_SOURCE_DIR}/include)

add_executable(ownership testownership.cpp)
target_link_libraries(ownership nuke-ms-common)

add_executable(msglayer testmsglayer.cpp byteprinter.cpp)
target_link_libraries(msglayer nuke-ms-common)

add_executable(byteorder testbyteorder.cpp byteprinter.cpp)

