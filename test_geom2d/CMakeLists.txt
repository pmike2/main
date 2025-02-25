cmake_minimum_required(VERSION 3.13)

project(test_geom2d)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)

include_directories(/usr/local/include)
include_directories(/usr/local/include/freetype2)
include_directories(../libs)

add_library(lib_utile STATIC ../libs/utile.cpp ../libs/utile.h)
add_library(lib_geom_2d STATIC ../libs/geom_2d.cpp ../libs/geom_2d.h)
add_library(lib_triangulation STATIC ../libs/triangulation.cpp ../libs/triangulation.h)
add_library(lib_bbox_2d STATIC ../libs/bbox_2d.cpp ../libs/bbox_2d.h)
add_library(lib_gl_utils STATIC ../libs/gl_utils.cpp ../libs/gl_utils.h)
add_library(lib_input_state STATIC ../libs/input_state.cpp ../libs/input_state.h)
add_library(lib_font STATIC ../libs/font.cpp ../libs/font.h)
add_library(lib_test_geom_2d STATIC ../srcs/test_geom_2d.cpp ../srcs/test_geom_2d.h)

add_executable(main ./srcs/main.cpp)
add_executable(main_gl ./srcs/main_gl.cpp)

target_link_libraries(main PUBLIC lib_utile)
target_link_libraries(main PUBLIC lib_triangulation)
target_link_libraries(main PUBLIC lib_geom_2d)
target_link_libraries(main PUBLIC lib_bbox_2d)

target_link_libraries(main_gl PUBLIC lib_utile)
target_link_libraries(main_gl PUBLIC lib_triangulation)
target_link_libraries(main_gl PUBLIC lib_geom_2d)
target_link_libraries(main_gl PUBLIC lib_bbox_2d)
target_link_libraries(main_gl PUBLIC lib_gl_utils)
target_link_libraries(main_gl PUBLIC lib_input_state)
target_link_libraries(main_gl PUBLIC lib_font)
target_link_libraries(main_gl PUBLIC lib_test_geom_2d)
target_link_libraries(main_gl PUBLIC ${SDL2_LIBRARIES})
target_link_libraries(main_gl PUBLIC ${OPENGL_LIBRARIES})
target_link_libraries(main_gl PUBLIC ${FREETYPE_LIBRARIES})
