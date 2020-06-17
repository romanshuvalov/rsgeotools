WORKDIR = `pwd`

CC = gcc
CXX = g++
LD = g++

INC = 
INC_RVTGEN3D = -I src/rvtgen3d
CFLAGS = -g 
CXXFLAGS = -g -fpermissive
OUTPUT_BIN_DIR = bin
SRC_DIR = src
OBJ_DIR = obj

all: rsgeotools

clean: 
	rm -rf $(OUTPUT_BIN_DIR)
	rm -rf $(OBJ_DIR)

before_rsgeotools: 
	test -d $(OUTPUT_BIN_DIR) || mkdir -p $(OUTPUT_BIN_DIR)

rsgeotools: before_rsgeotools rsgeotools-conv rsgeotools-csv2rvtdata rsgeotools-tiffcompose rsgeotools-tiff2hmdata rsgeotools-o5m-get-outlines rsgeotools-tilepacker rsgeotools-rvtgen3d

rsgeotools-conv: before_rsgeotools  
	$(CXX) $(CFLAGS) $(INC) $(SRC_DIR)/conv/conv.cpp -o $(OUTPUT_BIN_DIR)/rsgeotools-conv

rsgeotools-csv2rvtdata: before_rsgeotools  
	$(CXX) $(CFLAGS) $(INC) -I$(SRC_DIR)/csv2rvtdata/ -lgeos_c $(SRC_DIR)/csv2rvtdata/csv.c $(SRC_DIR)/csv2rvtdata/csv2rvtdata.cpp -o $(OUTPUT_BIN_DIR)/rsgeotools-csv2rvtdata

rsgeotools-tiffcompose: before_rsgeotools  
	$(CXX) $(CFLAGS) $(INC) $(SRC_DIR)/tiffcompose/tiffcompose.cpp -ltiff -o $(OUTPUT_BIN_DIR)/rsgeotools-tiffcompose

rsgeotools-tiff2hmdata: before_rsgeotools  
	$(CXX) $(CFLAGS) $(INC) $(SRC_DIR)/tiff2hmdata/tiff2hmdata.cpp -ltiff -o $(OUTPUT_BIN_DIR)/rsgeotools-tiff2hmdata
	
rsgeotools-o5m-get-outlines: before_rsgeotools  
	$(CC) $(CFLAGS) $(INC) $(SRC_DIR)/o5m_get_outlines/o5m_get_outlines.c -o $(OUTPUT_BIN_DIR)/rsgeotools-o5m-get-outlines
	
rsgeotools-tilepacker: before_rsgeotools  
	$(CXX) $(CFLAGS) $(INC) $(SRC_DIR)/tilepacker/tilepacker.cpp -lz -lbz2 -o $(OUTPUT_BIN_DIR)/rsgeotools-tilepacker 

rsgeotools-rvtgen3d: before_rsgeotools
	test -d $(OBJ_DIR) || mkdir -p $(OBJ_DIR)
	test -d $(OBJ_DIR)/rvtgen3d || mkdir -p $(OBJ_DIR)/rvtgen3d
	
	$(CXX) $(CXXFLAGS) $(INC_RVTGEN3D) -c $(SRC_DIR)/rvtgen3d/clipper/clipper.cpp -o $(OBJ_DIR)/rvtgen3d/clipper.o
	
	$(CC) $(CFLAGS) $(INC_RVTGEN3D) -c $(SRC_DIR)/rvtgen3d/loader.c -o $(OBJ_DIR)/rvtgen3d/loader.o
	$(CC) $(CFLAGS) $(INC_RVTGEN3D) -c $(SRC_DIR)/rvtgen3d/main.c -o $(OBJ_DIR)/rvtgen3d/main.o
	
	$(CXX) $(CXXFLAGS) $(INC_RVTGEN3D) -c $(SRC_DIR)/rvtgen3d/rs/rsboost.cpp -o $(OBJ_DIR)/rvtgen3d/rsboost.o
	$(CXX) $(CXXFLAGS) $(INC_RVTGEN3D) -c $(SRC_DIR)/rvtgen3d/rs/rsgeom.cpp -o $(OBJ_DIR)/rvtgen3d/rsgeom.o
	$(CC) $(CFLAGS) $(INC_RVTGEN3D) -c $(SRC_DIR)/rvtgen3d/rs/rsmem.c -o $(OBJ_DIR)/rvtgen3d/rsmem.o
	$(CC) $(CFLAGS) $(INC_RVTGEN3D) -c $(SRC_DIR)/rvtgen3d/rs/rsmx.c -o $(OBJ_DIR)/rvtgen3d/rsmx.o
	$(CC) $(CFLAGS) $(INC_RVTGEN3D) -c $(SRC_DIR)/rvtgen3d/rs/rsnoise.c -o $(OBJ_DIR)/rvtgen3d/rsnoise.o
	
	$(CC) $(CFLAGS) $(INC_RVTGEN3D) -c $(SRC_DIR)/rvtgen3d/rvt.c -o $(OBJ_DIR)/rvtgen3d/rvt.o
	$(CC) $(CFLAGS) $(INC_RVTGEN3D) -c $(SRC_DIR)/rvtgen3d/rvtapp.c -o $(OBJ_DIR)/rvtgen3d/rvtapp.o
	$(CC) $(CFLAGS) $(INC_RVTGEN3D) -c $(SRC_DIR)/rvtgen3d/rvtexport.c -o $(OBJ_DIR)/rvtgen3d/rvtexport.o
	$(CC) $(CFLAGS) $(INC_RVTGEN3D) -c $(SRC_DIR)/rvtgen3d/rvtgen.c -o $(OBJ_DIR)/rvtgen3d/rvtgen.o
	$(CC) $(CFLAGS) $(INC_RVTGEN3D) -c $(SRC_DIR)/rvtgen3d/rvtloader.c -o $(OBJ_DIR)/rvtgen3d/rvtloader.o
	$(CC) $(CFLAGS) $(INC_RVTGEN3D) -c $(SRC_DIR)/rvtgen3d/rvtutil.c -o $(OBJ_DIR)/rvtgen3d/rvtutil.o
	$(CC) $(CFLAGS) $(INC_RVTGEN3D) -c $(SRC_DIR)/rvtgen3d/tileloader.c -o $(OBJ_DIR)/rvtgen3d/tileloader.o

	$(CXX) -o $(OUTPUT_BIN_DIR)/rsgeotools-rvtgen3d $(OBJ_DIR)/rvtgen3d/clipper.o  $(OBJ_DIR)/rvtgen3d/loader.o  $(OBJ_DIR)/rvtgen3d/main.o  $(OBJ_DIR)/rvtgen3d/rsboost.o  $(OBJ_DIR)/rvtgen3d/rsgeom.o  $(OBJ_DIR)/rvtgen3d/rsmem.o  $(OBJ_DIR)/rvtgen3d/rsmx.o  $(OBJ_DIR)/rvtgen3d/rsnoise.o  $(OBJ_DIR)/rvtgen3d/rvt.o  $(OBJ_DIR)/rvtgen3d/rvtapp.o  $(OBJ_DIR)/rvtgen3d/rvtexport.o  $(OBJ_DIR)/rvtgen3d/rvtgen.o  $(OBJ_DIR)/rvtgen3d/rvtloader.o  $(OBJ_DIR)/rvtgen3d/rvtutil.o  $(OBJ_DIR)/rvtgen3d/tileloader.o  -m64 -lbz2 -lz -lpthread	

.PHONY: before_rsgeotools clean_rsgeotools
