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


project(samourai)


find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenGL REQUIRED)
find_package(TIFF REQUIRED)
find_package(GDAL REQUIRED)

include_directories(/usr/local/include)
include_directories(/usr/local/include/freetype2)
include_directories(../libs)

# si les installs des libs externes sont faites dans des emplacements différents de /usr/local, on peut faire ca
# pour chaque lib, cf https://cmake.org/cmake/help/latest/manual/cmake-modules.7.html
# include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})
include_directories(${GDAL_INCLUDE_DIRS})

add_library(lib_daefile STATIC ../libs/daefile.cpp ../libs/daefile.h)
add_library(lib_repere STATIC ../libs/repere.cpp ../libs/repere.h)
add_library(lib_light STATIC ../libs/light.cpp ../libs/light.h)
add_library(lib_gl_utils STATIC ../libs/gl_utils.cpp ../libs/gl_utils.h)
add_library(lib_utile STATIC ../libs/utile.cpp ../libs/utile.h)
add_library(lib_input_state STATIC ../libs/input_state.cpp ../libs/input_state.h)
add_library(lib_font STATIC ../libs/font.cpp ../libs/font.h)
add_library(lib_bbox STATIC ../libs/bbox.cpp ../libs/bbox.h)
add_library(lib_terrain STATIC ../libs/terrain.cpp ../libs/terrain.h)
add_library(lib_world STATIC ../libs/world.cpp ../libs/world.h)
add_library(lib_gl_interface STATIC ../libs/gl_interface.cpp ../libs/gl_interface.h)
add_library(lib_objfile STATIC ../libs/objfile.cpp ../libs/objfile.h)
add_library(lib_geom_2d STATIC ../libs/geom_2d.cpp ../libs/geom_2d.h)
add_library(lib_shapefile STATIC ../libs/shapefile.cpp ../libs/shapefile.h)
add_library(lib_bbox_2d STATIC ../libs/bbox_2d.cpp ../libs/bbox_2d.h)

add_executable(main ./srcs/main.cpp ./srcs/constantes.h)

target_link_libraries(main PUBLIC lib_daefile)
target_link_libraries(main PUBLIC lib_repere)
target_link_libraries(main PUBLIC lib_light)
target_link_libraries(main PUBLIC lib_gl_utils)
target_link_libraries(main PUBLIC lib_utile)
target_link_libraries(main PUBLIC lib_input_state)
target_link_libraries(main PUBLIC lib_font)
target_link_libraries(main PUBLIC lib_bbox)
target_link_libraries(main PUBLIC lib_terrain)
target_link_libraries(main PUBLIC lib_world)
target_link_libraries(main PUBLIC lib_gl_interface)
target_link_libraries(main PUBLIC lib_objfile)
target_link_libraries(main PUBLIC lib_geom_2d)
target_link_libraries(main PUBLIC lib_shapefile)
target_link_libraries(main PUBLIC lib_bbox_2d)

# bug debile cf https://stackoverflow.com/questions/45730098/cmake-leading-or-trailing-whitespace-policy-cmp0004
string(STRIP ${SDL2_LIBRARIES} SDL2_LIBRARIES)
target_link_libraries(main PUBLIC ${SDL2_LIBRARIES})
target_link_libraries(main PUBLIC ${SDL2_IMAGE_LIBRARIES})
target_link_libraries(main PUBLIC ${FREETYPE_LIBRARIES})
target_link_libraries(main PUBLIC ${OPENGL_LIBRARIES})
target_link_libraries(main PUBLIC ${TIFF_LIBRARIES})
target_link_libraries(main PUBLIC ${GDAL_LIBRARIES})
