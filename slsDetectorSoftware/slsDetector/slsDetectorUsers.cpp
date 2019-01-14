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
	if (myCmd)
		delete myCmd;
}

int slsDetectorUsers::getNumberOfDetectors() {
	return myDetector->getNumberOfDetectors();
}

int slsDetectorUsers::getMaximumDetectorSize(int &nx, int &ny){
	nx=myDetector->getMaxNumberOfChannelsPerDetector(slsDetectorDefs::X);
	ny=myDetector->getMaxNumberOfChannelsPerDetector(slsDetectorDefs::Y);
	return nx*ny;
}

int slsDetectorUsers::getDetectorSize(int &x, int &y, int &nx, int &ny, int detPos){
	if (detPos < 0) {
		x = 0;
		y = 0;
	} else {
		x = myDetector->getDetectorOffset(slsDetectorDefs::X, detPos);
		y = myDetector->getDetectorOffset(slsDetectorDefs::Y, detPos);
	}
	nx=myDetector->getTotalNumberOfChannels(slsDetectorDefs::X, detPos);
	ny=myDetector->getTotalNumberOfChannels(slsDetectorDefs::Y, detPos);
	return nx*ny;
}

string slsDetectorUsers::getDetectorType(int detPos){
	return myDetector->sgetDetectorsType(detPos);
}
int slsDetectorUsers::setOnline(int const online, int detPos){
	return myDetector->setOnline(online, detPos);
}

int slsDetectorUsers::setReceiverOnline(int const online, int detPos){
	return myDetector->setReceiverOnline(online, detPos);
}

int slsDetectorUsers::readConfigurationFile(string const fname){
	return myDetector->readConfigurationFile(fname);
}

int slsDetectorUsers::writeConfigurationFile(string const fname){
	return myDetector->writeConfigurationFile(fname);
}

int slsDetectorUsers::retrieveDetectorSetup(string const fname){
	return myDetector->retrieveDetectorSetup(fname);
}

int slsDetectorUsers::dumpDetectorSetup(string const fname){
	return myDetector->dumpDetectorSetup(fname);
}

int64_t slsDetectorUsers::getDetectorFirmwareVersion(int detPos){
	return myDetector->getId(slsDetectorDefs::DETECTOR_FIRMWARE_VERSION);
}

int64_t slsDetectorUsers::getDetectorSerialNumber(int detPos){
	return myDetector->getId(slsDetectorDefs::DETECTOR_SERIAL_NUMBER, detPos);
}

int64_t slsDetectorUsers::getDetectorSoftwareVersion(int detPos){
	return myDetector->getId(slsDetectorDefs::DETECTOR_SOFTWARE_VERSION, detPos);
}

int64_t slsDetectorUsers::getClientSoftwareVersion(int detPos){
	return myDetector->getId(slsDetectorDefs::THIS_SOFTWARE_VERSION, detPos);
}

int64_t slsDetectorUsers::getReceiverSoftwareVersion(int detPos){
	return myDetector->getId(slsDetectorDefs::RECEIVER_VERSION, detPos);
}

bool slsDetectorUsers::isDetectorVersionCompatible(int detPos) {
	return (myDetector->checkVersionCompatibility(slsDetectorDefs::CONTROL_PORT, detPos) == slsDetectorDefs::OK);
}

bool slsDetectorUsers::isReceiverVersionCompatible(int detPos) {
	return (myDetector->checkVersionCompatibility(slsDetectorDefs::DATA_PORT, detPos) == slsDetectorDefs::OK);
}

int slsDetectorUsers::startMeasurement(){
	return myDetector->acquire();
}

int slsDetectorUsers::stopMeasurement(int detPos){
	return myDetector->stopAcquisition(detPos);
}

int slsDetectorUsers::getDetectorStatus(int detPos){
	return (int)myDetector->getRunStatus(detPos);
}

int slsDetectorUsers::startAcquisition(int detPos) {
	return myDetector->startAcquisition(detPos);
}

int slsDetectorUsers::stopAcquisition(int detPos) {
	return myDetector->stopAcquisition(detPos);
}

int slsDetectorUsers::sendSoftwareTrigger(int detPos) {
	return myDetector->sendSoftwareTrigger(detPos);
}

int slsDetectorUsers::enableCountRateCorrection(int i, int detPos){
	if (i == 0)
		myDetector->setRateCorrection(0, detPos);
	else
		myDetector->setRateCorrection(-1, detPos);

	return myDetector->getRateCorrection(detPos);
}

int slsDetectorUsers::setBitDepth(int i, int detPos){
	return myDetector->setDynamicRange(i, detPos);
}

int slsDetectorUsers::setSettings(int isettings, int detPos){
	return myDetector->setSettings((slsDetectorDefs::detectorSettings)isettings, detPos);
}

int slsDetectorUsers::getThresholdEnergy(int detPos){
	return myDetector->getThresholdEnergy(detPos);
}  

int slsDetectorUsers::setThresholdEnergy(int e_ev, int tb, int isettings, int detPos) {
	return myDetector->setThresholdEnergy(e_ev,
			(isettings == -1) ?  slsDetectorDefs::GET_SETTINGS : (slsDetectorDefs::detectorSettings)isettings,
					tb, detPos);
}

double slsDetectorUsers::setExposureTime(double t, bool inseconds, int detPos){
	return myDetector->setExposureTime(t, inseconds, detPos);
}

double slsDetectorUsers::setExposurePeriod(double t, bool inseconds, int detPos){
	return myDetector->setExposurePeriod(t, inseconds, detPos);
}

double slsDetectorUsers::setDelayAfterTrigger(double t, bool inseconds, int detPos){
	return myDetector->setDelayAfterTrigger(t, inseconds, detPos);
}

double slsDetectorUsers::setSubFrameExposureTime(double t, bool inseconds, int detPos){
	return myDetector->setSubFrameExposureTime(t, inseconds, detPos);
}

double slsDetectorUsers::setSubFrameExposureDeadTime(double t, bool inseconds, int detPos){
	return myDetector->setSubFrameExposureDeadTime(t, inseconds, detPos);
}

int64_t slsDetectorUsers::setNumberOfFrames(int64_t t, int detPos){
	return myDetector->setNumberOfFrames(t, detPos);
}

int64_t slsDetectorUsers::setNumberOfCycles(int64_t t, int detPos){
	return myDetector->setNumberOfCycles(t, detPos);
}

int64_t slsDetectorUsers::setNumberOfGates(int64_t t, int detPos){
	return myDetector->setNumberOfGates(t, detPos);
} 

int64_t slsDetectorUsers::setNumberOfStorageCells(int64_t t, int detPos) {
	return myDetector->setNumberOfStorageCells(t, detPos);
}

double slsDetectorUsers::getMeasuredPeriod(bool inseconds, int detPos) {
	return myDetector->getMeasuredPeriod(inseconds, detPos);
}

double slsDetectorUsers::getMeasuredSubFramePeriod(bool inseconds, int detPos) {
	return myDetector->getMeasuredSubFramePeriod(inseconds, detPos);
}

int slsDetectorUsers::setTimingMode(int pol, int detPos){
	return myDetector->setExternalCommunicationMode(slsDetectorDefs::externalCommunicationMode(pol), detPos);
}

int slsDetectorUsers::setClockDivider(int value, int detPos) {
	return myDetector->setSpeed(slsDetectorDefs::CLOCK_DIVIDER, value, detPos);
}

int slsDetectorUsers::setParallelMode(int value, int detPos) {
	if(value >= 0)
		myDetector->setReadOutFlags(slsDetectorDefs::readOutFlags(value), detPos);
	return myDetector->setReadOutFlags(slsDetectorDefs::GET_READOUT_FLAGS, detPos);
}

int slsDetectorUsers::setOverflowMode(int value, int detPos) {
	if(value >= 0) {
		if (value == 1)
			myDetector->setReadOutFlags(slsDetectorDefs::SHOW_OVERFLOW, detPos);
		else
			myDetector->setReadOutFlags(slsDetectorDefs::NOOVERFLOW, detPos);
	}
	int ret = myDetector->setReadOutFlags(slsDetectorDefs::GET_READOUT_FLAGS, detPos);
	if (ret == -1)
		return -1;
	return ((ret & slsDetectorDefs::SHOW_OVERFLOW) ? 1 : 0);
}

int slsDetectorUsers::setAllTrimbits(int val, int detPos) {
	return myDetector->setAllTrimbits(val, detPos);
}

int slsDetectorUsers::setDAC(int val, int index , int detPos) {
	return myDetector->setDAC(val, slsDetectorDefs::dacIndex(index), 0, detPos);
}

int slsDetectorUsers::getADC(int index, int detPos) {
	return myDetector->getADC(slsDetectorDefs::dacIndex(index),detPos);
}

int slsDetectorUsers::setTenGigabitEthernet(int i, int detPos) {
	return myDetector->enableTenGigabitEthernet(i, detPos);
}

int slsDetectorUsers::setStoragecellStart(int pos, int detPos) {
	return myDetector->setStoragecellStart(pos, detPos);
}

int slsDetectorUsers::setHighVoltage(int i, int detPos) {
	return myDetector->setDAC(i, slsDetectorDefs::HIGH_VOLTAGE, 0, detPos);
}

int slsDetectorUsers::setFlowControl10G(int i, int detPos) {
	return myDetector->setFlowControl10G(i, detPos);
}


/************************************************************************

                        RECEIVER FUNCTIONS

 *********************************************************************/


int slsDetectorUsers::startReceiver(int detPos) {
	return myDetector->startReceiver(detPos);
}

int slsDetectorUsers::stopReceiver(int detPos) {
	return myDetector->stopReceiver(detPos);
}

int slsDetectorUsers::setReceiverSilentMode(int i, int detPos) {
	return myDetector->setReceiverSilentMode(i, detPos);
}

int slsDetectorUsers::resetFramesCaughtInReceiver(int detPos) {
	return myDetector->resetFramesCaught(detPos);
}

int slsDetectorUsers::setReceiverFifoDepth(int i, int detPos) {
	return myDetector->setReceiverFifoDepth(i, detPos);
}

string slsDetectorUsers::getFilePath(int detPos){
	return myDetector->getFilePath(detPos);
}

string slsDetectorUsers::setFilePath(string s, int detPos){
	return myDetector->setFilePath(s, detPos);
}

string slsDetectorUsers::getFileName(int detPos){
	return myDetector->getFileName(detPos);
}

string slsDetectorUsers::setFileName(string s, int detPos){
	return myDetector->setFileName(s, detPos);
}

int slsDetectorUsers::getFileIndex(int detPos){
	return (int)myDetector->getFileIndex(detPos);
}

int slsDetectorUsers::setFileIndex(int i, int detPos){
	return (int)myDetector->setFileIndex(i, detPos);
}

int slsDetectorUsers::enableWriteToFile(int enable, int detPos){
	return myDetector->enableWriteToFile(enable, detPos);
}

int slsDetectorUsers::enableOverwriteFile(int enable, int detPos){
	return myDetector->overwriteFile(enable, detPos);
}

int slsDetectorUsers::setReceiverStreamingFrequency(int freq, int detPos){
	return myDetector->setReceiverStreamingFrequency(freq, detPos);
}

int slsDetectorUsers::setReceiverStreamingTimer(int time_in_ms, int detPos){
	return myDetector->setReceiverStreamingTimer(time_in_ms, detPos);
}

int slsDetectorUsers::enableDataStreamingToClient(int i){
	return myDetector->enableDataStreamingToClient(i);
}

int slsDetectorUsers::enableDataStreamingFromReceiver(int i, int detPos){
	return myDetector->enableDataStreamingFromReceiver(i, detPos);
}

int slsDetectorUsers::setReceiverDataStreamingOutPort(int i, int detPos){
	  if (i >= 0) {
		  myDetector->setReceiverDataStreamingOutPort(i, detPos);
	  }
	return myDetector->getReceiverStreamingPort(detPos);
}

int slsDetectorUsers::setClientDataStreamingInPort(int i, int detPos){
	  if (i >= 0) {
		  myDetector->setClientDataStreamingInPort(i, detPos);
	  }
	return myDetector->getClientStreamingPort(detPos);
}

string slsDetectorUsers::setReceiverDataStreamingOutIP(string ip, int detPos){
	  if (ip.length()) {
		  myDetector->setReceiverDataStreamingOutIP(ip, detPos);
	  }
	return myDetector->getReceiverStreamingIP(detPos);
}

string slsDetectorUsers::setClientDataStreamingInIP(string ip, int detPos){
	  if (ip.length()) {
		  myDetector->setClientDataStreamingInIP(ip, detPos);
	  }
	return myDetector->getClientStreamingIP(detPos);
}

int slsDetectorUsers::enableGapPixels(int enable, int detPos) {
	return myDetector->enableGapPixels(enable, detPos);
}

int slsDetectorUsers::setReceiverFramesDiscardPolicy(int f, int detPos) {
	return myDetector->setReceiverFramesDiscardPolicy(slsDetectorDefs::frameDiscardPolicy(f), detPos);
}

int slsDetectorUsers::setReceiverPartialFramesPadding(int f, int detPos) {
	return myDetector->setReceiverPartialFramesPadding(f, detPos);
}

int slsDetectorUsers::setReceiverFramesPerFile(int f, int detPos) {
	return myDetector->setReceiverFramesPerFile(f, detPos);
}



/************************************************************************

                       CALLBACKS & COMMAND LINE PARSING

 *********************************************************************/

void slsDetectorUsers::registerDataCallback(int( *userCallback)(detectorData*, int, int, void*), void *pArg) {
	myDetector->registerDataCallback(userCallback,pArg);
}

void slsDetectorUsers::registerAcquisitionFinishedCallback(int( *func)(double,int, void*), void *pArg) {
	myDetector->registerAcquisitionFinishedCallback(func,pArg);
}

void slsDetectorUsers::registerMeasurementFinishedCallback(int( *func)(int,int, void*), void *pArg) {
	myDetector->registerMeasurementFinishedCallback(func,pArg);
}

void slsDetectorUsers::registerProgressCallback(int( *func)(double,void*), void *pArg) {
	myDetector->registerProgressCallback(func,pArg);
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











