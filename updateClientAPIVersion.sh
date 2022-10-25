# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
branch=""
client_list=("slsDetectorSoftware" 
	"slsReceiverSoftware" 
	"slsDetectorGui"
	)
usage="\nUsage: updateClientAPI.sh [all|slsDetectorSoftware|slsReceiverSoftware|slsDetectorGui] [branch]. \n\tNo arguments means all with 'developer' branch. \n\tNo 'branch' input means 'developer branch'"

# arguments
if [ $# -eq 0 ]; then
	declare -a client=$client_list
	echo "API Versioning all"
elif [ $# -eq 1 ] || [ $# -eq 2 ]; then
	# 'all' client
	if [[ $1 == "all" ]]; then
		declare -a client=$client_list
		echo "API Versioning all"
	else
		# only one server
		if [[ $client_list != *$1* ]]; then
			echo -e "Invalid argument 1: $1. $usage"
			return -1
		fi
		declare -a client=("${1}")
		#echo "Versioning only $1"
	fi
	if [ $# -eq 2 ]; then
		if [[ $client_list == *$2* ]]; then
			echo -e "Invalid argument 2: $2. $usage"
			return -1
		fi
		branch+=$2
		#echo "with branch $branch"
	fi
else
	echo -e "Too many arguments.$usage"
	return -1
fi

# versioning each client
for ((i=0;i<${#client[@]};++i))
do
	dir=${client[i]}
	case $dir in
		slsDetectorSoftware)
			declare -a name=APILIB
			;;
		slsReceiverSoftware)
			declare -a name=APIRECEIVER
			;;
		slsDetectorGui)
			declare -a name=APIGUI
			;;
		*)
			echo -n "unknown client argument"
			return -1
			;;
	esac
	echo -e "Versioning $dir [$name]"
	./updateAPIVersion.sh $name $dir $branch
done

#declare -a arraydirs=($LIB_DIR $RXR_DIR $GUI_DIR)
#declare -a arraynames=("APILIB" "APIRECEIVER" "APIGUI")

#arraylength=3
#for (( i=0; i<${arraylength}; ++i ));
#do
#	./updateAPIVersion.sh ${arraynames[$i]} ${arraydirs[$i]}
#done




