/********************************************//**
 * @file UDPStandardImplementation.cpp
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 ***********************************************/

sprintf(cstreambuf, "%s", " \0");
FILE_LOG(logDEBUG, cstreambuf);

#include "UDPStandardImplementation.h"

#include "moench02ModuleData.h"
#include "gotthardModuleData.h"
#include "gotthardShortModuleData.h"

#include <stdlib.h>			// exit()
#include <iomanip>			//set precision for printing parameters for create new file
#include <map>				//map
#include <iostream>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include <zmq.h>	//zmq
#include <sstream>
#include <inttypes.h>		//printf of uint64_t etc

using namespace std;

#define WRITE_HEADERS

/*************************************************************************
 * Constructor & Destructor **********************************************
 * They access local cache of configuration or detector parameters *******
 *************************************************************************/

UDPStandardImplementation::UDPStandardImplementation(){


	initializeMembers();

	for(int i= 0; i < MAX_NUMBER_OF_WRITER_THREADS; ++i)
		memset(streambuf[i], 0, MAX_STR_LENGTH );
	memset(cstreambuf, 0, MAX_STR_LENGTH );

	//***mutex***
	pthread_mutex_init(&statusMutex,NULL);
	pthread_mutex_init(&writeMutex,NULL);
	pthread_mutex_init(&dataReadyMutex,NULL);
	pthread_mutex_init(&progressMutex,NULL);

	//to increase socket receiver buffer size and max length of input queue by changing kernel settings
	if(myDetectorType == EIGER);
	else if(system("echo $((100*1024*1024)) > /proc/sys/net/core/rmem_max")){
		sprintf(cstreambuf, "%s", "Warning: No root permission to change socket receiver buffer size in file /proc/sys/net/core/rmem_max \0");
		FILE_LOG(logDEBUG, cstreambuf);
	}else if(system("echo 250000 > /proc/sys/net/core/netdev_max_backlog")){
		sprintf(cstreambuf, "%s", "Warning: No root permission to change max length of input queue in file /proc/sys/net/core/netdev_max_backlog \0");
		FILE_LOG(logDEBUG, cstreambuf);
	}

	/** permanent setting by heiner
	    net.core.rmem_max = 104857600 # 100MiB
	    net.core.netdev_max_backlog = 250000
	    sysctl -p
	    // from the manual
	    sysctl -w net.core.rmem_max=16777216
	    sysctl -w net.core.netdev_max_backlog=250000
	 */
}

UDPStandardImplementation::~UDPStandardImplementation(){

	for(int i=0;i<MAX_NUMBER_OF_WRITER_THREADS; i++)
		closeFile(i);
	deleteMembers();
}


/*************************************************************************
 * Setters ***************************************************************
 * They modify the local cache of configuration or detector parameters ***
 *************************************************************************/

/***initial parameters***/

void UDPStandardImplementation::deleteMembers(){
	sprintf(cstreambuf, "%s", "Deleting member pointers \0");
	FILE_LOG(logDEBUG, cstreambuf);

	shutDownUDPSockets();
	for(int i=0;i<MAX_NUMBER_OF_WRITER_THREADS; i++)
		closeFile(i);
	//filter
	deleteFilter();
	for(int i=0; i<MAX_NUMBER_OF_LISTENING_THREADS; i++){
		if(mem0[i])			{free(mem0[i]);			mem0[i] = 0;}
		if(fifo[i])			{delete fifo[i];		fifo[i] = 0;}
		if(fifoFree[i])		{delete fifoFree[i];	fifoFree[i] = 0;}
	}
	for(int i=0; i<MAX_NUMBER_OF_WRITER_THREADS; i++){
		if(latestData[i]) 	{delete[] latestData[i]; latestData[i] = 0;}
	}
	//kill threads
	if(threadStarted){
		createListeningThreads(true);
		createWriterThreads(true);
	}
	if(zmqThreadStarted)
		createDataCallbackThreads(true);
}

void UDPStandardImplementation::deleteFilter(){


	moenchCommonModeSubtraction = 0;
	for(int i=0; i<MAX_NUMBER_OF_WRITER_THREADS; i++){
		if(singlePhotonDetectorObject[i]){
			delete []singlePhotonDetectorObject[i];
			singlePhotonDetectorObject[i] = 0;
		}
		if(receiverData[i]){
			delete []receiverData[i];
			receiverData[i] = 0;
		}
	}
}

void UDPStandardImplementation::initializeBaseMembers(){


	UDPBaseImplementation::initializeMembers();
	acquisitionPeriod = SAMPLE_TIME_IN_NS;
}


void UDPStandardImplementation::initializeMembers(){

	sprintf(cstreambuf, "%s", "Initializing members \0");
	FILE_LOG(logDEBUG, cstreambuf);

	//***detector parameters***
	detID = 0;
	bufferSize = 0;
	onePacketSize = 0;
	oneDataSize = 0;
	frameIndexMask = 0;
	frameIndexOffset = 0;
	packetIndexMask = 0;
	footerOffset = 0;
	excludeMissingPackets = false;

	//***file parameters***
#ifdef MYROOT1
	for(int i=0; i<MAX_NUMBER_OF_WRITER_THREADS; i++){
		myTree[i] = (0);
		myFile[i] = (0);
	}
#endif
	for(int i=0; i<MAX_NUMBER_OF_WRITER_THREADS; i++){
		strcpy(completeFileName[i],"");
		strcpy(fileNamePerThread[i],"");
		strcpy(fileHeader[i],"");
		sfilefd[i] = 0;
	}
	maxFramesPerFile = 0;
	fileCreateSuccess = false;

	//***acquisition indices parameters***
	startAcquisitionIndex = 0;
	startFrameIndex = 0;
	for(int i=0; i<MAX_NUMBER_OF_WRITER_THREADS; i++){
		frameIndex[i] = 0;
		currentFrameNumber[i] = 0;
		frameNumberInPreviousFile[i] = 0;
		frameNumberInPreviousCheck[i] = 0;
		totalWritingPacketCountFromLastCheck[i] = 0;
		lastFrameNumberInFile[i] = -1;
		totalPacketsInFile[i] = 0;
		totalWritingPacketCount[i] = 0;
	}
	acqStarted = false;
	for(int i = 0; i < MAX_NUMBER_OF_LISTENING_THREADS; ++i){
		measurementStarted[i] = false;
		totalListeningPacketCount[i] = 0;
		totalIgnoredPacketCount[i] = 0;
	}


	//***receiver parameters***
	for(int i=0; i < MAX_NUMBER_OF_LISTENING_THREADS; i++){
		buffer[i] = 0;
		mem0[i] = 0;
		fifo[i] = 0;
		fifoFree[i] = 0;
		udpSocket[i] = 0;
	}
	numberofJobsPerBuffer = -1;
	fifoSize = 0;
	fifoBufferHeaderSize = 0;

	//***receiver to GUI parameters***
	for(int i=0; i<MAX_NUMBER_OF_WRITER_THREADS; i++){
		latestData[i] = 0;
		strcpy(guiFileName[i],"");
		guiNumPackets[i] = 0;
		frametoGuiCounter[i] = 0;
	}

	//***data callback thread parameters***
	zmqThreadStarted = false;
	numberofDataCallbackThreads = 1;
	dataCallbackThreadsMask = 0x0;
	killAllDataCallbackThreads = false;
	dataStreamEnable = false;


	//***general and listening thread parameters***
	threadStarted = false;
	currentThreadIndex = -1;
	numberofListeningThreads = 1;
	listeningThreadsMask = 0x0;
	killAllListeningThreads = false;

	//***writer thread parameters***
	numberofWriterThreads = 1;
	writerThreadsMask = 0x0;
	createFileMask = 0x0;
	killAllWritingThreads = false;

	//***filter parameters***
	commonModeSubtractionEnable = false;
	moenchCommonModeSubtraction = 0;
	for(int i=0; i<MAX_NUMBER_OF_WRITER_THREADS; i++){
		singlePhotonDetectorObject[i] = 0;
		receiverData[i] = 0;
	}

}



void UDPStandardImplementation::initializeFilter(){


	double hc = 0, sigma = 5;
	int sign = 1, csize, i;

	//common mode initialization
	moenchCommonModeSubtraction = 0;
	if (commonModeSubtractionEnable){
		if(myDetectorType == MOENCH)
			moenchCommonModeSubtraction=new moenchCommonMode();
		else
			cout << "Warning: No common mode subtraction for this detector" << endl;
	}

	//receiver data initialization
	switch(myDetectorType){
	case MOENCH:
		csize = 3;
		for(i=0; i<numberofWriterThreads; i++)
			receiverData[i]=new moench02ModuleData(hc);
		break;
	default:
		csize = 1;
		if(shortFrameEnable == -1){
			for(i=0; i<numberofWriterThreads; i++)
				receiverData[i]=new gotthardModuleData(hc);
		}else{
			for(i=0; i<numberofWriterThreads; i++)
				receiverData[i]=new gotthardShortModuleData(hc);
		}
		break;
	}

	//single photon detector initialization
	for(i=0; i<numberofWriterThreads; i++)
		singlePhotonDetectorObject[i]=new singlePhotonDetector<uint16_t>(receiverData[i], csize, sigma, sign, moenchCommonModeSubtraction);
}




int UDPStandardImplementation::setupFifoStructure(){



	//number of jobs per buffer
	int64_t i;
	int oldNumberofJobsPerBuffer = numberofJobsPerBuffer;
	//eiger always listens to 1 packet at a time
	if(excludeMissingPackets){
		numberofJobsPerBuffer = 1;
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os << "Info: 1 packet per buffer";
			string message(os.str());	FILE_LOG(logDEBUG, message);
		}

	}
	//else calculate best possible number of frames to listen to at a time (for fast readouts like gotthard)
	else{
		//if frequency to gui is not random (every nth frame), then listen to only n frames per buffer
		if(frameToGuiFrequency)
			numberofJobsPerBuffer = frameToGuiFrequency;
		//random frame sent to gui, then frames per buffer depends on acquisition period
		else{
			//calculate 100ms/period to get frames to listen to at a time
			if(acquisitionPeriod)
				i = SAMPLE_TIME_IN_NS/acquisitionPeriod;
			else{
				if(acquisitionTime)
					i = SAMPLE_TIME_IN_NS/acquisitionTime;
				else
					i = SAMPLE_TIME_IN_NS;
			}
			//max frames to listen to at a time is limited by 1000
			if (i > MAX_JOBS_PER_THREAD)
				numberofJobsPerBuffer = MAX_JOBS_PER_THREAD;
			else if (i < 1)
				numberofJobsPerBuffer = 1;
			else
				numberofJobsPerBuffer = i;

		}
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os << "Number of Frames per buffer:" << numberofJobsPerBuffer << endl;
			string message(os.str());	FILE_LOG(logINFO, message);
		}

	}



	// fifo depth
	uint32_t oldFifoSize = fifoSize;


	//reduce fifo depth if > 1 numberofJobsPerBuffer
	if(fifoDepth % numberofJobsPerBuffer)
		fifoSize = (fifoDepth/numberofJobsPerBuffer)+1;
	else
		fifoSize = fifoDepth/numberofJobsPerBuffer;

	//do not rebuild fifo structure if it is the same (oldfifosize differs only for different packetsperframe)
	if((oldNumberofJobsPerBuffer == numberofJobsPerBuffer) && (oldFifoSize == fifoSize))
		return OK;
	sprintf(cstreambuf, "%s", " \0");
	FILE_LOG(logDEBUG, cstreambuf);
	{
		ostringstream os;
		os << "Info: Total Fifo Size:" << fifoSize;
		string message(os.str());	FILE_LOG(logINFO, message);
	}




	//delete threads
	if(threadStarted){
		createListeningThreads(true);
		createWriterThreads(true);
	}


	//set up fifo structure
	for(int i=0;i<MAX_NUMBER_OF_LISTENING_THREADS;i++){

		//deleting
		if(fifoFree[i]){
			while(!fifoFree[i]->isEmpty()){
				fifoFree[i]->pop(buffer[i]);
				//cprintf(BLUE,"FifoFree[%d]: value:%d, pop 0x%x\n",i,fifoFree[i]->getSemValue(),(void*)(buffer[i]));
			}
			delete fifoFree[i];
			fifoFree[i] = 0;
		}
		if(fifo[i]){
			while(!fifo[i]->isEmpty()){
				fifo[i]->pop(buffer[i]);
				//cprintf(CYAN,"Fifo[%d]: value:%d, pop 0x%x\n",i,fifo[i]->getSemValue(),(void*)(buffer[i]));
			}
			delete fifo[i];
			fifo[i] = 0;
		}
		if(mem0[i]){
			free(mem0[i]);
			mem0[i] = 0;
		}

	}


	for(int i=0;i<numberofListeningThreads;i++){
		//creating
		fifoFree[i] 	= new CircularFifo<char>(fifoSize);
		fifo[i] 		= new CircularFifo<char>(fifoSize);

		//allocate memory
		mem0[i] = (char*)calloc((bufferSize * numberofJobsPerBuffer + fifoBufferHeaderSize) * fifoSize,sizeof(char));
		if (mem0[i] == NULL){
			cprintf(BG_RED,"Error: Could not allocate memory for listening \n");
			return FAIL;
		}

		//push free address into fifoFree
		buffer[i]=mem0[i];
		while (buffer[i] < (mem0[i]+(bufferSize * numberofJobsPerBuffer + fifoBufferHeaderSize) * (fifoSize-1))) {
			//cprintf(BLUE,"fifofree %d: push 0x%p\n",i,(void*)buffer[i]);
			/*for(int k=0;k<bufferSize;k=k+10){
				sprintf(buffer[i]+fifoBufferHeaderSize+k,"mem%d",i);
			}*/
			sprintf(buffer[i],"mem%d",i);
			while(!fifoFree[i]->push(buffer[i]));
			//cprintf(GREEN,"Fifofree[%d]: value:%d, push 0x%x\n",i,fifoFree[i]->getSemValue(),(void*)(buffer[i]));
#ifdef DEBUG5
			cprintf(BLUE,"Info: %d fifostructure free pushed into fifofree %p\n", i, (void*)(buffer[i]));
#endif
			buffer[i] += (bufferSize * numberofJobsPerBuffer + fifoBufferHeaderSize);
		}
	}
	cout << "Fifo structure(s) reconstructed" << endl;

	//create threads
	if(createListeningThreads() == FAIL){
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		ostringstream os;
		os << "Could not create listening thread";
		string message(os.str());	FILE_LOG(logERROR, message);
		return FAIL;
	}
	if(createWriterThreads() == FAIL){
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		ostringstream os;
		os << "Could not create writer threads";
		string message(os.str());	FILE_LOG(logERROR, message);
		return FAIL;
	}
	setThreadPriorities();

	return OK;
}








void UDPStandardImplementation::setFileName(const char c[]){


	char oldfilename[MAX_STR_LENGTH];
	strcpy(oldfilename,fileName);

	if(strlen(c))
		strcpy(fileName, c);

	if(strlen(fileName)){
		int detindex = -1;
		string tempname(fileName);
		size_t uscore=tempname.rfind("_");
		if (uscore!=string::npos){
			if (sscanf(tempname.substr(uscore+1,tempname.size()-uscore-1).c_str(),"d%d",&detindex)) {
				detID = detindex;
			}
		}
		if(detindex == -1)
			detID = 0;
	}
	sprintf(cstreambuf, "%s", " \0");
	FILE_LOG(logDEBUG, cstreambuf);

	{
		ostringstream os;
		os << "File name:" << fileName;
		string message(os.str());	FILE_LOG(logINFO, message);
	}
}

int UDPStandardImplementation::setDataCompressionEnable(const bool b){


	if(myDetectorType != EIGER){
		cout << "Info: Setting up Data Compression Enable to " << stringEnable(b);
#ifdef MYROOT1
		cout << " WITH ROOT" << endl;
#else
		cout << " WITHOUT ROOT" << endl;
#endif
	}

	//set data compression enable
	dataCompressionEnable = b;

	//-- create writer threads depending on enable
	pthread_mutex_lock(&statusMutex);
	writerThreadsMask = 0x0;
	pthread_mutex_unlock(&(statusMutex));

	createWriterThreads(true);
	if(b)
		numberofWriterThreads = MAX_NUMBER_OF_WRITER_THREADS;
	else
		numberofWriterThreads = 1;
	if(createWriterThreads() == FAIL){
		cprintf(BG_RED,"Error: Could not create writer threads\n");
		return FAIL;
	}
	//-- end of create writer threads
	setThreadPriorities();

	//filter
	deleteFilter();
	if(b)
		initializeFilter();
	sprintf(cstreambuf, "%s", " \0");
	FILE_LOG(logDEBUG, cstreambuf);
	{
		ostringstream os;
		os << "Data Compression: " << stringEnable(dataCompressionEnable);
		string message(os.str());	FILE_LOG(logINFO, message);
	}

	return OK;
}

/***acquisition count parameters***/
uint64_t UDPStandardImplementation::getTotalFramesCaught() const{


	return (totalPacketsCaught/(packetsPerFrame*numberofListeningThreads));
}

uint64_t UDPStandardImplementation::getFramesCaught() const{

	return (packetsCaught/(packetsPerFrame*numberofListeningThreads));
}

/***acquisition parameters***/
void UDPStandardImplementation::setShortFrameEnable(const int i){


	shortFrameEnable = i;

	if(shortFrameEnable!=-1){
		bufferSize = GOTTHARD_SHORT_BUFFER_SIZE;
		onePacketSize = GOTTHARD_SHORT_BUFFER_SIZE;
		oneDataSize = GOTTHARD_SHORT_DATABYTES;
		maxFramesPerFile = SHORT_MAX_FRAMES_PER_FILE;
		packetsPerFrame = GOTTHARD_SHORT_PACKETS_PER_FRAME;
		frameIndexMask = GOTTHARD_SHORT_FRAME_INDEX_MASK;
		frameIndexOffset = GOTTHARD_SHORT_FRAME_INDEX_OFFSET;
		packetIndexMask = GOTTHARD_SHORT_PACKET_INDEX_MASK;

	}else{
		bufferSize = GOTTHARD_BUFFER_SIZE;
		onePacketSize = GOTTHARD_ONE_PACKET_SIZE;
		oneDataSize = GOTTHARD_ONE_DATA_SIZE;
		maxFramesPerFile = MAX_FRAMES_PER_FILE;
		packetsPerFrame = GOTTHARD_PACKETS_PER_FRAME;
		frameIndexMask = GOTTHARD_FRAME_INDEX_MASK;
		frameIndexOffset = GOTTHARD_FRAME_INDEX_OFFSET;
		packetIndexMask = GOTTHARD_PACKET_INDEX_MASK;
	}

	//filter
	deleteFilter();
	if(dataCompressionEnable)
		initializeFilter();
	sprintf(cstreambuf, "%s", " \0");
	FILE_LOG(logDEBUG, cstreambuf);
	{
		ostringstream os;
		os << "Short Frame Enable: " << shortFrameEnable;
		string message(os.str());	FILE_LOG(logINFO, message);
	}
}


int UDPStandardImplementation::setFrameToGuiFrequency(const uint32_t freq){


	frameToGuiFrequency = freq;
	if(setupFifoStructure() == FAIL)
		return FAIL;
	sprintf(cstreambuf, "%s", " \0");
	FILE_LOG(logDEBUG, cstreambuf);
	{
		ostringstream os;
		os << "Frame to Gui Frequency: " << frameToGuiFrequency;
		string message(os.str());	FILE_LOG(logINFO, message);
	}

	return OK;
}



uint32_t UDPStandardImplementation::setDataStreamEnable(const uint32_t enable){


	int oldvalue = dataStreamEnable;
	dataStreamEnable = enable;
	sprintf(cstreambuf, "%s", " \0");
	FILE_LOG(logDEBUG, cstreambuf);
	{
		ostringstream os;
		os << "Data Send to Gui: " << dataStreamEnable;
		string message(os.str());	FILE_LOG(logINFO, message);
	}


	if(oldvalue!=dataStreamEnable){
		//data sockets have to be created again as the client ones are
		if(zmqThreadStarted)
			createDataCallbackThreads(true);

		if(dataStreamEnable){
			numberofDataCallbackThreads = numberofListeningThreads;
			if(createDataCallbackThreads() == FAIL){
				cprintf(BG_RED,"Error: Could not create data callback threads\n");
			}
		}
	}

	return OK;
}



int UDPStandardImplementation::setAcquisitionPeriod(const uint64_t i){


	acquisitionPeriod = i;
	if(setupFifoStructure() == FAIL)
		return FAIL;
	sprintf(cstreambuf, "%s", " \0");
	FILE_LOG(logDEBUG, cstreambuf);
	{
		ostringstream os;
		os << "Acquisition Period: " << (double)acquisitionPeriod/(1E9) << "s";
		string message(os.str());	FILE_LOG(logINFO, message);
	}

	if(myDetectorType == EIGER)
		for(int i=0; i<MAX_NUMBER_OF_WRITER_THREADS; i++)
			updateFileHeader(i);

	return OK;
}


int UDPStandardImplementation::setAcquisitionTime(const uint64_t i){


	acquisitionTime = i;
	if(setupFifoStructure() == FAIL)
		return FAIL;
	sprintf(cstreambuf, "%s", " \0");
	FILE_LOG(logDEBUG, cstreambuf);
	{
		ostringstream os;
		os << "Acquisition Period: " << (double)acquisitionTime/(1E9) << "s";
		string message(os.str());	FILE_LOG(logINFO, message);
	}

	if(myDetectorType == EIGER)
		for(int i=0; i<MAX_NUMBER_OF_WRITER_THREADS; i++)
			updateFileHeader(i);

	return OK;
}


int UDPStandardImplementation::setNumberOfFrames(const uint64_t i){


	numberOfFrames = i;
	if(setupFifoStructure() == FAIL)
		return FAIL;
	sprintf(cstreambuf, "%s", " \0");
	FILE_LOG(logDEBUG, cstreambuf);
	{
		ostringstream os;
		os << "Number of Frames:" << numberOfFrames;
		string message(os.str());	FILE_LOG(logINFO, message);
	}

	if(myDetectorType == EIGER)
		for(int i=0; i<MAX_NUMBER_OF_WRITER_THREADS; i++)
			updateFileHeader(i);

	return OK;
}

int UDPStandardImplementation::setDynamicRange(const uint32_t i){


	uint32_t oldDynamicRange = dynamicRange;
	sprintf(cstreambuf, "%s", " \0");
	FILE_LOG(logDEBUG, cstreambuf);
	{
		ostringstream os;
		os << "Info: Setting Dynamic Range to " << i;
		string message(os.str());	FILE_LOG(logDEBUG, message);
	}

	dynamicRange = i;

	if(myDetectorType == EIGER){

		//set parameters depending on new dynamic range.
		packetsPerFrame 	= (tengigaEnable ? EIGER_TEN_GIGA_CONSTANT : EIGER_ONE_GIGA_CONSTANT) * dynamicRange;
		bufferSize			= oneDataSize * packetsPerFrame;

		for(int i=0; i<MAX_NUMBER_OF_WRITER_THREADS; i++)
			updateFileHeader(i);

		//new dynamic range, then restart threads and resetup fifo structure
		if(oldDynamicRange != dynamicRange){

			//gui buffer
			for(int i=0;i<MAX_NUMBER_OF_WRITER_THREADS;i++){
				if(latestData[i]){delete[] latestData[i];latestData[i] = 0;}
			}
			for(int i=0;i<numberofWriterThreads;i++){
				latestData[i] = new char[bufferSize+sizeof(sls_detector_header)]();
			}
			//restructure fifo
			numberofJobsPerBuffer = -1;
			if(setupFifoStructure() == FAIL)
				return FAIL;

		}

	}
	sprintf(cstreambuf, "%s", " \0");
	FILE_LOG(logDEBUG, cstreambuf);
	{
		ostringstream os;
		os << "Dynamic Range: " << dynamicRange;
		string message(os.str());	FILE_LOG(logINFO, message);
	}

	return OK;
}



int UDPStandardImplementation::setTenGigaEnable(const bool b){

	sprintf(cstreambuf, "%s", " \0");
	FILE_LOG(logDEBUG, cstreambuf);
	{
		ostringstream os;
		os << "Info: Setting Ten Giga to " << stringEnable(b);
		string message(os.str());	FILE_LOG(logDEBUG, message);
	}

	bool oldTenGigaEnable = tengigaEnable;
	tengigaEnable = b;

	if(myDetectorType == EIGER){

		//set parameters depending on 10g
		if(tengigaEnable){
			packetsPerFrame = EIGER_TEN_GIGA_CONSTANT * dynamicRange;
			onePacketSize  	= EIGER_TEN_GIGA_ONE_PACKET_SIZE;
			oneDataSize 	= EIGER_TEN_GIGA_ONE_DATA_SIZE;
		}else{
			packetsPerFrame = EIGER_ONE_GIGA_CONSTANT * dynamicRange;
			onePacketSize  	= EIGER_ONE_GIGA_ONE_PACKET_SIZE;
			oneDataSize		= EIGER_ONE_GIGA_ONE_DATA_SIZE;
		}
		bufferSize			= oneDataSize * packetsPerFrame;
		footerOffset		= EIGER_DATA_PACKET_HEADER_SIZE + oneDataSize;
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os << dec <<
					"packetsPerFrame:" << packetsPerFrame <<
					"\nonePacketSize:" << onePacketSize <<
					"\noneDataSize:" << oneDataSize <<
					"\nbufferSize:" << bufferSize;
			string message(os.str());	FILE_LOG(logDEBUG, message);
		}


		for(int i=0; i<MAX_NUMBER_OF_WRITER_THREADS; i++)
			updateFileHeader(i);

		//new enable, then restart threads and resetup fifo structure
		if(oldTenGigaEnable != tengigaEnable){

			//gui buffer
			for(int i=0;i<MAX_NUMBER_OF_WRITER_THREADS;i++){
				if(latestData[i]){delete[] latestData[i];latestData[i] = 0;}
			}
			for(int i=0;i<numberofWriterThreads;i++){
				latestData[i] = new char[bufferSize+sizeof(sls_detector_header)]();
			}

			//restructure fifo
			if(setupFifoStructure() == FAIL)
				return FAIL;

		}

	}
	sprintf(cstreambuf, "%s", " \0");
	FILE_LOG(logDEBUG, cstreambuf);
	{
		ostringstream os;
		os << "Ten Giga: " << stringEnable(tengigaEnable);
		string message(os.str());	FILE_LOG(logINFO, message);
	}

	return OK;
}


int UDPStandardImplementation::setFifoDepth(const uint32_t i){


	if(i != fifoDepth){
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os << "Fifo Depth: " << i << endl;
			string message(os.str());	FILE_LOG(logINFO, message);
		}
		fifoDepth = i;
		return setupFifoStructure();
	}
	return OK;
}




/*************************************************************************
 * Behavioral functions***************************************************
 * They may modify the status of the receiver ****************************
 *************************************************************************/


/***initial functions***/
int UDPStandardImplementation::setDetectorType(const detectorType d){

	sprintf(cstreambuf, "%s", " \0");
	FILE_LOG(logDEBUG, cstreambuf);
	{
		ostringstream os;
		os << "Setting receiver type";
		string message(os.str());	FILE_LOG(logDEBUG, message);
	}

	deleteMembers();
	initializeBaseMembers();
	initializeMembers();

	myDetectorType = d;
	switch(myDetectorType){
	case GOTTHARD:
	case PROPIX:
	case MOENCH:
	case EIGER:
	case JUNGFRAUCTB:
	case JUNGFRAU:
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
	{
		ostringstream os;
		os << " ***** " << getDetectorType(d) << " Receiver *****";
		string message(os.str());	FILE_LOG(logINFO, message);
	}

	break;
	default:
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
	{
		ostringstream os;
		os << "This is an unknown receiver type " << (int)d;
		string message(os.str());	FILE_LOG(logERROR, message);
	}
	return FAIL;
	}

	//set detector specific variables
	switch(myDetectorType){
	case GOTTHARD:
		packetsPerFrame		= GOTTHARD_PACKETS_PER_FRAME;
		onePacketSize 		= GOTTHARD_ONE_PACKET_SIZE;
		oneDataSize 		= GOTTHARD_ONE_DATA_SIZE;
		bufferSize 			= GOTTHARD_BUFFER_SIZE;
		frameIndexMask 		= GOTTHARD_FRAME_INDEX_MASK;
		frameIndexOffset 	= GOTTHARD_FRAME_INDEX_OFFSET;
		packetIndexMask 	= GOTTHARD_PACKET_INDEX_MASK;
		maxFramesPerFile	= MAX_FRAMES_PER_FILE;
		fifoSize			= GOTTHARD_FIFO_SIZE;
		fifoDepth			= GOTTHARD_FIFO_SIZE;
		fifoBufferHeaderSize= HEADER_SIZE_NUM_TOT_PACKETS;
		//footerOffset		= Not applicable;
		break;
	case PROPIX:
		packetsPerFrame		= PROPIX_PACKETS_PER_FRAME;
		onePacketSize 		= PROPIX_ONE_PACKET_SIZE;
		//oneDataSize 		= Not applicable;
		bufferSize 			= PROPIX_BUFFER_SIZE;
		frameIndexMask 		= PROPIX_FRAME_INDEX_MASK;
		frameIndexOffset 	= PROPIX_FRAME_INDEX_OFFSET;
		packetIndexMask 	= PROPIX_PACKET_INDEX_MASK;
		maxFramesPerFile	= MAX_FRAMES_PER_FILE;
		fifoSize			= PROPIX_FIFO_SIZE;
		fifoDepth			= PROPIX_FIFO_SIZE;
		fifoBufferHeaderSize= HEADER_SIZE_NUM_TOT_PACKETS;
		//footerOffset		= Not applicable;
		break;
	case MOENCH:
		packetsPerFrame		= MOENCH_PACKETS_PER_FRAME;
		onePacketSize 		= MOENCH_ONE_PACKET_SIZE;
		oneDataSize 		= MOENCH_ONE_DATA_SIZE;
		bufferSize 			= MOENCH_BUFFER_SIZE;
		frameIndexMask 		= MOENCH_FRAME_INDEX_MASK;
		frameIndexOffset 	= MOENCH_FRAME_INDEX_OFFSET;
		packetIndexMask 	= MOENCH_PACKET_INDEX_MASK;
		maxFramesPerFile	= MOENCH_MAX_FRAMES_PER_FILE;
		fifoSize			= MOENCH_FIFO_SIZE;
		fifoDepth 			= MOENCH_FIFO_SIZE;
		fifoBufferHeaderSize= HEADER_SIZE_NUM_TOT_PACKETS;
		//footerOffset		= Not applicable;
		break;
	case EIGER:
		//assuming 1G in the beginning
		packetsPerFrame		= EIGER_ONE_GIGA_CONSTANT * dynamicRange;
		onePacketSize 		= EIGER_ONE_GIGA_ONE_PACKET_SIZE;
		oneDataSize 		= EIGER_ONE_GIGA_ONE_DATA_SIZE;
		bufferSize 			= oneDataSize * packetsPerFrame;
		frameIndexMask 		= EIGER_FRAME_INDEX_MASK;
		frameIndexOffset 	= EIGER_FRAME_INDEX_OFFSET;
		packetIndexMask 	= EIGER_PACKET_INDEX_MASK;
		maxFramesPerFile	= EIGER_MAX_FRAMES_PER_FILE;
		fifoSize			= EIGER_FIFO_SIZE;
		fifoDepth			= EIGER_FIFO_SIZE;
		footerOffset		= EIGER_DATA_PACKET_HEADER_SIZE + oneDataSize;
		fifoBufferHeaderSize= (HEADER_SIZE_NUM_TOT_PACKETS + sizeof(sls_detector_header));
		excludeMissingPackets= true;
		break;
	case JUNGFRAUCTB:
		packetsPerFrame		= JCTB_PACKETS_PER_FRAME;
		onePacketSize 		= JCTB_ONE_PACKET_SIZE;
		//oneDataSize 		= Not applicable;
		bufferSize 			= JCTB_BUFFER_SIZE;
		frameIndexMask 		= JCTB_FRAME_INDEX_MASK;
		frameIndexOffset 	= JCTB_FRAME_INDEX_OFFSET;
		packetIndexMask 	= JCTB_PACKET_INDEX_MASK;
		maxFramesPerFile	= JFCTB_MAX_FRAMES_PER_FILE;
		fifoSize			= JCTB_FIFO_SIZE;
		fifoDepth			= JCTB_FIFO_SIZE;
		fifoBufferHeaderSize= HEADER_SIZE_NUM_TOT_PACKETS;
		//footerOffset		= Not applicable;
		break;
	case JUNGFRAU:
		packetsPerFrame		= JFRAU_PACKETS_PER_FRAME;
		onePacketSize 		= JFRAU_ONE_PACKET_SIZE;
		oneDataSize 		= JFRAU_ONE_DATA_SIZE;
		bufferSize 			= oneDataSize * packetsPerFrame;
		frameIndexMask 		= JFRAU_FRAME_INDEX_MASK;
		frameIndexOffset 	= JFRAU_FRAME_INDEX_OFFSET;
		packetIndexMask 	= JFRAU_PACKET_INDEX_MASK;
		maxFramesPerFile	= JFRAU_MAX_FRAMES_PER_FILE;
		fifoDepth			= JFRAU_FIFO_SIZE;
		fifoSize			= JFRAU_FIFO_SIZE;
		fifoBufferHeaderSize= (HEADER_SIZE_NUM_TOT_PACKETS + sizeof(sls_detector_header));
		//footerOffset		= Not applicable;
		excludeMissingPackets=true;
		break;
	default:
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
	{
		ostringstream os;
		os << "This is an unknown receiver type " << (int)d;
		string message(os.str());	FILE_LOG(logERROR, message);
	}

	return FAIL;
	}

	//delete threads and set number of listening threads
	if(myDetectorType == EIGER){
		pthread_mutex_lock(&statusMutex);
		dataCallbackThreadsMask = 0x0;
		listeningThreadsMask = 0x0;
		writerThreadsMask = 0x0;
		pthread_mutex_unlock(&(statusMutex));
		if(threadStarted){
			createListeningThreads(true);
			createWriterThreads(true);
		}
		numberofListeningThreads = MAX_NUMBER_OF_LISTENING_THREADS;
		numberofWriterThreads = MAX_NUMBER_OF_WRITER_THREADS;
	}
	if(zmqThreadStarted)
		createDataCallbackThreads(true);


	//set up fifo structure -1 for numberofJobsPerBuffer ensure it is done
	numberofJobsPerBuffer = -1;
	setupFifoStructure();

	numberofDataCallbackThreads = numberofListeningThreads;
	if(dataStreamEnable)
		createDataCallbackThreads();

	//allocate for latest data (frame copy for gui), free variables
	for(int i=0; i<MAX_NUMBER_OF_WRITER_THREADS; i++){
		if(latestData[i]) 	{delete[] latestData[i];latestData[i] = 0;}
	}
	for(int i=0; i<numberofWriterThreads; i++){
		if(excludeMissingPackets)
			latestData[i] = new char[bufferSize+sizeof(sls_detector_header)]();
		else
			latestData[i] = new char[bufferSize]();
	}


	//updates File Header
	if(myDetectorType == EIGER){
		for(int i=0; i<MAX_NUMBER_OF_WRITER_THREADS; i++)
			updateFileHeader(i);
	}
	sprintf(cstreambuf, "%s", " \0");
	FILE_LOG(logDEBUG, cstreambuf);
	{
		ostringstream os;
		os << " Detector type set to " << getDetectorType(d);
		string message(os.str());	FILE_LOG(logDEBUG, message);
	}

	return OK;
}


/***acquisition functions***/
void UDPStandardImplementation::resetAcquisitionCount(){


	pthread_mutex_lock(&progressMutex);
	startAcquisitionIndex = 0;
	acqStarted = false;
	pthread_mutex_unlock(&progressMutex);
	pthread_mutex_lock(&writeMutex);
	totalPacketsCaught = 0;
	pthread_mutex_unlock(&writeMutex);
	sprintf(cstreambuf, "%s", " \0");
	FILE_LOG(logDEBUG, cstreambuf);
	{
		ostringstream os;
		os << "Acquisition Count has been reset";
		string message(os.str());	FILE_LOG(logINFO, message);
	}
}


int UDPStandardImplementation::startReceiver(char *c){

	sprintf(cstreambuf, "%s", " \0");
	FILE_LOG(logDEBUG, cstreambuf);
	{
		ostringstream os;
		os  << "Starting Receiver";
		string message(os.str());	FILE_LOG(logINFO, message);
	}

	//reseting variables

	//for every acquisition start (not every scan)
	if(!acqStarted){
		pthread_mutex_lock(&progressMutex);
		acquisitionIndex = 0;
		pthread_mutex_unlock(&progressMutex);
		for(int i=0;i<numberofWriterThreads;i++){
			currentFrameNumber[i] = 0;		//has to be zero to add to startframeindex for each scan
			frameIndex[i] = 0;
		}
	}

	//for every measurement
	pthread_mutex_lock(&progressMutex);
	startFrameIndex = 0;
	pthread_mutex_unlock(&progressMutex);
	for(int i=0;i<numberofListeningThreads;i++){
		measurementStarted[i] = false;
		totalListeningPacketCount[i] = 0;
		totalIgnoredPacketCount[i] = 0;
	}

	for(int i=0;i<numberofWriterThreads;i++){
		frameIndex[i] = 0;
		//reset file parameters
		lastFrameNumberInFile[i] = -1;
		totalPacketsInFile[i] = 0;
		totalWritingPacketCount[i] = 0;
		totalWritingPacketCountFromLastCheck[i] = 0;
		if(sfilefd[i]){
			fclose(sfilefd[i]);
			sfilefd[i] = 0;
		}
		//reset gui variables
		frametoGuiCounter[i] = 0;
		guiNumPackets[i] = 0;
		strcpy(guiFileName[i],"");

	}
	pthread_mutex_lock(&writeMutex);
	packetsCaught = 0;
	totalPacketsCaught = 0;
	pthread_mutex_unlock(&writeMutex);
	//reset masks
	pthread_mutex_lock(&statusMutex);
	writerThreadsMask = 0x0;
	createFileMask = 0x0;
	fileCreateSuccess = false;
	pthread_mutex_unlock(&statusMutex);


	//Print Receiver Configuration
	if(myDetectorType != EIGER){
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os << "Data Compression has been " << stringEnable(dataCompressionEnable);
			string message(os.str());	FILE_LOG(logINFO, message);
		}
	}
	sprintf(cstreambuf, "%s", " \0");
	FILE_LOG(logDEBUG, cstreambuf);
	{
		ostringstream os;
		os << "Number of Jobs Per Buffer: " << numberofJobsPerBuffer << endl;
		os << "Max Frames Per File:" << maxFramesPerFile;
		string message(os.str());	FILE_LOG(logINFO, message);
	}

	if(frameToGuiFrequency)
	{
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		ostringstream os;
		os << "Frequency of frames sent to gui: " << frameToGuiFrequency;
		string message(os.str());	FILE_LOG(logINFO, message);
	}
	else
	{
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		ostringstream os;
		os << "Frequency of frames sent to gui: Random";
		string message(os.str());	FILE_LOG(logINFO, message);
	}


	//create UDP sockets
	if(createUDPSockets() == FAIL){
		strcpy(c,"Could not create UDP Socket(s).");
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		ostringstream os;
		os << c;
		string message(os.str());	FILE_LOG(logERROR, message);
		return FAIL;
	}

	if(setupWriter() == FAIL){
		//stop udp socket
		shutDownUDPSockets();
		sprintf(c,"Could not create file");
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		ostringstream os;
		os << c;
		string message(os.str());	FILE_LOG(logERROR, message);

		for(int i=0; i < numberofWriterThreads; i++)
			sem_post(&writerSemaphore[i]);
		return FAIL;
	}

	//For compression, just for gui purposes
	if(dataCompressionEnable)
		sprintf(completeFileName[0], "%s/%s_fxxx_%lld_xx.root", filePath,fileNamePerThread[0],(long long int)fileIndex);

	//initialize semaphore to synchronize between writer and gui reader threads
	for(int i=0;i<numberofWriterThreads;i++){
		sem_init(&writerGuiSemaphore[i],1,0);
		sem_init(&dataCallbackWriterSemaphore[i],1,0);
	}

	//status and thread masks
	pthread_mutex_lock(&statusMutex);
	status = RUNNING;
	for(int i=0;i<numberofListeningThreads;i++)
		listeningThreadsMask|=(1<<i);
	for(int i=0;i<numberofWriterThreads;i++)
		writerThreadsMask|=(1<<i);
	if(dataStreamEnable){
		for(int i=0;i<numberofDataCallbackThreads;i++)
			dataCallbackThreadsMask|=(1<<i);
	}
	pthread_mutex_unlock(&(statusMutex));


	//start listening /writing
	for(int i=0;i<numberofListeningThreads;i++)
		sem_post(&listenSemaphore[i]);
	for(int i=0; i < numberofWriterThreads; i++)
		sem_post(&writerSemaphore[i]);
	if(dataStreamEnable){
		for(int i=0;i<numberofDataCallbackThreads;i++)
			sem_post(&dataCallbackSemaphore[i]);
	}
	sprintf(cstreambuf, "%s", " \0");
	FILE_LOG(logDEBUG, cstreambuf);
	{
		ostringstream os;
		os  << "Receiver Started" << endl;
		os  << "Status: " << runStatusType(status);
		string message(os.str());	FILE_LOG(logINFO, message);
	}

	return OK;
}


/**
 * Pre: status is running, semaphores have been instantiated,
 * Post: udp sockets shut down, status is idle, semaphores destroyed
 * */
void UDPStandardImplementation::stopReceiver(){
	sprintf(cstreambuf, "%s", " \0");
	FILE_LOG(logDEBUG, cstreambuf);

	{
		ostringstream os;
		os  << "Stopping Receiver";
		string message(os.str());	FILE_LOG(logINFO, message);
	}

	//set status to transmitting
	startReadout();

	//wait until status is run_finished
	while(status == TRANSMITTING){
		usleep(5000);
	}

	//semaphore destroy
	for(int i=0; i < numberofWriterThreads; i++){
		sem_destroy(&writerGuiSemaphore[i]);
		sem_destroy(&dataCallbackWriterSemaphore[i]);
	}

	//change status
	pthread_mutex_lock(&statusMutex);
	status = IDLE;
	pthread_mutex_unlock(&(statusMutex));
	sprintf(cstreambuf, "%s", " \0");
	FILE_LOG(logDEBUG, cstreambuf);
	{
		ostringstream os;
		os  << "Receiver Stopped" << endl;
		os  << "Status: " << runStatusType(status);
		string message(os.str());	FILE_LOG(logINFO, message);
	}
	cout << endl << endl;
}





int UDPStandardImplementation::shutDownUDPSockets(){


	for(int i=0;i<numberofListeningThreads;i++){
		if(udpSocket[i]){
			udpSocket[i]->ShutDownSocket();
			sprintf(cstreambuf, "%s", " \0");
			FILE_LOG(logDEBUG, cstreambuf);
			{
				ostringstream os;
				os << "Shut down UDP Socket " << i;
				string message(os.str());	FILE_LOG(logINFO, message);
			}
			delete udpSocket[i];
			udpSocket[i] = 0;
		}
	}
	return OK;
}




/**
 * Pre: status is running, udp sockets have been initialized, stop receiver initiated
 * Post:udp sockets closed, status is transmitting
 * */
void UDPStandardImplementation::startReadout(){
	sprintf(cstreambuf, "%s", " \0");
	FILE_LOG(logDEBUG, cstreambuf);

	{
		ostringstream os;
		os << "Transmitting last data";
		string message(os.str());	FILE_LOG(logDEBUG, message);
	}

	if(status == RUNNING){


		//needs to wait for packets only if activated
		if(activated){
			//check if all packets got
			int totalP = 0,prev=-1;
			for(int i=0; i<numberofListeningThreads; ++i)
				totalP += totalListeningPacketCount[i];

			//check if current buffer still receiving something
			int currentReceivedInBuffer=0,prevReceivedInBuffer=-1;
			for(int i=0; i<numberofListeningThreads; ++i)
				currentReceivedInBuffer += udpSocket[i]->getCurrentTotalReceived();

			//wait for all packets
			if((unsigned long long int)totalP!=numberOfFrames*packetsPerFrame*numberofListeningThreads){

				//wait as long as there is change from prev totalP,
				//and also change from received in buffer to previous value
				//(as one listens to many at a time, shouldnt cut off in between)
				while((prev != totalP) || (prevReceivedInBuffer!= currentReceivedInBuffer)){
#ifdef DEBUG5
					cprintf(MAGENTA,"waiting for all packets prevP:%d totalP:%d PrevBuffer:%d currentBuffer:%d\n",prev,totalP,prevReceivedInBuffer,currentReceivedInBuffer);

#endif
					//usleep(2*1000*1000);
					usleep(5*1000);/* Need to find optimal time (exposure time and acquisition period) **/
					prev = totalP;
					totalP = 0;
					for(int i=0; i<numberofListeningThreads; ++i)
						totalP += totalListeningPacketCount[i];

					prevReceivedInBuffer = currentReceivedInBuffer;
					currentReceivedInBuffer = 0;
					for(int i=0; i<numberofListeningThreads; ++i)
						currentReceivedInBuffer += udpSocket[i]->getCurrentTotalReceived();
#ifdef DEBUG5
					cprintf(MAGENTA,"\tupdated:  totalP:%d currently in buffer:%d\n",totalP,currentReceivedInBuffer);

#endif
				}

			}
		}

		//set status
		pthread_mutex_lock(&statusMutex);
		status = TRANSMITTING;
		pthread_mutex_unlock(&statusMutex);
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os << "Status: Transmitting";
			string message(os.str());	FILE_LOG(logINFO, message);
		}
	}

	//shut down udp sockets and make listeners push dummy (end) packets for writers
	shutDownUDPSockets();
}



/**make this better by asking all of it at once*/
void UDPStandardImplementation::readFrame(int ithread, char* c,char** raw, int64_t &startAcq, int64_t &startFrame){


}



void UDPStandardImplementation::closeFile(int ithread){
	//normal
	if(!dataCompressionEnable){
		if(sfilefd[ithread]){
			sprintf(cstreambuf, "%s", " \0");
			FILE_LOG(logDEBUG, cstreambuf);
			{
				ostringstream os;
				os << "Going to close file: " << fileno(sfilefd[ithread]);
				string message(os.str());	FILE_LOG(logDEBUG4, message);
			}

			fflush(sfilefd[ithread]);
			fclose(sfilefd[ithread]);
			sfilefd[ithread] = 0;
		}
	}

	//compression
	else{
#if (defined(MYROOT1) && defined(ALLFILE_DEBUG)) || !defined(MYROOT1)
		if(sfilefd[0]){
			sprintf(cstreambuf, "%s", " \0");
			FILE_LOG(logDEBUG, cstreambuf);
			{
				ostringstream os;
				os << "sfilefd: " << fileno(sfilefd[0]);
				string message(os.str());	FILE_LOG(logDEBUG4, message);
			}
			fclose(sfilefd[0]);
			sfilefd[0] = 0;
		}
#endif

#ifdef MYROOT1
		pthread_mutex_lock(&writeMutex);
		//write to file
		if(myTree[ithread] && myFile[ithread]){
			myFile[ithread] = myTree[ithread]->GetCurrentFile();

			if(myFile[ithread]->Write())
				//->Write(tall->GetName(),TObject::kOverwrite);
				cout << "Thread " << ithread <<": wrote frames to file" << endl;
			else
				cout << "Thread " << ithread << ": could not write frames to file" << endl;

		}else
			cout << "Thread " << ithread << ": could not write frames to file: No file or No Tree" << endl;
		//close file
		if(myTree[ithread] && myFile[ithread])
			myFile[ithread] = myTree[ithread]->GetCurrentFile();
		if(myFile[ithread] != 0)
			myFile[ithread]->Close();
		myFile[ithread] = 0;
		myTree[ithread] = 0;
		pthread_mutex_unlock(&writeMutex);

#endif
	}
}


//eiger only
int UDPStandardImplementation::setActivate(int enable){


	if(enable != -1){
		activated = enable;
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os << "Activation: " << stringEnable(activated);
			string message(os.str());	FILE_LOG(logINFO, message);
		}
	}

	for(int i=0; i<MAX_NUMBER_OF_WRITER_THREADS; i++)
		updateFileHeader(i);

	return activated;
}



/*************************************************************************
 * Listening and Writing Threads *****************************************
 *************************************************************************/

int UDPStandardImplementation::createDataCallbackThreads(bool destroy){

	if(!destroy) cprintf(MAGENTA,"Data Callback thread created\n"); else cprintf(MAGENTA,"Data Callback thread destroyed\n");
	//reset masks
	killAllDataCallbackThreads = false;
	pthread_mutex_lock(&statusMutex);
	dataCallbackThreadsMask = 0x0;
	pthread_mutex_unlock(&(statusMutex));

	//destroy
	if(destroy){
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os << "Info: Destroying Data Callback Thread(s)";
			string message(os.str());	FILE_LOG(logDEBUG, message);
		}

		killAllDataCallbackThreads = true;
		for(int i = 0; i < numberofDataCallbackThreads; ++i){
			sem_post(&dataCallbackSemaphore[i]);
			pthread_join(dataCallbackThreads[i],NULL);
			sem_destroy(&dataCallbackSemaphore[i]);
			sprintf(cstreambuf, "%s", " \0");
			FILE_LOG(logDEBUG, cstreambuf);
			{
				ostringstream os;
				os << "." << flush;
				string message(os.str());	FILE_LOG(logDEBUG, message);
			}
		}
		killAllDataCallbackThreads = false;
		zmqThreadStarted = false;
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os << "Data Callback thread(s) destroyed";
			string message(os.str());	FILE_LOG(logINFO, message);
		}
	}

	//create
	else{
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os << "Creating Data Callback Thread(s)";
			string message(os.str());	FILE_LOG(logINFO, message);
		}

		//reset current index
		currentThreadIndex = -1;

		for(int i = 0; i < numberofDataCallbackThreads; ++i){
			sem_init(&dataCallbackSemaphore[i],1,0);
			zmqThreadStarted = false;
			currentThreadIndex = i;
			if(pthread_create(&dataCallbackThreads[i], NULL,startDataCallbackThread, (void*) this)){
				{
					sprintf(cstreambuf, "%s", " \0");
					FILE_LOG(logDEBUG, cstreambuf);
					ostringstream os;
					os << "Could not create data call back thread with index " << i;
					string message(os.str());	FILE_LOG(logERROR, message);
				}
				return FAIL;
			}
			while(!zmqThreadStarted);
			{
				sprintf(cstreambuf, "%s", " \0");
				FILE_LOG(logDEBUG, cstreambuf);
				ostringstream os;
				os << "." << flush;
				string message(os.str());	FILE_LOG(logDEBUG, message);
			}
		}
		{
			sprintf(cstreambuf, "%s", " \0");
			FILE_LOG(logDEBUG, cstreambuf);
			ostringstream os;
			os << "Info: Data Callback thread(s) created successfully.";
			string message(os.str());	FILE_LOG(logDEBUG, message);
		}
	}

	return OK;
}



int UDPStandardImplementation::createListeningThreads(bool destroy){

	if(!destroy) cprintf(BLUE,"Listening thread created\n"); else cprintf(BLUE,"Listening thread destroyed\n");
	//reset masks
	killAllListeningThreads = false;
	pthread_mutex_lock(&statusMutex);
	listeningThreadsMask = 0x0;
	pthread_mutex_unlock(&(statusMutex));

	//destroy
	if(destroy){
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os << "Info: Destroying Listening Thread(s)";
			string message(os.str());	FILE_LOG(logDEBUG, message);
		}

		killAllListeningThreads = true;
		for(int i = 0; i < numberofListeningThreads; ++i){
			sem_post(&listenSemaphore[i]);
			pthread_join(listeningThreads[i],NULL);
			sem_destroy(&listenSemaphore[i]);
			sprintf(cstreambuf, "%s", " \0");
			FILE_LOG(logDEBUG, cstreambuf);
			{
				ostringstream os;
				os << "." << flush;
				string message(os.str());	FILE_LOG(logDEBUG, message);
			}
		}
		killAllListeningThreads = false;
		threadStarted = false;
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os << "Listening thread(s) destroyed";
			string message(os.str());	FILE_LOG(logINFO, message);
		}
	}

	//create
	else{
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os << "Creating Listening Thread(s)";
			string message(os.str());	FILE_LOG(logINFO, message);
		}

		//reset current index
		currentThreadIndex = -1;

		for(int i = 0; i < numberofListeningThreads; ++i){
			sem_init(&listenSemaphore[i],1,0);
			threadStarted = false;
			currentThreadIndex = i;
			if(pthread_create(&listeningThreads[i], NULL,startListeningThread, (void*) this)){
				sprintf(cstreambuf, "%s", " \0");
				FILE_LOG(logDEBUG, cstreambuf);
				{
					ostringstream os;
					os << "Could not create listening thread with index " << i;
					string message(os.str());	FILE_LOG(logERROR, message);
				}
				return FAIL;
			}
			while(!threadStarted);
			sprintf(cstreambuf, "%s", " \0");
			FILE_LOG(logDEBUG, cstreambuf);
			{
				ostringstream os;
				os << "." << flush;
				string message(os.str());	FILE_LOG(logDEBUG, message);
			}
		}
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os << "Info: Listening thread(s) created successfully.";
			string message(os.str());	FILE_LOG(logDEBUG, message);
		}
	}

	return OK;
}



int UDPStandardImplementation::createWriterThreads(bool destroy){

	if(!destroy) cprintf(GREEN,"Writer thread created\n"); else cprintf(GREEN,"Writer thread destroyed\n");
	//reset masks
	killAllWritingThreads = false;
	pthread_mutex_lock(&statusMutex);
	writerThreadsMask = 0x0;
	createFileMask = 0x0;
	pthread_mutex_unlock(&(statusMutex));

	//destroy threads
	if(destroy){
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os << "Info: Destroying Writer Thread(s)";
			string message(os.str());	FILE_LOG(logDEBUG, message);
		}

		killAllWritingThreads = true;
		for(int i = 0; i < numberofWriterThreads; ++i){
			sem_post(&writerSemaphore[i]);
			pthread_join(writingThreads[i],NULL);
			sem_destroy(&writerSemaphore[i]);
			sprintf(cstreambuf, "%s", " \0");
			FILE_LOG(logDEBUG, cstreambuf);
			{
				ostringstream os;
				os <<"."<<flush;
				string message(os.str());	FILE_LOG(logDEBUG, message);
			}
		}
		killAllWritingThreads = false;
		threadStarted = false;
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os << "Writer thread(s) destroyed";
			string message(os.str());	FILE_LOG(logINFO, message);
		}
	}

	//create threads
	else{
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os << "Creating Writer Thread(s)";
			string message(os.str());	FILE_LOG(logINFO, message);
		}

		//reset current index
		currentThreadIndex = -1;

		for(int i = 0; i < numberofWriterThreads; ++i){
			sem_init(&writerSemaphore[i],1,0);
			threadStarted = false;
			currentThreadIndex = i;
			if(pthread_create(&writingThreads[i], NULL,startWritingThread, (void*) this)){
				sprintf(cstreambuf, "%s", " \0");
				FILE_LOG(logDEBUG, cstreambuf);
				{
					ostringstream os;
					os << "Could not create writer thread with index " << i;
					string message(os.str());	FILE_LOG(logERROR, message);
				}
				return FAIL;
			}
			while(!threadStarted);
			sprintf(cstreambuf, "%s", " \0");
			FILE_LOG(logDEBUG, cstreambuf);
			{
				ostringstream os;
				os << "." << flush;
				string message(os.str());	FILE_LOG(logDEBUG, message);
			}
		}
#ifdef DEBUG
		cout << "\nWriter thread(s) created successfully" << endl;
#endif
	}

	return OK;
}



void UDPStandardImplementation::setThreadPriorities(){


	struct sched_param tcp_param, listen_param, write_param, datacallback_param;
	bool rights = true;

	//assign priorities
	datacallback_param.sched_priority = 55;
	tcp_param.sched_priority = 50;
	listen_param.sched_priority = 99;
	write_param.sched_priority = 90;

	//set them
	for(int i = 0; i < numberofListeningThreads; ++i){
		if (pthread_setschedparam(listeningThreads[i], SCHED_RR, &listen_param) == EPERM){
			rights = false;
			break;
		}
	}
	for(int i = 0; i < numberofWriterThreads; ++i){
		if(rights)
			if (pthread_setschedparam(writingThreads[i], SCHED_RR, &write_param) == EPERM){
				rights = false;
				break;
			}
	}
	if(dataStreamEnable){
		for(int i = 0; i < numberofDataCallbackThreads; ++i){
			if(rights)
				if (pthread_setschedparam(dataCallbackThreads[i], SCHED_RR, &datacallback_param) == EPERM){
					rights = false;
					break;
				}
		}
	}
	if (pthread_setschedparam(pthread_self(),5 , &tcp_param) == EPERM)
		rights = false;

	if(!rights){
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os << "Unable to prioritize threads. Root privileges required for this option.";
			string message(os.str());	FILE_LOG(logWARNING, message);
		}
	}else{
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os << "Priorities set - DataCallback: 55, TCP:50, Listening:99, Writing:90";
			string message(os.str());	FILE_LOG(logINFO, message);
		}
	}
}




int UDPStandardImplementation::createUDPSockets(){


	//switching ports if bottom enabled
	uint32_t port[2];
	port[0]= udpPortNum[0];
	port[1]= udpPortNum[1];


	//if eth is mistaken with ip address
	if (strchr(eth,'.') != NULL)
		strcpy(eth,"");

	shutDownUDPSockets();
	int headerpacketsize = 0;
	if(myDetectorType == EIGER)
		headerpacketsize = EIGER_HEADER_PACKET_LENGTH;

	//if no eth, listen to all
	if(!strlen(eth)){
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os << "eth is empty. Listening to all";
			string message(os.str());	FILE_LOG(logWARNING, message);
		}

		for(int i=0;i<numberofListeningThreads;i++)
			udpSocket[i] = new genericSocket(port[i],genericSocket::UDP,onePacketSize,NULL,headerpacketsize);
	}
	//normal socket
	else{
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os << "Ethernet Interface:" << eth;
			string message(os.str());	FILE_LOG(logINFO, message);
		}

		for(int i=0;i<numberofListeningThreads;i++)
			udpSocket[i] = new genericSocket(port[i],genericSocket::UDP,onePacketSize,eth,headerpacketsize);
	}

	//error
	for(int i=0;i<numberofListeningThreads;i++){
		int iret = udpSocket[i]->getErrorStatus();
		if(!iret){
			cout << "UDP port opened at port " << port[i] << endl;
		}else{
			sprintf(cstreambuf, "%s", " \0");
			FILE_LOG(logDEBUG, cstreambuf);
			{
				ostringstream os;
				os << "Could not create UDP socket on port " << port[i] << " error: " << iret;
				string message(os.str());	FILE_LOG(logERROR, message);
			}
			shutDownUDPSockets();
			return FAIL;
		}
	}
	sprintf(cstreambuf, "%s", " \0");
	FILE_LOG(logDEBUG, cstreambuf);
	{
		ostringstream os;
		os << "UDP socket(s) created successfully.";
		string message(os.str());	FILE_LOG(logDEBUG, message);
	}
	cout << "Listener Ready ..." << endl;

	return OK;
}



int UDPStandardImplementation::setupWriter(){


	//acquisition start call back returns enable write
	if (startAcquisitionCallBack) {
		//remove detector index in the file name
		int deti = -1;
		string tempname(fileName);
		size_t uscore=tempname.rfind("_");
		if ((uscore!=string::npos) && (sscanf(tempname.substr(uscore+1,tempname.size()-uscore-1).c_str(),"d%d",&deti)))
			tempname=tempname.substr(0,uscore);
		startAcquisitionCallBack(filePath, (char*)tempname.c_str(),fileIndex, (uint32_t)bufferSize,pStartAcquisition);
	}
	if (rawDataReadyCallBack) {
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		ostringstream os;
		os << "Data Write has been defined externally";
		string message(os.str());	FILE_LOG(logINFO, message);
	}
	if (!fileWriteEnable) {
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		ostringstream os;
		os << "Data will not be saved";
		string message(os.str());	FILE_LOG(logINFO, message);
	}


	//creating first file
	//setting all value to 1
	pthread_mutex_lock(&statusMutex);
	for(int i=0; i<numberofWriterThreads; i++)
		createFileMask|=(1<<i);
	pthread_mutex_unlock(&statusMutex);

	for(int i=0; i<numberofWriterThreads; i++){
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os <<	i << " Going to post 1st semaphore";
			string message(os.str());	FILE_LOG(logDEBUG4, message);
		}
		sem_post(&writerSemaphore[i]);
	}
	//wait till its mask becomes zero(all created)
	while(createFileMask){
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os << "*" << flush;
			string message(os.str());	FILE_LOG(logDEBUG4, message);
		}
		usleep(5000);
	}

	if(fileCreateSuccess == OK){
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os << "Successfully created file(s)";
			string message(os.str());	FILE_LOG(logDEBUG, message);
		}
		cout << "Writer Ready ..." << endl;
	}

	return fileCreateSuccess;
}



int UDPStandardImplementation::createNewFile(int ithread){


	//create file name
	if(!frameIndexEnable)
		sprintf(completeFileName[ithread], "%s/%s_%lld.raw", filePath,fileNamePerThread[ithread],(long long int)fileIndex);
	else
		sprintf(completeFileName[ithread], "%s/%s_f%012lld_%lld.raw", filePath,fileNamePerThread[ithread],(long long int)lastFrameNumberInFile[ithread]+1,(long long int)fileIndex);
	sprintf(cstreambuf, "%s", " \0");
	FILE_LOG(logDEBUG, cstreambuf);
	{
		ostringstream os;
		os << completeFileName[ithread];
		string message(os.str());	FILE_LOG(logDEBUG4, message);
	}

	//filewrite enable & we allowed to create/close files
	if(fileWriteEnable){

		//close file pointers
		if(sfilefd[ithread]){
			//all threads need to close file, reset mask and exit loop
			if(myDetectorType == EIGER && fileWriteEnable){
				updateFileHeader(ithread);
				fseek(sfilefd[ithread],0,0);
				fwrite((void*)fileHeader[ithread], 1, FILE_HEADER_SIZE, sfilefd[ithread]);
			}
			fflush(sfilefd[ithread]);
			fclose(sfilefd[ithread]);
			sfilefd[ithread] = 0;
		}

		//create file
		if(!overwriteEnable){
			if (NULL == (sfilefd[ithread] = fopen((const char *) (completeFileName[ithread]), "wx"))){
				sprintf(cstreambuf, "%s", " \0");
				FILE_LOG(logDEBUG, cstreambuf);
				ostringstream os;
				os << "Could not create/overwrite file" << completeFileName[ithread];
				string message(os.str());	FILE_LOG(logERROR, message);
				sfilefd[ithread] = 0;
				return FAIL;
			}
		}else if (NULL == (sfilefd[ithread] = fopen((const char *) (completeFileName[ithread]), "w"))){
			sprintf(cstreambuf, "%s", " \0");
			FILE_LOG(logDEBUG, cstreambuf);
			ostringstream os;
			os << "Could not create file" << completeFileName[ithread];
			string message(os.str());	FILE_LOG(logERROR, message);
			sfilefd[ithread] = 0;
			return FAIL;
		}
		//setting file buffer size to 16mb
		setvbuf(sfilefd[ithread],NULL,_IOFBF,BUF_SIZE);

		//Print packet loss and filenames
		if(totalWritingPacketCount[ithread]){
			if(numberofWriterThreads>1){
				cprintf(BLUE,"File:%s"
						"\nThread:%d"
						"\tLost:%lld"
						"\t\tPackets:%lld"
						"\tFrame#:%lld"
						"\tPFrame#:%lld\n",
						completeFileName[ithread],ithread,
						((frameNumberInPreviousFile[ithread]+1+maxFramesPerFile)>numberOfFrames)
						?(long long int)((numberOfFrames-(frameNumberInPreviousFile[ithread]+1))*packetsPerFrame - totalPacketsInFile[ithread])
								:(long long int)((frameNumberInPreviousFile[ithread]+maxFramesPerFile - frameNumberInPreviousFile[ithread])*packetsPerFrame - totalPacketsInFile[ithread]),
								 (long long int)totalPacketsInFile[ithread],
								 (long long int)currentFrameNumber[ithread],
								 (long long int)frameNumberInPreviousFile[ithread]
				);
			}else{
				cprintf(BLUE,"File:%s"
						"\nLost:%lld"
						"\t\tPackets:%lld"
						"\tFrame#:%lld"
						"\tPFrame#:%lld\n",
						completeFileName[ithread],
						((frameNumberInPreviousFile[ithread]+1+maxFramesPerFile)>numberOfFrames)
						?(long long int)(numberOfFrames-(frameNumberInPreviousFile[ithread]+1))
								:(long long int)(frameNumberInPreviousFile[ithread]+maxFramesPerFile - frameNumberInPreviousFile[ithread]),
								 (long long int)totalPacketsInFile[ithread],
								 (long long int)currentFrameNumber[ithread],
								 (long long int)frameNumberInPreviousFile[ithread]
				);
			}

		}else
			printf("Thread:%d File opened:%s\n",ithread, completeFileName[ithread]);

		//write file header
		if(myDetectorType == EIGER)
			fwrite((void*)fileHeader[ithread], 1, FILE_HEADER_SIZE, sfilefd[ithread]);
	}

	//reset counters for each new file
	if(totalWritingPacketCount[ithread]){
		frameNumberInPreviousFile[ithread] = currentFrameNumber[ithread];
		totalPacketsInFile[ithread] = 0;
	}else{
		frameNumberInPreviousFile[ithread] = -1;
		frameNumberInPreviousCheck[ithread] = -1;
	}



	return OK;
}




int UDPStandardImplementation::createCompressionFile(int ithread, int iframe){


#ifdef MYROOT1
	char temp[MAX_STR_LENGTH];
	//create file name for gui purposes, and set up acquistion parameters
	sprintf(temp, "%s/%s_fxxx_%d_%d.root", filePath,fileNamePerThread[ithread],fileIndex,ithread);
	//file
	myFile[ithread] = new TFile(temp,"RECREATE");/** later  return error if it exists */
	cprintf(GREEN,"Writing_Thread %d: Created Compression File: %s\n",ithread, temp);
	//tree
	sprintf(temp, "%s_fxxx_%d_%d",fileNamePerThread[ithread],fileIndex,ithread);
	myTree[ithread]=singlePhotonDetectorObject[ithread]->initEventTree(temp, &iframe);
	//resets the pedestalSubtraction array and the commonModeSubtraction
	singlePhotonDetectorObject[ithread]->newDataSet();
	if(myFile[ithread]==NULL){
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os << "File Null";
			string message(os.str());	FILE_LOG(logERROR, message);
		}
		return FAIL;
	}
	if(!myFile[ithread]->IsOpen()){
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os << "File Not Open";
			string message(os.str());	FILE_LOG(logERROR, message);
		}
		return FAIL;
	}
	return OK;
#endif
	return FAIL;
}



void* UDPStandardImplementation::startDataCallbackThread(void* this_pointer){

	((UDPStandardImplementation*)this_pointer)->startDataCallback();
	return this_pointer;
}



void* UDPStandardImplementation::startListeningThread(void* this_pointer){

	((UDPStandardImplementation*)this_pointer)->startListening();
	return this_pointer;
}



void* UDPStandardImplementation::startWritingThread(void* this_pointer){

	((UDPStandardImplementation*)this_pointer)->startWriting();
	return this_pointer;
}




void UDPStandardImplementation::startDataCallback(){


	//set current thread value  index
	int ithread = currentThreadIndex;
	struct timespec begin,end;

	// server address to bind
	char hostName[100] = "tcp://*:";//"tcp://127.0.0.1:";
	int portno = DEFAULT_ZMQ_PORTNO + (detID*numberofListeningThreads+ithread);
	sprintf(hostName,"%s%d",hostName,portno);

	//socket details
	void *context = zmq_ctx_new();
	void *zmqsocket = zmq_socket(context, ZMQ_PUSH);		// create a publisher
	int val = -1;
	zmq_setsockopt(zmqsocket, ZMQ_LINGER, &val,sizeof(val)); // wait for the unsent packets  before closing socket
	//val = 10;
	//zmq_setsockopt(zmqsocket,ZMQ_SNDHWM,&val,sizeof(val)); //set SEND HIGH WATER MARK (8-9ms slower)
	zmq_bind(zmqsocket,hostName);
	// bind
	sprintf(cstreambuf, "%s", " \0");
	FILE_LOG(logDEBUG, cstreambuf);
	{
		ostringstream os;
		os << "Thread" << ithread << ": ZMQ Server at " << hostName;
		string message(os.str());	FILE_LOG(logINFO, message);
	}


	int headersize=0;
	switch(myDetectorType){
	case EIGER:
		headersize = EIGER_DATA_PACKET_HEADER_SIZE; break;
	default:
		headersize = 0; break;
	}

	//let calling function know thread started and obtained current (after sockets created)
	if(!zmqThreadStarted)
		zmqThreadStarted = true;

	/* outer loop - loops once for each acquisition */
	//infinite loop, exited only to change dynamic range, 10G parameters etc (then recreated again)
	while(true){

		int oneframesize = oneDataSize * packetsPerFrame;
		char* buffer = new char[packetsPerFrame*oneDataSize]();
		memset(buffer,0xFF,oneframesize);
		int size = 0;
		int offset = 0;
		uint32_t currentfnum = 0;
		uint64_t fnum = 0;
		uint32_t pnum = 0;
		uint32_t snum = 0;
		uint64_t bid = 0;
		bool randomSendNow = true;
		bool newFrame = false;



		//header details
		const char *jsonFmt ="{"
				"\"jsonversion\":%u, "
				"\"acqIndex\":%llu, "
				"\"fIndex\":%llu, "
				"\"bitmode\":%d, "
				"\"shape\":[%d, %d], "
				"\"fname\":\"%s\", "

				"\"frameNumber\":%llu, "
				"\"expLength\":%u, "
				"\"packetNumber\":%u, "
				"\"bunchId\":%llu, "
				"\"timestamp\":%llu, "
				"\"modId\":%u, "
				"\"xCoord\":%u, "
				"\"yCoord\":%u, "
				"\"zCoord\":%u, "
				"\"debug\":%u, "
				"\"roundRNumber\":%u, "
				"\"detType\":%u, "
				"\"version\":%u"
				"}\0";
		int npixelsx=0, npixelsy=0;
		switch(myDetectorType) {
		case JUNGFRAU: 	npixelsx = JFRAU_PIXELS_IN_ONE_ROW;		npixelsy = JFRAU_PIXELS_IN_ONE_COL;		break;
		case EIGER: 	npixelsx = EIGER_PIXELS_IN_ONE_ROW;		npixelsy = EIGER_PIXELS_IN_ONE_COL;		break;
		default:break; /* will not work for other detectors*/
		}
		uint64_t acquisitionIndex = -1;
		uint64_t frameIndex = -1;
#ifdef DEBUG
		int oldpnum = -1;
#endif
		int datapacketscaught = 0;

		/* inner loop - loop for each buffer */
		//until mask reset (dummy pcaket got by writer)
		while((1 << ithread) & dataCallbackThreadsMask){

			//let the writer thread continue, while we process carry over if any
			sem_post(&writerGuiSemaphore[ithread]);
			//wait for receiver to send more data
			sem_wait(&dataCallbackWriterSemaphore[ithread]);

			//end if acquistion
			if(guiNumPackets[ithread] == dummyPacketValue){

				//send final header
				//update frame details
#ifdef DEBUG
				cprintf(BLUE,"%d sending dummy\n");
#endif

				frameIndex = -1;
				acquisitionIndex = -1;
				{
					char buf[1000]="";memset(buf,0,1000);
					int len = sprintf(buf,jsonFmt,
							SLS_DETECTOR_JSON_HEADER_VERSION, acquisitionIndex, frameIndex, dynamicRange, npixelsx, npixelsy,completeFileName[ithread],
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

					zmq_send(zmqsocket, buf,1000, 0);
				}
				//send final data
				zmq_send (zmqsocket, "end\0", 4, 0);

				pthread_mutex_lock(&statusMutex);
				dataCallbackThreadsMask^=(1<<ithread);
				pthread_mutex_unlock(&statusMutex);
#ifdef DEBUG
				cprintf(GREEN,"Data Streaming %d: packets sent:%d\n",ithread,datapacketscaught);
#endif
				continue;
			}

			//random and the timer is on (randomSendNow is false)
			if(!frameToGuiFrequency){
				if(!randomSendNow){
					clock_gettime(CLOCK_REALTIME, &end);
#ifdef DEBUG
					cprintf(BLUE,"%d Elapsed time:%f seconds\n",ithread,( end.tv_sec - begin.tv_sec )	+ ( end.tv_nsec - begin.tv_nsec ) / 1000000000.0);
#endif
					//still less than 250 ms, keep waiting
					if((( end.tv_sec - begin.tv_sec )	+ ( end.tv_nsec - begin.tv_nsec ) / 1000000000.0) < (frameToGuiTimerinMS/1000))
						continue;
					//done with timer, look into data
					randomSendNow = true;
				}
			}

			if(excludeMissingPackets){
#ifdef DEBUG
				cprintf(BLUE,"%d sending image\n", ithread);
#endif
				//send header
				//update frame details
				sls_detector_header* header = (sls_detector_header*) (latestData[ithread]);
				uint64_t fnum = header->frameNumber;
				frameIndex = fnum - startFrameIndex;
				acquisitionIndex = fnum - startAcquisitionIndex;
				{
					char buf[1000]="";memset(buf,0,1000);
					int len = sprintf(buf,jsonFmt,
							SLS_DETECTOR_JSON_HEADER_VERSION, acquisitionIndex, frameIndex, dynamicRange, npixelsx, npixelsy,completeFileName[ithread],
							header->frameNumber, header->expLength, header->packetNumber, header->bunchId, header->timestamp,
							header->modId, header->xCoord, header->yCoord, header->zCoord, header->debug, header->roundRNumber, header->detType, header->version);

					zmq_send(zmqsocket, buf,1000, 0);
				}
				//send data
				zmq_send(zmqsocket, (latestData[ithread]+sizeof(sls_detector_header)), bufferSize, 0);
				//start clock after sending
				if(!frameToGuiFrequency){
					randomSendNow = false;
					clock_gettime(CLOCK_REALTIME, &begin);
				}
			}


			//moench, jctb
			else{
				cprintf(BG_RED, "should not be in here, only for moench, jctb. Exiting\n");
				exit(-1);
			}



		}/*--end of loop for each buffer (inner loop)*/

		//free resources
		delete[] buffer;

		//end of acquisition, wait for next acquisition/change of parameters
		sem_wait(&dataCallbackSemaphore[ithread]);


		//check to exit thread (for change of parameters) - only EXIT possibility
		if(killAllDataCallbackThreads){
			break;//pthread_exit(NULL);
		}

	}/*--end of loop for each acquisition (outer loop) */


	//free resources
	zmq_unbind(zmqsocket, hostName); /* will this be too soon and cut the sending*/
	zmq_close(zmqsocket);
	zmq_ctx_destroy(context);
	cprintf(MAGENTA,"DataCallback_Thread %d:Goodbye!\n",ithread);
}




void UDPStandardImplementation::startListening(){


	//set current thread value  index
	int ithread = currentThreadIndex;
	//let calling function know thread started and obtained current
	threadStarted = 1;


	uint32_t rc = 0;				//size of buffer received in bytes
	//split frames for data compression
	int carryonBufferSize = 0; 		//from previous buffer to keep frames together in a buffer
	char* tempBuffer = 0;			//temporary buffer to store split frames


	/* outer loop - loops once for each acquisition */
	//infinite loop, exited only to change dynamic range, 10G parameters etc (then recreated again)
	while(true){

		//compression variables reset before acquisition
		if(dataCompressionEnable){
			carryonBufferSize = 0;
			if(tempBuffer){delete []tempBuffer;tempBuffer=0;}
			tempBuffer = new char[onePacketSize * (packetsPerFrame - 1)](); 	//store maximum of 1 packets less in a frame
		}

		/* inner loop - loop for each buffer */
		//until mask reset (udp sockets shut down by client)
		while((1 << ithread) & listeningThreadsMask){


			//pop from fifo
			fifoFree[ithread]->pop(buffer[ithread]);

			//udpsocket doesnt exist
			if(activated && !udpSocket[ithread]){
				sprintf(cstreambuf, "%s", " \0");
				FILE_LOG(logDEBUG, cstreambuf);
				{
					ostringstream os;
					os << "Listening_Thread " << ithread << ": UDP Socket not created or shut down earlier";
					string message(os.str());	FILE_LOG(logERROR, message);
				}
				stopListening(ithread,0);
				continue;
			}

			if(!activated)																//eiger not activated modules
				rc = prepareAndListenBufferDeactivated(ithread);
			else if(excludeMissingPackets)												//eiger and jungfrau
				rc = prepareAndListenBufferCompleteFrames(ithread);
			else{
				rc = prepareAndListenBuffer(ithread, carryonBufferSize, tempBuffer);	//others
				carryonBufferSize = 0;
			}

			//problem in receiving or end of acquisition
			if (status == TRANSMITTING||(rc <= 0 && activated == 0)){
				stopListening(ithread,rc);
				continue;
			}

			if(dataCompressionEnable)
				(*((uint32_t*)(buffer[ithread]))) = processListeningBuffer(ithread, carryonBufferSize, tempBuffer, rc);

			//push buffer to FIFO
			while(!fifo[ithread]->push(buffer[ithread]));

		}/*--end of loop for each buffer (inner loop)*/

		//end of acquisition, wait for next acquisition/change of parameters
		sem_wait(&listenSemaphore[ithread]);

		//check to exit thread (for change of parameters) - only EXIT possibility
		if(killAllListeningThreads){
			cprintf(BLUE,"Listening_Thread %d:Goodbye!\n",ithread);
			//free resources at exit
			if(tempBuffer) delete[] tempBuffer;
			pthread_exit(NULL);
		}

	}/*--end of loop for each acquisition (outer loop) */
}





int UDPStandardImplementation::prepareAndListenBuffer(int ithread, int cSize, char* temp){


	int receivedSize = 0;

	//carry over from previous buffer
	if(cSize)
		memcpy(buffer[ithread] + fifoBufferHeaderSize, temp, cSize);

	//listen to after the carry over buffer
	if(status != TRANSMITTING)
		receivedSize = udpSocket[ithread]->ReceiveDataOnly(buffer[ithread] + fifoBufferHeaderSize + cSize,
				(bufferSize * numberofJobsPerBuffer) - cSize);

	if(receivedSize > 0){
		//write packet count to buffer
		*((uint32_t*)(buffer[ithread])) = (receivedSize/onePacketSize);
		totalListeningPacketCount[ithread] += (receivedSize/onePacketSize);

		//start indices for each start of scan/acquisition
		if(!measurementStarted[ithread]) //and rc>0
			startFrameIndices(ithread);
	}
#ifdef DEBUG
	cprintf(BLUE, "Listening_Thread %d : Received bytes: %d. Expected bytes: %d\n", ithread, receivedSize, bufferSize * numberofJobsPerBuffer-cSize);
#endif
	return receivedSize;
}







int UDPStandardImplementation::prepareAndListenBufferDeactivated(int ithread){


	//last
	if(currentFrameNumber[ithread] == numberOfFrames)
		return 0;

	//copy dummy packets
	memset(buffer[ithread] + fifoBufferHeaderSize, 0xFF,bufferSize);

	//write fnum and number of packets
	(*((uint64_t*)(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS))) = currentFrameNumber[ithread]+1;
	(*((uint32_t*)(buffer[ithread]))) = packetsPerFrame;

	//start indices for each start of scan/acquisition (rc > 0)
	if(!measurementStarted[ithread])
		startFrameIndices(ithread);

	return bufferSize;
}




int UDPStandardImplementation::prepareAndListenBufferCompleteFrames(int ithread){


	int headerlength = 0;
	uint32_t LASTPNUM = 0;
	uint32_t FIRSTPNUM = 0;
	int INCORDECR = 0;
	switch(myDetectorType){
	case JUNGFRAU:
		headerlength 	= JFRAU_HEADER_LENGTH;
		FIRSTPNUM		= packetsPerFrame-1;
		LASTPNUM		= 0;
		INCORDECR		= -1;
		break;
	case EIGER:
		headerlength 	= EIGER_DATA_PACKET_HEADER_SIZE;
		FIRSTPNUM 		= 0;
		LASTPNUM		= packetsPerFrame-1;
		INCORDECR		= 1;
		break;
	default:break;
	}


	int offset = fifoBufferHeaderSize;
	uint32_t pnum = 0;
	uint64_t fnum = 0;
	uint64_t bnum = 0;
	uint64_t snum = 0;
	int rc = 0;
	//from getframeandpacketnumber()
	uint32_t pi = 0;
	uint64_t fi = 0;
	uint64_t bi = 0;
	uint32_t si = 0;


	//read first packet
	pnum = FIRSTPNUM;				//first packet number to validate
	if(status != TRANSMITTING)	rc = udpSocket[ithread]->ReceiveDataOnly(buffer[ithread] + offset);
	if(rc <= 0) return 0;
	if(getFrameandPacketNumber(ithread,buffer[ithread] + offset,fi,pi,si,bi) == FAIL){
		pi = ALL_MASK_32;					//got 0 from fpga
		fi = ALL_MASK_32;
	}
	else
		fnum = fi;								//fnum of first packet
	bnum = bi;									//bnum of first packet
	snum = si;									//snum of first packet
	totalListeningPacketCount[ithread]++;
#ifdef VERBOSE
	if(!ithread) cout << "1 pnum:" << pnum << endl;
#endif
	//start indices for each start of scan/acquisition (rc > 0)
	if(!measurementStarted[ithread])
		startFrameIndices(ithread);


	while(true){

		//------------------------------------------------------ correct packet --------------------------------------------------------
		if((myDetectorType == JUNGFRAU && pnum == pi) || 					//jungfrau only looks at pnum
				(myDetectorType == EIGER && pnum == pi && fnum == fi)){		// eiger looks at pnum and fnum
#ifdef VERBOSE
			if(!ithread) cout << "correct packet" << endl;
#endif
			//copy only data
			memcpy(buffer[ithread] + offset,buffer[ithread] + offset + headerlength, oneDataSize);
			offset+=oneDataSize;

			//if complete frame
			if(pnum == LASTPNUM)
				break;
			//else increment/decrement
			pnum += INCORDECR;

			rc=0;							//listen again
			if(status != TRANSMITTING)
				rc = udpSocket[ithread]->ReceiveDataOnly(buffer[ithread] + offset);
			if(rc <= 0){						//end: update ignored and return
				if(myDetectorType == JUNGFRAU)
					totalIgnoredPacketCount[ithread] += (packetsPerFrame - pnum);
				else
					totalIgnoredPacketCount[ithread] += (pnum + 1);
				return 0;
			}
			totalListeningPacketCount[ithread]++;
			if(getFrameandPacketNumber(ithread, buffer[ithread] + offset,fi,pi,si,bi) == FAIL){
				pi = ALL_MASK_32;			//got 0 from fpga
				fi = ALL_MASK_32;
				totalIgnoredPacketCount[ithread] += (pnum + 1);
			}
			else if(myDetectorType == EIGER)
				fnum = fi;						//update currentfnum for eiger (next packets should have currentfnum value)
#ifdef VERBOSE
			if(!ithread) cout << "next currentpnum :" << pnum << endl;
#endif
		}

		//------------------------------------------------------ wrong packet --------------------------------------------------------
		else{
#ifdef VERBOSE
			if(!ithread) cprintf(RED,"wrong packet %d, expected packet %d fnum of last good one:%d\n",
					pi,pnum,(*((uint32_t*)(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS))));
#endif
			if(myDetectorType == JUNGFRAU)
				totalIgnoredPacketCount[ithread] += (packetsPerFrame - pnum -1); //extra 1 subtracted now to be added in the while loop anyway
			else
				totalIgnoredPacketCount[ithread] += pnum; 						//extra 1 subtracted now to be added in the while loop anyway
			pnum = FIRSTPNUM;
			offset = fifoBufferHeaderSize;

			//find the start of next image
			while(pnum != pi){
				totalIgnoredPacketCount[ithread]++;

				rc=0;
				if(status != TRANSMITTING)
					rc = udpSocket[ithread]->ReceiveDataOnly(buffer[ithread] + offset);
				if(rc <= 0){
					if(myDetectorType == JUNGFRAU)
						totalIgnoredPacketCount[ithread] += (packetsPerFrame - pnum);
					else
						totalIgnoredPacketCount[ithread] += (pnum + 1);
					return 0;
				}
				totalListeningPacketCount[ithread]++;
				if(getFrameandPacketNumber(ithread, buffer[ithread] + offset,fi,pi,si,bi) == FAIL){
					pi = ALL_MASK_32;			//got 0 from fpga
					fi = ALL_MASK_32;
				}
#ifdef VERBOSE
				if(!ithread) cout << "trying to find pnum:" << pnum << " got " << pi << endl;
#endif
			}
			if(fi!=ALL_MASK_32)
				fnum = fi;						//fnum of first packet
			bnum = bi;							//bnum of first packet
			snum = si;							//snum of first packet
		}
	}
	//------------------------------------------------------ got a complete frame --------------------------------------------------------

	sls_detector_header* header = (sls_detector_header*) (buffer[ithread]+HEADER_SIZE_NUM_TOT_PACKETS);
	memset(header, 0, sizeof(sls_detector_header));
	header->frameNumber = (uint64_t) (fnum + startAcquisitionIndex);
	if (myDetectorType == EIGER && dynamicRange == 32)
		header->expLength = (uint32_t) snum;
	header->packetNumber = (uint32_t) packetsPerFrame;
	if (myDetectorType == JUNGFRAU)
		header->bunchId = (uint64_t) bnum;
	header->xCoord = (uint16_t) detID * numberofListeningThreads + ithread;
	header->detType = (uint8_t) myDetectorType;
	header->version = (uint8_t) SLS_DETECTOR_HEADER_VERSION;

#ifdef VERBOSE
	if(!ithread)
		cprintf(BLUE,
				"framenumber:%lu\tsubfnum:%u\tpnum:%u\tbunchid:%lu\txcoord:%u\tdettype:%u\tversion:%u\n",
				header->frameNumber, header->expLength, header->packetNumber,
				header->bunchId, header->xCoord, header->detType, header->version);
#endif

	//write packet count to buffer
	*((uint32_t*)(buffer[ithread])) = packetsPerFrame;
	return bufferSize;
}



void UDPStandardImplementation::startFrameIndices(int ithread){



	jfrau_packet_header_t* header=0;
	switch(myDetectorType){
	case EIGER:
		startFrameIndex = 1;	//frame number always resets
		break;
	case JUNGFRAU:
		header = (jfrau_packet_header_t*)(buffer[ithread]+HEADER_SIZE_NUM_TOT_PACKETS);
		startFrameIndex = (*( (uint32_t*) header->frameNumber))&frameIndexMask;
		break;
	default:
		if(shortFrameEnable < 0){
			startFrameIndex = (((((uint32_t)(*((uint32_t*)(buffer[ithread] + fifoBufferHeaderSize))))+1)
					& (frameIndexMask)) >> frameIndexOffset);
		}else{
			startFrameIndex = ((((uint32_t)(*((uint32_t*)(buffer[ithread]+fifoBufferHeaderSize))))
					& (frameIndexMask)) >> frameIndexOffset);
		}
		break;
	}

	//start of entire acquisition
	if(!acqStarted){
		pthread_mutex_lock(&progressMutex);
		startAcquisitionIndex = startFrameIndex;
		acqStarted = true;
		pthread_mutex_unlock(&progressMutex);
		cprintf(BLUE,"Listening_Thread %d: startAcquisitionIndex:%lld\n",ithread,(long long int)startAcquisitionIndex);
	}

	//set start of scan/real time measurement
	cprintf(BLUE,"Listening_Thread %d: startFrameIndex: %lld\n", ithread,(long long int)startFrameIndex);
	measurementStarted[ithread] = true;
}







void UDPStandardImplementation::stopListening(int ithread, int numbytes){


#ifdef DEBUG4
	cprintf(BLUE,"Listening_Thread %d: Stop Listening\nStatus: %s numbytes:%d\n", ithread, runStatusType(status).c_str(),numbytes);
#endif

	//free empty buffer
	if(numbytes <= 0){
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os << "Listening "<< ithread << ": End of Acquisition";
			string message(os.str());	FILE_LOG(logINFO, message);
		}
		while(!fifoFree[ithread]->push(buffer[ithread]));
	}


	//push last non empty buffer into fifo
	else{
		if(excludeMissingPackets){
			(*((uint32_t*)(buffer[ithread]))) = numbytes/oneDataSize;
			totalListeningPacketCount[ithread] += (numbytes/oneDataSize);
		}else{
			(*((uint32_t*)(buffer[ithread]))) = numbytes/onePacketSize;
			totalListeningPacketCount[ithread] += (numbytes/onePacketSize);
		}
#ifdef DEBUG
		cprintf(BLUE,"Listening_Thread %d: Last Buffer numBytes:%d\n",ithread, numbytes);
		cprintf(BLUE,"Listening_Thread %d: Last Buffer packet count:%d\n",ithread,(*((uint32_t*)(buffer[ithread]))) );
#endif
		while(!fifo[ithread]->push(buffer[ithread]));
	}

	//push dummy-end buffer into fifo for all writer threads
	fifoFree[ithread]->pop(buffer[ithread]);

	//creating dummy-end buffer with pc=0xFFFF
	(*((uint32_t*)(buffer[ithread]))) = dummyPacketValue;
	while(!fifo[ithread]->push(buffer[ithread]));


	//reset mask and exit loop
	pthread_mutex_lock(&statusMutex);
	listeningThreadsMask^=(1<<ithread);
	pthread_mutex_unlock(&(statusMutex));
	//#ifdef DEBUG4
	sprintf(cstreambuf, "%s", " \0");
	FILE_LOG(logDEBUG, cstreambuf);
	{
		ostringstream os;
		os << "Listening Thread of " << udpPortNum[ithread] << " got " << totalListeningPacketCount[ithread] << " packets";
		string message(os.str());	FILE_LOG(logINFO, message);
	}
	//#endif

}





uint32_t UDPStandardImplementation::processListeningBuffer(int ithread, int &cSize, char* temp, int rc){


	int lastPacketOffset;		//the offset of the last packet
	uint32_t lastFrameHeader;		//frame number of last packet in buffer
	uint32_t packetCount = (rc/onePacketSize);//(packetsPerFrame/numberofListeningThreads) * numberofJobsPerBuffer;		//packets received
	cSize = 0;					//reset size

	switch(myDetectorType){
	case GOTTHARD:
	case PROPIX:
		//for short frames, 1 packet/frame, so split frames is not a topic
		if(shortFrameEnable == -1){
			lastPacketOffset = (((packetCount - 1) * onePacketSize) + fifoBufferHeaderSize);
#ifdef DEBUG4
			cprintf(BLUE, "Listening_Thread %d: Last Packet Offset:%d\n",ithread, lastPacketOffset);
#endif
			//if not last packet = split frame, then store it temporarily to combine with next buffer
			if((unsigned int)(packetsPerFrame -1) !=
					((((uint32_t)(*((uint32_t*)(buffer[ithread]+lastPacketOffset))))+1) & (packetIndexMask))){
				memcpy(temp,buffer[ithread]+lastPacketOffset, onePacketSize);
#ifdef DEBUG4
				cprintf(BLUE,"Listening_Thread %d: temp Header:%d\n",ithread,(((((uint32_t)(*((uint32_t*)(temp))))+1)
						& (frameIndexMask)) >> frameIndexOffset));
#endif
				cSize = onePacketSize;
				--packetCount;
			}
		}
#ifdef DEBUG4
		cprintf(BLUE, "Listening_Thread %d: First Header:%d\n", (((((uint32_t)(*((uint32_t*)(buffer[ithread] + fifoBufferHeaderSize))))+1)
				& (frameIndexMask)) >> frameIndexOffset));
#endif
		break;

	case MOENCH:
		lastPacketOffset = (((packetCount - 1) * onePacketSize) + fifoBufferHeaderSize);
#ifdef DEBUG4
		cprintf(BLUE, "Listening_Thread %d: First Header:%d\t First Packet:%d\t Last Header:%d\t Last Packet:%d\tLast Packet Offset:%d\n",
				(((((uint32_t)(*((uint32_t*)(buffer[ithread]+fifoBufferHeaderSize))))) & (frameIndexMask)) >> frameIndexOffset),
				((((uint32_t)(*((uint32_t*)(buffer[ithread]+fifoBufferHeaderSize))))) & (packetIndexMask)),
				(((((uint32_t)(*((uint32_t*)(buffer[ithread]+lastpacketoffset))))) & (frameIndexMask)) >> frameIndexOffset),
				((((uint32_t)(*((uint32_t*)(buffer[ithread]+lastpacketoffset))))) & (packetIndexMask)),
				lastPacketOffset);
#endif
		//moench last packet value is 0, so find the last packet and store the others in a temp storage
		if( ((((uint32_t)(*((uint32_t*)(buffer[ithread]+lastPacketOffset))))) & (packetIndexMask))){
			lastFrameHeader = ((((uint32_t)(*((uint32_t*)(buffer[ithread]+lastPacketOffset)))))
					& (frameIndexMask)) >> frameIndexOffset;
			cSize += onePacketSize;
			lastPacketOffset -= onePacketSize;
			--packetCount;
			while (lastFrameHeader == (((((uint32_t)(*((uint32_t*)(buffer[ithread]+lastPacketOffset))))) & (frameIndexMask)) >> frameIndexOffset)){
				cSize += onePacketSize;
				lastPacketOffset -= onePacketSize;
				--packetCount;
			}
			memcpy(temp, buffer[ithread]+(lastPacketOffset+onePacketSize), cSize);
#ifdef DEBUG4
			cprintf(BLUE, "Listening_Thread %d: temp Header:%d\t temp Packet:%d\n",
					(((((uint32_t)(*((uint32_t*)(temp)))))& (frameIndexMask)) >> frameIndexOffset),
					((((uint32_t)(*((uint32_t*)(temp)))))	& (packetIndexMask)));
#endif
		}
		break;

	default:
		cprintf(RED,"Listening_Thread %d: Error: This detector is not implemented in the receiver\n", ithread);
		break;
	}

#ifdef DEBUG4
	cprintf(BLUE,"Listening_Thread %d: PacketCount:%d CarryonBufferSize:%d\n",ithread, packetCount, cSize);
#endif

	return packetCount;
}






void UDPStandardImplementation::startWriting(){


	int ithread = currentThreadIndex;				//set current thread value  index
	threadStarted = 1;								//let calling function know thread started and obtained current

	char* wbuf = NULL;								//buffer popped from FIFO
	sfilefd[ithread] = 0;							//file pointer
	uint64_t nf = 0;								//for compression, number of frames


	/* outer loop - loops once for each acquisition */
	//infinite loop, exited only to change dynamic range, 10G parameters etc (then recreated again)
	while(true){

		//--reset parameters before acquisition (depending on compression)
		nf = 0;										//compression has only one listening thread (anything not eiger)


		/* inner loop - loop for each buffer */
		//until mask unset (udp sockets shut down by client)
		while((1 << ithread) & writerThreadsMask){

			//pop
			if(!dataCompressionEnable)
				fifo[ithread]->pop(wbuf);
			else
				fifo[0]->pop(wbuf);
			uint32_t numPackets = (uint32_t)(*((uint32_t*)wbuf));

#ifdef DEBUG4
			cprintf(GREEN,"Writing_Thread %d: Number of Packets: %d for FIFO %d\n", ithread, numPackets, dataCompressionEnable?0:ithread);
#endif


			//end of acquisition
			if(numPackets == dummyPacketValue){
#ifdef DEBUG4
				cprintf(GREEN,"Writing_Thread %d: Dummy frame popped out of FIFO %d",ithread, dataCompressionEnable?0:ithread);
#endif
				stopWriting(ithread,wbuf);
				continue;
			}

			//jungfrau and eiger
			if(excludeMissingPackets)
				handleCompleteFramesOnly(ithread, wbuf);
			//normal
			else if(!dataCompressionEnable)
				handleWithoutDataCompression(ithread, wbuf, numPackets);

			//compression
			else{
#if defined(MYROOT1) && defined(ALLFILE_DEBUG)
				if(npackets > 0)
					writeFileWithoutCompression(wbuf, numPackets);
#endif
				handleDataCompression(ithread,wbuf,nf);
			}
		}/*--end of loop for each buffer (inner loop)*/

		waitWritingBufferForNextAcquisition(ithread);

	}/*--end of loop for each acquisition (outer loop) */
}








void UDPStandardImplementation::waitWritingBufferForNextAcquisition(int ithread){


	//in case they are not closed already
	closeFile(ithread);
#ifdef DEBUG4
	cprintf(GREEN,"Writing_Thread %d: Done with acquisition. Waiting for 1st sem to create new file/change of parameters\n", ithread);
#endif
	//end of acquisition, wait for file create/change of parameters
	sem_wait(&writerSemaphore[ithread]);
	//check to exit thread (for change of parameters) - only EXIT possibility
	if(killAllWritingThreads){
		cprintf(GREEN,"Writing_Thread %d:Goodbye!\n",ithread);
		pthread_exit(NULL);
	}
#ifdef DEBUG4
	cprintf(GREEN,"Writing_Thread %d: Got 1st post. Creating File\n", ithread);
#endif


	//pop fifo so that its empty
	char* temp;
	while(!fifo[ithread]->isEmpty()){
		cprintf(RED,"%d:fifo emptied\n", ithread);
		fifo[ithread]->pop(temp);
		fifoFree[ithread]->push(temp);
	}

	//create file
	if((1<<ithread)&createFileMask){
		//change the detector index in the file names
		if(myDetectorType == EIGER){
			int detindex = -1;
			string tempname(fileName);
			//detid (more than 1 half module)
			size_t uscore=tempname.rfind("_");
			if (uscore!=string::npos){
				if (sscanf(tempname.substr(uscore+1,tempname.size()-uscore-1).c_str(),"d%d",&detindex)) {
					tempname=tempname.substr(0,uscore);
					sprintf(fileNamePerThread[ithread],"%s_d%d",tempname.c_str(),detindex*2+ithread);
				}
			}
			//only one half module, so no detid
			if(detindex == -1)
				sprintf(fileNamePerThread[ithread],"%s_d%d",fileName,ithread);

		}else
			strcpy(fileNamePerThread[0],fileName);

		if(dataCompressionEnable){
#ifdef MYROOT1
			pthread_mutex_lock(&writeMutex);
			fileCreateSuccess = createCompressionFile(ithread,0);
			pthread_mutex_unlock(&writeMutex);
#endif
#if (defined(MYROOT1) && defined(ALLFILE_DEBUG)) || !defined(MYROOT1)
			if(!ithread){
				//wait till its mask becomes 1 (all created except this one)
				while(createFileMask!=0x1){
					sprintf(cstreambuf, "%s", " \0");
					FILE_LOG(logDEBUG, cstreambuf);
					{
						ostringstream os;
						os << "*" << flush;
						string message(os.str());	FILE_LOG(logDEBUG4, message);
					}
					usleep(5000);
				}
				//create the normal file
				if(fileCreateSuccess != FAIL)
					fileCreateSuccess = createNewFile(0);
			}
#endif
		}else
			fileCreateSuccess = createNewFile(ithread);

		//let startwriter know file created
		pthread_mutex_lock(&statusMutex);
		createFileMask^=(1<<ithread);
		pthread_mutex_unlock(&statusMutex);
	}
#ifdef DEBUG4
	cprintf(GREEN,"Writing_Thread %d: File Created. Waiting for 2nd sem to restart acquisition\n", ithread);
#endif


	//end of acquisition, wait for restart acquisition/change of parameters
	sem_wait(&writerSemaphore[ithread]);
	//check to exit thread (for change of parameters) - only EXIT possibility
	if(killAllWritingThreads){
		cprintf(GREEN,"Writing_Thread %d:Goodbye!\n",ithread);
		//free resources at exit
		pthread_exit(NULL);
	}
#ifdef DEBUG4
	cprintf(GREEN,"Writing_Thread %d: Got 2nd post. Restarting Acquisition\n", ithread);
#endif


}



void UDPStandardImplementation::stopWriting(int ithread, char* wbuffer){

	sprintf(cstreambuf, "%s", " \0");
	FILE_LOG(logDEBUG, cstreambuf);
	{
		ostringstream os;
		os << "Writing "<< ithread << ": End of Acquisition";
		string message(os.str());	FILE_LOG(logINFO, message);
	}

	//free fifo
	while(!fifoFree[ithread]->push(wbuffer));

	if(dataStreamEnable){
		sem_wait(&writerGuiSemaphore[ithread]);				//ensure previous frame was processed
		guiNumPackets[ithread] = dummyPacketValue;
		sem_post(&dataCallbackWriterSemaphore[ithread]);	//let it know its got data
	}


	//all threads need to close file, reset mask and exit loop
	if(myDetectorType == EIGER && fileWriteEnable){
		updateFileHeader(ithread);
		fseek(sfilefd[ithread],0,0);
		fwrite((void*)fileHeader[ithread], 1, FILE_HEADER_SIZE, sfilefd[ithread]);
	}

	//Print packet loss
	//if(totalWritingPacketCountFromLastCheck[ithread]){
#ifdef VERBOSE
	if(numberofWriterThreads>1){
		printf("Thread:%d"
				"\tLost:%lld"
				"\t\tPackets:%lld"
				"\tFrame#:%lld"
				"\tPFrame#:%lld\n",
				ithread,
				((frameNumberInPreviousCheck[ithread]+1+(maxFramesPerFile/progressFrequency))>numberOfFrames)
				?(long long int)((numberOfFrames-(frameNumberInPreviousCheck[ithread]+1))*packetsPerFrame - totalWritingPacketCountFromLastCheck[ithread])
						:(long long int)((frameNumberInPreviousCheck[ithread]+(maxFramesPerFile/progressFrequency) - frameNumberInPreviousCheck[ithread])*packetsPerFrame - totalWritingPacketCountFromLastCheck[ithread]),
						 (long long int)totalWritingPacketCountFromLastCheck[ithread],
						 (long long int)currentFrameNumber[ithread],
						 (long long int)frameNumberInPreviousCheck[ithread]
		);
	}else{
		printf("Lost:%lld"
				"\t\tPackets:%lld"
				"\tFrame#:%lld"
				"\tPFrame#:%lld\n",
				((frameNumberInPreviousCheck[ithread]+1+(maxFramesPerFile/progressFrequency))>numberOfFrames)
				?(long long int)((numberOfFrames-(frameNumberInPreviousCheck[ithread]+1))*packetsPerFrame - totalWritingPacketCountFromLastCheck[ithread])
						:(long long int)((frameNumberInPreviousCheck[ithread]+(maxFramesPerFile/progressFrequency) - frameNumberInPreviousCheck[ithread])*packetsPerFrame - totalWritingPacketCountFromLastCheck[ithread]),
						 (long long int)totalWritingPacketCountFromLastCheck[ithread],
						 (long long int)currentFrameNumber[ithread],
						 (long long int)frameNumberInPreviousCheck[ithread]
		);
	}

	if(numberofWriterThreads>1){
		cprintf(BLUE,"File:%s"
				"\nThread:%d"
				"\tLost:%lld"
				"\t\tPackets:%lld"
				"\tFrame#:%lld"
				"\tPFrame#:%lld\n",
				completeFileName[ithread],ithread,
				((frameNumberInPreviousFile[ithread]+1+maxFramesPerFile)>numberOfFrames)
				?(long long int)((numberOfFrames-(frameNumberInPreviousFile[ithread]+1))*packetsPerFrame - totalPacketsInFile[ithread])
						:(long long int)((frameNumberInPreviousFile[ithread]+maxFramesPerFile - frameNumberInPreviousFile[ithread])*packetsPerFrame - totalPacketsInFile[ithread]),
						 (long long int)totalPacketsInFile[ithread],
						 (long long int)currentFrameNumber[ithread],
						 (long long int)frameNumberInPreviousFile[ithread]
		);
	}else{
		cprintf(BLUE,"File:%s"
				"\nLost:%lld"
				"\t\tPackets:%lld"
				"\tFrame#:%lld"
				"\tPFrame#:%lld\n",
				completeFileName[ithread],
				((frameNumberInPreviousFile[ithread]+1+maxFramesPerFile)>numberOfFrames)
				?(long long int)(numberOfFrames-(frameNumberInPreviousFile[ithread]+1))
						:(long long int)(frameNumberInPreviousFile[ithread]+maxFramesPerFile - frameNumberInPreviousFile[ithread]),
						 (long long int)totalPacketsInFile[ithread],
						 (long long int)currentFrameNumber[ithread],
						 (long long int)frameNumberInPreviousFile[ithread]
		);
	}
#endif
	//}

	closeFile(ithread);
	pthread_mutex_lock(&statusMutex);
	writerThreadsMask^=(1<<ithread);
#ifdef DEBUG4
	cprintf(GREEN,"Writing_Thread %d: Resetting mask. New Mask: 0x%x\n", ithread,writerThreadsMask );
#endif
	pthread_mutex_unlock(&statusMutex);


	//thread 0 waits for all threads to finish & print statistics
	if(ithread == 0){
		//wait for all other threads
		while(writerThreadsMask)
			usleep(5000);
		//ensure listening threads done before updating status as it returns to client (from stopReceiver)
		while(listeningThreadsMask)
			usleep(5000);
		//ensure datacallbacks threads are done
		while(dataCallbackThreadsMask)
			usleep(5000);
		//update status
		pthread_mutex_lock(&statusMutex);
		status = RUN_FINISHED;
		pthread_mutex_unlock(&(statusMutex));

		//statistics
		sprintf(cstreambuf, "%s", " \0");
		FILE_LOG(logDEBUG, cstreambuf);
		{
			ostringstream os;
			os << "Status: Run Finished";
			string message(os.str());	FILE_LOG(logINFO, message);
		}
		for(int i=0;i<numberofListeningThreads;i++){

			if(totalWritingPacketCount[i] < ((uint64_t)numberOfFrames*packetsPerFrame)){
				cprintf(RED, "\nPort %d\n",udpPortNum[i]);
				if(excludeMissingPackets){
					cprintf(RED, "Ignored Packets   \t: %lld\n",(long long int)totalIgnoredPacketCount[i]);
					cprintf(RED, "Missing Packets   \t: %lld\n",(long long int)numberOfFrames*packetsPerFrame-totalWritingPacketCount[i]-totalIgnoredPacketCount[i]);
				}else
					cprintf(RED, "Missing Packets   \t: %lld\n",(long long int)numberOfFrames*packetsPerFrame-totalWritingPacketCount[i]);
				cprintf(RED, "Packets Caught \t\t: %lld\n",(long long int)totalWritingPacketCount[i]);
				cprintf(RED, "Frames Caught  \t\t: %lld\n",(long long int)(totalWritingPacketCount[i]/packetsPerFrame));
				int64_t lastFrameNumber = 0;
				lastFrameNumber = lastFrameNumberInFile[i];//lastFrameNumberInFile updated even if not written
				if(myDetectorType == EIGER)
					lastFrameNumber= lastFrameNumberInFile[i] - startFrameIndex + 1;

				cprintf(RED, "Last Frame Number Caught :%lld\n",(long long int)lastFrameNumber);
			}else{
				cprintf(GREEN, "\nPort %d\n",udpPortNum[i]);
				if(excludeMissingPackets){
					cprintf(GREEN, "Ignored Packets   \t: %lld\n",(long long int)totalIgnoredPacketCount[i]);
					cprintf(GREEN, "Missing Packets   \t: %lld\n",(long long int)numberOfFrames*packetsPerFrame-totalWritingPacketCount[i]-totalIgnoredPacketCount[i]);
				}else
					cprintf(GREEN, "Missing Packets   \t: %lld\n",(long long int)numberOfFrames*packetsPerFrame-totalWritingPacketCount[i]);
				cprintf(GREEN, "Packets Caught \t\t: %lld\n",(long long int)totalWritingPacketCount[i]);
				cprintf(GREEN, "Frames Caught  \t\t: %lld\n",(long long int)(totalWritingPacketCount[i]/packetsPerFrame));
				int64_t lastFrameNumber = 0;
				lastFrameNumber = lastFrameNumberInFile[i];//lastFrameNumberInFile updated even if not written
				if(myDetectorType == EIGER)
					lastFrameNumber= lastFrameNumberInFile[i] - startFrameIndex + 1;
				cprintf(GREEN, "Last Frame Number Caught: %lld\n",(long long int)lastFrameNumber);
			}

		}
		if(!activated)
			cprintf(RED,"Note: Deactivated Receiver\n");
		//acquisition end
		if (acquisitionFinishedCallBack)
			acquisitionFinishedCallBack((totalPacketsCaught/(packetsPerFrame*numberofListeningThreads)), pAcquisitionFinished);
	}
}




void UDPStandardImplementation::handleWithoutDataCompression(int ithread, char* wbuffer, uint32_t npackets){


	//get current frame number
	uint64_t tempframenumber;
	uint32_t pnum;
	uint32_t snum;
	uint64_t bunchid;
	if(getFrameandPacketNumber(ithread, wbuffer + fifoBufferHeaderSize,tempframenumber,pnum,snum,bunchid) == FAIL){
		//error in frame number sent by fpga
		while(!fifoFree[ithread]->push(wbuffer));

		return;
	}


	//callback to write data
	if (rawDataReadyCallBack)
		rawDataReadyCallBack(
				tempframenumber,//frameNumber
				0,//expLength
				0,//packetNumber
				0,//bunchId
				0,//timestamp
				0,//modId
				detID*numberofListeningThreads+ithread,//xCoord
				0,//yCoord
				0,//zCoord
				0,//debug
				0,//roundRNumber
				(uint8_t)myDetectorType,//detType
				SLS_DETECTOR_HEADER_VERSION,//version
				wbuffer + fifoBufferHeaderSize,
				bufferSize * numberofJobsPerBuffer + fifoBufferHeaderSize,
				pRawDataReady);//know which thread from sfilefd



	//write to file if enabled and update write parameters
	if(npackets > 0)
		writeFileWithoutCompression(ithread, wbuffer, npackets);
#ifdef DEBUG4
	cprintf(GREEN,"Writing_Thread: Writing done\nGoing to copy frame\n");
#endif


	//copy frame for gui
	//if(npackets >= (packetsPerFrame/numberofListeningThreads))
	if(dataStreamEnable && npackets > 0)
		copyFrameToGui(ithread, wbuffer,npackets);
#ifdef DEBUG4
	cprintf(GREEN,"Writing_Thread: Copied frame\n");
#endif


	//free fifo addresses
	int listenfifoThread = ithread;
	if(dataCompressionEnable)
		listenfifoThread = 0;
	while(!fifoFree[listenfifoThread]->push(wbuffer));
#ifdef EVERYFIFODEBUG
	if(fifoFree[listenfifoThread]->getSemValue()<100)
		cprintf(GREEN,"FifoFree[%d]: value:%d, push 0x%x\n",listenfifoThread,fifoFree[listenfifoThread]->getSemValue(),(void*)(wbuffer));
#endif
#ifdef DEBUG5
	cprintf(GREEN,"Writing_Thread %d: Freed buffer, pushed into fifofree %p for listener %d \n",listenfifoThread, (void*)(wbuffer), listenfifoThread);
#endif

}




void UDPStandardImplementation::handleCompleteFramesOnly(int ithread, char* wbuffer){


	//get header
	sls_detector_header* header = (sls_detector_header*) (wbuffer + HEADER_SIZE_NUM_TOT_PACKETS);
	uint64_t tempframenumber = header->frameNumber;

	if (rawDataReadyCallBack)
		rawDataReadyCallBack(
				header->frameNumber,
				header->expLength,
				header->packetNumber,
				header->bunchId,
				header->timestamp,
				header->modId,
				header->xCoord,
				header->yCoord,
				header->zCoord,
				header->debug,
				header->roundRNumber,
				header->detType,
				header->version,
				wbuffer + fifoBufferHeaderSize,
				bufferSize * numberofJobsPerBuffer + fifoBufferHeaderSize,
				pRawDataReady);


	//write to file if enabled and update write parameters
	if((fileWriteEnable) && (sfilefd[ithread])){
		if(tempframenumber && (tempframenumber%maxFramesPerFile) == 0)
			createNewFile(ithread);
		fwrite(wbuffer + HEADER_SIZE_NUM_TOT_PACKETS, 1, (bufferSize + sizeof(sls_detector_header)), sfilefd[ithread]);
	}

	tempframenumber -= startFrameIndex;

	//progress
	if(tempframenumber &&  (tempframenumber%(maxFramesPerFile/progressFrequency)) == 0){
		if(numberofWriterThreads>1){
			printf("Thread:%d"
					"\tLost:%lld"
					"\t\tPackets:%lld"
					"\tFrame#:%lld"
					"\tPFrame#:%lld\n",
					ithread,
					((frameNumberInPreviousCheck[ithread]+1+(maxFramesPerFile/progressFrequency))>numberOfFrames)
					?(long long int)((numberOfFrames-(frameNumberInPreviousCheck[ithread]+1))*packetsPerFrame - totalWritingPacketCountFromLastCheck[ithread])
							:(long long int)((frameNumberInPreviousCheck[ithread]+(maxFramesPerFile/progressFrequency) - frameNumberInPreviousCheck[ithread])*packetsPerFrame - totalWritingPacketCountFromLastCheck[ithread]),
							 (long long int)totalWritingPacketCountFromLastCheck[ithread],
							 (long long int)currentFrameNumber[ithread],
							 (long long int)frameNumberInPreviousCheck[ithread]
			);
		}else{
			printf("Lost:%lld"
					"\t\tPackets:%lld"
					"\tFrame#:%lld"
					"\tPFrame#:%lld\n",
					((frameNumberInPreviousCheck[ithread]+1+(maxFramesPerFile/progressFrequency))>numberOfFrames)
					?(long long int)((numberOfFrames-(frameNumberInPreviousCheck[ithread]+1))*packetsPerFrame - totalWritingPacketCountFromLastCheck[ithread])
							:(long long int)((frameNumberInPreviousCheck[ithread]+(maxFramesPerFile/progressFrequency) - frameNumberInPreviousCheck[ithread])*packetsPerFrame - totalWritingPacketCountFromLastCheck[ithread]),
							 (long long int)totalWritingPacketCountFromLastCheck[ithread],
							 (long long int)currentFrameNumber[ithread],
							 (long long int)frameNumberInPreviousCheck[ithread]
			);
		}
		//reset counters for each new file
		frameNumberInPreviousCheck[ithread] = currentFrameNumber[ithread];
		totalWritingPacketCountFromLastCheck[ithread] = 0;
	}

	totalWritingPacketCountFromLastCheck[ithread]+= packetsPerFrame;
	totalPacketsInFile[ithread] += packetsPerFrame;
	totalWritingPacketCount[ithread] += packetsPerFrame;
	lastFrameNumberInFile[ithread] = tempframenumber;
	currentFrameNumber[ithread] = tempframenumber;
	//cout<<"curentframenumber:"<<currentFrameNumber[ithread]<<endl;

	if(numberofWriterThreads > 1)
		pthread_mutex_lock(&writeMutex);

	packetsCaught += packetsPerFrame;
	totalPacketsCaught += packetsPerFrame;
	if((currentFrameNumber[ithread] - startAcquisitionIndex) > acquisitionIndex)
		acquisitionIndex = currentFrameNumber[ithread] - startAcquisitionIndex;
	if((currentFrameNumber[ithread] - startFrameIndex) > frameIndex[ithread])
		frameIndex[ithread] = currentFrameNumber[ithread] - startFrameIndex;

	if(numberofWriterThreads > 1)
		pthread_mutex_unlock(&writeMutex);

	if(!activated)
		currentFrameNumber[ithread]++;

#ifdef DEBUG4
	cprintf(GREEN,"Writing_Thread: Writing done\nGoing to copy frame\n");
#endif


	//copy frame for gui
	if(dataStreamEnable)
		copyFrameToGui(ithread, wbuffer, packetsPerFrame);
#ifdef DEBUG4
	cprintf(GREEN,"Writing_Thread: Copied frame\n");
#endif


	//free fifo addresses
	while(!fifoFree[ithread]->push(wbuffer));
#ifdef DEBUG5
	cprintf(GREEN,"Writing_Thread %d: Freed buffer, pushed into fifofree %p for listener %d \n",ithread, (void*)(wbuffer), ithread);
#endif

}



void UDPStandardImplementation::writeFileWithoutCompression(int ithread, char* wbuffer,uint32_t numpackets){


	//if write enabled
	if((fileWriteEnable) && (sfilefd[ithread])){
		if(numpackets){
			int offset = fifoBufferHeaderSize;
			uint64_t nextFileFrameNumber;
			int packetsWritten = 0;
			//if(ithread) cout<<"numpackets:"<<numpackets<<"lastframenumberinfile:"<<lastFrameNumberInFile[ithread]<<endl;

			//handle half frames from previous buffer
			//second part to not check when there has been something written previously
			if(numpackets &&(lastFrameNumberInFile[ithread]>=0)){
				//get start frame (required to create new file at the right juncture)
				uint64_t startframe = 0;
				uint32_t pnum = 0;
				uint32_t snum = 0;
				uint64_t bunchid = 0;
				//if(ithread) cout<<"getting start frame number"<<endl;
				if(getFrameandPacketNumber(ithread, wbuffer + offset, startframe,pnum,snum,bunchid) == FAIL){
					//error in frame number sent by fpga
					while(!fifoFree[ithread]->push(wbuffer));
					return;
				}
				//if(ithread) cout<<"done getting start frame number"<<endl;
				if(startframe == (unsigned long long int)lastFrameNumberInFile[ithread]){
					if(writeUptoFrameNumber(ithread, wbuffer, offset, startframe+1, numpackets, packetsWritten) == FAIL)
						//weird frame number of zero from fpga
						return;

					//update stats
					numpackets -= packetsWritten;
					totalPacketsInFile[ithread]  += packetsWritten;
					totalWritingPacketCount[ithread]  += packetsWritten;
					pthread_mutex_lock(&writeMutex);
					packetsCaught += packetsWritten;
					totalPacketsCaught  += packetsWritten;
					pthread_mutex_unlock(&writeMutex);
				}
			}


			while(numpackets){
				//new file
				//create new file only if something has been written and modulus works
				if((lastFrameNumberInFile[ithread]>=0) &&(!((lastFrameNumberInFile[ithread]+1) % maxFramesPerFile)))
					createNewFile(ithread);


				//frames to save in one file
				nextFileFrameNumber =   (lastFrameNumberInFile[ithread]+1) +
						(maxFramesPerFile - ((lastFrameNumberInFile[ithread]+1)%maxFramesPerFile));

				if(writeUptoFrameNumber(ithread, wbuffer, offset, nextFileFrameNumber, numpackets, packetsWritten) == FAIL)
					//weird frame number of zero from fpga
					return;

				//update stats
				numpackets -= packetsWritten;
				totalPacketsInFile[ithread]  += packetsWritten;
				totalWritingPacketCount[ithread]  += packetsWritten;
				pthread_mutex_lock(&writeMutex);
				packetsCaught += packetsWritten;
				totalPacketsCaught  += packetsWritten;
				pthread_mutex_unlock(&writeMutex);
				currentFrameNumber[ithread] = lastFrameNumberInFile[ithread];
			}
		}
	}

	//only update parameters
	else{

		if(numpackets){
			//get last frame number
			uint64_t finalLastFrameNumberToSave = 0;
			uint32_t pnum;
			uint32_t snum;
			uint64_t bunchid = 0;
			if(getFrameandPacketNumber(ithread, wbuffer + fifoBufferHeaderSize + ((numpackets - 1) * onePacketSize), finalLastFrameNumberToSave,pnum,snum,bunchid) == FAIL){
				//error in frame number sent by fpga
				while(!fifoFree[ithread]->push(wbuffer));
				return;
			}
			totalPacketsInFile[ithread] += numpackets;
			totalWritingPacketCount[ithread] += numpackets;
			lastFrameNumberInFile[ithread] = finalLastFrameNumberToSave;
			currentFrameNumber[ithread] = finalLastFrameNumberToSave;

		}

		if(numberofWriterThreads > 1) pthread_mutex_lock(&writeMutex);
		packetsCaught += numpackets;
		totalPacketsCaught += numpackets;
		if(numberofWriterThreads > 1) pthread_mutex_unlock(&writeMutex);
	}

	//set indices
	pthread_mutex_lock(&progressMutex);
	if((currentFrameNumber[ithread] - startAcquisitionIndex) > acquisitionIndex)
		acquisitionIndex = currentFrameNumber[ithread] - startAcquisitionIndex;
	if((currentFrameNumber[ithread] - startFrameIndex) > frameIndex[ithread])
		frameIndex[ithread] = currentFrameNumber[ithread] - startFrameIndex;
	pthread_mutex_unlock(&progressMutex);
}






void UDPStandardImplementation::updateFileHeader(int ithread){
	//update file header
	time_t t = time(0);
	sprintf(fileHeader[ithread],
			"\nHeader\t\t: %d bytes\n"
			"Top\t\t: %d\n"
			"Left\t\t: %d\n"
			"Active\t\t: %d\n"
			"Frames Caught\t: %lld\n"
			"Frames Lost\t: %lld\n"
			"Dynamic Range\t: %d\n"
			"Ten Giga\t: %d\n"
			"Image Size\t: %d bytes\n"
			"x\t\t: %d pixels\n"
			"y\t\t: %d pixels\n"
			"Total Frames\t: %lld\n"
			"Exptime (ns)\t: %lld\n"
			"Period (ns)\t: %lld\n"
			"Timestamp\t: %s\n\n"

			"#Frame Header\n"
			"Frame Number       : 8 bytes\n"
			"Exposure Length    : 4 bytes\n"
			"Packet Number      : 4 bytes\n"
			"Bunch ID           : 8 bytes\n"
			"Timestamp          : 8 bytes\n"
			"Module Id          : 2 bytes\n"
			"X Coordinate       : 2 bytes\n"
			"Y Coordinate       : 2 bytes\n"
			"Z Coordinate       : 2 bytes\n"
			"Debug              : 4 bytes\n"
			"Round Robin Number : 2 bytes\n"
			"Detector Type      : 1 byte\n"
			"Header Version     : 1 byte\n"
			,
			FILE_HEADER_SIZE,
			(flippedData[0]?0:1),
			(ithread?0:1),
			activated,
			(long long int)(totalPacketsInFile[ithread]/packetsPerFrame),
			((frameNumberInPreviousFile[ithread]+1+maxFramesPerFile)>numberOfFrames)
			?(long long int)((numberOfFrames-(frameNumberInPreviousFile[ithread]+1)) - (totalPacketsInFile[ithread]/packetsPerFrame))
					:(long long int)((frameNumberInPreviousFile[ithread]+maxFramesPerFile - frameNumberInPreviousFile[ithread]) - (totalPacketsInFile[ithread]/packetsPerFrame)),
					 dynamicRange,tengigaEnable,
					 bufferSize,
					 //only for eiger right now
					 EIGER_PIXELS_IN_ONE_ROW,EIGER_PIXELS_IN_ONE_COL,
					 (long long int)numberOfFrames,
					 (long long int)acquisitionTime,
					 (long long int)acquisitionPeriod,
					 ctime(&t));
	if(strlen(fileHeader[ithread]) > FILE_HEADER_SIZE)
		cprintf(BG_RED,"File Header Size %d is too small for fixed file header size %d\n",(int)strlen(fileHeader[ithread]),(int)FILE_HEADER_SIZE);


}

//called only if datacallback enabled
void UDPStandardImplementation::copyFrameToGui(int ithread, char* buffer, uint32_t numpackets){


	//if nthe frame, wait for your turn (1st frame always shown as its zero)
	if(frameToGuiFrequency && ((frametoGuiCounter[ithread])%frameToGuiFrequency));

	//random read (gui ready) or nth frame read: gui needs data now or it is the first frame
	else{

#ifdef DEBUG4
		cprintf(GREEN,"Writing_Thread: CopyingFrame: Going to copy data\n");
#endif
		//ensure previous frame was processed
		sem_wait(&writerGuiSemaphore[ithread]);

		//copy date
		guiNumPackets[ithread] = numpackets;
		strcpy(guiFileName[ithread],completeFileName[ithread]);

		if(excludeMissingPackets) //copy also the header
			memcpy(latestData[ithread],buffer+ HEADER_SIZE_NUM_TOT_PACKETS, bufferSize + sizeof(sls_detector_header));
		else //copy only the data
			memcpy(latestData[ithread],buffer+ fifoBufferHeaderSize , numpackets*onePacketSize);
		//let it know its got data
		sem_post(&dataCallbackWriterSemaphore[ithread]);

#ifdef DEBUG4
		cprintf(GREEN,"Writing_Thread: CopyingFrame: Copied Data\n");
#endif

	}

	//update the counter for nth frame
	if(frameToGuiFrequency)
		frametoGuiCounter[ithread]++;


}





void UDPStandardImplementation::handleDataCompression(int ithread, char* wbuffer, uint64_t &nf){


	//get frame number
	uint64_t tempframenumber=-1;
	uint32_t pnum;
	uint32_t snum;
	uint64_t bunchid=-1;
	if(getFrameandPacketNumber(ithread, wbuffer + fifoBufferHeaderSize, tempframenumber,pnum,snum,bunchid) == FAIL){
		//error in frame number sent by fpga
		while(!fifoFree[ithread]->push(wbuffer));
		return;
	}
	currentFrameNumber[ithread] =  tempframenumber;


	//set indices
	pthread_mutex_lock(&progressMutex);
	if((currentFrameNumber[ithread] - startAcquisitionIndex) > acquisitionIndex)
		acquisitionIndex = currentFrameNumber[ithread] - startAcquisitionIndex;
	if((currentFrameNumber[ithread] - startFrameIndex) > frameIndex[ithread])
		frameIndex[ithread] = currentFrameNumber[ithread] - startFrameIndex;
	pthread_mutex_unlock(&progressMutex);

	//variable definitions
	char* buff[2]={0,0};										//an array just to be compatible with copyframetogui
	char* data = wbuffer+ fifoBufferHeaderSize;	//data pointer to the next memory to be analysed
	int ndata;												//size of data returned
	uint32_t np;													//remaining number of packets returned
	uint32_t npackets = (uint32_t)(*((uint32_t*)wbuffer));	//number of total packets
	int remainingsize = npackets * onePacketSize;			//size of the memory slot to be analyzed

	eventType thisEvent = PEDESTAL;
	int once = 0;
	int xmax = 0, ymax = 0;									//max pixels in x and y direction
	int xmin = 1, ymin = 1;									//min pixels in x and y direction

	//determining xmax and ymax
	switch(myDetectorType){
	case MOENCH:
		xmax = MOENCH_PIXELS_IN_ONE_ROW-1;
		ymax = MOENCH_PIXELS_IN_ONE_ROW-1;
		break;
	case GOTTHARD:
		if(shortFrameEnable == -1){
			xmax = GOTTHARD_PIXELS_IN_ROW-1;
			ymax = GOTTHARD_PIXELS_IN_COL-1;
		}else{
			xmax = GOTTHARD_SHORT_PIXELS_IN_ROW-1;
			ymax = GOTTHARD_SHORT_PIXELS_IN_COL-1;
		}
		break;
	default:
		break;
	}

	while((buff[0] = receiverData[ithread]->findNextFrame(data,ndata,remainingsize))){

		//remaining number of packets
		np = ndata/onePacketSize;

		if ((np == packetsPerFrame) && (buff[0]!=NULL)){
			if(nf == 1000)
				cprintf(GREEN, "Writing_Thread %d: pedestal done\n", ithread);

			singlePhotonDetectorObject[ithread]->newFrame();

			//only for moench
			if(commonModeSubtractionEnable){
				for(int ix = xmin - 1; ix < xmax+1; ix++){
					for(int iy = ymin - 1; iy < ymax+1; iy++){
						thisEvent = singlePhotonDetectorObject[ithread]->getEventType(buff[0], ix, iy, 0);
					}
				}
			}


			for(int ix = xmin - 1; ix < xmax+1; ix++)
				for(int iy = ymin - 1; iy < ymax+1; iy++){
					thisEvent=singlePhotonDetectorObject[ithread]->getEventType(buff[0], ix, iy, commonModeSubtractionEnable);
					if (nf>1000) {
						if (thisEvent==PHOTON_MAX) {
							receiverData[ithread]->getFrameNumber(buff[0]);
							//iFrame=receiverData[ithread]->getFrameNumber(buff);
#ifdef MYROOT1
							myTree[ithread]->Fill();
							//cout << "Fill in event: frmNr: " << iFrame <<  " ix " << ix << " iy " << iy << " type " <<  thisEvent << endl;
#else
							pthread_mutex_lock(&writeMutex);
							if((fileWriteEnable) && (sfilefd[0]))
								singlePhotonDetectorObject[ithread]->writeCluster(sfilefd[0]);
							pthread_mutex_unlock(&writeMutex);
#endif
						}
					}
				}

			nf++;


#ifndef ALLFILE
			totalPacketsInFile[ithread] += (bufferSize/packetsPerFrame);
			totalWritingPacketCount[ithread] += (bufferSize/packetsPerFrame);
			pthread_mutex_lock(&writeMutex);
			if((packetsCaught%packetsPerFrame) >= (uint32_t)maxFramesPerFile)
				createNewFile(ithread);
			packetsCaught += (bufferSize/packetsPerFrame);
			totalPacketsCaught += (bufferSize/packetsPerFrame);
			pthread_mutex_unlock(&writeMutex);


#endif
			if(!once){
				if(dataStreamEnable)
					copyFrameToGui(ithread, buff[0],(uint32_t)packetsPerFrame);
				once = 1;
			}
		}

		remainingsize -= ((buff[0] + ndata) - data);
		data = buff[0] + ndata;
		if(data > (wbuffer + fifoBufferHeaderSize + npackets * onePacketSize) )
			cprintf(BG_RED,"Writing_Thread %d: Error: Compression data goes out of bounds!\n", ithread);
	}


	while(!fifoFree[0]->push(wbuffer));
#ifdef EVERYFIFODEBUG
	if(fifoFree[0]->getSemValue()<100)
		cprintf(GREEN,"FifoFree[%d]: value:%d, push 0x%x\n",0,fifoFree[0]->getSemValue(),(void*)(wbuffer));
#endif
#ifdef DEBUG5
	cprintf(GREEN,"Writing_Thread %d: Compression free pushed into fifofree %p for listerner 0\n", ithread, (void*)(wbuffer));
#endif
}



int UDPStandardImplementation::getFrameandPacketNumber(int ithread, char* wbuffer, uint64_t &framenumber, uint32_t &packetnumber,uint32_t &subframenumber, uint64_t &bunchid){


	eiger_packet_footer_t* footer=0;
	eiger_packet_header_t* e_header=0;
	jfrau_packet_header_t* header=0;
	framenumber = 0;
	packetnumber = 0;
	subframenumber = 0;
	bunchid = 0;

	switch(myDetectorType){

	case EIGER:
		footer = (eiger_packet_footer_t*)(wbuffer + footerOffset);
		framenumber = (uint32_t)(*( (uint64_t*) footer));
		//error in frame number sent by fpga
		if(((uint32_t)(*( (uint64_t*) footer)))==0){
			framenumber = 0;
			sprintf(cstreambuf, "%s", " \0");
			FILE_LOG(logDEBUG, cstreambuf);
			{
				ostringstream os;
				os << "Fifo "<< ithread << ": Frame Number is zero from firmware.";
				string message(os.str());	FILE_LOG(logERROR, message);


			}
			if(((uint32_t)(*( (uint64_t*) footer)))==0)
			{
				sprintf(cstreambuf, "%s", " \0");
				FILE_LOG(logDEBUG, cstreambuf);
			  packetnumber = (*( (uint16_t*) footer->packetNumber))-1;
				ostringstream os;
				os << "Fifo "<< ithread << ": Frame Number is still zero from firmware. and pnum:" << packetnumber << "udp port num:" << udpPortNum[ithread];
				string message(os.str());	FILE_LOG(logERROR, message);

			}
			return FAIL;
		}
		packetnumber = (*( (uint16_t*) footer->packetNumber))-1;
		e_header = (eiger_packet_header_t*) (wbuffer);
		subframenumber = *( (uint32_t*) e_header->subFrameNumber);
#ifdef DEBUG4
		if(!ithread) cprintf(GREEN,"Writing_Thread %d: fnum:%lld pnum:%d FPGA_fnum:%d subfnum:%d footeroffset:%d\n",
				ithread,
				(long long int)framenumber,
				packetnumber,
				framenumber,
				subframenumber,
				footerOffset);
#endif
		framenumber -= startFrameIndex;
		break;

	case JUNGFRAU:
		header = (jfrau_packet_header_t*)(wbuffer);
		framenumber = (*( (uint32_t*) header->frameNumber))&frameIndexMask;
		packetnumber = (uint32_t)(*( (uint8_t*) header->packetNumber));
		bunchid = (*((uint64_t*) header->bunchid));
#ifdef DEBUG4
		cprintf(GREEN, "Writing_Thread %d: fnum:%lld\t pnum:%d bunchid:%lld\n",
				(long long int)framenumber,
				packetnumber,
				(long long int)bunchid);
#endif
		framenumber -= startFrameIndex;
		break;

	default:
		framenumber = ((uint32_t)(*((uint32_t*)(wbuffer))));
		//for gotthard and normal frame, increment frame number to separate fnum and pnum
		if (myDetectorType == PROPIX ||(myDetectorType == GOTTHARD && shortFrameEnable == -1))
			framenumber++;
		packetnumber = framenumber&packetIndexMask;
		framenumber = (framenumber & frameIndexMask) >> frameIndexOffset;
#ifdef DEBUG4
		cprintf(GREEN, "Writing_Thread %d: fnum:%lld\t pnum:%d\n",
				(long long int)framenumber,
				packetnumber);
#endif
		framenumber -= startFrameIndex;
		break;
	}
	return OK;
}





int UDPStandardImplementation::writeUptoFrameNumber(int ithread, char* wbuffer, int &offset, uint64_t nextFrameNumber, uint32_t numpackets, int &numPacketsWritten){

	//if(ithread) cout<<"at writeUptoFrameNumber " << nextFrameNumber<< endl;


	int startoffset = offset;
	int endoffset = startoffset + numpackets * onePacketSize;
	uint64_t tempframenumber=-1;
	offset = endoffset;
	uint32_t pnum;
	uint32_t snum;
	uint64_t bunchid=-1;
	//get last frame number
	if(getFrameandPacketNumber(ithread, wbuffer + (endoffset-onePacketSize), tempframenumber,pnum,snum,bunchid) == FAIL){
		//error in frame number sent by fpga
		while(!fifoFree[ithread]->push(wbuffer));
		return FAIL;
	}
	//last packet's frame number < nextframenumber
	if(tempframenumber<nextFrameNumber){
		fwrite(wbuffer + startoffset, 1, offset-startoffset, sfilefd[ithread]);
		numPacketsWritten += ((offset-startoffset)/onePacketSize);
		lastFrameNumberInFile[ithread] = tempframenumber;
		return OK;
	}



	//somewhere in between
	int bigIncrements = onePacketSize * packetsPerFrame * 10; //10 frames at a time
	if(numberofJobsPerBuffer == 1)	bigIncrements = onePacketSize; //a packet at a time as we listen to only one frame in a buffer

	cout<<ithread<<" lastFrameNumberInFile:"<<lastFrameNumberInFile[ithread]<<endl;
	cout<<ithread<<" nextFeame number:"<<nextFrameNumber<<endl;
	cout<<ithread<<" tempframenumber:"<<tempframenumber<<endl;
	while(tempframenumber>=nextFrameNumber){
		offset -= bigIncrements;
		if(offset<startoffset)
			break;//if(ithread) cout<<"frame number at going backwards fast f#:"<<tempframenumber<< " offset:"<<offset<<endl;
		if(getFrameandPacketNumber(ithread, wbuffer + offset, tempframenumber,pnum,snum,bunchid) == FAIL){
			//error in frame number sent by fpga
			while(!fifoFree[ithread]->push(wbuffer));
			return FAIL;
		}
	}
	if(offset<startoffset){
		offset = startoffset;//if(ithread) cout<<"offset < start offset f#:"<<tempframenumber<< " offset:"<<offset<<endl;
		if(getFrameandPacketNumber(ithread, wbuffer + offset, tempframenumber,pnum,snum,bunchid) == FAIL){
			//error in frame number sent by fpga
			while(!fifoFree[ithread]->push(wbuffer));
			return FAIL;
		}
	}
	while(tempframenumber<nextFrameNumber){
		offset += onePacketSize;//if(ithread) cout<<"frame number at going forwards slow f#:"<<tempframenumber<< " offset:"<<offset<<endl;
		if(getFrameandPacketNumber(ithread, wbuffer + offset, tempframenumber,pnum,snum,bunchid) == FAIL){
			//error in frame number sent by fpga
			while(!fifoFree[ithread]->push(wbuffer));
			return FAIL;
		}
	}


	fwrite(wbuffer + startoffset, 1, offset-startoffset, sfilefd[ithread]);
	numPacketsWritten += ((offset-startoffset)/onePacketSize);
	lastFrameNumberInFile[ithread] = (nextFrameNumber-1);
	//if(ithread) cout<<"done with writeUptoFrameNumber" << endl;
	return OK;
}








