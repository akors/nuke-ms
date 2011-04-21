# CMakeLists.txt file for the boostasio-impl directory.
# Should not be called directly, use parent level cmake file in project
# directory instead.

# This is a library importing all Boost.Asio routines and classes into one lib.

# add library to project
add_library(nuke-ms-boostasio boostasio_impl.cpp)

# This lib needs Boost and Network libraries
target_link_libraries(nuke-ms-boostasio ${Boost_LIBRARIES} ${WIN32_NETWORK_LIBS})

# install into the bin/ directory if built as DLL on Win32, 
# and into lib/ otherwise
install(TARGETS nuke-ms-boostasio
    RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)
