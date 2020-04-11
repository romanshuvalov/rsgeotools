#!/bin/bash

# Create heightmap tiles from NASADEM and ASTER GDEM v3


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


if test -z "$5"
then
	echo "usage: rsgeotools-process-hm.sh Z X Y SUBPAK_Z DEST_Z (example: 7 81 41 11 11)"
	exit
fi

IFS=$'\n'

TILE_Z=$1
TILE_X=$2
TILE_Y=$3

SUBPAK_Z=$4

DEST_Z=$5
DEST_Z_PARENT=$(expr $DEST_Z - 1)

DEFAULT_DIR=`pwd`

cd $HM_DATA_ROOT_DIR

TEMP_DIR="$RVT_TEMP_DIR/heightmap_temp_${TILE_Z}_${TILE_X}_${TILE_Y}"

OUTPUT_DIR="$RVT_TEMP_DIR/heightmap_data_out_${TILE_Z}_${TILE_X}_${TILE_Y}"


if [ ! -d $RVT_HGT_DIR_NASADEM ]; then
	echo "$RVT_HGT_DIR_NASADEM is not a directory. Aborted."
	exit
fi

if [ ! -d $RVT_HGT_DIR_ASTERGDEMV3 ]; then
	echo "$RVT_HGT_DIR_ASTERGDEMV3 is not a directory. Aborted."
	exit
fi


# 32 for z14, 128 for z12, 256 for z11
IMAGE_SIZE="256"

LOG_FILE="log_hm.txt"

echo "-----------------------------" >> $LOG_FILE
date > $LOG_FILE


current_number=0


mkdir -p $OUTPUT_DIR
mkdir -p $RVT_GPAK_DIR
mkdir -p $TEMP_DIR

cd $TEMP_DIR












echo "Checking/unzipping heightmaps..."


HM_PRIMARY_PATH="$RVT_HGT_DIR_NASADEM"
HM_PRIMARY_PREFIX=""
HM_PRIMARY_SUFFIX=".hgt"
HM_PRIMARY_PREFIX_ZIP=""
HM_PRIMARY_SUFFIX_ZIP=".hgt.zip"
HM_PRIMARY_LOWERCASE=1
HM_PRIMARY_EXT="hgt"

HM_SECONDARY_PATH="$RVT_HGT_DIR_ASTERGDEMV3"
HM_SECONDARY_PREFIX="ASTGTMV003_"
HM_SECONDARY_SUFFIX="_dem.tif"
HM_SECONDARY_PREFIX_ZIP=""
HM_SECONDARY_SUFFIX_ZIP=".zip"
HM_SECONDARY_LOWERCASE=0
HM_SECONDARY_EXT="tif"

HGT_DIR="$TEMP_DIR"



for f in `rsgeotools-conv f 7 81 41`
do
	
	f1="$f"
	if [ "$HM_PRIMARY_LOWERCASE" -eq 1 ]
	then
		f1=`echo "$f" | awk '{print tolower($0)}'`
	fi
	
	f1_zip="${HM_PRIMARY_PATH}/${HM_PRIMARY_PREFIX_ZIP}${f1}${HM_PRIMARY_SUFFIX_ZIP}"
	
	if [ -f $f1_zip ]; then
		# echo "($f) Primary: $f1_zip"
		
		unzip -q -o -j $f1_zip -d $HGT_DIR
		
		mv ${HGT_DIR}/${HM_PRIMARY_PREFIX}${f1}${HM_PRIMARY_SUFFIX} ${HGT_DIR}/$f.${HM_PRIMARY_EXT}
		
		if [ "$HM_PRIMARY_EXT" != "tif" ]
		then
			gdalwarp ${HGT_DIR}/$f.${HM_PRIMARY_EXT} ${HGT_DIR}/$f.tif
			rm ${HGT_DIR}/$f.${HM_PRIMARY_EXT}
		fi
		
	else
	
		f2="$f"
		if [ "$HM_SECONDARY_LOWERCASE" -eq 1 ]
		then
			f2=`echo "$f" | awk '{print tolower($0)}'`
		fi
		
		f2_zip="${HM_SECONDARY_PATH}/${HM_SECONDARY_PREFIX_ZIP}${f2}${HM_SECONDARY_SUFFIX_ZIP}"
	
		if [ -f $f2_zip ]; then
			# echo "($f) Secondary: $f2_zip"
			
			unzip -q -o -j $f2_zip -d $HGT_DIR
			
			mv ${HGT_DIR}/${HM_SECONDARY_PREFIX}${f2}${HM_SECONDARY_SUFFIX} ${HGT_DIR}/$f.${HM_SECONDARY_EXT}

			if [ "$HM_SECONDARY_EXT" != "tif" ]
			then
				gdalwarp ${HGT_DIR}/$f.${HM_SECONDARY_EXT} ${HGT_DIR}/$f.tif
				rm ${HGT_DIR}/$f.${HM_SECONDARY_EXT}
			fi

		fi
	
	fi

done





HGT_PREFIX="./"
HGT_SUFFIX=".tif"


tile_pair_list=(`rsgeotools-conv i $TILE_Z $TILE_X $TILE_Y $DEST_Z_PARENT`)

tile_pair_list_len=${#tile_pair_list[@]}

echo "--- $TILE_Z/$TILE_X/$TILE_Y ($tile_pair_list_len total) ---"

for tile_pair in `rsgeotools-conv i $TILE_Z $TILE_X $TILE_Y $DEST_Z_PARENT`
do
	
	current_number=$(expr $current_number + 1)
	#echo "----- $current_number of $tile_pair_list_len -----"
	echo -n "."


	TP=$(echo $tile_pair | tr "_" "\n")


	subtile_pairs=(`rsgeotools-conv i $DEST_Z_PARENT $TP $DEST_Z`)


	t0_tp=$(echo ${subtile_pairs[0]} | tr "_" "\n")
	t1_tp=$(echo ${subtile_pairs[1]} | tr "_" "\n")
	t2_tp=$(echo ${subtile_pairs[2]} | tr "_" "\n")
	t3_tp=$(echo ${subtile_pairs[3]} | tr "_" "\n")

	t0_cmd="rsgeotools-conv h $DEST_Z ${t0_tp} $IMAGE_SIZE $HGT_PREFIX $HGT_SUFFIX"
	t1_cmd="rsgeotools-conv h $DEST_Z ${t1_tp} $IMAGE_SIZE $HGT_PREFIX $HGT_SUFFIX"
	t2_cmd="rsgeotools-conv h $DEST_Z ${t2_tp} $IMAGE_SIZE $HGT_PREFIX $HGT_SUFFIX"
	t3_cmd="rsgeotools-conv h $DEST_Z ${t3_tp} $IMAGE_SIZE $HGT_PREFIX $HGT_SUFFIX"


	eval $t0_cmd >> $LOG_FILE &
	eval $t1_cmd >> $LOG_FILE &
	eval $t2_cmd >> $LOG_FILE &
	eval $t3_cmd >> $LOG_FILE &
	wait

	t0_cmd="rsgeotools-tiff2hmdata tile_${DEST_Z}_${subtile_pairs[0]}_quad.tif $OUTPUT_DIR/${DEST_Z}_${subtile_pairs[0]}_q.data >> $LOG_FILE"
	t1_cmd="rsgeotools-tiff2hmdata tile_${DEST_Z}_${subtile_pairs[1]}_quad.tif $OUTPUT_DIR/${DEST_Z}_${subtile_pairs[1]}_q.data >> $LOG_FILE"
	t2_cmd="rsgeotools-tiff2hmdata tile_${DEST_Z}_${subtile_pairs[2]}_quad.tif $OUTPUT_DIR/${DEST_Z}_${subtile_pairs[2]}_q.data >> $LOG_FILE"
	t3_cmd="rsgeotools-tiff2hmdata tile_${DEST_Z}_${subtile_pairs[3]}_quad.tif $OUTPUT_DIR/${DEST_Z}_${subtile_pairs[3]}_q.data >> $LOG_FILE"

	eval $t0_cmd >> $LOG_FILE &
	eval $t1_cmd >> $LOG_FILE &
	eval $t2_cmd >> $LOG_FILE &
	eval $t3_cmd >> $LOG_FILE &
	wait


	rm -f ./*_quad.tif

done


cd $OUTPUT_DIR

echo ""

echo -n "Compressing and packing... "


ARCHIVE_FILENAME="${TILE_Z}_${TILE_X}_${TILE_Y}_h.gpak"

packer_cmd="rsgeotools-tilepacker h $TILE_Z $TILE_X $TILE_Y $SUBPAK_Z $DEST_Z $IMAGE_SIZE 0"


eval $packer_cmd


if [ -f $ARCHIVE_FILENAME ]; then
	cp_cmd="cp ${ARCHIVE_FILENAME} ${RVT_GPAK_DIR}/"
	eval $cp_cmd
fi

if test -z "$RVT_KEEP_TEMP_DATA"
then
	rm -rf $OUTPUT_DIR
	rm -rf $TEMP_DIR
fi



cd $DEFAULT_DIR


echo "Done."
echo ""



