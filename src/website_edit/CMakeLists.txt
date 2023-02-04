cmake_minimum_required(VERSION 3.10)

IF(NOT JLWE_MAIN_CMAKELISTS_READ)
  MESSAGE(FATAL_ERROR "Run cmake on the CMakeLists.txt in the project root, not the one in the sub-directories. You will need to delete CMakeCache.txt from the current directory.")
ENDIF(NOT JLWE_MAIN_CMAKELISTS_READ)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CGI_BIN_DIR}/website_edit")

add_executable(website_edit.cgi website_edit.cpp)
target_link_libraries(website_edit.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(get_webpage_json.cgi get_webpage_json.cpp)
target_link_libraries(get_webpage_json.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(set_webpage_json.cgi set_webpage_json.cpp)
target_link_libraries(set_webpage_json.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(upload_image.cgi upload_image.cpp)
target_link_libraries(upload_image.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(delete_image.cgi delete_image.cpp)
target_link_libraries(delete_image.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

