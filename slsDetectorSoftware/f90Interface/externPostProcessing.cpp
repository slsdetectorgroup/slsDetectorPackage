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

int externPostProcessing::finalizeDataset(double ang[], double val[], double err[])
{
cout<<"Finalize Dataset"<<endl;
return 0;
};


int addFrame(double data[], double *pos, double *IO, double expTime, const char *filename, int *var=0)
{
cout<<"Do Processing"<<endl;
return 0;
};

int calculateFlatField(int* nModules, int badChannelMask[], double ffData[], double ffCoeff[], double ffErr[])
{
cout<<"Outputs Flat Field Coefficient and errors from the input data"<<endl;
return 0;
};
