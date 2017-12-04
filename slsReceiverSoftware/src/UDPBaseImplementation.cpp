//#ifdef SLS_RECEIVER_UDP_FUNCTIONS
/********************************************//**
 * @file UDPBaseImplementation.cpp
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 ***********************************************/

#include "UDPBaseImplementation.h"
#include "genericSocket.h"

#include <sys/stat.h> 		// stat
#include <iostream>
#include <string.h>
using namespace std;



/*************************************************************************
 * Constructor & Destructor **********************************************
 * They access local cache of configuration or detector parameters *******
 *************************************************************************/
UDPBaseImplementation::UDPBaseImplementation(){

	initializeMembers();

	//***callback parameters***
	startAcquisitionCallBack = NULL;
	pStartAcquisition = NULL;
	acquisitionFinishedCallBack = NULL;
	pAcquisitionFinished = NULL;
	rawDataReadyCallBack = NULL;
	pRawDataReady = NULL;
}

void UDPBaseImplementation::initializeMembers(){

	//**detector parameters***
	myDetectorType = GENERIC;
	memset(detHostname,0,MAX_STR_LENGTH);
	packetsPerFrame = 0;
	acquisitionPeriod = 0;
	acquisitionTime = 0;
	numberOfFrames = 0;
	dynamicRange = 16;
	tengigaEnable = false;
	fifoDepth = 0;
	flippedData[0] = 0;
	flippedData[1] = 0;

	//***receiver parameters***
	status = IDLE;
	activated = true;

	//***connection parameters***
	memset(eth,0,MAX_STR_LENGTH);
	for(int i=0;i<MAX_NUMBER_OF_LISTENING_THREADS;i++){
		udpPortNum[i] = DEFAULT_UDP_PORTNO + i;
	}

	//***file parameters***
	memset(fileName,0,MAX_STR_LENGTH);
	snprintf(fileName,MAX_STR_LENGTH,"run");
	memset(filePath,0,MAX_STR_LENGTH);
	fileIndex = 0;
	scanTag = 0;
	frameIndexEnable = false;
	fileWriteEnable = true;
	overwriteEnable = true;
	dataCompressionEnable = false;

	//***acquisition count parameters***
	totalPacketsCaught = 0;
	packetsCaught = 0;

	//***acquisition indices parameters***
	acquisitionIndex = 0;

	//***acquisition parameters***
	shortFrameEnable = -1;
	frameToGuiFrequency = 0;
	frameToGuiTimerinMS = DEFAULT_STREAMING_TIMER;
	dataStreamEnable = false;
}

UDPBaseImplementation::~UDPBaseImplementation(){}


/*************************************************************************
 * Getters ***************************************************************
 * They access local cache of configuration or detector parameters *******
 *************************************************************************/

/**initial parameters***/
char *UDPBaseImplementation::getDetectorHostname() const{

	//not initialized
	if(!strlen(detHostname))
		return NULL;

	char* output = new char[MAX_STR_LENGTH]();
	memset(output,0,MAX_STR_LENGTH);
	snprintf(output,MAX_STR_LENGTH,detHostname);
	//freed by calling function
	return output;
}

int UDPBaseImplementation::getFlippedData(int axis) const{
	if(axis<0 || axis > 1) return -1;
	return flippedData[axis];
}


/***file parameters***/
char *UDPBaseImplementation::getFileName() const{

	//not initialized
	if(!strlen(fileName))
		return NULL;

	char* output = new char[MAX_STR_LENGTH]();
	memset(output,0,MAX_STR_LENGTH);
	snprintf(output,MAX_STR_LENGTH,fileName);
	//freed by calling function
	return output;
}

char *UDPBaseImplementation::getFilePath() const{

	//not initialized
	if(!strlen(filePath))
		return NULL;

	char* output = new char[MAX_STR_LENGTH]();
	memset(output,0,MAX_STR_LENGTH);
	snprintf(output,MAX_STR_LENGTH,filePath);
	//freed by calling function
	return output;
}

uint64_t UDPBaseImplementation::getFileIndex() const{	return fileIndex;}

int UDPBaseImplementation::getScanTag() const{	return scanTag;}

bool UDPBaseImplementation::getFrameIndexEnable() const{	return frameIndexEnable;}

bool UDPBaseImplementation::getFileWriteEnable() const{	return fileWriteEnable;}

bool UDPBaseImplementation::getOverwriteEnable() const{	return overwriteEnable;}

bool UDPBaseImplementation::getDataCompressionEnable() const{	return dataCompressionEnable;}

/***acquisition count parameters***/
uint64_t UDPBaseImplementation::getTotalFramesCaught() const{	return (totalPacketsCaught/packetsPerFrame);}

uint64_t UDPBaseImplementation::getFramesCaught() const{	return (packetsCaught/packetsPerFrame);}

int64_t UDPBaseImplementation::getAcquisitionIndex() const{

	if(!totalPacketsCaught)
		return -1;
	return acquisitionIndex;
}


/***connection parameters***/
uint32_t UDPBaseImplementation::getUDPPortNumber() const{	return udpPortNum[0];}

uint32_t UDPBaseImplementation::getUDPPortNumber2() const{	return udpPortNum[1];}

char *UDPBaseImplementation::getEthernetInterface() const{


	char* output = new char[MAX_STR_LENGTH]();
	memset(output,0,MAX_STR_LENGTH);
	snprintf(output,MAX_STR_LENGTH,eth);
	//freed by calling function
	return output;
}


/***acquisition parameters***/
int UDPBaseImplementation::getShortFrameEnable() const{	return shortFrameEnable;}

uint32_t UDPBaseImplementation::getFrameToGuiFrequency() const{	return frameToGuiFrequency;}

uint32_t UDPBaseImplementation::getFrameToGuiTimer() const{	return frameToGuiTimerinMS;}

uint32_t UDPBaseImplementation::getDataStreamEnable() const{	return dataStreamEnable;}

uint64_t UDPBaseImplementation::getAcquisitionPeriod() const{	return acquisitionPeriod;}

uint64_t UDPBaseImplementation::getAcquisitionTime() const{	return acquisitionTime;}

uint64_t UDPBaseImplementation::getNumberOfFrames() const{	return numberOfFrames;}

uint32_t UDPBaseImplementation::getDynamicRange() const{	return dynamicRange;}

bool UDPBaseImplementation::getTenGigaEnable() const{	return tengigaEnable;}

uint32_t UDPBaseImplementation::getFifoDepth() const{	return fifoDepth;}

/***receiver status***/
slsReceiverDefs::runStatus UDPBaseImplementation::getStatus() const{	return status;}

int UDPBaseImplementation::getActivate() const{ return activated;}


/*************************************************************************
 * Setters ***************************************************************
 * They modify the local cache of configuration or detector parameters ***
 *************************************************************************/

/**initial parameters***/
void UDPBaseImplementation::configure(map<string, string> config_map){
	FILE_LOG(logERROR, " must be overridden by child classes");
}

void UDPBaseImplementation::setFlippedData(int axis, int enable){

	if(axis<0 || axis>1) return;
	flippedData[axis] = enable==0?0:1;

	char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
	snprintf(cstreambuf, MAX_STR_LENGTH, "Flipped Data: %d , %d ", flippedData[0], flippedData[1]);
	FILE_LOG(logINFO, cstreambuf);
}


/***file parameters***/
void UDPBaseImplementation::setFileName(const char c[]){

  if(strlen(c)){
    memset(fileName,0,MAX_STR_LENGTH);
    snprintf(fileName,MAX_STR_LENGTH,c);
  }

	char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
	snprintf(cstreambuf, MAX_STR_LENGTH, "File name: %s ", fileName);
	FILE_LOG(logINFO, cstreambuf);
}

void UDPBaseImplementation::setFilePath(const char c[]){

	if(strlen(c)){
		//check if filepath exists
		struct stat st;
		if(stat(c,&st) == 0){
		  memset(filePath,0,MAX_STR_LENGTH);
		  snprintf(filePath,MAX_STR_LENGTH,c);
		}else{
		        memset(filePath,0,MAX_STR_LENGTH);

			char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
			snprintf(cstreambuf, MAX_STR_LENGTH, "FilePath does not exist: %s ", filePath);
			FILE_LOG(logWARNING, cstreambuf);
		}
	}
	/*{
	 	 char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
	 	 snprintf(cstreambuf, MAX_STR_LENGTH, "File path:: %s ", filePath);
	 	 FILE_LOG(logDEBUG, cstreambuf);
	  }*/
}

void UDPBaseImplementation::setFileIndex(const uint64_t i){

	fileIndex = i;
	char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
	snprintf(cstreambuf, MAX_STR_LENGTH, "File Index: %lu ", fileIndex);
	FILE_LOG(logINFO, cstreambuf);
}

//FIXME: needed?
void UDPBaseImplementation::setScanTag(const int i){

	scanTag = i;
	char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
	snprintf(cstreambuf, MAX_STR_LENGTH, "Scan Tag: %d ", scanTag);
	FILE_LOG(logINFO, cstreambuf);
}

void UDPBaseImplementation::setFrameIndexEnable(const bool b){

	frameIndexEnable = b;
	char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
	snprintf(cstreambuf, MAX_STR_LENGTH, "Frame Index Enable: %s ", stringEnable(frameIndexEnable).c_str());
	FILE_LOG(logINFO, cstreambuf);
}

void UDPBaseImplementation::setFileWriteEnable(const bool b){

	fileWriteEnable = b;
	char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
	snprintf(cstreambuf, MAX_STR_LENGTH, "File Write Enable: %s ", stringEnable(fileWriteEnable).c_str());
	FILE_LOG(logINFO, cstreambuf);
}

void UDPBaseImplementation::setOverwriteEnable(const bool b){

	overwriteEnable = b;
	char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
	snprintf(cstreambuf, MAX_STR_LENGTH, "Overwrite Enable: %s ", stringEnable(overwriteEnable).c_str());
	FILE_LOG(logINFO, cstreambuf);
}

int UDPBaseImplementation::setDataCompressionEnable(const bool b){

	dataCompressionEnable = b;
	char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
	snprintf(cstreambuf, MAX_STR_LENGTH, "Data Compression: %s ", stringEnable(dataCompressionEnable).c_str());
	FILE_LOG(logINFO, cstreambuf);

	//overridden methods might return FAIL
	return OK;
}


/***connection parameters***/
void UDPBaseImplementation::setUDPPortNumber(const uint32_t i){

	udpPortNum[0] = i;
	char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
	snprintf(cstreambuf, MAX_STR_LENGTH, "UDP Port Number[0]: %u ", udpPortNum[0]);
	FILE_LOG(logINFO, cstreambuf);
}

void UDPBaseImplementation::setUDPPortNumber2(const uint32_t i){

	udpPortNum[1] = i;
	char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
	snprintf(cstreambuf, MAX_STR_LENGTH, "UDP Port Number[1]: %u ", udpPortNum[1]);
	FILE_LOG(logINFO, cstreambuf);
}

void UDPBaseImplementation::setEthernetInterface(const char* c){
  memset(eth,0, MAX_STR_LENGTH);
  snprintf(eth,MAX_STR_LENGTH, c);
	char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
	snprintf(cstreambuf, MAX_STR_LENGTH, "Ethernet Interface: %s ", eth);
	FILE_LOG(logINFO, cstreambuf);
}


/***acquisition parameters***/
void UDPBaseImplementation::setShortFrameEnable(const int i){

	shortFrameEnable = i;
	char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
	snprintf(cstreambuf, MAX_STR_LENGTH, "Short Frame Enable: %d ", shortFrameEnable);
	FILE_LOG(logINFO, cstreambuf);
}

int UDPBaseImplementation::setFrameToGuiFrequency(const uint32_t freq){

	frameToGuiFrequency = freq;
	char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
	snprintf(cstreambuf, MAX_STR_LENGTH, "Frame To Gui Frequency: %u ", frameToGuiFrequency);
	FILE_LOG(logINFO, cstreambuf);

	//overrridden child classes might return FAIL
	return OK;
}

void UDPBaseImplementation::setFrameToGuiTimer(const uint32_t time_in_ms){

	frameToGuiTimerinMS = time_in_ms;
	char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
	snprintf(cstreambuf, MAX_STR_LENGTH, "Frame To Gui Timer: %u ", frameToGuiTimerinMS);
	FILE_LOG(logINFO, cstreambuf);
}


uint32_t UDPBaseImplementation::setDataStreamEnable(const uint32_t enable){

	dataStreamEnable = enable;
	char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
	snprintf(cstreambuf, MAX_STR_LENGTH, "Streaming Data from Receiver: %d ", dataStreamEnable);
	FILE_LOG(logINFO, cstreambuf);

	//overrridden child classes might return FAIL
	return OK;
}


int UDPBaseImplementation::setAcquisitionPeriod(const uint64_t i){

	acquisitionPeriod = i;
	char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
	snprintf(cstreambuf, MAX_STR_LENGTH, "Acquisition Period: %f s ", (double)acquisitionPeriod/(1E9));
	FILE_LOG(logINFO, cstreambuf);

	//overrridden child classes might return FAIL
	return OK;
}

int UDPBaseImplementation::setAcquisitionTime(const uint64_t i){

	acquisitionTime = i;
	char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
	snprintf(cstreambuf, MAX_STR_LENGTH, "Acquisition Time: %f s ", (double)acquisitionTime/(1E9));
	FILE_LOG(logINFO, cstreambuf);

	//overrridden child classes might return FAIL
	return OK;
}

int UDPBaseImplementation::setNumberOfFrames(const uint64_t i){

	numberOfFrames = i;
	char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
	snprintf(cstreambuf, MAX_STR_LENGTH, "Number of Frames: %lu ", numberOfFrames);
	FILE_LOG(logINFO, cstreambuf);

	//overrridden child classes might return FAIL
	return OK;
}

int UDPBaseImplementation::setDynamicRange(const uint32_t i){

	dynamicRange = i;
	char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
	snprintf(cstreambuf, MAX_STR_LENGTH, "Dynamic Range: %u ", dynamicRange);
	FILE_LOG(logINFO, cstreambuf);

	//overrridden child classes might return FAIL
	return OK;
}

int UDPBaseImplementation::setTenGigaEnable(const bool b){

	tengigaEnable = b;
	char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
	snprintf(cstreambuf, MAX_STR_LENGTH, "Ten Giga Enable: %s ", stringEnable(tengigaEnable).c_str());
	FILE_LOG(logINFO, cstreambuf);

	//overridden functions might return FAIL
	return OK;
}

int UDPBaseImplementation::setFifoDepth(const uint32_t i){

	fifoDepth = i;
	char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
	snprintf(cstreambuf, MAX_STR_LENGTH, "Fifo Depth: %u ", i);
	FILE_LOG(logINFO, cstreambuf);

	//overridden functions might return FAIL
	return OK;
}

/*************************************************************************
 * Behavioral functions***************************************************
 * They may modify the status of the receiver ****************************
 *************************************************************************/


/***initial functions***/
int UDPBaseImplementation::setDetectorType(const detectorType d){

	myDetectorType = d;
	//if eiger, set numberofListeningThreads = 2;
	char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
	snprintf(cstreambuf, MAX_STR_LENGTH, "Detector Type: %s ", getDetectorType(d).c_str());
	FILE_LOG(logINFO, cstreambuf);

	return OK;
}

void UDPBaseImplementation::initialize(const char *c){

  if(strlen(c)){
    memset(detHostname,0,MAX_STR_LENGTH);
    snprintf(detHostname, MAX_STR_LENGTH, c);
  }
	char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
	snprintf(cstreambuf, MAX_STR_LENGTH, "Detector Hostname: %s ", detHostname);
	FILE_LOG(logINFO, cstreambuf);

}


/***acquisition functions***/
void UDPBaseImplementation::resetAcquisitionCount(){


	totalPacketsCaught = 0;
	char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
	snprintf(cstreambuf, MAX_STR_LENGTH, "Total Packets Caught: %lu ", totalPacketsCaught);
	FILE_LOG(logINFO, cstreambuf);
}

int UDPBaseImplementation::startReceiver(char *c){

	FILE_LOG(logERROR, " must be overridden by child classes");

	return OK;
}

void UDPBaseImplementation::stopReceiver(){

	FILE_LOG(logERROR, " must be overridden by child classes");
}

void UDPBaseImplementation::startReadout(){

	FILE_LOG(logERROR, " must be overridden by child classes");
}

int UDPBaseImplementation::shutDownUDPSockets(){

	FILE_LOG(logERROR, " must be overridden by child classes");

	//overridden functions might return FAIL
	return OK;
}

void UDPBaseImplementation::readFrame(int ithread, char* c,char** raw, int64_t &startAcquisitionIndex, int64_t &startFrameIndex){

	FILE_LOG(logERROR, " must be overridden by child classes");
}


//FIXME: needed, isnt stopReceiver enough?
void UDPBaseImplementation::abort(){

	FILE_LOG(logERROR, " must be overridden by child classes");
}

void UDPBaseImplementation::closeFile(int ithread){

	FILE_LOG(logERROR, " must be overridden by child classes");
}


int UDPBaseImplementation::setActivate(int enable){

	if(enable != -1){
		activated = enable;

		char cstreambuf[MAX_STR_LENGTH]; memset(cstreambuf, 0, MAX_STR_LENGTH);
		snprintf(cstreambuf, MAX_STR_LENGTH, "Activation: %s ", stringEnable(activated).c_str());
		FILE_LOG(logINFO, cstreambuf);
	}

	return activated;
}

/***callback functions***/
void UDPBaseImplementation::registerCallBackStartAcquisition(int (*func)(char*, char*, uint64_t, uint32_t, void*),void *arg){
	startAcquisitionCallBack=func;
	pStartAcquisition=arg;
}

void UDPBaseImplementation::registerCallBackAcquisitionFinished(void (*func)(uint64_t, void*),void *arg){
	acquisitionFinishedCallBack=func;
	pAcquisitionFinished=arg;
}

void UDPBaseImplementation::registerCallBackRawDataReady(void (*func)(uint64_t, uint32_t, uint32_t, uint64_t, uint64_t, uint16_t, uint16_t, uint16_t, uint16_t, uint32_t, uint16_t, uint8_t, uint8_t,
		char*, uint32_t, void*),void *arg){
	rawDataReadyCallBack=func;
	pRawDataReady=arg;
}


//#endif
