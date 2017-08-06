PROJECT(sqlite3)
cmake_minimum_required(VERSION 3.6.1)

include_directories (include)

add_library(sqlite3 STATIC src/sqlite3.c)
add_executable(sqlite src/sqlite3.c src/shell.c)
set_target_properties(sqlite PROPERTIES OUTPUT_NAME sqlite3)
install (TARGETS sqlite RUNTIME DESTINATION bin)

add_definitions(-DSQLITE_ENABLE_RTREE)
add_definitions(-DSQLITE_ENABLE_FTS4)
add_definitions(-DSQLITE_ENABLE_FTS5)
add_definitions(-DSQLITE_ENABLE_JSON1)