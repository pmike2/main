cmake_minimum_required(VERSION 3.13)

project(sandbox)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/cmake/")

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)


include_directories(/usr/local/include)
include_directories(/usr/local/include/freetype2)
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
add_library(lib_triangulation STATIC ../libs/triangulation.cpp ../libs/triangulation.h)
add_library(lib_font STATIC ../libs/font.cpp ../libs/font.h)
add_library(lib_voro_z STATIC ../srcs/voro_z.cpp ../srcs/voro_z.h)

add_executable(main ./srcs/main.cpp)

set(EXT_LIBS ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${OPENGL_LIBRARIES} ${FREETYPE_LIBRARIES})
set(LOCAL_LIBS lib_repere lib_gl_utils lib_utile lib_input_state lib_bbox lib_bbox_2d lib_voronoi lib_dcel lib_geom_2d lib_triangulation lib_font lib_voro_z)
 
target_link_libraries(main PUBLIC ${EXT_LIBS})
target_link_libraries(main PUBLIC ${LOCAL_LIBS})
