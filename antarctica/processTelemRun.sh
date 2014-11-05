#!/bin/bash
if [ "$1" = "" ]
then
   echo "usage: `basename $0` <run no> <skip root>" 1>&2
   exit 1
fi



RUN=$1
BASE_DIR=/anitaStorage/antarctica14/telem
EVENT_BASE_DIR=${BASE_DIR}/root
ROOT_RUN_DIR=${EVENT_BASE_DIR}/run${RUN}

#Step 2: Generate the AWARE Files
cd /home/radio/anita14/anitaAwareFileMaker/
source setupAwareVariables.sh
if [ -d "$ROOT_RUN_DIR" ]; then
    ./makeHeaderJsonFiles ${ROOT_RUN_DIR}/headFile${RUN}.root 
    ./makePrettyHkJsonFiles ${ROOT_RUN_DIR}/hkFile${RUN}.root  
#    ./makeSurfHkJsonFiles ${ROOT_RUN_DIR}/surfHkFile${RUN}.root    
#    ./makeAcqdStartRunJsonFiles ${ROOT_RUN_DIR}/auxFile${RUN}.root     
    ./makeMonitorHkJsonFiles ${ROOT_RUN_DIR}/monitorFile${RUN}.root     
    ./makeOtherMonitorHkJsonFiles ${ROOT_RUN_DIR}/monitorFile${RUN}.root  
    ./makeG12PosJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root
    ./makeG12SatJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root
#    ./makeEventJsonFiles ${ROOT_RUN_DIR}/headFile${RUN}.root ${ROOT_RUN_DIR}/eventFile${RUN}.root  &
fi
