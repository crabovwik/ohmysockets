cmake_minimum_required(VERSION 3.16)
project(server C)

set(CMAKE_C_STANDARD 11)

add_executable(server main.c main.h circular_buffer.c circular_buffer.h)

target_link_libraries(server helper network mbedtls pthread)
