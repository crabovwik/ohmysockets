cmake_minimum_required(VERSION 3.16)
project(ohmysockets C)

set(CMAKE_C_STANDARD 11)

#include_directories(${CMAKE_SOURCE_DIR}/common/lib/mbedtls-2.16.6/build/include)
#add_subdirectory(common/lib/mbedtls-2.16.6)
add_subdirectory(client)
add_subdirectory(server)
