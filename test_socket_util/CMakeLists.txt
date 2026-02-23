
cmake_minimum_required(VERSION 3.13)

project(test_socket_util)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(/usr/local/include)
include_directories(../libs)

add_library(lib_socket_util STATIC ../libs/socket_util.cpp ../libs/socket_util.h)

add_executable(main ./srcs/main.cpp)

target_link_libraries(main PUBLIC lib_socket_util)
