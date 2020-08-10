API_FILE=$PWD/slsSupportLib/include/versionAPI.h
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

#find branch
CURR_BRANCH=$(git rev-parse --abbrev-ref HEAD)

#update branch
BRANCH=$(cat $API_FILE | grep GITBRANCH | awk '{print $3}' )
sed -i s/$BRANCH/\"$CURR_BRANCH\"/g $API_FILE



