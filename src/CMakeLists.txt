# CMakeLists.txt file for the nuke-ms source

# we need at lest cmake 2.6
cmake_minimum_required(VERSION 2.6)

# we are compiling nuke-ms, and it's C++-Code
project(nuke-ms)

# include endiannes testing macro
include(TestBigEndian)

# We need boost for all executeables
set(Boost_USE_MULTITHREADED ON)
find_package( Boost 1.39.0 REQUIRED thread system)

# Add Boost header and library directories
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

# All executables need the nuke-ms-common library
include_directories(${nuke-ms_SOURCE_DIR}/common)

# Include directories will be referenced from the top level
include_directories(${nuke-ms_SOURCE_DIR})

# If this is a big endian system, we must add a define
TEST_BIG_ENDIAN(NUKE_MS_BIG_ENDIAN)
if(NUKE_MS_BIG_ENDIAN)
    add_definitions(-DNUKE_MS_BIG_ENDIAN)
endif(NUKE_MS_BIG_ENDIAN)

# Copy all executables (and dll's on Win32) to bin/ 
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(WIN32)
    # Compile the Unicode version (non-win32 cmake is not smart enough)
    add_definitions(-D_UNICODE)

    # add network libraries that need to be linked
    set(WIN32_NETWORK_LIBS ws2_32 mswsock)
		
	# try to guess the value of _WIN32_WINNT macro
	# note for the future: should MS get to version numbers >= 10, this 
	# regex wont work anymore
	string(REGEX REPLACE 
		"([0123456789])\\.([0123456789])" "0x0\\10\\2" 
		WINNTDEF ${CMAKE_HOST_SYSTEM_VERSION})
	add_definitions(-D_WIN32_WINNT=${WINNTDEF})

    if(MSVC)
		# add hack to prevent multiple inclusion of WinSock.h by Boost and 
		# wxWidgets
        add_definitions(-DWIN32_LEAN_AND_MEAN)
		
		# Turn off boosts autolinking "feature", we do NOT need regex and 
		# date_time
		add_definitions(-DBOOST_ALL_NO_LIB)
		
		# Disable MSVC's paranoia about std::copy
		add_definitions(/wd4996)
		
		# building shared libs on MSVC is not allowed, because the code is not 
		# clad in dllexport definitions
		if (BUILD_SHARED_LIBS)
			set(BUILD_SHARED_LIBS OFF)
			message(WARNING "Building shared libraries with Visual Studio is not allowed! Building static libs instead.")
		endif(BUILD_SHARED_LIBS)
    endif(MSVC)	
endif(WIN32)

# compile nuke-ms-common first
add_subdirectory(common)
add_subdirectory(clientnode)
add_subdirectory(client-wx)
add_subdirectory(server)
add_subdirectory(test EXCLUDE_FROM_ALL)
