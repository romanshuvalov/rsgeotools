cmake_minimum_required(VERSION 3.7)
project(rsgeotools-csv2rvtdata)

set(CMAKE_CXX_STANDARD 14)

add_executable(${PROJECT_NAME} csv.c csv2rvtdata.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE geos_c) #TODO add package search before
