# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
usage="\nUsage: updateAPIVersion.sh [API_NAME] [API_DIR] [API_BRANCH(opt)]."

if [ $# -lt 2 ]; then
    echo -e "Requires atleast 2 arguments. $usage"
    return [-1]
fi

API_NAME=$1
PACKAGE_DIR=$PWD
API_DIR=$PACKAGE_DIR/$2
API_FILE=$PACKAGE_DIR/slsSupportLib/include/sls/versionAPI.h
CURR_DIR=$PWD

if [ ! -d "$API_DIR" ]; then
    echo "[API_DIR] does not exist. $usage"
    return [-1]
fi

#go to directory
cd $API_DIR

#deleting line from file
NUM=$(sed -n '/'$API_NAME' /=' $API_FILE)
#echo $NUM


if [ "$NUM" -gt 0 ]; then
    sed -i ${NUM}d $API_FILE
fi

#find new API date
API_DATE="find .  -printf \"%T@ %CY-%Cm-%Cd\n\"| sort -nr | cut -d' ' -f2- | egrep -v '(\.)o' | head -n 1"

API_DATE=`eval $API_DATE`

API_DATE=$(sed "s/-//g" <<< $API_DATE | awk '{print $1;}' ) 

#extracting only date
API_DATE=${API_DATE:2:6}

#prefix of 0x
API_DATE=${API_DATE/#/0x}
echo "date="$API_DATE


# API_VAL concatenates branch and date
API_VAL=""
# API branch is defined (3rd argument)
if [ $# -eq 3 ]; then
    API_BRANCH=$3
    echo "branch="$API_BRANCH
    API_VAL+="\"$API_BRANCH $API_DATE\""
else
    # API branch not defined (default is developer)
    echo "branch=developer"
    API_VAL+="\"developer $API_DATE\""
fi

#copy it to versionAPI.h
echo "#define "$API_NAME $API_VAL >> $API_FILE

#go back to original directory
cd $CURR_DIR
