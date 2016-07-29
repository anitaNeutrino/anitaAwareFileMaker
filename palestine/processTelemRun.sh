#!/bin/bash
if [ "$1" = "" ]
then
   echo "usage: `basename $0` <run no> <skip root>" 1>&2
   exit 1
fi



RUN=$1
BASE_DIR=/data/palestine2016/telem
EVENT_BASE_DIR=${BASE_DIR}/root
ROOT_RUN_DIR=${EVENT_BASE_DIR}/run${RUN}

#Step 2: Generate the AWARE Files
cd /home/anita/Code/anitaAwareFileMaker/
source setupAwareVariables.sh
if [ -d "$ROOT_RUN_DIR" ]; then
    ./makeHeaderJsonFiles ${ROOT_RUN_DIR}/headFile${RUN}.root 
    ./makeHkJsonFiles ${ROOT_RUN_DIR}/hkFile${RUN}.root
    
    ./makeSurfHkJsonFiles ${ROOT_RUN_DIR}/surfHkFile${RUN}.root    
    ./makeAcqdStartRunJsonFiles ${ROOT_RUN_DIR}/auxFile${RUN}.root
    ./makeAdu5PatJsonFiles ${ROOT_RUN_DIR}/gpsFiles${RUN}.root 0
    ./makeAdu5PatJsonFiles ${ROOT_RUN_DIR}/gpsFiles${RUN}.root 1
    ./makeAdu5SatJsonFiles ${ROOT_RUN_DIR}/gpsFiles${RUN}.root 0
    ./makeAdu5SatJsonFiles ${ROOT_RUN_DIR}/gpsFiles${RUN}.root 1
    ./makeAdu5VtgJsonFiles ${ROOT_RUN_DIR}/gpsFiles${RUN}.root 0
    ./makeAdu5VtgJsonFiles ${ROOT_RUN_DIR}/gpsFiles${RUN}.root 1
    ./makeMonitorHkJsonFiles ${ROOT_RUN_DIR}/monitorFile${RUN}.root     
    ./makeOtherMonitorHkJsonFiles ${ROOT_RUN_DIR}/monitorFile${RUN}.root  
    ./makeG12PosJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root
    ./makeG12SatJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root
    ./makeGpsGgaJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root 0
    ./makeGpsGgaJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root 1
    ./makeGpsGgaJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root 2
    ./makeTurfRateJsonFiles ${ROOT_RUN_DIR}/turfRateFile${RUN}.root
    ./makeSumTurfRateJsonFiles ${ROOT_RUN_DIR}/sumTurfRateFile${RUN}.root
    ./makeAvgSurfHkJsonFiles ${ROOT_RUN_DIR}/avgSurfHkFile${RUN}.root
    
#    ./makeWaveformSummaryJsonFiles ${ROOT_RUN_DIR}/eventHeadFile${RUN}.root ${ROOT_RUN_DIR}/eventFile${RUN}.root
#    ./makeEventJsonFiles ${ROOT_RUN_DIR}/headFile${RUN}.root ${ROOT_RUN_DIR}/eventFile${RUN}.root  &
fi
