# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package

det_list=("ctbDetectorServer" 
	"gotthardDetectorServer" 
	"gotthard2DetectorServer"
	"jungfrauDetectorServer"
	"mythen3DetectorServer"
	"moenchDetectorServer"
	"xilinx_ctbDetectorServer"
	)
usage="\nUsage: compileAllServers.sh [server|all(opt)]. \n\tNo arguments mean all servers with 'developer' branch."

# arguments
if [ $# -eq 0 ]; then
	# no argument, all servers
	declare -a det=${det_list[@]}
	echo "Compiling all servers"
elif [ $# -eq 1 ]; then
	# 'all' servers
	if [[ $1 == "all" ]]; then
		declare -a det=${det_list[@]}
		echo "Compiling all servers"
	else
		# only one server
		# arg not in list
		if [[ $det_list != *$1* ]]; then
			echo -e "Invalid argument 1: $1. $usage"
			return -1
		fi
		declare -a det=("${1}")
		#echo "Compiling only $1"
	fi
else
	echo -e "Too many arguments.$usage"
	return -1
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
	if make clean; then
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
	



