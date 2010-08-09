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
link_directories(${nuke-ms_BINARY_DIR}/common)
include_directories(${nuke-ms_SOURCE_DIR}/common)

# If this is a big endian system, we must add a define
TEST_BIG_ENDIAN(NUKE_MS_BIG_ENDIAN)
if(NUKE_MS_BIG_ENDIAN)
    add_definitions(-DNUKE_MS_BIG_ENDIAN)
endif()


if(WIN32)
    # Compile the Unicode version (non-win32 cmake is not smart enough)
    add_definitions(-D_UNICODE)

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
add_subdirectory(test EXCLUDE_FROM_ALL)
