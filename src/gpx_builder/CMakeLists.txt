cmake_minimum_required(VERSION 3.10)

IF(NOT JLWE_MAIN_CMAKELISTS_READ)
  MESSAGE(FATAL_ERROR "Run cmake on the CMakeLists.txt in the project root, not the one in the sub-directories. You will need to delete CMakeCache.txt from the current directory.")
ENDIF(NOT JLWE_MAIN_CMAKELISTS_READ)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CGI_BIN_DIR}/gpx_builder")

add_executable(gpx_builder.cgi gpx_builder.cpp)
target_link_libraries(gpx_builder.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(builder_map.cgi builder_map.cpp)
target_link_libraries(builder_map.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(download_gpx.cgi download_gpx.cpp)
target_link_libraries(download_gpx.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(download_cache_list.cgi download_cache_list.cpp WriteCacheListDOCX.cpp ../ooxml/WriteDOCX.cpp)
target_link_libraries(download_cache_list.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(download_cache_photos.cgi download_cache_photos.cpp WriteCachePhotosDOCX.cpp ../ooxml/WriteDOCX.cpp)
target_link_libraries(download_cache_photos.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(delete_cache.cgi delete_cache.cpp)
target_link_libraries(delete_cache.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(edit_caches.cgi edit_caches.cpp)
target_link_libraries(edit_caches.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(get_cache.cgi get_cache.cpp)
target_link_libraries(get_cache.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(set_cache.cgi set_cache.cpp)
target_link_libraries(set_cache.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(set_handout_cache.cgi set_handout_cache.cpp)
target_link_libraries(set_handout_cache.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})


