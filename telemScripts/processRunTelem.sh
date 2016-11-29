#!/bin/bash
if [ "$1" = "" ]
then
   echo "usage: `basename $0` <run no> <skip root>" 1>&2
   exit 1
fi


RUN=$1
BASE_DIR=${ANITA_TELEM_DATA_DIR}
RAW_RUN_DIR=${BASE_DIR}/raw/run${RUN}
EVENT_BASE_DIR=${BASE_DIR}/root
ROOT_RUN_DIR=${EVENT_BASE_DIR}/run${RUN}

#Generate the AWARE Files
#cd $ANITA_AWARE_FILEMAKER_DIR

if [ -d "$ROOT_RUN_DIR" ]; then
    echo "Header"
    echo "=========================================="
    makeHeaderJsonFiles ${ROOT_RUN_DIR}/headFile${RUN}.root 
    echo "Hk"
    echo "=========================================="
    makeHkJsonFiles ${ROOT_RUN_DIR}/hkFile${RUN}.root  
    makeSSHkJsonFiles ${ROOT_RUN_DIR}/sshkFile${RUN}.root  
    echo "SURF Hk"
    echo "=========================================="
    makeSurfHkJsonFiles ${ROOT_RUN_DIR}/surfHkFile${RUN}.root
    echo "Avg. SURF Hk"
    echo "=========================================="
    makeAvgSurfHkJsonFiles ${ROOT_RUN_DIR}/avgSurfHkFile${RUN}.root    
    echo "TURF Rate"
    echo "=========================================="
    makeTurfRateJsonFiles ${ROOT_RUN_DIR}/turfRateFile${RUN}.root    
    echo "Acqd"
    echo "=========================================="
    makeAcqdStartRunJsonFiles ${ROOT_RUN_DIR}/auxFile${RUN}.root     
    echo "Monitor"
    echo "=========================================="
    makeMonitorHkJsonFiles ${ROOT_RUN_DIR}/monitorFile${RUN}.root     
    echo "Other"
    echo "=========================================="
    makeOtherMonitorHkJsonFiles ${ROOT_RUN_DIR}/monitorFile${RUN}.root 
    echo "GPS"
    echo "=========================================="
    makeAdu5PatJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root 0
    makeAdu5PatJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root 1
    makeAdu5SatJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root 0
    makeAdu5SatJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root 1
    makeAdu5VtgJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root 0
    makeAdu5VtgJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root 1
    makeG12PosJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root
    makeG12SatJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root
    makeGpsGgaJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root 0
    makeGpsGgaJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root 1
    makeGpsGgaJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root 2
#    makeEventJsonFiles ${ROOT_RUN_DIR}/headFile${RUN}.root ${ROOT_RUN_DIR}/eventFile${RUN}.root  &
fi
