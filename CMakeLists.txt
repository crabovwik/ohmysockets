cmake_minimum_required(VERSION 3.16)
project(ohmysockets C)

set(CMAKE_C_STANDARD 11)

#add_executable(ohmysockets main.c)
add_subdirectory(client)
add_subdirectory(server)