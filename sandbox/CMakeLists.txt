# traditionnellement on fait :
#
# mkdir build && cd build
# cmake ..
# make
# ./main
#

cmake_minimum_required(VERSION 3.13)

project(sandbox)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(OpenGL REQUIRED)

include_directories(/usr/local/include)
include_directories(../libs)

link_directories(/usr/local/lib)

add_executable(main ./srcs/main.cpp)

target_link_libraries(main PUBLIC ${OPENGL_LIBRARIES})
