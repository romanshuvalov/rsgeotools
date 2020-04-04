#!/bin/bash

START_DATE=`date`

IFS=$'\n'

DEFAULT_DIR=`pwd`

if test -z "$RVT_O5M_DIR"
then
	echo "RVT_O5M_DIR is not set. "
	exit
fi

if test -z "$1"
then
	echo "Usage: rsgeotools-planet-init.sh <PBF_FILENAME>"
	exit
fi


PBF_FILENAME=$1


echo "PBF to O5M..."
date
osmconvert --drop-author --drop-version $PBF_FILENAME -o=$RVT_O5M_DIR/planet.o5m


echo "Filtering O5M..."
date

osmfilter $RVT_O5M_DIR/planet.o5m --drop-tags="created_by= converted_by= source*= lacounty*= gnis*= tiger*= NHD*= nhd*= time= ele= note= openGeoDB*= fixme= FIXME= is_in= is_in*= wikipedia*= wikidata*= website*= media*= description*= addr:street*= addr:city*= addr:postcode*= addr:country*= addr:place*= addr:suburb*= addr:state*= addr:province*= addr:cons*= addr:muni*= addr:interp*= addr:distric*= addr:sub*=" -o=$RVT_O5M_DIR/t0_0_0.o5m

rm $RVT_O5M_DIR/planet.o5m

cd $RVT_O5M_DIR

echo "-----------------"
echo "z1..."
date
RVT_SINGLE_THREAD=1 rsgeotools-subdiv-o5m.sh 0 0 0 1
rm t0_0_0.o5m

echo "-----------------"
echo "z2..."
date
RVT_SINGLE_THREAD=1 rsgeotools-subdiv-o5m.sh 1 0 0 2
rm t1_0_0.o5m
RVT_SINGLE_THREAD=1 rsgeotools-subdiv-o5m.sh 1 0 1 2
rm t1_0_1.o5m
RVT_SINGLE_THREAD=1 rsgeotools-subdiv-o5m.sh 1 1 0 2
rm t1_1_0.o5m
RVT_SINGLE_THREAD=1 rsgeotools-subdiv-o5m.sh 1 1 1 2
rm t1_1_1.o5m

echo "-----------------"
echo "z3..."
date
RVT_SINGLE_THREAD=1 rsgeotools-subdiv-o5m.sh 0 0 0 3
rm t2_*

echo "-----------------"
echo "z4..."
date
RVT_SINGLE_THREAD=1 rsgeotools-subdiv-o5m.sh 0 0 0 4
rm t3_*

echo "-----------------"
echo "z5..."
date
RVT_SINGLE_THREAD=1 rsgeotools-subdiv-o5m.sh 0 0 0 5
rm t4_*

echo "-----------------"
echo "z6..."
date
RVT_SINGLE_THREAD=1 rsgeotools-subdiv-o5m.sh 0 0 0 6
rm t5_*

echo "-----------------"
echo "z7..."
date
RVT_SINGLE_THREAD=1 rsgeotools-subdiv-o5m.sh 0 0 0 7
rm t6_*

cd $DEFAULT_DIR

echo "------"

END_DATE=`date`
echo "Start: $START_DATE"
echo "End:   $END_DATE"
echo "Done."
echo ""


