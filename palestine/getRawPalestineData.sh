#!/bin/bash
cd /home/anita/palestine
RAW_DATA_DIR=/data/palestine2016/raw/

while :
do

    rsync -a anita@flight:/mnt/data/numbers/lastRunNumber .
    LAST_RUN=`cat lastRunNumber`
    START_RUN=`cat lastCopiedRun`
    PENULTIMATE_RUN=`expr $LAST_RUN - 1`
    echo "START and LAST $START_RUN $LAST_RUN"
    for run in `seq $START_RUN $PENULTIMATE_RUN`; do
	rsync -av anita@flight:/mnt/helium1/run${run} $RAW_DATA_DIR  > /tmp/getRawData.log
	if  test `cat /tmp/getRawData.log  | wc -l` -gt 04 ; then
	    if [ -d "$RAW_DATA_DIR/run$run" ]; then
		echo "Copied Run $run"
		echo  $run > lastCopiedRun
		
		#Insert something here
		./processRunHk.sh $run > /tmp/processRun.log
#		./updateRunLogDB $run $run > /tmp/updateRunLogDB.log
	    fi
	fi
    done
    sleep 10
done



