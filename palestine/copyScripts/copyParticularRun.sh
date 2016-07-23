#!/bin/bash

RAW_DATA_DIR=/anitaStorage/antarctica14/raw/
run=$1
echo "Copying run " $run "..."
rsync -av anita@flight:/mnt/helium1/run${run} $RAW_DATA_DIR  #> /tmp/getRawData.log
if  test `cat /tmp/getRawData.log  | wc -l` -gt 04 ; then
    if [ -d "$RAW_DATA_DIR/run$run" ]; then
	echo "Copied Run $run"
	echo  $run > lastCopiedRun
	
	#Insert something here
	echo "Trying to ROOTify..." $run "..."
	/home/radio/anita14/anitaAwareFileMaker/antarctica/processRun.sh $run > /tmp/processRun.log
	/home/radio/anita14/copyScripts/updateRunLogDB $run $run > /tmp/updateRunLogDB.log
	
    fi
fi




