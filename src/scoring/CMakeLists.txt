cmake_minimum_required(VERSION 3.10)

IF(NOT JLWE_MAIN_CMAKELISTS_READ)
  MESSAGE(FATAL_ERROR "Run cmake on the CMakeLists.txt in the project root, not the one in the sub-directories. You will need to delete CMakeCache.txt from the current directory.")
ENDIF(NOT JLWE_MAIN_CMAKELISTS_READ)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CGI_BIN_DIR}/scoring")


add_library(powerpoint STATIC PowerPoint.cpp)
add_library(point_calculator STATIC PointCalculator.cpp)

add_executable(scoring.cgi scoring.cpp)
target_link_libraries(scoring.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(download_ppt.cgi download_ppt.cpp)
target_link_libraries(download_ppt.cgi jlwecore powerpoint ${MYSQLCPPCONN_LIBRARY})

add_executable(download_scoring_xlsx.cgi download_scoring_xlsx.cpp WriteScoringXLSX.cpp)
target_link_libraries(download_scoring_xlsx.cgi jlwecore point_calculator ${MYSQLCPPCONN_LIBRARY})

add_executable(upload_scoring_xlsx.cgi upload_scoring_xlsx.cpp)
target_link_libraries(upload_scoring_xlsx.cgi jlwecore ${MYSQLCPPCONN_LIBRARY} point_calculator OpenXLSX::OpenXLSX)

add_executable(set_best_cache.cgi set_best_cache.cpp)
target_link_libraries(set_best_cache.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(auto_team_import.cgi auto_team_import.cpp)
target_link_libraries(auto_team_import.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(save_team_info.cgi save_team_info.cpp)
target_link_libraries(save_team_info.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(get_scores.cgi get_scores.cpp)
target_link_libraries(get_scores.cgi jlwecore point_calculator ${MYSQLCPPCONN_LIBRARY})

add_executable(get_slides.cgi get_slides.cpp)
target_link_libraries(get_slides.cgi jlwecore powerpoint ${MYSQLCPPCONN_LIBRARY})

add_executable(set_slide.cgi set_slide.cpp)
target_link_libraries(set_slide.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(get_points.cgi get_points.cpp)
target_link_libraries(get_points.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(set_points.cgi set_points.cpp)
target_link_libraries(set_points.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(set_find_value.cgi set_find_value.cpp)
target_link_libraries(set_find_value.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(save_scoring_data.cgi save_scoring_data.cpp)
target_link_libraries(save_scoring_data.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

