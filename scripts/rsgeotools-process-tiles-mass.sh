#!/bin/bash


if test -z "$RVT_O5M_DIR"
then
	echo "RVT_O5M_DIR is not set. "
	exit
fi

if test -z "$RVT_SHP_ARCHIVE_DIR"
then
	echo "RVT_SHP_ARCHIVE_DIR is not set. "
	exit
fi

if test -z "$RVT_GPAK_DIR"
then
	echo "RVT_GPAK_DIR is not set. "
	exit
fi


if test -z "$RVT_CSV_CONF"
then
	echo "RVT_CSV_CONF ini file is not set. "
	exit
fi

if test -z "$RVT_TEMP_DIR"
then
	echo "RVT_TEMP_DIR is not set. "
	exit
fi


IFS=$'\n'

if test -z "$8"
then
	echo "Usage: cmd TOP_Z TOP_X TOP_Y TILE_Z PROCESS_Z SUBPAK_Z DEST_Z PLANET_TIMESTAMP"
	exit
fi

TOP_Z=$1
TOP_X=$2
TOP_Y=$3

TILE_Z=$4
TILE_Z_PARENT=$(expr $TILE_Z - 1)

PROCESS_Z=$5

SUBPAK_Z=$6

DEST_Z=$7

PLANET_TIMESTAMP=$8

LOG_FILE="$RVT_O5M_DIR/log_file.txt"

rm -f $LOG_FILE

DEFAULT_DIR=`pwd`


for tile_pair in `geoconv I $TOP_Z $TOP_X $TOP_Y $TILE_Z_PARENT`
do

	current_number=$(expr $current_number + 1)

	TP=$(echo $tile_pair | tr "_" "\n")

	echo "--- #$current_number (z $TILE_Z_PARENT) ($tile_pair) (single thread = $RVT_SINGLE_THREAD) ---"

	subtile_pairs=(`geoconv i $TILE_Z_PARENT $TP $TILE_Z`)

	t0_tp=$(echo ${subtile_pairs[0]} | tr "_" " ")
	t1_tp=$(echo ${subtile_pairs[1]} | tr "_" " ")
	t2_tp=$(echo ${subtile_pairs[2]} | tr "_" " ")
	t3_tp=$(echo ${subtile_pairs[3]} | tr "_" " ")

	t0_cmd="rsgeotools-process-tile.sh $TILE_Z ${t0_tp} $PROCESS_Z $SUBPAK_Z $DEST_Z $PLANET_TIMESTAMP"
	t1_cmd="rsgeotools-process-tile.sh $TILE_Z ${t1_tp} $PROCESS_Z $SUBPAK_Z $DEST_Z $PLANET_TIMESTAMP"
	t2_cmd="rsgeotools-process-tile.sh $TILE_Z ${t2_tp} $PROCESS_Z $SUBPAK_Z $DEST_Z $PLANET_TIMESTAMP"
	t3_cmd="rsgeotools-process-tile.sh $TILE_Z ${t3_tp} $PROCESS_Z $SUBPAK_Z $DEST_Z $PLANET_TIMESTAMP"

	
	if test -z "$RVT_SINGLE_THREAD"
	then
		# Caution: t7_67_43, t7_65_42, t7_66_42 are too big, can cause 'out of memory' errors
		# That's why we use 2 threads instead of 4

		eval $t0_cmd >> $LOG_FILE &
		eval $t1_cmd >> $LOG_FILE &
		wait
		eval $t2_cmd >> $LOG_FILE &
		eval $t3_cmd >> $LOG_FILE &
		wait
		
	else
		eval $t0_cmd
		eval $t1_cmd
		eval $t2_cmd
		eval $t3_cmd
	fi


done

cd $DEFAULT_DIR



