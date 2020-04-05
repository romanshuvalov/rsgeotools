#!/bin/bash


IFS=$'\n'

TOP_Z=$1
TOP_X=$2
TOP_Y=$3

TILE_Z=$4
TILE_Z_PARENT=$(expr $TILE_Z - 1)

TILE_PREFIX=$5

OUTPUT_DIR="."


if test -z "$5"
then
	echo "Usage: rsgeotools-subdiv-shapefile.sh TOP_Z TOP_X TOP_Y DEST_Z TILE_PREFIX "
	exit
fi


current_number=0



for tile_pair in `rsgeotools-conv I $TOP_Z $TOP_X $TOP_Y $TILE_Z_PARENT`
do
	

	current_number=$(expr $current_number + 1)

	TP=$(echo $tile_pair | tr "_" "\n")


	echo "------- #$current_number (zoom $TILE_Z_PARENT) ($tile_pair) -------"


	subtile_pairs=(`rsgeotools-conv i $TILE_Z_PARENT $TP $TILE_Z`)

	t0_tp=$(echo ${subtile_pairs[0]} | tr "_" "\n")
	t1_tp=$(echo ${subtile_pairs[1]} | tr "_" "\n")
	t2_tp=$(echo ${subtile_pairs[2]} | tr "_" "\n")
	t3_tp=$(echo ${subtile_pairs[3]} | tr "_" "\n")

	
	t0_cmd="ogr2ogr -skipfailures -clipsrc `rsgeotools-conv c $TILE_Z $t0_tp` ${TILE_PREFIX}${TILE_Z}_${subtile_pairs[0]}.shp ${TILE_PREFIX}${TILE_Z_PARENT}_${tile_pair}.shp"
	t1_cmd="ogr2ogr -skipfailures -clipsrc `rsgeotools-conv c $TILE_Z $t1_tp` ${TILE_PREFIX}${TILE_Z}_${subtile_pairs[1]}.shp ${TILE_PREFIX}${TILE_Z_PARENT}_${tile_pair}.shp"
	t2_cmd="ogr2ogr -skipfailures -clipsrc `rsgeotools-conv c $TILE_Z $t2_tp` ${TILE_PREFIX}${TILE_Z}_${subtile_pairs[2]}.shp ${TILE_PREFIX}${TILE_Z_PARENT}_${tile_pair}.shp"
	t3_cmd="ogr2ogr -skipfailures -clipsrc `rsgeotools-conv c $TILE_Z $t3_tp` ${TILE_PREFIX}${TILE_Z}_${subtile_pairs[3]}.shp ${TILE_PREFIX}${TILE_Z_PARENT}_${tile_pair}.shp"


	eval $t0_cmd &
	eval $t1_cmd &
	eval $t2_cmd &
	eval $t3_cmd &
	wait

done


echo "Done."


