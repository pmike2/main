
cmake_minimum_required(VERSION 3.13)

project(audio2video)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/../cmake_files/")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(LibSndFile REQUIRED)
find_package(PORTAUDIO REQUIRED)
find_package(Freetype REQUIRED)


include_directories(/usr/local/include)
include_directories(/usr/local/include/freetype2)
include_directories(../libs)
include_directories(${SDL2_INCLUDE_DIRS} ${LIBSNDFILE_INCLUDE_DIRS} ${PORTAUDIO_INCLUDE_DIRS})

link_directories(/usr/local/lib)

add_library(lib_pa_utils STATIC ../libs/pa_utils.cpp ../libs/pa_utils.h)
add_library(lib_input_state STATIC ../libs/input_state.cpp ../libs/input_state.h)
add_library(lib_utile STATIC ../libs/utile.cpp ../libs/utile.h)
add_library(lib_gl_utils STATIC ../libs/gl_utils.cpp ../libs/gl_utils.h)
add_library(lib_font STATIC ../libs/font.cpp ../libs/font.h)
add_library(lib_repere STATIC ../libs/repere.cpp ../libs/repere.h)
add_library(lib_objfile STATIC ../libs/objfile.cpp ../libs/objfile.h)
add_library(lib_light STATIC ../libs/light.cpp ../libs/light.h)
add_library(lib_bbox STATIC ../libs/bbox.cpp ../libs/bbox.h)
add_library(lib_bbox_2d STATIC ../libs/bbox_2d.cpp ../libs/bbox_2d.h)

add_library(lib_spectrum STATIC ./srcs/spectrum.cpp ./srcs/spectrum.h)

add_executable(audio2video ./srcs/main.cpp ./srcs/constantes.h)

target_link_libraries(audio2video PUBLIC ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES} ${FREETYPE_LIBRARIES} ${PORTAUDIO_LIBRARIES} ${LIBSNDFILE_LIBRARIES} fftw3)
target_link_libraries(audio2video PUBLIC lib_pa_utils lib_input_state lib_utile lib_gl_utils lib_font lib_spectrum lib_repere lib_objfile lib_light lib_bbox lib_bbox_2d)
