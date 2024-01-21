cmake_minimum_required(VERSION 3.13)

project(sandbox)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)


include_directories(/usr/local/include)
include_directories(../libs)

add_library(lib_3body STATIC ../srcs/threebody.h ../srcs/threebody.cpp)
add_library(lib_repere STATIC ../libs/repere.cpp ../libs/repere.h)
add_library(lib_gl_utils STATIC ../libs/gl_utils.cpp ../libs/gl_utils.h)
add_library(lib_utile STATIC ../libs/utile.cpp ../libs/utile.h)
add_library(lib_input_state STATIC ../libs/input_state.cpp ../libs/input_state.h)

add_executable(main ./srcs/main.cpp)
 
target_link_libraries(main PUBLIC lib_3body)
target_link_libraries(main PUBLIC lib_repere)
target_link_libraries(main PUBLIC lib_gl_utils)
target_link_libraries(main PUBLIC lib_utile)
target_link_libraries(main PUBLIC lib_input_state)

target_link_libraries(main PUBLIC ${SDL2_LIBRARIES})
target_link_libraries(main PUBLIC ${OPENGL_LIBRARIES})
