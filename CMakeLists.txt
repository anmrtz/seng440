# Specify minimum required version of CMake.
cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall")

# Specify project and identify languages used.
project(color LANGUAGES CXX C)

#find_library(pigpio REQUIRED)

include_directories(include)

add_library(lodepng src/lodepng.cpp)
add_executable(cc-simple src/cc_simple.cpp)

target_link_libraries(cc-simple lodepng)
