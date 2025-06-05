cmake_minimum_required(VERSION 3.13)

project(sandbox)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

include_directories(/usr/local/include)
include_directories(../libs)

add_library(lib_utile STATIC ../libs/utile.cpp ../libs/utile.h)
add_library(lib_turing STATIC ../srcs/turing_game.cpp ../srcs/turing_game.h)

add_executable(main ./srcs/main.cpp)

set(LOCAL_LIBS lib_utile lib_turing)
 
target_link_libraries(main PUBLIC ${LOCAL_LIBS})
