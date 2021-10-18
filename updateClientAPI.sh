# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
API_FILE=$PWD/slsSupportLib/include/sls/versionAPI.h
arraylength=3

LIB_DIR=slsDetectorSoftware
RXR_DIR=slsReceiverSoftware
GUI_DIR=slsDetectorGui

declare -a arraydirs=($LIB_DIR $RXR_DIR $GUI_DIR)
declare -a arraynames=("APILIB" "APIRECEIVER" "APIGUI")

for (( i=0; i<${arraylength}; ++i ));
do
	./updateAPIVersion.sh ${arraynames[$i]} ${arraydirs[$i]}
done

#use tag
if [ $# -eq 0 ]; then
	declare -a TAG=$(git rev-parse --abbrev-ref HEAD)
#find branch
else
	declare -a TAG=${1}
fi
#CURR_BRANCH=$(git rev-parse --abbrev-ref HEAD)

#update branch
BRANCH=$(cat $API_FILE | grep GITBRANCH | awk '{print $3}' )
#sed -i s/$BRANCH/\"$CURR_BRANCH\"/g $API_FILE
sed -i s/$BRANCH/\"$TAG\"/g $API_FILE



