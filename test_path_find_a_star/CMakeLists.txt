# traditionnellement on fait :
#
# mkdir build && cd build
# cmake ..
# make
# ./main
#

cmake_minimum_required(VERSION 3.13)

project(a_star)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

find_package(GDAL REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(/usr/local/include)
include_directories(../libs)
include_directories(${GDAL_INCLUDE_DIRS})

add_library(lib_path_find STATIC ../libs/path_find.cpp ../libs/path_find.h)
add_library(lib_utile STATIC ../libs/utile.cpp ../libs/utile.h)
add_library(lib_geom_2d STATIC ../libs/geom_2d.cpp ../libs/geom_2d.h)
add_library(lib_shapefile STATIC ../libs/shapefile.cpp ../libs/shapefile.h)
add_library(lib_bbox_2d STATIC ../libs/bbox_2d.cpp ../libs/bbox_2d.h)
add_library(lib_graph STATIC ../libs/graph.cpp ../libs/graph.h)
add_library(lib_triangulation STATIC ../libs/triangulation.cpp ../libs/triangulation.h)


add_executable(main ./srcs/main.cpp)

target_link_libraries(main PUBLIC ${GDAL_LIBRARIES} ${OPENGL_LIBRARIES} lib_path_find lib_utile lib_geom_2d lib_shapefile lib_bbox_2d lib_graph lib_triangulation)
