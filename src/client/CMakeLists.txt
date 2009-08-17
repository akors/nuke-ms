# first of all find wxWidgets
find_package(wxWidgets REQUIRED base core)

# add wxWidgets flags
include(${wxWidgets_USE_FILE})

# add client directory for includes 
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

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
)
