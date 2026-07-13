cmake_minimum_required(VERSION 3.13)

project(test_obj_anim)

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

# glm a besoin d'un c++ >= 14
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake_files")
# celui-ci ne fonctionne pas correctement
#list(APPEND CMAKE_PREFIX_PATH "/Volumes/Data/usr_local/lib/cmake/SDL2_image")

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)


include_directories(/usr/local/include)
include_directories(../libs)
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2IMAGE_INCLUDE_DIRS})

add_library(lib_repere STATIC ../libs/repere.cpp ../libs/repere.h)
add_library(lib_gl_utils STATIC ../libs/gl_utils.cpp ../libs/gl_utils.h)
add_library(lib_input_state STATIC ../libs/input_state.cpp ../libs/input_state.h)
add_library(lib_font STATIC ../libs/font.cpp ../libs/font.h)
add_library(lib_utile STATIC ../libs/utile.cpp ../libs/utile.h)
add_library(lib_gl_draw STATIC ../libs/gl_draw.cpp ../libs/gl_draw.h)
add_library(lib_fps_count STATIC ../libs/fps_count.cpp ../libs/fps_count.h)

add_library(lib_bbox_2d STATIC ../libs/bbox_2d.cpp ../libs/bbox_2d.h)
add_library(lib_polygon_2d STATIC ../libs/polygon_2d.cpp ../libs/polygon_2d.h)
add_library(lib_geom_2d STATIC ../libs/geom_2d.cpp ../libs/geom_2d.h)
add_library(lib_triangulation STATIC ../libs/triangulation.cpp ../libs/triangulation.h)
add_library(lib_bbox STATIC ../libs/bbox.cpp ../libs/bbox.h)
add_library(lib_geom STATIC ../libs/geom.cpp ../libs/geom.h)

add_library(lib_obj_parser STATIC ../libs/obj_parser.cpp ../libs/obj_parser.h)
add_library(lib_animated_obj STATIC ../libs/animated_obj.cpp ../libs/animated_obj.h)

add_library(lib_test_obj_anim STATIC ./srcs/test_obj_anim.cpp ./srcs/test_obj_anim.h)

add_executable(main ./srcs/main.cpp)

set(EXT_LIBS ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${OPENGL_LIBRARIES} ${FREETYPE_LIBRARIES})
set(GENERIC_LIBS lib_repere lib_gl_utils lib_input_state lib_font lib_utile lib_gl_draw lib_fps_count lib_bbox_2d lib_polygon_2d lib_geom_2d lib_bbox lib_geom lib_triangulation lib_obj_parser lib_animated_obj)
set(SPECIFIC_LIBS lib_test_obj_anim)
 
target_link_libraries(main PUBLIC ${EXT_LIBS})
target_link_libraries(main PUBLIC ${GENERIC_LIBS})
target_link_libraries(main PUBLIC ${SPECIFIC_LIBS})
