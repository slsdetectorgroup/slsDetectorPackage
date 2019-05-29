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
    #thispath=$(dirname ${BASH_ARGV[0]})  
    thispath=$(dirname $f) 
    p=$(cd ${thispath};pwd); 
    THIS_PATH="$p/build/bin/"
   #     echo "ERROR: must cd where/this/package/is before calling this_path.sh"
#	echo "Try sourcing it"                                 
    else
	THIS_PATH="$PWD/build/bin/"; 
    fi                        
else                   
    #thispath=$(dirname ${BASH_ARGV[0]})
    thispath=${BASH_ARGV[0]}  
    #thispath=$(dirname $f) 
    THIS_PATH=$(cd ${thispath};pwd); 
fi   

    echo $THIS_PATH  
    export PATH=$THIS_PATH:$PATH
    export LD_LIBRARY_PATH=$THIS_PATH:$LD_LIBRARY_PATH
