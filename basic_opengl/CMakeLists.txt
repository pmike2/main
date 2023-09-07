# traditionnellement on fait :
#
# mkdir build && cd build
# cmake ..
# make
# ./main
#

cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
set(FFMPEG_OPTIONS "-Wl,-no_compact_unwind -L/usr/local/lib" "-L/usr/lib" "-lbz2" "-liconv" "-lz" "-lavutil" "-lavcodec" "-lavdevice" "-lavformat" "-lavfilter" "-lswresample" "-lswscale" "-pthread" "-lm" "-framework AudioUnit" "-framework AudioToolbox" "-framework CoreAudio" "-framework VideoToolbox" "-framework CoreFoundation" "-framework CoreMedia" "-framework CoreVideo" "-framework CoreGraphics" "-framework CoreImage" "-framework CoreServices" "-framework Security" "-framework Foundation" "-framework AppKit")

project(tmp)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(/usr/local/include)
include_directories(../libs)

link_directories(/usr/local/lib)

add_library(lib_utile STATIC ../libs/utile.cpp ../libs/utile.h)
add_library(lib_gl_utils STATIC ../libs/gl_utils.cpp ../libs/gl_utils.h)
add_library(lib_mpeg STATIC ../libs/mpeg.cpp ../libs/mpeg.h)

add_executable(basic ./srcs/basic.cpp)
add_executable(basic_recording ./srcs/basic_recording.cpp)

target_link_libraries(basic PUBLIC ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES} lib_gl_utils lib_utile)
target_link_libraries(basic_recording PUBLIC ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES} ${FFMPEG_OPTIONS} lib_gl_utils lib_utile lib_mpeg)
