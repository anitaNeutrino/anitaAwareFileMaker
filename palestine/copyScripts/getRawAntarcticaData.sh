#!/bin/bash
cd /home/radio/anita14/copyScripts
RAW_DATA_DIR=/anitaStorage/antarctica14/raw/

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
		/home/radio/anita14/anitaAwareFileMaker/antarctica/processRun.sh $run > /tmp/processRun.log
		/home/radio/anita14/copyScripts/updateRunLogDB $run $run > /tmp/updateRunLogDB.log
	    fi
	fi
    done
    sleep 10
done



