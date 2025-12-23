cmake_minimum_required(VERSION 3.13)

project(sandbox)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/../cmake_files/")

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
add_library(lib_font STATIC ../libs/font.cpp ../libs/font.h)
add_library(lib_geom STATIC ../libs/geom.cpp ../libs/geom.h)
add_library(lib_geom_2d STATIC ../libs/geom_2d.cpp ../libs/geom_2d.h)
add_library(lib_polygon_2d STATIC ../libs/polygon_2d.cpp ../libs/polygon_2d.h)
add_library(lib_convex_hull STATIC ../libs/convex_hull.cpp ../libs/convex_hull.h)
add_library(lib_triangulation STATIC ../libs/triangulation.cpp ../libs/triangulation.h)
add_library(lib_elements STATIC ../libs/elements.cpp ../libs/elements.h)
add_library(lib_elements_gl STATIC ./srcs/elements_gl.cpp ./srcs/elements_gl.h)

add_executable(main ./srcs/main.cpp)

set(EXT_LIBS ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${OPENGL_LIBRARIES} ${FREETYPE_LIBRARIES})
set(LOCAL_LIBS lib_repere lib_gl_utils lib_utile lib_input_state lib_bbox lib_bbox_2d lib_font lib_geom lib_geom_2d lib_polygon_2d lib_convex_hull lib_triangulation lib_elements lib_elements_gl)
 
target_link_libraries(main PUBLIC ${EXT_LIBS})
target_link_libraries(main PUBLIC ${LOCAL_LIBS})
