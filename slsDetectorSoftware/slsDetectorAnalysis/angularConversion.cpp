#include "angularConversion.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <cstring>


using namespace std;

angularConversion::angularConversion():  angularConversionStatic(), currentPosition(0), 
					 currentPositionIndex(0)
					 
{
  //angleFunctionPointer=0;
  // registerAngleFunctionCallback(&defaultAngleFunction);

}

angularConversion::~angularConversion(){

}




double* angularConversion::convertAngles(double pos) {

  int nmod=getNMods();
  int *chansPerMod=new int[nmod];
  angleConversionConstant **angOff=new angleConversionConstant*[nmod];
  int *mF=new int[nmod];
  double fo=*fineOffset;
  double go=*globalOffset;
  int angdir=*angDirection;
    


  for (int im=0; im<nmod; im++) {
    angOff[im]=getAngularConversionPointer(im);
    mF[im]=getMoveFlag(im);
    chansPerMod[im]=getChansPerMod(im);
  }

  return angularConversionStatic::convertAngles(pos, getTotalNumberOfChannels(), chansPerMod, angOff,mF, fo, go, angdir);

}




int angularConversion::deleteMerging() {

  if (mergingBins)
    delete [] mergingBins;

  if (mergingCounts)
    delete [] mergingCounts;

  if (mergingErrors)
    delete [] mergingErrors;
  return 0;
}


int angularConversion::resetMerging() {
  getAngularConversionParameter(BIN_SIZE);

  cout << "reset merging * " << endl;

  mergingBins=new double[nBins];
  

  mergingCounts=new double[nBins];


  mergingErrors=new double[nBins];

  
  mergingMultiplicity=new int[nBins];

  return resetMerging(mergingBins, mergingCounts, mergingErrors, mergingMultiplicity);

}

int angularConversion::resetMerging(double *mp, double *mv, double *me, int *mm) {
  cout << "reset merging " << endl;

  getAngularConversionParameter(BIN_SIZE);
  if (nBins)
    return angularConversionStatic::resetMerging(mp, mv, me, mm,nBins);
  else 
    return FAIL;
}




int angularConversion::finalizeMerging() {
  cout << "finalize merging *" << endl;
  int np=finalizeMerging(mergingBins, mergingCounts, mergingErrors, mergingMultiplicity);

  if (mergingMultiplicity)
    delete [] mergingMultiplicity;

  return np;

}




int angularConversion::finalizeMerging(double *mp, double *mv, double *me, int *mm) {
  if (nBins)
    return angularConversionStatic::finalizeMerging(mp, mv, me, mm, nBins);
  else
    return FAIL;
}

int  angularConversion::addToMerging(double *p1, double *v1, double *e1, int *badChanMask ) {

  return addToMerging(p1,v1,e1,mergingBins,mergingCounts, mergingErrors, mergingMultiplicity, badChanMask);


}


int  angularConversion::addToMerging(double *p1, double *v1, double *e1, double *mp, double *mv,double *me, int *mm, int *badChanMask ) {

  int del=0;
  
  if (getAngularConversionParameter(BIN_SIZE)==0){
    cout << "no bin size " << endl;
    return FAIL;
  }
  
  if (nBins==0) {
    cout << "no bins " << endl;
    return FAIL;
  }
  
  if (p1==NULL) {
    del=1;
    p1=convertAngles();
  }
  

  int ret=angularConversionStatic::addToMerging(p1, v1, e1, mp, mv,me, mm,getTotalNumberOfChannels(), *binSize,nBins, badChanMask );
  
  
  if (del) {
    delete [] p1;
    p1=NULL;
  }
  return ret;
}



   /**
     sets the value of s angular conversion parameter
     \param c can be ANGULAR_DIRECTION, GLOBAL_OFFSET, FINE_OFFSET, BIN_SIZE
     \param v the value to be set
     \returns the actual value
  */

double angularConversion::setAngularConversionParameter(angleConversionParameter c, double v){
  

  switch (c) {
  case ANGULAR_DIRECTION:
    if (v<0)
      *angDirection=-1;
    else
      *angDirection=1;
    return *angDirection;
  case GLOBAL_OFFSET:
    *globalOffset=v;
    return *globalOffset;
  case FINE_OFFSET:
    *fineOffset=v;
    return *fineOffset;
  case BIN_SIZE:
    if (v>0) {
      *binSize=v;
      nBins=(int)(360./(*binSize))+1;
    }
    return *binSize;
  case MOVE_FLAG:
    if (moveFlag) {
      if (v>0)
	*moveFlag=1;
      else if (v==0)
	*moveFlag=0;
      return *moveFlag;
    }
    return -1;
  case SAMPLE_X:
    if (sampleDisplacement) {
      sampleDisplacement[X]=v;
      return sampleDisplacement[X];
    }
    return 0;
  case SAMPLE_Y:
    if (sampleDisplacement) {
      sampleDisplacement[Y]=v;
      return sampleDisplacement[Y];
    }
    return 0;
  default:
    return 0;
  }
}

  /**
     returns the value of an angular conversion parameter
     \param c can be ANGULAR_DIRECTION, GLOBAL_OFFSET, FINE_OFFSET, BIN_SIZE
     \returns the actual value

  */

double angularConversion::getAngularConversionParameter(angleConversionParameter c) {

  switch (c) {
  case ANGULAR_DIRECTION:
    return *angDirection;
  case GLOBAL_OFFSET:
    return *globalOffset;
  case FINE_OFFSET:
    return *fineOffset;
  case BIN_SIZE: 
    if (*binSize>0)
      nBins=(int)(360./(*binSize))+1;
    else 
      nBins=0;
    return *binSize;
  case MOVE_FLAG:
    if (moveFlag)
      return *moveFlag;
    else
      return -1;
  default:
    return 0;
  }
}




int angularConversion::setAngularConversionFile(string fname) {
  if (fname=="") {
    setAngularCorrectionMask(0);
#ifdef VERBOSE
    std::cout << "Unsetting angular conversion" <<  std::endl;
#endif
  } else {
    if (fname=="default") {
      fname=string(angConvFile);
    }
    
#ifdef VERBOSE
    std::cout << "Setting angular conversion to " << fname << std:: endl;
#endif
    if (readAngularConversionFile(fname)>=0) {
      setAngularCorrectionMask(1);
      strcpy(angConvFile,fname.c_str());
    }
  }
  return setAngularCorrectionMask();
}





  /*
      set  positions for the acquisition
      \param nPos number of positions
      \param pos array with the encoder positions
      \returns number of positions
  */
int angularConversion::setPositions(int nPos, double *pos){
  if (nPos>=0)
    *numberOfPositions=nPos; 
  for (int ip=0; ip<nPos; ip++) 
    detPositions[ip]=pos[ip]; 

  return *numberOfPositions;
}
/* 
   get  positions for the acquisition
   \param pos array which will contain the encoder positions
   \returns number of positions
*/
int angularConversion::getPositions(double *pos){
  if (pos) {
    for (int ip=0; ip<(*numberOfPositions); ip++) 
      pos[ip]=detPositions[ip];
  } 

  return  *numberOfPositions;
}
  




