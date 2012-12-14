# Script to create svnInfo.txt files and export software

#folders
MAINDIR=newMythenSoftware
LIBDIR=slsDetectorSoftware
CLIENTDIR=slsDetectorClient
GUIDIR=slsDetectorGuiOriginal

SVNPATH=file:///afs/psi.ch/project/sls_det_software/svn
MAINDIRSVN=$SVNPATH/$MAINDIR
LIBDIRSVN=$SVNPATH/$LIBDIR
CLIENTDIRSVN=$SVNPATH/$CLIENTDIR
GUIDIRSVN=$SVNPATH/$GUIDIR


#export
svn export $MAINDIRSVN
cd $MAINDIR
svn export $LIBDIRSVN
svn export $CLIENTDIRSVN
svn export $GUIDIRSVN


#create svnInfo.txt
cd $LIBDIR
svn info $LIBDIRSVN > svnInfo.txt

exit 0
