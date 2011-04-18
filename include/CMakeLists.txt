# CMakeLists.txt file for the testing directory.
# Should not be called directly, use parent level cmake file in project
# directory instead.


# install headers to include directory
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION "include/${PROJECT_NAME}"
    FILES_MATCHING PATTERN "*.hpp"
)
