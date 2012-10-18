#include "fileIO.h"

			   
				   

/* I/O */

/* generates file name without extension*/

string fileIO::createFileName() {
  currentFileName=fileIOStatic::createFileName(filePath,	\
					       fileName, \
					       getActionMask(),	\
					       getCurrentScanVariable(0),	\
					       getScanPrecision(0),		\
					       getCurrentScanVariable(1),	\
					       getScanPrecision(1),		\
					       getCurrentPositionIndex(),	\
					       getNumberOfPositions(),		\
					       *fileIndex,	      \
					       frameIndex,		  \
					       detIndex			  \
					       );
  return currentFileName;
  
}



/* generates file prefix for receivers */

string fileIO::createReceiverFilePrefix() {
  currentReceiverFilePrefix=fileIOStatic::createReceiverFilePrefix(fileName, \
								   getActionMask(),	\
								   getCurrentScanVariable(0),	\
								   getScanPrecision(0),		\
								   getCurrentScanVariable(1),	\
								   getScanPrecision(1),		\
								   getCurrentPositionIndex(),	\
								   getNumberOfPositions(),		\
								   detIndex			  \
								   );
  return currentReceiverFilePrefix;

}



/*writes raw data file */

int fileIO::writeDataFile(string fname, double *data, double *err, double *ang, char dataformat, int nch){
  if (nch==-1)
    nch=getTotalNumberOfChannels();
  
  return fileIOStatic::writeDataFile(fname, nch, data, err, ang, dataformat);

}
int fileIO::writeDataFile(ofstream &outfile, double *data, double *err, double *ang, char dataformat, int nch, int offset){
  if (nch==-1)
    nch=getTotalNumberOfChannels();
  
  return fileIOStatic::writeDataFile(outfile, nch, data, err, ang, dataformat, offset);

}


int fileIO::writeDataFile(string fname, int *data){
  
  return fileIOStatic::writeDataFile(fname, getTotalNumberOfChannels(), data);
}

int fileIO::writeDataFile(ofstream &outfile, int *data, int offset){
  
  return fileIOStatic::writeDataFile(outfile, getTotalNumberOfChannels(), data, offset);
}





int fileIO::writeDataFile(string fname, short int *data){

  return fileIOStatic::writeDataFile(fname, getTotalNumberOfChannels(), data);
}

int fileIO::writeDataFile(ofstream &outfile, short int *data, int offset){

  return fileIOStatic::writeDataFile(outfile, getTotalNumberOfChannels(), data, offset);
}




int fileIO::readDataFile(string fname, double *data, double *err, double *ang, char dataformat) {
  return fileIOStatic::readDataFile(getTotalNumberOfChannels(), fname, data, err, ang, dataformat);

}

int fileIO::readDataFile(ifstream &infile, double *data, double *err, double *ang, char dataformat, int offset) {
  return fileIOStatic::readDataFile(getTotalNumberOfChannels(), infile, data, err, ang, dataformat, offset);

}



int fileIO::readDataFile(string fname, int *data){

  return fileIOStatic::readDataFile(fname, data, getTotalNumberOfChannels());
};


int fileIO::readDataFile(ifstream &infile, int *data, int offset){

  return fileIOStatic::readDataFile(infile, data, getTotalNumberOfChannels(), offset);
};





int fileIO::readDataFile(string fname, short int *data){

  return fileIOStatic::readDataFile(fname, data, getTotalNumberOfChannels());
};


int fileIO::readDataFile(ifstream &infile, short int *data, int offset){

  return fileIOStatic::readDataFile(infile, data, getTotalNumberOfChannels(),offset);
};

