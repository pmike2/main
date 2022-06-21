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

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/cmake/")

project(tmp)

find_package(SDL2 REQUIRED)
find_package(SDL2_image)
find_package(OpenGL REQUIRED)

include_directories(/usr/local/include)
include_directories(../libs)

link_directories(/usr/local/lib)

add_library(lib_utile STATIC ../libs/utile.cpp ../libs/utile.h)
add_library(lib_gl_utils STATIC ../libs/gl_utils.cpp ../libs/gl_utils.h)
add_library(lib_mpeg STATIC ../libs/mpeg.cpp ../libs/mpeg.h)

add_executable(main ./srcs/main.cpp)

set(FFMPEG_OPTIONS "-Wl,-no_compact_unwind -L/usr/local/lib" "-L/usr/lib" "-lbz2" "-liconv" "-lz" "-lavutil" "-lavcodec" "-lavdevice" "-lavformat" "-lavfilter" "-lswresample" "-lswscale" "-lx264" "-pthread" "-lm" "-framework AudioUnit" "-framework AudioToolbox" "-framework CoreAudio" "-framework VideoToolbox" "-framework CoreFoundation" "-framework CoreMedia" "-framework CoreVideo" "-framework CoreServices" "-framework Security")
target_link_libraries(main PUBLIC ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${OPENGL_LIBRARIES} ${FFMPEG_OPTIONS} lib_gl_utils lib_utile lib_mpeg)
