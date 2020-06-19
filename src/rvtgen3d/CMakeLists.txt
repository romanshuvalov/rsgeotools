cmake_minimum_required(VERSION 3.7)
project(rsgeotools-rvtgen3d)

set(CMAKE_CXX_STANDARD 14)

add_executable(${PROJECT_NAME}
		clipper/clipper.cpp
		loader.c
		main.c
		rs/rsboost.cpp
		rs/rsgeom.cpp
		rs/rsmem.c
		rs/rsmx.c
		rs/rsnoise.c
		rvt.c
		rvtapp.c
		rvtexport.c
		rvtgen.c
		rvtloader.c
		rvtutil.c
		tileloader.c
		)

target_link_libraries(${PROJECT_NAME} PRIVATE bz2 z pthread) #TODO add package search before
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
