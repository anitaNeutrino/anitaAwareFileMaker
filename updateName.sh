#!/bin/bash
for fl in *.py; do
    mv $fl $fl.old
    sed 's/ANITA3/ANITA4/g' $fl.old > $fl
    rm -f $fl.old
done
