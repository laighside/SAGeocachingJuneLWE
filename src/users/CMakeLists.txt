cmake_minimum_required(VERSION 3.10)

IF(NOT JLWE_MAIN_CMAKELISTS_READ)
  MESSAGE(FATAL_ERROR "Run cmake on the CMakeLists.txt in the project root, not the one in the sub-directories. You will need to delete CMakeCache.txt from the current directory.")
ENDIF(NOT JLWE_MAIN_CMAKELISTS_READ)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CGI_BIN_DIR}/users")

add_executable(users.cgi users.cpp)
target_link_libraries(users.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(set_user_perm.cgi set_user_perm.cpp)
target_link_libraries(set_user_perm.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(set_user_preferences.cgi set_user_preferences.cpp)
target_link_libraries(set_user_preferences.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

