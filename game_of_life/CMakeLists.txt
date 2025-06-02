cmake_minimum_required(VERSION 3.13)

project(gol)

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

add_library(lib_gl_utils STATIC ../libs/gl_utils.cpp ../libs/gl_utils.h)
add_library(lib_utile STATIC ../libs/utile.cpp ../libs/utile.h)
add_library(lib_input_state STATIC ../libs/input_state.cpp ../libs/input_state.h)
add_library(lib_bbox_2d STATIC ../libs/bbox_2d.cpp ../libs/bbox_2d.h)
add_library(lib_font STATIC ../libs/font.cpp ../libs/font.h)

add_library(lib_grid STATIC ../srcs/grid.cpp ../srcs/grid.h)
add_library(lib_gol STATIC ../srcs/gol.cpp ../srcs/gol.h)
add_library(lib_turmite STATIC ../srcs/turmite.cpp ../srcs/turmite.h)

add_executable(main_gol ./srcs/main_gol.cpp)
add_executable(main_turmite ./srcs/main_turmite.cpp)

set(EXT_LIBS ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${OPENGL_LIBRARIES} ${FREETYPE_LIBRARIES})
set(LOCAL_LIBS lib_gl_utils lib_utile lib_input_state lib_bbox_2d lib_font lib_grid)
 
target_link_libraries(main_gol PUBLIC ${EXT_LIBS})
target_link_libraries(main_gol PUBLIC ${LOCAL_LIBS})
target_link_libraries(main_gol PUBLIC lib_gol)

target_link_libraries(main_turmite PUBLIC ${EXT_LIBS})
target_link_libraries(main_turmite PUBLIC ${LOCAL_LIBS})
target_link_libraries(main_turmite PUBLIC lib_turmite)
