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
	strcpy(detHostname,"");
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
	strcpy(output,detHostname);
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
	strcpy(output,fileName);
	//freed by calling function
	return output;
}

char *UDPBaseImplementation::getFilePath() const{

	//not initialized
	if(!strlen(filePath))
		return NULL;

	char* output = new char[MAX_STR_LENGTH]();
	strcpy(output,filePath);
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
	strcpy(output,eth);
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
	ostringstream os;
	os << " must be overridden by child classes";
	string message(os.str());	FILE_LOG(logERROR, message);
}

void UDPBaseImplementation::setFlippedData(int axis, int enable){

	if(axis<0 || axis>1) return;
	flippedData[axis] = enable==0?0:1;

	ostringstream os;
	os << "Flipped Data: " << flippedData[0] << " , " << flippedData[1];
	string message(os.str());	FILE_LOG(logINFO, message);
}


/***file parameters***/
void UDPBaseImplementation::setFileName(const char c[]){

	if(strlen(c))
		strcpy(fileName, c);

	ostringstream os;
	os << "File name:" << fileName;
	string message(os.str());	FILE_LOG(logINFO, message);
}

void UDPBaseImplementation::setFilePath(const char c[]){


	if(strlen(c)){
		//check if filepath exists
		struct stat st;
		if(stat(c,&st) == 0)
			strcpy(filePath,c);
		else{
			strcpy(filePath,"");
			ostringstream os;
			os <<  "FilePath does not exist:" << filePath;
			string message(os.str());	FILE_LOG(logWARNING, message);
		}
		strcpy(filePath, c);
	}
	/*FILE_LOG(logDEBUG) << "Info: File path:" << filePath;*/
}

void UDPBaseImplementation::setFileIndex(const uint64_t i){


	fileIndex = i;
	ostringstream os;
	os << "File Index:" << fileIndex;
	string message(os.str());	FILE_LOG(logINFO, message);
}

//FIXME: needed?
void UDPBaseImplementation::setScanTag(const int i){


	scanTag = i;
	ostringstream os;
	os << "Scan Tag:" << scanTag;
	string message(os.str());	FILE_LOG(logINFO, message);

}

void UDPBaseImplementation::setFrameIndexEnable(const bool b){


	frameIndexEnable = b;
	ostringstream os;
	os << "Frame Index Enable: " << stringEnable(frameIndexEnable);
	string message(os.str());	FILE_LOG(logINFO, message);
}

void UDPBaseImplementation::setFileWriteEnable(const bool b){


	fileWriteEnable = b;
	ostringstream os;
	os << "File Write Enable: " << stringEnable(fileWriteEnable);
	string message(os.str());	FILE_LOG(logINFO, message);
}

void UDPBaseImplementation::setOverwriteEnable(const bool b){


	overwriteEnable = b;
	ostringstream os;
	os << "Overwrite Enable: " << stringEnable(overwriteEnable);
	string message(os.str());	FILE_LOG(logINFO, message);
}

int UDPBaseImplementation::setDataCompressionEnable(const bool b){


	dataCompressionEnable = b;
	ostringstream os;
	os << "Data Compression : " << stringEnable(dataCompressionEnable);
	string message(os.str());	FILE_LOG(logINFO, message);

	//overridden methods might return FAIL
	return OK;
}


/***connection parameters***/
void UDPBaseImplementation::setUDPPortNumber(const uint32_t i){


	udpPortNum[0] = i;
	ostringstream os;
	os << "UDP Port Number[0]:" << udpPortNum[0];
	string message(os.str());	FILE_LOG(logINFO, message);
}

void UDPBaseImplementation::setUDPPortNumber2(const uint32_t i){


	udpPortNum[1] = i;
	ostringstream os;
	os << "UDP Port Number[1]:" << udpPortNum[1];
	string message(os.str());	FILE_LOG(logINFO, message);
}

void UDPBaseImplementation::setEthernetInterface(const char* c){


	strcpy(eth, c);
	ostringstream os;
	os << "Ethernet Interface: " << eth;
	string message(os.str());	FILE_LOG(logINFO, message);
}


/***acquisition parameters***/
void UDPBaseImplementation::setShortFrameEnable(const int i){


	shortFrameEnable = i;
	ostringstream os;
	os << "Short Frame Enable: " << stringEnable(shortFrameEnable);
	string message(os.str());	FILE_LOG(logINFO, message);
}

int UDPBaseImplementation::setFrameToGuiFrequency(const uint32_t freq){


	frameToGuiFrequency = freq;
	ostringstream os;
	os << "Frame To Gui Frequency:" << frameToGuiFrequency;
	string message(os.str());	FILE_LOG(logINFO, message);

	//overrridden child classes might return FAIL
	return OK;
}

void UDPBaseImplementation::setFrameToGuiTimer(const uint32_t time_in_ms){


	frameToGuiTimerinMS = time_in_ms;
	ostringstream os;
	os << "Frame To Gui Timer:" << frameToGuiTimerinMS;
	string message(os.str());	FILE_LOG(logINFO, message);
}


uint32_t UDPBaseImplementation::setDataStreamEnable(const uint32_t enable){


	dataStreamEnable = enable;
	ostringstream os;
	os << "Streaming Data from Receiver:" << dataStreamEnable;
	string message(os.str());	FILE_LOG(logINFO, message);

	//overrridden child classes might return FAIL
	return OK;
}


int UDPBaseImplementation::setAcquisitionPeriod(const uint64_t i){


	acquisitionPeriod = i;
	ostringstream os;
	os << "Acquisition Period:" <<  (double)acquisitionPeriod/(1E9) << "s";
	string message(os.str());	FILE_LOG(logINFO, message);

	//overrridden child classes might return FAIL
	return OK;
}

int UDPBaseImplementation::setAcquisitionTime(const uint64_t i){


	acquisitionTime = i;
	ostringstream os;
	os << "Acquisition Time:" <<  (double)acquisitionTime/(1E9) << "s";
	string message(os.str());	FILE_LOG(logINFO, message);

	//overrridden child classes might return FAIL
	return OK;
}

int UDPBaseImplementation::setNumberOfFrames(const uint64_t i){


	numberOfFrames = i;
	ostringstream os;
	os << "Number of Frames:" << numberOfFrames;
	string message(os.str());	FILE_LOG(logINFO, message);

	//overrridden child classes might return FAIL
	return OK;
}

int UDPBaseImplementation::setDynamicRange(const uint32_t i){


	dynamicRange = i;
	ostringstream os;
	os << "Dynamic Range:" << dynamicRange;
	string message(os.str());	FILE_LOG(logINFO, message);

	//overrridden child classes might return FAIL
	return OK;
}

int UDPBaseImplementation::setTenGigaEnable(const bool b){


	tengigaEnable = b;
	ostringstream os;
	os << "Ten Giga Enable: " << stringEnable(tengigaEnable);
	string message(os.str());	FILE_LOG(logINFO, message);

	//overridden functions might return FAIL
	return OK;
}

int UDPBaseImplementation::setFifoDepth(const uint32_t i){


	fifoDepth = i;
	ostringstream os;
	os << "Fifo Depth: " << i;
	string message(os.str());	FILE_LOG(logINFO, message);

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
	ostringstream os;
	os << "Detector Type:" << getDetectorType(d);
	string message(os.str());	FILE_LOG(logINFO, message);
	return OK;
}

void UDPBaseImplementation::initialize(const char *c){


	if(strlen(c))
		strcpy(detHostname, c);
	ostringstream os;
	os << "Detector Hostname:" << detHostname;
	string message(os.str());	FILE_LOG(logINFO, message);
}


/***acquisition functions***/
void UDPBaseImplementation::resetAcquisitionCount(){


	totalPacketsCaught = 0;
	ostringstream os;
	os << "totalPacketsCaught:" << totalPacketsCaught;
	string message(os.str());	FILE_LOG(logINFO, message);
}

int UDPBaseImplementation::startReceiver(char *c){
	ostringstream os;
	os << " must be overridden by child classes";
	string message(os.str());	FILE_LOG(logERROR, message);
	return OK;
}

void UDPBaseImplementation::stopReceiver(){
	ostringstream os;
	os << " must be overridden by child classes";
	string message(os.str());	FILE_LOG(logERROR, message);
}

void UDPBaseImplementation::startReadout(){
	ostringstream os;
	os << " must be overridden by child classes";
	string message(os.str());	FILE_LOG(logERROR, message);
}

int UDPBaseImplementation::shutDownUDPSockets(){
	ostringstream os;
	os << " must be overridden by child classes";
	string message(os.str());	FILE_LOG(logERROR, message);

	//overridden functions might return FAIL
	return OK;
}

void UDPBaseImplementation::readFrame(int ithread, char* c,char** raw, int64_t &startAcquisitionIndex, int64_t &startFrameIndex){
	ostringstream os;
	os << " must be overridden by child classes";
	string message(os.str());	FILE_LOG(logERROR, message);
}


//FIXME: needed, isnt stopReceiver enough?
void UDPBaseImplementation::abort(){
	ostringstream os;
	os << " must be overridden by child classes";
	string message(os.str());	FILE_LOG(logERROR, message);
}

void UDPBaseImplementation::closeFile(int ithread){
	ostringstream os;
	os << " must be overridden by child classes";
	string message(os.str());	FILE_LOG(logERROR, message);
}


int UDPBaseImplementation::setActivate(int enable){


	if(enable != -1){
		activated = enable;
		ostringstream os;
		os << "Activation: " << stringEnable(activated);
		string message(os.str());	FILE_LOG(logINFO, message);
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
