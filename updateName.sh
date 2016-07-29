#!/bin/bash
for fl in *.cxx; do
    mv $fl $fl.old
    sed 's/writeSingleFullJSONFile(fullDir/writeSingleFullJSONFile(dirName/g' $fl.old > $fl
    rm -f $fl.old
done
