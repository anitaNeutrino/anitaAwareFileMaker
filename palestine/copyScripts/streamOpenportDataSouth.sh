#!/bin/bash
OPENPORT_DIR=/data/palestine2016/telem/openport

echo "PID = $$"
echo $$ > /tmp/pidOpenportCopy

while [ 1 ] ; do
    rsync -avz tdrss1:/data/anita/openport/* $OPENPORT_DIR

    sleep 10
done
