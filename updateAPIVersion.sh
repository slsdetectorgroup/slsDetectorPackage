#require 2 arguments, API_NAME API_DIR (relative to package)
if [ $# -lt 2 ]; then
    echo "wrong usage of updateVersion.sh"
    exit -1
fi

API_NAME=$1
PACKAGE_DIR=$PWD
API_DIR=$PACKAGE_DIR/$2
API_FILE=$PACKAGE_DIR/slsSupportLib/include/versionAPI.h
CURR_DIR=$PWD

#go to directory
cd $API_DIR

#deleting line from file
NUM=$(sed -n '/'$API_NAME'/=' $API_FILE)
#echo $NUM

if [ "$NUM" -gt 0 ]; then
    sed -i ${NUM}d $API_FILE
fi

#find new API date
API_DATE="find .  -printf \"%T@ %CY-%Cm-%CdT%CH:%CM:%CS %p\n\"| sort -nr | cut -d' ' -f2- | egrep -v build | egrep -v '(\.)o'| egrep -v 'versionAPI.h' | head -n 1"
API_DATE=`eval $API_DATE`
API_DATE=$(sed "s/-//g" <<< $API_DATE | awk '{print $1;}' ) 
#extracting only date
API_DATE=${API_DATE:2:6}
#prefix of 0x
API_DATE=${API_DATE/#/0x}

#copy it to versionAPI.h
echo "#define "$API_NAME $API_DATE >> $API_FILE

#go back to original directory
cd $CURR_DIR
