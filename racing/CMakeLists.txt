cmake_minimum_required(VERSION 3.13)

project(racing)

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
add_library(lib_geom_2d STATIC ../libs/geom_2d.cpp ../libs/geom_2d.h)
add_library(lib_triangulation STATIC ../libs/triangulation.cpp ../libs/triangulation.h)
add_library(lib_font STATIC ../libs/font.cpp ../libs/font.h)

add_library(lib_racing STATIC ../srcs/racing.cpp ../srcs/racing.h)
add_library(lib_car STATIC ../srcs/car.cpp ../srcs/car.h)
add_library(lib_static_object STATIC ../srcs/static_object.cpp ../srcs/static_object.h)
add_library(lib_track STATIC ../srcs/track.cpp ../srcs/track.h)
add_library(lib_track_editor STATIC ../srcs/track_editor.cpp ../srcs/track_editor.h)
add_library(lib_smoke STATIC ../srcs/smoke.cpp ../srcs/smoke.h)
add_library(lib_tire_track STATIC ../srcs/tire_track.cpp ../srcs/tire_track.h)
add_library(lib_material STATIC ../srcs/material.cpp ../srcs/material.h)
add_library(lib_collision STATIC ../srcs/collision.cpp ../srcs/collision.h)
add_library(lib_spark STATIC ../srcs/spark.cpp ../srcs/spark.h)
add_library(lib_driver STATIC ../srcs/driver.cpp ../srcs/driver.h)
add_library(lib_action STATIC ../srcs/action.cpp ../srcs/action.h)
add_library(lib_grid STATIC ../srcs/grid.cpp ../srcs/grid.h)
add_library(lib_water STATIC ../srcs/water.cpp ../srcs/water.h)

add_executable(main ./srcs/main.cpp)
add_executable(editor ./srcs/editor.cpp)

set(EXT_LIBS ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_MIXER_LIBRARIES} ${OPENGL_LIBRARIES} ${FREETYPE_LIBRARIES} sioclient)
set(LOCAL_COMMON_LIBS lib_repere lib_gl_utils lib_utile lib_input_state lib_bbox lib_bbox_2d lib_geom_2d lib_triangulation 
	lib_font lib_track lib_static_object lib_car lib_material lib_collision lib_driver lib_action lib_grid)
set(LOCAL_MAIN_LIBS lib_racing lib_smoke lib_tire_track lib_spark lib_water)
set(LOCAL_EDITOR_LIBS lib_track_editor)
 
target_link_libraries(main PUBLIC ${EXT_LIBS})
target_link_libraries(main PUBLIC ${LOCAL_COMMON_LIBS})
target_link_libraries(main PUBLIC ${LOCAL_MAIN_LIBS})

target_link_libraries(editor PUBLIC ${EXT_LIBS})
target_link_libraries(editor PUBLIC ${LOCAL_COMMON_LIBS})
target_link_libraries(editor PUBLIC ${LOCAL_EDITOR_LIBS})
