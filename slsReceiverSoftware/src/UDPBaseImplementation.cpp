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
	FILE_LOG(logDEBUG) << __AT__ << " starting";

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
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	FILE_LOG(logDEBUG) << "Info: Initializing base members";
	//**detector parameters***
	myDetectorType = GENERIC;
	strcpy(detHostname,"");
	packetsPerFrame = 0;
	acquisitionPeriod = 0;
	numberOfFrames = 0;
	dynamicRange = 16;
	tengigaEnable = false;
	fifoDepth = 0;
	bottomEnable = false;

	//***receiver parameters***
	status = IDLE;

	//***connection parameters***
	strcpy(eth,"");
	for(int i=0;i<MAX_NUMBER_OF_LISTENING_THREADS;i++){
		udpPortNum[i] = DEFAULT_UDP_PORTNO + i;
	}

	//***file parameters***
	strcpy(fileName,"run");
	strcpy(filePath,"");
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
	FrameToGuiFrequency = 0;
}

UDPBaseImplementation::~UDPBaseImplementation(){}


/*************************************************************************
 * Getters ***************************************************************
 * They access local cache of configuration or detector parameters *******
 *************************************************************************/

/**initial parameters***/
char *UDPBaseImplementation::getDetectorHostname() const{
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	//not initialized
	if(!strlen(detHostname))
		return NULL;

	char* output = new char[MAX_STR_LENGTH];
	strcpy(output,detHostname);
	//freed by calling function
	return output;
}


/***file parameters***/
char *UDPBaseImplementation::getFileName() const{
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	//not initialized
	if(!strlen(fileName))
		return NULL;

	char* output = new char[MAX_STR_LENGTH];
	strcpy(output,fileName);
	//freed by calling function
	return output;
}

char *UDPBaseImplementation::getFilePath() const{
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	//not initialized
	if(!strlen(filePath))
		return NULL;

	char* output = new char[MAX_STR_LENGTH];
	strcpy(output,filePath);
	//freed by calling function
	return output;
}

uint64_t UDPBaseImplementation::getFileIndex() const{	FILE_LOG(logDEBUG) << __AT__ << " starting";	return fileIndex;}

int UDPBaseImplementation::getScanTag() const{	FILE_LOG(logDEBUG) << __AT__ << " starting";	return scanTag;}

bool UDPBaseImplementation::getFrameIndexEnable() const{	FILE_LOG(logDEBUG) << __AT__ << " starting";	return frameIndexEnable;}

bool UDPBaseImplementation::getFileWriteEnable() const{	FILE_LOG(logDEBUG) << __AT__ << " starting";	return fileWriteEnable;}

bool UDPBaseImplementation::getOverwriteEnable() const{	FILE_LOG(logDEBUG) << __AT__ << " starting";	return overwriteEnable;}

bool UDPBaseImplementation::getDataCompressionEnable() const{	FILE_LOG(logDEBUG) << __AT__ << " starting";	return dataCompressionEnable;}

/***acquisition count parameters***/
uint64_t UDPBaseImplementation::getTotalFramesCaught() const{	FILE_LOG(logDEBUG) << __AT__ << " starting";	return (totalPacketsCaught/packetsPerFrame);}

uint64_t UDPBaseImplementation::getFramesCaught() const{	FILE_LOG(logDEBUG) << __AT__ << " starting";	return (packetsCaught/packetsPerFrame);}

int64_t UDPBaseImplementation::getAcquisitionIndex() const{
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	if(!totalPacketsCaught)
		return -1;
	return acquisitionIndex;
}


/***connection parameters***/
uint32_t UDPBaseImplementation::getUDPPortNumber() const{	FILE_LOG(logDEBUG) << __AT__ << " starting";	return udpPortNum[0];}

uint32_t UDPBaseImplementation::getUDPPortNumber2() const{	FILE_LOG(logDEBUG) << __AT__ << " starting";	return udpPortNum[1];}

char *UDPBaseImplementation::getEthernetInterface() const{
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	char* output = new char[MAX_STR_LENGTH];
	strcpy(output,eth);
	//freed by calling function
	return output;
}


/***acquisition parameters***/
int UDPBaseImplementation::getShortFrameEnable() const{	FILE_LOG(logDEBUG) << __AT__ << " starting";	return shortFrameEnable;}

uint32_t UDPBaseImplementation::getFrameToGuiFrequency() const{	FILE_LOG(logDEBUG) << __AT__ << " starting";	return FrameToGuiFrequency;}

uint64_t UDPBaseImplementation::getAcquisitionPeriod() const{	FILE_LOG(logDEBUG) << __AT__ << " starting";	return acquisitionPeriod;}

uint64_t UDPBaseImplementation::getNumberOfFrames() const{	FILE_LOG(logDEBUG) << __AT__ << " starting";	return numberOfFrames;}

uint32_t UDPBaseImplementation::getDynamicRange() const{	FILE_LOG(logDEBUG) << __AT__ << " starting";	return dynamicRange;}

bool UDPBaseImplementation::getTenGigaEnable() const{	FILE_LOG(logDEBUG) << __AT__ << " starting";	return tengigaEnable;}

uint32_t UDPBaseImplementation::getFifoDepth() const{	FILE_LOG(logDEBUG) << __AT__ << " starting";	return fifoDepth;}

/***receiver status***/
slsReceiverDefs::runStatus UDPBaseImplementation::getStatus() const{	FILE_LOG(logDEBUG) << __AT__ << " starting";	return status;}




/*************************************************************************
 * Setters ***************************************************************
 * They modify the local cache of configuration or detector parameters ***
 *************************************************************************/

/**initial parameters***/
void UDPBaseImplementation::configure(map<string, string> config_map){
	FILE_LOG(logWARNING) << __AT__ << " doing nothing...";
	FILE_LOG(logERROR) << __AT__ << " must be overridden by child classes";
}

void UDPBaseImplementation::setBottomEnable(const bool b){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	bottomEnable = b;
	FILE_LOG(logINFO)  << "Bottom - " << stringEnable(bottomEnable);
}


/***file parameters***/
void UDPBaseImplementation::setFileName(const char c[]){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	if(strlen(c))
		strcpy(fileName, c);
	FILE_LOG(logINFO) << "File name:" << fileName;
}

void UDPBaseImplementation::setFilePath(const char c[]){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	if(strlen(c)){
		//check if filepath exists
		struct stat st;
		if(stat(c,&st) == 0)
			strcpy(filePath,c);
		else{
			strcpy(filePath,"");
			FILE_LOG(logWARNING) << "FilePath does not exist:" << filePath;
		}
		strcpy(filePath, c);
	}
	FILE_LOG(logDEBUG) << "Info: File path:" << filePath;
}

void UDPBaseImplementation::setFileIndex(const uint64_t i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	fileIndex = i;
	FILE_LOG(logINFO) << "File Index:" << fileIndex;
}

//FIXME: needed?
void UDPBaseImplementation::setScanTag(const int i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	scanTag = i;
	FILE_LOG(logINFO) << "Scan Tag:" << scanTag;

}

void UDPBaseImplementation::setFrameIndexEnable(const bool b){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	frameIndexEnable = b;
	FILE_LOG(logINFO) << "Frame Index Enable: " << stringEnable(frameIndexEnable);
}

void UDPBaseImplementation::setFileWriteEnable(const bool b){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	fileWriteEnable = b;
	FILE_LOG(logINFO) << "File Write Enable: " << stringEnable(fileWriteEnable);
}

void UDPBaseImplementation::setOverwriteEnable(const bool b){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	overwriteEnable = b;
	FILE_LOG(logINFO) << "Overwrite Enable: " << stringEnable(overwriteEnable);
}

int UDPBaseImplementation::setDataCompressionEnable(const bool b){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	dataCompressionEnable = b;
	FILE_LOG(logINFO) << "Data Compression : " << stringEnable(dataCompressionEnable);

	//overridden methods might return FAIL
	return OK;
}


/***connection parameters***/
void UDPBaseImplementation::setUDPPortNumber(const uint32_t i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	udpPortNum[0] = i;
	FILE_LOG(logINFO) << "UDP Port Number[0]:" << udpPortNum[0];
}

void UDPBaseImplementation::setUDPPortNumber2(const uint32_t i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	udpPortNum[1] = i;
	FILE_LOG(logINFO) << "UDP Port Number[1]:" << udpPortNum[1];
}

void UDPBaseImplementation::setEthernetInterface(const char* c){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	strcpy(eth, c);
	FILE_LOG(logINFO) << "Ethernet Interface: " << eth;
}


/***acquisition parameters***/
void UDPBaseImplementation::setShortFrameEnable(const int i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	shortFrameEnable = i;
	FILE_LOG(logINFO) << "Short Frame Enable: " << stringEnable(shortFrameEnable);
}

int UDPBaseImplementation::setFrameToGuiFrequency(const uint32_t i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	FrameToGuiFrequency = i;
	FILE_LOG(logINFO) << "Frame To Gui Frequency:" << FrameToGuiFrequency;

	//overrridden child classes might return FAIL
	return OK;
}

int UDPBaseImplementation::setAcquisitionPeriod(const uint64_t i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	acquisitionPeriod = i;
	FILE_LOG(logINFO) << "Acquisition Period:" <<  (double)acquisitionPeriod/(1E9) << "s";

	//overrridden child classes might return FAIL
	return OK;
}

void UDPBaseImplementation::setNumberOfFrames(const uint64_t i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	numberOfFrames = i;
	FILE_LOG(logINFO) << "Number of Frames:" << numberOfFrames;
}

int UDPBaseImplementation::setDynamicRange(const uint32_t i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	dynamicRange = i;
	FILE_LOG(logINFO) << "Dynamic Range:" << dynamicRange;

	//overrridden child classes might return FAIL
	return OK;
}

int UDPBaseImplementation::setTenGigaEnable(const bool b){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	tengigaEnable = b;
	FILE_LOG(logINFO) << "Ten Giga Enable: " << stringEnable(tengigaEnable);

	//overridden functions might return FAIL
	return OK;
}

int UDPBaseImplementation::setFifoDepth(const uint32_t i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	fifoDepth = i;
	FILE_LOG(logINFO) << "Fifo Depth: " << i;

	//overridden functions might return FAIL
	return OK;
}

/*************************************************************************
 * Behavioral functions***************************************************
 * They may modify the status of the receiver ****************************
 *************************************************************************/


/***initial functions***/
int UDPBaseImplementation::setDetectorType(const detectorType d){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	myDetectorType = d;
	//if eiger, set numberofListeningThreads = 2;
	FILE_LOG(logINFO) << "Detector Type:" << getDetectorType(d);
	return OK;
}

void UDPBaseImplementation::initialize(const char *c){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	if(strlen(c))
		strcpy(detHostname, c);
	FILE_LOG(logINFO) << "Detector Hostname:" << detHostname;
}


/***acquisition functions***/
void UDPBaseImplementation::resetAcquisitionCount(){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	totalPacketsCaught = 0;
	FILE_LOG(logINFO) << "totalPacketsCaught:" << totalPacketsCaught;
}

int UDPBaseImplementation::startReceiver(char *c){
	FILE_LOG(logWARNING) << __AT__ << " doing nothing...";
	FILE_LOG(logERROR) << __AT__ << " must be overridden by child classes";
	return OK;
}

void UDPBaseImplementation::stopReceiver(){
	FILE_LOG(logWARNING) << __AT__ << " doing nothing...";
	FILE_LOG(logERROR) << __AT__ << " must be overridden by child classes";
}

void UDPBaseImplementation::startReadout(){
	FILE_LOG(logWARNING) << __AT__ << " doing nothing...";
	FILE_LOG(logERROR) << __AT__ << " must be overridden by child classes";
}

int UDPBaseImplementation::shutDownUDPSockets(){
	FILE_LOG(logWARNING) << __AT__ << " doing nothing...";
	FILE_LOG(logERROR) << __AT__ << " must be overridden by child classes";

	//overridden functions might return FAIL
	return OK;
}

void UDPBaseImplementation::readFrame(char* c,char** raw, uint64_t &startAcquisitionIndex, uint64_t &startFrameIndex){
	FILE_LOG(logWARNING) << __AT__ << " doing nothing...";
	FILE_LOG(logERROR) << __AT__ << " must be overridden by child classes";
}

//FIXME: needed, isnt stopReceiver enough?
void UDPBaseImplementation::abort(){
	FILE_LOG(logWARNING) << __AT__ << " doing nothing...";
	FILE_LOG(logERROR) << __AT__ << " must be overridden by child classes";
}

void UDPBaseImplementation::closeFile(int i){
	FILE_LOG(logWARNING) << __AT__ << " doing nothing...";
	FILE_LOG(logERROR) << __AT__ << " must be overridden by child classes";
}


/***callback functions***/
void UDPBaseImplementation::registerCallBackStartAcquisition(int (*func)(char*, char*,int, int, void*),void *arg){
	startAcquisitionCallBack=func;
	pStartAcquisition=arg;
}

void UDPBaseImplementation::registerCallBackAcquisitionFinished(void (*func)(int, void*),void *arg){
	acquisitionFinishedCallBack=func;
	pAcquisitionFinished=arg;
}

void UDPBaseImplementation::registerCallBackRawDataReady(void (*func)(int, char*, int, FILE*, char*, void*),void *arg){
	rawDataReadyCallBack=func;
	pRawDataReady=arg;
}


//#endif
