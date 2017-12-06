MAINDIR=slsDetectorsPackage
SPECDIR=slsDetectorSoftware
TMPFILE=slsDetector/gitInfoLibTmp.h
INCLFILE=slsDetector/gitInfoLib.h
WD=$PWD

#evaluate the variables
EVALFILE=../evalVersionVariables.sh
source $EVALFILE

#get modified date
#RDATE1='git log --pretty=format:"%ci" -1'
RDATE1="find . -not -path '*DetectorServer/*' -type f -exec stat --format '%Y :%y %n' '{}' \; | sort -nr | cut -d: -f2- | egrep -v 'gitInfo|build|.git|updateGitVersion' | head -n 1"
RDATE=`eval $RDATE1`
NEWDATE=$(sed "s/-//g" <<< $RDATE | awk '{print $1;}') 
NEWDATE=${NEWDATE/#/0x}

#get old date from INCLFILE
OLDDATE=$(more $INCLFILE | grep '#define GITDATE' | awk '{print $3}')


#update INCLFILE if changes
#if [ "$OLDDATE" != "$NEWDATE" ]; then
	echo Path: ${MAINDIR}/${SPECDIR}  $'\n'URL: ${GITREPO}  $'\n'Repository Root: ${GITREPO}  $'\n'Repsitory UUID: ${REPUID}  $'\n'Revision: ${FOLDERREV}  $'\n'Branch: ${BRANCH}  $'\n'Last Changed Author: ${AUTH1}_${AUTH2}  $'\n'Last Changed Rev: ${REV}  $'\n'Last Changed Date: ${RDATE} > gitInfo.txt 
	cd ..
	./genVersionHeader.sh $SPECDIR/gitInfo.txt $SPECDIR/$TMPFILE $SPECDIR/$INCLFILE 
	cd $WD
#fi