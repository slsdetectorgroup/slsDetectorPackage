/********************************************//**
 * @file slsReceiverImplementation.cpp
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 ***********************************************/


#include "slsReceiverImplementation.h"
#include "GeneralData.h"
#include "Listener.h"
#include "DataProcessor.h"
#include "DataStreamer.h"
#include "Fifo.h"
#include "ZmqSocket.h" 		//just for the zmq port define

#include <sys/stat.h> 		// stat
#include <iostream>
#include <string.h>
#include <cstdlib>			//system
#include <cstring>			//strcpy
#include <errno.h>			//eperm
#include <fstream>


/** cosntructor & destructor */

slsReceiverImplementation::slsReceiverImplementation() {
	InitializeMembers();
}


slsReceiverImplementation::~slsReceiverImplementation() {
	DeleteMembers();
}


void slsReceiverImplementation::DeleteMembers() {
	if (generalData) {
		delete generalData;
		generalData=0;
	}

	for (auto* it : listener)
		delete it;
	listener.clear();

	for (auto* it : dataProcessor)
		delete it;
	dataProcessor.clear();

	for (auto* it : dataStreamer)
		delete it;
	dataStreamer.clear();

	for (auto* it : fifo)
		delete it;
	fifo.clear();
}


void slsReceiverImplementation::InitializeMembers() {

	//**detector parameters***
	myDetectorType = GENERIC;
	for (int i = 0; i < MAX_DIMENSIONS; ++i)
		numDet[i] = 0;
	detID = 0;
	strcpy(detHostname,"");
	acquisitionPeriod = SAMPLE_TIME_IN_NS;
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

	//*** receiver parameters ***
	numThreads = 1;
	numberofJobs = 1;
	nroichannels = 0;
	status = IDLE;
	activated = true;
	deactivatedPaddingEnable = true;
	frameDiscardMode = NO_DISCARD;
	framePadding = false;
	silentMode = false;

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
	fileWriteEnable = true;
	overwriteEnable = true;

	//***acquisition parameters***
	roi.clear();
	streamingFrequency = 0;
	streamingTimerInMs = DEFAULT_STREAMING_TIMER_IN_MS;
	dataStreamEnable = false;
	streamingPort = 0;
	memset(streamingSrcIP, 0, sizeof(streamingSrcIP));
    memset(additionalJsonHeader, 0, sizeof(additionalJsonHeader));

	//** class objects ***
	generalData = 0;

	//***callback parameters***
	startAcquisitionCallBack = NULL;
	pStartAcquisition = NULL;
	acquisitionFinishedCallBack = NULL;
	pAcquisitionFinished = NULL;
	rawDataReadyCallBack = NULL;
	rawDataModifyReadyCallBack = NULL;
	pRawDataReady = NULL;
}

/*************************************************************************
 * Getters ***************************************************************
 * They access local cache of configuration or detector parameters *******
 *************************************************************************/

/**initial parameters***/
int* slsReceiverImplementation::getMultiDetectorSize() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return (int*) numDet;
}

int slsReceiverImplementation::getDetectorPositionId() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return detID;
}

char *slsReceiverImplementation::getDetectorHostname() const{
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	//not initialized
	if(!strlen(detHostname))
		return NULL;

	char* output = new char[MAX_STR_LENGTH]();
	strcpy(output,detHostname);
	//freed by calling function
	return output;
}

int slsReceiverImplementation::getFlippedData(int axis) const{
	FILE_LOG(logDEBUG) << __AT__ << " starting";
	if(axis<0 || axis > 1) return -1;
	return flippedData[axis];
}

bool slsReceiverImplementation::getGapPixelsEnable() const {
	FILE_LOG(logDEBUG) << __AT__ << " starting";
	return gapPixelsEnable;
}

/***file parameters***/
slsReceiverDefs::fileFormat slsReceiverImplementation::getFileFormat() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return fileFormatType;
}


char *slsReceiverImplementation::getFileName() const{
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	//not initialized
	if(!strlen(fileName))
		return NULL;

	char* output = new char[MAX_STR_LENGTH]();
	strcpy(output,fileName);
	//freed by calling function
	return output;
}

char *slsReceiverImplementation::getFilePath() const{
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	//not initialized
	if(!strlen(filePath))
		return NULL;

	char* output = new char[MAX_STR_LENGTH]();
	strcpy(output,filePath);
	//freed by calling function
	return output;
}

uint64_t slsReceiverImplementation::getFileIndex() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return fileIndex;
}

uint32_t slsReceiverImplementation::getFramesPerFile() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return framesPerFile;
}

slsReceiverDefs::frameDiscardPolicy slsReceiverImplementation::getFrameDiscardPolicy() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return frameDiscardMode;
}

bool slsReceiverImplementation::getFramePaddingEnable() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return framePadding;
}


bool slsReceiverImplementation::getFileWriteEnable() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return fileWriteEnable;
}

bool slsReceiverImplementation::getOverwriteEnable() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return overwriteEnable;
}


/***acquisition count parameters***/
uint64_t slsReceiverImplementation::getTotalFramesCaught() const {
	uint64_t sum = 0;
	uint32_t flagsum = 0;

	std::vector<DataProcessor*>::const_iterator it;
	for (std::vector<DataProcessor*>::const_iterator it = dataProcessor.begin(); it != dataProcessor.end(); ++it) {
		flagsum += ((*it)->GetMeasurementStartedFlag() ? 1 : 0);
		sum += (*it)->GetNumTotalFramesCaught();
	}

	//no data processed
	if (flagsum != dataProcessor.size()) return 0;

	return (sum/dataProcessor.size());
}

uint64_t slsReceiverImplementation::getFramesCaught() const {
	uint64_t sum = 0;
	uint32_t flagsum = 0;

	for (std::vector<DataProcessor*>::const_iterator it = dataProcessor.begin(); it != dataProcessor.end(); ++it) {
		flagsum += ((*it)->GetAcquisitionStartedFlag() ? 1 : 0);
		sum += (*it)->GetNumFramesCaught();
	}

	//no data processed
	if (flagsum != dataProcessor.size()) return 0;

	return (sum/dataProcessor.size());
}

int64_t slsReceiverImplementation::getAcquisitionIndex() const {
	uint64_t sum = 0;
	uint32_t flagsum = 0;

	for (std::vector<DataProcessor*>::const_iterator it = dataProcessor.begin(); it != dataProcessor.end(); ++it){
		flagsum += ((*it)->GetAcquisitionStartedFlag() ? 1 : 0);
		sum += (*it)->GetActualProcessedAcquisitionIndex();
	}

	//no data processed
	if (flagsum != dataProcessor.size()) return -1;

	return (sum/dataProcessor.size());
}




/***connection parameters***/
uint32_t slsReceiverImplementation::getUDPPortNumber() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return udpPortNum[0];
}

uint32_t slsReceiverImplementation::getUDPPortNumber2() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return udpPortNum[1];
}

char *slsReceiverImplementation::getEthernetInterface() const{
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	char* output = new char[MAX_STR_LENGTH]();
	strcpy(output,eth);
	//freed by calling function
	return output;
}


/***acquisition parameters***/
std::vector<slsReceiverDefs::ROI> slsReceiverImplementation::getROI() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return roi;
}

uint32_t slsReceiverImplementation::getStreamingFrequency() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return streamingFrequency;
}

uint32_t slsReceiverImplementation::getStreamingTimer() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return streamingTimerInMs;
}

bool slsReceiverImplementation::getDataStreamEnable() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return dataStreamEnable;
}

uint64_t slsReceiverImplementation::getAcquisitionPeriod() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return acquisitionPeriod;
}

uint64_t slsReceiverImplementation::getAcquisitionTime() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return acquisitionTime;
}

uint64_t slsReceiverImplementation::getSubExpTime() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return subExpTime;
}

uint64_t slsReceiverImplementation::getSubPeriod() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return subPeriod;
}

uint64_t slsReceiverImplementation::getNumberOfFrames() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return numberOfFrames;
}

uint64_t slsReceiverImplementation::getNumberofSamples() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return numberOfSamples;
}

uint32_t slsReceiverImplementation::getDynamicRange() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return dynamicRange;}

bool slsReceiverImplementation::getTenGigaEnable() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return tengigaEnable;
}

uint32_t slsReceiverImplementation::getFifoDepth() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return fifoDepth;
}

/***receiver status***/
slsReceiverDefs::runStatus slsReceiverImplementation::getStatus() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return status;}

bool slsReceiverImplementation::getSilentMode() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return silentMode;}

bool slsReceiverImplementation::getActivate() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return activated;
}

bool slsReceiverImplementation::getDeactivatedPadding() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return deactivatedPaddingEnable;
}

uint32_t slsReceiverImplementation::getStreamingPort() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return streamingPort;
}

char *slsReceiverImplementation::getStreamingSourceIP() const{
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	char* output = new char[MAX_STR_LENGTH]();
	strcpy(output,streamingSrcIP);
	//freed by calling function
	return output;
}

char *slsReceiverImplementation::getAdditionalJsonHeader() const{
    FILE_LOG(logDEBUG) << __AT__ << " starting";

    char* output = new char[MAX_STR_LENGTH]();
    memset(output, 0, MAX_STR_LENGTH);
    strcpy(output,additionalJsonHeader);
    //freed by calling function
    return output;
}

uint32_t slsReceiverImplementation::getUDPSocketBufferSize() const {
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return udpSocketBufferSize;
}

uint32_t slsReceiverImplementation::getActualUDPSocketBufferSize() const {
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    return actualUDPSocketBufferSize;
}


/*************************************************************************
 * Setters ***************************************************************
 * They modify the local cache of configuration or detector parameters ***
 *************************************************************************/

/**initial parameters***/

void slsReceiverImplementation::setDetectorHostname(const char *c){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	if(strlen(c))
		strcpy(detHostname, c);
	FILE_LOG(logINFO) << "Detector Hostname: " << detHostname;
}


void slsReceiverImplementation::setMultiDetectorSize(const int* size) {
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


void slsReceiverImplementation::setFlippedData(int axis, int enable){
	FILE_LOG(logDEBUG) << __AT__ << " starting";
	if(axis<0 || axis>1) return;
	flippedData[axis] = enable==0?0:1;
	FILE_LOG(logINFO)  << "Flipped Data: " << flippedData[0] << " , " << flippedData[1];
}


int slsReceiverImplementation::setGapPixelsEnable(const bool b) {
	if (gapPixelsEnable != b) {
		gapPixelsEnable = b;

		// side effects
		generalData->SetGapPixelsEnable(b, dynamicRange);
		// to update npixelsx, npixelsy in file writer
		for (std::vector<DataProcessor*>::const_iterator it = dataProcessor.begin(); it != dataProcessor.end(); ++it)
			(*it)->SetPixelDimension();

		numberofJobs = -1; //changes to imagesize has to be noted to recreate fifo structure
		if (SetupFifoStructure() == FAIL)
			return FAIL;
	}
	FILE_LOG(logINFO)  << "Gap Pixels Enable: " << gapPixelsEnable;
	return OK;
}


void slsReceiverImplementation::setFileFormat(const fileFormat f){
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
	//destroy file writer, set file format and create file writer
	for (std::vector<DataProcessor*>::const_iterator it = dataProcessor.begin(); it != dataProcessor.end(); ++it)
		(*it)->SetFileFormat(f);

	FILE_LOG(logINFO) << "File Format:" << getFileFormatType(fileFormatType);
}


void slsReceiverImplementation::setFileName(const char c[]){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	if(strlen(c))
		strcpy(fileName, c);
	FILE_LOG(logINFO) << "File name: " << fileName;
}


void slsReceiverImplementation::setFilePath(const char c[]){
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


void slsReceiverImplementation::setFileIndex(const uint64_t i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	fileIndex = i;
	FILE_LOG(logINFO) << "File Index: " << fileIndex;
}


void slsReceiverImplementation::setFramesPerFile(const uint32_t i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	framesPerFile = i;
	FILE_LOG(logINFO) << "Frames per file: " << framesPerFile;
}


void slsReceiverImplementation::setFrameDiscardPolicy(const frameDiscardPolicy i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	if (i >= 0 && i < NUM_DISCARD_POLICIES)
		frameDiscardMode = i;

	FILE_LOG(logINFO) << "Frame Discard Policy: " << getFrameDiscardPolicyType(frameDiscardMode);
}


void slsReceiverImplementation::setFramePaddingEnable(const bool i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	framePadding = i;
	FILE_LOG(logINFO) << "Frame Padding: " << framePadding;
}


void slsReceiverImplementation::setFileWriteEnable(const bool b){
	if (fileWriteEnable != b){
		fileWriteEnable = b;
		for (unsigned int i = 0; i < dataProcessor.size(); ++i) {
				dataProcessor[i]->SetupFileWriter(fileWriteEnable, (int*)numDet,
					&framesPerFile, fileName, filePath, &fileIndex,	&overwriteEnable,
					&detID, &numThreads, &numberOfFrames, &dynamicRange, &udpPortNum[i],
					generalData);
		}
	}

	FILE_LOG(logINFO) << "File Write Enable: " << stringEnable(fileWriteEnable);
}


void slsReceiverImplementation::setOverwriteEnable(const bool b){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	overwriteEnable = b;
	FILE_LOG(logINFO) << "Overwrite Enable: " << stringEnable(overwriteEnable);
}


/***connection parameters***/
void slsReceiverImplementation::setUDPPortNumber(const uint32_t i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	udpPortNum[0] = i;
	FILE_LOG(logINFO) << "UDP Port Number[0]: " << udpPortNum[0];
}

void slsReceiverImplementation::setUDPPortNumber2(const uint32_t i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	udpPortNum[1] = i;
	FILE_LOG(logINFO) << "UDP Port Number[1]: " << udpPortNum[1];
}

void slsReceiverImplementation::setEthernetInterface(const char* c){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	strcpy(eth, c);
	FILE_LOG(logINFO) << "Ethernet Interface: " << eth;
}

int slsReceiverImplementation::setUDPSocketBufferSize(const uint32_t s) {
    if (listener.size())
        return listener[0]->CreateDummySocketForUDPSocketBufferSize(s);
    return FAIL;
}


/***acquisition parameters***/
int slsReceiverImplementation::setROI(const std::vector<slsReceiverDefs::ROI> i) {
	if (myDetectorType != GOTTHARD) {
		cprintf(RED, "Error: Can not set ROI for this detector\n");
		return FAIL;
	}


	bool change = false;
	if (roi.size() != i.size())
		change = true;
	else {
		for (unsigned int iloop = 0; iloop < i.size(); ++iloop) {
			if (
					(roi[iloop].xmin != i[iloop].xmin) ||
					(roi[iloop].xmax != i[iloop].xmax) ||
					(roi[iloop].ymin != i[iloop].ymin) ||
					(roi[iloop].xmax != i[iloop].xmax)) {
				change = true;
				break;
			}
		}
	}

	if (change) {

		roi = i;

		generalData->SetROI(i);
		framesPerFile = generalData->maxFramesPerFile;

		numberofJobs = -1; //changes to imagesize has to be noted to recreate fifo structure
		if (SetupFifoStructure() == FAIL)
			return FAIL;

		for (std::vector<Listener*>::const_iterator it = listener.begin(); it != listener.end(); ++it)
			(*it)->SetGeneralData(generalData);
		for (std::vector<DataProcessor*>::const_iterator it = dataProcessor.begin(); it != dataProcessor.end(); ++it)
			(*it)->SetGeneralData(generalData);
		for (std::vector<DataStreamer*>::const_iterator it = dataStreamer.begin(); it != dataStreamer.end(); ++it)
			(*it)->SetGeneralData(generalData);
	}


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
	return OK;
}


int slsReceiverImplementation::setStreamingFrequency(const uint32_t freq) {
	if (streamingFrequency != freq) {
		streamingFrequency = freq;
	}
	FILE_LOG(logINFO) << "Streaming Frequency: " << streamingFrequency;
	return OK;
}

void slsReceiverImplementation::setStreamingTimer(const uint32_t time_in_ms){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	streamingTimerInMs = time_in_ms;
	FILE_LOG(logINFO) << "Streamer Timer: " << streamingTimerInMs;
}


int slsReceiverImplementation::setDataStreamEnable(const bool enable) {

	if (dataStreamEnable != enable) {
		dataStreamEnable = enable;

		//data sockets have to be created again as the client ones are
		for (std::vector<DataStreamer*>::const_iterator it = dataStreamer.begin(); it != dataStreamer.end(); ++it)
			delete(*it);
		dataStreamer.clear();

		if (enable) {
		    for ( int i = 0; i < numThreads; ++i ) {
		        try {
		            DataStreamer* s = new DataStreamer(i, fifo[i], &dynamicRange,
		                  &roi, &fileIndex, flippedData, additionalJsonHeader, &silentMode);
		            dataStreamer.push_back(s);
		            dataStreamer[i]->SetGeneralData(generalData);
		            dataStreamer[i]->CreateZmqSockets(&numThreads, streamingPort, streamingSrcIP);
		        }
		        catch(...) {
		            for (std::vector<DataStreamer*>::const_iterator it = dataStreamer.begin(); it != dataStreamer.end(); ++it)
		                delete(*it);
		            dataStreamer.clear();
		            dataStreamEnable = false;
		            return FAIL;
		        }
		    }
		    SetThreadPriorities();
		}
	}
	FILE_LOG(logINFO) << "Data Send to Gui: " << dataStreamEnable;
	return OK;
}


void slsReceiverImplementation::setStreamingPort(const uint32_t i) {
	streamingPort = i;

	FILE_LOG(logINFO) << "Streaming Port: " << streamingPort;
}


void slsReceiverImplementation::setStreamingSourceIP(const char c[]){
	FILE_LOG(logDEBUG) << __AT__ << " starting";
	strcpy(streamingSrcIP, c);
	FILE_LOG(logINFO) << "Streaming Source IP: " << streamingSrcIP;
}


void slsReceiverImplementation::setAdditionalJsonHeader(const char c[]){
    FILE_LOG(logDEBUG) << __AT__ << " starting";
    strcpy(additionalJsonHeader, c);
    FILE_LOG(logINFO) << "Additional JSON Header: " << additionalJsonHeader;
}


int slsReceiverImplementation::setAcquisitionPeriod(const uint64_t i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	acquisitionPeriod = i;
	FILE_LOG(logINFO) << "Acquisition Period: " <<  (double)acquisitionPeriod/(1E9) << "s";

	//overrridden child classes might return FAIL
	return OK;
}

int slsReceiverImplementation::setAcquisitionTime(const uint64_t i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	acquisitionTime = i;
	FILE_LOG(logINFO) << "Acquisition Time: " <<  (double)acquisitionTime/(1E9) << "s";

	//overrridden child classes might return FAIL
	return OK;
}

void slsReceiverImplementation::setSubExpTime(const uint64_t i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	subExpTime = i;
	FILE_LOG(logINFO) << "Sub Exposure Time: " <<  (double)subExpTime/(1E9) << "s";
}

void slsReceiverImplementation::setSubPeriod(const uint64_t i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	subPeriod = i;
	FILE_LOG(logINFO) << "Sub Exposure Period: " <<  (double)subPeriod/(1E9) << "s";
}

int slsReceiverImplementation::setNumberOfFrames(const uint64_t i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	numberOfFrames = i;
	FILE_LOG(logINFO) << "Number of Frames: " << numberOfFrames;

	//overrridden child classes might return FAIL
	return OK;
}


int slsReceiverImplementation::setNumberofSamples(const uint64_t i) {
	if (numberOfSamples != i) {
		numberOfSamples = i;

		generalData->setNumberofSamples(i, nroichannels);
		numberofJobs = -1; //changes to imagesize has to be noted to recreate fifo structure
		if (SetupFifoStructure() == FAIL)
			return FAIL;
	}
	FILE_LOG (logINFO) << "Number of Samples: " << numberOfSamples;
	FILE_LOG (logINFO) << "Packets per Frame: " << (generalData->packetsPerFrame);
	return OK;
}


int slsReceiverImplementation::setDynamicRange(const uint32_t i) {
	if (dynamicRange != i) {
		dynamicRange = i;

		//side effects
		generalData->SetDynamicRange(i,tengigaEnable);
		generalData->SetGapPixelsEnable(gapPixelsEnable, dynamicRange);
		// to update npixelsx, npixelsy in file writer
		for (std::vector<DataProcessor*>::const_iterator it = dataProcessor.begin(); it != dataProcessor.end(); ++it)
			(*it)->SetPixelDimension();

		numberofJobs = -1; //changes to imagesize has to be noted to recreate fifo structure
		if (SetupFifoStructure() == FAIL)
			return FAIL;
	}
	FILE_LOG(logINFO) << "Dynamic Range: " << dynamicRange;
	return OK;
}


int slsReceiverImplementation::setTenGigaEnable(const bool b) {
	if (tengigaEnable != b) {
		tengigaEnable = b;
		//side effects
		generalData->SetTenGigaEnable(b,dynamicRange);

		numberofJobs = -1; //changes to imagesize has to be noted to recreate fifo structure
		if (SetupFifoStructure() == FAIL)
			return FAIL;
	}
	FILE_LOG(logINFO) << "Ten Giga: " << stringEnable(tengigaEnable);
	return OK;
}


int slsReceiverImplementation::setFifoDepth(const uint32_t i) {
	if (fifoDepth != i) {
		fifoDepth = i;

		numberofJobs = -1; //changes to imagesize has to be noted to recreate fifo structure
		if (SetupFifoStructure() == FAIL)
			return FAIL;
	}
	FILE_LOG(logINFO) << "Fifo Depth: " << i;
	return OK;
}


/***receiver parameters***/
bool slsReceiverImplementation::setActivate(bool enable){
	FILE_LOG(logDEBUG) << __AT__ << " starting";
	activated = enable;
	FILE_LOG(logINFO) << "Activation: " << stringEnable(activated);
	return activated;
}


bool slsReceiverImplementation::setDeactivatedPadding(bool enable){
	FILE_LOG(logDEBUG) << __AT__ << " starting";
	deactivatedPaddingEnable = enable;
	FILE_LOG(logINFO) << "Deactivated Padding Enable: " << stringEnable(deactivatedPaddingEnable);
	return deactivatedPaddingEnable;
}

void slsReceiverImplementation::setSilentMode(const bool i){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	silentMode = i;
	FILE_LOG(logINFO) << "Silent Mode: " << i;
}


/*************************************************************************
 * Behavioral functions***************************************************
 * They may modify the status of the receiver ****************************
 *************************************************************************/


/***initial functions***/
int slsReceiverImplementation::setDetectorType(const detectorType d) {
	FILE_LOG(logDEBUG) << "Setting receiver type";
	DeleteMembers();
	InitializeMembers();
	myDetectorType = d;
	switch(myDetectorType) {
	case GOTTHARD:
	case PROPIX:
	case MOENCH:
	case EIGER:
	case JUNGFRAUCTB:
	case JUNGFRAU:
		FILE_LOG(logINFO) << " ***** " << getDetectorType(d) << " Receiver *****";
		break;
	default:
		FILE_LOG(logERROR) << "This is an unknown receiver type " << (int)d;
		return FAIL;
	}


	//set detector specific variables
	switch(myDetectorType) {
	case GOTTHARD:		generalData = new GotthardData();	break;
	case PROPIX:		generalData = new PropixData();		break;
	case MOENCH:		generalData = new Moench02Data();	break;
	case EIGER:			generalData = new EigerData();		break;
	case JUNGFRAUCTB:	generalData = new JCTBData();		break;
	case JUNGFRAU:		generalData = new JungfrauData();	break;
	default: break;
	}
	numThreads = generalData->threadsPerReceiver;
	fifoDepth = generalData->defaultFifoDepth;
	udpSocketBufferSize = generalData->defaultUdpSocketBufferSize;
	framesPerFile = generalData->maxFramesPerFile;

	//local network parameters
	SetLocalNetworkParameters();

	//create fifo structure
	numberofJobs = -1;
	if (SetupFifoStructure() == FAIL) {
		FILE_LOG(logERROR) << "Could not allocate memory for fifo structure";
		return FAIL;
	}

	//create threads
	for ( int i = 0; i < numThreads; ++i ) {

	    try {
	        Listener* l = new Listener(i, myDetectorType, fifo[i], &status,
	                &udpPortNum[i], eth, &numberOfFrames, &dynamicRange,
	                &udpSocketBufferSize, &actualUDPSocketBufferSize, &framesPerFile,
					&frameDiscardMode, &activated, &deactivatedPaddingEnable, &silentMode);
	        listener.push_back(l);

	        DataProcessor* p = new DataProcessor(i, myDetectorType, fifo[i], &fileFormatType,
	                fileWriteEnable, &dataStreamEnable, &gapPixelsEnable,
	                &dynamicRange, &streamingFrequency, &streamingTimerInMs,
					&framePadding, &activated, &deactivatedPaddingEnable, &silentMode,
	                rawDataReadyCallBack, rawDataModifyReadyCallBack, pRawDataReady);
	        dataProcessor.push_back(p);
	    }
	    catch (...) {
	         FILE_LOG(logERROR) << "Could not create listener/dataprocessor threads (index:" << i << ")";
	            for (std::vector<Listener*>::const_iterator it = listener.begin(); it != listener.end(); ++it)
	                delete(*it);
	            listener.clear();
	            for (std::vector<DataProcessor*>::const_iterator it = dataProcessor.begin(); it != dataProcessor.end(); ++it)
	                delete(*it);
	            dataProcessor.clear();
	            return FAIL;
	    }
	}

	//set up writer and callbacks
	for (std::vector<Listener*>::const_iterator it = listener.begin(); it != listener.end(); ++it)
		(*it)->SetGeneralData(generalData);
	for (std::vector<DataProcessor*>::const_iterator it = dataProcessor.begin(); it != dataProcessor.end(); ++it)
		(*it)->SetGeneralData(generalData);

	SetThreadPriorities();

    // check udp socket buffer size
    setUDPSocketBufferSize(udpSocketBufferSize);

	FILE_LOG(logDEBUG) << " Detector type set to " << getDetectorType(d);
	return OK;
}




void slsReceiverImplementation::setDetectorPositionId(const int i){
	detID = i;
	FILE_LOG(logINFO) << "Detector Position Id:" << detID;
	for (unsigned int i = 0; i < dataProcessor.size(); ++i) {
		dataProcessor[i]->SetupFileWriter(fileWriteEnable, (int*)numDet,
				&framesPerFile,	fileName, filePath, &fileIndex,	&overwriteEnable,
				&detID,	&numThreads, &numberOfFrames, &dynamicRange, &udpPortNum[i],
				generalData);
	}

	for (unsigned int i = 0; i < listener.size(); ++i) {
		uint16_t row = 0, col = 0;
		row = detID % numDet[1]; // row
		col = (detID / numDet[1])  * ((myDetectorType == EIGER) ? 2 : 1) + i; // col for horiz. udp ports
		listener[i]->SetHardCodedPosition(row, col);
	}
}


/***acquisition functions***/
void slsReceiverImplementation::resetAcquisitionCount() {
	for (std::vector<Listener*>::const_iterator it = listener.begin(); it != listener.end(); ++it)
		(*it)->ResetParametersforNewAcquisition();

	for (std::vector<DataProcessor*>::const_iterator it = dataProcessor.begin(); it != dataProcessor.end(); ++it)
		(*it)->ResetParametersforNewAcquisition();

	for (std::vector<DataStreamer*>::const_iterator it = dataStreamer.begin(); it != dataStreamer.end(); ++it)
		(*it)->ResetParametersforNewAcquisition();

	FILE_LOG(logINFO) << "Acquisition Count has been reset";
}



int slsReceiverImplementation::startReceiver(char *c) {
	cprintf(RESET,"\n");
	FILE_LOG(logINFO) << "Starting Receiver";
	ResetParametersforNewMeasurement();

	//listener
	if (CreateUDPSockets() == FAIL) {
		strcpy(c,"Could not create UDP Socket(s).");
		FILE_LOG(logERROR) << c;
		return FAIL;
	}

	//callbacks
	if (startAcquisitionCallBack) {
		startAcquisitionCallBack(filePath, fileName, fileIndex,
				(generalData->imageSize) * numberofJobs + (generalData->fifoBufferHeaderSize), pStartAcquisition);
		if (rawDataReadyCallBack != NULL) {
			FILE_LOG(logINFO) << "Data Write has been defined externally";
		}
	}

	//processor->writer
	if (fileWriteEnable) {
		if (SetupWriter() == FAIL) {
			strcpy(c,"Could not create file.\n");
			FILE_LOG(logERROR) << c;
			return FAIL;
		}
	} else
		FILE_LOG(logINFO) << "File Write Disabled";

	FILE_LOG(logINFO) << "Ready ...";

	//status
	status = RUNNING;

	//Let Threads continue to be ready for acquisition
	StartRunning();

	FILE_LOG(logINFO)  << "Receiver Started";
	FILE_LOG(logINFO)  << "Status: " << runStatusType(status);
	return OK;
}



void slsReceiverImplementation::stopReceiver(){
	FILE_LOG(logINFO)  << "Stopping Receiver";

	//set status to transmitting
	startReadout();

	//wait for the processes (Listener and DataProcessor) to be done
	bool running = true;
	while(running) {
	    running = false;
	    for (std::vector<Listener*>::const_iterator it = listener.begin(); it != listener.end(); ++it)
	        if ((*it)->IsRunning())
	            running = true;
	    for (std::vector<DataProcessor*>::const_iterator it = dataProcessor.begin(); it != dataProcessor.end(); ++it)
            if ((*it)->IsRunning())
                running = true;
	    usleep(5000);
	}


	//create virtual file
	if (fileWriteEnable && fileFormatType == HDF5) {
		uint64_t maxIndexCaught = 0;
		bool anycaught = false;
		for (std::vector<DataProcessor*>::const_iterator it = dataProcessor.begin(); it != dataProcessor.end(); ++it) {
			maxIndexCaught = std::max(maxIndexCaught, (*it)->GetProcessedMeasurementIndex());
			if((*it)->GetMeasurementStartedFlag())
				anycaught = true;
		}
		//to create virtual file & set files/acquisition to 0 (only hdf5 at the moment)
		dataProcessor[0]->EndofAcquisition(anycaught, maxIndexCaught);
	}

	//wait for the processes (DataStreamer) to be done
	running = true;
    while(running) {
        running = false;
        for (std::vector<DataStreamer*>::const_iterator it = dataStreamer.begin(); it != dataStreamer.end(); ++it)
            if ((*it)->IsRunning())
                running = true;
        usleep(5000);
    }

	status = RUN_FINISHED;
	FILE_LOG(logINFO)  << "Status: " << runStatusType(status);


	{	//statistics
		uint64_t tot = 0;
		for (int i = 0; i < numThreads; i++) {
			tot += dataProcessor[i]->GetNumFramesCaught();

			uint64_t missingpackets = numberOfFrames*generalData->packetsPerFrame-listener[i]->GetPacketsCaught();
			if ((int)missingpackets > 0) {
				cprintf(RED, "\n[Port %d]\n",udpPortNum[i]);
				cprintf(RED, "Missing Packets\t\t: %lld\n",(long long int)missingpackets);
				cprintf(RED, "Complete Frames\t\t: %lld\n",(long long int)dataProcessor[i]->GetNumFramesCaught());
				cprintf(RED, "Last Frame Caught\t: %lld\n",(long long int)listener[i]->GetLastFrameIndexCaught());
			}else{
				cprintf(GREEN, "\n[Port %d]\n",udpPortNum[i]);
				cprintf(GREEN, "Missing Packets\t\t: %lld\n",(long long int)missingpackets);
				cprintf(GREEN, "Complete Frames\t\t: %lld\n",(long long int)dataProcessor[i]->GetNumFramesCaught());
				cprintf(GREEN, "Last Frame Caught\t: %lld\n",(long long int)listener[i]->GetLastFrameIndexCaught());
			}
		}
		if(!activated)
			cprintf(RED,"Note: Deactivated Receiver\n");
		//callback
		if (acquisitionFinishedCallBack)
			acquisitionFinishedCallBack((tot/numThreads), pAcquisitionFinished);
	}

	//change status
	status = IDLE;

	FILE_LOG(logINFO)  << "Receiver Stopped";
	FILE_LOG(logINFO)  << "Status: " << runStatusType(status);
}



void slsReceiverImplementation::startReadout(){
	if(status == RUNNING){

		// wait for incoming delayed packets
		//current packets caught
		volatile int totalP = 0,prev=-1;
		for (std::vector<Listener*>::const_iterator it = listener.begin(); it != listener.end(); ++it)
			totalP += (*it)->GetPacketsCaught();

		//wait for all packets
		if((unsigned long long int)totalP!=numberOfFrames*generalData->packetsPerFrame*listener.size()){

			//wait as long as there is change from prev totalP,
			while(prev != totalP){
#ifdef VERY_VERBOSE
				cprintf(MAGENTA,"waiting for all packets prevP:%d totalP:%d\n",
						prev,totalP);

#endif
				//usleep(1*1000*1000);usleep(1*1000*1000);usleep(1*1000*1000);usleep(1*1000*1000);
				usleep(5*1000);/* Need to find optimal time **/

				prev = totalP;
				totalP = 0;

				for (std::vector<Listener*>::const_iterator it = listener.begin(); it != listener.end(); ++it)
					totalP += (*it)->GetPacketsCaught();
#ifdef VERY_VERBOSE
				cprintf(MAGENTA,"\tupdated:  totalP:%d\n",totalP);
#endif
			}
		}


		//set status
		status = TRANSMITTING;
		FILE_LOG(logINFO) << "Status: Transmitting";
	}
	//shut down udp sockets so as to make listeners push dummy (end) packets for processors
	shutDownUDPSockets();
}


void slsReceiverImplementation::shutDownUDPSockets() {
	for (std::vector<Listener*>::const_iterator it = listener.begin(); it != listener.end(); ++it)
		(*it)->ShutDownUDPSocket();
}



void slsReceiverImplementation::closeFiles() {
	uint64_t maxIndexCaught = 0;
	bool anycaught = false;
	for (std::vector<DataProcessor*>::const_iterator it = dataProcessor.begin(); it != dataProcessor.end(); ++it) {
		(*it)->CloseFiles();
		maxIndexCaught = std::max(maxIndexCaught, (*it)->GetProcessedMeasurementIndex());
		if((*it)->GetMeasurementStartedFlag())
			anycaught = true;
	}
	//to create virtual file & set files/acquisition to 0 (only hdf5 at the moment)
	dataProcessor[0]->EndofAcquisition(anycaught, maxIndexCaught);
}


int slsReceiverImplementation::restreamStop() {
	bool ret = OK;
	for (std::vector<DataStreamer*>::const_iterator it = dataStreamer.begin(); it != dataStreamer.end(); ++it) {
		if ((*it)->RestreamStop() == FAIL)
			ret = FAIL;
	}

	// if fail, prints in datastreamer
	if (ret == OK) {
		FILE_LOG(logINFO) << "Restreaming Dummy Header via ZMQ successful";
	}

	return ret;
}



/***callback functions***/
void slsReceiverImplementation::registerCallBackStartAcquisition(int (*func)(char*, char*, uint64_t, uint32_t, void*),void *arg){
	startAcquisitionCallBack=func;
	pStartAcquisition=arg;
}

void slsReceiverImplementation::registerCallBackAcquisitionFinished(void (*func)(uint64_t, void*),void *arg){
	acquisitionFinishedCallBack=func;
	pAcquisitionFinished=arg;
}

void slsReceiverImplementation::registerCallBackRawDataReady(void (*func)(char* ,
		char*, uint32_t, void*),void *arg){
	rawDataReadyCallBack=func;
	pRawDataReady=arg;
}

void slsReceiverImplementation::registerCallBackRawDataModifyReady(void (*func)(char* ,
        char*, uint32_t&, void*),void *arg){
    rawDataModifyReadyCallBack=func;
    pRawDataReady=arg;
}


void slsReceiverImplementation::SetLocalNetworkParameters() {

	// to increase Max length of input packet queue
	int max_back_log;
	const char *proc_file_name = "/proc/sys/net/core/netdev_max_backlog";
	{
	    std::ifstream proc_file(proc_file_name);
	    proc_file >> max_back_log;
	}

	if (max_back_log < MAX_SOCKET_INPUT_PACKET_QUEUE) {
	    std::ofstream proc_file(proc_file_name);
	    if (proc_file.good()) {
	        proc_file << MAX_SOCKET_INPUT_PACKET_QUEUE << std::endl;
	        cprintf(GREEN, "Max length of input packet queue "
	                "[/proc/sys/net/core/netdev_max_backlog] modified to %d\n",
	                 MAX_SOCKET_INPUT_PACKET_QUEUE);
	    } else {
	        const char *msg = "Could not change max length of "
	                "input packet queue [net.core.netdev_max_backlog]. (No Root Privileges?)";
	        FILE_LOG(logWARNING) << msg;
	    }
	}
}



void slsReceiverImplementation::SetThreadPriorities() {

	for (std::vector<Listener*>::const_iterator it = listener.begin(); it != listener.end(); ++it){
		if ((*it)->SetThreadPriority(LISTENER_PRIORITY) == FAIL) {
			FILE_LOG(logWARNING) << "Could not prioritize listener threads. (No Root Privileges?)";
			return;
		}
	}
	std::ostringstream osfn;
	osfn << "Priorities set - "
			"Listener:" << LISTENER_PRIORITY;

	FILE_LOG(logINFO) << osfn.str();
}


int slsReceiverImplementation::SetupFifoStructure() {
		numberofJobs = 1;


	for (std::vector<Fifo*>::const_iterator it = fifo.begin(); it != fifo.end(); ++it)
		delete(*it);
	fifo.clear();
	for ( int i = 0; i < numThreads; ++i ) {

		//create fifo structure
	    try {
	        Fifo* f = new Fifo (i,
	                (generalData->imageSize) * numberofJobs + (generalData->fifoBufferHeaderSize),
	                fifoDepth);
	        fifo.push_back(f);
	    } catch (...) {
            cprintf(RED,"Error: Could not allocate memory for fifo structure of index %d\n", i);
            for (std::vector<Fifo*>::const_iterator it = fifo.begin(); it != fifo.end(); ++it)
                delete(*it);
            fifo.clear();
            return FAIL;
	    }
		//set the listener & dataprocessor threads to point to the right fifo
		if(listener.size())listener[i]->SetFifo(fifo[i]);
		if(dataProcessor.size())dataProcessor[i]->SetFifo(fifo[i]);
		if(dataStreamer.size())dataStreamer[i]->SetFifo(fifo[i]);
	}

	FILE_LOG(logINFO) << "Memory Allocated Per Fifo: " << ( ((generalData->imageSize) * numberofJobs + (generalData->fifoBufferHeaderSize)) * fifoDepth) << " bytes" ;
	FILE_LOG(logINFO) << numThreads << " Fifo structure(s) reconstructed";
	return OK;
}



void slsReceiverImplementation::ResetParametersforNewMeasurement() {
	for (std::vector<Listener*>::const_iterator it = listener.begin(); it != listener.end(); ++it)
		(*it)->ResetParametersforNewMeasurement();
	for (std::vector<DataProcessor*>::const_iterator it = dataProcessor.begin(); it != dataProcessor.end(); ++it)
		(*it)->ResetParametersforNewMeasurement();

	if (dataStreamEnable) {
		char fnametostream[MAX_STR_LENGTH];
		snprintf(fnametostream, MAX_STR_LENGTH, "%s/%s", filePath, fileName);
		for (std::vector<DataStreamer*>::const_iterator it = dataStreamer.begin(); it != dataStreamer.end(); ++it)
			(*it)->ResetParametersforNewMeasurement(fnametostream);
	}
}



int slsReceiverImplementation::CreateUDPSockets() {
	bool error = false;
	for (unsigned int i = 0; i < listener.size(); ++i)
		if (listener[i]->CreateUDPSockets() == FAIL) {
			error = true;
			break;
		}
	if (error) {
		shutDownUDPSockets();
		return FAIL;
	}

	FILE_LOG(logDEBUG) << "UDP socket(s) created successfully.";
	return OK;
}


int slsReceiverImplementation::SetupWriter() {
	bool error = false;
	for (unsigned int i = 0; i < dataProcessor.size(); ++i)
		if (dataProcessor[i]->CreateNewFile(tengigaEnable,
				numberOfFrames, acquisitionTime, subExpTime, subPeriod, acquisitionPeriod) == FAIL) {
			error = true;
			break;
		}
	if (error) {
		shutDownUDPSockets();
		closeFiles();
		return FAIL;
	}

	return OK;
}


void slsReceiverImplementation::StartRunning() {
	//set running mask and post semaphore to start the inner loop in execution thread
	for (std::vector<Listener*>::const_iterator it = listener.begin(); it != listener.end(); ++it) {
		(*it)->StartRunning();
		(*it)->Continue();
	}
	for (std::vector<DataProcessor*>::const_iterator it = dataProcessor.begin(); it != dataProcessor.end(); ++it){
		(*it)->StartRunning();
		(*it)->Continue();
	}
	for (std::vector<DataStreamer*>::const_iterator it = dataStreamer.begin(); it != dataStreamer.end(); ++it){
		(*it)->StartRunning();
		(*it)->Continue();
	}
}
