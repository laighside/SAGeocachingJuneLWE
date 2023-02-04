cmake_minimum_required(VERSION 3.10)

IF(NOT JLWE_MAIN_CMAKELISTS_READ)
  MESSAGE(FATAL_ERROR "Run cmake on the CMakeLists.txt in the project root, not the one in the sub-directories. You will need to delete CMakeCache.txt from the current directory.")
ENDIF(NOT JLWE_MAIN_CMAKELISTS_READ)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CGI_BIN_DIR}/notes")

add_executable(notes.cgi notes.cpp)
target_link_libraries(notes.cgi jlwecore md4c ${MYSQLCPPCONN_LIBRARY})

add_executable(save_notes.cgi save_notes.cpp)
target_link_libraries(save_notes.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

