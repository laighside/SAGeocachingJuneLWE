cmake_minimum_required(VERSION 3.10)

# Disable in-source builds to prevent source tree corruption.
if(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "
FATAL: In-source builds are not allowed.
       You should create a separate directory for build files.
")
endif()

#
# Project name
#
project(JLWE)

# Builds must use this CMakeLists.txt, not the ones somewhere else.
# If users try to use something else the results can be confusing. We set a
# variable here that we require to be set elsewhere, otherwise we'll complain.
set(JLWE_MAIN_CMAKELISTS_READ TRUE)

# Use C++17
set(CMAKE_CXX_STANDARD 17)

if(DEFINED CGI_BIN_DIR)
	MESSAGE("-- cgi-bin directory defined to be ${CGI_BIN_DIR}")
else()
	set(CGI_BIN_DIR "${CMAKE_BINARY_DIR}/cgi-bin")
	MESSAGE("-- cgi-bin directory not defined, using ${CGI_BIN_DIR}")
endif()

if(DEFINED CONFIG_FILE)
	MESSAGE("-- config file location defined to be at ${CONFIG_FILE}")
else()
	MESSAGE("-- config file location not defined, using default location")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CGI_BIN_DIR})

# Don't allow CGI scripts to be installed
install(CODE "MESSAGE(FATAL_ERROR \"Install is disabled for CGI scripts.\")")

# Find MySQL C++ Connecter library
find_path(MYSQLCPPCONN_INCLUDE_DIR mysql_connection.h)
find_library(MYSQLCPPCONN_LIBRARY NAMES mysqlcppconn mysqlcppconn-static)

IF(NOT MYSQLCPPCONN_LIBRARY STREQUAL "MYSQLCPPCONN_LIBRARY-NOTFOUND")
	INCLUDE_DIRECTORIES(${MYSQLCPPCONN_INCLUDE_DIR})
	MESSAGE("-- MySQL C++ connector found, headers in ${MYSQLCPPCONN_INCLUDE_DIR}, library at ${MYSQLCPPCONN_LIBRARY}")
ELSE()
	MESSAGE(FATAL_ERROR "FATAL: MySQL C++ connector not found. This is required.")
ENDIF()

# Find curl
find_package(CURL REQUIRED)

# Find maxminddb
find_library(MAXMINDDB_LIBRARY NAMES maxminddb)
IF(NOT MAXMINDDB_LIBRARY STREQUAL "MAXMINDDB_LIBRARY-NOTFOUND")
	MESSAGE("-- Maxminddb library found at ${MAXMINDDB_LIBRARY}")
        add_compile_definitions(HAVE_MAXMINDDB)
ELSE()
	MESSAGE("-- Maxminddb library not found. Build will not include Geo IP features.")
        set(MAXMINDDB_LIBRARY "")
ENDIF()


add_subdirectory(ext)
add_subdirectory(core)
add_subdirectory(admin)
add_subdirectory(files)
add_subdirectory(email)
add_subdirectory(kml)
add_subdirectory(gpx_builder)
add_subdirectory(registration)
#add_subdirectory(merch)
add_subdirectory(notes)
add_subdirectory(website_edit)
add_subdirectory(mailing_list)
add_subdirectory(email_forward)
add_subdirectory(scoring)
add_subdirectory(settings)
add_subdirectory(password)
add_subdirectory(log)
add_subdirectory(users)
add_subdirectory(contact_form)

add_executable(jlwe.cgi jlwe.cpp)
target_link_libraries(jlwe.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(jlwe_year.cgi jlwe_year.cpp)
target_link_libraries(jlwe_year.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(jlwe_map.cgi jlwe_map.cpp)
target_link_libraries(jlwe_map.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(download_file.cgi download_file.cpp)
target_link_libraries(download_file.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(js_files.cgi js_files.cpp)
target_link_libraries(js_files.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(login.cgi login.cpp password/Password.cpp)
target_link_libraries(login.cgi jlwecore ${MYSQLCPPCONN_LIBRARY} cryptblowfish csprng)

add_executable(logout.cgi logout.cpp)
target_link_libraries(logout.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(admin_index.cgi admin_index.cpp)
target_link_libraries(admin_index.cgi jlwecore ${MYSQLCPPCONN_LIBRARY} md4c)

add_executable(gpx.cgi gpx.cpp)
target_link_libraries(gpx.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(contact.cgi contact.cpp)
target_link_libraries(contact.cgi email jlwecore ${MYSQLCPPCONN_LIBRARY} httprequest)

add_executable(stripe_keys.cgi stripe_keys.cpp)
target_link_libraries(stripe_keys.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(hide_cache.cgi hide_cache.cpp)
target_link_libraries(hide_cache.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(get_coord_info.cgi get_coord_info.cpp)
target_link_libraries(get_coord_info.cgi jlwecore ${MYSQLCPPCONN_LIBRARY} kml pugixml)

add_executable(stripe_webhook.cgi stripe_webhook.cpp)
target_link_libraries(stripe_webhook.cgi jlwecore ${MYSQLCPPCONN_LIBRARY} hash_library)

add_executable(csp_report.cgi csp_report.cpp)
target_link_libraries(csp_report.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

