# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
branch=""
if [ $# -eq 0 ]; then
	declare -a det=("ctbDetectorServer" 
		"gotthardDetectorServer" 
		"gotthard2DetectorServer"
		"jungfrauDetectorServer"
		"mythen3DetectorServer"
		"moenchDetectorServer"
		)
else
	declare -a det=("${1}")
	branch = $2
	echo "braaaaaaaaaaanch:"$branch
fi

declare -a deterror=("OK" "OK" "OK" "OK" "OK" "OK")

for ((i=0;i<${#det[@]};++i))
do
	dir=${det[i]}
	file="${det[i]}_developer"
	echo -e "Compiling $dir [$file]"
	cd $dir
	make clean
	if make version API_BRANCH=$branch; then
		deterror[i]="OK"
	else
		deterror[i]="FAIL"
	fi
	
	mv bin/$dir bin/$file
	git add -f bin/$file
	cp bin/$file /tftpboot/
	cd ..
	echo -e "\n\n"
done

echo -e "Results:"
for ((i=0;i<${#det[@]};++i))
do
	printf "%s\t\t= %s\n" "${det[i]}" "${deterror[i]}"
done
	



