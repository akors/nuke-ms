set(SERVER_SRCS dispatcher.cpp main.cpp remotepeer.cpp)

add_executable(nuke-ms-serv ${SERVER_SRCS})

target_link_libraries(nuke-ms-serv nuke-ms-common ${Boost_LIBRARIES})
