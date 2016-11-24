#!/bin/bash
# Simple script to stream slow rate data for AWARE
# TDRSS_DIR is local directory for storing slow rate TDRSS telem
# IRIDIUM_DIR is local directory for storing slow rate Iridium telem
# TDRSS_HOST is the hostname (or .ssh/config alias) of the machine receiving
# the TDRSS data. This machine needs to be set up for passwordless ssh
# i.e. we must have used ssh-copy-id to copy this machines public ssh key
# to the remote machines .ssh/authorized_keys

TDRSS_DIR=/data/ldb2016/telem/slow_tdrss
IRIDIUM_DIR=/data/ldb2016/telem/iridium
#TDRSS_HOST=anitaxfer
TDRSS_HOST=tdrss1

echo "PID = $$"
echo $$ > /tmp/pidSlowCopy

while [ 1 ] ; do
    scp ${TDRSS_HOST}:/data/anita/slow_tdrss/.stor_cur_run $TDRSS_DIR
    CURRENT_RUN=`cat $TDRSS_DIR/.stor_cur_run`
    date
    echo $CURRENT_RUN 
    rsync -avz ${TDRSS_HOST}:/data/anita/slow_tdrss/${CURRENT_RUN} $TDRSS_DIR > /tmp/lastStreamSlowTDRSS 2>&1

    scp ${TDRSS_HOST}:/data/anita/iridium/.stor_cur_run $IRIDIUM_DIR
    CURRENT_RUN=`cat $IRIDIUM_DIR/.stor_cur_run`
    date
    echo $CURRENT_RUN 
    rsync -avz ${TDRSS_HOST}:/data/anita/iridium/${CURRENT_RUN} $IRIDIUM_DIR > /tmp/lastStreamIRIDIUM 2>&1

    sleep 10
done
