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
#include <ctime>
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
	closeFile();
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
	closeFile();
	//filter
	deleteFilter();
	for(int i=0; i<numberofListeningThreads; i++){
		if(mem0[i])			{free(mem0[i]);			mem0[i] = NULL;}
		if(fifo[i])			{delete fifo[i];		fifo[i] = NULL;}
		if(fifoFree[i])		{delete fifoFree[i];	fifoFree[i] = NULL;}
	}
	if(latestData) 	{delete[] latestData; 	latestData = NULL;}
	guiData = NULL;
	//kill threads
	if(threadStarted){
		createListeningThreads(true);
		createWriterThreads(true);
	}
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
	frameSize = 0;
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
	strcpy(completeFileName,"");
	maxPacketsPerFile = 0;
	fileCreateSuccess = false;
	strcpy(fileHeader,"");

	//***acquisition indices parameters***
	startAcquisitionIndex = 0;
	startFrameIndex = 0;
	frameIndex = 0;
	currentFrameNumber = 0;
	previousFrameNumber = -1;
	lastFrameIndex = 0;
	acqStarted = false;
	measurementStarted = false;
	for(int i = 0; i < MAX_NUMBER_OF_LISTENING_THREADS; ++i){
		totalListeningFrameCount[i] = 0;
	}
	packetsInFile = 0;
	numMissingPackets = 0;
	numTotMissingPackets = 0;
	numTotMissingPacketsInFile = 0;


	//***receiver parameters***
	for(int i=0; i < MAX_NUMBER_OF_LISTENING_THREADS; i++){
		buffer[i] = NULL;
		mem0[i] = NULL;
		fifo[i] = NULL;
		fifoFree[i] = NULL;
		udpSocket[i] = NULL;
	}
	sfilefd = NULL;
	numberofJobsPerBuffer = -1;
	fifoSize = 0;

	//***receiver to GUI parameters***
	latestData = NULL;
	guiDataReady = false;
	guiData = NULL;
	strcpy(guiFileName,"");

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
		if(FrameToGuiFrequency)
			numberofJobsPerBuffer = FrameToGuiFrequency;
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

	if(myDetectorType == EIGER)
		fifoSize = fifoDepth * packetsPerFrame;//listens to 1 packet at a time and size depends on packetsperframe
	else
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
#ifdef DEBUG5
			cprintf(BLUE,"Info: %d fifostructure popped from fifofree %p\n", i, (void*)(buffer[i]));
#endif
			delete fifoFree[i];
		}
		if(fifo[i]){
			while(!fifo[i]->isEmpty()){
				fifo[i]->pop(buffer[i]);
				//cprintf(CYAN,"Fifo[%d]: value:%d, pop 0x%x\n",i,fifo[i]->getSemValue(),(void*)(buffer[i]));
			}
			delete fifo[i];
		}
		if(mem0[i]) 	free(mem0[i]);

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


/***file parameters***/
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


/***acquisition parameters***/
void UDPStandardImplementation::setShortFrameEnable(const int i){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	shortFrameEnable = i;

	if(shortFrameEnable!=-1){
		frameSize = GOTTHARD_SHORT_BUFFER_SIZE;
		bufferSize = GOTTHARD_SHORT_BUFFER_SIZE;
		onePacketSize = GOTTHARD_SHORT_BUFFER_SIZE;
		oneDataSize = GOTTHARD_SHORT_DATABYTES;
		maxPacketsPerFile = SHORT_MAX_FRAMES_PER_FILE * GOTTHARD_SHORT_PACKETS_PER_FRAME;
		packetsPerFrame = GOTTHARD_SHORT_PACKETS_PER_FRAME;
		frameIndexMask = GOTTHARD_SHORT_FRAME_INDEX_MASK;
		frameIndexOffset = GOTTHARD_SHORT_FRAME_INDEX_OFFSET;
		packetIndexMask = GOTTHARD_SHORT_PACKET_INDEX_MASK;

	}else{
		frameSize = GOTTHARD_BUFFER_SIZE;
		bufferSize = GOTTHARD_BUFFER_SIZE;
		onePacketSize = GOTTHARD_ONE_PACKET_SIZE;
		oneDataSize = GOTTHARD_ONE_DATA_SIZE;
		maxPacketsPerFile = MAX_FRAMES_PER_FILE * GOTTHARD_PACKETS_PER_FRAME;
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


int UDPStandardImplementation::setFrameToGuiFrequency(const uint32_t i){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	FrameToGuiFrequency = i;
	if(setupFifoStructure() == FAIL)
		return FAIL;

	FILE_LOG(logINFO) << "Frame to Gui Frequency: " << FrameToGuiFrequency;

	return OK;
}


int UDPStandardImplementation::setAcquisitionPeriod(const uint64_t i){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	acquisitionPeriod = i;
	if((myDetectorType == GOTTHARD) && (myDetectorType == MOENCH))
		if(setupFifoStructure() == FAIL)
			return FAIL;

	FILE_LOG(logINFO) << "Acquisition Period: " << (double)acquisitionPeriod/(1E9) << "s";

	return OK;
}


int UDPStandardImplementation::setNumberOfFrames(const uint64_t i){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	numberOfFrames = i;
	if((myDetectorType == GOTTHARD) && (myDetectorType == MOENCH))
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
		packetsPerFrame 	= (tengigaEnable ? EIGER_TEN_GIGA_CONSTANT : EIGER_ONE_GIGA_CONSTANT)
																* dynamicRange * EIGER_MAX_PORTS;
		frameSize			= onePacketSize * packetsPerFrame;
		maxPacketsPerFile 	= EIGER_MAX_FRAMES_PER_FILE * packetsPerFrame;

		updateFileHeader();

		//new dynamic range, then restart threads and resetup fifo structure
		if(oldDynamicRange != dynamicRange){

			//gui buffer
			if(latestData){delete[] latestData; latestData = NULL;}
			latestData = new char[frameSize];

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
			packetsPerFrame = EIGER_TEN_GIGA_CONSTANT * dynamicRange * EIGER_MAX_PORTS;
			onePacketSize  	= EIGER_TEN_GIGA_ONE_PACKET_SIZE;
			oneDataSize 	= EIGER_TEN_GIGA_ONE_DATA_SIZE;
		}else{
			packetsPerFrame = EIGER_ONE_GIGA_CONSTANT * dynamicRange * EIGER_MAX_PORTS;
			onePacketSize  	= EIGER_ONE_GIGA_ONE_PACKET_SIZE;
			oneDataSize		= EIGER_ONE_GIGA_ONE_DATA_SIZE;
		}
		frameSize			= onePacketSize * packetsPerFrame;
		bufferSize 			= onePacketSize;
		maxPacketsPerFile 	= EIGER_MAX_FRAMES_PER_FILE * packetsPerFrame;
		footerOffset		= EIGER_PACKET_HEADER_SIZE + oneDataSize;
		FILE_LOG(logDEBUG) << dec <<
				"packetsPerFrame:" << packetsPerFrame <<
				"\nonePacketSize:" << onePacketSize <<
				"\noneDataSize:" << oneDataSize <<
				"\nframesize:" << frameSize <<
				"\nbufferSize:" << bufferSize <<
				"\nmaxPacketsPerFile:" << maxPacketsPerFile;


		updateFileHeader();

		//new enable, then restart threads and resetup fifo structure
		if(oldTenGigaEnable != tengigaEnable){

			//gui buffer
			if(latestData){delete[] latestData; latestData = NULL;}
			latestData = new char[frameSize];

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
		frameSize 			= GOTTHARD_BUFFER_SIZE;
		bufferSize 			= GOTTHARD_BUFFER_SIZE;
		frameIndexMask 		= GOTTHARD_FRAME_INDEX_MASK;
		frameIndexOffset 	= GOTTHARD_FRAME_INDEX_OFFSET;
		packetIndexMask 	= GOTTHARD_PACKET_INDEX_MASK;
		maxPacketsPerFile	= MAX_FRAMES_PER_FILE * GOTTHARD_PACKETS_PER_FRAME;
		fifoSize			= GOTTHARD_FIFO_SIZE;
		fifoDepth			= GOTTHARD_FIFO_SIZE;
		//footerOffset		= Not applicable;
		break;
	case PROPIX:
		packetsPerFrame		= PROPIX_PACKETS_PER_FRAME;
		onePacketSize 		= PROPIX_ONE_PACKET_SIZE;
		//oneDataSize 		= Not applicable;
		frameSize 			= PROPIX_BUFFER_SIZE;
		bufferSize 			= PROPIX_BUFFER_SIZE;
		frameIndexMask 		= PROPIX_FRAME_INDEX_MASK;
		frameIndexOffset 	= PROPIX_FRAME_INDEX_OFFSET;
		packetIndexMask 	= PROPIX_PACKET_INDEX_MASK;
		maxPacketsPerFile	= MAX_FRAMES_PER_FILE * PROPIX_PACKETS_PER_FRAME;
		fifoSize			= PROPIX_FIFO_SIZE;
		fifoDepth			= PROPIX_FIFO_SIZE;
		//footerOffset		= Not applicable;
		break;
	case MOENCH:
		packetsPerFrame		= MOENCH_PACKETS_PER_FRAME;
		onePacketSize 		= MOENCH_ONE_PACKET_SIZE;
		oneDataSize 		= MOENCH_ONE_DATA_SIZE;
		frameSize 			= MOENCH_BUFFER_SIZE;
		bufferSize 			= MOENCH_BUFFER_SIZE;
		frameIndexMask 		= MOENCH_FRAME_INDEX_MASK;
		frameIndexOffset 	= MOENCH_FRAME_INDEX_OFFSET;
		packetIndexMask 	= MOENCH_PACKET_INDEX_MASK;
		maxPacketsPerFile	= MOENCH_MAX_FRAMES_PER_FILE * MOENCH_PACKETS_PER_FRAME;
		fifoSize			= MOENCH_FIFO_SIZE;
		fifoDepth 			= MOENCH_FIFO_SIZE;
		//footerOffset		= Not applicable;
		break;
	case EIGER:
		//assuming 1G in the beginning
		packetsPerFrame		= EIGER_ONE_GIGA_CONSTANT * dynamicRange * EIGER_MAX_PORTS;
		onePacketSize 		= EIGER_ONE_GIGA_ONE_PACKET_SIZE;
		oneDataSize 		= EIGER_ONE_GIGA_ONE_DATA_SIZE;
		frameSize 			= onePacketSize * packetsPerFrame;
		bufferSize 			= onePacketSize;
		frameIndexMask 		= EIGER_FRAME_INDEX_MASK;
		frameIndexOffset 	= EIGER_FRAME_INDEX_OFFSET;
		packetIndexMask 	= EIGER_PACKET_INDEX_MASK;
		maxPacketsPerFile	= EIGER_MAX_FRAMES_PER_FILE * packetsPerFrame;
		fifoSize			= EIGER_FIFO_SIZE;
		fifoDepth			= EIGER_FIFO_SIZE;
		footerOffset		= EIGER_PACKET_HEADER_SIZE + oneDataSize;
		break;
	case JUNGFRAUCTB:
		packetsPerFrame		= JCTB_PACKETS_PER_FRAME;
		onePacketSize 		= JCTB_ONE_PACKET_SIZE;
		//oneDataSize 		= Not applicable;
		frameSize 			= JCTB_BUFFER_SIZE;
		bufferSize 			= JCTB_BUFFER_SIZE;
		frameIndexMask 		= JCTB_FRAME_INDEX_MASK;
		frameIndexOffset 	= JCTB_FRAME_INDEX_OFFSET;
		packetIndexMask 	= JCTB_PACKET_INDEX_MASK;
		maxPacketsPerFile	= JFCTB_MAX_FRAMES_PER_FILE * JCTB_PACKETS_PER_FRAME;
		fifoSize			= JCTB_FIFO_SIZE;
		fifoDepth			= JCTB_FIFO_SIZE;
		//footerOffset		= Not applicable;
		break;
	case JUNGFRAU:
		packetsPerFrame		= JFRAU_PACKETS_PER_FRAME;
		onePacketSize 		= JFRAU_ONE_PACKET_SIZE;
		oneDataSize 		= JFRAU_DATA_BYTES;
		frameSize 			= JFRAU_BUFFER_SIZE;
		bufferSize 			= JFRAU_BUFFER_SIZE;
		frameIndexMask 		= JFRAU_FRAME_INDEX_MASK;
		frameIndexOffset 	= JFRAU_FRAME_INDEX_OFFSET;
		packetIndexMask 	= JFRAU_PACKET_INDEX_MASK;
		maxPacketsPerFile	= JFRAU_MAX_FRAMES_PER_FILE * JFRAU_PACKETS_PER_FRAME;
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
		listeningThreadsMask = 0x0;
		pthread_mutex_unlock(&(statusMutex));
		if(threadStarted)
			createListeningThreads(true);
		numberofListeningThreads = MAX_NUMBER_OF_LISTENING_THREADS;
	}

	//set up fifo structure -1 for numberofJobsPerBuffer ensure it is done
	numberofJobsPerBuffer = -1;
	setupFifoStructure();

	//allocate for latest data (frame copy for gui)
	latestData = new char[frameSize];

	//updates File Header
	if(myDetectorType == EIGER)
		updateFileHeader();

	FILE_LOG(logDEBUG) << " Detector type set to " << getDetectorType(d);

	return OK;
}


/***acquisition functions***/
void UDPStandardImplementation::resetAcquisitionCount(){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	totalPacketsCaught = 0;
	acqStarted = false;
	startAcquisitionIndex = 0;

	FILE_LOG(logINFO) << "Acquisition Count has been reset";
}


int UDPStandardImplementation::startReceiver(char *c){
	FILE_LOG(logDEBUG) << __AT__ << " called";
	
	FILE_LOG(logINFO)  << "Stopping Receiver";


	//RESET
	//reset measurement variables
	measurementStarted = false;
	startFrameIndex = 0;
	frameIndex = 0;
	if(!acqStarted){
		currentFrameNumber = 0;		//has to be zero to add to startframeindex for each scan
		acquisitionIndex = 0;
		frameIndex = 0;
	}
	for(int i = 0; i < numberofListeningThreads; ++i)
		totalListeningFrameCount[i] = 0;
	packetsCaught = 0;
	numMissingPackets = 0;
	numTotMissingPackets = 0;
	numTotMissingPacketsInFile = 0;
	//reset file parameters
	packetsInFile = 0;
	if(sfilefd){
		fclose(sfilefd);
		sfilefd = NULL;
	}
	//reset gui variables
	guiData = NULL;
	guiDataReady=0;
	strcpy(guiFileName,"");
	//reset masks
	pthread_mutex_lock(&statusMutex);
	writerThreadsMask = 0x0;
	createFileMask = 0x0;
	fileCreateSuccess = false;
	pthread_mutex_unlock(&statusMutex);


	//Print Receiver Configuration
	if(myDetectorType != EIGER){
		FILE_LOG(logINFO) << "Data Compression has been " << stringEnable(dataCompressionEnable);
		FILE_LOG(logINFO) << "Number of Jobs Per Buffer: " << numberofJobsPerBuffer;
		FILE_LOG(logINFO) << "Max Packets Per File:" << maxPacketsPerFile;
	}
	if(FrameToGuiFrequency)
		FILE_LOG(logINFO) << "Frequency of frames sent to gui: " << FrameToGuiFrequency;
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
		sprintf(c,"Could not create file %s.",completeFileName);
		//FILE_LOG(logERROR) << c;
		for(int i=0; i < numberofWriterThreads; i++)	sem_post(&writerSemaphore[i]);
		return FAIL;
	}


	//For compression, just for gui purposes
	if(dataCompressionEnable)
		sprintf(completeFileName, "%s/%s_fxxx_%lld_xx.root", filePath,fileName,(long long int)fileIndex);

	//initialize semaphore to synchronize between writer and gui reader threads
	sem_init(&writerGuiSemaphore,1,0);

	//status and thread masks
	pthread_mutex_lock(&statusMutex);
	status = RUNNING;
	for(int i=0;i<numberofListeningThreads;i++)		listeningThreadsMask|=(1<<i);
	for(int i=0;i<numberofWriterThreads;i++)		writerThreadsMask|=(1<<i);
	pthread_mutex_unlock(&(statusMutex));


	//start listening /writing
	for(int i=0;i<numberofListeningThreads;i++)		sem_post(&listenSemaphore[i]);
	for(int i=0; i < numberofWriterThreads; i++)	sem_post(&writerSemaphore[i]);

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
		sem_post(&writerGuiSemaphore);
		usleep(5000);
	}

	//semaphore destroy
	sem_destroy(&writerGuiSemaphore);

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

		//check if all packets got
		int totalP = 0,prev,i;
		for(i=0; i<numberofListeningThreads; ++i){
			totalP += totalListeningFrameCount[i];
		}
		//wait for all packets
		if(totalP!=numberOfFrames*packetsPerFrame){

			prev = -1;
			//wait as long as there is change from prev totalP
			while(prev != totalP){
#ifdef DEBUG5
				cprintf(MAGENTA,"waiting for all packets totalP:%d\n",totalP);
#endif

				usleep(5000);/* Need to find optimal time (exposure time and acquisition period) **/
				prev = totalP;
				totalP=0;
				for(i=0; i<numberofListeningThreads; ++i){
					totalP += totalListeningFrameCount[i];
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




void UDPStandardImplementation::readFrame(char* c,char** raw, uint64_t &startAcq, uint64_t &startFrame){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	//point to gui data, to let writer thread know that gui is back for data
	if (guiData == NULL){
		guiData = latestData;
#ifdef DEBUG4
		cprintf(CYAN,"Info: gui data not null anymore - ready to get data\n");
#endif
	}

	//copy data and filename
	strcpy(c,guiFileName);
	startAcq = startAcquisitionIndex;
	startFrame = startFrameIndex;


	//gui data not copied yet
	if(!guiDataReady){
#ifdef DEBUG4
		cprintf(CYAN,"Info: gui data not ready\n");
#endif
		*raw = NULL;
	}

	//gui data ready, pass address to gui to copy the data
	else{
#ifdef DEBUG4
		cprintf(CYAN,"Info: gui data ready\n");
#endif
		*raw = guiData;
		guiData = NULL;

		//for nth frame to gui, post semaphore so writer stops waiting
		if((FrameToGuiFrequency) && (writerThreadsMask)){
#ifdef DEBUG4
			cprintf(CYAN,"Info: gonna post\n");
#endif
			//release after getting data
			sem_post(&writerGuiSemaphore);
		}
#ifdef DEBUG4
		cprintf(CYAN,"Info: done post\n");
#endif

	}
}



void UDPStandardImplementation::closeFile(int i){
	FILE_LOG(logDEBUG) << __AT__ << " called for " << i ;

	//normal
	if(!dataCompressionEnable){
		if(sfilefd){
#ifdef DEBUG4
			FILE_LOG(logDEBUG4) << "Going to close file: " << fileno(sfilefd));
#endif
			fclose(sfilefd);
			sfilefd = NULL;
		}
	}

	//compression
	else{
#if (defined(MYROOT1) && defined(ALLFILE_DEBUG)) || !defined(MYROOT1)
		if(sfilefd){
#ifdef DEBUG4
			FILE_LOG(logDEBUG4) << "sfield: " << (int)sfilefd;
#endif
			fclose(sfilefd);
			sfilefd = NULL;
		}
#endif

#ifdef MYROOT1
		pthread_mutex_lock(&writeMutex);
		//write to file
		if(myTree[i] && myFile[i]){
			myFile[i] = myTree[i]->GetCurrentFile();

			if(myFile[i]->Write())
				//->Write(tall->GetName(),TObject::kOverwrite);
				cout << "Thread " << i <<": wrote frames to file" << endl;
			else
				cout << "Thread " << i << ": could not write frames to file" << endl;

		}else
			cout << "Thread " << i << ": could not write frames to file: No file or No Tree" << endl;
		//close file
		if(myTree[i] && myFile[i])
			myFile[i] = myTree[i]->GetCurrentFile();
		if(myFile[i] != NULL)
			myFile[i]->Close();
		myFile[i] = NULL;
		myTree[i] = NULL;
		pthread_mutex_unlock(&writeMutex);

#endif
	}
}




/*************************************************************************
 * Listening and Writing Threads *****************************************
 *************************************************************************/


int UDPStandardImplementation::createListeningThreads(bool destroy){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

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
			FILE_LOG(logDEBUG) << "." << flush;
		}
		killAllListeningThreads = false;
		threadStarted = false;
		FILE_LOG(logDEBUG) << "Info: Listening thread(s) destroyed";
	}

	//create
	else{
		FILE_LOG(logDEBUG) << "Info: Creating Listening Thread(s)";

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
			FILE_LOG(logDEBUG) <<"."<<flush;
		}
		killAllWritingThreads = false;
		threadStarted = false;
		FILE_LOG(logDEBUG) << "Info: Writer thread(s) destroyed";
	}

	//create threads
	else{
		FILE_LOG(logDEBUG) << "Info: Creating Writer Thread(s)";

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

	struct sched_param tcp_param, listen_param, write_param;
	bool rights = true;

	//assign priorities
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
	if (pthread_setschedparam(pthread_self(),5 , &tcp_param) == EPERM)
		rights = false;

	if(!rights){
		FILE_LOG(logWARNING) << "No root permission to prioritize threads.";
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

	//if no eth, listen to all
	if(!strlen(eth)){
		FILE_LOG(logWARNING) << "eth is empty. Listening to all";

		for(int i=0;i<numberofListeningThreads;i++)
			udpSocket[i] = new genericSocket(port[i],genericSocket::UDP,bufferSize);
	}
	//normal socket
	else{
		FILE_LOG(logINFO) << "Ethernet Interface:" << eth;

		for(int i=0;i<numberofListeningThreads;i++)
			udpSocket[i] = new genericSocket(port[i],genericSocket::UDP,bufferSize,eth);
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
		cbAction=startAcquisitionCallBack(filePath,fileName,(int)fileIndex,bufferSize,pStartAcquisition);

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
	for(int i=0; i<numberofWriterThreads; i++)		createFileMask|=(1<<i);
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


	if(dataCompressionEnable){
#if (defined(MYROOT1) && defined(ALLFILE_DEBUG)) || !defined(MYROOT1)
		if(fileCreateSuccess != FAIL)
			fileCreateSuccess = createNewFile();
#endif
	}

	if(fileCreateSuccess == OK){
		FILE_LOG(logDEBUG) << "Successfully created file(s)";
		cout << "Writer Ready ..." << endl;
	}

	return fileCreateSuccess;
}



int UDPStandardImplementation::createNewFile(){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	int index = 0;
	if(packetsCaught)
		index = frameIndex;

	//create file name
	if(!frameIndexEnable)
		sprintf(completeFileName, "%s/%s_%lld.raw", filePath,fileName,(long long int)fileIndex);
	else if (myDetectorType == EIGER)
		sprintf(completeFileName, "%s/%s_f%012lld_%lld.raw", filePath,fileName,(long long int)currentFrameNumber,(long long int)fileIndex);
	else
		sprintf(completeFileName, "%s/%s_f%012lld_%lld.raw", filePath,fileName,(long long int)(packetsCaught/packetsPerFrame),(long long int)fileIndex);

#ifdef DEBUG4
	FILE_LOG(logINFO) << completefileName;
#endif

	//filewrite enable & we allowed to create/close files
	if(fileWriteEnable && cbAction > DO_NOTHING){

		//close file pointers
		if(sfilefd){
			fclose(sfilefd);
			sfilefd = NULL;
		}

		//create file
		if(!overwriteEnable){
			if (NULL == (sfilefd = fopen((const char *) (completeFileName), "wx"))){
				FILE_LOG(logERROR) << "Could not create/overwrite file" << completeFileName;
				return FAIL;
			}
		}else if (NULL == (sfilefd = fopen((const char *) (completeFileName), "w"))){
			FILE_LOG(logERROR) << "Could not create file" << completeFileName;
			return FAIL;
		}
		//setting file buffer size to 16mb
		setvbuf(sfilefd,NULL,_IOFBF,BUF_SIZE);

		//Print packet loss and filenames
		if(!packetsCaught){
			previousFrameNumber = -1;
			cout << "File: " << completeFileName << endl;
		}else{
			if (previousFrameNumber == -1)
				previousFrameNumber = startFrameIndex-1;

			cout << completeFileName
					<< "\tPacket Loss: " << setw(4)<<fixed << setprecision(4) << dec <<
					(int)((( (currentFrameNumber-previousFrameNumber) - ((packetsInFile-numTotMissingPacketsInFile)/packetsPerFrame))/
							 (double)(currentFrameNumber-previousFrameNumber))*100.000)
					<< "%\tFramenumber: " << currentFrameNumber
				  << "\t\t PreviousFrameNumber: " << previousFrameNumber
					<< "\tIndex " << dec << index
					<< "\tLost " << dec << ( ((int)(currentFrameNumber-previousFrameNumber)) -
							                 ((packetsInFile-numTotMissingPacketsInFile)/packetsPerFrame)) << endl;

		}

		//write file header
		if(myDetectorType == EIGER)
			fwrite((void*)fileHeader, 1, strlen(fileHeader), sfilefd);
	}

	//reset counters for each new file
	if(packetsCaught){
		previousFrameNumber = currentFrameNumber;
		packetsInFile = 0;
		numTotMissingPacketsInFile = 0;
	}



	return OK;
}




int UDPStandardImplementation::createCompressionFile(int ithread, int iframe){
	FILE_LOG(logDEBUG) << __AT__ << " called";

#ifdef MYROOT1
	char temp[MAX_STR_LENGTH];
		//create file name for gui purposes, and set up acquistion parameters
		sprintf(temp, "%s/%s_fxxx_%d_%d.root", filePath,fileName,fileIndex,ithread);
		//file
		myFile[ithread] = new TFile(temp,"RECREATE");/** later  return error if it exists */
		cprintf(GREEN,"Writing_Thread %d: Created Compression File: %s\n",ithread, temp);
		//tree
		sprintf(temp, "%s_fxxx_%d_%d",fileName,fileIndex,ithread);
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






void UDPStandardImplementation::startListening(){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	//set current thread value  index
	int ithread = currentThreadIndex;
	//let calling function know thread started and obtained current
	threadStarted = 1;


	//variable definitions
	int listenSize = 0; 		//listen to only 1 packet
	uint32_t rc;				//size of buffer received in bytes
	//split frames
	int carryonBufferSize; 		//from previous buffer to keep frames together in a buffer
	char* tempBuffer = NULL;	//temporary buffer to store split frames

	/* outer loop - loops once for each acquisition */
	//infinite loop, exited only to change dynamic range, 10G parameters etc (then recreated again)
	while(true){

		//reset parameters before acquisition
		carryonBufferSize = 0;
		if(myDetectorType != EIGER){
			listenSize = bufferSize * numberofJobsPerBuffer;				//listen to more than 1 packet
			if(tempBuffer!=NULL){delete []tempBuffer;tempBuffer=NULL;}
			tempBuffer = new char[onePacketSize * (packetsPerFrame - 1)]; 	//store maximum of 1 packets less in a frame
		}

		/* inner loop - loop for each buffer */
		//until mask unset (udp sockets shut down by client)
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
			if(udpSocket[ithread] == NULL){
				FILE_LOG(logERROR) << "Listening_Thread " << ithread << ": UDP Socket not created or shut down earlier";
				stopListening(ithread,0);
				continue;
			}

			rc = prepareAndListenBuffer(ithread, listenSize, carryonBufferSize, tempBuffer);

			//start indices for each start of scan/acquisition
			if((!measurementStarted) && (rc > 0)){
				pthread_mutex_lock(&progressMutex);
				if(!measurementStarted)
					startFrameIndices(ithread);
				pthread_mutex_unlock(&progressMutex);
			}


			//problem in receiving or end of acquisition
			if (status == TRANSMITTING){
				stopListening(ithread,rc);
				continue;
			}
			//write packet count to buffer
			if(myDetectorType == EIGER)
				(*((uint32_t*)(buffer[ithread]))) = 1;
			//handling split frames and writing packet Count to buffer
			else
				(*((uint32_t*)(buffer[ithread]))) = processListeningBuffer(ithread, carryonBufferSize, tempBuffer);


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





int UDPStandardImplementation::prepareAndListenBuffer(int ithread, int lSize, int cSize, char* temp){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	//listen to UDP packets
	if(cSize)
		memcpy(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS, temp, cSize);

	int receivedSize = udpSocket[ithread]->ReceiveDataOnly(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS + cSize, lSize + cSize);

	//throw away packets that is not one packet size, need to check status if socket is shut down
	while(status != TRANSMITTING && myDetectorType == EIGER && receivedSize != onePacketSize) {
		if(receivedSize != EIGER_HEADER_LENGTH){
			cprintf(RED,"Listening_Thread %d: Listened to a weird packet size %d\n",ithread, receivedSize);
		}
#ifdef DEBUG
		else
			cprintf(BLUE,"Listening_Thread %d: Listened to a header packet\n",ithread);
#endif
		receivedSize = udpSocket[ithread]->ReceiveDataOnly(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS);
	}
	totalListeningFrameCount[ithread] += (receivedSize/onePacketSize);

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
		startAcquisitionIndex = startFrameIndex;
		acqStarted = true;
		cprintf(BLUE,"Listening_Thread %d: startAcquisitionIndex:%lld\n",ithread,(long long int)startAcquisitionIndex);
	}

	//set start of scan/real time measurement
	cprintf(BLUE,"Listening_Thread %d: startFrameIndex: %lld\n", ithread,(long long int)startFrameIndex);
	measurementStarted = true;
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
		totalListeningFrameCount[ithread] += (numbytes/onePacketSize);
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
	for(int i=0; i<numberofWriterThreads; ++i){
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
	}


	//reset mask and exit loop
	pthread_mutex_lock(&statusMutex);
	listeningThreadsMask^=(1<<ithread);
#ifdef DEBUG4
	cprintf(BLUE,"Listening_Thread %d: Resetting mask of listening thread. New Mask: 0x%x", ithread, listeningThreadsMask);
	cprintf(BLUE,"Listening_Thread %d: Frames listened to :%d\n",ithread, ((totalListeningFrameCount[ithread]*numberofListeningThreads)/packetsPerFrame));
#endif
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
			t += totalListeningFrameCount[i];
		cprintf(BLUE,"Listening_Thread %d :Total Frames listened to %d\n", ithread,(t/packetsPerFrame));
#endif
	}
}





uint32_t UDPStandardImplementation::processListeningBuffer(int ithread, int cSize, char* temp){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	int lastPacketOffset;		//the offset of the last packet
	uint32_t lastFrameHeader;		//frame number of last packet in buffer
	uint64_t lastFrameHeader64;		//frame number of last packet in buffer
	uint32_t packetCount = (packetsPerFrame/numberofListeningThreads) * numberofJobsPerBuffer;		//packets received
	cSize = 0;					//reset size
	jfrau_packet_header_t* header;

	switch(myDetectorType){
	case GOTTHARD:
	case PROPIX:
		//for short frames, 1 packet/frame, so split frames is not a topic
		if(shortFrameEnable == -1){
			lastPacketOffset = (((numberofJobsPerBuffer * packetsPerFrame - 1) * onePacketSize) + HEADER_SIZE_NUM_TOT_PACKETS);
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
		lastPacketOffset = (((numberofJobsPerBuffer * packetsPerFrame - 1) * onePacketSize) + HEADER_SIZE_NUM_TOT_PACKETS);
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
		lastPacketOffset = (((numberofJobsPerBuffer * packetsPerFrame - 1) * onePacketSize) + HEADER_SIZE_NUM_TOT_PACKETS);
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
			while (lastFrameHeader64 == (*( (uint32_t*) header->frameNumber))&0xffffff){
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

	switch(myDetectorType){
	case EIGER:
		processWritingBufferPacketByPacket(ithread);
		break;
	default:
		processWritingBuffer(ithread);
		break;
	}

}



void UDPStandardImplementation::processWritingBuffer(int ithread){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	//variable definitions
	char* wbuf[numberofListeningThreads];				//buffer popped from FIFO
	sfilefd = NULL;										//file pointer
	uint64_t nf;												//for compression, number of frames


	/* outer loop - loops once for each acquisition */
	//infinite loop, exited only to change dynamic range, 10G parameters etc (then recreated again)
	while(true){

		//--reset parameters before acquisition
		nf = 0;
		guiData = latestData;  //so that the first frame is always copied


		/* inner loop - loop for each buffer */
		//until mask unset (udp sockets shut down by client)
		while((1 << ithread) & writerThreadsMask){
			//pop
			fifo[0]->pop(wbuf[0]);
#ifdef EVERYFIFODEBUG
			if(fifo[0]->getSemValue()>(fifoSize-100))
			cprintf(CYAN,"Fifo[%d]: value:%d, pop 0x%x\n",0,fifo[0]->getSemValue(),(void*)(wbuf[0]));
#endif
#ifdef DEBUG5
			cprintf(GREEN,"Writing_Thread %d: Popped %p from FIFO %d\n", ithread, (void*)(wbuf[0]),0);
#endif
			uint32_t numPackets = (uint32_t)(*((uint32_t*)wbuf[0]));
#ifdef DEBUG4
			cprintf(GREEN,"Writing_Thread %d: Number of Packets: %d for FIFO %d\n", ithread, numPackets, 0);
#endif


			//end of acquisition
			if(numPackets == dummyPacketValue){
#ifdef DEBUG3
				cprintf(GREEN,"Writing_Thread %d: Dummy frame popped out of FIFO %d",ithread, 0);
#endif
				stopWriting(ithread,wbuf);
				continue;
			}



			//process
			if(!dataCompressionEnable)
				handleWithoutDataCompression(ithread, wbuf, numPackets);
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








void UDPStandardImplementation::processWritingBufferPacketByPacket(int ithread){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	//variable definitions
	char* packetBuffer[numberofListeningThreads];		//buffer popped from FIFO
	sfilefd = NULL;										//file pointer
	bool popReady[numberofListeningThreads];			//if the FIFO can be popped
	uint32_t numPackets[numberofListeningThreads];		//number of packets popped from the FIFO

	int MAX_NUM_PACKETS = 1024;							//highest 32 bit has 1024 number of packets
	uint32_t LAST_PACKET_VALUE;							//last packet number

	CircularFifo<char>* fifoTempFree[numberofListeningThreads];//ciruclar fifo to keep track of one frame packets to be freed and reused later
	char* temp = NULL;

	char* frameBuffer[MAX_NUM_PACKETS];					//buffer offset created for a whole frame
	int frameBufferoffset[numberofListeningThreads];	//buffer offset created for a whole frame for both listening threads
	char* blankframe[MAX_NUM_PACKETS];					//blank buffer for a whole frame with missing packets
	int blankoffset;									//blank buffer offset

	bool fullframe[numberofListeningThreads];			//if full frame processed for each listening thread
	volatile uint32_t threadFrameNumber[numberofListeningThreads];	//thread frame number for each listening thread buffer popped out
	volatile uint32_t presentFrameNumber;							//the  current frame number aiming to be built
	volatile uint32_t lastPacketNumber[numberofListeningThreads];	//last packet number got
	volatile uint32_t currentPacketNumber[numberofListeningThreads];//current packet number
	volatile int numberofMissingPackets[numberofListeningThreads];	// number of missing packets in this buffer

	for(int i=0; i<MAX_NUM_PACKETS; ++i){
		frameBuffer[i] = NULL;
		blankframe[i] = NULL;
	}
	for(int i=0; i<numberofListeningThreads; ++i){
		fifoTempFree[i] = NULL;
	}

	/* outer loop - loops once for each acquisition */
	//infinite loop, exited only to change dynamic range, 10G parameters etc (then recreated again)
	while(true){

		//--reset parameters before acquisition
		presentFrameNumber = 0;
		blankoffset = 0;		//blank frame - initializing with missing packet values
		guiData = latestData;     //so that the first frame is always copied
		LAST_PACKET_VALUE = (packetsPerFrame/numberofListeningThreads);

		for(int i=0; i<numberofListeningThreads; ++i){
			packetBuffer[i] = NULL;
			popReady[i] = true;
			numPackets[i] = 0;
			frameBufferoffset[i] = (i*packetsPerFrame/numberofListeningThreads);
			fullframe[i] = false;
			threadFrameNumber[i] = 0;
			lastPacketNumber[i] = 0;
			currentPacketNumber[i] = 0;
			numberofMissingPackets[i] = 0;

			//circular temp fifo between getting a whole frame and freeing them
			if(fifoTempFree[i]){
				while(!fifoTempFree[i]->isEmpty()){
					fifoTempFree[i]->pop(temp);
#ifdef EVERYFIFODEBUG
					if(fifoTempFree[i]->getSemValue()>((packetsPerFrame/numberofListeningThreads)-3))
					cprintf(RED,"FifoTempFree[%d]: value:%d, pop 0x%x\n",i,fifoTempFree[i]->getSemValue(),(void*)(temp));
#endif
				}
				delete fifoTempFree[i];
			}
			fifoTempFree[i] = new CircularFifo<char>(MAX_NUM_PACKETS);
		}

		for(uint32_t i=0; i<packetsPerFrame; ++i){
			if(blankframe[i]){delete [] blankframe[i]; blankframe[i] = 0;}
			blankframe[i] = new char[onePacketSize];
			//set missing packet to 0xff
			eiger_packet_header_t* blankframe_header = (eiger_packet_header_t*) blankframe[i];
			eiger_packet_footer_t* blankframe_footer = (eiger_packet_footer_t*)(blankframe[i] + footerOffset);
			*( (uint16_t*) blankframe_header->missingPacket) = missingPacketValue;
			*( (uint16_t*) blankframe_footer->packetNumber) = i+1;

			//set each value inside blank frame to 0xff
			for(int j=0;j<(oneDataSize);++j){
				unsigned char* blankframe_data = (unsigned char*)blankframe[i] + sizeof(eiger_packet_header_t) + j;
				*(blankframe_data) = 0xFF;
			}
		}
		//last frame read out
		lastFrameIndex = -1;




		/* inner loop - loop for each buffer */
		//until mask unset (udp sockets shut down by client)
		while((1 << ithread) & writerThreadsMask){


			//pop fifo and if end of acquisition
			//cprintf(BLUE,"popready[0]:%d popready[1]:%d\n",popReady[0],popReady[1]);
			if(popAndCheckEndofAcquisition(ithread, packetBuffer, popReady, numPackets,fifoTempFree)){
#ifdef DEBUG4
				cprintf(GREEN,"Writing_Thread All dummy-end buffers popped\n");
#endif
				//finish missing packets
				if(((frameBufferoffset[0]!=0) || (frameBufferoffset[1]!=((int)packetsPerFrame/numberofListeningThreads))));
				else{
					stopWriting(ithread,packetBuffer);
					continue;
				}
			}
#ifdef DEBUG4
			else{cprintf(BLUE,"POPped but i see?\n");}
#endif

			//get a full frame-------------------------------------------------------------------------------------------------------
			for(int i=0;i<numberofListeningThreads;++i){

				numberofMissingPackets[i] = 0;


				//dummy done-----------------------------------------------------------------------------------------------------------
				if(numPackets[i] == dummyPacketValue && frameBufferoffset[i] == (((i+1)*(int)packetsPerFrame/numberofListeningThreads))){
#ifdef DEBUG4
					cprintf(RED,"dummy done\n");
#endif
					continue;
				}


				//not full frame
				else if(!fullframe[i]){

					//update frame number and packet number
					if(numPackets[i] != dummyPacketValue){
						eiger_packet_footer_t* packetBuffer_footer = (eiger_packet_footer_t*)(packetBuffer[i] + footerOffset + HEADER_SIZE_NUM_TOT_PACKETS);

						if(!((uint32_t)(*( (uint64_t*) packetBuffer_footer)))){
							FILE_LOG(logERROR) << "Fifo "<< i << ": Frame Number is zero from firmware. popready[" << i << "]:" << popReady[i];
							popReady[i]=true;
							continue;
						}
						//frame number
						threadFrameNumber[i] = (uint32_t)(*( (uint64_t*) packetBuffer_footer));
						//last frame read out
						lastFrameIndex = threadFrameNumber[i];
						threadFrameNumber[i] +=	(startFrameIndex - 1);

						//packet number
						currentPacketNumber[i] = *( (uint16_t*) packetBuffer_footer->packetNumber);
#ifdef DEBUG4
						cprintf(MAGENTA,"Fifo %d: threadframenumber original:%d currentpacketnumber real:%d\n",
								i,threadFrameNumber[i],currentPacketNumber[i]);
#endif
					}

					//calculate number of missing packets-----------------------------------------------------
					numberofMissingPackets[i] = 0;
					if((numPackets[i] == dummyPacketValue) || (threadFrameNumber[i] != presentFrameNumber))
						numberofMissingPackets[i] = (LAST_PACKET_VALUE - lastPacketNumber[i]);
					else
						numberofMissingPackets[i] = (currentPacketNumber[i] - lastPacketNumber[i] - 1);
					numMissingPackets += numberofMissingPackets[i];

#ifdef DEBUG4
					if(numPackets[i] == dummyPacketValue)
						cprintf(GREEN, "Fifo %d: Calc missing packets (Dummy): Adding missing packets %d to the last frame\n",
								i, numberofMissingPackets[i]);
					else{
						cprintf(GREEN,"Fifo %d: Calc missing packets: fnum %d, fnum_thread %d, "
								"pnum %d, last_pnum %d, pnum_offset %d missing_packets %d\n",
								i,presentFrameNumber,threadFrameNumber[i],
								currentPacketNumber[i],lastPacketNumber[i],frameBufferoffset[i],numberofMissingPackets[i]);
					}
#endif


					//add missing packets---------------------------------------------------------------------
					for(int j=0;j<numberofMissingPackets[i];++j){

						blankoffset = frameBufferoffset[i];
						frameBuffer[frameBufferoffset[i]] = blankframe[blankoffset];
						eiger_packet_header_t* frameBuffer_header = (eiger_packet_header_t*) frameBuffer[frameBufferoffset[i]];
						if (*( (uint16_t*) frameBuffer_header->missingPacket)!= missingPacketValue){
							eiger_packet_header_t* blankframe_header = (eiger_packet_header_t*) blankframe[blankoffset];
							cprintf(BG_RED, "Fifo %d: Add Missing Packet Error: "
									"pnum_offset %d, pnum %d, fnum_thread %d, missingpacket_buffer 0x%x, missingpacket_blank 0x%x\n",
									i,frameBufferoffset[i],currentPacketNumber[i],threadFrameNumber[i],
									*( (uint16_t*) frameBuffer_header->missingPacket),
									*( (uint16_t*) blankframe_header->missingPacket));
							exit(-1);
						}else{
#ifdef DEBUG4
							cprintf(RED, "Fifo %d: Add Missing Packet success: "
									"pnum_offset %d, pnum_got %d, fnum_thread %d, missingpacket_buffer 0x%x\n",
									i,frameBufferoffset[i],currentPacketNumber[i],threadFrameNumber[i],
									*( (uint16_t*) frameBuffer_header->missingPacket));
#endif
							frameBufferoffset[i]=frameBufferoffset[i]+1;
						}
					}

					//missed packets/future packet: do not pop over and determine fullframe--------------------
					popReady[i] = false;
					if((numPackets[i] == dummyPacketValue) ||(threadFrameNumber[i] != presentFrameNumber))
						fullframe[i] = true;
					else
						fullframe[i] = false;
					if(threadFrameNumber[i] != presentFrameNumber)
						threadFrameNumber[i] = presentFrameNumber;


					//add current packet--------------------------------------------------------------
					if(fullframe[i] == false){
						if(currentPacketNumber[i] != (uint32_t)(frameBufferoffset[i]-(i*packetsPerFrame/numberofListeningThreads))+1){
							cprintf(BG_RED, "Fifo %d: Correct Packet Offset Error: "
									"pnum_offset %d,pnum %d fnum_thread %d\n",
									i,frameBufferoffset[i],currentPacketNumber[i],threadFrameNumber[i]);
							exit(-1);
						}


						while(!fifoTempFree[i]->push(packetBuffer[i]));
#ifdef EVERYFIFODEBUG
						if(fifoTempFree[i]->getSemValue()>((packetsPerFrame/numberofListeningThreads)-3))
							cprintf(YELLOW,"FifoTempfree[%d]: value:%d, push 0x%x\n",i,fifoTempFree[i]->getSemValue(),(void*)(wbuffer[i]));
#endif



						//cprintf(RED,"Current Packet frameBufferoffset[i]:%d\n",frameBufferoffset[i]);
						frameBuffer[frameBufferoffset[i]] = (packetBuffer[i] + HEADER_SIZE_NUM_TOT_PACKETS);
#ifdef DEBUG4
						eiger_packet_header_t* frameBuffer_header = (eiger_packet_header_t*) frameBuffer[frameBufferoffset[i]];
						eiger_packet_footer_t* frameBuffer_footer = (eiger_packet_footer_t*) (frameBuffer[frameBufferoffset[i]] + footerOffset);
						cprintf(GREEN, "Fifo %d: Current Packet added success:"
								"pnum_offset %d, pnum %d, real pnum %d fnum_thread %d, missingpacket_buffer 0x%x\n",
								i,frameBufferoffset[i],currentPacketNumber[i],*( (uint16_t*) frameBuffer_footer->packetNumber),threadFrameNumber[i],
								*( (uint16_t*) frameBuffer_header->missingPacket));
#endif
						frameBufferoffset[i]=frameBufferoffset[i]+1;
						//update last packet
						lastPacketNumber[i] = currentPacketNumber[i];
						popReady[i] = true;
						fullframe[i] = false;
						if(currentPacketNumber[i] == LAST_PACKET_VALUE){
#ifdef DEBUG4
							cprintf(GREEN, "Fifo %d: Got last packet\n",i);
#endif
							popReady[i] = false;
							fullframe[i] = true;
						} //end of last packet
					}//end of add current packet
				}//end of if(!fullframe)
			}//end of for listening threads


			//full frame
			if(fullframe[0] && fullframe[1]){
				currentFrameNumber = presentFrameNumber;
				numTotMissingPacketsInFile += numMissingPackets;
				numTotMissingPackets += numMissingPackets;

/*
				cprintf(CYAN,"**framenum:%lld\n ",(long long int)currentFrameNumber);
				if(currentFrameNumber>500){
					cprintf(BG_RED,"too high frame number %lld \n",(long long int)currentFrameNumber );
					exit(-1);
				}
				for(int i=0;i<numberofListeningThreads;i++){
					eiger_packet_footer_t* wbuf_footer1 = (eiger_packet_footer_t*)(packetBuffer[i] + footerOffset + HEADER_SIZE_NUM_TOT_PACKETS);
					cprintf(GREEN,"Fifo %d:End of loop popready %d, threadfnum:%d fnum:%d, pnum:%d, add0x%p\n",
							i,popReady[i],threadFrameNumber[i],(uint32_t)(*( (uint64_t*) wbuf_footer1)),
							*( (uint16_t*) wbuf_footer1->packetNumber),	(void*)(packetBuffer[i]));
				}*/
#ifdef DEBUG4
				cprintf(BLUE," nummissingpackets:%d\n",numMissingPackets);
#endif
#ifdef FNUM_DEBUG
				cprintf(GREEN,"**fnum:%lld**\n",(long long int)currentFrameNumber);
#endif
#ifdef MISSINGP_DEBUG
				if(numMissingPackets){
					cprintf(RED, "Total missing packets %d for fnum %d\n",numMissingPackets,currentFrameNumber);
					for (int j=0;j<packetsPerFrame;++j){
						eiger_packet_header_t* frameBuffer_header = (eiger_packet_header_t*) frameBuffer[j];
						if (*( (uint16_t*) frameBuffer_header->missingPacket)==missingPacketValue)
							cprintf(RED,"Found missing packet at pnum %d\n",j);
					}
				}
#endif

				//write and copy to gui
				handleWithoutDataCompression(ithread,frameBuffer,packetsPerFrame);

				//reset a few stuff
				presentFrameNumber++;
				for(int i=0; i<numberofListeningThreads; ++i){
					fullframe[i] = false;

					//ensuring last packet got is not of some other future frame but of the current one
					eiger_packet_footer_t* wbuf_footer1 = (eiger_packet_footer_t*)(packetBuffer[i] + footerOffset + HEADER_SIZE_NUM_TOT_PACKETS);
					uint64_t packfnum = (((uint32_t)(*( (uint64_t*) wbuf_footer1)))+(startFrameIndex - 1));

					//to reset to get new frame: not dummy and the last packet
					if((numPackets[i] != dummyPacketValue) && (currentPacketNumber[i] == LAST_PACKET_VALUE) && (packfnum == currentFrameNumber) )
						popReady[i] = true;
					frameBufferoffset[i] = (i*packetsPerFrame/numberofListeningThreads);
					//blankoffset = 0;
					lastPacketNumber[i] = 0;
					currentPacketNumber[i] = 0;
					numberofMissingPackets[i] = 0;
#ifdef DEBUG4
					cprintf(GREEN,"popready[%d]: %d\n",i,popReady[i]);
#endif
				}


				//freeing
				for(int i=0; i<numberofListeningThreads; ++i){
					while(!fifoTempFree[i]->isEmpty()){
						fifoTempFree[i]->pop(temp);
#ifdef EVERYFIFODEBUG
						if(fifoTempFree[i]->getSemValue()>((packetsPerFrame/numberofListeningThreads)-3))
						cprintf(GRAY,"FifoTempFree[%d]: value:%d, pop 0x%x\n",i,fifoTempFree[i]->getSemValue(),(void*)(temp));
#endif
						while(!fifoFree[i]->push(temp));
#ifdef EVERYFIFODEBUG
						if(fifoFree[i]->getSemValue()<100)
						cprintf(GREEN,"FifoFree[%d]: value:%d, push 0x%x\n",i,fifoFree[i]->getSemValue(),(void*)(temp));
#endif
#ifdef CFIFODEBUG
						if(i==0)
							cprintf(CYAN,"Fifo %d: Writing_Thread freed: pushed into fifofree %p\n",i, (void*)(temp));
						else
							cprintf(YELLOW,"Fifo %d: Writing_Thread freed: pushed into fifofree %p\n",i, (void*)(temp));
#endif
					}
				}
#ifdef DEBUG4
				cprintf(GREEN,"Writing_Thread: finished freeing\n");
#endif


			}//end of full frame

		}/*--end of loop for each buffer (inner loop)*/

		waitWritingBufferForNextAcquisition(ithread);

	}/*--end of loop for each acquisition (outer loop) */
}




void UDPStandardImplementation::waitWritingBufferForNextAcquisition(int ithread){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	//in case they are not closed already
	closeFile();
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
		cprintf(RED,"%d:emptied buffer in fifo\n", ithread);
		fifo[ithread]->pop(temp);
#ifdef EVERYFIFODEBUG
		if(fifo[ithread]->getSemValue()>(fifoSize-100))
		cprintf(CYAN,"Fifo[%d]: value:%d, pop 0x%x\n",ithread,fifo[ithread]->getSemValue(),(void*)(temp));
#endif
	}

	//create file
	if((1<<ithread)&createFileMask){
		if(dataCompressionEnable){
#ifdef MYROOT1
			pthread_mutex_lock(&writeMutex);
			fileCreateSuccess = createCompressionFile(ithread,0);
			pthread_mutex_unlock(&writeMutex);
#endif
		}else
			fileCreateSuccess = createNewFile();

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


bool UDPStandardImplementation::popAndCheckEndofAcquisition(int ithread, char* wbuffer[], bool ready[], uint32_t nP[],CircularFifo<char>* fifoTempFree[]){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	bool endofAcquisition = true;
	for(int i=0; i<numberofListeningThreads; ++i){
		//pop if ready
		if(ready[i]){
			fifo[i]->pop(wbuffer[i]);
#ifdef EVERYFIFODEBUG
			if(fifo[i]->getSemValue()>(fifoSize-100))
			cprintf(CYAN,"Fifo[%d]: value:%d, pop 0x%x\n",i,fifo[i]->getSemValue(),(void*)(wbuffer[i]));
#endif
#ifdef CFIFODEBUG
			if(i == 0)
				cprintf(CYAN,"Writing_Thread %d: Popped %p from FIFO %d\n", ithread, (void*)(wbuffer[i]),i);
			else
				cprintf(YELLOW,"Writing_Thread %d: Popped %p from FIFO %d\n", ithread, (void*)(wbuffer[i]),i);
#endif
			nP[i] = (uint32_t)(*((uint32_t*)wbuffer[i]));
#ifdef DEBUG4
			cprintf(GREEN,"Writing_Thread %d: Number of Packets: %d for FIFO %d\n", ithread, nP[i], i);
#endif
			//dummy-end buffer
			if(nP[i] == dummyPacketValue){
				ready[i] = false;
#ifdef DEBUG3
				cprintf(GREEN,"Writing_Thread %d: Dummy frame popped out of FIFO %d",ithread, i);
#endif
			}
			//normal buffer popped out
			else{
				endofAcquisition = false;
#ifdef DEBUG4
				if(myDetectorType == EIGER){
					eiger_packet_footer_t* wbuf_footer = (eiger_packet_footer_t*)(wbuffer[i] + footerOffset + HEADER_SIZE_NUM_TOT_PACKETS);
					//cprintf(BLUE,"footer value:0x%x\n",i,(uint64_t)(*( (uint64_t*) wbuf_footer)));
					//if(*( (uint16_t*) wbuf_footer->packetNumber) == 1){
					cprintf(BLUE,"Fnum[%d]:%d\n",i,(uint32_t)(*( (uint64_t*) wbuf_footer)));
					cprintf(BLUE,"Pnum[%d]:%d\n",i,*( (uint16_t*) wbuf_footer->packetNumber));
					//}
				}
#endif
			}
		}
		//when both are not popped but curretn frame number is being processed
		else{
			if(nP[i] != dummyPacketValue)
				endofAcquisition = false;
		}
	}

	return endofAcquisition;
}



void UDPStandardImplementation::stopWriting(int ithread, char* wbuffer[]){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	FILE_LOG(logINFO) << "Writing "<< ithread << ": End of Acquisition";

	//free fifo
	for(int i=0; i<numberofListeningThreads; ++i){
		while(!fifoFree[i]->push(wbuffer[i]));
#ifdef EVERYFIFODEBUG
		if(fifoFree[i]->getSemValue()<100)
		cprintf(GREEN,"FifoFree[%d]: value:%d, push 0x%x\n",i,fifoFree[i]->getSemValue(),(void*)(wbuffer[i]));
#endif
#ifdef CFIFODEBUG
		if(i==0)
			cprintf(CYAN,"Writing_Thread %d: Freeing dummy-end buffer. Pushed into fifofree %p for listener %d\n", ithread,(void*)(wbuffer[i]),i);
		else
			cprintf(YELLOW,"Writing_Thread %d: Freeing dummy-end buffer. Pushed into fifofree %p for listener %d\n", ithread,(void*)(wbuffer[i]),i);
#endif
	}

	//all threads need to close file, reset mask and exit loop
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
		if(dataCompressionEnable){
			cprintf(GREEN,"Writing_Thread %d: Waiting for jobs to be done.. current mask:0x%x\n",ithread, writerThreadsMask);
			while(writerThreadsMask){
				/*cout << "." << flush;*/
				usleep(50000);
			}
			cprintf(GREEN,"Writing_Thread %d: Jobs Done!\n",ithread);
		}

		//ensure listening threads done before updating status as it returns to client (from stopReceiver)
		while(listeningThreadsMask)
			usleep(5000);
		//update status
		pthread_mutex_lock(&statusMutex);
		status = RUN_FINISHED;
		pthread_mutex_unlock(&(statusMutex));

		//statistics
		FILE_LOG(logINFO) << "Status: Run Finished";
		FILE_LOG(logINFO) << "Last Frame Number Caught:" << lastFrameIndex;
		if(totalPacketsCaught < ((uint64_t)numberOfFrames*packetsPerFrame)){
			cprintf(RED, "Total Missing Packets padded: %d\n",numTotMissingPackets);
			cprintf(RED, "Total Packets Caught: %lld\n",(long long int)totalPacketsCaught);
			cprintf(RED, "Total Frames Caught: %lld\n",(long long int)(totalPacketsCaught/packetsPerFrame));
		}else{
			cprintf(GREEN, "Total Missing Packets padded: %d\n",numTotMissingPackets);
			cprintf(GREEN, "Total Packets Caught:%lld\n", (long long int)totalPacketsCaught);
			cprintf(GREEN, "Total Frames Caught:%lld\n",(long long int)(totalPacketsCaught/packetsPerFrame));
		}
		//acquisition end
		if (acquisitionFinishedCallBack)
			acquisitionFinishedCallBack((int)(totalPacketsCaught/packetsPerFrame), pAcquisitionFinished);
	}
}




void UDPStandardImplementation::handleWithoutDataCompression(int ithread, char* wbuffer[],uint32_t npackets){
	FILE_LOG(logDEBUG) << __AT__ << " called";


	//get frame number (eiger already gets it when it does packet to packet processing)
	if(myDetectorType != EIGER){
		if(myDetectorType == JUNGFRAU){
			jfrau_packet_header_t* header = (jfrau_packet_header_t*)(wbuffer[0] + HEADER_SIZE_NUM_TOT_PACKETS);
			currentFrameNumber = (*( (uint32_t*) header->frameNumber))&0xffffff;
		}else{
			uint64_t tempframenumber = ((uint32_t)(*((uint32_t*)(wbuffer[0] + HEADER_SIZE_NUM_TOT_PACKETS))));
			//for gotthard and normal frame, increment frame number to separate fnum and pnum
			if (myDetectorType == PROPIX ||(myDetectorType == GOTTHARD && shortFrameEnable == -1))
				tempframenumber++;
			//get frame number
			currentFrameNumber = (tempframenumber & frameIndexMask) >> frameIndexOffset;
		}
		//set indices
		acquisitionIndex = currentFrameNumber - startAcquisitionIndex;
		frameIndex = currentFrameNumber - startFrameIndex;
	}




	//callback to write data
	if (cbAction < DO_EVERYTHING){
		switch(myDetectorType){
		case EIGER:
			for(uint32_t i=0;i<npackets;++i)
				rawDataReadyCallBack((int)currentFrameNumber, wbuffer[i], onePacketSize, sfilefd, guiData, pRawDataReady);
			break;
		default:
			rawDataReadyCallBack((int)currentFrameNumber, wbuffer[0] + HEADER_SIZE_NUM_TOT_PACKETS, npackets * onePacketSize,
					sfilefd, guiData,pRawDataReady);
			break;
		}
	}

	//write to file if enabled and update write parameters
	if(npackets > 0)
		writeFileWithoutCompression(wbuffer, npackets);
#ifdef DEBUG4
	cprintf(GREEN,"Writing_Thread: Writing done\nGoing to copy frame\n");
#endif


	//copy frame for gui
	if(npackets >= packetsPerFrame)
		copyFrameToGui(wbuffer);
#ifdef DEBUG4
	cprintf(GREEN,"Writing_Thread: Copied frame\n");
#endif


	//free fifo addresses (eiger frees for each packet later)
	if(myDetectorType != EIGER){
		while(!fifoFree[0]->push(wbuffer[0]));
#ifdef EVERYFIFODEBUG
		if(fifoFree[0]->getSemValue()<100)
		cprintf(GREEN,"FifoFree[%d]: value:%d, push 0x%x\n",0,fifoFree[0]->getSemValue(),(void*)(wbuffer[0]));
#endif
#ifdef DEBUG5
			cprintf(GREEN,"Writing_Thread %d: Freed buffer, pushed into fifofree %p for listener 0\n",ithread, (void*)(wbuffer[0]));
#endif
	}
}




void UDPStandardImplementation::writeFileWithoutCompression(char* wbuffer[],uint32_t numpackets){
	FILE_LOG(logDEBUG) << __AT__ << " called";


	//create headers for eiger
#ifdef WRITE_HEADERS
	if (myDetectorType == EIGER && cbAction == DO_EVERYTHING)
		createHeaders(wbuffer);
#endif

	//if write enabled
	if((fileWriteEnable) && (sfilefd)){
		int offset = HEADER_SIZE_NUM_TOT_PACKETS; 		//offset (not eiger) to keep track of how many packets saved
		uint32_t packetsToSave;								//how many packets to save at a time
		volatile uint64_t tempframenumber;
		int lastpacket;

		//loop to take care of creating new files when it reaches max packets per file
		while(numpackets > 0){

			//new file
			if(packetsInFile >= (uint32_t)maxPacketsPerFile){
				//for packet loss, because currframenum is the latest one for eiger
				//get frame number (eiger already gets it when it does packet to packet processing)
				if(myDetectorType != EIGER){
					lastpacket = (((packetsToSave - 1) * onePacketSize) + offset);
					if(myDetectorType == JUNGFRAU){
						jfrau_packet_header_t* header = (jfrau_packet_header_t*) (wbuffer[0] + lastpacket);
						currentFrameNumber = (*( (uint32_t*) header->frameNumber))&0xffffff;
					}else{
						tempframenumber = ((uint32_t)(*((uint32_t*)(wbuffer[0] + lastpacket))));
						//for gotthard and normal frame, increment frame number to separate fnum and pnum
						if (myDetectorType == PROPIX ||(myDetectorType == GOTTHARD && shortFrameEnable == -1))
							tempframenumber++;
						//get frame number
						currentFrameNumber = (tempframenumber & frameIndexMask) >> frameIndexOffset;
					}

					//set indices
					acquisitionIndex = currentFrameNumber - startAcquisitionIndex;
					frameIndex = currentFrameNumber - startFrameIndex;
				}
#ifdef DEBUG3
				cprintf(GREEN,"Writing_Thread: Current Frame Number:%d\n",currentFrameNumber);
#endif
				createNewFile();
			}
			//to create new file when max reached
			packetsToSave = maxPacketsPerFile - packetsInFile;
			if(packetsToSave > numpackets)
				packetsToSave = numpackets;

			//write to file
			if(cbAction == DO_EVERYTHING){
				switch(myDetectorType){
				case EIGER:
					for(uint32_t i=0; i<packetsToSave; ++i)
						fwrite((void*)wbuffer[i], 1, onePacketSize, sfilefd);
					break;
				default:
					fwrite(wbuffer[0] + offset, 1, packetsToSave * onePacketSize, sfilefd);
					break;
				}
			}

			//update parameters
			packetsInFile += packetsToSave;
#ifdef DEBUG4
			cprintf(GREEN,"Writing Thread: packetsCaught till now:%d packetsToSave:%d numMissingPackets:%d packetsCaught now:%d\n",
								packetsCaught,packetsToSave,numMissingPackets,(packetsToSave - numMissingPackets));
#endif
			packetsCaught += (packetsToSave - numMissingPackets);
			totalPacketsCaught += (packetsToSave - numMissingPackets);
			numMissingPackets = 0;
#ifdef DEBUG4
			cprintf(GREEN,"Writing Thread: packetscaught:%d totalPacketsCaught:%d\n", packetsCaught,totalPacketsCaught);
#endif

			//increase offset
			if(myDetectorType != EIGER)
				offset += (packetsToSave * onePacketSize);
			numpackets -= packetsToSave;
		}
	}

	//only update parameters
	else{
		if(numberofWriterThreads > 1) pthread_mutex_lock(&writeMutex);
		packetsInFile += numpackets;
		packetsCaught += (numpackets - numMissingPackets);
		totalPacketsCaught += (numpackets - numMissingPackets);
		numMissingPackets = 0;
		if(numberofWriterThreads > 1) pthread_mutex_unlock(&writeMutex);
	}

}





void UDPStandardImplementation::createHeaders(char* wbuffer[]){


	int port = 0, missingPacket;
	bool exitVal = 0;
	eiger_packet_header_t* wbuf_header;
	eiger_packet_footer_t* wbuf_footer;

	for (uint32_t i = 0; i < packetsPerFrame; i++){


		wbuf_header = (eiger_packet_header_t*) wbuffer[i];
		wbuf_footer = (eiger_packet_footer_t*)(wbuffer[i] + footerOffset);
#ifdef DEBUG4
		cprintf(GREEN, "Loop index:%d Pnum:%d real fnum %d,missingPacket 0x%x\n",
				i,
				*( (uint16_t*) wbuf_footer->packetNumber),
				(uint32_t)(*( (uint64_t*) wbuf_footer)),
				*( (uint16_t*) wbuf_header->missingPacket)
				); cout <<flush;
#endif
		//which port
		if (i ==(packetsPerFrame/2))	port = 1;

		//missing packet
		if (*( (uint16_t*) wbuf_header->missingPacket)== missingPacketValue){
#ifdef DEBUG4
			cprintf(RED,"-Missing packet at Loop Index %d\n", i);
#endif
			missingPacket = 1;

			//DEBUGGING
			if(*( (uint16_t*) wbuf_footer->packetNumber) != (i+1)){
				cprintf(BG_RED, "Writing_Thread: Packet Number Mismatch (missing p)! "
						"i %d, real pnum %d, real fnum %d, missingPacket 0x%x\n",
						i,
						*( (uint16_t*) wbuf_footer->packetNumber),
						(uint32_t)(*( (uint64_t*) wbuf_footer)),
						*( (uint16_t*) wbuf_header->missingPacket));
				exitVal =1;
			}

			//add frame number
			*( (uint64_t*) wbuf_footer) = (currentFrameNumber+1) | (((uint64_t)(*( (uint16_t*) wbuf_footer->packetNumber)))<<0x30);
			//*( (uint16_t*) wbuf_footer->packetNumber) = (i+1); // missing frames already have the right packet number
#ifdef DEBUG4
			cprintf(RED, "Missing Packet Loop index:%d fnum:%d Pnum:%d\n",i,
					(uint32_t)(*( (uint64_t*) wbuf_footer)),
					*( (uint16_t*) wbuf_footer->packetNumber));
#endif
		}
		//normal packet
		else{
			missingPacket = 0;

			//DEBUGGING
			if(*( (uint16_t*) wbuf_footer->packetNumber) != ( (i>((packetsPerFrame/2)-1)?(i-(packetsPerFrame/2)+1):i+1)  )){
				cprintf(BG_RED, "Writing_Thread: Packet Number Mismatch! "
						"i %d, real pnum %d, real fnum %d, missingPacket 0x%x\n",
						i,
						*( (uint16_t*) wbuf_footer->packetNumber),
						(uint32_t)(*( (uint64_t*) wbuf_footer)),
						*( (uint16_t*) wbuf_header->missingPacket));
				exitVal =1;
			}

			uint16_t p = *( (uint16_t*) wbuf_footer->packetNumber);
			//correct the packet numbers of port2 so that port1 and 2 are not the same
			if(port)  *( (uint16_t*) wbuf_footer->packetNumber) = (p +(packetsPerFrame/2));

		}

		//overwriting port number and dynamic range
		*( (uint8_t*) wbuf_header->portIndex) = (uint8_t)port;
		//*( (uint8_t*) wbuf_header->dynamicRange) = (uint8_t)dynamicRange;

		//DEBUGGING
		if(*( (uint16_t*) wbuf_footer->packetNumber) != (i+1)){
			cprintf(BG_RED, "Writing_Thread: Packet Number Mismatch! "
					"i %d, real pnum %d, real fnum %d, missingPacket 0x%x\n",
					i,
					*( (uint16_t*) wbuf_footer->packetNumber),
					(uint32_t)(*( (uint64_t*) wbuf_footer)),
					*( (uint16_t*) wbuf_header->missingPacket));
			exitVal =1;
		}
	}

	if(exitVal){exit(-1);}

}


void UDPStandardImplementation::updateFileHeader(){
	int xpix=-1,ypix=-1;

	//create detector specific packet header
	char packetheader[1000];
	strcpy(packetheader,"");

	//only for eiger right now
	/*switch(myDetectorType){
	case EIGER:
	*/	sprintf(packetheader,"#Packet Header\n"
				"Sub Frame Number 4 bytes\n"
				"Missing Packet\t 2 bytes\n"
				"Port Number\t 1 byte\n"
				"Unused\t\t 1 byte\n\n"
				"#Packet Footer\n"
				"Frame Number\t 6 bytes\n"
				"Packet Number\t 2 bytes\n");
		xpix = EIGER_PIXELS_IN_ONE_ROW;
		ypix = EIGER_PIXELS_IN_ONE_COL;
	/*	break;
	default:
		break;
	}
*/

	//update file header
	time_t t = time(0);
	int length = sizeof(fileHeader);
	while(length!=strlen(fileHeader)){
		length = strlen(fileHeader);
		sprintf(fileHeader,"\nHeader\t\t %d bytes\n"
				"Dynamic Range\t %d\n"
				"Packet\t\t %d bytes\n"
				"x\t\t %d pixels\n"
				"y\t\t %d pixels\n"
				"Timestamp\t %s\n\n"
				"%s",
				length,dynamicRange,onePacketSize,xpix,ypix,ctime(&t),
				packetheader);
	}

}


void UDPStandardImplementation::copyFrameToGui(char* buffer[]){
	FILE_LOG(logDEBUG) << __AT__ << " called";


	//random read (gui not ready)
	//need to toggle guiDataReady or the second frame wont be copied
	if((!FrameToGuiFrequency) && (!guiData)){
#ifdef DEBUG4
		cprintf(GREEN,"Writing_Thread: CopyingFrame: Resetting guiDataReady\n");
#endif
		pthread_mutex_lock(&dataReadyMutex);
		guiDataReady=0;
		pthread_mutex_unlock(&dataReadyMutex);
	}

	//random read (gui ready) or nth frame read: gui needs data now or it is the first frame
	else{
#ifdef DEBUG4
		cprintf(GREEN,"Writing_Thread: CopyingFrame: Gui needs data now OR 1st frame\n");
#endif
		pthread_mutex_lock(&dataReadyMutex);
		guiDataReady=0;
#ifdef DEBUG4
		cprintf(GREEN,"Writing_Thread: CopyingFrame: guidataready is  0, Copying data\n");
#endif
		switch(myDetectorType){
		case EIGER:
			for(uint32_t i=0; i<packetsPerFrame; ++i)
				memcpy((((char*)latestData)+i * onePacketSize) ,buffer[i],onePacketSize);
			break;
		default:
			memcpy(latestData,buffer[0] + HEADER_SIZE_NUM_TOT_PACKETS,bufferSize);
			break;
		}

		strcpy(guiFileName,completeFileName);
		guiDataReady=1;
		pthread_mutex_unlock(&dataReadyMutex);
#ifdef DEBUG4
		cprintf(GREEN,"Writing_Thread: CopyingFrame: Copied Data, guidataready is 1\n");
#endif

		//nth frame read, block current process if the guireader hasnt read it yet
		if(FrameToGuiFrequency){
#ifdef DEBUG4
			cprintf(GREEN,"Writing_Thread: CopyingFrame: Waiting after copying\n");
#endif
			sem_wait(&writerGuiSemaphore);
#ifdef DEBUG4
			cprintf(GREEN,"Writing_Thread: CopyingFrame: Done waiting\n");
#endif
		}

	}
}





void UDPStandardImplementation::handleDataCompression(int ithread, char* wbuffer[], uint64_t &nf){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	//frame number
	uint64_t tempframenumber = ((uint32_t)(*((uint32_t*)(wbuffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS))));
	//for gotthard and normal frame, increment frame number to separate fnum and pnum
	if (myDetectorType == PROPIX ||(myDetectorType == GOTTHARD && shortFrameEnable == -1))
		tempframenumber++;
	//get frame number
	tempframenumber = (tempframenumber & frameIndexMask) >> frameIndexOffset;
	//handle multi threads
	pthread_mutex_lock(&progressMutex);
	if(tempframenumber > currentFrameNumber)
		currentFrameNumber = tempframenumber;
	pthread_mutex_unlock(&progressMutex);
	//set indices
	acquisitionIndex = currentFrameNumber - startAcquisitionIndex;
	frameIndex = currentFrameNumber - startFrameIndex;


	//variable definitions
	char* buff[2]={0,0};										//an array just to be compatible with copyframetogui
	char* data = wbuffer[0]+ HEADER_SIZE_NUM_TOT_PACKETS;	//data pointer to the next memory to be analysed
	int ndata;												//size of data returned
	uint32_t np;													//remaining number of packets returned
	uint32_t npackets = (uint32_t)(*((uint32_t*)wbuffer[0]));	//number of total packets
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
							if((fileWriteEnable) && (sfilefd))
								singlePhotonDetectorObject[ithread]->writeCluster(sfilefd);
							pthread_mutex_unlock(&writeMutex);
#endif
						}
					}
				}

			nf++;
#ifndef ALLFILE
			pthread_mutex_lock(&progressMutex);
			packetsInFile += packetsPerFrame;
			packetsCaught += packetsPerFrame;
			totalPacketsCaught += packetsPerFrame;
			if(packetsInFile >= (uint32_t)maxPacketsPerFile)
				createNewFile();
			pthread_mutex_unlock(&progressMutex);

#endif
			if(!once){
				copyFrameToGui(buff);
				once = 1;
			}
		}

		remainingsize -= ((buff[0] + ndata) - data);
		data = buff[0] + ndata;
		if(data > (wbuffer[0] + HEADER_SIZE_NUM_TOT_PACKETS + npackets * onePacketSize) )
			cprintf(BG_RED,"Writing_Thread %d: Error: Compression data goes out of bounds!\n", ithread);
	}


	while(!fifoFree[0]->push(wbuffer[0]));
#ifdef EVERYFIFODEBUG
	if(fifoFree[0]->getSemValue()<100)
	cprintf(GREEN,"FifoFree[%d]: value:%d, push 0x%x\n",0,fifoFree[0]->getSemValue(),(void*)(wbuffer[0]));
#endif
#ifdef DEBUG5
	cprintf(GREEN,"Writing_Thread %d: Compression free pushed into fifofree %p for listerner 0\n", ithread, (void*)(wbuffer[0]));
#endif
}


