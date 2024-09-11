#!/bin/bash
# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
CMAKE="cmake3"
BUILDDIR="build"
INSTALLDIR=""
HDF5DIR="/opt/hdf5v1.10.0"
HDF5=0
COMPILERTHREADS=0
TEXTCLIENT=0
RECEIVER=0
GUI=0
DEBUG=0
PYTHON=0
TESTS=0
SIMULATOR=0
CTBGUI=0
MANUALS=0
MANUALS_ONLY_RST=0
MOENCHZMQ=0


CLEAN=0
REBUILD=0
CMAKE_PRE=""
CMAKE_POST=""

usage() { echo -e "
Usage: $0 [-b] [-c] [-d <HDF5 directory>] [-e] [-g] [-h] [-i] [-j <Number of threads>] [-k <CMake command>] [-l <Install directory>] [-m] [-n] [-p] [-r] [-s] [-t] [-u] [-z]  
 -[no option]: only make
 -b: Builds/Rebuilds CMake files normal mode
 -c: Clean
 -d: HDF5 Custom Directory
 -e: Debug mode
 -g: Build/Rebuilds gui
 -h: Builds/Rebuilds Cmake files with HDF5 package
 -i: Builds tests
 -j: Number of threads to compile through
 -k: CMake command
 -l: Install directory
 -m: Manuals
 -n: Manuals without compiling doxygen (only rst)
 -p: Builds/Rebuilds Python API
 -r: Build/Rebuilds only receiver
 -s: Simulator
 -t: Build/Rebuilds only text client
 -u: Chip Test Gui
 -z: Moench zmq processor

Rebuild when you switch to a new build and compile in parallel:
./cmk.sh -bj5

Rebuild python
./cmk.sh -p
 
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

while getopts ":bcd:eghij:k:l:mnpq:rstuz" opt ; do
	case $opt in
	b) 
		echo "Building of CMake files Required"
		REBUILD=1
		;;
	c) 
		echo "Clean Required"
		CLEAN=1
		;;
	d) 
		echo "New HDF5 directory: $OPTARG" 
		HDF5DIR=$OPTARG
		;;
	e)
		echo "Compiling Options: Debug" 
		DEBUG=1
		;;  
	g) 
		echo "Compiling Options: GUI" 
		GUI=1
		REBUILD=1
		;; 
	h) 
		echo "Building of CMake files with HDF5 option Required"
		HDF5=1
		REBUILD=1
		;;
	i)
		echo "Compiling Options: Tests" 
		TESTS=1
		;;   
	j) 
		echo "Number of compiler threads: $OPTARG" 
		COMPILERTHREADS=$OPTARG
		;;
	k)
		echo "CMake command: $OPTARG"
		CMAKE="$OPTARG"
		;;
	l)
		echo "CMake install directory: $OPTARG"
		INSTALLDIR="$OPTARG"
		;;
	m)	
		echo "Compiling Manuals"
		MANUALS=1
		;;
	n)	
		echo "Compiling Manuals (Only RST)"
		MANUALS_ONLY_RST=1
		;;
	p)
    	echo "Compiling Options: Python" 
		PYTHON=1
		REBUILD=1
		;;   
	r) 
		echo "Compiling Options: Receiver" 
		RECEIVER=1
		REBUILD=1
		;;      
	s)
		echo "Compiling Options: Simulator" 
		SIMULATOR=1
		;; 
	t) 
    	echo "Compiling Options: Text Client" 
		TEXTCLIENT=1
		REBUILD=1
		;;
	u)
		echo "Compiling Options: Chip Test Gui"
		CTBGUI=1
		;;
	z)	
		echo "Compiling Moench Zmq Processor"
		MOENCHZMQ=1
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


#python
if [ $PYTHON -eq 1 ]; then
	CMAKE_POST+=" -DSLS_USE_PYTHON=ON "
	echo "Enabling Compile Option: Python"
fi


if [ $TEXTCLIENT -eq 0 ] && [ $RECEIVER -eq 0 ]  && [ $GUI -eq 0 ]; then
	#CMAKE_POST+=" -DSLS_USE_TEXTCLIENT=ON -DSLS_USE_RECEIVER=ON -DSLS_USE_GUI=ON "
	CMAKE_POST+=" -DSLS_USE_TEXTCLIENT=ON -DSLS_USE_RECEIVER=ON -DSLS_USE_GUI=OFF "
    echo "Enabling Compile Option: TextClient, Receiver and GUI"
else 
    if [ $TEXTCLIENT -eq 1 ]; then
        CMAKE_POST+=" -DSLS_USE_TEXTCLIENT=ON -DSLS_USE_RECEIVER=OFF "
        echo "Enabling Compile Option: TextClient"
    fi
    if [ $RECEIVER -eq 1 ]; then
        CMAKE_POST+=" -DSLS_USE_RECEIVER=ON "
        echo "Enabling Compile Option: Receiver"
    fi              
    if [ $GUI -eq 1 ]; then
        CMAKE_POST+=" -DSLS_USE_GUI=ON "
        echo "Enabling Compile Option: GUI"
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
#	CMAKE_POST+=" -DCMAKE_BUILD_TYPE=Debug "
	CMAKE_POST+=" -DCMAKE_BUILD_TYPE=Debug -DSLS_USE_SANITIZER=ON "
	echo "Debug Option enabled"
fi 

#Simulator
if [ $SIMULATOR -eq 1 ]; then
	CMAKE_POST+=" -DSLS_USE_SIMULATOR=ON "
	echo "Simulator Option enabled"
fi 

#Manuals
if [ $MANUALS -eq 1 ]; then
	CMAKE_POST+=" -DSLS_BUILD_DOCS=ON "
	echo "Manuals Option enabled"
fi 

#Moench zmq processor
if [ $MOENCHZMQ -eq 1 ]; then
	CMAKE_POST+=" -DSLS_USE_MOENCH=ON "
	echo "Moench Zmq Processor Option enabled"
fi 

#Chip Test Gui
if [ $CTBGUI -eq 1 ]; then
	CMAKE_POST+=" -DSLS_USE_CTBGUI=ON "
	echo "CTB Gui Option enabled"
fi 

#Tests
if [ $TESTS -eq 1 ]; then
	CMAKE_POST+=" -DSLS_USE_TESTS=ON -DSLS_USE_INTEGRATION_TESTS=ON"
	echo "Tests Option enabled"
fi 

#hdf5 rebuild
if [ $HDF5 -eq 1 ]; then
#	CMAKE_PRE+="HDF5_ROOT="$HDF5DIR
	CMAKE_POST+=" -DCMAKE_INSTALL_PREFIX="$HDF5DIR
	CMAKE_POST+=" -DSLS_USE_HDF5=ON "
#normal mode rebuild
else
	CMAKE_POST+=" -DSLS_USE_HDF5=OFF "
fi


#install
if [ -n "$INSTALLDIR" ]; then
	CMAKE_POST+=" -DCMAKE_INSTALL_PREFIX=$INSTALLDIR"
	CMAKE_POST+=" -DCMAKE_FIND_ROOT_PATH=$INSTALLDIR"
fi


#enter build dir
#pushd $BUILDDIR;
cd $BUILDDIR;
echo "in "$PWD



#cmake
if [ $REBUILD -eq 1 ]; then
	rm -f CMakeCache.txt
	BUILDCOMMAND="$CMAKE_PRE $CMAKE $CMAKE_POST .."
	echo $BUILDCOMMAND
	eval $BUILDCOMMAND
fi

#make clean
if [ $CLEAN -eq 1 ]; then
	make clean;
fi


#make
if [ $COMPILERTHREADS -gt 0 ]; then
	if [ $MANUALS -eq 0 ] && [ $MANUALS_ONLY_RST -eq 0 ]; then
		BUILDCOMMAND="make -j$COMPILERTHREADS"
		echo $BUILDCOMMAND
		eval $BUILDCOMMAND
	else
		if [ $MANUALS -eq 1 ]; then
			BUILDCOMMAND="make docs -j$COMPILERTHREADS"
			echo $BUILDCOMMAND
			eval $BUILDCOMMAND
		else
			BUILDCOMMAND="make rst -j$COMPILERTHREADS"
			echo $BUILDCOMMAND
			eval $BUILDCOMMAND		
		fi
	fi 
else
	if [ $MANUALS -eq 0 ] && [ $MANUALS_ONLY_RST -eq 0 ]; then
		echo "make"
		make
	else
		if [ $MANUALS -eq 1 ]; then
			echo "make docs"
			make docs
		else
			echo "make rst"
			make rst
		fi
	fi 
fi


#install
if [ -n "$INSTALLDIR" ]; then
	make install
#	popd
#	$CMAKE --build $BUILDDIR --target install
fi





