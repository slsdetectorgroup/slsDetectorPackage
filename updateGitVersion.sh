# Script to create gitInfo.txt files and export software


#folders
MAINDIR=slsDetectorsPackage
LIBDIR=slsDetectorSoftware
RXRDIR=slsReceiverSoftware
GUIDIR=slsDetectorGui
CALWIZDIR=calibrationWizards

#paths
WD=$PWD
LIBPATH=$WD/$LIBDIR
RXRPATH=$WD/$RXRDIR
GUIPATH=$WD/$GUIDIR
CALWIZPATH=$WD/$CALWIZDIR

#commands to create gitInfo.txt, but these commands get executed before entering directory
#GITREPO=`git remote -v | cut -d' ' -f1`
#BRANCH=`git branch -v | grep '*' | cut -d' ' -f2`
#REPUID=`git log --pretty=format:"%H" -1`
#AUTH1=`git log --pretty=format:"%cn" -1 | cut -d' ' -f1`
#AUTH2=`git log --pretty=format:"%cn" -1 | cut -d' ' -f2`
#FOLDERREV=`git log --oneline .  | wc -l`  #used for all the individual server folders
#REV=`git log --oneline  | wc -l` 
#RDATE=`git log --pretty=format:"%ci" -1`


GITREPO1='git remote -v'
GITREPO2=" | grep \"fetch\" | cut -d' ' -f1"
BRANCH1='git branch -v'
BRANCH2=" | grep '*' | cut -d' ' -f2"
REPUID1='git log --pretty=format:"%H" -1'
AUTH1_1='git log --pretty=format:"%cn" -1'
AUTH1_2=" | cut -d' ' -f1"
AUTH2_1='git log --pretty=format:"%cn" -1'
AUTH2_2=" | cut -d' ' -f2"
FOLDERREV1='git log --oneline . '   #used for all the individual server folders
FOLDERREV2=" | wc -l"  #used for all the individual server folders
REV1='git log --oneline  '
REV2=" | wc -l"
RDATE1='git log --pretty=format:"%ci" -1'
COMMIT_TITLE_SCRIPT='git log --pretty=format:"%s" -1'


#create gitInfo.txt
#have to go into path to execute some git commands, different variables are upto Revision

cd $LIBPATH
echo -e "\nIn slsDetectorSoftware"
COMMIT_TITLE=`eval $COMMIT_TITLE_SCRIPT`
echo $COMMIT_TITLE
if [ "$COMMIT_TITLE" == "updaterev" ]; then
echo "No update"
else
GITREPO=`eval $GITREPO1  $GITREPO2`
BRANCH=`eval $BRANCH1  $BRANCH2`
REPUID=`eval $REPUID1`
AUTH1=`eval $AUTH1_1  $AUTH1_2`
AUTH2=`eval $AUTH2_1  $AUTH2_2`
REV=`eval $REV1  $REV2`
RDATE=`eval $RDATE1`
echo Path: ${MAINDIR}/${LIBDIR}  $'\n'URL: ${GITREPO}  $'\n'Repository Root: ${GITREPO}  $'\n'Repsitory UUID: ${REPUID}  $'\n'Revision: ${REV}  $'\n'Branch: ${BRANCH}  $'\n'Last Changed Author: ${AUTH1}_${AUTH2}  $'\n'Last Changed Rev: ${REV}  $'\n'Last Changed Date: ${RDATE} > gitInfo.txt 
cd $WD
./genVersionHeader.sh $LIBDIR/gitInfo.txt $LIBDIR/slsDetector/gitInfoLibTmp.h $LIBDIR/slsDetector/gitInfoLib.h 
echo "Revision Updated"
fi


cd $RXRPATH
echo -e "\nIn slsReceiverSoftware"
COMMIT_TITLE=`eval $COMMIT_TITLE_SCRIPT`
echo $COMMIT_TITLE
if [ "$COMMIT_TITLE" == "updaterev" ]; then
echo "No update"
else
GITREPO=`eval $GITREPO1  $GITREPO2`
BRANCH=`eval $BRANCH1  $BRANCH2`
REPUID=`eval $REPUID1`
AUTH1=`eval $AUTH1_1  $AUTH1_2`
AUTH2=`eval $AUTH2_1  $AUTH2_2`
REV=`eval $REV1  $REV2`
RDATE=`eval $RDATE1`
echo Path: ${MAINDIR}/${RXRDIR}  $'\n'URL: ${GITREPO}  $'\n'Repository Root: ${GITREPO}  $'\n'Repsitory UUID: ${REPUID}  $'\n'Revision: ${REV}  $'\n'Branch: ${BRANCH}  $'\n'Last Changed Author: ${AUTH1}_${AUTH2}  $'\n'Last Changed Rev: ${REV}  $'\n'Last Changed Date: ${RDATE} > gitInfo.txt 
cd $WD
./genVersionHeader.sh $RXRDIR/gitInfo.txt $RXRDIR/include/gitInfoReceiverTmp.h $RXRDIR/include/gitInfoReceiver.h
echo "Revision Updated"
fi


cd $GUIPATH
echo -e "\nIn slsDetectorGui"
COMMIT_TITLE=`eval $COMMIT_TITLE_SCRIPT`
echo $COMMIT_TITLE
if [ "$COMMIT_TITLE" == "updaterev" ]; then
echo "No update"
else
GITREPO=`eval $GITREPO1  $GITREPO2`
BRANCH=`eval $BRANCH1  $BRANCH2`
REPUID=`eval $REPUID1`
AUTH1=`eval $AUTH1_1  $AUTH1_2`
AUTH2=`eval $AUTH2_1  $AUTH2_2`
REV=`eval $REV1  $REV2`
RDATE=`eval $RDATE1`
echo Path: ${MAINDIR}/${GUIDIR}  $'\n'URL: ${GITREPO}  $'\n'Repository Root: ${GITREPO}  $'\n'Repsitory UUID: ${REPUID}  $'\n'Revision: ${REV}  $'\n'Branch: ${BRANCH}  $'\n'Last Changed Author: ${AUTH1}_${AUTH2}  $'\n'Last Changed Rev: ${REV}  $'\n'Last Changed Date: ${RDATE} > gitInfo.txt 
cd $WD
./genVersionHeader.sh $GUIDIR/gitInfo.txt $GUIDIR/include/gitInfoGuiTmp.h $GUIDIR/include/gitInfoGui.h 
echo "Revision Updated"
fi


cd $CALWIZPATH
echo -e "\nIn calibrationWizards"
COMMIT_TITLE=`eval $COMMIT_TITLE_SCRIPT`
echo $COMMIT_TITLE
if [ "$COMMIT_TITLE" == "updaterev" ]; then
echo "No update"
else
GITREPO=`eval $GITREPO1  $GITREPO2`
BRANCH=`eval $BRANCH1  $BRANCH2`
REPUID=`eval $REPUID1`
AUTH1=`eval $AUTH1_1  $AUTH1_2`
AUTH2=`eval $AUTH2_1  $AUTH2_2`
REV=`eval $REV1  $REV2`
RDATE=`eval $RDATE1`
echo Path: ${MAINDIR}/${CALWIZDIR}  $'\n'URL: ${GITREPO}  $'\n'Repository Root: ${GITREPO}  $'\n'Repsitory UUID: ${REPUID}  $'\n'Revision: ${REV}  $'\n'Branch: ${BRANCH}  $'\n'Last Changed Author: ${AUTH1}_${AUTH2}  $'\n'Last Changed Rev: ${REV}  $'\n'Last Changed Date: ${RDATE} > gitInfo.txt 
cd $WD
./genVersionHeader.sh $CALWIZDIR/gitInfo.txt $CALWIZDIR/gitInfoCalWizTmp.h $CALWIZDIR/gitInfoCalWiz.h 
echo "Revision Updated"
fi


cd $LIBPATH/mythenDetectorServer
echo -e "\nIn mythenDetectorServer"
COMMIT_TITLE=`eval $COMMIT_TITLE_SCRIPT`
echo $COMMIT_TITLE
if [ "$COMMIT_TITLE" == "updaterevmythen" ]; then
echo "No update"
elif [ "$COMMIT_TITLE" == "updaterev" ]; then
echo "No update"
else
GITREPO=`eval $GITREPO1  $GITREPO2`
BRANCH=`eval $BRANCH1  $BRANCH2`
REPUID=`eval $REPUID1`
AUTH1=`eval $AUTH1_1  $AUTH1_2`
AUTH2=`eval $AUTH2_1  $AUTH2_2`
FOLDERREV=`eval $FOLDERREV1  $FOLDERREV2`
RDATE=`eval $RDATE1`
echo Path: ${MAINDIR}/${LIBDIR}/mythenDetectorServer  $'\n'URL: ${GITREPO}/mythenDetectorServer  $'\n'Repository Root: ${GITREPO}  $'\n'Repsitory UUID: ${REPUID}  $'\n'Revision: ${FOLDERREV}  $'\n'Branch: ${BRANCH}  $'\n'Last Changed Author: ${AUTH1}_${AUTH2}  $'\n'Last Changed Rev: ${REV}  $'\n'Last Changed Date: ${RDATE} > gitInfo.txt 
cd $WD
./genVersionHeader.sh $LIBDIR/mythenDetectorServer/gitInfo.txt $LIBDIR/mythenDetectorServer/gitInfoMythenTmp.h $LIBDIR/mythenDetectorServer/gitInfoMythen.h 
echo "Revision Updated"
fi


cd $LIBPATH/gotthardDetectorServer
echo -e "\nIn gotthardDetectorServer"
COMMIT_TITLE=`eval $COMMIT_TITLE_SCRIPT`
echo $COMMIT_TITLE
if [ "$COMMIT_TITLE" == "updaterevgotthard" ]; then
echo "No update"
elif [ "$COMMIT_TITLE" == "updaterev" ]; then
echo "No update"
else
GITREPO=`eval $GITREPO1  $GITREPO2`
BRANCH=`eval $BRANCH1  $BRANCH2`
REPUID=`eval $REPUID1`
AUTH1=`eval $AUTH1_1  $AUTH1_2`
AUTH2=`eval $AUTH2_1  $AUTH2_2`
FOLDERREV=`eval $FOLDERREV1  $FOLDERREV2`
RDATE=`eval $RDATE1`
echo Path: ${MAINDIR}/${LIBDIR}/gotthardDetectorServer  $'\n'URL: ${GITREPO}/gotthardDetectorServer  $'\n'Repository Root: ${GITREPO}  $'\n'Repsitory UUID: ${REPUID}  $'\n'Revision: ${FOLDERREV}  $'\n'Branch: ${BRANCH}  $'\n'Last Changed Author: ${AUTH1}_${AUTH2}  $'\n'Last Changed Rev: ${REV}  $'\n'Last Changed Date: ${RDATE} > gitInfo.txt 
cd $WD
./genVersionHeader.sh $LIBDIR/gotthardDetectorServer/gitInfo.txt $LIBDIR/gotthardDetectorServer/gitInfoGotthardTmp.h $LIBDIR/gotthardDetectorServer/gitInfoGotthard.h 
echo "Revision Updated"
fi


cd $LIBPATH/moenchDetectorServer
echo -e "\nIn moenchDetectorServer"
COMMIT_TITLE=`eval $COMMIT_TITLE_SCRIPT`
echo $COMMIT_TITLE
if [ "$COMMIT_TITLE" == "updaterevmoench" ]; then
echo "No update"
elif [ "$COMMIT_TITLE" == "updaterev" ]; then
echo "No update"
else
GITREPO=`eval $GITREPO1  $GITREPO2`
BRANCH=`eval $BRANCH1  $BRANCH2`
REPUID=`eval $REPUID1`
AUTH1=`eval $AUTH1_1  $AUTH1_2`
AUTH2=`eval $AUTH2_1  $AUTH2_2`
FOLDERREV=`eval $FOLDERREV1  $FOLDERREV2`
RDATE=`eval $RDATE1`
echo Path: ${MAINDIR}/${LIBDIR}/moenchDetectorServer  $'\n'URL: ${GITREPO}/moenchDetectorServer  $'\n'Repository Root: ${GITREPO}  $'\n'Repsitory UUID: ${REPUID}  $'\n'Revision: ${FOLDERREV}  $'\n'Branch: ${BRANCH}  $'\n'Last Changed Author: ${AUTH1}_${AUTH2}  $'\n'Last Changed Rev: ${REV}  $'\n'Last Changed Date: ${RDATE} > gitInfo.txt 
cd $WD
./genVersionHeader.sh $LIBDIR/moenchDetectorServer/gitInfo.txt $LIBDIR/moenchDetectorServer/gitInfoMoenchTmp.h $LIBDIR/moenchDetectorServer/gitInfoMoench.h 
echo "Revision Updated"
fi

cd $LIBPATH/eigerDetectorServer
echo -e "\nIn eigerDetectorServer"
COMMIT_TITLE=`eval $COMMIT_TITLE_SCRIPT`
echo $COMMIT_TITLE
if [ "$COMMIT_TITLE" == "updatereveiger" ]; then
echo "No update"
elif [ "$COMMIT_TITLE" == "updaterev" ]; then
echo "No update"
else
GITREPO=`eval $GITREPO1  $GITREPO2`
BRANCH=`eval $BRANCH1  $BRANCH2`
REPUID=`eval $REPUID1`
AUTH1=`eval $AUTH1_1  $AUTH1_2`
AUTH2=`eval $AUTH2_1  $AUTH2_2`
FOLDERREV=`eval $FOLDERREV1  $FOLDERREV2`
RDATE=`eval $RDATE1`
echo Path: ${MAINDIR}/${LIBDIR}/eigerDetectorServer  $'\n'URL: ${GITREPO}/eigerDetectorServer  $'\n'Repository Root: ${GITREPO}  $'\n'Repsitory UUID: ${REPUID}  $'\n'Revision: ${FOLDERREV}  $'\n'Branch: ${BRANCH}  $'\n'Last Changed Author: ${AUTH1}_${AUTH2}  $'\n'Last Changed Rev: ${REV}  $'\n'Last Changed Date: ${RDATE} > gitInfo.txt 
cd $WD
./genVersionHeader.sh $LIBDIR/eigerDetectorServer/gitInfo.txt $LIBDIR/eigerDetectorServer/gitInfoEigerTmp.h $LIBDIR/eigerDetectorServer/gitInfoEiger.h 
echo "Revision Updated"
fi




exit 0
