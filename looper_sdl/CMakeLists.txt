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

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/cmake/")

project(looper_sdl)

find_package(SDL2 REQUIRED)
find_package(LibSndFile REQUIRED)
find_package(PORTAUDIO REQUIRED)

include_directories(/usr/local/include)
include_directories(../libs)
include_directories(${SDL2_INCLUDE_DIRS} ${LIBSNDFILE_INCLUDE_DIRS} ${PORTAUDIO_INCLUDE_DIRS})

add_library(lib_looper STATIC ../libs/looper.cpp ../libs/looper.h)
add_library(lib_pa_utils STATIC ../libs/pa_utils.cpp ../libs/pa_utils.h)
add_library(lib_input_state STATIC ../libs/input_state.cpp ../libs/input_state.h)
add_library(lib_utile STATIC ../libs/utile.cpp ../libs/utile.h)

add_executable(looper_main ./srcs/looper_main.cpp ./srcs/looper_sdl.cpp ./srcs/looper_sdl.h)
add_executable(receiver_test_main ./srcs/receiver_test_main.cpp ./srcs/receiver_test.cpp ./srcs/receiver_test.h)
add_executable(sampler_main ./srcs/sampler_main.cpp ./srcs/sampler.cpp ./srcs/sampler.h)

target_link_libraries(looper_main PUBLIC lib_looper)
target_link_libraries(looper_main PUBLIC lib_input_state)
target_link_libraries(looper_main PUBLIC lib_utile)

target_link_libraries(receiver_test_main PUBLIC lib_looper)

target_link_libraries(sampler_main PUBLIC lib_looper)
target_link_libraries(sampler_main PUBLIC lib_pa_utils)

string(STRIP ${SDL2_LIBRARIES} SDL2_LIBRARIES)
target_link_libraries(looper_main PUBLIC ${SDL2_LIBRARIES})

target_link_libraries(sampler_main PUBLIC ${LIBSNDFILE_LIBRARIES})
target_link_libraries(sampler_main PUBLIC ${PORTAUDIO_LIBRARIES})
