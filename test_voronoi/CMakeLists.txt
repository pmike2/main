
cmake_minimum_required(VERSION 3.13)

project(test_voronoi)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# pour ne pas avoir les warnings de opengl deprecated
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

include_directories(/usr/local/include)
include_directories(../libs)

add_library(lib_utile STATIC ../libs/utile.cpp ../libs/utile.h)
add_library(lib_dcel STATIC ../libs/dcel.cpp ../libs/dcel.h)
add_library(lib_voronoi STATIC ../libs/voronoi.cpp ../libs/voronoi.h)
add_library(lib_geom_2d STATIC ../libs/geom_2d.cpp ../libs/geom_2d.h)
add_library(lib_bbox_2d STATIC ../libs/bbox_2d.cpp ../libs/bbox_2d.h)
add_library(lib_triangulation STATIC ../libs/triangulation.cpp ../libs/triangulation.h)

add_executable(main ./srcs/main.cpp)

target_link_libraries(main PUBLIC lib_utile)
target_link_libraries(main PUBLIC lib_dcel)
target_link_libraries(main PUBLIC lib_voronoi)
target_link_libraries(main PUBLIC lib_geom_2d)
target_link_libraries(main PUBLIC lib_bbox_2d)
target_link_libraries(main PUBLIC lib_triangulation)
