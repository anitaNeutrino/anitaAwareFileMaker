#!/bin/bash
if [ "$1" = "" ]
then
   echo "usage: `basename $0` <run no>" 1>&2
   exit 1
fi



RUN=$1
BASE_DIR=/storage/palestine14
RAW_RUN_DIR=${BASE_DIR}/raw/run${RUN}
EVENT_BASE_DIR=${BASE_DIR}/root
ROOT_RUN_DIR=${EVENT_BASE_DIR}/run${RUN}

#Step 1: Generate the ROOT Files
cd /home/radio/anita14/simpleTreeMaker/
./runAnitaIIIFileMaker.sh $RUN

#Step 2: Generate the AWARE Files
cd /home/radio/anita14/anitaAwareFileMaker/
source setupAwareVariables.sh
if [ -d "$ROOT_RUN_DIR" ]; then
    ./makeHeaderJsonFiles ${ROOT_RUN_DIR}/headFile${RUN}.root 
    ./makePrettyHkJsonFiles ${ROOT_RUN_DIR}/prettyHkFile${RUN}.root     
fi
