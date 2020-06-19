cmake_minimum_required(VERSION 3.7)
project(rsgeotools-tiffcompose)

set(CMAKE_CXX_STANDARD 14)

add_executable(${PROJECT_NAME} tiffcompose.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE tiff) #TODO add package search before
