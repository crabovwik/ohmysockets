cmake_minimum_required(VERSION 3.16)
project(network C)

set(CMAKE_C_STANDARD 11)

add_library(network STATIC network.c)

target_include_directories(network PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(network helper mbedtls)
