# CMakeLists.txt file for the testing directory.
# Should not be called directly, use parent level cmake file in src/
# directory instead.

add_executable(testownership testownership.cpp)
target_link_libraries(testownership nuke-ms-common)

add_executable(testsegmlayer testsegmlayer.cpp byteprinter.cpp)
target_link_libraries(testsegmlayer nuke-ms-common)

add_executable(teststringwrap teststringwrap.cpp byteprinter.cpp)
target_link_libraries(teststringwrap nuke-ms-common)

add_executable(testbyteorder testbyteorder.cpp byteprinter.cpp)

