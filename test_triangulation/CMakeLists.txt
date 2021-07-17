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

# pour ne pas avoir les warnings de opengl deprecated
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

# cmake peut faire par défaut des find_package sur des libs classiques
# mais par il ne connait pas SDL2_image, j'ai récupéré un .cmake que j'ai mis dans le dossier cmake
# il faut faire la ligne ci-dessous pour lui dire d'aller chercher dans le dossier cmake
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")


project(test_triangulation)


include_directories(/usr/local/include)
include_directories(../libs)

add_library(lib_bbox_2d STATIC ../libs/bbox_2d.cpp ../libs/bbox_2d.h)
add_library(lib_geom_2d STATIC ../libs/geom_2d.cpp ../libs/geom_2d.h)
add_library(lib_utile STATIC ../libs/utile.cpp ../libs/utile.h)
add_library(lib_triangulation STATIC ../libs/triangulation.cpp ../libs/triangulation.h)

add_executable(main ./srcs/main.cpp ./srcs/constantes.h)

target_link_libraries(main PUBLIC lib_bbox_2d)
target_link_libraries(main PUBLIC lib_geom_2d)
target_link_libraries(main PUBLIC lib_utile)
target_link_libraries(main PUBLIC lib_triangulation)

