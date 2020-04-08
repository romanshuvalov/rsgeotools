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


if test -z "$2"
then
	echo "Usage: cmd PLANET_TIMESTAMP START_STAGE_INDEX"
	exit
fi

PLANET_TIMESTAMP=$1
START_STAGE_INDEX=$2

LOG_FILE="$RVT_GPAK_DIR/planet_process_log_file.txt"

TIME_START=`date +%s`

echo "" >> $LOG_FILE
echo "--------------------" >> $LOG_FILE
echo "Processing planet with timestamp $PLANET_TIMESTAMP, starting from stage $START_STAGE_INDEX" >> $LOG_FILE

if test "0" -ge "$START_STAGE_INDEX"; then
	echo "[ 0 of 12] [2/0/0] Started at `date`" >> $LOG_FILE
	rsgeotools-process-tiles-mass.sh 2 0 0 7 11 12 14 $PLANET_TIMESTAMP
fi

if test "1" -ge "$START_STAGE_INDEX"; then
	echo "[ 1 of 12] [2/1/0] Started at `date`" >> $LOG_FILE
	rsgeotools-process-tiles-mass.sh 2 1 0 7 11 12 14 $PLANET_TIMESTAMP
fi

if test "2" -ge "$START_STAGE_INDEX"; then
	echo "[ 2 of 12] [2/0/1] Started at `date`" >> $LOG_FILE
	rsgeotools-process-tiles-mass.sh 2 0 1 7 11 12 14 $PLANET_TIMESTAMP
fi

if test "3" -ge "$START_STAGE_INDEX"; then
	echo "[ 3 of 12] [2/1/1] Started at `date`" >> $LOG_FILE
	rsgeotools-process-tiles-mass.sh 2 1 1 7 11 12 14 $PLANET_TIMESTAMP
fi




if test "4" -ge "$START_STAGE_INDEX"; then
	echo "[ 4 of 12] [2/2/0] Started at `date`" >> $LOG_FILE
	rsgeotools-process-tiles-mass.sh 2 2 0 7 11 12 14 $PLANET_TIMESTAMP
fi

if test "5" -ge "$START_STAGE_INDEX"; then
	echo "[ 5 of 12] [2/3/0] Started at `date`" >> $LOG_FILE
	rsgeotools-process-tiles-mass.sh 2 3 0 7 11 12 14 $PLANET_TIMESTAMP
fi

if test "6" -ge "$START_STAGE_INDEX"; then
	echo "[ 6 of 12] [2/2/1] Started at `date`" >> $LOG_FILE
	rsgeotools-process-tiles-mass.sh 2 2 1 7 11 12 14 $PLANET_TIMESTAMP
fi

if test "7" -ge "$START_STAGE_INDEX"; then
	echo "[ 7 of 12] [2/3/1] Started at `date`" >> $LOG_FILE
	rsgeotools-process-tiles-mass.sh 2 3 1 7 11 12 14 $PLANET_TIMESTAMP
fi




if test "8" -ge "$START_STAGE_INDEX"; then
	echo "[ 8 of 12] [2/0/2] Started at `date`" >> $LOG_FILE
	rsgeotools-process-tiles-mass.sh 2 0 2 7 11 12 14 $PLANET_TIMESTAMP
fi

if test "9" -ge "$START_STAGE_INDEX"; then
	echo "[ 9 of 12] [2/1/2] Started at `date`" >> $LOG_FILE
	rsgeotools-process-tiles-mass.sh 2 1 2 7 11 12 14 $PLANET_TIMESTAMP
fi

if test "10" -ge "$START_STAGE_INDEX"; then
	echo "[10 of 12] [2/2/2] Started at `date`" >> $LOG_FILE
	rsgeotools-process-tiles-mass.sh 2 2 2 7 11 12 14 $PLANET_TIMESTAMP
fi

if test "11" -ge "$START_STAGE_INDEX"; then
	echo "[11 of 12] [2/3/2] Started at `date`" >> $LOG_FILE
	rsgeotools-process-tiles-mass.sh 2 3 2 7 11 12 14 $PLANET_TIMESTAMP
fi

echo "Done at `date`"  >> $LOG_FILE

TIME_END=`date +%s`
TIME_DIFF_HOURS=$(( ($TIME_END - $TIME_START)/(60*60) ))
TIME_DIFF_DAYS=$(( $TIME_DIFF_HOURS/(24) ))

echo "Total $TIME_DIFF_HOURS hours ($TIME_DIFF_DAYS days). " >> $LOG_FILE

echo ""  >> $LOG_FILE




