cmake_minimum_required(VERSION 3.7)
project(rsgeotools-tilepacker)

set(CMAKE_CXX_STANDARD 14)

add_executable(${PROJECT_NAME} tilepacker.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE z bz2) #TODO add package search before
