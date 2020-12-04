#!/bin/bash
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
Usage: $0 [-c] [-b] [-p] [e] [t] [r] [g] [s] [u] [i] [m] [n] [-h] [z] [-d <HDF5 directory>] [-l Install directory] [-k <CMake command>] [-j <Number of threads>]
 -[no option]: only make
 -c: Clean
 -b: Builds/Rebuilds CMake files normal mode
 -p: Builds/Rebuilds Python API
 -h: Builds/Rebuilds Cmake files with HDF5 package
 -d: HDF5 Custom Directory
 -k: CMake command
 -l: Install directory
 -t: Build/Rebuilds only text client
 -r: Build/Rebuilds only receiver
 -g: Build/Rebuilds only gui
 -s: Simulator
 -u: Chip Test Gui
 -j: Number of threads to compile through
 -e: Debug mode
 -i: Builds tests
 -m: Manuals
 -n: Manuals without compiling doxygen (only rst)
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

while getopts ":bpchd:k:l:j:trgeisumnz" opt ; do
	case $opt in
	b)
		echo "Building of CMake files Required"
		REBUILD=1
		;;
	p)
    	echo "Compiling Options: Python"
		PYTHON=1
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
	l)
		echo "CMake install directory: $OPTARG"
		INSTALLDIR="$OPTARG"
		;;
	k)
		echo "CMake command: $OPTARG"
		CMAKE="$OPTARG"
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
	i)
		echo "Compiling Options: Tests"
		TESTS=1
		;;
	s)
		echo "Compiling Options: Simulator"
		SIMULATOR=1
		;;
	m)
		echo "Compiling Manuals"
		MANUALS=1
		;;
	n)
		echo "Compiling Manuals (Only RST)"
		MANUALS_ONLY_RST=1
		;;
	z)
		echo "Compiling Moench Zmq Processor"
		MOENCHZMQ=1
		;;
	u)
		echo "Compiling Options: Chip Test Gui"
		CTBGUI=1
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

#targets
if [ $TEXTCLIENT -eq 0 ] && [ $RECEIVER -eq 0 ]  && [ $GUI -eq 0 ]; then
	TEXTCLIENT=1
	RECEIVER=1
fi

if [ $TEXTCLIENT -eq 1 ]; then
        CMAKE_POST+=" -DSLS_USE_TEXTCLIENT=ON "
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
	CMAKE_POST+=" -DCMAKE_INSTALL_PREFIX="$HDF5DIR
	CMAKE_POST+=" -DSLS_USE_HDF5=ON "
	echo "HDF5 Option enabled"
#normal mode rebuild
else
	CMAKE_POST+=" -DSLS_USE_HDF5=OFF "
fi


#install
if [ -n "$INSTALLDIR" ]; then
	CMAKE_POST+=" -DCMAKE_INSTALL_PREFIX=$INSTALLDIR"
	CMAKE_POST+=" -DCMAKE_FIND_ROOT_PATH=$INSTALLDIR"
	echo "Installing files to $INSTALLDIR"
fi


#enter build dir
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
MAKEOPTIONS=""
if [ $COMPILERTHREADS -gt 0 ]; then
	MAKEOPTIONS="-j$COMPILERTHREADS"
fi

MAKETARGET=""
if [ $MANUALS -eq 1 ]; then
	MAKETARGET="docs"
elif [ $MANUALS_ONLY_RST -eq 1 ]; then
	MAKETARGET="rst"
fi

BUILDCOMMAND="make $MAKETARGET $MAKEOPTIONS"
echo $BUILDCOMMAND
eval $BUILDCOMMAND


#install
if [ -n "$INSTALLDIR" ]; then
	make install
fi
