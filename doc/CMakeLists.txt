# CMakeLists.txt file for the nuke-ms project

# we need at lest cmake 2.6
cmake_minimum_required(VERSION 2.6)

# look for doxygen package, but don't bother if not found
find_package(Doxygen)

if(DOXYGEN_FOUND)
    # feed source directory to doxygen
    set(DOXYGEN_INPUT_DIR ${PROJECT_SOURCE_DIR}/src)

    # output to html dir
    set(DOXYGEN_OUTPUT_DIR ${PROJECT_BINARY_DIR})

	# generate docs for private members if desired
	set (APIDOC_GEN_PRIVATE "$ENV{APIDOC_GEN_PRIVATE}")

    # transform Doxyfile.in to real Doxyfile with correct paths
    configure_file(Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # add apidoc target to generate Doxygen docu
    add_custom_target(apidoc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    )
endif(DOXYGEN_FOUND)

# thx to
# http://invalidmagic.wordpress.com/2009/11/30/cmake-and-doxygen-github-com/
# for hints on CMake+Doxygen
