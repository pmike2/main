cmake_minimum_required(VERSION 3.13)

project(test_dcel)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(/usr/local/include)
include_directories(../libs)

#add_library(lib_utile STATIC ../libs/utile.cpp ../libs/utile.h)
add_library(lib_dcel STATIC ../libs/dcel.cpp ../libs/dcel.h)

add_executable(main ./srcs/main.cpp)

#target_link_libraries(main PUBLIC lib_utile)
target_link_libraries(main PUBLIC lib_dcel)
