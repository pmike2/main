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


project(hpa)

find_package(GDAL REQUIRED)

include_directories(/usr/local/include)
include_directories(../libs)
include_directories(${GDAL_INCLUDE_DIRS})

add_library(lib_path_find_hierarchical STATIC ../libs/path_find_hierarchical.cpp ../libs/path_find_hierarchical.h)
add_library(lib_utile STATIC ../libs/utile.cpp ../libs/utile.h)
add_library(lib_geom_2d STATIC ../libs/geom_2d.cpp ../libs/geom_2d.h)
add_library(lib_shapefile STATIC ../libs/shapefile.cpp ../libs/shapefile.h)
add_library(lib_bbox_2d STATIC ../libs/bbox_2d.cpp ../libs/bbox_2d.h)

add_executable(main ./srcs/main.cpp)

target_link_libraries(main PUBLIC lib_path_find_hierarchical)
target_link_libraries(main PUBLIC lib_utile)
target_link_libraries(main PUBLIC lib_geom_2d)
target_link_libraries(main PUBLIC lib_shapefile)
target_link_libraries(main PUBLIC lib_bbox_2d)

target_link_libraries(main PUBLIC ${GDAL_LIBRARIES})
