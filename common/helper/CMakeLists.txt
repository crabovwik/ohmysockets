cmake_minimum_required(VERSION 3.16)
project(helper C)

set(CMAKE_C_STANDARD 11)

add_library(helper STATIC helper.c helper.h)

target_include_directories(helper PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
