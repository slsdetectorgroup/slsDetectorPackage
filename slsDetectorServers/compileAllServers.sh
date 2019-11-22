if [ $# -eq 0 ]; then
	declare -a det=("ctbDetectorServer" 
		"gotthardDetectorServer" 
		"gotthard2DetectorServer"
		"jungfrauDetectorServer"
		"mythen3DetectorServer"
		)
else
	declare -a det=("${1}")
	echo "got something"
fi

declare -a deterror=("OK" "OK" "OK" "OK")

for ((i=0;i<${#det[@]};++i))
do
	dir=${det[i]}
	file="${det[i]}_developer"
	echo -e "Compiling $dir [$file]"
	cd $dir
	make clean
	if make version; then
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
	



