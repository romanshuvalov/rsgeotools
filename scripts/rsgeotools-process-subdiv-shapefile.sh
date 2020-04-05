#!/bin/sh

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


if test -z "$4"
then
	echo "Usage: rsgeotools-process-subdiv-shapefile.sh TOP_Z TOP_X TOP_Y PREFIX"
	exit
fi

TOP_Z=$1
TOP_X=$2
TOP_Y=$3

SHAPEFILE_PREFIX=$4


DEFAULT_DIR=`pwd`

# SHP_DIR=$DEFAULT_DIR

mkdir -p $RVT_TEMP_DIR

TEMP_DIR="$RVT_TEMP_DIR/shp_temp_${SHAPEFILE_PREFIX}${TOP_Z}_${TOP_X}_${TOP_Y}"

rm -rf $TEMP_DIR
mkdir -p $TEMP_DIR

mkdir -p $RVT_SHP_ARCHIVE_DIR
mkdir -p $RVT_SHP_ARCHIVE_DIR/$SHAPEFILE_PREFIX

cp $RVT_SHP_DIR/${SHAPEFILE_PREFIX}${TOP_Z}_${TOP_X}_${TOP_Y}* $TEMP_DIR/

cd $TEMP_DIR

rsgeotools-subdiv-shapefile.sh $TOP_Z $TOP_X $TOP_Y 8 $SHAPEFILE_PREFIX

rsgeotools-subdiv-shapefile.sh $TOP_Z $TOP_X $TOP_Y 9 $SHAPEFILE_PREFIX
rm -r ${SHAPEFILE_PREFIX}8*

rsgeotools-subdiv-shapefile.sh $TOP_Z $TOP_X $TOP_Y 10 $SHAPEFILE_PREFIX
rm -r ${SHAPEFILE_PREFIX}9*

rsgeotools-subdiv-shapefile.sh $TOP_Z $TOP_X $TOP_Y 11 $SHAPEFILE_PREFIX
rm -r ${SHAPEFILE_PREFIX}10*

TAR_FILENAME="${SHAPEFILE_PREFIX}${TOP_Z}_${TOP_X}_${TOP_Y}.tar.gz"

tar -cf $TAR_FILENAME -z ${SHAPEFILE_PREFIX}11*

cp $TAR_FILENAME $RVT_SHP_ARCHIVE_DIR/$SHAPEFILE_PREFIX/

echo "Process subdiv shapefile done."

cd $DEFAULT_DIR

if test -z "$SHAPEFILE_KEEP_TEMP_DATA"
then
	rm -r $TEMP_DIR
fi


