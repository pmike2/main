cmake_minimum_required(VERSION 3.13)

project(sandbox)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/../cmake_files/")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

find_package(LibSndFile REQUIRED)

include_directories(/usr/local/include)
include_directories(../libs)

link_directories(/usr/local/lib)

add_library(lib_utile STATIC ../libs/utile.cpp ../libs/utile.h)

add_executable(main ./srcs/main.cpp)

set(LOCAL_LIBS lib_utile)
set(EXT_LIBS ${LIBSNDFILE_LIBRARIES} fftw3)
 
target_link_libraries(main PUBLIC ${LOCAL_LIBS})
target_link_libraries(main PUBLIC ${EXT_LIBS})
