#!/bin/bash
OPENPORT_DIR=/storage/flight1415/telem/openport

while [ 1 ] ; do
    rsync -avz tdrss:/data/anita/openport/00[6789]* $OPENPORT_DIR

    sleep 10
done
