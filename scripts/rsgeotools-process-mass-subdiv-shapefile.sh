#!/bin/bash

if test -z "$RVT_SHP_ARCHIVE_DIR"
then
	echo "RVT_SHP_ARCHIVE_DIR is not set. "
	exit
fi

if test -z "$RVT_SHP_DIR"
then
	echo "RVT_SHP_DIR is not set. "
	exit
fi

if test -z "$RVT_TEMP_DIR"
then
	echo "RVT_TEMP_DIR is not set. "
	exit
fi


IFS=$'\n'

if test -z "$5"
then
	echo "Usage: cmd TOP_Z TOP_X TOP_Y TILE_Z SHAPEFILE_PREFIX"
	exit
fi

TOP_Z=$1
TOP_X=$2
TOP_Y=$3

TILE_Z=$4
TILE_Z_PARENT=$(expr $TILE_Z - 1)

SHAPEFILE_PREFIX=$5

LOG_FILE="$RVT_SHP_ARCHIVE_DIR/${SHAPEFILE_PREFIX}_log_file.txt"

rm -f $LOG_FILE

DEFAULT_DIR=`pwd`


for tile_pair in `geoconv I $TOP_Z $TOP_X $TOP_Y $TILE_Z_PARENT`
do


	current_number=$(expr $current_number + 1)

	TP=$(echo $tile_pair | tr "_" "\n")

	echo "------- #$current_number (zoom $TILE_Z_PARENT) ($tile_pair) -------"

	subtile_pairs=(`geoconv i $TILE_Z_PARENT $TP $TILE_Z`)

	t0_tp=$(echo ${subtile_pairs[0]} | tr "_" " ")
	t1_tp=$(echo ${subtile_pairs[1]} | tr "_" " ")
	t2_tp=$(echo ${subtile_pairs[2]} | tr "_" " ")
	t3_tp=$(echo ${subtile_pairs[3]} | tr "_" " ")

	t0_cmd="rsgeotools-process-subdiv-shapefile.sh $TILE_Z ${t0_tp} $PROCESS_Z $SHAPEFILE_PREFIX"
	t1_cmd="rsgeotools-process-subdiv-shapefile.sh $TILE_Z ${t1_tp} $PROCESS_Z $SHAPEFILE_PREFIX"
	t2_cmd="rsgeotools-process-subdiv-shapefile.sh $TILE_Z ${t2_tp} $PROCESS_Z $SHAPEFILE_PREFIX"
	t3_cmd="rsgeotools-process-subdiv-shapefile.sh $TILE_Z ${t3_tp} $PROCESS_Z $SHAPEFILE_PREFIX"
	
	eval $t0_cmd >> $LOG_FILE
	eval $t1_cmd >> $LOG_FILE
	eval $t2_cmd >> $LOG_FILE
	eval $t3_cmd >> $LOG_FILE


done

cd $DEFAULT_DIR



