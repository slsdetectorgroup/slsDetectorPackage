/********************************************//**
 * @file UDPStandardImplementation.cpp
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 ***********************************************/


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

using namespace std;

#define WRITE_HEADERS

/*************************************************************************
 * Constructor & Destructor **********************************************
 * They access local cache of configuration or detector parameters *******
 *************************************************************************/

UDPStandardImplementation::UDPStandardImplementation(){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	initializeMembers();

	//***mutex***
	pthread_mutex_init(&statusMutex,NULL);
	pthread_mutex_init(&writeMutex,NULL);
	pthread_mutex_init(&dataReadyMutex,NULL);
	pthread_mutex_init(&progressMutex,NULL);

	//to increase socket receiver buffer size and max length of input queue by changing kernel settings
	if(myDetectorType == EIGER);
	else if(system("echo $((100*1024*1024)) > /proc/sys/net/core/rmem_max")){
		FILE_LOG(logDEBUG) << "Warning: No root permission to change socket receiver buffer size in file /proc/sys/net/core/rmem_max";
	}else if(system("echo 250000 > /proc/sys/net/core/netdev_max_backlog")){
		FILE_LOG(logDEBUG) << "Warning: No root permission to change max length of input queue in file /proc/sys/net/core/netdev_max_backlog";
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
	FILE_LOG(logDEBUG) << __AT__ << " called";
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
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	FILE_LOG(logDEBUG) << "Info: Deleting member pointers";
	shutDownUDPSockets();
	for(int i=0;i<MAX_NUMBER_OF_WRITER_THREADS; i++)
		closeFile(i);
	//filter
	deleteFilter();
	for(int i=0; i<MAX_NUMBER_OF_LISTENING_THREADS; i++){
		if(mem0[i])			{free(mem0[i]);			mem0[i] = NULL;}
		if(fifo[i])			{delete fifo[i];		fifo[i] = NULL;}
		if(fifoFree[i])		{delete fifoFree[i];	fifoFree[i] = NULL;}
	}
	for(int i=0; i<MAX_NUMBER_OF_WRITER_THREADS; i++){
		if(latestData[i]) 	{delete[] latestData[i]; 	latestData[i] = NULL;}
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
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	moenchCommonModeSubtraction = NULL;
	for(int i=0; i<MAX_NUMBER_OF_WRITER_THREADS; i++){
			if(singlePhotonDetectorObject[i]){
				delete []singlePhotonDetectorObject[i];
				singlePhotonDetectorObject[i] = NULL;
			}
			if(receiverData[i]){
				delete []receiverData[i];
				receiverData[i] = NULL;
			}
		}
}

void UDPStandardImplementation::initializeBaseMembers(){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	UDPBaseImplementation::initializeMembers();
	acquisitionPeriod = SAMPLE_TIME_IN_NS;
}


void UDPStandardImplementation::initializeMembers(){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	FILE_LOG(logDEBUG) << "Info: Initializing members";

	//***detector parameters***
	detID = 0;
	bufferSize = 0;
	onePacketSize = 0;
	oneDataSize = 0;
	frameIndexMask = 0;
	frameIndexOffset = 0;
	packetIndexMask = 0;
	footerOffset = 0;

	//***file parameters***
#ifdef MYROOT1
	for(int i=0; i<MAX_NUMBER_OF_WRITER_THREADS; i++){
		myTree[i] = (NULL);
		myFile[i] = (NULL);
	}
#endif
	for(int i=0; i<MAX_NUMBER_OF_WRITER_THREADS; i++){
		strcpy(completeFileName[i],"");
		strcpy(fileHeader[i],"");
		sfilefd[i] = NULL;
	}
	maxFramesPerFile = 0;
	fileCreateSuccess = false;

	//***acquisition indices parameters***
	startAcquisitionIndex = 0;
	acqStarted = false;
	startFrameIndex = 0;
	for(int i = 0; i < MAX_NUMBER_OF_LISTENING_THREADS; ++i){
		measurementStarted[i] = false;
		totalListeningPacketCount[i] = 0;
	}
	for(int i=0; i<MAX_NUMBER_OF_WRITER_THREADS; i++){
		frameIndex[i] = 0;
		currentFrameNumber[i] = 0;
		frameNumberInPreviousFile[i] = 0;
		lastFrameNumberInFile[i] = -1;
		totalPacketsInFile[i] = 0;
		totalWritingPacketCount[i] = 0;
	}



	//***receiver parameters***
	for(int i=0; i < MAX_NUMBER_OF_LISTENING_THREADS; i++){
		buffer[i] = NULL;
		mem0[i] = NULL;
		fifo[i] = NULL;
		fifoFree[i] = NULL;
		udpSocket[i] = NULL;
	}
	numberofJobsPerBuffer = -1;
	fifoSize = 0;

	//***receiver to GUI parameters***
	for(int i=0; i<MAX_NUMBER_OF_WRITER_THREADS; i++){
		latestData[i] = NULL;
		guiNumPackets[i] = 0;
		strcpy(guiFileName[i],"");
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


	//***deactivated parameters***
	for(int i=0; i < MAX_NUMBER_OF_LISTENING_THREADS; i++)
		deactivatedFrameNumber[i] = 0;
	deactivatedFrameIncrement = 0;

	//***filter parameters***
	commonModeSubtractionEnable = false;
	moenchCommonModeSubtraction = NULL;
	for(int i=0; i<MAX_NUMBER_OF_WRITER_THREADS; i++){
		singlePhotonDetectorObject[i] = NULL;
		receiverData[i] = NULL;
	}


	//***callback***
	cbAction = DO_EVERYTHING;
}



void UDPStandardImplementation::initializeFilter(){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	double hc = 0, sigma = 5;
	int sign = 1, csize, i;

	//common mode initialization
	moenchCommonModeSubtraction = NULL;
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
	FILE_LOG(logDEBUG) << __AT__ << " called";


	//number of jobs per buffer
	int64_t i;
	int oldNumberofJobsPerBuffer = numberofJobsPerBuffer;
	//eiger always listens to 1 packet at a time
	if((myDetectorType == EIGER) || (myDetectorType == JUNGFRAU)){
		numberofJobsPerBuffer = 1;
		FILE_LOG(logDEBUG) << "Info: 1 packet per buffer";
	}
	//else calculate best possible number of frames to listen to at a time (for fast readouts like gotthard)
	else{
		//if frequency to gui is not random (every nth frame), then listen to only n frames per buffer
		if(frameToGuiFrequency)
			numberofJobsPerBuffer = frameToGuiFrequency;
		//random frame sent to gui, then frames per buffer depends on acquisition period
		else{
			//calculate 100ms/period to get frames to listen to at a time
			if(!acquisitionPeriod)
				i = SAMPLE_TIME_IN_NS;
			else i = SAMPLE_TIME_IN_NS/acquisitionPeriod;
			//max frames to listen to at a time is limited by 1000
			if (i > MAX_JOBS_PER_THREAD)
				numberofJobsPerBuffer = MAX_JOBS_PER_THREAD;
			else if (i < 1)
				numberofJobsPerBuffer = 1;
			else
				numberofJobsPerBuffer = i;

		}
		FILE_LOG(logINFO) << "Number of Frames per buffer:" << numberofJobsPerBuffer << endl;
	}



	// fifo depth
	uint32_t oldFifoSize = fifoSize;

	fifoSize = fifoDepth;

	//reduce fifo depth if > 1 numberofJobsPerBuffer
	if(fifoSize % numberofJobsPerBuffer)
		fifoSize = (fifoSize/numberofJobsPerBuffer)+1;
	else
		fifoSize = fifoSize/numberofJobsPerBuffer;

	//do not rebuild fifo structure if it is the same (oldfifosize differs only for different packetsperframe)
	if((oldNumberofJobsPerBuffer == numberofJobsPerBuffer) && (oldFifoSize == fifoSize))
		return OK;
	FILE_LOG(logINFO) << "Info: Total Fifo Size:" << fifoSize;



	//delete threads
	if(threadStarted){
		createListeningThreads(true);
		createWriterThreads(true);
	}


	//set up fifo structure
	for(int i=0;i<numberofListeningThreads;i++){

		//deleting
		if(fifoFree[i]){
			while(!fifoFree[i]->isEmpty()){
				fifoFree[i]->pop(buffer[i]);
				//cprintf(BLUE,"FifoFree[%d]: value:%d, pop 0x%x\n",i,fifoFree[i]->getSemValue(),(void*)(buffer[i]));
			}
			delete fifoFree[i];
			fifoFree[i] = NULL;
		}
		if(fifo[i]){
			while(!fifo[i]->isEmpty()){
				fifo[i]->pop(buffer[i]);
				//cprintf(CYAN,"Fifo[%d]: value:%d, pop 0x%x\n",i,fifo[i]->getSemValue(),(void*)(buffer[i]));
			}
			delete fifo[i];
			fifo[i] = NULL;
		}
		if(mem0[i]){
			free(mem0[i]);
			mem0[i] = NULL;
		}

		//creating
		fifoFree[i] 	= new CircularFifo<char>(fifoSize);
		fifo[i] 		= new CircularFifo<char>(fifoSize);

		//allocate memory
		mem0[i] = (char*)malloc((bufferSize * numberofJobsPerBuffer + HEADER_SIZE_NUM_TOT_PACKETS) * fifoSize);
		if (mem0[i] == NULL){
			cprintf(BG_RED,"Error: Could not allocate memory for listening \n");
			return FAIL;
		}

		//push free address into fifoFree
		buffer[i]=mem0[i];
		while (buffer[i] < (mem0[i]+(bufferSize * numberofJobsPerBuffer + HEADER_SIZE_NUM_TOT_PACKETS) * (fifoSize-1))) {
			//cprintf(BLUE,"fifofree %d: push 0x%p\n",i,(void*)buffer[i]);
			/*for(int k=0;k<bufferSize;k=k+4){
				sprintf(buffer[i]+HEADER_SIZE_NUM_TOT_PACKETS+k,"mem%d",i);
			}*/
			sprintf(buffer[i],"mem%d",i);
			while(!fifoFree[i]->push(buffer[i]));
			//cprintf(GREEN,"Fifofree[%d]: value:%d, push 0x%x\n",i,fifoFree[i]->getSemValue(),(void*)(buffer[i]));
#ifdef DEBUG5
			cprintf(BLUE,"Info: %d fifostructure free pushed into fifofree %p\n", i, (void*)(buffer[i]));
#endif
			buffer[i] += (bufferSize * numberofJobsPerBuffer + HEADER_SIZE_NUM_TOT_PACKETS);
		}
	}
	cout << "Fifo structure(s) reconstructed" << endl;

	//create threads
	if(createListeningThreads() == FAIL){
		FILE_LOG(logERROR) << "Could not create listening thread";
		return FAIL;
	}
	if(createWriterThreads() == FAIL){
		FILE_LOG(logERROR) << "Could not create writer threads";
		return FAIL;
	}
	setThreadPriorities();

	return OK;
}







void UDPStandardImplementation::configure(map<string, string> config_map){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	map<string, string>::const_iterator pos;
	pos = config_map.find("mode");
	if (pos != config_map.end() ){
		int b;
		if(!sscanf(pos->second.c_str(), "%d", &b)){
			cout << "Warning: Could not parse mode. Assuming top mode." << endl;
			b = 0;
		}
		bottomEnable = b!= 0;
		FILE_LOG(logINFO) << "Bottom: " << stringEnable(bottomEnable);
	}

}


void UDPStandardImplementation::setFileName(const char c[]){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

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

	FILE_LOG(logINFO) << "File name:" << fileName;
}

int UDPStandardImplementation::setDataCompressionEnable(const bool b){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

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

	FILE_LOG(logINFO) << "Data Compression: " << stringEnable(dataCompressionEnable);

	return OK;
}

/***acquisition count parameters***/
uint64_t UDPStandardImplementation::getTotalFramesCaught() const{
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	return (totalPacketsCaught/(packetsPerFrame*numberofListeningThreads));
}

uint64_t UDPStandardImplementation::getFramesCaught() const{
	FILE_LOG(logDEBUG) << __AT__ << " starting";
	return (packetsCaught/(packetsPerFrame*numberofListeningThreads));
}

/***acquisition parameters***/
void UDPStandardImplementation::setShortFrameEnable(const int i){
	FILE_LOG(logDEBUG) << __AT__ << " called";

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

	FILE_LOG(logINFO) << "Short Frame Enable: " << shortFrameEnable;
}


int UDPStandardImplementation::setFrameToGuiFrequency(const uint32_t freq){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	frameToGuiFrequency = freq;
	if(setupFifoStructure() == FAIL)
		return FAIL;

	FILE_LOG(logINFO) << "Frame to Gui Frequency: " << frameToGuiFrequency;

	return OK;
}



uint32_t UDPStandardImplementation::setDataStreamEnable(const uint32_t enable){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	int oldvalue = dataStreamEnable;
	dataStreamEnable = enable;
	FILE_LOG(logINFO) << "Data Send to Gui: " << dataStreamEnable;


	if(oldvalue!=dataStreamEnable){
		//data sockets have to be created again as the client ones are
		if(zmqThreadStarted)
			createDataCallbackThreads(true);

		if(dataStreamEnable){
			numberofDataCallbackThreads = MAX_NUMBER_OF_LISTENING_THREADS;
			if(createDataCallbackThreads() == FAIL){
				cprintf(BG_RED,"Error: Could not create data callback threads\n");
			}
		}
	}

	return OK;
}



int UDPStandardImplementation::setAcquisitionPeriod(const uint64_t i){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	acquisitionPeriod = i;
	if(setupFifoStructure() == FAIL)
		return FAIL;

	FILE_LOG(logINFO) << "Acquisition Period: " << (double)acquisitionPeriod/(1E9) << "s";

	return OK;
}


int UDPStandardImplementation::setNumberOfFrames(const uint64_t i){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	numberOfFrames = i;
	if(setupFifoStructure() == FAIL)
		return FAIL;

	FILE_LOG(logINFO) << "Number of Frames:" << numberOfFrames;

	return OK;
}

int UDPStandardImplementation::setDynamicRange(const uint32_t i){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	uint32_t oldDynamicRange = dynamicRange;

	FILE_LOG(logDEBUG) << "Info: Setting Dynamic Range to " << i;
	dynamicRange = i;

	if(myDetectorType == EIGER){

		//set parameters depending on new dynamic range.
		packetsPerFrame 	= (tengigaEnable ? EIGER_TEN_GIGA_CONSTANT : EIGER_ONE_GIGA_CONSTANT) * dynamicRange;
		bufferSize			= onePacketSize * packetsPerFrame;

		for(int i=0; i<MAX_NUMBER_OF_WRITER_THREADS; i++)
			updateFileHeader(i);

		//new dynamic range, then restart threads and resetup fifo structure
		if(oldDynamicRange != dynamicRange){

			//gui buffer
			for(int i=0;i<numberofWriterThreads;i++){
				if(latestData[i]){delete[] latestData[i]; latestData[i] = NULL;}
				latestData[i] = new char[bufferSize];
			}
			//restructure fifo
			numberofJobsPerBuffer = -1;
			if(setupFifoStructure() == FAIL)
				return FAIL;

		}

	}

	FILE_LOG(logINFO) << "Dynamic Range: " << dynamicRange;

	return OK;
}



int UDPStandardImplementation::setTenGigaEnable(const bool b){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	FILE_LOG(logDEBUG) << "Info: Setting Ten Giga to " << stringEnable(b);
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
		bufferSize			= onePacketSize * packetsPerFrame;
		footerOffset		= EIGER_DATA_PACKET_HEADER_SIZE + oneDataSize;
		FILE_LOG(logDEBUG) << dec <<
				"packetsPerFrame:" << packetsPerFrame <<
				"\nonePacketSize:" << onePacketSize <<
				"\noneDataSize:" << oneDataSize <<
				"\nbufferSize:" << bufferSize;


		for(int i=0; i<MAX_NUMBER_OF_WRITER_THREADS; i++)
			updateFileHeader(i);

		//new enable, then restart threads and resetup fifo structure
		if(oldTenGigaEnable != tengigaEnable){

			//gui buffer
			for(int i=0;i<numberofWriterThreads;i++){
				if(latestData[i]){delete[] latestData[i]; latestData[i] = NULL;}
				latestData[i] = new char[bufferSize];
			}

			//restructure fifo
			if(setupFifoStructure() == FAIL)
				return FAIL;

		}

	}

	FILE_LOG(logINFO) << "Ten Giga: " << stringEnable(tengigaEnable);

	return OK;
}


int UDPStandardImplementation::setFifoDepth(const uint32_t i){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	if(i != fifoDepth){
		FILE_LOG(logINFO) << "Fifo Depth: " << i << endl;
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
	FILE_LOG(logDEBUG) << __AT__ << " called";

	FILE_LOG(logDEBUG) << "Setting receiver type";

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
		FILE_LOG(logINFO) << " ***** " << getDetectorType(d) << " Receiver *****";
		break;
	default:
		FILE_LOG(logERROR) << "This is an unknown receiver type " << (int)d;
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
		//footerOffset		= Not applicable;
		break;
	case EIGER:
		//assuming 1G in the beginning
		packetsPerFrame		= EIGER_ONE_GIGA_CONSTANT * dynamicRange;
		onePacketSize 		= EIGER_ONE_GIGA_ONE_PACKET_SIZE;
		oneDataSize 		= EIGER_ONE_GIGA_ONE_DATA_SIZE;
		bufferSize 			= onePacketSize * packetsPerFrame;
		frameIndexMask 		= EIGER_FRAME_INDEX_MASK;
		frameIndexOffset 	= EIGER_FRAME_INDEX_OFFSET;
		packetIndexMask 	= EIGER_PACKET_INDEX_MASK;
		maxFramesPerFile	= EIGER_MAX_FRAMES_PER_FILE;
		fifoSize			= EIGER_FIFO_SIZE;
		fifoDepth			= EIGER_FIFO_SIZE;
		footerOffset		= EIGER_DATA_PACKET_HEADER_SIZE + oneDataSize;
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
		//footerOffset		= Not applicable;
		break;
	case JUNGFRAU:
		packetsPerFrame		= JFRAU_PACKETS_PER_FRAME;
		onePacketSize 		= JFRAU_ONE_PACKET_SIZE;
		oneDataSize 		= JFRAU_DATA_BYTES;
		bufferSize 			= JFRAU_BUFFER_SIZE;
		frameIndexMask 		= JFRAU_FRAME_INDEX_MASK;
		frameIndexOffset 	= JFRAU_FRAME_INDEX_OFFSET;
		packetIndexMask 	= JFRAU_PACKET_INDEX_MASK;
		maxFramesPerFile	= JFRAU_MAX_FRAMES_PER_FILE;
		fifoDepth			= JFRAU_FIFO_SIZE;
		fifoSize			= JFRAU_FIFO_SIZE;
		//footerOffset		= Not applicable;
		break;
	default:
		FILE_LOG(logERROR) << "This is an unknown receiver type " << (int)d;
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

	numberofDataCallbackThreads = MAX_NUMBER_OF_LISTENING_THREADS;
	if(dataStreamEnable)
		createDataCallbackThreads();

	//allocate for latest data (frame copy for gui), free variables
	for(int i=0; i<MAX_NUMBER_OF_WRITER_THREADS; i++){
		if(latestData[i]) 	{delete[] latestData[i]; 	latestData[i] = NULL;}
		latestData[i] = new char[bufferSize];
	}


	//updates File Header
	if(myDetectorType == EIGER){
		for(int i=0; i<MAX_NUMBER_OF_WRITER_THREADS; i++)
			updateFileHeader(i);
	}

	FILE_LOG(logDEBUG) << " Detector type set to " << getDetectorType(d);

	return OK;
}


/***acquisition functions***/
void UDPStandardImplementation::resetAcquisitionCount(){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	pthread_mutex_lock(&progressMutex);
	startAcquisitionIndex = 0;
	acqStarted = false;
	pthread_mutex_unlock(&progressMutex);
	pthread_mutex_lock(&writeMutex);
	totalPacketsCaught = 0;
	pthread_mutex_unlock(&writeMutex);

	FILE_LOG(logINFO) << "Acquisition Count has been reset";
}


int UDPStandardImplementation::startReceiver(char *c){
	FILE_LOG(logDEBUG) << __AT__ << " called";
	
	FILE_LOG(logINFO)  << "Starting Receiver";

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
	}

	for(int i=0;i<numberofWriterThreads;i++){
		frameIndex[i] = 0;
		//reset file parameters
		lastFrameNumberInFile[i] = -1;
		totalPacketsInFile[i] = 0;
		totalWritingPacketCount[i] = 0;
		if(sfilefd[i]){
			fclose(sfilefd[i]);
			sfilefd[i] = NULL;
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

	//deactivated parameters
	for(int i = 0; i < numberofListeningThreads; ++i)
		deactivatedFrameNumber[i] = 0;
	deactivatedFrameIncrement = (bufferSize/(onePacketSize*packetsPerFrame))*numberofJobsPerBuffer;
	FILE_LOG(logINFO) << "Deactivated Frame Increment:" << deactivatedFrameIncrement;


	//Print Receiver Configuration
	if(myDetectorType != EIGER){
		FILE_LOG(logINFO) << "Data Compression has been " << stringEnable(dataCompressionEnable);
	}
	FILE_LOG(logINFO) << "Number of Jobs Per Buffer: " << numberofJobsPerBuffer;
	FILE_LOG(logINFO) << "Max Frames Per File:" << maxFramesPerFile;
	if(frameToGuiFrequency)
		FILE_LOG(logINFO) << "Frequency of frames sent to gui: " << frameToGuiFrequency;
	else
		FILE_LOG(logINFO) << "Frequency of frames sent to gui: Random";



	//create UDP sockets
	if(createUDPSockets() == FAIL){
		strcpy(c,"Could not create UDP Socket(s).");
		FILE_LOG(logERROR) << c;
		return FAIL;
	}

	if(setupWriter() == FAIL){
		//stop udp socket
		shutDownUDPSockets();
		sprintf(c,"Could not create file");
		//FILE_LOG(logERROR) << c;
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

	FILE_LOG(logINFO)  << "Receiver Started";
	FILE_LOG(logINFO)  << "Status: " << runStatusType(status);

	return OK;
}


/**
 * Pre: status is running, semaphores have been instantiated,
 * Post: udp sockets shut down, status is idle, semaphores destroyed
 * */
void UDPStandardImplementation::stopReceiver(){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	FILE_LOG(logINFO)  << "Stopping Receiver";

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

	FILE_LOG(logINFO)  << "Receiver Stopped";
	FILE_LOG(logINFO)  << "Status: " << runStatusType(status);
	cout << endl << endl;
}





int UDPStandardImplementation::shutDownUDPSockets(){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	for(int i=0;i<numberofListeningThreads;i++){
		if(udpSocket[i]){
			udpSocket[i]->ShutDownSocket();
			FILE_LOG(logINFO) << "Shut down UDP Socket " << i;
			delete udpSocket[i];
			udpSocket[i] = NULL;
		}
	}
	return OK;
}




/**
 * Pre: status is running, udp sockets have been initialized, stop receiver initiated
 * Post:udp sockets closed, status is transmitting
 * */
void UDPStandardImplementation::startReadout(){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	FILE_LOG(logDEBUG) << "Transmitting last data";

	if(status == RUNNING){


		//needs to wait for packets only if activated
		if(activated){
			//check if all packets got
			int totalP = 0,prev=-1,i;
			for(i=0; i<numberofListeningThreads; ++i)
				totalP += totalListeningPacketCount[i];

			//check if current buffer still receiving something
			int currentReceivedInBuffer=0,prevReceivedInBuffer=-1;
			for(i=0; i<numberofListeningThreads; ++i)
				currentReceivedInBuffer += udpSocket[i]->getCurrentTotalReceived();

			//wait for all packets
			if(totalP!=numberOfFrames*packetsPerFrame*numberofListeningThreads){

				//wait as long as there is change from prev totalP,
				//and also change from received in buffer to previous value
				//(as one listens to many at a time, shouldnt cut off in between)
				while((prev != totalP) || (prevReceivedInBuffer!= currentReceivedInBuffer)){
#ifdef DEBUG5
					cprintf(MAGENTA,"waiting for all packets totalP:%d currently in buffer:%d\n",totalP,currentReceivedInBuffer);

#endif

					usleep(5000);/* Need to find optimal time (exposure time and acquisition period) **/
					prev = totalP;
					totalP = 0;
					for(i=0; i<numberofListeningThreads; ++i)
						totalP += totalListeningPacketCount[i];

					prevReceivedInBuffer = currentReceivedInBuffer;
					currentReceivedInBuffer = 0;
					for(i=0; i<numberofListeningThreads; ++i)
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

		FILE_LOG(logINFO) << "Status: Transmitting";
	}

	//shut down udp sockets and make listeners push dummy (end) packets for writers
	shutDownUDPSockets();
}



/**make this better by asking all of it at once*/
void UDPStandardImplementation::readFrame(int ithread, char* c,char** raw, int64_t &startAcq, int64_t &startFrame){
	FILE_LOG(logDEBUG) << __AT__ << " called";

}



void UDPStandardImplementation::closeFile(int ithread){
	FILE_LOG(logDEBUG) << __AT__ << " called for " << ithread ;

	//normal
	if(!dataCompressionEnable){
		if(sfilefd[ithread]){
#ifdef DEBUG4
			FILE_LOG(logDEBUG4) << "Going to close file: " << fileno(sfilefd));
#endif
			fclose(sfilefd[ithread]);
			sfilefd[ithread] = NULL;
		}
	}

	//compression
	else{
#if (defined(MYROOT1) && defined(ALLFILE_DEBUG)) || !defined(MYROOT1)
		if(sfilefd[0]){
#ifdef DEBUG4
			FILE_LOG(logDEBUG4) << "sfilefd: " << (int)sfilefd[i];
#endif
			fclose(sfilefd[0]);
			sfilefd[0] = NULL;
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
		if(myFile[ithread] != NULL)
			myFile[ithread]->Close();
		myFile[ithread] = NULL;
		myTree[ithread] = NULL;
		pthread_mutex_unlock(&writeMutex);

#endif
	}
}




/*************************************************************************
 * Listening and Writing Threads *****************************************
 *************************************************************************/

int UDPStandardImplementation::createDataCallbackThreads(bool destroy){
	FILE_LOG(logDEBUG) << __AT__ << " starting";
	if(!destroy) cprintf(MAGENTA,"Data Callback thread created\n"); else cprintf(MAGENTA,"Data Callback thread destroyed\n");
	//reset masks
	killAllDataCallbackThreads = false;
	pthread_mutex_lock(&statusMutex);
	dataCallbackThreadsMask = 0x0;
	pthread_mutex_unlock(&(statusMutex));

	//destroy
	if(destroy){
	        FILE_LOG(logDEBUG) << "Info: Destroying Data Callback Thread(s)";

	        killAllDataCallbackThreads = true;
		for(int i = 0; i < numberofDataCallbackThreads; ++i){
			sem_post(&dataCallbackSemaphore[i]);
			pthread_join(dataCallbackThreads[i],NULL);
			sem_destroy(&dataCallbackSemaphore[i]);
			FILE_LOG(logDEBUG) << "." << flush;
		}
		killAllDataCallbackThreads = false;
		zmqThreadStarted = false;
		FILE_LOG(logINFO) << "Data Callback thread(s) destroyed";
	}

	//create
	else{
		FILE_LOG(logINFO) << "Creating Data Callback Thread(s)";

		//reset current index
		currentThreadIndex = -1;

		for(int i = 0; i < numberofDataCallbackThreads; ++i){
			sem_init(&dataCallbackSemaphore[i],1,0);
			zmqThreadStarted = false;
			currentThreadIndex = i;
			if(pthread_create(&dataCallbackThreads[i], NULL,startDataCallbackThread, (void*) this)){
				FILE_LOG(logERROR) << "Could not create data call back thread with index " << i;
				return FAIL;
			}
			while(!zmqThreadStarted);
			FILE_LOG(logDEBUG) << "." << flush;
		}
		FILE_LOG(logDEBUG) << "Info: Data Callback thread(s) created successfully.";
	}

	return OK;
}



int UDPStandardImplementation::createListeningThreads(bool destroy){
	FILE_LOG(logDEBUG) << __AT__ << " starting";
	if(!destroy) cprintf(BLUE,"Listening thread created\n"); else cprintf(BLUE,"Listening thread destroyed\n");
	//reset masks
	killAllListeningThreads = false;
	pthread_mutex_lock(&statusMutex);
	listeningThreadsMask = 0x0;
	pthread_mutex_unlock(&(statusMutex));

	//destroy
	if(destroy){
	        FILE_LOG(logDEBUG) << "Info: Destroying Listening Thread(s)";

		killAllListeningThreads = true;
		for(int i = 0; i < numberofListeningThreads; ++i){
			sem_post(&listenSemaphore[i]);
			pthread_join(listeningThreads[i],NULL);
			sem_destroy(&listenSemaphore[i]);
			FILE_LOG(logDEBUG) << "." << flush;
		}
		killAllListeningThreads = false;
		threadStarted = false;
		FILE_LOG(logINFO) << "Listening thread(s) destroyed";
	}

	//create
	else{
		FILE_LOG(logINFO) << "Creating Listening Thread(s)";

		//reset current index
		currentThreadIndex = -1;

		for(int i = 0; i < numberofListeningThreads; ++i){
			sem_init(&listenSemaphore[i],1,0);
			threadStarted = false;
			currentThreadIndex = i;
			if(pthread_create(&listeningThreads[i], NULL,startListeningThread, (void*) this)){
				FILE_LOG(logERROR) << "Could not create listening thread with index " << i;
				return FAIL;
			}
			while(!threadStarted);
			FILE_LOG(logDEBUG) << "." << flush;
		}
		FILE_LOG(logDEBUG) << "Info: Listening thread(s) created successfully.";
	}

	return OK;
}



int UDPStandardImplementation::createWriterThreads(bool destroy){
	FILE_LOG(logDEBUG) << __AT__ << " starting";
	if(!destroy) cprintf(GREEN,"Writer thread created\n"); else cprintf(GREEN,"Writer thread destroyed\n");
	//reset masks
	killAllWritingThreads = false;
	pthread_mutex_lock(&statusMutex);
	writerThreadsMask = 0x0;
	createFileMask = 0x0;
	pthread_mutex_unlock(&(statusMutex));

	//destroy threads
	if(destroy){
		FILE_LOG(logDEBUG) << "Info: Destroying Writer Thread(s)";

		killAllWritingThreads = true;
		for(int i = 0; i < numberofWriterThreads; ++i){
			sem_post(&writerSemaphore[i]);
			pthread_join(writingThreads[i],NULL);
			sem_destroy(&writerSemaphore[i]);
			FILE_LOG(logDEBUG) <<"."<<flush;
		}
		killAllWritingThreads = false;
		threadStarted = false;
		FILE_LOG(logINFO) << "Writer thread(s) destroyed";
	}

	//create threads
	else{
		FILE_LOG(logINFO) << "Creating Writer Thread(s)";

		//reset current index
		currentThreadIndex = -1;

		for(int i = 0; i < numberofWriterThreads; ++i){
			sem_init(&writerSemaphore[i],1,0);
			threadStarted = false;
			currentThreadIndex = i;
			if(pthread_create(&writingThreads[i], NULL,startWritingThread, (void*) this)){
				FILE_LOG(logERROR) << "Could not create writer thread with index " << i;
				return FAIL;
			}
			while(!threadStarted);
			FILE_LOG(logDEBUG) << "." << flush;
		}
#ifdef DEBUG
		cout << "\nWriter thread(s) created successfully" << endl;
#endif
	}

	return OK;
}



void UDPStandardImplementation::setThreadPriorities(){
	FILE_LOG(logDEBUG) << __AT__ << " called";

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
	for(int i = 0; i < numberofDataCallbackThreads; ++i){
		if(rights)
			if (pthread_setschedparam(dataCallbackThreads[i], SCHED_RR, &datacallback_param) == EPERM){
				rights = false;
				break;
			}
	}
	if (pthread_setschedparam(pthread_self(),5 , &tcp_param) == EPERM)
		rights = false;

	if(!rights){
		FILE_LOG(logWARNING) << "Unable to prioritize threads. Root privileges required for this option.";
	}else{
		FILE_LOG(logINFO) << "Priorities set - DataCallback: 55, TCP:50, Listening:99, Writing:90";
	}
}




int UDPStandardImplementation::createUDPSockets(){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	//switching ports if bottom enabled
	uint32_t port[2];
	port[0]= udpPortNum[0];
	port[1]= udpPortNum[1];
	//port =  udpPortNum;
	if(bottomEnable){
		port[0] = udpPortNum[1];
		port[1] = udpPortNum[0];
	}

	//if eth is mistaken with ip address
	if (strchr(eth,'.') != NULL)
		strcpy(eth,"");

	shutDownUDPSockets();
	int headerpacketsize = 0;
	if(myDetectorType == EIGER)
		headerpacketsize = EIGER_HEADER_PACKET_LENGTH;

	//if no eth, listen to all
	if(!strlen(eth)){
		FILE_LOG(logWARNING) << "eth is empty. Listening to all";

		for(int i=0;i<numberofListeningThreads;i++)
			udpSocket[i] = new genericSocket(port[i],genericSocket::UDP,onePacketSize,NULL,headerpacketsize);
	}
	//normal socket
	else{
		FILE_LOG(logINFO) << "Ethernet Interface:" << eth;

		for(int i=0;i<numberofListeningThreads;i++)
			udpSocket[i] = new genericSocket(port[i],genericSocket::UDP,onePacketSize,eth,headerpacketsize);
	}

	//error
	for(int i=0;i<numberofListeningThreads;i++){
		int iret = udpSocket[i]->getErrorStatus();
		if(!iret){
			cout << "UDP port opened at port " << port[i] << endl;
		}else{
			FILE_LOG(logERROR) << "Could not create UDP socket on port " << port[i] << " error: " << iret;
			shutDownUDPSockets();
			return FAIL;
		}
	}

	FILE_LOG(logDEBUG) << "UDP socket(s) created successfully.";
	cout << "Listener Ready ..." << endl;

	return OK;
}



int UDPStandardImplementation::setupWriter(){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	//acquisition start call back returns enable write
	cbAction = DO_EVERYTHING;
	if (startAcquisitionCallBack)
		cbAction=startAcquisitionCallBack(filePath,fileNamePerThread[0],(int)fileIndex,bufferSize,pStartAcquisition);


	if(cbAction < DO_EVERYTHING){
		FILE_LOG(logINFO) << "Call back activated. Data saving must be taken care of by user in call back.";
		if (rawDataReadyCallBack){
			FILE_LOG(logINFO) << "Data Write has been defined externally";
		}
	}else if(!fileWriteEnable){
		FILE_LOG(logINFO) << "Data will not be saved";
	}


	//creating first file
	//setting all value to 1
	pthread_mutex_lock(&statusMutex);
	for(int i=0; i<numberofWriterThreads; i++)
		createFileMask|=(1<<i);
	pthread_mutex_unlock(&statusMutex);

	for(int i=0; i<numberofWriterThreads; i++){
		FILE_LOG(logDEBUG4) <<	i << " Going to post 1st semaphore";
		sem_post(&writerSemaphore[i]);
	}
	//wait till its mask becomes zero(all created)
	while(createFileMask){
		FILE_LOG(logDEBUG4) << "*" << flush;
		usleep(5000);
	}

	if(fileCreateSuccess == OK){
		FILE_LOG(logDEBUG) << "Successfully created file(s)";
		cout << "Writer Ready ..." << endl;
	}

	return fileCreateSuccess;
}



int UDPStandardImplementation::createNewFile(int ithread){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	//create file name
	if(!frameIndexEnable)
		sprintf(completeFileName[ithread], "%s/%s_%lld.raw", filePath,fileNamePerThread[ithread],(long long int)fileIndex);
	else
		sprintf(completeFileName[ithread], "%s/%s_f%012lld_%lld.raw", filePath,fileNamePerThread[ithread],(long long int)lastFrameNumberInFile[ithread]+1,(long long int)fileIndex);

#ifdef DEBUG4
	FILE_LOG(logINFO) << completefileName;
#endif

	//filewrite enable & we allowed to create/close files
	if(fileWriteEnable && cbAction > DO_NOTHING){

		//close file pointers
		if(sfilefd[ithread]){
			fclose(sfilefd[ithread]);
			sfilefd[ithread] = NULL;
		}

		//create file
		if(!overwriteEnable){
			if (NULL == (sfilefd[ithread] = fopen((const char *) (completeFileName[ithread]), "wx"))){
				FILE_LOG(logERROR) << "Could not create/overwrite file" << completeFileName[ithread];
				return FAIL;
			}
		}else if (NULL == (sfilefd[ithread] = fopen((const char *) (completeFileName[ithread]), "w"))){
			FILE_LOG(logERROR) << "Could not create file" << completeFileName[ithread];
			return FAIL;
		}
		//setting file buffer size to 16mb
		setvbuf(sfilefd[ithread],NULL,_IOFBF,BUF_SIZE);


		//Print packet loss and filenames
		if(!totalWritingPacketCount[ithread]){
			frameNumberInPreviousFile[ithread] = -1;
			printf("Thread:%d File:%s\n",ithread,completeFileName[ithread]);
		}else{
			if(frameNumberInPreviousFile[ithread] == -1)
				frameNumberInPreviousFile[ithread] = startFrameIndex -1;

			printf("\nThread:%d File:%s\n"
					"\ttotalpacketsinfile:%d\tPackets Lost:%d\tCurrentFrameNumber:%lld\tPreviousFrameNumber:%lld\n",
					ithread,completeFileName[ithread],
					totalPacketsInFile[ithread],
					( ((int)(currentFrameNumber[ithread]-frameNumberInPreviousFile[ithread])*packetsPerFrame) - totalPacketsInFile[ithread]),
					currentFrameNumber[ithread],frameNumberInPreviousFile[ithread]);
		}

		//write file header
		if(myDetectorType == EIGER)
			fwrite((void*)fileHeader[ithread], 1, FILE_HEADER_SIZE, sfilefd[ithread]);
	}

	//reset counters for each new file
	if(totalWritingPacketCount[ithread]){
		frameNumberInPreviousFile[ithread] = currentFrameNumber[ithread];
		totalPacketsInFile[ithread] = 0;
	}



	return OK;
}




int UDPStandardImplementation::createCompressionFile(int ithread, int iframe){
	FILE_LOG(logDEBUG) << __AT__ << " called";

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
			FILE_LOG(logERROR) << "File Null";
			return FAIL;
		}
		if(!myFile[ithread]->IsOpen()){
			FILE_LOG(logERROR) << "File Not Open";
			return FAIL;
		}
		return OK;
#endif
	return FAIL;
}



void* UDPStandardImplementation::startDataCallbackThread(void* this_pointer){
	FILE_LOG(logDEBUG) << __AT__ << " called";
	((UDPStandardImplementation*)this_pointer)->startDataCallback();
	return this_pointer;
}



void* UDPStandardImplementation::startListeningThread(void* this_pointer){
	FILE_LOG(logDEBUG) << __AT__ << " called";
	((UDPStandardImplementation*)this_pointer)->startListening();
	return this_pointer;
}



void* UDPStandardImplementation::startWritingThread(void* this_pointer){
	FILE_LOG(logDEBUG) << __AT__ << " called";
	((UDPStandardImplementation*)this_pointer)->startWriting();
	return this_pointer;
}




void UDPStandardImplementation::startDataCallback(){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	//set current thread value  index
	int ithread = currentThreadIndex;
	struct timespec begin,end;

	// server address to bind
	char hostName[100] = "tcp://127.0.0.1:";
	int portno = DEFAULT_ZMQ_PORTNO + (detID*2+ithread);
	sprintf(hostName,"%s%d",hostName,portno);

	//socket details
	void *context = zmq_ctx_new();
	void *zmqsocket = zmq_socket(context, ZMQ_PUSH);		// create a publisher
	int val = -1;
	zmq_setsockopt(zmqsocket, ZMQ_LINGER, &val,sizeof(val)); // wait for the unsent packets  before closing socket
	//val = 10;
	//zmq_setsockopt(zmqsocket,ZMQ_SNDHWM,&val,sizeof(val)); //set SEND HIGH WATER MARK (8-9ms slower)
	zmq_bind(zmqsocket,hostName);		// bind
	FILE_LOG(logINFO) << "Thread" << ithread << ": ZMQ Server at " << hostName;


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
		char* buffer = new char[packetsPerFrame*oneDataSize];
		memset(buffer,0xFF,oneframesize);
		int bufferoffset = 0;
		int size = 0;
		int offset = 0;
		int currentfnum = 0;
		uint64_t fnum = 0;
		uint32_t pnum = 0;
		uint32_t snum = 0;
		bool randomSendNow = true;
		bool newFrame = false;



		//header details
		const char *type = "float64";
		const char *shape= "[1024, 512]";
		const char *jsonFmt ="{\"htype\":[\"chunk-1.0\"], \"type\":\"%s\", \"shape\":%s, \"acqIndex\":%d, \"fIndex\":%d, \"subfnum\":%d, \"fname\":\"%s\"}";
		char buf[1000];
		int acquisitionIndex = -1;
		int frameIndex = -1;
		int subframeIndex = -1;
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

				//sending previous half frames if any
				if(newFrame){
					//send header
					//update frame details
					frameIndex = fnum;if(frameIndex==-1) cprintf(RED,"frameindex = -1, 111\n");
					acquisitionIndex = fnum - startAcquisitionIndex;
					if(dynamicRange == 32) subframeIndex = snum;
					int len = sprintf(buf,jsonFmt,type,shape, acquisitionIndex, frameIndex, subframeIndex,completeFileName[ithread]);
					zmq_send(zmqsocket, buf,len, ZMQ_SNDMORE);
					//send data
					zmq_send(zmqsocket, buffer, oneframesize, 0);
					cout<<"sent last dummy"<<endl;
					newFrame = false;
				}



				//send final header
				//update frame details
#ifdef DEBUG
				cout << "sending dummy" << endl;
#endif
				frameIndex = -9;
				acquisitionIndex = -9;
				subframeIndex = -9;
				int len = sprintf(buf,jsonFmt,type,shape, acquisitionIndex, frameIndex, subframeIndex,completeFileName[ithread]);
				zmq_send(zmqsocket, buf,len, ZMQ_SNDMORE);
				//send final data
				zmq_send (zmqsocket, "end", 3, 0);


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
					if((( end.tv_sec - begin.tv_sec )	+ ( end.tv_nsec - begin.tv_nsec ) / 1000000000.0) < 0.5)/**fixed 250 ms*/
						continue;
					//done with timer, look into data
					randomSendNow = true;
				}
			}

			size = guiNumPackets[ithread]*onePacketSize;
			datapacketscaught+=guiNumPackets[ithread];
			offset=0;

			//copy packet by packet -getting rid of headers, -in the right order(padding missing packets)
			while(offset < size){

				//until getting frame number is not error
				while((size>0) && (getFrameandPacketNumber(ithread, latestData[ithread]+offset, fnum, pnum,snum)==FAIL)){
					offset+= onePacketSize;
				}
				//if(!ithread) cout<< ithread <<" fnum:"<< fnum<<" pnum:"<<pnum<<endl;

#ifdef DEBUG
				if(pnum != (oldpnum+1)){
					cprintf(RED,"%d - packets missing: %d (old pnum: %d, new pnum: %d)\n",ithread, pnum-oldpnum-1,oldpnum,pnum);
				}
				oldpnum=pnum;
#endif
				//end of buffer
				if(offset >= size)
					break;

				if(!frameToGuiFrequency)
					currentfnum = fnum;


				//last packet of same frame
				if(fnum == currentfnum && pnum == (packetsPerFrame-1)){
#ifdef DEBUG
					oldpnum=0;
#endif
					memcpy(buffer+(pnum*oneDataSize), latestData[ithread]+offset+headersize,oneDataSize);
					offset+= onePacketSize;
					//send header
					//update frame details
					frameIndex = fnum;
					acquisitionIndex = fnum - startAcquisitionIndex;
					if(dynamicRange == 32) subframeIndex = snum;
					int len = sprintf(buf,jsonFmt,type,shape, acquisitionIndex, frameIndex, subframeIndex,completeFileName[ithread]);
					zmq_send(zmqsocket, buf,len, ZMQ_SNDMORE);
					//send data
					zmq_send(zmqsocket, buffer, oneframesize, 0);
					newFrame = false;
#ifdef DEBUG
					if(!ithread)cprintf(BLUE,"%d sent (last packet)\n",ithread);
#endif
					currentfnum++;
					//start clock after sending
					if(!frameToGuiFrequency){
						randomSendNow = false;
						clock_gettime(CLOCK_REALTIME, &begin);
					}
					memset(buffer,0xFF,oneframesize);

				}
				//same frame (not last) or next frame
				else {
					//next frame
#ifdef DEBUG
					int once = true;
#endif
					while(fnum > currentfnum){
#ifdef DEBUG
						if(once){
							if((fnum-currentfnum-1)>1) cprintf(RED,"%d Complete sub image missing:%d (cfnum:%d nfnum:%d)\n",
									ithread,fnum-currentfnum-1,currentfnum,fnum);
							once = false;
						}
#endif
						//send header
						//update frame details
						frameIndex = fnum;
						acquisitionIndex = fnum - startAcquisitionIndex;
						if(dynamicRange == 32) subframeIndex = snum;
						int len = sprintf(buf,jsonFmt,type,shape, acquisitionIndex, frameIndex, subframeIndex,completeFileName[ithread]);
						zmq_send(zmqsocket, buf,len, ZMQ_SNDMORE);
						//send data
						zmq_send(zmqsocket, buffer, oneframesize, 0);
						newFrame = false;
#ifdef DEBUG
						cprintf(BLUE,"%d sent (last packet of previous frame)\n",ithread);
#endif
						currentfnum++;
						//start clock after sending
						if(!frameToGuiFrequency){
							randomSendNow = false;
							clock_gettime(CLOCK_REALTIME, &begin);
						}
						memset(buffer,0xFF,oneframesize);
					}

					memcpy(buffer+(pnum*oneDataSize), latestData[ithread]+offset+headersize,oneDataSize);
					offset+= onePacketSize;
					newFrame = true;
				}

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
	FILE_LOG(logDEBUG) << __AT__ << " called";

	//set current thread value  index
	int ithread = currentThreadIndex;
	//let calling function know thread started and obtained current
	threadStarted = 1;


	uint32_t rc;				//size of buffer received in bytes
	//split frames for data compression
	int carryonBufferSize; 		//from previous buffer to keep frames together in a buffer
	char* tempBuffer = NULL;	//temporary buffer to store split frames


	/* outer loop - loops once for each acquisition */
	//infinite loop, exited only to change dynamic range, 10G parameters etc (then recreated again)
	while(true){

		//compression variables reset before acquisition
		carryonBufferSize = 0;
		if(dataCompressionEnable){
			if(tempBuffer!=NULL){delete []tempBuffer;tempBuffer=NULL;}
			tempBuffer = new char[onePacketSize * (packetsPerFrame - 1)]; 	//store maximum of 1 packets less in a frame
		}

		/* inner loop - loop for each buffer */
		//until mask reset (udp sockets shut down by client)
		while((1 << ithread) & listeningThreadsMask){


			//pop from fifo
			fifoFree[ithread]->pop(buffer[ithread]);

#ifdef EVERYFIFODEBUG
			if(fifoFree[ithread]->getSemValue()<100)
				cprintf(BLUE,"FifoFree[%d]: value:%d, pop 0x%x\n",ithread,fifoFree[ithread]->getSemValue(),(void*)(buffer[ithread]));
#endif
#ifdef CFIFODEBUG
			if(ithread == 0)
				cprintf(CYAN,"Listening_Thread %d :Listener popped from fifofree %p\n", ithread, (void*)(buffer[ithread]));
			else
				cprintf(YELLOW,"Listening_Thread %d :Listener popped from fifofree %p\n", ithread, (void*)(buffer[ithread]));
#endif


			//udpsocket doesnt exist
			if(activated && udpSocket[ithread] == NULL){
				FILE_LOG(logERROR) << "Listening_Thread " << ithread << ": UDP Socket not created or shut down earlier";
				stopListening(ithread,0);
				continue;
			}

			rc = prepareAndListenBuffer(ithread, carryonBufferSize, tempBuffer);
			carryonBufferSize = 0;

			//start indices for each start of scan/acquisition
			if((!measurementStarted) && (rc > 0))
				startFrameIndices(ithread);
			//problem in receiving or end of acquisition
			if (status == TRANSMITTING||(rc == 0 && activated == 0)){
				stopListening(ithread,rc);
				continue;
			}

			//write packet count to buffer
			if(myDetectorType == EIGER)
				(*((uint32_t*)(buffer[ithread]))) = (rc/onePacketSize);

			if(dataCompressionEnable)
				(*((uint32_t*)(buffer[ithread]))) = processListeningBuffer(ithread, carryonBufferSize, tempBuffer, rc);


			//push buffer to FIFO
			while(!fifo[ithread]->push(buffer[ithread]));

#ifdef EVERYFIFODEBUG
			if(fifo[ithread]->getSemValue()>(fifoSize-100))
			cprintf(MAGENTA,"Fifo[%d]: value:%d, push 0x%x\n",ithread,fifo[ithread]->getSemValue(),(void*)(buffer[ithread]));
#endif
#ifdef CFIFODEBUG
				if(ithread == 0)
					cprintf(CYAN,"Listening_Thread %d: Listener pushed into fifo %p\n",ithread, (void*)(buffer[ithread]));
				else
					cprintf(YELLOW,"Listening_Thread %d: Listener pushed into fifo %p\n",ithread, (void*)(buffer[ithread]));

#endif


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
	FILE_LOG(logDEBUG) << __AT__ << " called";

	int receivedSize = 0;

	//carry over from previous buffer
	if(cSize)	memcpy(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS, temp, cSize);


	if(!activated){

		//cSize = 0 for deactivated
		int framestoclone = 0;
		//first
		if(deactivatedFrameNumber[ithread]==0)
			deactivatedFrameNumber[ithread]++;
		//done
		if(deactivatedFrameNumber[ithread] == (numberOfFrames+1))
			return 0;
		//last
		if((deactivatedFrameNumber[ithread] + deactivatedFrameIncrement) > (numberOfFrames+1))
			framestoclone = (numberOfFrames+1) - deactivatedFrameNumber[ithread];
		//in progress
		else
			framestoclone = deactivatedFrameIncrement;

		//copy dummy packets
		receivedSize = framestoclone*packetsPerFrame*onePacketSize;
		memset(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS, 0xFF,receivedSize);

		//set fnum, pnum and deactivatedpacket label
		eiger_packet_header_t* header;
		eiger_packet_footer_t* footer;
		int pnum=0;
		//loop by each packet
		for(int offset=HEADER_SIZE_NUM_TOT_PACKETS;
				offset<receivedSize;
				offset+=onePacketSize){
			header = (eiger_packet_header_t*)(buffer[ithread] + offset);
			footer = (eiger_packet_footer_t*)(buffer[ithread] + offset + footerOffset);
			*( (uint64_t*) footer) = deactivatedFrameNumber[ithread];
			*( (uint16_t*) footer->packetNumber) = ++pnum;
#ifdef MANUALDEBUG
			if(!ithread){
				cprintf(GREEN,"thread:%d pnum:%d fnum:%d\n",
						ithread,
						(*( (uint16_t*) footer->packetNumber)),
						(uint32_t)(*( (uint64_t*) footer)));
			}
#endif
			if(pnum == packetsPerFrame){
				pnum = 0;
				deactivatedFrameNumber[ithread]++;
			}
		}

		return receivedSize;
	}


	if(status != TRANSMITTING)
		receivedSize = udpSocket[ithread]->ReceiveDataOnly(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS + cSize, (bufferSize * numberofJobsPerBuffer) - cSize);
	//eiger returns 0 when header packet caught
	while(receivedSize < onePacketSize && status != TRANSMITTING)
		receivedSize = udpSocket[ithread]->ReceiveDataOnly(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS + cSize, (bufferSize * numberofJobsPerBuffer) - cSize);

	totalListeningPacketCount[ithread] += (receivedSize/onePacketSize);


#ifdef MANUALDEBUG
	if(receivedSize>0){
		if(myDetectorType == JUNGFRAU){
			jfrau_packet_header_t* header;

			for(int iloop=0;iloop<2;iloop++){
				header = (jfrau_packet_header_t*)(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS + iloop * (JFRAU_HEADER_LENGTH+JFRAU_ONE_DATA_SIZE));
				cprintf(RED,"[%d]: packetnumber:%x\n",iloop, (*( (uint8_t*) header->packetNumber)));
				cprintf(RED,"    : framenumber :%x\n",       (*( (uint32_t*) header->frameNumber))&0xffffff);
			}
		}else if(myDetectorType == EIGER){
			eiger_packet_header_t* header = (eiger_packet_header_t*) (buffer[ithread]+HEADER_SIZE_NUM_TOT_PACKETS);
			eiger_packet_footer_t* footer = (eiger_packet_footer_t*)(buffer[ithread] + footerOffset + HEADER_SIZE_NUM_TOT_PACKETS);
			cprintf(GREEN,"thread:%d footeroffset:%dsubframenum:%d oldpacketnum:%d new pnum:%d new fnum:%d\n",
					ithread,footerOffset,
					(*( (unsigned int*) header->subFrameNumber)),
					(*( (uint8_t*) header->dynamicRange)),
					(*( (uint16_t*) footer->packetNumber)),
					(uint32_t)(*( (uint64_t*) footer)));
		}
	}
#endif
#ifdef DEBUG
	cprintf(BLUE, "Listening_Thread %d : Received bytes: %d. Expected bytes: %d\n", ithread, receivedSize, bufferSize * numberofJobsPerBuffer-cSize);
#endif
	return receivedSize;
}






void UDPStandardImplementation::startFrameIndices(int ithread){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	//determine startFrameIndex
	jfrau_packet_header_t* header=0;
	switch(myDetectorType){
	case EIGER:
		startFrameIndex = 0;	//frame number always resets
		break;
	case JUNGFRAU:
		header = (jfrau_packet_header_t*)(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS);
		startFrameIndex = (*( (uint32_t*) header->frameNumber))&0xffffff;
		break;
	default:
		if(shortFrameEnable < 0){
			startFrameIndex = (((((uint32_t)(*((uint32_t*)(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS))))+1)
					& (frameIndexMask)) >> frameIndexOffset);
		}else{
			startFrameIndex = ((((uint32_t)(*((uint32_t*)(buffer[ithread]+HEADER_SIZE_NUM_TOT_PACKETS))))
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
	FILE_LOG(logDEBUG) << __AT__ << " called";

#ifdef DEBUG4
	cprintf(BLUE,"Listening_Thread %d: Stop Listening\nStatus: %s numbytes:%d\n", ithread, runStatusType(status).c_str(),numbytes);
#endif

	//less than 1 packet size (especially for eiger), ignore the buffer (so that 2 dummy buffers are not sent with pc=0)
	if(numbytes < onePacketSize)
		numbytes = 0;


	//free empty buffer
	if(numbytes <= 0){
		FILE_LOG(logINFO) << "Listening "<< ithread << ": End of Acquisition";
		while(!fifoFree[ithread]->push(buffer[ithread]));
#ifdef EVERYFIFODEBUG
		if(fifoFree[ithread]->getSemValue()<100)
		cprintf(GREEN,"Fifofree[%d]: value:%d, push 0x%x\n",ithread,fifoFree[ithread]->getSemValue(),(void*)(buffer[ithread]));
#endif
#ifdef CFIFODEBUG
		if(ithread == 0)
			cprintf(CYAN,"Listening_Thread %d :Listener push empty buffer into fifofree %p\n", ithread, (void*)(buffer[ithread]));
		else
			cprintf(YELLOW,"Listening_Thread %d :Listener push empty buffer into fifofree %p\n", ithread, (void*)(buffer[ithread]));
#endif
	}


	//push last non empty buffer into fifo
	else{
		(*((uint32_t*)(buffer[ithread]))) = numbytes/onePacketSize;
		totalListeningPacketCount[ithread] += (numbytes/onePacketSize);
#ifdef DEBUG
		cprintf(BLUE,"Listening_Thread %d: Last Buffer numBytes:%d\n",ithread, numbytes);
		cprintf(BLUE,"Listening_Thread %d: Last Buffer packet count:%d\n",ithread, numbytes/onePacketSize);
#endif
		while(!fifo[ithread]->push(buffer[ithread]));
#ifdef EVERYFIFODEBUG
		if(fifo[ithread]->getSemValue()>(fifoSize-100))
		cprintf(MAGENTA,"Fifo[%d]: value:%d, push 0x%x\n",ithread,fifo[ithread]->getSemValue(),(void*)(buffer[ithread]));
#endif
#ifdef CFIFODEBUG
		if(ithread == 0)
		 cprintf(CYAN,"Listening_Thread %d: Listener Last Buffer pushed into fifo %p\n",  ithread,(void*)(buffer[ithread]));
		else
			cprintf(YELLOW,"Listening_Thread %d: Listener Last Buffer pushed into fifo %p\n",  ithread,(void*)(buffer[ithread]));
#endif
	}

	//push dummy-end buffer into fifo for all writer threads
	fifoFree[ithread]->pop(buffer[ithread]);
#ifdef EVERYFIFODEBUG
	if(fifoFree[ithread]->getSemValue()<100)
		cprintf(BLUE,"FifoFree[%d]: value:%d, pop 0x%x\n",ithread,fifoFree[ithread]->getSemValue(),(void*)(buffer[ithread]));
#endif
#ifdef CFIFODEBUG
	if(ithread == 0)
		cprintf(CYAN,"Listening_Thread %d: Popped Dummy from fifoFree %p\n",  ithread,(void*)(buffer[ithread]));
	else
		cprintf(YELLOW,"Listening_Thread %d: Popped Dummy from fifoFree %p\n",  ithread,(void*)(buffer[ithread]));
#endif
	//creating dummy-end buffer with pc=0xFFFF
	(*((uint32_t*)(buffer[ithread]))) = dummyPacketValue;
	while(!fifo[ithread]->push(buffer[ithread]));
#ifdef EVERYFIFODEBUG
	if(fifo[ithread]->getSemValue()>(fifoSize-100))
		cprintf(MAGENTA,"Fifo[%d]: value:%d, push 0x%x\n",ithread,fifo[ithread]->getSemValue(),(void*)(buffer[ithread]));
#endif
#ifdef CFIFODEBUG
	if(ithread == 0)
		cprintf(CYAN,"Listening_Thread %d: Listener pushed dummy-end buffer into fifo %p\n", ithread,(void*)(buffer[ithread]));
	else
		cprintf(YELLOW,"Listening_Thread %d: Listener pushed dummy-end buffer into fifo %p\n", ithread,(void*)(buffer[ithread]));
#endif



	//reset mask and exit loop
	pthread_mutex_lock(&statusMutex);
	listeningThreadsMask^=(1<<ithread);
//#ifdef DEBUG4
	//cprintf(BLUE,"Listening_Thread %d: Resetting mask of listening thread. New Mask: 0x%x", ithread, listeningThreadsMask);
	FILE_LOG(logINFO) << "Listening Thread of " << udpPortNum[ithread] << "got " << totalListeningPacketCount[ithread] << " packets";
	//cprintf(BLUE,"Listening_Thread %d: Frames listened to :%d\n",ithread, (totalListeningPacketCount[ithread]/packetsPerFrame));
//#endif
	pthread_mutex_unlock(&(statusMutex));


	//first thread, waiting for all other listening threads to be done, to print statistics
	if(ithread == 0){
#ifdef DEBUG4
		if(numberofListeningThreads > 1)
			cprintf(BLUE,"Listening_Thread %d: Waiting for other listening threads to be done.. current mask:0x%x\n", ithread, listeningThreadsMask);
#endif
		while(listeningThreadsMask)
			usleep(5000);
#ifdef DEBUG4
		int t=0;
		for(i=0;i<numberofListeningThreads;++i)
			t += totalListeningPacketCount[i];
		cprintf(BLUE,"Listening_Thread %d :Total Frames listened to %d\n", ithread,(t/(packetsPerFrame*numberofListeningThreads)));
#endif
	}
}





uint32_t UDPStandardImplementation::processListeningBuffer(int ithread, int &cSize, char* temp, int rc){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	int lastPacketOffset;		//the offset of the last packet
	uint32_t lastFrameHeader;		//frame number of last packet in buffer
	uint64_t lastFrameHeader64;		//frame number of last packet in buffer
	uint32_t packetCount = (rc/onePacketSize);//(packetsPerFrame/numberofListeningThreads) * numberofJobsPerBuffer;		//packets received
	cSize = 0;					//reset size
	jfrau_packet_header_t* header;

	switch(myDetectorType){
	case GOTTHARD:
	case PROPIX:
		//for short frames, 1 packet/frame, so split frames is not a topic
		if(shortFrameEnable == -1){
			lastPacketOffset = (((packetCount - 1) * onePacketSize) + HEADER_SIZE_NUM_TOT_PACKETS);
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
		cprintf(BLUE, "Listening_Thread %d: First Header:%d\n", (((((uint32_t)(*((uint32_t*)(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS))))+1)
				& (frameIndexMask)) >> frameIndexOffset));
#endif
		break;

	case MOENCH:
		lastPacketOffset = (((packetCount - 1) * onePacketSize) + HEADER_SIZE_NUM_TOT_PACKETS);
#ifdef DEBUG4
		cprintf(BLUE, "Listening_Thread %d: First Header:%d\t First Packet:%d\t Last Header:%d\t Last Packet:%d\tLast Packet Offset:%d\n",
				(((((uint32_t)(*((uint32_t*)(buffer[ithread]+HEADER_SIZE_NUM_TOT_PACKETS))))) & (frameIndexMask)) >> frameIndexOffset),
				((((uint32_t)(*((uint32_t*)(buffer[ithread]+HEADER_SIZE_NUM_TOT_PACKETS))))) & (packetIndexMask)),
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


	case JUNGFRAU:
		lastPacketOffset = (((packetCount - 1) * onePacketSize) + HEADER_SIZE_NUM_TOT_PACKETS);
#ifdef DEBUG4
		header = (jfrau_packet_header_t*) (buffer[ithread]+HEADER_SIZE_NUM_TOT_PACKETS);
		cprintf(BLUE, "Listening_Thread: First Header:%d\t First Packet:%d\n",
				(*( (uint32_t*) header->frameNumber))&0xffffff,
				(*( (uint8_t*) header->packetNumber)));
#endif
		header = (jfrau_packet_header_t*) (buffer[ithread]+lastPacketOffset);
#ifdef DEBUG4
		cprintf(BLUE, "Listening_Thread: Last Header:%du\t Last Packet:%d\n",
				(*( (uint32_t*) header->frameNumber))&0xffffff,
				(*( (uint8_t*) header->packetNumber)));
#endif
		//jungfrau last packet value is 0, so find the last packet and store the others in a temp storage
		if((*( (uint8_t*) header->packetNumber))){
			//cprintf(RED,"entering missing packet zone\n");
			lastFrameHeader64 = (*( (uint32_t*) header->frameNumber))&0xffffff;
			cSize += onePacketSize;
			lastPacketOffset -= onePacketSize;
			--packetCount;
			while (lastFrameHeader64 == ((*( (uint32_t*) header->frameNumber))&0xffffff)){
				cSize += onePacketSize;
				lastPacketOffset -= onePacketSize;
				header = (jfrau_packet_header_t*) (buffer[ithread]+lastPacketOffset);
#ifdef DEBUG4
				cprintf(RED,"new header:%d new packet:%d\n",
						(*( (uint32_t*) header->frameNumber))&0xffffff,
						(*( (uint8_t*) header->packetNumber)));
#endif
				--packetCount;
			}
			memcpy(temp, buffer[ithread]+(lastPacketOffset+onePacketSize), cSize);
		}

		break;

	default:
		cprintf(RED,"Listening_Thread %d: Error: This detector %s is not implemented in the receiver\n",
				ithread, getDetectorType(myDetectorType).c_str());
		break;
	}

#ifdef DEBUG4
	cprintf(BLUE,"Listening_Thread %d: PacketCount:%d CarryonBufferSize:%d\n",ithread, packetCount, cSize);
#endif

	return packetCount;
}






void UDPStandardImplementation::startWriting(){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	//set current thread value  index
	int ithread = currentThreadIndex;
	//let calling function know thread started and obtained current
	threadStarted = 1;

	//variable definitions
	char* wbuf;											//buffer popped from FIFO
	sfilefd[ithread] = NULL;							//file pointer
	uint64_t nf;										//for compression, number of frames
	int listenfifoIndex = ithread;


	/* outer loop - loops once for each acquisition */
	//infinite loop, exited only to change dynamic range, 10G parameters etc (then recreated again)
	while(true){

		//--reset parameters before acquisition
		nf = 0;
		if(dataCompressionEnable)
			listenfifoIndex = 0;						//compression has only one listening thread


		/* inner loop - loop for each buffer */
		//until mask unset (udp sockets shut down by client)
		while((1 << ithread) & writerThreadsMask){
			//pop
			fifo[listenfifoIndex]->pop(wbuf);
#ifdef EVERYFIFODEBUG
			if(fifo[listenfifoIndex]->getSemValue()>(fifoSize-100))
			cprintf(CYAN,"Fifo[%d]: value:%d, pop 0x%x\n",listenfifoIndex,fifo[listenfifoIndex]->getSemValue(),(void*)(wbuf));
#endif
#ifdef DEBUG4
			cprintf(GREEN,"Writing_Thread %d: Popped %p from FIFO %d\n", ithread, (void*)(wbuf),listenfifoIndex);
#endif
			uint32_t numPackets = (uint32_t)(*((uint32_t*)wbuf));
#ifdef DEBUG4
			cprintf(GREEN,"Writing_Thread %d: Number of Packets: %d for FIFO %d\n", ithread, numPackets, listenfifoIndex);
#endif


			//end of acquisition
			if(numPackets == dummyPacketValue){
#ifdef DEBUG4
				cprintf(GREEN,"Writing_Thread %d: Dummy frame popped out of FIFO %d",ithread, listenfifoIndex);
#endif
				stopWriting(ithread,wbuf);
				continue;
			}



			//normal
			if(!dataCompressionEnable)
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
	FILE_LOG(logDEBUG) << __AT__ << " called";

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
#ifdef EVERYFIFODEBUG
		if(fifo[ithread]->getSemValue()>(fifoSize-100))
		cprintf(CYAN,"Fifo[%d]: value:%d, pop 0x%x\n",ithread,fifo[ithread]->getSemValue(),(void*)(temp));
#endif
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
					FILE_LOG(logDEBUG4) << "*" << flush;
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
	FILE_LOG(logDEBUG) << __AT__ << " called";

	FILE_LOG(logINFO) << "Writing "<< ithread << ": End of Acquisition";

	//free fifo
	while(!fifoFree[ithread]->push(wbuffer));
#ifdef EVERYFIFODEBUG
	if(fifoFree[ithread]->getSemValue()<100)
		cprintf(GREEN,"FifoFree[%d]: value:%d, push 0x%x\n",ithread,fifoFree[ithread]->getSemValue(),(void*)(wbuffer));
#endif
#ifdef CFIFODEBUG
	if(ithread==0)
		cprintf(CYAN,"Writing_Thread %d: Freeing dummy-end buffer. Pushed into fifofree %p for listener %d\n", ithread,(void*)(wbuffer),ithread);
	else
		cprintf(YELLOW,"Writing_Thread %d: Freeing dummy-end buffer. Pushed into fifofree %p for listener %d\n", ithread,(void*)(wbuffer),ithread);
#endif

	if(dataStreamEnable){
		//ensure previous frame was processed
		sem_wait(&writerGuiSemaphore[ithread]);
		guiNumPackets[ithread] = dummyPacketValue;
		//let it know its got data
		sem_post(&dataCallbackWriterSemaphore[ithread]);
	}


	//all threads need to close file, reset mask and exit loop
	missingPacketinFile = (long long int)numberOfFrames*packetsPerFrame-totalWritingPacketCount[ithread];
	if(fileWriteEnable && (cbAction > DO_NOTHING) && missingPacketinFile){
		updateFileHeader(ithread);
		fseek(sfilefd[ithread],0,0);
		fwrite((void*)fileHeader[ithread], 1, FILE_HEADER_SIZE, sfilefd[ithread]);
	}

	if(totalWritingPacketCount[ithread]){
		if(frameNumberInPreviousFile[ithread]==-1)
			frameNumberInPreviousFile[ithread] = startFrameIndex-1;

		printf("\nThread:%d File:%s\n"
				"\ttotalpacketsinfile:%d\tPackets Lost:%d\tCurrentFrameNumber:%lld\tPreviousFrameNumber:%lld\n",
				ithread,completeFileName[ithread],
				totalPacketsInFile[ithread],
				( ((int)(currentFrameNumber[ithread]-frameNumberInPreviousFile[ithread])*packetsPerFrame) - totalPacketsInFile[ithread]),
				currentFrameNumber[ithread],frameNumberInPreviousFile[ithread]);
	}
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
		FILE_LOG(logINFO) << "Status: Run Finished";
		for(int i=0;i<numberofListeningThreads;i++){


			if(totalWritingPacketCount[i] < ((uint64_t)numberOfFrames*packetsPerFrame)){
				cprintf(RED, "\nPort %d\n",udpPortNum[i]);
				cprintf(RED, "Missing Packets   \t: %lld\n",(long long int)numberOfFrames*packetsPerFrame-totalWritingPacketCount[i]);
				cprintf(RED, "Packets Caught \t\t: %lld\n",(long long int)totalWritingPacketCount[i]);
				cprintf(RED, "Frames Caught  \t\t: %lld\n",(long long int)(totalWritingPacketCount[i]/packetsPerFrame));
				int64_t lastFrameNumber = 0;
				lastFrameNumber = lastFrameNumberInFile[i] - startFrameIndex;//lastFrameNumberInFile updated even if not written
				if(myDetectorType == EIGER)
					lastFrameNumber+= 1;
				cprintf(RED, "Last Frame Number Caught :%lld\n",(long long int)lastFrameNumber);
			}else{
				cprintf(GREEN, "\nPort %d\n",udpPortNum[i]);
				cprintf(GREEN, "Missing Packets   \t: %lld\n",(long long int)numberOfFrames*packetsPerFrame-totalWritingPacketCount[i]);
				cprintf(GREEN, "Packets Caught \t\t: %lld\n",(long long int)totalWritingPacketCount[i]);
				cprintf(GREEN, "Frames Caught  \t\t: %lld\n",(long long int)(totalWritingPacketCount[i]/packetsPerFrame));
				int64_t lastFrameNumber = 0;
				lastFrameNumber = lastFrameNumberInFile[i] - startFrameIndex;//lastFrameNumberInFile updated even if not written
				if(myDetectorType == EIGER)
					lastFrameNumber+= 1;
				cprintf(GREEN, "Last Frame Number Caught: %lld\n",(long long int)lastFrameNumber);
			}

		}
		if(!activated)
			cprintf(RED,"Note: Deactivated Receiver\n");
		//acquisition end
		if (acquisitionFinishedCallBack)
			acquisitionFinishedCallBack((int)totalPacketsCaught, pAcquisitionFinished);
	}
}




void UDPStandardImplementation::handleWithoutDataCompression(int ithread, char* wbuffer, uint32_t npackets){
	FILE_LOG(logDEBUG) << __AT__ << " called";


	//get current frame number
	uint64_t tempframenumber;
	uint32_t pnum;
	uint32_t snum;
	if(getFrameandPacketNumber(ithread, wbuffer + HEADER_SIZE_NUM_TOT_PACKETS,tempframenumber,pnum,snum) == FAIL){
		//error in frame number sent by fpga
		while(!fifoFree[ithread]->push(wbuffer));

		return;
	}

	//callback to write data
	if (cbAction < DO_EVERYTHING)
		rawDataReadyCallBack((int)tempframenumber, wbuffer + HEADER_SIZE_NUM_TOT_PACKETS, npackets * onePacketSize,
				sfilefd[ithread], latestData[ithread],pRawDataReady);//know which thread from sfilefd



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




void UDPStandardImplementation::writeFileWithoutCompression(int ithread, char* wbuffer,uint32_t numpackets){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	//if write enabled
	if((fileWriteEnable) && (sfilefd[ithread])){
		if(numpackets){
			int offset = HEADER_SIZE_NUM_TOT_PACKETS;
			uint64_t nextFileFrameNumber;
			int packetsWritten = 0;
			//if(ithread) cout<<"numpackets:"<<numpackets<<"lastframenumberinfile:"<<lastFrameNumberInFile[ithread]<<endl;

			//handle half frames from previous buffer
			//second part to not check when there has been something written previously
			if(numpackets &&(lastFrameNumberInFile[ithread]>=0)){
				//get start frame (required to create new file at the right juncture)
				uint64_t startframe =-1;
				uint32_t pnum;
				uint32_t snum;
				//if(ithread) cout<<"getting start frame number"<<endl;
				if(getFrameandPacketNumber(ithread, wbuffer + offset, startframe,pnum,snum) == FAIL){
					//error in frame number sent by fpga
					while(!fifoFree[ithread]->push(wbuffer));
					return;
				}
				//if(ithread) cout<<"done getting start frame number"<<endl;
				if(startframe == lastFrameNumberInFile[ithread]){
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
				currentFrameNumber[ithread] += lastFrameNumberInFile[ithread];
				if(!ithread)cprintf(BLUE,"currentFrameNumber[ithread]:%d\n",currentFrameNumber[ithread]);


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
			if(getFrameandPacketNumber(ithread, wbuffer + HEADER_SIZE_NUM_TOT_PACKETS + ((numpackets - 1) * onePacketSize), finalLastFrameNumberToSave,pnum,snum) == FAIL){
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
			"\nHeader\t\t %d bytes\n"
			"Top\t\t %d\n"
			"Left\t\t %d\n"
			"Active\t\t %d\n"
			"Packets Lost\t %d\n"
			"Dynamic Range\t %d\n"
			"Ten Giga\t %d\n"
			"Packet\t\t %d bytes\n"
			"Data\t\t %d bytes\n"
			"x\t\t %d pixels\n"
			"y\t\t %d pixels\n"
			"Timestamp\t %s\n\n"

			//only for eiger right now
			"#Packet Header\n"
			"Sub Frame Number 4 bytes\n"
			"Unused\t\t 2 bytes\n"
			"Port Number\t 1 byte\n"
			"Unused\t\t 1 byte\n\n"
			"#Packet Footer\n"
			"Frame Number\t 6 bytes\n"
			"Packet Number\t 2 bytes\n",
			FILE_HEADER_SIZE,
			(bottomEnable?0:1),(ithread?0:1),
			activated,
			missingPacketinFile,
			dynamicRange,tengigaEnable,
			onePacketSize,oneDataSize,
			//only for eiger right now
			EIGER_PIXELS_IN_ONE_ROW,EIGER_PIXELS_IN_ONE_COL,
			ctime(&t));
	if(strlen(fileHeader[ithread]) > FILE_HEADER_SIZE)
		cprintf(BG_RED,"File Header Size is too small for file header\n");


}

//called only if datacallback enabled
void UDPStandardImplementation::copyFrameToGui(int ithread, char* buffer, uint32_t numpackets){
	FILE_LOG(logDEBUG) << __AT__ << " called";

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
		memcpy(latestData[ithread],buffer+ HEADER_SIZE_NUM_TOT_PACKETS , numpackets*onePacketSize);
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
	FILE_LOG(logDEBUG) << __AT__ << " called";

	//get frame number
	uint64_t tempframenumber=-1;
	uint32_t pnum;
	uint32_t snum;
	if(getFrameandPacketNumber(ithread, wbuffer + HEADER_SIZE_NUM_TOT_PACKETS, tempframenumber,pnum,snum) == FAIL){
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
	char* data = wbuffer+ HEADER_SIZE_NUM_TOT_PACKETS;	//data pointer to the next memory to be analysed
	int ndata;												//size of data returned
	uint32_t np;													//remaining number of packets returned
	uint32_t npackets = (uint32_t)(*((uint32_t*)wbuffer));	//number of total packets
	int remainingsize = npackets * onePacketSize;			//size of the memory slot to be analyzed

	eventType thisEvent = PEDESTAL;
	int once = 0;
	int xmax = 0, ymax = 0;									//max pixels in x and y direction
	int xmin = 1, ymin = 1;									//min pixels in x and y direction
	double tot, tl, tr, bl, br;

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

	while(buff[0] = receiverData[ithread]->findNextFrame(data,ndata,remainingsize)){

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
						tot=0;
						tl=0;
						tr=0;
						bl=0;
						br=0;
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
		if(data > (wbuffer + HEADER_SIZE_NUM_TOT_PACKETS + npackets * onePacketSize) )
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



int UDPStandardImplementation::getFrameandPacketNumber(int ithread, char* wbuffer, uint64_t &framenumber, uint32_t &packetnumber,uint32_t &subframenumber){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	eiger_packet_footer_t* footer=0;
	eiger_packet_header_t* e_header=0;
	jfrau_packet_header_t* header=0;
	framenumber = 0;
	packetnumber = 0;
	subframenumber = 0;

	switch(myDetectorType){

	case EIGER:
		footer = (eiger_packet_footer_t*)(wbuffer + footerOffset);
		framenumber = (uint32_t)(*( (uint64_t*) footer));
		//error in frame number sent by fpga
		if(!((uint32_t)(*( (uint64_t*) footer)))){
			framenumber = -1;
			FILE_LOG(logERROR) << "Fifo "<< ithread << ": Frame Number is zero from firmware.";
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
		framenumber += (startFrameIndex - 1);
		break;

	case JUNGFRAU:
		header = (jfrau_packet_header_t*)(wbuffer);
		framenumber = (*( (uint32_t*) header->frameNumber))&frameIndexMask;
		packetnumber = (uint32_t)(*( (uint8_t*) header->packetNumber));
#ifdef DEBUG4
		cprintf(GREEN, "Writing_Thread %d: fnum:%lld\t pnum:%d\n",
				(long long int)framenumber,
				packetnumber);
#endif
		framenumber += startFrameIndex;
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
		framenumber += startFrameIndex;
		break;
	}
	return OK;
}





int UDPStandardImplementation::writeUptoFrameNumber(int ithread, char* wbuffer, int &offset, uint64_t nextFrameNumber, uint32_t numpackets, int &numPacketsWritten){
	FILE_LOG(logDEBUG) << __AT__ << " called";
	//if(ithread) cout<<"at writeUptoFrameNumber " << nextFrameNumber<< endl;


	int startoffset = offset;
	int endoffset = startoffset + numpackets * onePacketSize;
	uint64_t tempframenumber=-1;
	offset = endoffset;
	uint32_t pnum;
	uint32_t snum;
	//get last frame number
	if(getFrameandPacketNumber(ithread, wbuffer + (endoffset-onePacketSize), tempframenumber,pnum,snum) == FAIL){
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
		if(getFrameandPacketNumber(ithread, wbuffer + offset, tempframenumber,pnum,snum) == FAIL){
			//error in frame number sent by fpga
			while(!fifoFree[ithread]->push(wbuffer));
			return FAIL;
		}
	}
	if(offset<startoffset){
		offset = startoffset;//if(ithread) cout<<"offset < start offset f#:"<<tempframenumber<< " offset:"<<offset<<endl;
		if(getFrameandPacketNumber(ithread, wbuffer + offset, tempframenumber,pnum,snum) == FAIL){
			//error in frame number sent by fpga
			while(!fifoFree[ithread]->push(wbuffer));
			return FAIL;
		}
	}
	while(tempframenumber<nextFrameNumber){
		offset += onePacketSize;//if(ithread) cout<<"frame number at going forwards slow f#:"<<tempframenumber<< " offset:"<<offset<<endl;
		if(getFrameandPacketNumber(ithread, wbuffer + offset, tempframenumber,pnum,snum) == FAIL){
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








