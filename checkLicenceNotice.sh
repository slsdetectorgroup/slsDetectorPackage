# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
#!/bin/bash
RED='\033[0;31m'
NC='\033[0m' # No Color
WD=$PWD

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


NOTICE="SPDX-License-Identifier"
#echo -e "Notice is \n$NOTICE"


for file in $(find $CURRENT -name "$FILE_TYPE" -not -path "*./build/*" -not -path "*./libs/*"); do
  #echo -e "\nProcessing $file"
  #echo "$file"
  firstline=$(head -n 1 $file)

#<<testing
  if ! grep -q "$NOTICE" "$file"; then
    f="$(basename -- $file)"
    prefix="/afs/psi.ch/project/sls_det_software/dhanya_softwareDevelopment/mySoft/slsDetectorPackage/"
    p=${file#"$prefix"}
    printf "${RED}Does not have Notice: %s${NC}\n" "$p"
  fi

  #cat notice_to_add_for_every_file $file > $file.modified

  #mv $file.modified $file
#testing
done


cd $WD
