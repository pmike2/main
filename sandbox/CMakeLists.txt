cmake_minimum_required(VERSION 3.13)

project(sandbox)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(/usr/local/include)
include_directories(../libs)

add_library(lib_3body STATIC ../srcs/threebody.h ../srcs/threebody.cpp)

add_executable(main ./srcs/main.cpp)
 
target_link_libraries(main PUBLIC lib_3body)
