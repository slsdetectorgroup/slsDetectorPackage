
#include "slsDetectorUsers.h"
#include "detectorData.h"
#include "multiSlsDetector.h"

slsDetectorUsers::slsDetectorUsers(int id) : myDetector(NULL){

  myDetector=new multiSlsDetector(id);

};


slsDetectorUsers::~slsDetectorUsers() {
  if (myDetector)
    delete myDetector;
};



string slsDetectorUsers::getDetectorDeveloper(){
  return myDetector->getDetectorDeveloper();
}



int slsDetectorUsers::setOnline(int const online){
  return myDetector->setOnline(online);
}

void slsDetectorUsers::startMeasurement(){
  myDetector->acquire(0); 
}

int slsDetectorUsers::stopMeasurement(){
  return myDetector->stopAcquisition();
}
 

int slsDetectorUsers::getDetectorStatus(){
  return (int)myDetector->getRunStatus();
}

string slsDetectorUsers::getFilePath(){
  return myDetector->getFilePath();
}


string slsDetectorUsers::setFilePath(string s){
  return myDetector->setFilePath(s);
}  


string slsDetectorUsers::getFileName(){
  return myDetector->getFileName();
}  

string slsDetectorUsers::setFileName(string s){
  return myDetector->setFileName(s);
}  
  
int slsDetectorUsers::getFileIndex(){
  return (int)myDetector->getFileIndex();
}
  
int slsDetectorUsers::setFileIndex(int i){
  return (int)myDetector->setFileIndex(i);
}

string slsDetectorUsers::getFlatFieldCorrectionDir(){
  return myDetector->getFlatFieldCorrectionDir();
} 

string slsDetectorUsers::setFlatFieldCorrectionDir(string dir){
  return myDetector->setFlatFieldCorrectionDir(dir);
}
 
string slsDetectorUsers::getFlatFieldCorrectionFile(){
  return myDetector->getFlatFieldCorrectionFile();
};
  
int slsDetectorUsers::setFlatFieldCorrectionFile(string fname){
  return myDetector->setFlatFieldCorrectionFile(fname);
} 

int slsDetectorUsers::enableFlatFieldCorrection(int i){
  return myDetector->enableFlatFieldCorrection(i);
}

int slsDetectorUsers::enableCountRateCorrection(int i){  
  return myDetector->enableCountRateCorrection(i);
}

int slsDetectorUsers::enablePixelMaskCorrection(int i){
  return myDetector->enablePixelMaskCorrection(i);
}
int slsDetectorUsers::enableAngularConversion(int i){
  return myDetector->enableAngularConversion(i);
}
int slsDetectorUsers::enableWriteToFile(int i){
  return myDetector->enableWriteToFile(i);
}

int slsDetectorUsers::setPositions(int nPos, double *pos){
  return myDetector->setPositions(nPos, pos);
}

int slsDetectorUsers::getPositions(double *pos){
  return myDetector->getPositions(pos);
}

int slsDetectorUsers::setDetectorSize(int x0, int y0, int nx, int ny){
  int nmod=nx/(myDetector->getChansPerMod(0));
  cout << myDetector->getChansPerMod(0) << " " << nx << " " << nmod << endl;
  return myDetector->setNumberOfModules(nmod)*myDetector->getChansPerMod(0);}

int slsDetectorUsers::getDetectorSize(int &x0, int &y0, int &nx, int &ny){ 
  y0=0; 
  ny=1; 
  x0=0; 
  nx=myDetector->setNumberOfModules()*myDetector->getChansPerMod(0); 
  return nx;
}

int slsDetectorUsers::getMaximumDetectorSize(int &nx, int &ny){
  ny=1;
  nx=myDetector->getMaxNumberOfModules()*myDetector->getChansPerMod(0); 
  return nx;
}

int slsDetectorUsers::setBitDepth(int i){
  return myDetector->setDynamicRange(i);
}


int slsDetectorUsers::setSettings(int isettings){
  return myDetector->slsDetectorBase::setSettings(isettings);
}

int slsDetectorUsers::getThresholdEnergy(){
  return myDetector->getThresholdEnergy(-1);
}  

int slsDetectorUsers::setThresholdEnergy(int e_eV){
  return myDetector->setThresholdEnergy(e_eV);
}

int slsDetectorUsers::getBeamEnergy(){
  return 2*myDetector->getThresholdEnergy();
}  

int slsDetectorUsers::setBeamEnergy(int e_eV){
  return 2*myDetector->setThresholdEnergy(e_eV/2);
}

int64_t slsDetectorUsers::setExposureTime(int64_t t){
  return myDetector->setExposureTime(t);
}
int64_t slsDetectorUsers::setExposurePeriod(int64_t t){
  return myDetector->setExposurePeriod(t);
}
  
int64_t slsDetectorUsers::setDelayAfterTrigger(int64_t t){
  return myDetector->setDelayAfterTrigger(t);
}

double slsDetectorUsers::setExposureTime(double t){
  int64_t tms = t * 1E+9;
  return  1E-9 * (double)myDetector->setExposureTime(tms);
}

double slsDetectorUsers::setExposurePeriod(double t){
  int64_t tms = t * 1E+9;
  return  1E-9 * (double)myDetector->setExposurePeriod(tms);
}

double slsDetectorUsers::setDelayAfterTrigger(double t){
  int64_t tms = t * 1E+9;
  return 1E-9 * (double)(myDetector->setDelayAfterTrigger(tms));
}

int64_t slsDetectorUsers::setNumberOfGates(int64_t t){
  return myDetector->setNumberOfGates(t);
} 

int64_t slsDetectorUsers::setNumberOfFrames(int64_t t){
  return myDetector->setNumberOfFrames(t);
}

int64_t slsDetectorUsers::setNumberOfCycles(int64_t t){
  return myDetector->setNumberOfCycles(t);
}
  
int slsDetectorUsers::setTimingMode(int pol){
  return myDetector->setTimingMode(pol);
}

int slsDetectorUsers::readConfigurationFile(string const fname){
  return myDetector->readConfigurationFile(fname);
}  

int slsDetectorUsers::dumpDetectorSetup(string const fname){
  return myDetector->dumpDetectorSetup(fname);
} 

int slsDetectorUsers::retrieveDetectorSetup(string const fname){
  return myDetector->retrieveDetectorSetup(fname);
}

string slsDetectorUsers::getDetectorType(){
  return myDetector->sgetDetectorsType();
}


  
void slsDetectorUsers::initDataset(int refresh){
  myDetector->initDataset(refresh);
}




void slsDetectorUsers::addFrame(double *data, double pos, double i0, double t, string fname, double var){
  myDetector->addFrame(data,pos,i0,t,fname,var);
}


void slsDetectorUsers::finalizeDataset(double *a, double *v, double *e, int &np){
  myDetector->finalizeDataset(a, v, e, np);
}



int slsDetectorUsers::setReceiverMode(int n){
	return myDetector->setReadReceiverFrequency(1,n);
}






void slsDetectorUsers::registerDataCallback(int( *userCallback)(detectorData*, int, void*), void *pArg){
  myDetector->registerDataCallback(userCallback,pArg);
}

void slsDetectorUsers::registerRawDataCallback(int( *userCallback)(double*, int, void*), void *pArg){
  myDetector->registerRawDataCallback(userCallback,pArg);
}

void slsDetectorUsers::registerAcquisitionFinishedCallback(int( *func)(double,int, void*), void *pArg){
  myDetector->registerAcquisitionFinishedCallback(func,pArg);
}

void slsDetectorUsers::registerGetPositionCallback( double (*func)(void*),void *arg){
  myDetector->registerGetPositionCallback(func,arg);
}

void slsDetectorUsers::registerConnectChannelsCallback( int (*func)(void*),void *arg){
  myDetector->registerConnectChannelsCallback(func,arg);
}

void slsDetectorUsers::registerDisconnectChannelsCallback( int (*func)(void*),void *arg){
  myDetector->registerDisconnectChannelsCallback(func,arg);
}  

void slsDetectorUsers::registerGoToPositionCallback( int (*func)(double,void*),void *arg){
  myDetector->registerGoToPositionCallback(func,arg);
}

void slsDetectorUsers::registerGoToPositionNoWaitCallback( int (*func)(double,void*),void *arg){
  myDetector->registerGoToPositionNoWaitCallback(func,arg);
}

void slsDetectorUsers::registerGetI0Callback( double (*func)(int,void*),void *arg){
  myDetector->registerGetI0Callback(func,arg);
}



  
