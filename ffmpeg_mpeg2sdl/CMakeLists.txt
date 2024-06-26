
cmake_minimum_required(VERSION 3.13)

project(decode_mpeg)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(/usr/local/include)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(../libs)

add_library(lib_utile STATIC ../libs/utile.cpp ../libs/utile.h)
add_library(lib_gl_utils STATIC ../libs/gl_utils.cpp ../libs/gl_utils.h)
add_library(lib_mpeg STATIC ../libs/mpeg.cpp ../libs/mpeg.h)
add_library(lib_geom_2d STATIC ../libs/geom_2d.cpp ../libs/geom_2d.h)
add_library(lib_bbox_2d STATIC ../libs/bbox_2d.cpp ../libs/bbox_2d.h)
add_library(lib_input_state STATIC ../libs/input_state.cpp ../libs/input_state.h)

add_executable(decode_mpeg_v1 ./srcs/decode_mpeg_v1.cpp)
add_executable(decode_mpeg_v2 ./srcs/decode_mpeg_v2.cpp)
add_executable(decode_mpeg_v3 ./srcs/decode_mpeg_v3.cpp)

#string(STRIP ${SDL2_LIBRARIES} SDL2_LIBRARIES)
set(FFMPEG_OPTIONS "-Wl,-no_compact_unwind -L/usr/local/lib" "-L/usr/lib" "-lbz2" "-liconv" "-lz" "-lavutil" "-lavcodec" "-lavdevice" "-lavformat" "-lavfilter" "-lswresample" "-lswscale" "-pthread" "-lm" "-framework AudioUnit" "-framework AudioToolbox" "-framework CoreAudio" "-framework VideoToolbox" "-framework CoreFoundation" "-framework CoreMedia" "-framework CoreVideo" "-framework CoreServices" "-framework Security")

target_link_libraries(decode_mpeg_v1 ${SDL2_LIBRARIES} ${FFMPEG_OPTIONS})
target_link_libraries(decode_mpeg_v2 ${SDL2_LIBRARIES} ${FFMPEG_OPTIONS})
target_link_libraries(decode_mpeg_v3 ${SDL2_LIBRARIES} ${FFMPEG_OPTIONS})
