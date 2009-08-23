# CMakeLists.txt file for the client application.
# Should not be called directly, use top-level cmake file in src/
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
    gui/gui.cpp
    protocol/protocol.cpp
    protocol/statemachine.cpp
)

add_executable(nuke-ms-client ${CLIENT_SRCS})

target_link_libraries(nuke-ms-client
    nuke-ms-common
    ${Boost_LIBRARIES}
    ${wxWidgets_LIBRARIES}
	${WIN32_NETWORK_LIBS}
)

# We want our executables to be in the bin/ directory
set_target_properties(nuke-ms-client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR}/bin
)
