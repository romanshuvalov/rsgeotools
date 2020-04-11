#!/bin/sh

if test -z "$RVT_GPAK_DIR"
then
	echo "RVT_GPAK_DIR is not set. "
	exit
fi

if test -z "$RVT_TEMP_DIR"
then
	echo "RVT_TEMP_DIR is not set. "
	exit
fi

if test -z "$RVT_HGT_DIR_NASADEM"
then
	echo "RVT_HGT_DIR_NASADEM is not set. "
	exit
fi

if test -z "$RVT_HGT_DIR_ASTERGDEMV3"
then
	echo "RVT_HGT_DIR_ASTERGDEMV3 is not set. "
	exit
fi


if test -z "$6"
then
	echo "Usage: rsgeotools-process-hm-full.sh TOP_Z TOP_X TOP_Y ARCHIVE_Z SUBPAK_Z DEST_Z (example: 5 20 10 6 11 11) "
	exit
fi


TOP_Z=$1
TOP_X=$2
TOP_Y=$3

TILE_Z=$4

SUBPAK_Z=$5

DEST_Z=$6


for tile_pair in `geoconv I $TOP_Z $TOP_X $TOP_Y $TILE_Z`
do

	TP=$(echo $tile_pair | tr "_" " ")
	eval "rsgeotools-process-hm.sh $TILE_Z $TP $SUBPAK_Z $DEST_Z"

done




