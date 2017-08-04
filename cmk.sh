#!/bin/bash
BUILDDIR="build"
HDF5DIR="/opt/hdf5v1.10.0"
HDF5=0

CLEAN=0
REBUILD=0
CMAKE_PRE=""
CMAKE_POST=""

usage() { echo -e "
Usage: $0 [-c] [-r] [-h] [-d <HDF5 directory>]
 -[no option]: only make
 -c: Clean
 -b: Builds/Rebuilds CMake files normal mode
 -h: Builds/Rebuilds Cmake files with HDF5 package
 -d: HDF5 Custom Directory
 " ; exit 1; }

while getopts ":bchd:" opt ; do
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


#hdf5 rebuild
if [ $HDF5 -eq 1 ]; then
	CMAKE_PRE+="HDF5_ROOT="$HDF5DIR
	CMAKE_POST+="-DUSE_HDF5=ON"
#normal mode rebuild
else
	CMAKE_POST+="-DUSE_HDF5=OFF"
fi


#enter build dir
cd $BUILDDIR;
echo "in "$PWD



#if rebuild required
if [ $REBUILD -eq 1 ]; then
	BUILDCOMMAND="$CMAKE_PRE cmake $CMAKE_POST .."
	echo $BUILDCOMMAND
	eval $BUILDCOMMAND
fi
if [ $CLEAN -eq 1 ]; then
	make clean;
fi

make -j9






