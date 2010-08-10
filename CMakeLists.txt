# CMakeLists.txt file for the nuke-ms project

# we need at lest cmake 2.6
cmake_minimum_required(VERSION 2.6)

# we are compiling nuke-ms, and it's C++-Code
project(nuke-ms)

# Add source directory, place resulting files in build directory
add_subdirectory(src build)

# Add doc directory, but exclude from all target
add_subdirectory(doc EXCLUDE_FROM_ALL)
