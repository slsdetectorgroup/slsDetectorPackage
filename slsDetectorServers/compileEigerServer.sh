# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
deterror="OK"
dir="eigerDetectorServer"
file="${dir}_developer"
branch=""

# arguments
if [ $# -eq 1 ]; then
	branch+=$1
	#echo "with branch $branch"
elif [ ! $# -eq 0 ]; then
	echo -e "Only one optional argument allowed for branch."
	return -1
fi

echo -e "Compiling $dir [$file]"
cd $dir
make clean
if make version API_BRANCH=$branch; then
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
	
