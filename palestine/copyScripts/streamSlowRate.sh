#!/bin/bash
TDRSS_DIR=/data/palestine2016/telem/slow_tdrss
IRIDIUM_DIR=/data/palestine2016/telem/iridium

while [ 1 ] ; do
    scp tdrss:/data/anita/slow_tdrss/.stor_cur_run $TDRSS_DIR
    CURRENT_RUN=`cat $TDRSS_DIR/.stor_cur_run`
    date
    echo $CURRENT_RUN 
    rsync -avz tdrss:/data/anita/slow_tdrss/${CURRENT_RUN} $TDRSS_DIR > /tmp/lastStreamSlowTDRSS 2>&1

    scp tdrss:/data/anita/iridium/.stor_cur_run $IRIDIUM_DIR
    CURRENT_RUN=`cat $IRIDIUM_DIR/.stor_cur_run`
    date
    echo $CURRENT_RUN 
    rsync -avz tdrss:/data/anita/iridium/${CURRENT_RUN} $IRIDIUM_DIR > /tmp/lastStreamIRIDIUM 2>&1

    


    sleep 10
done
