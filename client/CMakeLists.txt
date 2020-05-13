cmake_minimum_required(VERSION 3.16)
project(client C)

set(CMAKE_C_STANDARD 11)

add_executable(client main.c helper.c helper.h)

add_subdirectory(lib/mbedtls-2.16.6)
target_link_libraries(client mbedtls)
