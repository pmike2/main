cmake_minimum_required(VERSION 3.13)

project(asteroid)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/cmake/")

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
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

add_library(lib_asteroid STATIC ../srcs/asteroid.cpp ../srcs/asteroid.h)
add_library(lib_ship STATIC ../srcs/ship.cpp ../srcs/ship.h)
add_library(lib_ship_model STATIC ../srcs/ship_model.cpp ../srcs/ship_model.h)
add_library(lib_level STATIC ../srcs/level.cpp ../srcs/level.h)
add_library(lib_star STATIC ../srcs/star.cpp ../srcs/star.h)

add_executable(main ./srcs/main.cpp)
 
target_link_libraries(main PUBLIC lib_repere)
target_link_libraries(main PUBLIC lib_gl_utils)
target_link_libraries(main PUBLIC lib_utile)
target_link_libraries(main PUBLIC lib_input_state)
target_link_libraries(main PUBLIC lib_bbox)
target_link_libraries(main PUBLIC lib_bbox_2d)
target_link_libraries(main PUBLIC lib_font)

target_link_libraries(main PUBLIC lib_asteroid)
target_link_libraries(main PUBLIC lib_ship)
target_link_libraries(main PUBLIC lib_ship_model)
target_link_libraries(main PUBLIC lib_level)
target_link_libraries(main PUBLIC lib_star)

target_link_libraries(main PUBLIC ${SDL2_LIBRARIES})
target_link_libraries(main PUBLIC ${SDL2_IMAGE_LIBRARIES})
target_link_libraries(main PUBLIC ${SDL2_MIXER_LIBRARIES})
target_link_libraries(main PUBLIC ${OPENGL_LIBRARIES})
target_link_libraries(main PUBLIC ${FREETYPE_LIBRARIES})

