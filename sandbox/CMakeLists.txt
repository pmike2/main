cmake_minimum_required(VERSION 3.13)

project(sandbox)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

include_directories(/usr/local/include)
include_directories(../libs)

add_library(lib_utile STATIC ../libs/utile.cpp ../libs/utile.h)
add_library(lib_bbox STATIC ../libs/bbox.cpp ../libs/bbox.h)
add_library(lib_bbox_2d STATIC ../libs/bbox_2d.cpp ../libs/bbox_2d.h)
add_library(lib_geom STATIC ../libs/geom.cpp ../libs/geom.h)
add_library(lib_geom_2d STATIC ../libs/geom_2d.cpp ../libs/geom_2d.h)
add_library(lib_polygon_2d STATIC ../libs/polygon_2d.cpp ../libs/polygon_2d.h)
add_library(lib_triangulation STATIC ../libs/triangulation.cpp ../libs/triangulation.h)

add_executable(main ./srcs/main.cpp)

set(LOCAL_LIBS lib_utile lib_bbox lib_bbox_2d lib_geom lib_geom_2d lib_polygon_2d lib_triangulation)
 
target_link_libraries(main PUBLIC ${LOCAL_LIBS})
