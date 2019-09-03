#include "slsDetectorUsers.h"
#include "detectorData.h"
#include "multiSlsDetectorClient.h"



int slsDetectorUsers::size() const {
	return detector.size();
}

int slsDetectorUsers::getDetectorSize(int &nx, int &ny, int detPos){
	slsDetectorDefs::xy res = detector.getNumberOfChannels();
	nx=res.x;
	ny=res.y;
	return nx*ny;
}

std::string slsDetectorUsers::getDetectorType(int detPos){
	return detector.getDetectorTypeAsString(detPos);
}

int slsDetectorUsers::readConfigurationFile(const std::string& fname){
	try{
		detector.readConfigurationFile(fname);
		return slsDetectorDefs::OK;
	} catch (...) {
		return slsDetectorDefs::FAIL;
	}
}

void slsDetectorUsers::writeConfigurationFile(const std::string& fname){
	detector.writeConfigurationFile(fname);
}

int slsDetectorUsers::retrieveDetectorSetup(const std::string& fname){
	return detector.retrieveDetectorSetup(fname);
}

int slsDetectorUsers::dumpDetectorSetup(const std::string& fname){
	return detector.dumpDetectorSetup(fname);
}

int64_t slsDetectorUsers::getDetectorFirmwareVersion(int detPos){
	return detector.getId(slsDetectorDefs::DETECTOR_FIRMWARE_VERSION, detPos);
}

int64_t slsDetectorUsers::getDetectorSerialNumber(int detPos){
	return detector.getId(slsDetectorDefs::DETECTOR_SERIAL_NUMBER, detPos);
}

int64_t slsDetectorUsers::getDetectorSoftwareVersion(int detPos){
	return detector.getId(slsDetectorDefs::DETECTOR_SOFTWARE_VERSION, detPos);
}

int64_t slsDetectorUsers::getClientSoftwareVersion(){
	return detector.getClientSoftwareVersion();
}

int64_t slsDetectorUsers::getReceiverSoftwareVersion(int detPos){
	return detector.getId(slsDetectorDefs::RECEIVER_VERSION, detPos);
}

void slsDetectorUsers::isDetectorVersionCompatible(int detPos) {
	detector.checkDetectorVersionCompatibility(detPos);
}

void slsDetectorUsers::isReceiverVersionCompatible(int detPos) {
	detector.checkReceiverVersionCompatibility(detPos);
}

int slsDetectorUsers::startMeasurement(){
	return detector.acquire();
}

void slsDetectorUsers::stopMeasurement(int detPos){
	detector.stopAcquisition(detPos);
}

int slsDetectorUsers::getDetectorStatus(int detPos){
	return (int)detector.getRunStatus(detPos);
}

void slsDetectorUsers::startAcquisition(int detPos) {
	detector.startAcquisition(detPos);
}

void slsDetectorUsers::stopAcquisition(int detPos) {
	detector.stopAcquisition(detPos);
}

void slsDetectorUsers::sendSoftwareTrigger(int detPos) {
	detector.sendSoftwareTrigger(detPos);
}

int slsDetectorUsers::enableCountRateCorrection(int i, int detPos){
	if (i == 0)
		detector.setRateCorrection(0, detPos);
	else
		detector.setRateCorrection(-1, detPos);

	return detector.getRateCorrection(detPos);
}

int slsDetectorUsers::setBitDepth(int i, int detPos){
	return detector.setDynamicRange(i, detPos);
}

int slsDetectorUsers::setSettings(int isettings, int detPos){
	return detector.setSettings((slsDetectorDefs::detectorSettings)isettings, detPos);
}

int slsDetectorUsers::getThresholdEnergy(int detPos){
	return detector.getThresholdEnergy(detPos);
}  

int slsDetectorUsers::setThresholdEnergy(int e_ev, int tb, int isettings, int detPos) {
	return detector.setThresholdEnergy(e_ev,
			(isettings == -1) ?  slsDetectorDefs::GET_SETTINGS : (slsDetectorDefs::detectorSettings)isettings,
					tb, detPos);
}

double slsDetectorUsers::setExposureTime(double t, bool inseconds, int detPos){
	return detector.setExposureTime(t, inseconds, detPos);
}

double slsDetectorUsers::setExposurePeriod(double t, bool inseconds, int detPos){
	return detector.setExposurePeriod(t, inseconds, detPos);
}

double slsDetectorUsers::setDelayAfterTrigger(double t, bool inseconds, int detPos){
	return detector.setDelayAfterTrigger(t, inseconds, detPos);
}

double slsDetectorUsers::setSubFrameExposureTime(double t, bool inseconds, int detPos){
	return detector.setSubFrameExposureTime(t, inseconds, detPos);
}

double slsDetectorUsers::setSubFrameExposureDeadTime(double t, bool inseconds, int detPos){
	return detector.setSubFrameExposureDeadTime(t, inseconds, detPos);
}

int64_t slsDetectorUsers::setNumberOfFrames(int64_t t, int detPos){
	return detector.setNumberOfFrames(t, detPos);
}

int64_t slsDetectorUsers::setNumberOfCycles(int64_t t, int detPos){
	return detector.setNumberOfCycles(t, detPos);
}

int64_t slsDetectorUsers::setNumberOfStorageCells(int64_t t, int detPos) {
	return detector.setNumberOfStorageCells(t, detPos);
}

double slsDetectorUsers::getMeasuredPeriod(bool inseconds, int detPos) {
	return detector.getMeasuredPeriod(inseconds, detPos);
}

double slsDetectorUsers::getMeasuredSubFramePeriod(bool inseconds, int detPos) {
	return detector.getMeasuredSubFramePeriod(inseconds, detPos);
}

int slsDetectorUsers::setTimingMode(int pol, int detPos){
	return detector.setTimingMode(slsDetectorDefs::timingMode(pol), detPos);
}

int slsDetectorUsers::setClockDivider(int value, int detPos) {
	return detector.setSpeed(slsDetectorDefs::CLOCK_DIVIDER, value, detPos);
}

int slsDetectorUsers::setParallelMode(bool value, int detPos) {
	/* to be uncommented when moving to Detector.h
	detector.setParallelMode(value, {detPos});
	auto res = detector.getParallelMode({detPos});
	if (res.equal())
		return res.front();*/
	return -1;
}

int slsDetectorUsers::setOverflowMode(bool value, int detPos) {
	/* to be uncommented when moving to Detector.h
	detector.setOverFlowMode(value, {detPos});
	auto res = detector.getOverFlowMode({detPos});
	if (res.equal())
		return res.front();*/
	return -1;	
}

int slsDetectorUsers::setAllTrimbits(int val, int detPos) {
	return detector.setAllTrimbits(val, detPos);
}

int slsDetectorUsers::setDAC(int val, int index , int detPos) {
	return detector.setDAC(val, slsDetectorDefs::dacIndex(index), 0, detPos);
}

int slsDetectorUsers::getADC(int index, int detPos) {
	return detector.getADC(slsDetectorDefs::dacIndex(index),detPos);
}

int slsDetectorUsers::setTenGigabitEthernet(int i, int detPos) {
	return detector.enableTenGigabitEthernet(i, detPos);
}

int slsDetectorUsers::setStoragecellStart(int pos, int detPos) {
	return detector.setStoragecellStart(pos, detPos);
}

int slsDetectorUsers::setHighVoltage(int i, int detPos) {
	return detector.setDAC(i, slsDetectorDefs::HIGH_VOLTAGE, 0, detPos);
}

int slsDetectorUsers::setFlowControl10G(int i, int detPos) {
	return detector.setFlowControl10G(i, detPos);
}

void slsDetectorUsers::setROI(slsDetectorDefs::ROI arg, int detPos) {
    detector.setROI(arg, detPos);
}

slsDetectorDefs::ROI slsDetectorUsers::getROI(int detPos) {
    return detector.getROI(detPos);
}

/************************************************************************

                        RECEIVER FUNCTIONS

 *********************************************************************/


void slsDetectorUsers::startReceiver(int detPos) {
	detector.startReceiver(detPos);
}

void slsDetectorUsers::stopReceiver(int detPos) {
	detector.stopReceiver(detPos);
}

int slsDetectorUsers::setReceiverSilentMode(int i, int detPos) {
	return detector.setReceiverSilentMode(i, detPos);
}

void slsDetectorUsers::resetFramesCaughtInReceiver(int detPos) {
	detector.resetFramesCaught(detPos);
}

int slsDetectorUsers::setReceiverFifoDepth(int i, int detPos) {
	return detector.setReceiverFifoDepth(i, detPos);
}

std::string slsDetectorUsers::getFilePath(int detPos){
	return detector.getFilePath(detPos);
}

std::string slsDetectorUsers::setFilePath(const std::string& s, int detPos){
	return detector.setFilePath(s, detPos);
}

std::string slsDetectorUsers::getFileName(int detPos){
	return detector.getFileName(detPos);
}

std::string slsDetectorUsers::setFileName(const std::string& s, int detPos){
	return detector.setFileName(s, detPos);
}

int slsDetectorUsers::getFileIndex(int detPos){
	return detector.getFileIndex(detPos);
}

int slsDetectorUsers::setFileIndex(int i, int detPos){
	return detector.setFileIndex(i, detPos);
}

int slsDetectorUsers::enableWriteToFile(int enable, int detPos){
	if (enable >0)
		return detector.setFileWrite(enable, detPos);
	else
		return detector.getFileWrite(detPos);
	
}

int slsDetectorUsers::enableOverwriteFile(int enable, int detPos) {
    if (enable > 0)
        return detector.setFileOverWrite(enable, detPos);
    else
        return detector.getFileOverWrite(detPos);
}

int slsDetectorUsers::setReceiverStreamingFrequency(int freq, int detPos){
	return detector.setReceiverStreamingFrequency(freq, detPos);
}

int slsDetectorUsers::setReceiverStreamingTimer(int time_in_ms, int detPos){
	return detector.setReceiverStreamingTimer(time_in_ms, detPos);
}

int slsDetectorUsers::enableDataStreamingToClient(int i){
	return detector.enableDataStreamingToClient(i);
}

int slsDetectorUsers::enableDataStreamingFromReceiver(int i, int detPos){
	return detector.enableDataStreamingFromReceiver(i, detPos);
}

int slsDetectorUsers::setReceiverDataStreamingOutPort(int i, int detPos){
	  if (i >= 0) {
		  detector.setReceiverDataStreamingOutPort(i, detPos);
	  }
	return detector.getReceiverStreamingPort(detPos);
}

int slsDetectorUsers::setClientDataStreamingInPort(int i, int detPos){
	  if (i >= 0) {
		  detector.setClientDataStreamingInPort(i, detPos);
	  }
	return detector.getClientStreamingPort(detPos);
}

std::string slsDetectorUsers::setReceiverDataStreamingOutIP(const std::string& ip, int detPos){
	  if (ip.length()) {
		  detector.setReceiverDataStreamingOutIP(ip, detPos);
	  }
	return detector.getReceiverStreamingIP(detPos);
}

std::string slsDetectorUsers::setClientDataStreamingInIP(const std::string& ip, int detPos){
	  if (ip.length()) {
		  detector.setClientDataStreamingInIP(ip, detPos);
	  }
	return detector.getClientStreamingIP(detPos);
}

int slsDetectorUsers::enableGapPixels(int enable, int detPos) {
	return detector.enableGapPixels(enable, detPos);
}

int slsDetectorUsers::setReceiverFramesDiscardPolicy(int f, int detPos) {
	return detector.setReceiverFramesDiscardPolicy(slsDetectorDefs::frameDiscardPolicy(f), detPos);
}

int slsDetectorUsers::setReceiverPartialFramesPadding(int f, int detPos) {
	if (f>=0)
		return detector.setPartialFramesPadding(f, detPos);
	else
		return detector.getPartialFramesPadding(detPos);
}

int slsDetectorUsers::setReceiverFramesPerFile(int f, int detPos) {
    if (f > 0) {
        return detector.setFramesPerFile(f, detPos);
    }
    else {
        return detector.getFramesPerFile(detPos);
    }
}

int slsDetectorUsers::setDetectorMinMaxEnergyThreshold(const int index, int v, int detPos) {
    return detector.setDetectorMinMaxEnergyThreshold(index, v, detPos);
}

int slsDetectorUsers::setFrameMode(int value, int detPos) {
    return detector.setFrameMode(slsDetectorDefs::frameModeType(value), detPos);
}

int slsDetectorUsers::setDetectorMode(int value, int detPos) {
    return detector.setDetectorMode(slsDetectorDefs::detectorModeType(value), detPos);
}


/************************************************************************

                       CALLBACKS & COMMAND LINE PARSING

 *********************************************************************/

void slsDetectorUsers::registerDataCallback(void( *userCallback)(detectorData*, uint64_t, uint32_t, void*), void *pArg) {
	detector.registerDataCallback(userCallback,pArg);
}

void slsDetectorUsers::registerAcquisitionFinishedCallback(void( *func)(double,int, void*), void *pArg) {
	detector.registerAcquisitionFinishedCallback(func,pArg);
}

void slsDetectorUsers::putCommand(const std::string& command){
	multiSlsDetectorClient(command, slsDetectorDefs::PUT_ACTION, &detector);
}












