//#ifdef SLS_RECEIVER_UDP_FUNCTIONS
/********************************************//**
 * @file UDPBaseImplementation.cpp
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 ***********************************************/

#include "UDPBaseImplementation.h"

#include <sys/stat.h> 		// stat
#include <iostream>
#include <string.h>



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
	rawDataModifyReadyCallBack = NULL;
	pRawDataReady = NULL;
}

void UDPBaseImplementation::initializeMembers(){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	FILE_LOG(logDEBUG) << "Info: Initializing base members";
	//**detector parameters***
	for (int i = 0; i < MAX_DIMENSIONS; ++i)
		numDet[i] = 0;
	detID = 0;
	myDetectorType = GENERIC;
	strcpy(detHostname,"");
	acquisitionPeriod = 0;
	acquisitionTime = 0;
	subExpTime = 0;
	subPeriod = 0;
	numberOfFrames = 0;
	numberOfSamples = 0;
	dynamicRange = 16;
	tengigaEnable = false;
	fifoDepth = 0;
	flippedData[0] = 0;
	flippedData[1] = 0;
	gapPixelsEnable = false;

	//***receiver parameters***
	status = IDLE;
	activated = true;
	deactivatedPaddingEnable = true;
	frameDiscardMode = NO_DISCARD;
	framePadding = false;

	//***connection parameters***
	strcpy(eth,"");
	for(int i=0;i<MAX_NUMBER_OF_LISTENING_THREADS;i++){
		udpPortNum[i] = DEFAULT_UDP_PORTNO + i;
	}
	udpSocketBufferSize = 0;
	actualUDPSocketBufferSize = 0;

	//***file parameters***
	fileFormatType = BINARY;
	strcpy(fileName,"run");
	strcpy(filePath,"");
	fileIndex = 0;
	framesPerFile = 0;
	scanTag = 0;
	fileWriteEnable = true;
	overwriteEnable = true;
	dataCompressionEnable = false;

	//***acquisition parameters***
	roi.clear();
	frameToGuiFrequency = 0;
	frameToGuiTimerinMS = DEFAULT_STREAMING_TIMER_IN_MS;
	dataStreamEnable = false;
	streamingPort = 0;
	memset(streamingSrcIP, 0, sizeof(streamingSrcIP));
    memset(additionalJsonHeader, 0, sizeof(additionalJsonHeader));

	//***receiver parameters***
	silentMode = false;

}

UDPBaseImplementation::~UDPBaseImplementation(){}


/*************************************************************************
 * Getters ***************************************************************
 * They access local cache of configuration or detector parameters *******
 *************************************************************************/

/**initial parameters***/
int* UDPBaseImplementation::getMultiDetectorSize() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return (int*) numDet;
}

int UDPBaseImplementation::getDetectorPositionId() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return detID;
}

char *UDPBaseImplementation::getDetectorHostname() const{
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	//not initialized
	if(!strlen(detHostname))
		return NULL;

	char* output = new char[MAX_STR_LENGTH]();
	strcpy(output,detHostname);
	//freed by calling function
	return output;
}

int UDPBaseImplementation::getFlippedData(int axis) const{
	FILE_LOG(logDEBUG) << __AT__ << " starting";
	if(axis<0 || axis > 1) return -1;
	return flippedData[axis];
}

bool UDPBaseImplementation::getGapPixelsEnable() const {
	FILE_LOG(logDEBUG) << __AT__ << " starting";
	return gapPixelsEnable;
}

/***file parameters***/
slsReceiverDefs::fileFormat UDPBaseImplementation::getFileFormat() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return fileFormatType;
}


char *UDPBaseImplementation::getFileName() const{
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	//not initialized
	if(!strlen(fileName))
		return NULL;

	char* output = new char[MAX_STR_LENGTH]();
	strcpy(output,fileName);
	//freed by calling function
	return output;
}

char *UDPBaseImplementation::getFilePath() const{
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	//not initialized
	if(!strlen(filePath))
		return NULL;

	char* output = new char[MAX_STR_LENGTH]();
	strcpy(output,filePath);
	//freed by calling function
	return output;
}

uint64_t UDPBaseImplementation::getFileIndex() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return fileIndex;
}

uint32_t UDPBaseImplementation::getFramesPerFile() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return framesPerFile;
}

slsReceiverDefs::frameDiscardPolicy UDPBaseImplementation::getFrameDiscardPolicy() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return frameDiscardMode;
}

bool UDPBaseImplementation::getFramePaddingEnable() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return framePadding;
}

int UDPBaseImplementation::getScanTag() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return scanTag;
}

bool UDPBaseImplementation::getFileWriteEnable() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return fileWriteEnable;
}

bool UDPBaseImplementation::getOverwriteEnable() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return overwriteEnable;
}

bool UDPBaseImplementation::getDataCompressionEnable() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return dataCompressionEnable;
}

/***acquisition count parameters***/
uint64_t UDPBaseImplementation::getTotalFramesCaught() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return 0;
}

uint64_t UDPBaseImplementation::getFramesCaught() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return 0;
}

int64_t UDPBaseImplementation::getAcquisitionIndex() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return -1;
}


/***connection parameters***/
uint32_t UDPBaseImplementation::getUDPPortNumber() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return udpPortNum[0];
}

uint32_t UDPBaseImplementation::getUDPPortNumber2() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return udpPortNum[1];
}

char *UDPBaseImplementation::getEthernetInterface() const{
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	char* output = new char[MAX_STR_LENGTH]();
	strcpy(output,eth);
	//freed by calling function
	return output;
}


/***acquisition parameters***/
std::vector<slsReceiverDefs::ROI> UDPBaseImplementation::getROI() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return roi;
}

uint32_t UDPBaseImplementation::getFrameToGuiFrequency() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return frameToGuiFrequency;
}

uint32_t UDPBaseImplementation::getFrameToGuiTimer() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return frameToGuiTimerinMS;
}

bool UDPBaseImplementation::getDataStreamEnable() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return dataStreamEnable;
}

uint64_t UDPBaseImplementation::getAcquisitionPeriod() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return acquisitionPeriod;
}

uint64_t UDPBaseImplementation::getAcquisitionTime() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return acquisitionTime;
}

uint64_t UDPBaseImplementation::getSubExpTime() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return subExpTime;
}

uint64_t UDPBaseImplementation::getSubPeriod() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return subPeriod;
}

uint64_t UDPBaseImplementation::getNumberOfFrames() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return numberOfFrames;
}

uint64_t UDPBaseImplementation::getNumberofSamples() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return numberOfSamples;
}

uint32_t UDPBaseImplementation::getDynamicRange() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return dynamicRange;}

bool UDPBaseImplementation::getTenGigaEnable() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return tengigaEnable;
}

uint32_t UDPBaseImplementation::getFifoDepth() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return fifoDepth;
}

/***receiver status***/
slsReceiverDefs::runStatus UDPBaseImplementation::getStatus() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return status;}

bool UDPBaseImplementation::getSilentMode() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return silentMode;}

bool UDPBaseImplementation::getActivate() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return activated;
}

bool UDPBaseImplementation::getDeactivatedPadding() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return deactivatedPaddingEnable;
}

uint32_t UDPBaseImplementation::getStreamingPort() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return streamingPort;
}

char *UDPBaseImplementation::getStreamingSourceIP() const{
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	char* output = new char[MAX_STR_LENGTH]();
	strcpy(output,streamingSrcIP);
	//freed by calling function
	return output;
}

char *UDPBaseImplementation::getAdditionalJsonHeader() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";

    char* output = new char[MAX_STR_LENGTH]();
    memset(output, 0, MAX_STR_LENGTH);
    strcpy(output,additionalJsonHeader);
    //freed by calling function
    return output;
}

uint32_t UDPBaseImplementation::getUDPSocketBufferSize() const {
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return udpSocketBufferSize;
}

uint32_t UDPBaseImplementation::getActualUDPSocketBufferSize() const {
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return actualUDPSocketBufferSize;
}

/*************************************************************************
 * Setters ***************************************************************
 * They modify the local cache of configuration or detector parameters ***
 *************************************************************************/

/**initial parameters***/
void UDPBaseImplementation::configure(std::map<std::string, std::string> config_map){
	FILE_LOG(logERROR) << __AT__ << " doing nothing...";
	FILE_LOG(logERROR) << __AT__ << " must be overridden by child classes";
}

void UDPBaseImplementation::setMultiDetectorSize(const int* size) {
	FILE_LOG(logDEBUG) << __AT__ << " starting";
	char message[100];
	strcpy(message, "Detector Size: (");
	for (int i = 0; i < MAX_DIMENSIONS; ++i) {
		if (myDetectorType == EIGER && (!i))
			numDet[i] = size[i]*2;
		else
			numDet[i] = size[i];
		sprintf(message,"%s%d",message,numDet[i]);
		if (i < MAX_DIMENSIONS-1 )
			strcat(message,",");
	}
	strcat(message,")");
	FILE_LOG(logINFO)  << message;
}

void UDPBaseImplementation::setFlippedData(int axis, int enable){
	FILE_LOG(logDEBUG) << __AT__ << " starting";
	if(axis<0 || axis>1) return;
	flippedData[axis] = enable==0?0:1;
	FILE_LOG(logINFO)  << "Flipped Data: " << flippedData[0] << " , " << flippedData[1];
}

int UDPBaseImplementation::setGapPixelsEnable(const bool b) {
	FILE_LOG(logDEBUG) << __AT__ << " starting";
	gapPixelsEnable = b;
	FILE_LOG(logINFO)  << "Gap Pixels Enable: " << gapPixelsEnable;

	// overridden
	return OK;
}

/***file parameters***/
void UDPBaseImplementation::setFileFormat(const fileFormat f){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	switch(f){
#ifdef HDF5C
	case HDF5:
		fileFormatType = HDF5;
		break;
#endif
	default:
		fileFormatType = BINARY;
		break;
	}

	FILE_LOG(logINFO) << "File Format: " << getFileFormatType(fileFormatType);
}


void UDPBaseImplementation::setFileName(const char c[]){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	if(strlen(c))
		strcpy(fileName, c);
	FILE_LOG(logINFO) << "File name: " << fileName;
}

void UDPBaseImplementation::setFilePath(const char c[]){
	FILE_LOG(logDEBUG) << __AT__ << " starting";


	if(strlen(c)){
		//check if filepath exists
		struct stat st;
		if(stat(c,&st) == 0)
			strcpy(filePath,c);
		else
			FILE_LOG(logERROR) << "FilePath does not exist: " << filePath;
	}
	FILE_LOG(logINFO) << "File path: " << filePath;
}

void UDPBaseImplementation::setFileIndex(const uint64_t i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	fileIndex = i;
	FILE_LOG(logINFO) << "File Index: " << fileIndex;
}

void UDPBaseImplementation::setFramesPerFile(const uint32_t i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	framesPerFile = i;
	FILE_LOG(logINFO) << "Frames per file: " << framesPerFile;
}

void UDPBaseImplementation::setFrameDiscardPolicy(const frameDiscardPolicy i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	if (i >= 0 && i < NUM_DISCARD_POLICIES)
		frameDiscardMode = i;

	FILE_LOG(logINFO) << "Frame Discard Policy: " << getFrameDiscardPolicyType(frameDiscardMode);
}

void UDPBaseImplementation::setFramePaddingEnable(const bool i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	framePadding = i;
	FILE_LOG(logINFO) << "Frame Padding: " << framePadding;
}

//FIXME: needed?
void UDPBaseImplementation::setScanTag(const int i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	scanTag = i;
	FILE_LOG(logINFO) << "Scan Tag: " << scanTag;

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
	FILE_LOG(logINFO) << "UDP Port Number[0]: " << udpPortNum[0];
}

void UDPBaseImplementation::setUDPPortNumber2(const uint32_t i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	udpPortNum[1] = i;
	FILE_LOG(logINFO) << "UDP Port Number[1]: " << udpPortNum[1];
}

void UDPBaseImplementation::setEthernetInterface(const char* c){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	strcpy(eth, c);
	FILE_LOG(logINFO) << "Ethernet Interface: " << eth;
}


/***acquisition parameters***/
int UDPBaseImplementation::setROI(const std::vector<slsReceiverDefs::ROI> i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	roi = i;

	std::stringstream sstm;
	sstm << "ROI: ";
	if (!roi.size())
		sstm << "0";
	else {
		for (unsigned int i = 0; i < roi.size(); ++i) {
			sstm << "( " <<
					roi[i].xmin << ", " <<
					roi[i].xmax << ", " <<
					roi[i].ymin << ", " <<
					roi[i].ymax << " )";
		}
	}
	std::string message = sstm.str();
	FILE_LOG(logINFO) << message;

	//overrridden child classes might return FAIL
	return OK;
}

int UDPBaseImplementation::setFrameToGuiFrequency(const uint32_t freq){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	frameToGuiFrequency = freq;
	FILE_LOG(logINFO) << "Frame To Gui Frequency: " << frameToGuiFrequency;

	//overrridden child classes might return FAIL
	return OK;
}

void UDPBaseImplementation::setFrameToGuiTimer(const uint32_t time_in_ms){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	frameToGuiTimerinMS = time_in_ms;
	FILE_LOG(logINFO) << "Frame To Gui Timer: " << frameToGuiTimerinMS;
}


int UDPBaseImplementation::setDataStreamEnable(const bool enable){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	dataStreamEnable = enable;
	FILE_LOG(logINFO) << "Streaming Data from Receiver: " << dataStreamEnable;

	//overrridden child classes might return FAIL
	return OK;
}


int UDPBaseImplementation::setAcquisitionPeriod(const uint64_t i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	acquisitionPeriod = i;
	FILE_LOG(logINFO) << "Acquisition Period: " <<  (double)acquisitionPeriod/(1E9) << "s";

	//overrridden child classes might return FAIL
	return OK;
}

int UDPBaseImplementation::setAcquisitionTime(const uint64_t i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	acquisitionTime = i;
	FILE_LOG(logINFO) << "Acquisition Time: " <<  (double)acquisitionTime/(1E9) << "s";

	//overrridden child classes might return FAIL
	return OK;
}

void UDPBaseImplementation::setSubExpTime(const uint64_t i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	subExpTime = i;
	FILE_LOG(logINFO) << "Sub Exposure Time: " <<  (double)subExpTime/(1E9) << "s";
}

void UDPBaseImplementation::setSubPeriod(const uint64_t i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	subPeriod = i;
	FILE_LOG(logINFO) << "Sub Exposure Time: " <<  (double)subPeriod/(1E9) << "s";
}

int UDPBaseImplementation::setNumberOfFrames(const uint64_t i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	numberOfFrames = i;
	FILE_LOG(logINFO) << "Number of Frames: " << numberOfFrames;

	//overrridden child classes might return FAIL
	return OK;
}

int UDPBaseImplementation::setNumberofSamples(const uint64_t i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	numberOfSamples = i;
	FILE_LOG(logINFO) << "Number of Samples: " << numberOfSamples;

	//overrridden child classes might return FAIL
	return OK;
}

int UDPBaseImplementation::setDynamicRange(const uint32_t i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	dynamicRange = i;
	FILE_LOG(logINFO) << "Dynamic Range: " << dynamicRange;

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

/***receiver parameters***/
void UDPBaseImplementation::setSilentMode(const bool i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	silentMode = i;
	FILE_LOG(logINFO) << "Silent Mode: " << i;
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
	FILE_LOG(logINFO) << "Detector Type: " << getDetectorType(d);
	return OK;
}

void UDPBaseImplementation::setDetectorPositionId(const int i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	detID = i;
	FILE_LOG(logINFO) << "Detector Position Id: " << detID;
}

void UDPBaseImplementation::initialize(const char *c){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	if(strlen(c))
		strcpy(detHostname, c);
	FILE_LOG(logINFO) << "Detector Hostname: " << detHostname;
}


/***acquisition functions***/
void UDPBaseImplementation::resetAcquisitionCount(){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	//overriden by resetting of new acquisition parameters
}

int UDPBaseImplementation::startReceiver(char *c){
	FILE_LOG(logERROR) << __AT__ << " doing nothing...";
	FILE_LOG(logERROR) << __AT__ << " must be overridden by child classes";
	return OK;
}

void UDPBaseImplementation::stopReceiver(){
	FILE_LOG(logERROR) << __AT__ << " doing nothing...";
	FILE_LOG(logERROR) << __AT__ << " must be overridden by child classes";
}

void UDPBaseImplementation::startReadout(){
	FILE_LOG(logERROR) << __AT__ << " doing nothing...";
	FILE_LOG(logERROR) << __AT__ << " must be overridden by child classes";
}

void UDPBaseImplementation::shutDownUDPSockets(){
	FILE_LOG(logERROR) << __AT__ << " doing nothing...";
	FILE_LOG(logERROR) << __AT__ << " must be overridden by child classes";
}


//FIXME: needed, isnt stopReceiver enough?
void UDPBaseImplementation::abort(){
	FILE_LOG(logERROR) << __AT__ << " doing nothing...";
	FILE_LOG(logERROR) << __AT__ << " must be overridden by child classes";
}


bool UDPBaseImplementation::setActivate(bool enable){
	FILE_LOG(logDEBUG) << __AT__ << " starting";
	activated = enable;
	FILE_LOG(logINFO) << "Activation: " << stringEnable(activated);
	return activated;
}

bool UDPBaseImplementation::setDeactivatedPadding(bool enable){
	FILE_LOG(logDEBUG) << __AT__ << " starting";
	deactivatedPaddingEnable = enable;
	FILE_LOG(logINFO) << "Deactivated Padding Enable: " << stringEnable(deactivatedPaddingEnable);
	return deactivatedPaddingEnable;
}

void UDPBaseImplementation::setStreamingPort(const uint32_t i) {
	streamingPort = i;

	FILE_LOG(logINFO) << "Streaming Port: " << streamingPort;
}

void UDPBaseImplementation::setStreamingSourceIP(const char c[]){
	FILE_LOG(logDEBUG) << __AT__ << " starting";
	strcpy(streamingSrcIP, c);
	FILE_LOG(logINFO) << "Streaming Source IP: " << streamingSrcIP;
}

void UDPBaseImplementation::setAdditionalJsonHeader(const char c[]){
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    strcpy(additionalJsonHeader, c);
    FILE_LOG(logINFO) << "Additional JSON Header: " << additionalJsonHeader;
}

int UDPBaseImplementation::setUDPSocketBufferSize(const uint32_t s) {
    FILE_LOG(logDEBUG) << __AT__ << " starting";

    udpSocketBufferSize = s;
    return OK;
}


int UDPBaseImplementation::restreamStop() {
	FILE_LOG(logERROR) << __AT__ << " doing nothing...";
	FILE_LOG(logERROR) << __AT__ << " must be overridden by child classes";
	return OK;
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

void UDPBaseImplementation::registerCallBackRawDataReady(void (*func)(char* ,
		char*, uint32_t, void*),void *arg){
	rawDataReadyCallBack=func;
	pRawDataReady=arg;
}

void UDPBaseImplementation::registerCallBackRawDataModifyReady(void (*func)(char* ,
        char*, uint32_t&, void*),void *arg){
    rawDataModifyReadyCallBack=func;
    pRawDataReady=arg;
}
//#endif
