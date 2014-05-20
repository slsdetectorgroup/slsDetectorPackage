# Script to create svnInfo.txt files and export software

#folders
MAINDIR=slsDetectorsPackage
LIBDIR=slsDetectorSoftware
RXRDIR=slsReceiverSoftware
GUIDIR=slsDetectorGui
CALWIZDIR=calibrationWizards

SVNPATH=file:///afs/psi.ch/project/sls_det_software/svn
MAINDIRSVN=$SVNPATH/$MAINDIR
LIBDIRSVN=$SVNPATH/$LIBDIR
RXRDIRSVN=$SVNPATH/$RXRDIR
GUIDIRSVN=$SVNPATH/$GUIDIR
CALWIZSVN=$SVNPATH/$GUIDIR


#export
#svn export --force $MAINDIRSVN
#cd $MAINDIR
#svn export --force  $LIBDIRSVN
#svn export --force  $RXRDIRSVN
#svn export --force  $GUIDIRSVN
#svn export --force  $CALWIZSVN


#create svnInfo.txt
svn info $LIBDIRSVN > $LIBDIR/svnInfo.txt
./genVersionHeader.sh $LIBDIR/svnInfo.txt $LIBDIR/slsDetector/svnInfoLibTmp.h $LIBDIR/slsDetector/svnInfoLib.h 

svn info $LIBDIRSVN/mythenDetectorServer > $LIBDIR/mythenDetectorServer/svnInfo.txt
./genVersionHeader.sh $LIBDIR/mythenDetectorServer/svnInfo.txt $LIBDIR/mythenDetectorServer/svnInfoMythenTmp.h $LIBDIR/mythenDetectorServer/svnInfoMythen.h 

svn info $LIBDIRSVN/gotthardDetectorServer > $LIBDIR/gotthardDetectorServer/svnInfo.txt
./genVersionHeader.sh $LIBDIR/gotthardDetectorServer/svnInfo.txt $LIBDIR/gotthardDetectorServer/svnInfoGotthardTmp.h $LIBDIR/gotthardDetectorServer/svnInfoGotthard.h 

svn info $LIBDIRSVN/moenchDetectorServer > $LIBDIR/moenchDetectorServer/svnInfo.txt
./genVersionHeader.sh $LIBDIR/moenchDetectorServer/svnInfo.txt $LIBDIR/moenchDetectorServer/svnInfoMoenchTmp.h $LIBDIR/moenchDetectorServer/svnInfoMoench.h 

svn info $LIBDIRSVN/eigerDetectorServer > $LIBDIR/eigerDetectorServer/svnInfo.txt
./genVersionHeader.sh $LIBDIR/eigerDetectorServer/svnInfo.txt $LIBDIR/eigerDetectorServer/svnInfoEigerTmp.h $LIBDIR/eigerDetectorServer/svnInfoEiger.h 


svn info $RXRDIRSVN/includes > $RXRDIR/includes/svnInfo.txt
./genVersionHeader.sh $RXRDIR/includes/svnInfo.txt $RXRDIR/includes/svnInfoReceiverTmp.h $RXRDIR/includes/svnInfoReceiver.h 

svn info $GUIDIRSVN > $GUIDIR/svnInfo.txt
./genVersionHeader.sh $GUIDIR/svnInfo.txt $GUIDIR/include/svnInfoGuiTmp.h $GUIDIR/include/svnInfoGui.h 

svn info $CALWIZSVN > $CALWIZDIR/svnInfo.txt
./genVersionHeader.sh $CALWIZDIR/svnInfo.txt $CALWIZDIR/svnInfoCalWizTmp.h $CALWIZDIR/svnInfoCalWiz.h 

exit 0
