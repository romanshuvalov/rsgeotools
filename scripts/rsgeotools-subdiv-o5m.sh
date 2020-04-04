#!/bin/bash


if test -z "$RVT_O5M_DIR"
then
	echo "RVT_O5M_DIR is not set. "
	exit
fi


IFS=$'\n'

TOP_Z=$1
TOP_X=$2
TOP_Y=$3

TILE_Z=$4
TILE_Z_PARENT=$(expr $TILE_Z - 1)

OUTPUT_DIR="."


if test -z "$4"
then
	echo "Usage: rsgeotools-subdiv-o5m.sh TOP_Z TOP_X TOP_Y DEST_Z "
	exit
fi


current_number=0

for tile_pair in `rsgeotools-conv I $TOP_Z $TOP_X $TOP_Y $TILE_Z_PARENT`
do
	

	current_number=$(expr $current_number + 1)

	TP=$(echo $tile_pair | tr "_" "\n")


	echo "---- #$current_number (zoom $TILE_Z_PARENT) ($tile_pair) ----"

	subtile_pairs=(`rsgeotools-conv i $TILE_Z_PARENT $TP $TILE_Z`)

	t0_tp=$(echo ${subtile_pairs[0]} | tr "_" "\n")
	t1_tp=$(echo ${subtile_pairs[1]} | tr "_" "\n")
	t2_tp=$(echo ${subtile_pairs[2]} | tr "_" "\n")
	t3_tp=$(echo ${subtile_pairs[3]} | tr "_" "\n")

	t0_cmd="osmconvert t${TILE_Z_PARENT}_${tile_pair}.o5m -b=`rsgeotools-conv D $TILE_Z $t0_tp` -o=t${TILE_Z}_${subtile_pairs[0]}.o5m --complex-ways"
	t1_cmd="osmconvert t${TILE_Z_PARENT}_${tile_pair}.o5m -b=`rsgeotools-conv D $TILE_Z $t1_tp` -o=t${TILE_Z}_${subtile_pairs[1]}.o5m --complex-ways"
	t2_cmd="osmconvert t${TILE_Z_PARENT}_${tile_pair}.o5m -b=`rsgeotools-conv D $TILE_Z $t2_tp` -o=t${TILE_Z}_${subtile_pairs[2]}.o5m --complex-ways"
	t3_cmd="osmconvert t${TILE_Z_PARENT}_${tile_pair}.o5m -b=`rsgeotools-conv D $TILE_Z $t3_tp` -o=t${TILE_Z}_${subtile_pairs[3]}.o5m --complex-ways"

	if test -z "$RVT_SINGLE_THREAD"
	then
		eval $t0_cmd &
		eval $t1_cmd &
		eval $t2_cmd &
		eval $t3_cmd &
		wait
	else
		#echo "Executing [$t0_cmd]..."
		eval $t0_cmd
		#echo "Executing [$t1_cmd]..."
		eval $t1_cmd
		#echo "Executing [$t2_cmd]..."
		eval $t2_cmd
		#echo "Executing [$t3_cmd]..."
		eval $t3_cmd
	fi


done



echo "Done."


