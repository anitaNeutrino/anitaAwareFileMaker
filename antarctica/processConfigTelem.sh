#!/bin/bash
if [ "$1" = "" ]
then
   echo "usage: `basename $0` <run no> " 1>&2
   exit 1
fi



RUN=$1
BASE_DIR=${ANITA_TELEM_DATA_DIR}
RAW_RUN_DIR=${BASE_DIR}/raw/run${RUN}
RAW_CONFIG_DIR=${RAW_RUN_DIR}/config


cd ${ANITA_AWARE_FILEMAKER_DIR}

pwd

export PYTHONPATH=${AWARE_BASE_DIR}/python/

#Step 2: Deal with the config files
for configFile in ${RAW_CONFIG_DIR}/*.config; do
    python ./processConfig.py -i $configFile -r $RUN
done

