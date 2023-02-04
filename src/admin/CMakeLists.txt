cmake_minimum_required(VERSION 3.10)

IF(NOT JLWE_MAIN_CMAKELISTS_READ)
  MESSAGE(FATAL_ERROR "Run cmake on the CMakeLists.txt in the project root, not the one in the sub-directories. You will need to delete CMakeCache.txt from the current directory.")
ENDIF(NOT JLWE_MAIN_CMAKELISTS_READ)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CGI_BIN_DIR}/admin")

add_executable(admin_tools.cgi admin_tools.cpp)
target_link_libraries(admin_tools.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(clear_table.cgi clear_table.cpp)
target_link_libraries(clear_table.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})




