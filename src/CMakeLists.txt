# CMakeLists.txt file for the nuke-ms project

# we need at lest cmake 2.6
cmake_minimum_required(VERSION 2.6)

project(nuke-ms CXX)


# We need boost for both programs
set(Boost_USE_MULTITHREADED ON)
find_package( Boost 1.35.0 REQUIRED thread system)

# Add Boost libs to linked libraries
include_directories(${Boost_INCLUDE_DIRS})

# add nuke-ms-common
add_subdirectory(common)
    link_directories(${nuke-ms_BINARY_DIR}/common)
    include_directories(${nuke-ms_SOURCE_DIR}/common)

add_subdirectory(client)
add_subdirectory(server)
