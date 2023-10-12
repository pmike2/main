cmake_minimum_required(VERSION 3.13)

project(sandbox)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(/usr/local/include)
include_directories(../libs)

#link_directories(/usr/local/lib)

add_library(lib_utile STATIC ../libs/utile.cpp ../libs/utile.h)

add_executable(main ./srcs/main.cpp ../libs/bst.h)

target_link_libraries(main PUBLIC lib_utile)
