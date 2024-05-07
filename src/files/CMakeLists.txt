cmake_minimum_required(VERSION 3.10)

IF(NOT JLWE_MAIN_CMAKELISTS_READ)
  MESSAGE(FATAL_ERROR "Run cmake on the CMakeLists.txt in the project root, not the one in the sub-directories. You will need to delete CMakeCache.txt from the current directory.")
ENDIF(NOT JLWE_MAIN_CMAKELISTS_READ)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CGI_BIN_DIR}/files")

add_executable(files.cgi files.cpp)
target_link_libraries(files.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(file_api.cgi file_api.cpp)
target_link_libraries(file_api.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(thumbnail.cgi thumbnail.cpp)
target_link_libraries(thumbnail.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(upload_file.cgi upload_file.cpp)
target_link_libraries(upload_file.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(public_upload.cgi public_upload.cpp)
target_link_libraries(public_upload.cgi jlwecore ${MYSQLCPPCONN_LIBRARY} httprequest)

