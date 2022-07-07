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

project(sandbox)

find_package(OpenGL REQUIRED)

include_directories(/usr/local/include)
include_directories(../libs)

link_directories(/usr/local/lib)

add_library(lib_mpeg STATIC ../libs/mpeg.cpp ../libs/mpeg.h)

add_executable(main ./srcs/main.cpp)

set(FFMPEG_OPTIONS "-Wl,-no_compact_unwind -L/usr/local/lib" "-L/usr/lib" "-lbz2" "-liconv" "-lz" "-lavutil" "-lavcodec" "-lavdevice" "-lavformat" "-lavfilter" "-lpostproc" "-lswresample" "-lswscale" "-lx264" "-pthread" "-lm" "-framework AudioUnit" "-framework AudioToolbox" "-framework CoreAudio" "-framework VideoToolbox" "-framework CoreFoundation" "-framework CoreMedia" "-framework CoreVideo" "-framework CoreGraphics" "-framework CoreImage" "-framework CoreServices" "-framework Security" "-framework Foundation" "-framework AppKit")
target_link_libraries(main PUBLIC ${FFMPEG_OPTIONS} ${OPENGL_LIBRARIES} lib_mpeg)
