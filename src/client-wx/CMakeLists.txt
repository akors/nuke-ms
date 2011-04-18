# CMakeLists.txt file for the client-wx directory.
# Should not be called directly, use parent level cmake file in project
# directory instead.

# first of all find wxWidgets
find_package(wxWidgets REQUIRED core base)

# add wxWidgets flags, include dirs and link dirs
include(${wxWidgets_USE_FILE})

# add client directory for includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# these are the sources for the client
set(CLIENT_SRCS
    main.cpp
    gui.cpp
)

add_executable(nuke-ms-client WIN32 ${CLIENT_SRCS})

target_link_libraries(nuke-ms-client
    nuke-ms-common
	nuke-ms-clientnode
    ${Boost_LIBRARIES}
    ${wxWidgets_LIBRARIES}
)

install(TARGETS nuke-ms-client
    RUNTIME DESTINATION bin
)
