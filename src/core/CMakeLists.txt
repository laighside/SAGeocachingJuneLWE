cmake_minimum_required(VERSION 3.10)

IF(NOT JLWE_MAIN_CMAKELISTS_READ)
  MESSAGE(FATAL_ERROR "Run cmake on the CMakeLists.txt in the project root, not the one in the sub-directories. You will need to delete CMakeCache.txt from the current directory.")
ENDIF(NOT JLWE_MAIN_CMAKELISTS_READ)

add_library(jlwecore STATIC CgiEnvironment.cpp Encoder.cpp FormElements.cpp HtmlTemplate.cpp JlweCore.cpp JlweUtils.cpp JsonUtils.cpp KeyValueParser.cpp PaymentUtils.cpp PostDataParser.cpp)
target_link_libraries(jlwecore csprng ${MAXMINDDB_LIBRARY})
IF(DEFINED CONFIG_FILE)
	target_compile_definitions(jlwecore PUBLIC CONFIG_FILE=\"${CONFIG_FILE}\")
ENDIF()

add_library(httprequest STATIC HttpRequest.cpp)
target_link_libraries(httprequest curl)
