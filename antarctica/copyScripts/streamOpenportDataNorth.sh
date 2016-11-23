#/bin/bash

OPENPORT_DIR=/storage/flight1415/telem/openport
OPENPORT_HOST=tdrss
OPENPORT_REMOTE_DIR=/data/anita/openport/

while [ 1 ] ; do
    CURRENT_RUN=`ls $OPENPORT_DIR | tail -1`
    echo $CURRENT_RUN #$PREVIOUS_RUN
    rsync -avz --exclude '.*' $OPENPORT_DIR/$CURRENT_RUN ${OPENPORT_HOST}:${OPENPORT_REMOTE_DIR} # > /tmp/lastStreamOpenport 2>&1

    sleep 10
done
