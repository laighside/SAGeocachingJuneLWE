cmake_minimum_required(VERSION 3.10)

IF(NOT JLWE_MAIN_CMAKELISTS_READ)
  MESSAGE(FATAL_ERROR "Run cmake on the CMakeLists.txt in the project root, not the one in the sub-directories. You will need to delete CMakeCache.txt from the current directory.")
ENDIF(NOT JLWE_MAIN_CMAKELISTS_READ)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CGI_BIN_DIR}/mailing_list")

add_executable(mailing_list.cgi mailing_list.cpp)
target_link_libraries(mailing_list.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(write_email.cgi write_email.cpp)
target_link_libraries(write_email.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(send_email.cgi send_email.cpp)
target_link_libraries(send_email.cgi email jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(email_signup.cgi email_signup.cpp)
target_link_libraries(email_signup.cgi email jlwecore ${MYSQLCPPCONN_LIBRARY} httprequest)

add_executable(manage_subscription.cgi manage_subscription.cpp)
target_link_libraries(manage_subscription.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(resend_verify_email.cgi resend_verify_email.cpp)
target_link_libraries(resend_verify_email.cgi email jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(download_email_list.cgi download_email_list.cpp)
target_link_libraries(download_email_list.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

