#/bin/bash
# Simple script to stream LOS data for AWARE
# LOS_DIR is local directory for storing LOS telem
# LOS_HOST is the hostname (or .ssh/config alias) of the machine receiving
# the LOS data. This machine needs to be set up for passwordless ssh
# i.e. we must have used ssh-copy-id to copy this machines public ssh key
# to the remote machines .ssh/authorized_keys 

LOS_DIR=/data/palestine2016/telem/raw_los
LOS_HOST=los

while [ 1 ] ; do
    scp ${LOS_HOST}:/data/anita/raw_los/.stor_cur_run $LOS_DIR
    CURRENT_RUN=`cat $LOS_DIR/.stor_cur_run`
 #   let PREVIOUS_RUN=${CURRENT_RUN}-1
    date
    echo "Current Run: $CURRENT_RUN" #$PREVIOUS_RUN
    rsync -avz ${LOS_HOST}:/data/anita/raw_los/${CURRENT_RUN} $LOS_DIR > /tmp/lastStreamLOS 2>&1

    sleep 10
done
