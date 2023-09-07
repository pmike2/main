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
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

# cmake peut faire par défaut des find_package sur des libs classiques
# mais par il ne connait pas SDL2_image, j'ai récupéré un .cmake que j'ai mis dans le dossier cmake
# il faut faire la ligne ci-dessous pour lui dire d'aller chercher dans le dossier cmake
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/cmake/")


project(spaceship)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenGL REQUIRED)


include_directories(/usr/local/include)
include_directories(/usr/local/include/freetype2)
include_directories(../libs)

# si les installs des libs externes sont faites dans des emplacements différents de /usr/local, on peut faire ca
# pour chaque lib, cf https://cmake.org/cmake/help/latest/manual/cmake-modules.7.html
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})


add_library(lib_repere STATIC ../libs/repere.cpp ../libs/repere.h)
add_library(lib_gl_utils STATIC ../libs/gl_utils.cpp ../libs/gl_utils.h)
add_library(lib_utile STATIC ../libs/utile.cpp ../libs/utile.h)
add_library(lib_input_state STATIC ../libs/input_state.cpp ../libs/input_state.h)
add_library(lib_font STATIC ../libs/font.cpp ../libs/font.h)
add_library(lib_light STATIC ../libs/light.cpp ../libs/light.h)
add_library(lib_bbox STATIC ../libs/bbox.cpp ../libs/bbox.h)
add_library(lib_terrain STATIC ../libs/terrain.cpp ../libs/terrain.h)
add_library(lib_objfile STATIC ../libs/objfile.cpp ../libs/objfile.h)
add_library(lib_world STATIC ../libs/world.cpp ../libs/world.h)

add_library(lib_ship STATIC ./srcs/ship.cpp ./srcs/ship.h ./srcs/constantes.h)
add_library(lib_level STATIC ./srcs/level.cpp ./srcs/level.h ./srcs/constantes.h)

add_executable(main ./srcs/main.cpp ./srcs/constantes.h)

target_link_libraries(main PUBLIC ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${FREETYPE_LIBRARIES} ${OPENGL_LIBRARIES})
target_link_libraries(main PUBLIC lib_repere lib_gl_utils lib_utile lib_input_state lib_font lib_light lib_bbox lib_terrain lib_objfile lib_world lib_ship lib_level)

