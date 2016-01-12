cmake_minimum_required(VERSION 3.3)
project(University_Projects)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    "Principles Of Programming/Compiler/main.c")

add_executable(University_Projects ${SOURCE_FILES})