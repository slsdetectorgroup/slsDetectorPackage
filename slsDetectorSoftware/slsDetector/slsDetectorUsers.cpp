#include "slsDetectorUsers.h"
#include "detectorData.h"
#include "multiSlsDetector.h"
#include "multiSlsDetectorCommand.h"


using namespace std;

slsDetectorUsers::slsDetectorUsers(int& ret, int id) : myDetector(0), myCmd(0){
	try {
		myDetector=new multiSlsDetector(id);
	} catch(...) {
		ret = 1;
		return;
	}
	myCmd=new multiSlsDetectorCommand(myDetector);
	ret = 0;

}


slsDetectorUsers::~slsDetectorUsers() {
  if (myDetector)
    delete myDetector;
}

string slsDetectorUsers::getDetectorDeveloper(){
  return myDetector->getDetectorDeveloper();
}

int slsDetectorUsers::setOnline(int const online){
  return myDetector->setOnline(online);
}

int slsDetectorUsers::setReceiverOnline(int const online){
  return myDetector->setReceiverOnline(online);
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
}
  
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
    // only one roi
    slsDetectorDefs::ROI roi[1];
    roi[0].xmin = x0;
    roi[0].ymin = y0;
    roi[0].xmax = x0 + nx;
    roi[0].ymax = y0 + ny;
    return myDetector->setROI(1, roi);
}

int slsDetectorUsers::getDetectorSize(int &x0, int &y0, int &nx, int &ny){ 
    // default (no roi)
    y0=0;
    x0=0;
    nx=myDetector->getTotalNumberOfChannels(slsDetectorDefs::X);
    ny=myDetector->getTotalNumberOfChannels(slsDetectorDefs::Y);

    int n = 0;
    slsDetectorDefs::ROI* roi = myDetector->getROI(n);

    // roi
    if (roi != NULL && n == 1) {
        x0 = roi[0].xmin;
        y0 = roi[0].ymin;
        nx = roi[0].xmax - roi[0].xmin;
        ny = roi[0].ymax - roi[0].ymin;
    }

    if (roi != NULL)
        delete [] roi;

    return nx*ny;
}

int slsDetectorUsers::getMaximumDetectorSize(int &nx, int &ny){
  nx=myDetector->getMaxNumberOfChannelsPerDetector(slsDetectorDefs::X);
  ny=myDetector->getMaxNumberOfChannelsPerDetector(slsDetectorDefs::Y);
  return nx*ny;
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

int slsDetectorUsers::setThresholdEnergy(int e_ev, int tb, int isettings, int id) {
  return myDetector->slsDetectorBase::setThresholdEnergy(e_ev, tb, isettings, id);
}

double slsDetectorUsers::setExposureTime(double t, bool inseconds, int imod){
	if(!inseconds)
		return myDetector->setExposureTime((int64_t)t,imod);

	// + 0.5 to round for precision lost from converting double to int64_t
	int64_t tms = (int64_t)(t * (1E+9) + 0.5);
	if (t < 0) tms = -1;
	tms = myDetector->setExposureTime(tms,imod);
	if (tms < 0)
		return -1;
	return  ((1E-9) * (double)tms);
}

double slsDetectorUsers::setExposurePeriod(double t, bool inseconds, int imod){
	if(!inseconds)
		return myDetector->setExposurePeriod((int64_t)t,imod);

	// + 0.5 to round for precision lost from converting double to int64_t
	int64_t tms = (int64_t)(t * (1E+9) + 0.5);
	if (t < 0) tms = -1;
	tms = myDetector->setExposurePeriod(tms,imod);
	if (tms < 0)
		return -1;
	return  ((1E-9) * (double)tms);
}

double slsDetectorUsers::setDelayAfterTrigger(double t, bool inseconds, int imod){
	if(!inseconds)
		return myDetector->setDelayAfterTrigger((int64_t)t,imod);

	// + 0.5 to round for precision lost from converting double to int64_t
	int64_t tms = (int64_t)(t * (1E+9) + 0.5);
	if (t < 0) tms = -1;
	tms = myDetector->setDelayAfterTrigger(tms,imod);
	if (tms < 0)
		return -1;
	return  ((1E-9) * (double)tms);
}

int64_t slsDetectorUsers::setNumberOfGates(int64_t t, int imod){
  return myDetector->setNumberOfGates(t,imod);
} 

int64_t slsDetectorUsers::setNumberOfFrames(int64_t t, int imod){
  return myDetector->setNumberOfFrames(t,imod);
}

int64_t slsDetectorUsers::setNumberOfCycles(int64_t t, int imod){
  return myDetector->setNumberOfCycles(t,imod);
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
	return myDetector->setReadReceiverFrequency(n);
}

int slsDetectorUsers::enableDataStreamingFromReceiver(int i){
	return myDetector->enableDataStreamingFromReceiver(i);
}

int slsDetectorUsers::enableDataStreamingToClient(int i){
	return myDetector->enableDataStreamingToClient(i);
}

int slsDetectorUsers::setReceiverDataStreamingOutPort(int i){
	return myDetector->setReceiverDataStreamingOutPort(i);
}

int slsDetectorUsers::setClientDataStreamingInPort(int i){
	return myDetector->setClientDataStreamingInPort(i);
}

string slsDetectorUsers::setReceiverDataStreamingOutIP(string ip){
	return myDetector->setReceiverDataStreamingOutIP(ip);
}

string slsDetectorUsers::setClientDataStreamingInIP(string ip){
	return myDetector->setClientDataStreamingInIP(ip);
}

int64_t slsDetectorUsers::getModuleFirmwareVersion(int imod){
	return myDetector->getModuleFirmwareVersion(imod);
}

int64_t slsDetectorUsers::getModuleSerialNumber(int imod){
	return myDetector->getModuleSerialNumber(imod);
}

int64_t slsDetectorUsers::getDetectorFirmwareVersion(int imod){
	return myDetector->getDetectorFirmwareVersion(imod);
}

int64_t slsDetectorUsers::getDetectorSerialNumber(int imod){
	return myDetector->getDetectorSerialNumber(imod);
}

int64_t slsDetectorUsers::getDetectorSoftwareVersion(int imod){
	return myDetector->getDetectorSoftwareVersion(imod);
}

int64_t slsDetectorUsers::getThisSoftwareVersion(){
	return myDetector->getThisSoftwareVersion();
}

int slsDetectorUsers::enableGapPixels(int enable) {
    return myDetector->enableGapPixels(enable);
}

std::string slsDetectorUsers::setReceiverFramesDiscardPolicy(std::string f) {
	return myDetector->getReceiverFrameDiscardPolicy(
			myDetector->setReceiverFramesDiscardPolicy(
					myDetector->getReceiverFrameDiscardPolicy(f)));
}

int slsDetectorUsers::setReceiverPartialFramesPadding(int f) {
	return myDetector->setReceiverPartialFramesPadding(f);
}

int slsDetectorUsers::setReceiverFramesPerFile(int f) {
	return myDetector->setReceiverFramesPerFile(f);
}

int slsDetectorUsers::sendSoftwareTrigger() {
	return myDetector->sendSoftwareTrigger();
}

double slsDetectorUsers::getMeasuredPeriod(bool inseconds, int imod) {
	if(!inseconds)
		return myDetector->getTimeLeft(slsReceiverDefs::MEASURED_PERIOD, imod);

	int64_t tms = myDetector->getTimeLeft(slsReceiverDefs::MEASURED_PERIOD, imod);
	if (tms < 0)
		return -1;
	return  ((1E-9) * (double)tms);
}

double slsDetectorUsers::getMeasuredSubFramePeriod(bool inseconds, int imod) {
	if(!inseconds)
		return myDetector->getTimeLeft(slsReceiverDefs::MEASURED_SUBPERIOD, imod);

	int64_t tms = myDetector->getTimeLeft(slsReceiverDefs::MEASURED_SUBPERIOD, imod);
	if (tms < 0)
		return -1;
	return  ((1E-9) * (double)tms);
}

void slsDetectorUsers::registerDataCallback(int( *userCallback)(detectorData*, int, int, void*), void *pArg){
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


string slsDetectorUsers::putCommand(int narg, char *args[], int pos){
	if(narg < 2)
		return string("Error: Insufficient Parameters");
	return myCmd->putCommand(narg, args, pos);
}

string slsDetectorUsers::getCommand(int narg, char *args[], int pos){
	if(narg < 1)
		return string("Error: Insufficient Parameters");
	return myCmd->getCommand(narg, args, pos);
}


int slsDetectorUsers::setClockDivider(int value) {
	return myDetector->setClockDivider(value);
}

int slsDetectorUsers::setParallelMode(int value) {
	if(value >= 0)
		myDetector->setParallelMode(value);
	return myDetector->getParallelMode();
}

int slsDetectorUsers::setOverflowMode(int value) {
	if(value >= 0)
		myDetector->setOverflowMode(value);
	return myDetector->getOverflowMode();
}

int slsDetectorUsers::setAllTrimbits(int val, int id) {
	return myDetector->setAllTrimbits(val, id);
}

int slsDetectorUsers::setDAC(string dac, int val, int id) {
	int dacindex = myDetector->getDACIndex(dac);
	if(dacindex == -1) return -9999;
	return myDetector->setDACValue(val, dacindex, id);
}

int slsDetectorUsers::getADC(string adc, int id) {
	int adcindex = myDetector->getADCIndex(adc);
	if(adcindex == -1) return -9999;
	return myDetector->getADCValue(adcindex, id);
}

int slsDetectorUsers::startReceiver() {
	return myDetector->startReceiver();
}

int slsDetectorUsers::stopReceiver() {
	return myDetector->stopReceiver();
}

int slsDetectorUsers::startAcquisition() {
	return myDetector->startAcquisition();
}

int slsDetectorUsers::stopAcquisition() {
	return myDetector->stopAcquisition();
}

int slsDetectorUsers::setReceiverSilentMode(int i) {
	return myDetector->setReceiverSilentMode(i);
}

int slsDetectorUsers::setHighVoltage(int i) {
	return myDetector->setHighVoltage(i);
}

int slsDetectorUsers::resetFramesCaughtInReceiver() {
    return myDetector->resetFramesCaught();
}

int slsDetectorUsers::setReceiverFifoDepth(int i) {
    return myDetector->setReceiverFifoDepth(i);
}

int slsDetectorUsers::setFlowControl10G(int i) {
    return myDetector->setFlowControl10G(i);
}

int slsDetectorUsers::setTenGigabitEthernet(int i) {
    return myDetector->enableTenGigabitEthernet(i);
}

int slsDetectorUsers::getNMods() {
    return myDetector->getNMods();
}

double slsDetectorUsers::setSubFrameExposureTime(double t, bool inseconds, int imod){
	if(!inseconds)
		return myDetector->setSubFrameExposureTime((int64_t)t,imod);
	else {
		// + 0.5 to round for precision lost from converting double to int64_t
		int64_t tms = (int64_t)(t * (1E+9) + 0.5);
		if (t < 0) tms = -1;
		tms = myDetector->setSubFrameExposureTime(tms,imod);
		if (tms < 0)
			return -1;
		return  ((1E-9) * (double)tms);
	}
}

double slsDetectorUsers::setSubFrameExposureDeadTime(double t, bool inseconds, int imod){
	if(!inseconds)
		return myDetector->setSubFrameDeadTime((int64_t)t,imod);
	else {
		// + 0.5 to round for precision lost from converting double to int64_t
		int64_t tms = (int64_t)(t * (1E+9) + 0.5);
		if (t < 0) tms = -1;
		tms = myDetector->setSubFrameDeadTime(tms,imod);
		if (tms < 0)
			return -1;
		return  ((1E-9) * (double)tms);
	}
}

int64_t slsDetectorUsers::setNumberOfStorageCells(int64_t t, int imod) {
	return myDetector->setTimer(slsReceiverDefs::STORAGE_CELL_NUMBER, t, imod);
}

int slsDetectorUsers::setStoragecellStart(int pos) {
	return myDetector->setStoragecellStart(pos);
}

int slsDetectorUsers::setROI(int n, slsDetectorDefs::ROI roiLimits[], int imod) {
    return myDetector->setROI(n, roiLimits, imod);
}

slsDetectorDefs::ROI* slsDetectorUsers::getROI(int &n, int imod) {
    return myDetector->getROI(n, imod);
}
