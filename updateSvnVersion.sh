# Script to create gitInfo.txt files and export software

#git clone git@gitorious.psi.ch:sls_det_software/sls_detectors_package.git slsDetectorsPackage
#cd slsDetectorsPackage
#git clone git@gitorious.psi.ch:sls_det_software/sls_detector_software.git slsDetectorSoftware
#git clone git@gitorious.psi.ch:sls_det_software/sls_detector_gui.git slsDetectorGui
#git clone git@gitorious.psi.ch:sls_det_software/sls_receiver_software.git slsReceiverSoftware
#git clone git@gitorious.psi.ch:sls_det_software/calibration_wizards.git calibrationWizards

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



#create gitInfo.txt
#have to go into path to execute some git commands, different variables are upto Revision

cd $LIBPATH
GITREPO=`eval $GITREPO1  $GITREPO2`
BRANCH=`eval $BRANCH1  $BRANCH2`
REPUID=`eval $REPUID1`
AUTH1=`eval $AUTH1_1  $AUTH1_2`
AUTH2=`eval $AUTH2_1  $AUTH2_2`
REV=`eval $REV1  $REV2`
RDATE=`eval $RDATE1`
echo Path: ${MAINDIR}/${LIBDIR}  $'\n'URL: ${GITREPO}  $'\n'Repository Root: ${GITREPO}  $'\n'Repsitory UUID: ${REPUID}  $'\n'Revision: ${REV}  $'\n'Branch: ${BRANCH}  $'\n'Last Changed Author: ${AUTH1}_${AUTH2}  $'\n'Last Changed Rev: ${REV}  $'\n'Last Changed Date: ${RDATE} > gitInfo.txt 

cd $RXRPATH
GITREPO=`eval $GITREPO1  $GITREPO2`
BRANCH=`eval $BRANCH1  $BRANCH2`
REPUID=`eval $REPUID1`
AUTH1=`eval $AUTH1_1  $AUTH1_2`
AUTH2=`eval $AUTH2_1  $AUTH2_2`
REV=`eval $REV1  $REV2`
RDATE=`eval $RDATE1`
echo Path: ${MAINDIR}/${RXRDIR}  $'\n'URL: ${GITREPO}  $'\n'Repository Root: ${GITREPO}  $'\n'Repsitory UUID: ${REPUID}  $'\n'Revision: ${REV}  $'\n'Branch: ${BRANCH}  $'\n'Last Changed Author: ${AUTH1}_${AUTH2}  $'\n'Last Changed Rev: ${REV}  $'\n'Last Changed Date: ${RDATE} > gitInfo.txt 

cd $GUIPATH
GITREPO=`eval $GITREPO1  $GITREPO2`
BRANCH=`eval $BRANCH1  $BRANCH2`
REPUID=`eval $REPUID1`
AUTH1=`eval $AUTH1_1  $AUTH1_2`
AUTH2=`eval $AUTH2_1  $AUTH2_2`
REV=`eval $REV1  $REV2`
RDATE=`eval $RDATE1`
echo Path: ${MAINDIR}/${GUIDIR}  $'\n'URL: ${GITREPO}  $'\n'Repository Root: ${GITREPO}  $'\n'Repsitory UUID: ${REPUID}  $'\n'Revision: ${REV}  $'\n'Branch: ${BRANCH}  $'\n'Last Changed Author: ${AUTH1}_${AUTH2}  $'\n'Last Changed Rev: ${REV}  $'\n'Last Changed Date: ${RDATE} > gitInfo.txt 

cd $CALWIZPATH
GITREPO=`eval $GITREPO1  $GITREPO2`
BRANCH=`eval $BRANCH1  $BRANCH2`
REPUID=`eval $REPUID1`
AUTH1=`eval $AUTH1_1  $AUTH1_2`
AUTH2=`eval $AUTH2_1  $AUTH2_2`
REV=`eval $REV1  $REV2`
RDATE=`eval $RDATE1`
echo Path: ${MAINDIR}/${CALWIZDIR}  $'\n'URL: ${GITREPO}  $'\n'Repository Root: ${GITREPO}  $'\n'Repsitory UUID: ${REPUID}  $'\n'Revision: ${REV}  $'\n'Branch: ${BRANCH}  $'\n'Last Changed Author: ${AUTH1}_${AUTH2}  $'\n'Last Changed Rev: ${REV}  $'\n'Last Changed Date: ${RDATE} > gitInfo.txt 

cd $LIBPATH/mythenDetectorServer
GITREPO=`eval $GITREPO1  $GITREPO2`
BRANCH=`eval $BRANCH1  $BRANCH2`
REPUID=`eval $REPUID1`
AUTH1=`eval $AUTH1_1  $AUTH1_2`
AUTH2=`eval $AUTH2_1  $AUTH2_2`
FOLDERREV=`eval $FOLDERREV1  $FOLDERREV2`
RDATE=`eval $RDATE1`
echo Path: ${MAINDIR}/${LIBDIR}/mythenDetectorServer  $'\n'URL: ${GITREPO}/mythenDetectorServer  $'\n'Repository Root: ${GITREPO}  $'\n'Repsitory UUID: ${REPUID}  $'\n'Revision: ${FOLDERREV}  $'\n'Branch: ${BRANCH}  $'\n'Last Changed Author: ${AUTH1}_${AUTH2}  $'\n'Last Changed Rev: ${REV}  $'\n'Last Changed Date: ${RDATE} > gitInfo.txt 

cd $LIBPATH/gotthardDetectorServer
GITREPO=`eval $GITREPO1  $GITREPO2`
BRANCH=`eval $BRANCH1  $BRANCH2`
REPUID=`eval $REPUID1`
AUTH1=`eval $AUTH1_1  $AUTH1_2`
AUTH2=`eval $AUTH2_1  $AUTH2_2`
FOLDERREV=`eval $FOLDERREV1  $FOLDERREV2`
RDATE=`eval $RDATE1`
echo Path: ${MAINDIR}/${LIBDIR}/gotthardDetectorServer  $'\n'URL: ${GITREPO}/gotthardDetectorServer  $'\n'Repository Root: ${GITREPO}  $'\n'Repsitory UUID: ${REPUID}  $'\n'Revision: ${FOLDERREV}  $'\n'Branch: ${BRANCH}  $'\n'Last Changed Author: ${AUTH1}_${AUTH2}  $'\n'Last Changed Rev: ${REV}  $'\n'Last Changed Date: ${RDATE} > gitInfo.txt 

cd $LIBPATH/moenchDetectorServer
GITREPO=`eval $GITREPO1  $GITREPO2`
BRANCH=`eval $BRANCH1  $BRANCH2`
REPUID=`eval $REPUID1`
AUTH1=`eval $AUTH1_1  $AUTH1_2`
AUTH2=`eval $AUTH2_1  $AUTH2_2`
FOLDERREV=`eval $FOLDERREV1  $FOLDERREV2`
RDATE=`eval $RDATE1`
echo Path: ${MAINDIR}/${LIBDIR}/moenchDetectorServer  $'\n'URL: ${GITREPO}/moenchDetectorServer  $'\n'Repository Root: ${GITREPO}  $'\n'Repsitory UUID: ${REPUID}  $'\n'Revision: ${FOLDERREV}  $'\n'Branch: ${BRANCH}  $'\n'Last Changed Author: ${AUTH1}_${AUTH2}  $'\n'Last Changed Rev: ${REV}  $'\n'Last Changed Date: ${RDATE} > gitInfo.txt 

cd $LIBPATH/eigerDetectorServer
GITREPO=`eval $GITREPO1  $GITREPO2`
BRANCH=`eval $BRANCH1  $BRANCH2`
REPUID=`eval $REPUID1`
AUTH1=`eval $AUTH1_1  $AUTH1_2`
AUTH2=`eval $AUTH2_1  $AUTH2_2`
FOLDERREV=`eval $FOLDERREV1  $FOLDERREV2`
RDATE=`eval $RDATE1`
echo Path: ${MAINDIR}/${LIBDIR}/eigerDetectorServer  $'\n'URL: ${GITREPO}/eigerDetectorServer  $'\n'Repository Root: ${GITREPO}  $'\n'Repsitory UUID: ${REPUID}  $'\n'Revision: ${FOLDERREV}  $'\n'Branch: ${BRANCH}  $'\n'Last Changed Author: ${AUTH1}_${AUTH2}  $'\n'Last Changed Rev: ${REV}  $'\n'Last Changed Date: ${RDATE} > gitInfo.txt 





#creating the header files
cd $WD
./genVersionHeader.sh $LIBDIR/gitInfo.txt $LIBDIR/slsDetector/gitInfoLibTmp.h $LIBDIR/slsDetector/gitInfoLib.h 
./genVersionHeader.sh $RXRDIR/gitInfo.txt $RXRDIR/include/gitInfoReceiverTmp.h $RXRDIR/include/gitInfoReceiver.h
./genVersionHeader.sh $GUIDIR/gitInfo.txt $GUIDIR/include/gitInfoGuiTmp.h $GUIDIR/include/gitInfoGui.h 
./genVersionHeader.sh $CALWIZDIR/gitInfo.txt $CALWIZDIR/gitInfoCalWizTmp.h $CALWIZDIR/gitInfoCalWiz.h 
./genVersionHeader.sh $LIBDIR/mythenDetectorServer/gitInfo.txt $LIBDIR/mythenDetectorServer/gitInfoMythenTmp.h $LIBDIR/mythenDetectorServer/gitInfoMythen.h 
./genVersionHeader.sh $LIBDIR/gotthardDetectorServer/gitInfo.txt $LIBDIR/gotthardDetectorServer/gitInfoGotthardTmp.h $LIBDIR/gotthardDetectorServer/gitInfoGotthard.h 
./genVersionHeader.sh $LIBDIR/moenchDetectorServer/gitInfo.txt $LIBDIR/moenchDetectorServer/gitInfoMoenchTmp.h $LIBDIR/moenchDetectorServer/gitInfoMoench.h 
./genVersionHeader.sh $LIBDIR/eigerDetectorServer/gitInfo.txt $LIBDIR/eigerDetectorServer/gitInfoEigerTmp.h $LIBDIR/eigerDetectorServer/gitInfoEiger.h 


exit 0
