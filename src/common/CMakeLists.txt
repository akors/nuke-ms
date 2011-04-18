# CMakeLists.txt file for the common subroutines library.
# Should not be called directly, use parent level cmake file in src/
# directory instead.

# set library sources
set(COMMON_SRCS msglayer.cpp neartypes.cpp)

# add library to project
add_library(nuke-ms-common ${COMMON_SRCS})

# install into the bin/ directory if built as DLL on Win32, 
# and into lib/ otherwise
install(TARGETS nuke-ms-common
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# install headers to include directory
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DESTINATION "include/${PROJECT_NAME}"
    FILES_MATCHING PATTERN "*.hpp"
)

