#!/bin/bash
if [ "$1" = "" ]
then
   echo "usage: `basename $0` <run no> <skip root>" 1>&2
   exit 1
fi


#Local config directory
BASE_DIR=/data/palestine2016
AWARE_FILEMAKER_DIR=/home/anita/Code/anitaAwareFileMaker
ANITA_FILEMAKER_DIR=/home/anita/Code/anitaTreeMaker
export PYTHONPATH=/home/anita/Code/aware/python

RUN=$1
RAW_RUN_DIR=${BASE_DIR}/raw/run${RUN}
RAW_CONFIG_DIR=${RAW_RUN_DIR}/config
EVENT_BASE_DIR=${BASE_DIR}/root
ROOT_RUN_DIR=${EVENT_BASE_DIR}/run${RUN}

#Step 1: Generate the ROOT Files


if [ "$2" = "" ]
then
    cd ${ANITA_FILEMAKER_DIR}
    ./remakeHkFiles.sh $RUN
fi


cd ${AWARE_FILEMAKER_DIR}
source setupAwareVariables.sh

#Step 2: Deal with the config files
for configFile in ${RAW_CONFIG_DIR}/*.config; do
    python ./processConfig.py -i $configFile -r $RUN
done


#Step 2: Generate the AWARE Files

if [ -d "$ROOT_RUN_DIR" ]; then
    echo "Header"
    echo "=========================================="
    ./makeHeaderJsonFiles ${ROOT_RUN_DIR}/headFile${RUN}.root 
    echo "Hk"
    echo "=========================================="
    ./makeHkJsonFiles ${ROOT_RUN_DIR}/hkFile${RUN}.root  
    ./makeSSHkJsonFiles ${ROOT_RUN_DIR}/sshkFile${RUN}.root  
    echo "SURF Hk"
    echo "=========================================="
    ./makeSurfHkJsonFiles ${ROOT_RUN_DIR}/surfHkFile${RUN}.root
    echo "Avg. SURF Hk"
    echo "=========================================="
    ./makeAvgSurfHkJsonFiles ${ROOT_RUN_DIR}/avgSurfHkFile${RUN}.root    
    echo "TURF Rate"
    echo "=========================================="
    ./makeTurfRateJsonFiles ${ROOT_RUN_DIR}/turfRateFile${RUN}.root
    echo "Summed TURF Rate"
    echo "=========================================="
    ./makeSumTurfRateJsonFiles ${ROOT_RUN_DIR}/sumTurfRateFile${RUN}.root    
    echo "Acqd"
    echo "=========================================="
    ./makeAcqdStartRunJsonFiles ${ROOT_RUN_DIR}/auxFile${RUN}.root     
    echo "Monitor"
    echo "=========================================="
    ./makeMonitorHkJsonFiles ${ROOT_RUN_DIR}/monitorFile${RUN}.root     
    echo "Other"
    echo "=========================================="
    ./makeOtherMonitorHkJsonFiles ${ROOT_RUN_DIR}/monitorFile${RUN}.root 
    echo "GPS"
    echo "=========================================="
    ./makeAdu5PatJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root 0
    ./makeAdu5PatJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root 1
    ./makeAdu5SatJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root 0
    ./makeAdu5SatJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root 1
    ./makeAdu5VtgJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root 0
    ./makeAdu5VtgJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root 1
    ./makeG12PosJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root
    ./makeG12SatJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root
    ./makeGpsGgaJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root 0
    ./makeGpsGgaJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root 1
    ./makeGpsGgaJsonFiles ${ROOT_RUN_DIR}/gpsFile${RUN}.root 2
#    ./makeWaveformSummaryJsonFiles ${ROOT_RUN_DIR}/headFile${RUN}.root ${ROOT_RUN_DIR}/eventFile${RUN}.root  &
fi
