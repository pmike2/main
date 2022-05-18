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

project(nodejs_socket_io)

include_directories(/usr/local/include)
include_directories(../libs)

link_directories(/usr/local/lib)

add_executable(main_curl ./srcs/main_curl.cpp)
add_executable(main_io ./srcs/main_io.cpp)

target_link_libraries(main_io PUBLIC sioclient)
