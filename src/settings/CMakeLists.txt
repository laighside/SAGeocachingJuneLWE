cmake_minimum_required(VERSION 3.10)

IF(NOT JLWE_MAIN_CMAKELISTS_READ)
  MESSAGE(FATAL_ERROR "Run cmake on the CMakeLists.txt in the project root, not the one in the sub-directories. You will need to delete CMakeCache.txt from the current directory.")
ENDIF(NOT JLWE_MAIN_CMAKELISTS_READ)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CGI_BIN_DIR}/settings")

add_executable(settings.cgi settings.cpp)
target_link_libraries(settings.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(set_variable.cgi set_variable.cpp)
target_link_libraries(set_variable.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(set_zone_points.cgi set_zone_points.cpp)
target_link_libraries(set_zone_points.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

