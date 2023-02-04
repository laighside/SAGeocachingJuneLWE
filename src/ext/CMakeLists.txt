cmake_minimum_required(VERSION 3.10)

IF(NOT JLWE_MAIN_CMAKELISTS_READ)
  MESSAGE(FATAL_ERROR "Run cmake on the CMakeLists.txt in the project root, not the one in the sub-directories. You will need to delete CMakeCache.txt from the current directory.")
ENDIF(NOT JLWE_MAIN_CMAKELISTS_READ)

add_library(md4c STATIC md4c/md4c.c md4c/md4c-entity.c md4c/md4c-html.c)
add_library(csprng STATIC csprng/csprng.cpp)
add_library(cryptblowfish STATIC crypt_blowfish/crypt_blowfish.c crypt_blowfish/crypt_gensalt.c crypt_blowfish/wrapper.c)
add_library(pugixml STATIC pugixml/pugixml.cpp)
add_library(hash_library STATIC hash_library/sha256.cpp)

# OpenXLSX uses cmake so we just add that directory to the project
add_subdirectory(OpenXLSX)
