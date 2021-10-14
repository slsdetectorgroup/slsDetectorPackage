#!/bin/bash
WD=$PWD
LICENCE_NOTICE_FILE=$WD/notice_to_add_for_every_file


if [ $# -lt 1 ]; then
    echo "Wrong usage of updateLicenseNotice.sh. Requires atleast 1 argument [RELATIVE PATH]"
    return -1
fi

CURRENT=$WD/$1

if [ ! -d "$CURRENT" ]; then
  echo "This directory ${CURRENT} does not exist"
  return -1
fi

cd CURRENT

for file in $(find $CURRENT -name "*.h"); do
  echo Processing $file

  cat notice_to_add_for_every_file $file > $file.modified

  mv $file.modified $file

done

cd $WD
