# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
branch=""
client_list=("slsDetectorSoftware" "slsReceiverSoftware")
usage="\nUsage: updateClientAPI.sh [all|slsDetectorSoftware|slsReceiverSoftware] [branch]. \n\tNo arguments means all with 'developer' branch. \n\tNo 'branch' input means 'developer branch'"

# arguments
if [ $# -eq 0 ]; then
	declare -a client=${client_list[@]}
	echo "API Versioning all"
elif [ $# -eq 1 ] || [ $# -eq 2 ]; then
	# 'all' client
	if [[ $1 == "all" ]]; then
		declare -a client=${client_list[@]}
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

#echo "list is: ${client[@]}"

# versioning each client
for i in ${client[@]}
do
	dir=$i
	case $dir in
		slsDetectorSoftware)
			declare -a name=APILIB
			;;
		slsReceiverSoftware)
			declare -a name=APIRECEIVER
			;;
		*)
			echo -n "unknown client argument $i"
			return -1
			;;
	esac
	echo -e "Versioning $dir [$name]"
	./updateAPIVersion.sh $name $dir $branch
done

