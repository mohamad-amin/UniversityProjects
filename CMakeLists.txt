cmake_minimum_required(VERSION 3.3)
project(University_Projects)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
        "Principles Of Programming/Compiler/main.c" "Principles Of Programming/Compiler/IRGenerator.c" "Principles Of Programming/Compiler/ir_test.cpp" "Principles Of Programming/Compiler/ir_test.h")

add_executable(University_Projects ${SOURCE_FILES})