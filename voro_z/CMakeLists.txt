cmake_minimum_required(VERSION 3.13)

project(sandbox)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/cmake/")

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(OpenGL REQUIRED)


include_directories(/usr/local/include)
include_directories(../libs)

add_library(lib_repere STATIC ../libs/repere.cpp ../libs/repere.h)
add_library(lib_gl_utils STATIC ../libs/gl_utils.cpp ../libs/gl_utils.h)
add_library(lib_utile STATIC ../libs/utile.cpp ../libs/utile.h)
add_library(lib_input_state STATIC ../libs/input_state.cpp ../libs/input_state.h)
add_library(lib_bbox STATIC ../libs/bbox.cpp ../libs/bbox.h)
add_library(lib_bbox_2d STATIC ../libs/bbox_2d.cpp ../libs/bbox_2d.h)
add_library(lib_voronoi STATIC ../libs/voronoi.cpp ../libs/voronoi.h)
add_library(lib_dcel STATIC ../libs/dcel.cpp ../libs/dcel.h)
add_library(lib_geom_2d STATIC ../libs/geom_2d.cpp ../libs/geom_2d.h)
add_library(lib_voro_z STATIC ../srcs/voro_z.cpp ../srcs/voro_z.h)

add_executable(main ./srcs/main.cpp)
 
target_link_libraries(main PUBLIC lib_repere)
target_link_libraries(main PUBLIC lib_gl_utils)
target_link_libraries(main PUBLIC lib_utile)
target_link_libraries(main PUBLIC lib_input_state)
target_link_libraries(main PUBLIC lib_bbox)
target_link_libraries(main PUBLIC lib_bbox_2d)
target_link_libraries(main PUBLIC lib_voronoi)
target_link_libraries(main PUBLIC lib_dcel)
target_link_libraries(main PUBLIC lib_geom_2d)
target_link_libraries(main PUBLIC lib_voro_z)

target_link_libraries(main PUBLIC ${SDL2_LIBRARIES})
target_link_libraries(main PUBLIC ${SDL2_IMAGE_LIBRARIES})
target_link_libraries(main PUBLIC ${OPENGL_LIBRARIES})
