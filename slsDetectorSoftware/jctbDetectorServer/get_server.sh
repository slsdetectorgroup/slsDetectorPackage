#!/bin/sh
serv="pc8498"
f="jungfrauDetectorServerTest"
if [ "$#" -gt 0 ]; then
    f=$1
fi
if [ "$#" -gt 1 ]; then
    serv=$2
fi
tftp $serv -r $f -g
chmod a+xrw $f 

