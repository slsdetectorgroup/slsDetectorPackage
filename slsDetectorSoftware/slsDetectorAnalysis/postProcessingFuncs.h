#ifndef POSTPROCESSINGFUNCS_H
#define POSTPROCESSINGFUNC_H



#include <string>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <string>
#include <sstream>
#include <queue>
#include <math.h>

#include "angularConversionStatic.h"
class angleConversionConstant;

using namespace std;


class postProcessingFuncs : public virtual angularConversionStatic

{

 public:
  postProcessingFuncs(int *nModules=NULL,int *chPerMod=NULL,int *modMask=NULL,int *badChMask=NULL, double *ffcoeff=NULL, double *fferr=NULL, double* t=NULL, int *dir=NULL, double *angRadius=NULL, double *angOffset=NULL, double *angCentre=NULL, double* to=NULL, double* bs=NULL, double *sX=NULL, double *sY=NULL);


  ~postProcessingFuncs();
  
  
  int initDataset(int *nModules,int *chPerMod,int modMask[],int badCh[], double ffcoeff[], double fferr[], double* tDead, int *dir, double angRadius[], double angOffset[], double angCentre[], double* to, double* bs, double *sX, double *sY);
  
  int initDataset();
  
  
  int finalizeDataset(double ang[], double val[], double err[], int *np);
  
  int addFrame(double data[], double *pos, double *IO, double *expTime, const char *filename, double *var=0);
  
  static int calculateFlatField(int* nModules, int *chPerMod, int moduleMask[], int badChannelMask[], double ffData[], double ffCoeff[], double ffErr[]);
  
  static int flatFieldCorrect(double datain, double errin, double &dataout, double &errout, double ffcoefficient, double fferr);


  static int rateCorrect(double datain, double errin, double &dataout, double &errout, double tau, double t);
   
 private:
  void deletePointers();


  int nMods;
  int *chansPerMod;
  int *moduleMask;
  int *badChannelMask;
  double *ffCoeff;
  double *ffErr;
  double tDead;
  int angDir;
  angleConversionConstant **angConv;
  double totalOffset;
  double binSize;
  double sampleX;
  double sampleY;
  int totalChans;
 
  int nBins;

  double totalI0;


  double *mp, *mv,*me;
  int *mm;

}; 


#endif
