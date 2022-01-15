cmake_minimum_required(VERSION 3.17)
project(geo-hospital)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_library(geo-hospital INTERFACE)

target_include_directories(geo-hospital INTERFACE ../../include/)
target_include_directories(geo-hospital INTERFACE ../../external/)

add_executable(main_gis main.cpp)
target_link_libraries(main_gis geo-hospital)