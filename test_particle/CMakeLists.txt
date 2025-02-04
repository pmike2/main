cmake_minimum_required(VERSION 3.13)

project(test_particle)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/cmake/")

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(OpenGL REQUIRED)


include_directories(/usr/local/include)
include_directories(../libs)

add_library(lib_particle STATIC ../libs/particle.cpp ../libs/particle.h)
add_library(lib_gl_utils STATIC ../libs/gl_utils.cpp ../libs/gl_utils.h)
add_library(lib_utile STATIC ../libs/utile.cpp ../libs/utile.h)

add_executable(main ./srcs/main.cpp)
 
target_link_libraries(main PUBLIC lib_particle)
target_link_libraries(main PUBLIC lib_gl_utils)
target_link_libraries(main PUBLIC lib_utile)

target_link_libraries(main PUBLIC ${SDL2_LIBRARIES})
target_link_libraries(main PUBLIC ${SDL2_IMAGE_LIBRARIES})
target_link_libraries(main PUBLIC ${OPENGL_LIBRARIES})
