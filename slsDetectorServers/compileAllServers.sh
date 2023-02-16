# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package

# empty branch = developer branch in updateAPIVersion.sh
branch=""
det_list=("ctbDetectorServer  
	gotthardDetectorServer  
	gotthard2DetectorServer 
	jungfrauDetectorServer 
	mythen3DetectorServer 
	moenchDetectorServer" 
	)
usage="\nUsage: compileAllServers.sh [server|all(opt)] [branch(opt)]. \n\tNo arguments mean all servers with 'developer' branch. \n\tNo 'branch' input means 'developer branch'"

# arguments
if [ $# -eq 0 ]; then
	# no argument, all servers
	declare -a det=${det_list[@]}
	echo "Compiling all servers"
elif [ $# -eq 1 ] || [ $# -eq 2 ]; then
	# 'all' servers
	if [[ $1 == "all" ]]; then
		declare -a det=${det_list[@]}
		echo "Compiling all servers"
	else
		# only one server
		# arg not in list
		echo $det_list | grep -w -q $1
		#if [[ $det_list != *$1* ]]; then
		if ! [[ $? ]] ; then
			echo -e "Invalid argument 1: $1. $usage"
			return 1
		fi
		declare -a det=("${1}")
		#echo "Compiling only $1"
	fi
	# branch
	if [ $# -eq 2 ]; then
		# arg in list
		if [[ $det_list == *$2* ]]; then
			echo -e "Invalid argument 2: $2. $usage"
			return 1
		fi
		branch+=$2
		#echo "with branch $branch"
	fi
else
	echo -e "Too many arguments.$usage"
	return 1
fi

declare -a deterror=("OK" "OK" "OK" "OK" "OK" "OK")

echo -e "list is ${det[@]}"

# compile each server
idet=0
for i in ${det[@]}
do
	dir=$i
	file="${i}_developer"
	echo -e "Compiling $dir [$file]"
	cd $dir
	make clean
	if make version API_BRANCH=$branch; then
		deterror[$idet]="OK"
	else
		deterror[$idet]="FAIL"
	fi
	mv bin/$dir bin/$file
	git add -f bin/$file
	cp bin/$file /tftpboot/
	cd ..
	echo -e "\n\n"
	((++idet))
done

echo -e "Results:"
idet=0
for i in ${det[@]}
do
	printf "%s\t\t= %s\n" "$i" "${deterror[$idet]}"
	((++idet))
done
	



