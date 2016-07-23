#/bin/bash

LOS_DIR=/data/palestine2016/telem/raw_los

while [ 1 ] ; do
    scp los:/data/anita/raw_los/.stor_cur_run $LOS_DIR
    CURRENT_RUN=`cat $LOS_DIR/.stor_cur_run`
 #   let PREVIOUS_RUN=${CURRENT_RUN}-1
    date
    echo "Current Run: $CURRENT_RUN" #$PREVIOUS_RUN
    rsync -avz los:/data/anita/raw_los/${CURRENT_RUN} $LOS_DIR > /tmp/lastStreamLOS 2>&1

    sleep 10
done
