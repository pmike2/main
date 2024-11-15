cmake_minimum_required(VERSION 3.13)

project(test_midi)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(/usr/local/include)
include_directories(/usr/local/include/rtmidi)
include_directories(../libs)

add_executable(main ./srcs/main.cpp)

target_link_libraries(main "-L/usr/local/lib -lrtmidi")
