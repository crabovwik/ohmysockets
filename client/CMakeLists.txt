cmake_minimum_required(VERSION 3.16)
project(client C)

set(CMAKE_C_STANDARD 11)

add_executable(client main.c ../common/network/network.c ../common/network/network.h)

target_link_libraries(client helper network mbedtls)
