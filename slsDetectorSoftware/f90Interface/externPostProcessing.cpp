#include "externPostProcessing.h"



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




int externPostProcessing::InitDataset(int *nModules,int *chPerMod,int moduleMask[],int badChans[], double ffcoeff[], double fferr[], double* tDead, double angRadius[], double angOffset[], double angCentre[], double* totalOffset, double* binSize, double *sampleX, double *sampleY)
 {
 
 init_dataset(nModules,chPerMod,moduleMask,badChans,ffcoeff,fferr,tDead,angRadius,angOffset,angCentre,totalOffset,binSize,sampleX,sampleY);
 return 0;
 
 }

int externPostProcessing::finalizeDataset(double ang[], double val[], double err[], int *np)
{
  finalize_dataset(ang, val, err, np);
  return 0;
};


int addFrame(double data[], double *pos, double *IO, double *expTime, const char *filename, int *var)
{
  add_frame(data, pos, i0, expTime, filename, var);
  return 0;
};

int calculateFlatField(int* nModules, int *chPerMod, int modMask[], int badChanMask[], double data[], double ffCoeff[], double ffErr[])
{
  calculate_flat_field(nModules, chPerMod, modMask, badChanMask, data, ffCoeff, ffErr); 
  return 0;
};
