# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
#!/bin/bash
WD=$PWD
LICENCE_NOTICE_FILE=$WD/notice_to_add_for_every_file

if [ $# -lt 2 ]; then
    echo "Wrong usage of updateLicenseNotice.sh. Requires atleast 1 argument [RELATIVE PATH] [.h/.cpp etc]"
    return [-1]
fi

CURRENT=$WD/$1
FILE_TYPE=$2

if [ ! -d "$CURRENT" ]; then
  echo "This directory ${CURRENT} does not exist"
  return [-1]
fi

cd $CURRENT


for file in $(find $CURRENT -name "*$FILE_TYPE" -not -path "*./build/*" -not -path "*./libs/*"); do
  prefix="/afs/psi.ch/project/sls_det_software/dhanya_softwareDevelopment/mySoft/slsDetectorPackage/"
  p=${file#"$prefix"}
  echo Processing $p

  cat $LICENCE_NOTICE_FILE $file > $file.modified

  mv $file.modified $file

done

cd $WD
