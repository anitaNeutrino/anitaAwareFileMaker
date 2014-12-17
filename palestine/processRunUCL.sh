#!/bin/bash
if [ "$1" = "" ]
then
   echo "usage: `basename $0` <run no>" 1>&2
   exit 1
fi



RUN=$1
BASE_DIR=/unix/anita2/palestine14
RAW_RUN_DIR=${BASE_DIR}/raw/run${RUN}
RAW_CONFIG_DIR=${RAW_RUN_DIR}/config
EVENT_BASE_DIR=${BASE_DIR}/root
ROOT_RUN_DIR=${EVENT_BASE_DIR}/run${RUN}

source /home/rjn/anita/anitaAwareFileMaker/setupAwareVariablesUCL.sh
cd /home/rjn/anita/anitaAwareFileMaker/

for configFile in ${RAW_CONFIG_DIR}/*.config; do
    python ./processConfig.py -i $configFile -r $RUN
done



#Step 2: Generate the AWARE Files

if [ -d "$ROOT_RUN_DIR" ]; then
    ./makeHeaderJsonFiles ${ROOT_RUN_DIR}/headFile${RUN}.root 
    ./makePrettyHkJsonFiles ${ROOT_RUN_DIR}/prettyHkFile${RUN}.root    
    ./makeAcqdStartRunJsonFiles ${ROOT_RUN_DIR}/auxFile${RUN}.root     
    ./makeMonitorHkJsonFiles ${ROOT_RUN_DIR}/monitorFile${RUN}.root     
    ./makeOtherMonitorHkJsonFiles ${ROOT_RUN_DIR}/monitorFile${RUN}.root    
#    ./makeEventJsonFiles ${ROOT_RUN_DIR}/headFile${RUN}.root ${ROOT_RUN_DIR}/eventFile${RUN}.root  &
fi


