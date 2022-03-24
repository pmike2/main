
cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(decode_mpeg)

find_package(SDL2 REQUIRED)

include_directories(/usr/local/include)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(decode_mpeg_v1 ./srcs/decode_mpeg_v1.cpp)
add_executable(decode_mpeg_v2 ./srcs/decode_mpeg_v2.cpp)
add_executable(decode_mpeg_v3 ./srcs/decode_mpeg_v3.cpp)

string(STRIP ${SDL2_LIBRARIES} SDL2_LIBRARIES)
set(FFMPEG_OPTIONS "-Wl,-no_compact_unwind -L/usr/local/lib" "-L/usr/lib" "-lbz2" "-liconv" "-lz" "-lavutil" "-lavcodec" "-lavdevice" "-lavformat" "-lavfilter" "-lswresample" "-lswscale" "-pthread" "-lm" "-framework AudioUnit" "-framework AudioToolbox" "-framework CoreAudio" "-framework VideoToolbox" "-framework CoreFoundation" "-framework CoreMedia" "-framework CoreVideo" "-framework CoreServices" "-framework Security")

target_link_libraries(decode_mpeg_v1 ${SDL2_LIBRARIES} ${FFMPEG_OPTIONS})
target_link_libraries(decode_mpeg_v2 ${SDL2_LIBRARIES} ${FFMPEG_OPTIONS})
target_link_libraries(decode_mpeg_v3 ${SDL2_LIBRARIES} ${FFMPEG_OPTIONS})
