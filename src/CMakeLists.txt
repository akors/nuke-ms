# CMakeLists.txt file for the nuke-ms project

# we need at lest cmake 2.6
cmake_minimum_required(VERSION 2.6)

# we are compiling nuke-ms, and it's C++-Code
project(nuke-ms CXX)


# Not compiling when in source directory
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed.\nPlease delete the CMakeCache.txt file and the CMakeFiles directory, change into a different directory and start the build from there.")
endif()


# We need boost for all executeables
set(Boost_USE_MULTITHREADED ON)
find_package( Boost 1.35.0 REQUIRED thread system)

# Add Boost header and library directories
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

# All executables need the nuke-ms-common library
link_directories(${nuke-ms_BINARY_DIR}/common)
include_directories(${nuke-ms_SOURCE_DIR}/common)


if(WIN32)
	# silence win32 warnings when compiling Boost.Asio
	# Compile the Unicode version (non-win32 cmake is not smart enough)
	add_definitions(-D_WIN32_WINNT=0x0501 -D_UNICODE)
	
	# add network libraries that need to be linked
	set(WIN32_NETWORK_LIBS ws2_32 mswsock)
	
	# add hack to prevent multiple inclusion of WinSock.h by Boost and wxWidgets
	if(MSVC)
		add_definitions(-DWIN32_LEAN_AND_MEAN)
	endif()
endif()

# compile nuke-ms-common first
add_subdirectory(common)
add_subdirectory(client)
add_subdirectory(server)
