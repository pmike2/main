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

project(decode_mp4)

find_package(SDL2 REQUIRED)

include_directories(/usr/local/include)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(decode_mp4 ./srcs/decode_mp4.cpp)

string(STRIP ${SDL2_LIBRARIES} SDL2_LIBRARIES)
target_link_libraries(decode_mp4 ${SDL2_LIBRARIES} "-Wl,-no_compact_unwind -L/usr/local/lib" "-L/usr/lib" "-lbz2" "-liconv" "-lz" "-lavutil" "-lavcodec" "-lavdevice" "-lavformat" "-lavfilter" "-lswresample" "-lswscale" "-pthread" "-lm" "-framework AudioUnit" "-framework AudioToolbox" "-framework CoreAudio" "-framework VideoToolbox" "-framework CoreFoundation" "-framework CoreMedia" "-framework CoreVideo" "-framework CoreServices" "-framework Security")
