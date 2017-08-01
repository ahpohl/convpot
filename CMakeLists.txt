cmake_minimum_required (VERSION 3.6.1)
set (CMAKE_CXX_STANDARD 11)
project (convpot)

# bring the headers into the project
include_directories (include)

# the file(GLOB...) allows for wildcard additions:
file (GLOB SOURCES "src/*.cpp")

# find odbc library    
set (ODBC_LIB odbc32)
link_directories (odbc)

# build sqlite static library
include_directories (sqlite)
link_directories (sqlite)

# link static libraries
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
SET(CMAKE_EXE_LINKER_FLAGS "-static")
SET(BUILD_SHARED_LIBRARIES OFF)

# build and link
add_executable (convpot ${SOURCES})
target_link_libraries (convpot m sqlite3 ${ODBC_LIB})
