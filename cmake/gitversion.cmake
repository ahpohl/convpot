#
# get git version from tag
#
execute_process (COMMAND git describe --tags --dirty OUTPUT_VARIABLE GIT_VERSION_FULL)
string (REGEX REPLACE "\n$" "" GIT_VERSION_FULL "${GIT_VERSION_FULL}")
string (REGEX REPLACE "^v([0-9]+)\\..*" "\\1" CPACK_PACKAGE_VERSION_MAJOR "${GIT_VERSION_FULL}")
string (REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" CPACK_PACKAGE_VERSION_MINOR "${GIT_VERSION_FULL}")
string (REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" CPACK_PACKAGE_VERSION_PATCH "${GIT_VERSION_FULL}")
set (GIT_VERSION_SHORT "v${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

# set files and folders
set (RESOURCE_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/resources")
set (VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/version.cpp") 

# configure version.cpp
configure_file (${RESOURCE_FOLDER}/version.cpp.in ${CMAKE_BINARY_DIR}/version.cpp)
