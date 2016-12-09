#!/bin/bash

if [ "$1" = "" ]
then
   echo "usage: `basename $0` <run no>" 1>&2
   exit 1
fi

if [ "$ANITA_TELEM_DATA_DIR" = "" ]
then
    echo "ANITA_TELEM_DATA_DIR must be set"
    exit 1
fi


RUN=$1

makeGpsEventTree ${ANITA_TELEM_DATA_DIR}/root/ ${RUN}
 makeGpuSourceJsonFiles ${ANITA_TELEM_DATA_DIR}/root/run${RUN}/headFile${RUN}.root ${ANITA_TELEM_DATA_DIR}/root/run${RUN}/gpsEvent${RUN}.root 

