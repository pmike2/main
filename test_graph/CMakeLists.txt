cmake_minimum_required(VERSION 3.13)

project(test_graph)

if (CMAKE_HOST_SYSTEM_VERSION VERSION_LESS 19)
	message("Mojave")
	set(CMAKE_CXX_COMPILER "/opt/local/bin/g++-mp-12")
	include_directories(/usr/local/include/freetype2)
else()
	message("Sonoma")
	list(APPEND CMAKE_PREFIX_PATH "/Volumes/Data/usr_local/lib/cmake/SDL2")
	include_directories(/Volumes/Data/usr_local/include)
	include_directories(/Volumes/Data/usr_local/include/freetype2)
	include_directories(/Volumes/Data/usr_local/include/nlohmann)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

find_package(GDAL REQUIRED)

include_directories(/usr/local/include)
include_directories(../libs)
include_directories(${GDAL_INCLUDE_DIRS})

add_library(lib_graph STATIC ../libs/graph.cpp ../libs/graph.h)
add_library(lib_utile STATIC ../libs/utile.cpp ../libs/utile.h)
add_library(lib_geom_2d STATIC ../libs/geom_2d.cpp ../libs/geom_2d.h)
add_library(lib_polygon_2d STATIC ../libs/polygon_2d.cpp ../libs/polygon_2d.h)
add_library(lib_bbox_2d STATIC ../libs/bbox_2d.cpp ../libs/bbox_2d.h)
add_library(lib_triangulation STATIC ../libs/triangulation.cpp ../libs/triangulation.h)

add_executable(main ./srcs/main.cpp)

target_link_libraries(main PUBLIC ${GDAL_LIBRARIES} lib_graph lib_utile lib_geom_2d lib_polygon_2d lib_bbox_2d lib_triangulation)
