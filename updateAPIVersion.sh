PACKAGE_DIR=$PWD
API_FILE=$PACKAGE_DIR/slsSupportLib/include/versionAPI.h

arraylength=2

RXR_DIR=$PACKAGE_DIR/slsReceiverSoftware
GUI_DIR=$PACKAGE_DIR/slsDetectorGui

declare -a arraydirs=($RXR_DIR $GUI_DIR)
declare -a arraynames=("APIRECEIVER" "APIGUI")

for (( i=0; i<${arraylength}; ++i ));
do
	echo ${arraydirs[$i]}
	cd ${arraydirs[$i]}

	API_NAME=${arraynames[$i]}
	#deleting line from file
	NUM=$(sed -n '/'$API_NAME'/=' $API_FILE)
	if [ "$NUM" > 0 ]; then
		sed -i ${NUM}d $API_FILE
	fi
	#find new API date
	API_DATE="find . -type f -exec stat --format '%Y :%y %n' '{}' \; | sort -nr | cut -d: -f2- | 		egrep -v build | egrep -v '(\.)o'| head -n 1"
	API_DATE=`eval $API_DATE`
	API_DATE=$(sed "s/-//g" <<< $API_DATE | awk '{print $1;}') 
	API_DATE=${API_DATE/#/0x}
	echo "#define "$API_NAME $API_DATE >> $API_FILE

done


