cmake_minimum_required(VERSION 3.10)

IF(NOT JLWE_MAIN_CMAKELISTS_READ)
  MESSAGE(FATAL_ERROR "Run cmake on the CMakeLists.txt in the project root, not the one in the sub-directories. You will need to delete CMakeCache.txt from the current directory.")
ENDIF(NOT JLWE_MAIN_CMAKELISTS_READ)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CGI_BIN_DIR}/registration")

add_executable(registration.cgi registration.cpp)
target_link_libraries(registration.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(registration_form.cgi registration_form.cpp)
target_link_libraries(registration_form.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(submit_reg.cgi submit_reg.cpp)
target_link_libraries(submit_reg.cgi email jlwecore ${MYSQLCPPCONN_LIBRARY} httprequest)

add_executable(confirmation_reg.cgi confirmation_reg.cpp)
target_link_libraries(confirmation_reg.cgi email jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(add_payments.cgi add_payments.cpp)
target_link_libraries(add_payments.cgi email jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(get_details.cgi get_details.cpp)
target_link_libraries(get_details.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(get_form_content.cgi get_form_content.cpp)
target_link_libraries(get_form_content.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(payment_history.cgi payment_history.cpp)
target_link_libraries(payment_history.cgi jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(payment_reminder_email.cgi payment_reminder_email.cpp)
target_link_libraries(payment_reminder_email.cgi email jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(registration_reminder_email.cgi registration_reminder_email.cpp)
target_link_libraries(registration_reminder_email.cgi email jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(cancel.cgi cancel.cpp)
target_link_libraries(cancel.cgi email jlwecore ${MYSQLCPPCONN_LIBRARY})

add_executable(download_event_registrations.cgi download_event_registrations.cpp RegistrationXLS.cpp)
target_link_libraries(download_event_registrations.cgi jlwecore ${MYSQLCPPCONN_LIBRARY} OpenXLSX::OpenXLSX pugixml)

add_executable(download_dinner_orders.cgi download_dinner_orders.cpp DinnerOrderXLS.cpp)
target_link_libraries(download_dinner_orders.cgi jlwecore ${MYSQLCPPCONN_LIBRARY} OpenXLSX::OpenXLSX)

