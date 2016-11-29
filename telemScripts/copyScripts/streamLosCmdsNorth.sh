#!/bin/bash
# Simple script to stream los commands northafor AWARE
# CMD_SEND_DIR is the remote directory
# LOCAL_CMD_LOG_DIR is the local directory
# IRIDIUM_DIR is local directory for storing slow rate Iridium telem
# TDRSS_HOST is the hostname (or .ssh/config alias) of the machine receiving
# the TDRSS data. This machine needs to be set up for passwordless ssh
# i.e. we must have used ssh-copy-id to copy this machines public ssh key
# to the remote machines .ssh/authorized_keys

CMD_SEND_DIR=/home/anita/losLog
LOCAL_CMD_LOG_DIR=/home/anita/Code/anitaCmdSend/jsonLog/
#TDRSS_HOST=anitaxfer
TDRSS_HOST=tdrss1

echo "PID = $$"
echo $$ > /tmp/pidStreamCmdNorth

while [ 1 ] ; do
    rsync -avz ${LOCAL_CMD_LOG_DIR}/*.json ${TDRSS_HOST}:${CMD_SEND_DIR} > /tmp/lastStreamCmdNorth.log 2>&1
    sleep 10
done
