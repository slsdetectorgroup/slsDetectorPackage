# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
#echo $#
#if [ $# = 0 ]; then 
#    f=$0
#else
#    f=$1
#fi
#echo $f
if [ "x${BASH_ARGV[0]}" = "x" ]; then
#if [ "x$f" = "x" ]; then
    if [ ! -f this_build_bin_path.sh ]; then
	f=$0
	echo "aaaa"
    #thispath=$(dirname ${BASH_ARGV[0]})  
    thispath=$(dirname $f) 
    p=$(cd ${thispath};pwd); 
    THIS_PATH="$p/build/bin/"
   #     echo "ERROR: must cd where/this/package/is before calling this_path.sh"
#	echo "Try sourcing it"                                 
    else
	echo "bbb"
	THIS_PATH="$PWD/build/bin/"; 
    fi                        
else        
    thispath=$(dirname ${BASH_ARGV[0]})
    p=$(cd ${thispath};pwd); 
    THIS_PATH="$p/build/bin/"    
    echo "ccc"
fi   

    echo "this_path="$THIS_PATH  
    export PATH=$THIS_PATH:$PATH
    export LD_LIBRARY_PATH=$THIS_PATH:$LD_LIBRARY_PATH
    export PYTHONPATH=$THIS_PATH:$PYTHONPATH
    
    echo "path="$PATH
    echo "ld_library_path="$LD_LIBRARY_PATH
    echo "pythonpath="$PYTHON_PATH
