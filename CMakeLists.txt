cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Werror")

project(cc LANGUAGES C CXX)

include_directories(include)

add_library(lodepng src/lodepng.cpp)

add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/cc/cc_naive.c
    )

target_link_libraries(${PROJECT_NAME} lodepng)
