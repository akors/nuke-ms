# CMakeLists.txt file for the server directory.
# Should not be called directly, use parent level cmake file in project
# directory instead.

# these are the sources for the server
set(SERVER_SRCS dispatcher.cpp main.cpp remotepeer.cpp)

# temporary fix to prevent failing assertion
add_definitions("-DNUKE_MS_REFCOUNTER_NOT_MULTITHREADED")

add_executable(nuke-ms-serv ${SERVER_SRCS})


# link Boost, Win32 network libs and Boost.Asio implementation library if desired
if(BOOSTASIO_OWNLIB)
	set(SERVER_DEPS nuke-ms-common  ${Boost_LIBRARIES} nuke-ms-boostasio)
else(BOOSTASIO_OWNLIB)
	set(SERVER_DEPS nuke-ms-common  ${Boost_LIBRARIES} ${WIN32_NETWORK_LIBS})
endif(BOOSTASIO_OWNLIB)


target_link_libraries(nuke-ms-serv ${SERVER_DEPS})

install(TARGETS nuke-ms-serv
    RUNTIME DESTINATION bin
)

