cmake_minimum_required(VERSION 3.10)

IF(NOT JLWE_MAIN_CMAKELISTS_READ)
  MESSAGE(FATAL_ERROR "Run cmake on the CMakeLists.txt in the project root, not the one in the sub-directories. You will need to delete CMakeCache.txt from the current directory.")
ENDIF(NOT JLWE_MAIN_CMAKELISTS_READ)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CGI_BIN_DIR}/contact_form")

add_executable(contact_form.cgi contact_form.cpp)
target_link_libraries(contact_form.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(view_message.cgi view_message.cpp)
target_link_libraries(view_message.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(set_message_status.cgi set_message_status.cpp)
target_link_libraries(set_message_status.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

