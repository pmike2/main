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

include_directories(/usr/local/include)

add_executable(decode_mp4 ./srcs/decode_mp4.cpp)

target_link_libraries(decode_mp4 "-L/usr/local/lib" "-L/usr/lib" "-lbz2" "-liconv" "-lz" "-lavutil" "-lavcodec" "-lavdevice" "-lavformat" "-lavfilter" "-lswresample" "-lswscale" "-pthread" "-lm" "-framework AudioUnit" "-framework AudioToolbox" "-framework CoreAudio" "-framework VideoToolbox" "-framework CoreFoundation" "-framework CoreMedia" "-framework CoreVideo" "-framework CoreServices" "-framework Security")
