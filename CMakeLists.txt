cmake_minimum_required (VERSION 3.6.1)
set (CMAKE_CXX_STANDARD 11)

project (convpot)

# bring the headers into the project
include_directories (include)
include_directories (sqlite/src)

# the file(GLOB...) allows for wildcard additions:
file (GLOB SOURCES "src/*.cpp")

# find odbc library
if (WIN32)
    set (ODBC_LIB odbc32)
    # path to windows odbc library
    link_directories (../odbc)
endif (WIN32) 

if (UNIX)
    set (ODBC_LIB odbc)
endif (UNIX)

# link static libraries
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
SET(CMAKE_EXE_LINKER_FLAGS "-static")
SET(BUILD_SHARED_LIBRARIES OFF)

# build and link
add_executable (convpot ${SOURCES})
target_link_libraries (convpot m sqlite3 ${ODBC_LIB})
