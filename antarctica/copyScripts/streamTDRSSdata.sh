#!/bin/bash
# Simple script to stream TDRSS data for AWARE
# TDRSS_DIR is local directory for storing telem
# TDRSS_HOST is the hostname (or .ssh/config alias) of the machine receiving
# the TDRSS data. This machine needs to be set up for passwordless ssh
# i.e. we must have used ssh-copy-id to copy this machines public ssh key
# to the remote machines .ssh/authorized_keys

TDRSS_DIR=/data/ldb2016/telem/fast_tdrss
TDRSS_HOST=anitaxfer
#TDRSS_HOST=tdrss1


echo "PID = $$"
echo $$ > /tmp/pidTDRSSCopy

while [ 1 ] ; do
    scp ${TDRSS_HOST}:/data/anita/fast_tdrss/.stor_cur_run $TDRSS_DIR
    CURRENT_RUN=`cat $TDRSS_DIR/.stor_cur_run`    
    date
    echo $CURRENT_RUN 
    rsync -avz ${TDRSS_HOST}:/data/anita/fast_tdrss/${CURRENT_RUN} $TDRSS_DIR > /tmp/lastStreamTDRSS 2>&1

    sleep 10
done
