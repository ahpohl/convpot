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
set (CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set (CMAKE_EXE_LINKER_FLAGS "-static")
set (BUILD_SHARED_LIBRARIES OFF)

# build and link
add_executable (convpot ${SOURCES})
target_link_libraries (convpot m sqlite3 ${ODBC_LIB})
install (TARGETS convpot RUNTIME DESTINATION "bin")

# install files
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set (CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set (RESOURCE_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/resources")
install (FILES "${RESOURCE_FOLDER}/convpot.bat" DESTINATION ".")
install (FILES ${CPACK_RESOURCE_FILE_LICENSE} DESTINATION "doc")
install (FILES ${CPACK_RESOURCE_FILE_README} DESTINATION "doc")
install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/CONTRIBUTORS.md" DESTINATION "doc")
install (DIRECTORY "${PROJECT_SOURCE_DIR}/test/" DESTINATION "examples")


# build a CPack driven installer package               
set (CPACK_GENERATOR "ZIP;NSIS")
set (CPACK_PACKAGE_VENDOR "Alexander Pohl")
set (CPACK_PACKAGE_VERSION_MAJOR "1")
set (CPACK_PACKAGE_VERSION_MINOR "0")
set (CPACK_PACKAGE_VERSION_PATCH "1")
set (CPACK_PACKAGE_INSTALL_DIRECTORY "Convpot")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Convert potentiostat data to a SQLite database")
set (CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set (CPACK_PACKAGE_ICON "${RESOURCE_FOLDER}\\\\convpot_installer.bmp")
set (CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} v${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set (CPACK_NSIS_MUI_ICON "${RESOURCE_FOLDER}/convpot.ico")
set (CPACK_NSIS_MUI_UNIICON "${RESOURCE_FOLDER}/convpot.ico")
set (CPACK_NSIS_CONTACT "alex@ahpohl.com")
set (CPACK_NSIS_HELP_LINK "https://github.com/ahpohl/convpot")
set (CPACK_NSIS_URL_INFO_ABOUT "https://github.com/ahpohl")
set (CPACK_NSIS_MENU_LINKS "convpot.bat" "Run Convpot"
                           "examples" "Examples"
                           "${CPACK_NSIS_HELP_LINK}" "GitHub page")
set (CPACK_NSIS_MODIFY_PATH ON)
                           
include (CPack)