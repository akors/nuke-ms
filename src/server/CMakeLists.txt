# CMakeLists.txt file for the server application.
# Should not be called directly, use parent level cmake file in src/
# directory instead.

# these are the sources for the server
set(SERVER_SRCS dispatcher.cpp main.cpp remotepeer.cpp)

add_executable(nuke-ms-serv ${SERVER_SRCS})

target_link_libraries(nuke-ms-serv nuke-ms-common ${Boost_LIBRARIES} ${WIN32_NETWORK_LIBS})

# We want our executables to be in the bin/ directory
set_target_properties(nuke-ms-serv PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR}/bin
)