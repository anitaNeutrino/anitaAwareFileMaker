#!/bin/bash
TDRSS_DIR=/data/palestine2016/telem/fast_tdrss

while [ 1 ] ; do
    scp tdrss:/data/anita/fast_tdrss/.stor_cur_run $TDRSS_DIR
    CURRENT_RUN=`cat $TDRSS_DIR/.stor_cur_run`    
    date
    echo $CURRENT_RUN 
    rsync -avz tdrss:/data/anita/fast_tdrss/${CURRENT_RUN} $TDRSS_DIR > /tmp/lastStreamTDRSS 2>&1

    sleep 10
done
