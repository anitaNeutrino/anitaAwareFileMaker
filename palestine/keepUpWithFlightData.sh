#!/bin/bash

cd /home/anita/Code/anitaAwareFileMaker/palestine

while true; do
    echo "rsync every 30 mins until you stop me"
    ./copyDataFromFlight.sh 
    sleep 60
done

