#
# mkdir build && cd build
# cmake ..
# make
# ./main
#

cmake_minimum_required(VERSION 3.13)

project(looper_audio_video)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/../cmake_files/")

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
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
add_library(lib_mpeg STATIC ../libs/mpeg.cpp ../libs/mpeg.h)
add_library(lib_geom_2d STATIC ../libs/geom_2d.cpp ../libs/geom_2d.h)
add_library(lib_triangulation STATIC ../libs/triangulation.cpp ../libs/triangulation.h)
add_library(lib_bbox_2d STATIC ../libs/bbox_2d.cpp ../libs/bbox_2d.h)
add_library(lib_sio_util STATIC ../libs/sio_util.cpp ../libs/sio_util.h)
add_library(lib_font STATIC ../libs/font.cpp ../libs/font.h)

add_library(lib_shared_mem STATIC ./srcs/shared_mem.cpp ./srcs/shared_mem.h)
add_library(lib_looper STATIC ./srcs/looper.cpp ./srcs/looper.h)
add_library(lib_mpeg_reader STATIC ./srcs/mpeg_reader.cpp ./srcs/mpeg_reader.h)

#add_executable(looper_sdl_main ./srcs/looper_sdl_main.cpp ./srcs/looper_sdl.cpp ./srcs/looper_sdl.h)
add_executable(looper_gl_main ./srcs/looper_gl_main.cpp ./srcs/looper_gl.cpp ./srcs/looper_gl.h)
add_executable(receiver_test_main ./srcs/receiver_test_main.cpp ./srcs/receiver_test.cpp ./srcs/receiver_test.h)
add_executable(audio_sampler_main ./srcs/audio_sampler_main.cpp ./srcs/audio_sampler.cpp ./srcs/audio_sampler.h)
add_executable(cv_out_main ./srcs/cv_out_main.cpp ./srcs/cv_out.cpp ./srcs/cv_out.h)
add_executable(cv_in_main ./srcs/cv_in_main.cpp ./srcs/cv_in.cpp ./srcs/cv_in.h)
add_executable(list_audio_devices ./srcs/list_audio_devices.cpp)
add_executable(video_sampler_main ./srcs/video_sampler_main.cpp ./srcs/video_sampler.cpp ./srcs/video_sampler.h)


#string(STRIP ${SDL2_LIBRARIES} SDL2_LIBRARIES)
set(FFMPEG_OPTIONS "-Wl,-no_compact_unwind -L/usr/local/lib" "-L/usr/lib" "-lbz2" "-liconv" "-lz" "-lavutil" "-lavcodec" "-lavdevice" "-lavformat" "-lavfilter" "-lswresample" "-lswscale" "-pthread" "-lm" "-framework AudioUnit" "-framework AudioToolbox" "-framework CoreAudio" "-framework VideoToolbox" "-framework CoreFoundation" "-framework CoreMedia" "-framework CoreVideo" "-framework CoreGraphics" "-framework CoreImage" "-framework CoreServices" "-framework Security" "-framework Foundation" "-framework AppKit")


#message(PORTAUDIO_LIBRARIES="${PORTAUDIO_LIBRARIES}")
#message(PORTAUDIO_INCLUDE_DIRS="${PORTAUDIO_INCLUDE_DIRS}")

set(EXT_LIBS ${LIBSNDFILE_LIBRARIES} ${PORTAUDIO_LIBRARIES} ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${OPENGL_LIBRARIES} ${FREETYPE_LIBRARIES} ${FFMPEG_OPTIONS} sioclient)

target_link_libraries(looper_gl_main PUBLIC ${EXT_LIBS} lib_shared_mem lib_looper lib_input_state lib_gl_utils lib_font lib_utile)
target_link_libraries(receiver_test_main PUBLIC lib_shared_mem)
target_link_libraries(audio_sampler_main PUBLIC ${EXT_LIBS} lib_shared_mem lib_pa_utils lib_sio_util)
target_link_libraries(cv_out_main PUBLIC ${EXT_LIBS} lib_shared_mem lib_pa_utils lib_sio_util)
target_link_libraries(cv_in_main PUBLIC ${EXT_LIBS} lib_shared_mem lib_pa_utils lib_sio_util)
target_link_libraries(list_audio_devices PUBLIC ${EXT_LIBS} lib_pa_utils)
target_link_libraries(video_sampler_main PUBLIC ${EXT_LIBS} lib_shared_mem lib_input_state lib_utile lib_gl_utils lib_mpeg lib_mpeg_reader lib_geom_2d lib_triangulation lib_bbox_2d lib_sio_util)
