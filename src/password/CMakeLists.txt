cmake_minimum_required(VERSION 3.10)

IF(NOT JLWE_MAIN_CMAKELISTS_READ)
  MESSAGE(FATAL_ERROR "Run cmake on the CMakeLists.txt in the project root, not the one in the sub-directories. You will need to delete CMakeCache.txt from the current directory.")
ENDIF(NOT JLWE_MAIN_CMAKELISTS_READ)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CGI_BIN_DIR}/password")

add_executable(change.cgi change.cpp Password.cpp)
target_link_libraries(change.cgi jlwecore ${MYSQLCPPCONN_LIBRARY} cryptblowfish csprng)

add_executable(reset.cgi reset.cpp Password.cpp)
target_link_libraries(reset.cgi jlwecore ${MYSQLCPPCONN_LIBRARY} cryptblowfish csprng)

add_executable(send_reset.cgi send_reset.cpp)
target_link_libraries(send_reset.cgi email jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(send_reset_email.cgi send_reset_email.cpp)
target_link_libraries(send_reset_email.cgi email jlwecore ${MYSQLCPPCONN_LIBRARY})
