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
svn export --force $MAINDIRSVN
cd $MAINDIR
svn export --force  $LIBDIRSVN
svn export --force  $CLIENTDIRSVN
svn export --force  $GUIDIRSVN


#create svnInfo.txt
cd $LIBDIR
svn info $LIBDIRSVN > svnInfo.txt
cd ../$GUIDIR
svn info $GUIDIRSVN > svnInfo.txt

exit 0
