# traditionnellement on fait :
#
# mkdir build && cd build
# cmake ..
# make
# ./main
#

cmake_minimum_required(VERSION 3.13)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


project(a_star)


include_directories(/usr/local/include)
include_directories(../libs)

add_library(lib_geom_2d STATIC ../libs/geom_2d.cpp ../libs/geom_2d.h)
add_library(lib_utile STATIC ../libs/utile.cpp ../libs/utile.h)

add_executable(main ./srcs/main.cpp)

target_link_libraries(main PUBLIC lib_geom_2d)
target_link_libraries(main PUBLIC lib_utile)

