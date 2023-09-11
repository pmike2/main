
cmake_minimum_required(VERSION 3.13)

project(seq)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/../cmake_files/")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

find_package(LibSndFile REQUIRED)
find_package(PORTAUDIO REQUIRED)

include_directories(/usr/local/include)
include_directories(../libs)
include_directories(${LIBSNDFILE_INCLUDE_DIRS} ${PORTAUDIO_INCLUDE_DIRS})

link_directories(/usr/local/lib)

add_library(lib_utile STATIC ../libs/utile.cpp ../libs/utile.h)

add_executable(transient_detect ./srcs/transient_detect.cpp)

target_link_libraries(transient_detect PUBLIC ${PORTAUDIO_LIBRARIES} ${LIBSNDFILE_LIBRARIES} lib_utile)
