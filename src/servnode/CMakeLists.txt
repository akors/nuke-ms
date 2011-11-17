# CMakeLists.txt file for the servnode directory.
# Should not be called directly, use parent level cmake file in project
# directory instead.

# set library sources
set(SERVNODE_SRCS connected-client.cpp)

# add library to project
add_library(nuke-ms-servnode ${SERVNODE_SRCS})

# link Boost, Win32 network libs and Boost.Asio implementation library if desired
if(BOOSTASIO_OWNLIB)
	set(SERVNODE_DEPS nuke-ms-common ${Boost_LIBRARIES} nuke-ms-boostasio)
else(BOOSTASIO_OWNLIB)
	set(SERVNODE_DEPS nuke-ms-common ${Boost_LIBRARIES} ${WIN32_NETWORK_LIBS})
endif(BOOSTASIO_OWNLIB)

# This lib needs nuke-ms-common library, Boost and the Win32 network libs
target_link_libraries(nuke-ms-servnode ${SERVNODE_DEPS})

# install into the bin/ directory if built as DLL on Win32,
# and into lib/ otherwise
install(TARGETS nuke-ms-servnode
    RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)
