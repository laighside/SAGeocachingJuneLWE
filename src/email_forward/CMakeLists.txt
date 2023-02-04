cmake_minimum_required(VERSION 3.10)

IF(NOT JLWE_MAIN_CMAKELISTS_READ)
  MESSAGE(FATAL_ERROR "Run cmake on the CMakeLists.txt in the project root, not the one in the sub-directories. You will need to delete CMakeCache.txt from the current directory.")
ENDIF(NOT JLWE_MAIN_CMAKELISTS_READ)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CGI_BIN_DIR}/email_forward")

add_executable(email_forward.cgi email_forward.cpp)
target_link_libraries(email_forward.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(set_email_forwarder.cgi set_email_forwarder.cpp)
target_link_libraries(set_email_forwarder.cgi email jlwecore ${MYSQLCPPCONN_LIBRARY})

