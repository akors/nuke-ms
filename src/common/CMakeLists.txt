# CMakeLists.txt file for the common subroutines library.
# Should not be called directly, use parent level cmake file in src/
# directory instead.

# set library sources
set(COMMON_SRCS msglayer.cpp neartypes.cpp)

# add library to project
add_library(nuke-ms-common ${COMMON_SRCS})
