#ifndef EXTERNPOSTPROCESSING_H
#define EXTERNPOSTPROCESSING_H


#include "detectorData.h"
#include "sls_detector_defs.h"
#include "slsDetectorBase_Standalone.h"



#include <string>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <string>
#include <sstream>
#include <queue>
#include <math.h>


using namespace std;



extern "C" {

void init_dataset(int *nMod, int *chPerMod, int* modMask, int *badChanMask, double *ffCoeff, double *ffErr, double *tDead, double *angRadius, double *angOffset, double *angCenter, double *totalOff, double *binSize, double * sampleX, double* sampleY);
void finalize_dataset(double *outang, double *outval, double *outerr, int *np);
void add_frame(double *data, double *pos, double *i0, char *fn, double *var);
void calculate_flat_field(int *nMod, int *chPerMod, int *modMask,int *badChanMask, double *data, double *ffc, double *fferr); 
}



class externPostProcessing :  public virtual slsDetectorBase1

{




 public:
  externPostProcessing(){};
  virtual ~externPostProcessing(){};


 static int InitDataset(int *nModules,int *chPerMod,int moduleMask[],int badChans[], double ffcoeff[], double fferr[], double* tDead, double angRadius[], double angOffset[], double angCentre[], double* totalOffset, double* binSize, double *sampleX, double *sampleY);
 


 int finalizeDataset(double ang[], double val[], double err[]);

 int addFrame(double data[], double *pos, double *IO, double expTime, const char *filename, int *var=0);

 int calculateFlatField(int* nModules, int badChannelMask[], double ffData[], double ffCoeff[], double ffErr[]);




 }; 


#endif
