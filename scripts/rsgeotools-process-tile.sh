#!/bin/sh

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


if test -z "$7"
then
	echo "Usage: cmd TOP_Z TOP_X TOP_Y PROCESS_Z SUBPAK_Z DEST_Z PLANET_TIMESTAMP"
	exit
fi

TOP_Z=$1
TOP_X=$2
TOP_Y=$3

PROCESS_Z=$4

SUBPAK_Z=$5

DEST_Z=$6

PLANET_TIMESTAMP=$7


# DEST_Z should be 14, PROCESS_Z 11, SUBPAK_Z 12, TOP_Z must be 7

DEFAULT_DIR=`pwd`
TEMP_DIR="$RVT_TEMP_DIR/rvt_temp_${TOP_Z}_${TOP_X}_${TOP_Y}"

SRC_Z=7

mkdir -p $RVT_GPAK_DIR

rm -rf $TEMP_DIR
mkdir -p $TEMP_DIR

cp $RVT_O5M_DIR/t${TOP_Z}_${TOP_X}_${TOP_Y}.o5m $TEMP_DIR/
cp $RVT_CSV_CONF $TEMP_DIR/

cd $TEMP_DIR

RVT_SINGLE_THREAD=1 rsgeotools-subdiv-o5m.sh $TOP_Z $TOP_X $TOP_Y 8
rm -r t7_*

RVT_SINGLE_THREAD=1 rsgeotools-subdiv-o5m.sh $TOP_Z $TOP_X $TOP_Y 9
rm -r t8_*

rsgeotools-subdiv-o5m.sh $TOP_Z $TOP_X $TOP_Y 10
rm -r t9_*

rsgeotools-subdiv-o5m.sh $TOP_Z $TOP_X $TOP_Y 11
rm -r t10_*


tar -x -f $RVT_SHP_ARCHIVE_DIR/ocean/ocean${TOP_Z}_${TOP_X}_${TOP_Y}.tar.gz


for tile_pair in `geoconv I $TOP_Z $TOP_X $TOP_Y $PROCESS_Z`
do

	TP=$(echo $tile_pair | tr "_" " ");

	FILENAME=t${PROCESS_Z}_${tile_pair}
	
	OCEAN_FILENAME=ocean${PROCESS_Z}_${tile_pair}

#	echo "osmconvert o5m -> pbf..."
	osmconvert $FILENAME.o5m -o=${FILENAME}.pbf

#	echo "o5m -> outlines..."
	cat ${FILENAME}.o5m | rsgeotools-o5m-get-outlines > ${FILENAME}_outlines.txt

	CSV_DIR=$TEMP_DIR/csv_out
	
	CSV_DIR_OCEAN=$TEMP_DIR/csv_out_ocean

#	echo "ogr2ogr -> WKT..."

	rm -rf $CSV_DIR
	ogr2ogr -f CSV -t_srs EPSG:4326 $CSV_DIR $TEMP_DIR/${FILENAME}.pbf -lco GEOMETRY=AS_WKT --config OSM_CONFIG_FILE $RVT_CSV_CONF 
	
	rm -rf $CSV_DIR_OCEAN
	OCEAN_PARAM=""
	if test -f "$TEMP_DIR/${OCEAN_FILENAME}.shp"
	then
		ogr2ogr -f CSV $CSV_DIR_OCEAN $TEMP_DIR/${OCEAN_FILENAME}.shp -lco GEOMETRY=AS_WKT 
		OCEAN_PARAM="${CSV_DIR_OCEAN}/${OCEAN_FILENAME}.csv"
	fi

#	echo "geocsv2db (b, a, l, p) $PROCESS_Z $TP $DEST_Z..."

	geocsv2db b $PROCESS_Z $TP $DEST_Z ${CSV_DIR}/multipolygons.csv ${FILENAME}_outlines.txt
	geocsv2db a $PROCESS_Z $TP $DEST_Z ${CSV_DIR}/multipolygons.csv $OCEAN_PARAM
	geocsv2db l $PROCESS_Z $TP $DEST_Z ${CSV_DIR}/lines.csv 
	geocsv2db p $PROCESS_Z $TP $DEST_Z ${CSV_DIR}/points.csv 

	rm ${FILENAME}.pbf
	
	if test -z "$RVT_KEEP_TEMP_DATA"
	then
		rm ${FILENAME}.o5m	
		rm -f ${FILENAME}_outlines.txt
		rm -f ${OCEAN_FILENAME}*
	fi

done

echo "geotilepacker g $TOP_Z $TOP_X $TOP_Y $DEST_Z..."

geotilepacker_cmd="geotilepacker g $TOP_Z $TOP_X $TOP_Y $SUBPAK_Z $DEST_Z 256 $PLANET_TIMESTAMP"
echo "Executing: $geotilepacker_cmd"
eval $geotilepacker_cmd

cp ${TOP_Z}_${TOP_X}_${TOP_Y}_g.gpak $RVT_GPAK_DIR/

if test -z "$RVT_KEEP_TEMP_DATA"
then
	rm *.data
fi

cd $DEFAULT_DIR

if test -z "$RVT_KEEP_TEMP_DATA"
then
	rm -r $TEMP_DIR
fi


