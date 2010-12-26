# CMakeLists.txt file for the common subroutines library.
# Should not be called directly, use parent level cmake file in src/
# directory instead.

# set library sources
set(CLIENTNODE_SRCS protocol.cpp statemachine.cpp)

# add library to project
add_library(nuke-ms-clientnode ${CLIENTNODE_SRCS})

# This lib needs nuke-ms-common library
target_link_libraries(nuke-ms-clientnode nuke-ms-common)
