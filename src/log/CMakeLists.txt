cmake_minimum_required(VERSION 3.10)

IF(NOT JLWE_MAIN_CMAKELISTS_READ)
  MESSAGE(FATAL_ERROR "Run cmake on the CMakeLists.txt in the project root, not the one in the sub-directories. You will need to delete CMakeCache.txt from the current directory.")
ENDIF(NOT JLWE_MAIN_CMAKELISTS_READ)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CGI_BIN_DIR}/log")

add_executable(sendmail.cgi sendmail.cpp)
target_link_libraries(sendmail.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(user.cgi user.cpp)
target_link_libraries(user.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(server_status.cgi server_status.cpp)
target_link_libraries(server_status.cgi jlwecore ${MYSQLCPPCONN_LIBRARY} httprequest)

