# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
deterror="OK"
dir="eigerDetectorServer"
file="${dir}_developer"

usage="\nUsage: compileAllServers.sh [update_api(opt)]. \n\t update_api if true updates the api to version in VERSION file"

update_api=false
target=clean

# arguments
if [ $# -eq 1 ]; then
	update_api=$1
	if $update_api ; then
		target=version
		echo "updating api to $(cat ../VERSION)"
	fi
elif [ ! $# -eq 0 ]; then
	echo -e "Only one optional argument allowed for update_api."
	return -1
fi

echo -e "Compiling $dir [$file]"
cd $dir
if make $target; then
	deterror="OK"
else
	deterror="FAIL"
fi

mv bin/$dir bin/$file
git add -f bin/$file
cp bin/$file /tftpboot/
cd ..
echo -e "\n\n"
printf "Result:\t\t= %s\n"  "${deterror}"
	
