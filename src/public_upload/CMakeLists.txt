cmake_minimum_required(VERSION 3.10)

IF(NOT JLWE_MAIN_CMAKELISTS_READ)
  MESSAGE(FATAL_ERROR "Run cmake on the CMakeLists.txt in the project root, not the one in the sub-directories. You will need to delete CMakeCache.txt from the current directory.")
ENDIF(NOT JLWE_MAIN_CMAKELISTS_READ)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CGI_BIN_DIR}/public_upload")

add_library(GoogleAuthToken STATIC GoogleAuthToken.cpp)
target_link_libraries(GoogleAuthToken PRIVATE OpenSSL::Crypto)

add_executable(list_public_upload.cgi list_public_upload.cpp)
target_link_libraries(list_public_upload.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(gd_list_files.cgi gd_list_files.cpp)
target_link_libraries(gd_list_files.cgi jlwecore ${MYSQLCPPCONN_LIBRARY} httprequest GoogleAuthToken)

add_executable(gd_upload_file.cgi gd_upload_file.cpp)
target_link_libraries(gd_upload_file.cgi jlwecore ${MYSQLCPPCONN_LIBRARY} httprequest GoogleAuthToken)

add_executable(edit_image.cgi edit_image.cpp ImageUtils.cpp)
target_link_libraries(edit_image.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

