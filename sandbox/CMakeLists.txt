cmake_minimum_required(VERSION 3.13)

project(sandbox)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(/usr/local/include)
include_directories(../libs)

link_directories(/usr/local/lib)

add_library(lib_mathematics STATIC ../libs/mathematics.cpp ../libs/mathematics.h)

add_executable(main ./srcs/main.cpp)

target_link_libraries(main PUBLIC lib_mathematics)
