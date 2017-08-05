# Specify the minimum version for CMake
cmake_minimum_required (VERSION 3.6.1)

# Project's name
project (convpot)

# Require the compiler to support at least C++11
set (CMAKE_CXX_STANDARD 11)

# build sqlite library
add_subdirectory (sqlite)

# the file(GLOB...) allows for wildcard additions:
file (GLOB SOURCES "src/*.cpp")

# find odbc library    
set (ODBC_LIB odbc32)
link_directories (odbc)

# bring the headers into the project
include_directories (include)
include_directories (sqlite/include)

# find external libraries
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
SET(CMAKE_EXE_LINKER_FLAGS "-static")
SET(BUILD_SHARED_LIBRARIES OFF)

# build and link
add_executable (convpot ${SOURCES})
target_link_libraries (convpot m sqlite3 ${ODBC_LIB})

# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set (CPACK_GENERATOR "ZIP")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set (CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set (CPACK_PACKAGE_VERSION_MAJOR "1")
set (CPACK_PACKAGE_VERSION_MINOR "0")
include (CPack)

