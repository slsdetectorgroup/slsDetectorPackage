#!/bin/bash
BUILDDIR="build"
HDF5DIR="/opt/hdf5v1.10.0"
HDF5=0
COMPILERTHREADS=0
TEXTCLIENT=0
RECEIVER=0
GUI=0
DEBUG=0


CLEAN=0
REBUILD=0
CMAKE_PRE=""
CMAKE_POST=""

usage() { echo -e "
Usage: $0 [-c] [-b] [-h] [-d <HDF5 directory>] [-j]
 -[no option]: only make
 -c: Clean
 -b: Builds/Rebuilds CMake files normal mode
 -h: Builds/Rebuilds Cmake files with HDF5 package
 -d: HDF5 Custom Directory
 -t: Build/Rebuilds only text client
 -r: Build/Rebuilds only receiver
 -g: Build/Rebuilds only gui
 -j: Number of threads to compile through
 -e: Debug mode
 
For only make:
./cmk.sh

For make clean;make:
./cmk.sh -c

For using hdf5 without default dir /opt/hdf5v1.10.0:
./cmk.sh -h

For using hdf5 without custom dir /blabla:
./cmk.sh -h -d /blabla

For rebuilding cmake without hdf5 (Use this if you had previously run with hdf5 and now you dont want it)
./cmk.sh -b

For using multiple cores to compile faster:
(all these options work)
./cmk.sh -j9
./cmk.sh -cj9 #with clean
./cmk.sh -hj9 #with hdf5
./cmk.sh -j9 -h #with hdf

For rebuilding only certain sections
./cmk.sh -tg #only text client and gui
./cmk.sh -r #only receiver
 
 " ; exit 1; }

while getopts ":bchd:j:trge" opt ; do
	case $opt in
	b) 
		echo "Building of CMake files Required"
		REBUILD=1
		;;
	c) 
		echo "Clean Required"
		CLEAN=1
		;;
	h) 
		echo "Building of CMake files with HDF5 option Required"
		HDF5=1
		REBUILD=1
		;;
	d) 
		echo "New HDF5 directory: $OPTARG" 
		HDF5DIR=$OPTARG
		;;
	j) 
		echo "Number of compiler threads: $OPTARG" 
		COMPILERTHREADS=$OPTARG
		;;
	t) 
    	echo "Compiling Options: Text Client" 
		TEXTCLIENT=1
		REBUILD=1
		;;      
	r) 
		echo "Compiling Options: Receiver" 
		RECEIVER=1
		REBUILD=1
		;;      
	g) 
		echo "Compiling Options: GUI" 
		GUI=1
		REBUILD=1
		;;  
	e)
		echo "Compiling Options: Debug" 
		DEBUG=1
		;;    
    \?)
     	echo "Invalid option: -$OPTARG" 
		usage
      	exit 1
      	;;
    :)
      	echo "Option -$OPTARG requires an argument."
		usage
      	exit 1
      	;;	
	esac
done






if [ $TEXTCLIENT -eq 0 ] && [ $RECEIVER -eq 0 ]  && [ $GUI -eq 0 ]; then
       CMAKE_POST+=" -DSLS_USE_TEXTCLIENT=ON -DSLS_USE_RECEIVER=ON -DSLS_USE_GUI=ON "
       echo "Compile Option: TextClient, Receiver and GUI"
else 
       if [ $TEXTCLIENT -eq 1 ]; then
              CMAKE_POST+=" -DSLS_USE_TEXTCLIENT=ON "
               echo "Compile Option: TextClient"
       fi
       if [ $RECEIVER -eq 1 ]; then
               CMAKE_POST+=" -DSLS_USE_RECEIVER=ON "
               echo "Compile Option: Receiver"
       fi
                               
       if [ $GUI -eq 1 ]; then
               CMAKE_POST+=" -DSLS_USE_GUI=ON "
               echo "Compile Option: GUI"
       fi
fi





#build dir doesnt exist
if [ ! -d "$BUILDDIR" ] ; then
	echo "No Build Directory. Building of Cmake files required"
	mkdir $BUILDDIR;
	REBUILD=1
else
	#rebuild not requested, but no makefile
	if [ $REBUILD -eq 0 ] && [ ! -f "$BUILDDIR/Makefile" ] ; then
		echo "No Makefile. Building of Cmake files required"
		REBUILD=1
	fi
fi

#Debug
if [ $DEBUG -eq 1 ]; then
	CMAKE_POST+=" -DCMAKE_BUILD_TYPE=Debug "
	echo "Debug Option enabled"
fi 


#hdf5 rebuild
if [ $HDF5 -eq 1 ]; then
	CMAKE_PRE+="HDF5_ROOT="$HDF5DIR
	CMAKE_POST+=" -DSLS_USE_HDF5=ON "
#normal mode rebuild
else
	CMAKE_POST+=" -DSLS_USE_HDF5=OFF "
fi


#enter build dir
cd $BUILDDIR;
echo "in "$PWD



#cmake
if [ $REBUILD -eq 1 ]; then
	rm -f CMakeCache.txt
	BUILDCOMMAND="$CMAKE_PRE cmake $CMAKE_POST .."
	echo $BUILDCOMMAND
	eval $BUILDCOMMAND
fi

#make clean
if [ $CLEAN -eq 1 ]; then
	make clean;
fi


#make
if [ $COMPILERTHREADS -gt 0 ]; then
	BUILDCOMMAND="make -j$COMPILERTHREADS"
	echo $BUILDCOMMAND
	eval $BUILDCOMMAND
else
	make
fi






