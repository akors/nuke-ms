# CMakeLists.txt file for the common subroutines library.
# Should not be called directly, use parent level cmake file in src/
# directory instead.

# set library sources
set(CLIENTNODE_SRCS clientnode.cpp statemachine.cpp)

# add library to project
add_library(nuke-ms-clientnode ${CLIENTNODE_SRCS})

# This lib needs nuke-ms-common library, Boost and the Win32 network libs
target_link_libraries(nuke-ms-clientnode
    nuke-ms-common 
    ${Boost_LIBRARIES}
    ${WIN32_NETWORK_LIBS}
)

# install into the bin/ directory if built as DLL on Win32, 
# and into lib/ otherwise
install(TARGETS nuke-ms-clientnode
    RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

# install headers to include directory
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DESTINATION "include/${PROJECT_NAME}"
    FILES_MATCHING PATTERN "*.hpp"
)
