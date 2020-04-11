#!/bin/bash

HM_PRIMARY_PATH="/home/romik/data/hm/nasadem/zips"
HM_PRIMARY_PREFIX=""
HM_PRIMARY_SUFFIX=".hgt"
HM_PRIMARY_PREFIX_ZIP=""
HM_PRIMARY_SUFFIX_ZIP=".hgt.zip"
HM_PRIMARY_LOWERCASE=1
HM_PRIMARY_EXT="hgt"

HM_SECONDARY_PATH="/home/romik/data/hm/zips"
HM_SECONDARY_PREFIX="ASTGTMV003_"
HM_SECONDARY_SUFFIX="_dem.tif"
HM_SECONDARY_PREFIX_ZIP=""
HM_SECONDARY_SUFFIX_ZIP=".zip"
HM_SECONDARY_LOWERCASE=0
HM_SECONDARY_EXT="tif"

HGT_DIR="/home/romik/hgt"



for f in `rsgeotools-conv f 7 81 41`
do
	
	f1="$f"
	if [ "$HM_PRIMARY_LOWERCASE" -eq 1 ]
	then
		f1=`echo "$f" | awk '{print tolower($0)}'`
	fi
	
	f1_zip="${HM_PRIMARY_PATH}/${HM_PRIMARY_PREFIX_ZIP}${f1}${HM_PRIMARY_SUFFIX_ZIP}"
	
	if [ -f $f1_zip ]; then
		echo "($f) Primary: $f1_zip"
		
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
			echo "($f) Secondary: $f2_zip"
			
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




echo "Done."


