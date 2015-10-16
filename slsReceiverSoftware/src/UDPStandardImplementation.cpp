/********************************************//**
 * @file UDPStandardImplementation.cpp
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 ***********************************************/


#include "UDPStandardImplementation.h"

#include "moench02ModuleData.h"
#include "gotthardModuleData.h"
#include "gotthardShortModuleData.h"

//#include <sys/socket.h>		// socket(), bind(), listen(), accept(), shut down
//#include <arpa/inet.h>		// sock_addr_in, htonl, INADDR_ANY
#include <stdlib.h>			// exit()
#include <iomanip>			//set precision for printing parameters for create new file
#include <map>				//map
//#include <sys/mman.h>		//munmap

#include <iostream>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
using namespace std;

#define WRITE_HEADERS

/*************************************************************************
 * Constructor & Destructor **********************************************
 * They access local cache of configuration or detector parameters *******
 *************************************************************************/

UDPStandardImplementation::UDPStandardImplementation(){
	FILE_LOG(logDEBUG1) << __AT__ << " called";

	initializeMembers();

	//***mutex***
	pthread_mutex_init(&statusMutex,NULL);
	pthread_mutex_init(&writeMutex,NULL);
	pthread_mutex_init(&dataReadyMutex,NULL);
	pthread_mutex_init(&progressMutex,NULL);

	//to increase socket receiver buffer size and max length of input queue by changing kernel settings
	if(system("echo $((100*1024*1024)) > /proc/sys/net/core/rmem_max"))
	  cout << "Warning: No root permission to change socket receiver buffer size in file /proc/sys/net/core/rmem_max" << endl;
	else if(system("echo 250000 > /proc/sys/net/core/netdev_max_backlog"))
	  cout << "Warning: No root permission to change max length of input queue in file /proc/sys/net/core/netdev_max_backlog" << endl;
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
	FILE_LOG(logDEBUG1) << __AT__ << " called";

	deleteMembers();
}


/*************************************************************************
 * Setters ***************************************************************
 * They modify the local cache of configuration or detector parameters ***
 *************************************************************************/

/***initial parameters***/
void UDPStandardImplementation::deleteBaseMembers(){
	FILE_LOG(logDEBUG1) << __AT__ << " starting";

	UDPBaseImplementation::~UDPBaseImplementation();
}

void UDPStandardImplementation::deleteMembers(){
	FILE_LOG(logDEBUG1) << __AT__ << " starting";

	cout << "Info: Deleting member pointers" << endl;
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
	FILE_LOG(logDEBUG1) << __AT__ << " starting";

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
	FILE_LOG(logDEBUG1) << __AT__ << " starting";

	UDPBaseImplementation::UDPBaseImplementation();
	acquisitionPeriod = SAMPLE_TIME_IN_NS;
}


void UDPStandardImplementation::initializeMembers(){
	FILE_LOG(logDEBUG1) << __AT__ << " starting";

	cout << "Info: Initializing members" << endl;

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

	//***acquisition indices parameters***
	startAcquisitionIndex = 0;
	startFrameIndex = 0;
	frameIndex = 0;
	currentFrameNumber = 0;
	previousFrameNumber = -1;
	acqStarted = false;
	measurementStarted = false;
	for(int i = 0; i < numberofListeningThreads; ++i)
		totalListeningFrameCount[i] = 0;
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
	FILE_LOG(logDEBUG1) << __AT__ << " starting";

	double hc = 0, sigma = 5;
	int sign = 1, csize, i;

	//common mode initialization
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
		singlePhotonDetectorObject[i]=new singlePhotonDetector<uint16_t>(receiverData[i], csize, sigma, sign, commonModeSubtractionEnable);
}




int UDPStandardImplementation::setupFifoStructure(){
	FILE_LOG(logDEBUG1) << __AT__ << " called";

	int64_t i;
	int oldNumberofJobsPerBuffer = numberofJobsPerBuffer;
	int oldFifoSize = fifoSize;

	//eiger always listens to 1 packet at a time
	if(myDetectorType == EIGER){
		numberofJobsPerBuffer = 1;
		cout << "Info: 1 packet per buffer" << endl;
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
		cout << "Info: Number of Frames per buffer:" << numberofJobsPerBuffer << endl;
	}

	//set fifo depth
	//eiger listens to 1 packet at a time and size changes depending on packets per frame
	if(myDetectorType == EIGER)
		fifoSize = EIGER_FIFO_SIZE * packetsPerFrame;
	else{
		fifoSize = GOTTHARD_FIFO_SIZE;
		if(myDetectorType == MOENCH)
			fifoSize = MOENCH_FIFO_SIZE;
		else if(myDetectorType == PROPIX)
			fifoSize = PROPIX_FIFO_SIZE;
		//reduce fifo depth if more frames listened to at a time
		if(fifoSize % numberofJobsPerBuffer)
			fifoSize = (fifoSize/numberofJobsPerBuffer)+1;
		else
			fifoSize = fifoSize/numberofJobsPerBuffer;
	}
#ifdef VERBOSE
	cout << "Info: Fifo Depth:" << fifoSize << endl;
#endif


	//do not rebuild fifo structure if it is the same
	if((oldNumberofJobsPerBuffer == numberofJobsPerBuffer) && (oldFifoSize == fifoSize))
		return OK;


	//set up fifo structure
	for(int i=0;i<numberofListeningThreads;i++){

		//deleting
		if(fifoFree[i]){
			while(!fifoFree[i]->isEmpty())
				fifoFree[i]->pop(buffer[i]);
#ifdef FIFO_DEBUG
			cprintf(GREEN,"%d fifostructure popped from fifofree %p\n", i, (void*)(buffer[i]));
#endif
			delete fifoFree[i];
		}
		if(fifo[i])		delete fifo[i];
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
			fifoFree[i]->push(buffer[i]);
#ifdef FIFO_DEBUG
			cprintf(BLUE,"%d fifostructure free pushed into fifofree %p\n", i, (void*)(buffer[i]));
#endif
			buffer[i] += (bufferSize * numberofJobsPerBuffer + HEADER_SIZE_NUM_TOT_PACKETS);
		}
	}
	cout << "Info: Fifo structure(s) reconstructed" << endl;
}







void UDPStandardImplementation::configure(map<string, string> config_map){
	FILE_LOG(logDEBUG1) << __AT__ << " starting";

	map<string, string>::const_iterator pos;
	pos = config_map.find("mode");
	if (pos != config_map.end() ){
		int b;
		if(!sscanf(pos->second.c_str(), "%d", &b)){
			cout << "Warning: Could not parse mode. Assuming top mode." << endl;
			b = 0;
		}
		bottomEnable = b!= 0;
		cout << "Info: Bottom Enable: " << stringEnable(bottomEnable) << endl;
	}

}


/***file parameters***/
int UDPStandardImplementation::setDataCompressionEnable(const bool b){
	FILE_LOG(logDEBUG1) << __AT__ << " starting";

	cout << "Info: Setting up Data Compression Enable to " << stringEnable(b);
#ifdef MYROOT1
	cout << " WITH ROOT" << endl;
#else
	cout << " WITHOUT ROOT" << endl;
#endif

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

	cout << "Info: Data Compression " << stringEnable(dataCompressionEnable) << endl;

	return OK;
}


/***acquisition parameters***/
void UDPStandardImplementation::setShortFrameEnable(const int i){
	FILE_LOG(logDEBUG1) << __AT__ << " called";

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

	cout << "Info: Short Frame Enable set to " << shortFrameEnable << endl;
}


int UDPStandardImplementation::setFrameToGuiFrequency(const uint32_t i){
	FILE_LOG(logDEBUG1) << __AT__ << " called";

	if(i >= 0){
		FrameToGuiFrequency = i;
		if(setupFifoStructure() == FAIL)
			return FAIL;
	}

	cout << "Info: Frame to Gui Frequency set to " << FrameToGuiFrequency << endl;

	return OK;
}


int UDPStandardImplementation::setAcquisitionPeriod(int64_t i){
	FILE_LOG(logDEBUG1) << __AT__ << " called";

	if(i >= 0){
		acquisitionPeriod = i;
		if(setupFifoStructure() == FAIL)
			return FAIL;
	}

	cout << "Info: Acquisition Period set to " << acquisitionPeriod << endl;


	return OK;
}

int UDPStandardImplementation::setDynamicRange(const uint32_t i){
	FILE_LOG(logDEBUG1) << __AT__ << " called";

	int oldDynamicRange = dynamicRange;

	cout << "Info: Setting Dynamic Range to " << i << endl;
	dynamicRange = i;

	if(myDetectorType == EIGER){

		//set parameters depending on new dynamic range.
		packetsPerFrame 	= (tengigaEnable ? EIGER_TEN_GIGA_CONSTANT : EIGER_ONE_GIGA_CONSTANT)
																* dynamicRange * EIGER_MAX_PORTS;
		frameSize			= onePacketSize * packetsPerFrame;
		maxPacketsPerFile 	= EIGER_MAX_FRAMES_PER_FILE * packetsPerFrame;

		//new dynamic range, then restart threads and resetup fifo structure
		if(oldDynamicRange != dynamicRange){

			//delete threads
			if(threadStarted){
				createListeningThreads(true);
				createWriterThreads(true);
			}

			//gui buffer
			if(latestData){delete[] latestData; latestData = NULL;}
			latestData = new char[frameSize];

			//restructure fifo
			if(setupFifoStructure() == FAIL)
				return FAIL;

			//create threads
			if(createListeningThreads() == FAIL){
				cprintf(BG_RED,"Error: Could not create listening thread\n");
				return FAIL;
			}
			if(createWriterThreads() == FAIL){
				cprintf(BG_RED,"Error: Could not create writer threads\n");
				return FAIL;
			}
			setThreadPriorities();
		}

	}

	cout << "Info: Dynamic Range set to " << dynamicRange << endl;

	return OK;
}



int UDPStandardImplementation::setTenGigaEnable(const bool b){
	FILE_LOG(logDEBUG1) << __AT__ << " called";

	cout << "Info: Setting Ten Giga to " << string(b) << endl;
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

		FILE_LOG(logDEBUG1) << dec <<
				"packetsPerFrame:" << packetsPerFrame <<
				"\nonePacketSize:" << onePacketSize <<
				"\noneDataSize:" << oneDataSize <<
				"\nframesize:" << frameSize <<
				"\nbufferSize:" << bufferSize <<
				"\nmaxPacketsPerFile:" << maxPacketsPerFile << endl;



		//new enable, then restart threads and resetup fifo structure
		if(oldTenGigaEnable != tengigaEnable){

			//delete threads
			if(threadStarted){
				createListeningThreads(true);
				createWriterThreads(true);
			}

			//gui buffer
			if(latestData){delete[] latestData; latestData = NULL;}
			latestData = new char[frameSize];

			//restructure fifo
			if(setupFifoStructure() == FAIL)
				return FAIL;

			//create threads
			if(createListeningThreads() == FAIL){
				cprintf(BG_RED,"Error: Could not create listening thread\n");
				return FAIL;
			}
			if(createWriterThreads() == FAIL){
				cprintf(BG_RED,"Error: Could not create writer threads\n");
				return FAIL;
			}
			setThreadPriorities();
		}

	}

	cout << "Info: Ten Giga " << string(tengigaEnable) << endl;

	return OK;
}







/*************************************************************************
 * Behavioral functions***************************************************
 * They may modify the status of the receiver ****************************
 *************************************************************************/


/***initial functions***/
int UDPStandardImplementation::setDetectorType(const slsReceiverDefs::detectorType d){
	FILE_LOG(logDEBUG1) << __AT__ << " called";

	cout << "Setting receiver type ..." << endl;

	deleteBaseMembers();
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
		cout << "Info: ***** This is a " << slsDetectorBase::getDetectorType(d) << " Receiver *****" << endl;
		break;
	default:
		cprintf(BG_RED, "Error: This is an unknown receiver type %d\n", (int)d);
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
		footerOffset		= EIGER_PACKET_HEADER_SIZE + oneDataSize;
		break;
	case JUNGFRAUCTB:
	case JUNGFRAU:
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
		//footerOffset		= Not applicable;
		break;
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

	//create threads
	if(createListeningThreads() == FAIL){
		cprintf(BG_RED,"Error: Could not create listening thread\n");
		return FAIL;
	}
	if(createWriterThreads() == FAIL){
		cprintf(BG_RED,"Error: Could not create writer threads\n");
		return FAIL;
	}
	setThreadPriorities();

	//allocate for latest data (frame copy for gui)
	latestData = new char[frameSize];

	cout << " Detector type set to " << slsDetectorBase::getDetectorType(d) << endl;
	cout << "Ready..." << endl;

	return OK;
}


/***acquisition functions***/
void UDPStandardImplementation::resetAcquisitionCount(){
	FILE_LOG(logDEBUG1) << __AT__ << " starting";

	totalPacketsCaught = 0;
	acqStarted = false;
	startAcquisitionIndex = 0;

	cout << "Info: Acquisition Count has been reset" << endl;
}


int UDPStandardImplementation::startReceiver(char *c){
	FILE_LOG(logDEBUG1) << __AT__ << " called";

	cout << "Info: Starting Receiver" << endl;


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
	cout << "Info: ***Receiver Configuration***" << endl;
	cout << "Info: Max Packets Per File:" << maxPacketsPerFile << endl;
	cout << "Info: Data Compression has been " << stringEnable(dataCompressionEnable) << endl;
	if(myDetectorType != EIGER)
		cout << "Info: Number of Jobs Per Buffer: " << numberofJobsPerBuffer << endl;
	if(FrameToGuiFrequency)
		cout << "Info: Frequency of frames sent to gui" << FrameToGuiFrequency << endl;
	else
		cout << "Info: Random frames sent to gui" << endl;



	//create UDP sockets
	if(createUDPSockets() == FAIL){
		strcpy(c,"Could not create UDP Socket(s).\n");
		cout << endl;
		cprintf(BG_RED, "Error: %s\n",c);
		return FAIL;
	}

	if(setupWriter() == FAIL){
		//stop udp socket
		shutDownUDPSockets();
		sprintf(c,"Could not create file %s.\n",completeFileName);
		cout << endl;
		cprintf(BG_RED, "Error: %s\n",c);
		return FAIL;
	}


	//For compression, just for gui purposes
	if(dataCompressionEnable)
		sprintf(completeFileName, "%s/%s_fxxx_%d_xx.root", filePath,fileName,fileIndex);

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

	//usleep(5000000);
	cout << "Info: Receiver Started." << endl;
	cout << "Info: Status:" << slsDetectorBase::runStatusType(status) << endl;

	return OK;
}


/**
 * Pre: status is running, semaphores have been instantiated,
 * Post: udp sockets shut down, status is idle, semaphores destroyed
 * */
void UDPStandardImplementation::stopReceiver(){
	FILE_LOG(logDEBUG1) << __AT__ << " called";

	cout << "Info: Stopping Receiver" << endl;

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

	cout << "Info: Receiver Stopped" << endl;
	cout << "Info: Status:" << slsDetectorBase::runStatusType(status) << endl;
	cout << endl;
}





int UDPStandardImplementation::shutDownUDPSockets(){
	FILE_LOG(logDEBUG1) << __AT__ << " called";

	cout << "Info: Shutting down UDP Socket(s)" << endl;

	for(int i=0;i<numberofListeningThreads;i++){
		if(udpSocket[i]){
			udpSocket[i]->ShutDownSocket();
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
	FILE_LOG(logDEBUG1) << __AT__ << " called";

	cout << "Info: Transmitting last data" << endl;

	if(status == RUNNING){
		//wait for all packets
		uint64_t prev = totalPacketsCaught;
		usleep(50000);
		while(prev!=totalPacketsCaught){
			prev=totalPacketsCaught;
			usleep(50000);
		}

		//set status
		pthread_mutex_lock(&statusMutex);
		status = TRANSMITTING;
		pthread_mutex_unlock(&statusMutex);
		cout << "Info: Status: Transmitting" << endl;
	}

	//shut down udp sockets and make listeners push dummy (end) packets for writers
	shutDownUDPSockets();
}




void UDPStandardImplementation::readFrame(char* c,char** raw, uint64_t &startAcq, uint64_t &startFrame){
	FILE_LOG(logDEBUG1) << __AT__ << " called";

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
	FILE_LOG(logDEBUG1) << __AT__ << " called for " << i ;

	//normal
	if(!dataCompressionEnable){
		if(sfilefd){
#ifdef DEBUG4
			cprintf(YELLOW, "Going to close file:%d\n",fileno(sfilefd));
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
			cout << "sfield:" << (int)sfilefd << endl;
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
				cout << "Info: Thread " << i <<": wrote frames to file" << endl;
			else
				cout << "Info: Thread " << i << ": could not write frames to file" << endl;

		}else
			cout << "Info: Thread " << i << ": could not write frames to file: No file or No Tree" << endl;
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
	FILE_LOG(logDEBUG1) << __AT__ << " starting";

	//reset masks
	killAllListeningThreads = false;
	pthread_mutex_lock(&statusMutex);
	listeningThreadsMask = 0x0;
	pthread_mutex_unlock(&(statusMutex));

	//destroy
	if(destroy){
		cout << "Info: Destroying Listening Thread(s)" << endl;

		killAllListeningThreads = true;
		for(int i = 0; i < numberofListeningThreads; ++i){
			sem_post(&listenSemaphore[i]);
			pthread_join(listeningThreads[i],NULL);
			cout <<"."<<flush;
		}
		killAllListeningThreads = false;
		threadStarted = false;
		cout << endl;
		cout << "Info: Listening thread(s) destroyed" << endl;
	}

	//create
	else{
		cout << "Info: Creating Listening Thread(s)" << endl;

		//reset current index
		currentThreadIndex = -1;

		for(int i = 0; i < numberofListeningThreads; ++i){
			sem_init(&listenSemaphore[i],1,0);
			threadStarted = false;
			currentThreadIndex = i;
			if(pthread_create(&listeningThreads[i], NULL,startListeningThread, (void*) this)){
				cout << "Warning: Could not create listening thread with index " << i << endl;
				return FAIL;
			}
			while(!threadStarted);
			cout << ".";
			cout << flush;
		}
		cout << endl;
#ifdef VERBOSE
		cout << "Info: Listening thread(s) created successfully." << endl;
#endif

	}

	return OK;
}



int UDPStandardImplementation::createWriterThreads(bool destroy){
	FILE_LOG(logDEBUG1) << __AT__ << " starting";

	//reset masks
	killAllWritingThreads = false;
	pthread_mutex_lock(&statusMutex);
	writerThreadsMask = 0x0;
	createFileMask = 0x0;
	pthread_mutex_unlock(&(statusMutex));

	//destroy threads
	if(destroy){
		cout << "Info: Destroying Writer Thread(s)" << endl;

		killAllWritingThreads = true;
		for(int i = 0; i < numberofWriterThreads; ++i){
			sem_post(&writerSemaphore[i]);
			pthread_join(writingThreads[i],NULL);
			cout <<"."<<flush;
		}
		killAllWritingThreads = false;
		threadStarted = false;
		cout << endl;
		cout << "Info: Writer thread(s) destroyed" << endl;
	}

	//create threads
	else{
		cout << "Info: Creating Writer Thread(s)" << endl;

		//reset current index
		currentThreadIndex = -1;

		for(int i = 0; i < numberofWriterThreads; ++i){
			sem_init(&writerSemaphore[i],1,0);
			threadStarted = false;
			currentThreadIndex = i;
			if(pthread_create(&writingThreads[i], NULL,startWritingThread, (void*) this)){
				cout << "Warning: Could not create writer thread with index " << i << endl;
				return FAIL;
			}
			while(!threadStarted);
			cout << ".";
			cout << flush;
		}
		cout << endl;
#ifdef VERBOSE
		cout << "Info: Writer thread(s) created successfully." << endl;
#endif
	}

	return OK;
}



void UDPStandardImplementation::setThreadPriorities(){
	FILE_LOG(logDEBUG1) << __AT__ << " called";

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

	if(!rights)
		cout << "Warning: No root permission to prioritize threads." << endl;

}




int UDPStandardImplementation::createUDPSockets(){
	FILE_LOG(logDEBUG1) << __AT__ << " called";

	//switching ports if bottom enabled
	int port[2];
	port =  udpPortNum;
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
		cout << "Warning: eth is empty. Listening to all"<<endl;

		for(int i=0;i<numberofListeningThreads;i++)
			udpSocket[i] = new genericSocket(port[i],genericSocket::UDP,bufferSize);
	}
	//normal socket
	else{
		cout << "Info: eth:" << eth << endl;

		for(int i=0;i<numberofListeningThreads;i++)
			udpSocket[i] = new genericSocket(port[i],genericSocket::UDP,bufferSize,eth);
	}

	//error
	for(int i=0;i<numberofListeningThreads;i++){
		int iret = udpSocket[i]->getErrorStatus();
		if(!iret){
			cout << "Info: UDP port opened at port " << port[i] << endl;
		}else{
#ifdef VERBOSE
			cprintf(BG_RED,"Error: Could not create UDP socket on port %d error: %d\n", port[i], iret);
#endif
			shutDownUDPSockets();
			return FAIL;
		}
	}

	cout << "Info: UDP socket(s) created successfully." << endl;
	cout << "Info: Listener Ready ..." << endl;

	return OK;
}



int UDPStandardImplementation::setupWriter(){
	FILE_LOG(logDEBUG1) << __AT__ << " starting";

	//acquisition start call back returns enable write
	cbAction = DO_EVERYTHING;
	if (startAcquisitionCallBack)
		cbAction=startAcquisitionCallBack(filePath,fileName,fileIndex,bufferSize,pStartAcquisition);

	if(cbAction < DO_EVERYTHING){
		cout << "Info: Call back activated. Data saving must be taken care of by user in call back." << endl;
		if (rawDataReadyCallBack)
			cout << "Info: Data Write has been defined externally" << endl;
	}else if(!fileWriteEnable)
		cout << "Info: Data will not be saved" << endl;



	//creating first file
	//setting all value to 1
	pthread_mutex_lock(&statusMutex);
	for(int i=0; i<numberofWriterThreads; i++)		createFileMask|=(1<<i);
	pthread_mutex_unlock(&statusMutex);

	for(int i=0; i<numberofWriterThreads; i++){
		FILE_LOG(logDEBUG4) <<	i << " Going to post 1st semaphore" << endl;
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

	cout << "Info: Successfully created file(s)" << endl;
	cout << "Info: Writer Ready ..." << endl;

	return fileCreateSuccess;
}



int UDPStandardImplementation::createNewFile(){
	FILE_LOG(logDEBUG1) << __AT__ << " called";

	int index = 0;
	if(packetsCaught)
		index = frameIndex;

	//create file name
	if(!frameIndexEnable)
		sprintf(completeFileName, "%s/%s_%d.raw", filePath,fileName,fileIndex);
	else if (myDetectorType == EIGER)
		sprintf(completeFileName, "%s/%s_f%012d_%d.raw", filePath,fileName,currentFrameNumber,fileIndex);
	else
		sprintf(completeFileName, "%s/%s_f%012d_%d.raw", filePath,fileName,(packetsCaught/packetsPerFrame),fileIndex);

#ifdef DEBUG4
	cout << "Info: " << completefileName << endl;
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
				cprintf(BG_RED,"Error: Could not create/overwrite file %s\n",completeFileName);
				return FAIL;
			}
		}else if (NULL == (sfilefd = fopen((const char *) (completeFileName), "w"))){
			cprintf(BG_RED,"Error: Could not create file %s\n",completeFileName);
			return FAIL;
		}
		//setting file buffer size to 16mb
		setvbuf(sfilefd,NULL,_IOFBF,BUF_SIZE);

		//Print packet loss and filenames
		if(!packetsCaught){
			previousFrameNumber = -1;
			cout << "Info: " << completeFileName << endl;
		}else{
			cout	<< "Info:" << completeFileName
					<< "\tPacket Loss: " << setw(4)<<fixed << setprecision(4) << dec <<
					(int)((( (currentFrameNumber-previousFrameNumber) - ((packetsInFile-numTotMissingPacketsInFile)/packetsPerFrame))/
							 (double)(currentFrameNumber-previousFrameNumber))*100.000)
					<< "%\tFramenumber: " << currentFrameNumber
				  //<< "\t\t PreviousFrameNumber: " << previousFrameNumber
					<< "\tIndex " << dec << index
					<< "\tLost " << dec << ( ((int)(currentFrameNumber-previousFrameNumber)) -
							                 ((packetsInFile-numTotMissingPacketsInFile)/packetsPerFrame)) << endl;

		}

	}

	//reset counters for each new file
	if(packetsCaught){
		previousFrameNumber = currentFrameNumber;
		packetsInFile = 0;
		numTotMissingPacketsInFile = 0;
	}

	return OK;
}




void* UDPStandardImplementation::startListeningThread(void* this_pointer){
	FILE_LOG(logDEBUG1) << __AT__ << " called";
	((UDPStandardImplementation*)this_pointer)->startListening();
	return this_pointer;
}



void* UDPStandardImplementation::startWritingThread(void* this_pointer){
	FILE_LOG(logDEBUG1) << __AT__ << " called";
	((UDPStandardImplementation*)this_pointer)->startWriting();
	return this_pointer;
}




void UDPStandardImplementation::startListening(){
	FILE_LOG(logDEBUG1) << __AT__ << " called";

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
	if(myDetectorType != EIGER){
		listenSize = bufferSize * numberofJobsPerBuffer;				//listen to more than 1 packet
		tempBuffer = new char[onePacketSize * (packetsPerFrame - 1)]; 	//store maximum of 1 packets less in a frame
	}
	/* outer loop - loops once for each acquisition */
	//infinite loop, exited only to change dynamic range, 10G parameters etc (then recreated again)
	while(true){

		//reset parameters before acquisition
		carryonBufferSize = 0;

		/* inner loop - loop for each buffer */
		//until mask unset (udp sockets shut down by client)
		while((1 << ithread) & listeningThreadsMask){

			//pop from fifo
			fifoFree[ithread]->pop(buffer[ithread]);
#ifdef FIFODEBUG
			cprintf(BLUE,"%d :Listener popped from fifofree %p\n", ithread, (void*)(buffer[ithread]));
#endif

			//udpsocket doesnt exist
			if(udpSocket[ithread] == NULL){
				cprintf(RED, "Error: Thread %d :UDP Socket not created\n",ithread);
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
#ifdef FIFODEBUG
					cprintf(BLUE,"Listening_Thread %d: Listener pushed into fifo %p\n",ithread, (void*)(buffer[ithread]));
#endif

		}/*--end of loop for each buffer (inner loop)*/

		//end of acquisition, wait for next acquisition/change of parameters
		sem_wait(&listenSemaphore[ithread]);

		//check to exit thread (for change of parameters) - only EXIT possibility
		if(killAllListeningThreads){
			cprintf(GREEN,"Listening_Thread %d:Goodbye!\n",ithread);
			//free resources at exit
			if(tempBuffer) delete[] tempBuffer;
			pthread_exit(NULL);
		}

	}/*--end of loop for each acquisition (outer loop) */
}





int UDPStandardImplementation::prepareAndListenBuffer(int ithread, int lSize, int cSize, char* temp){
	FILE_LOG(logDEBUG1) << __AT__ << " called";

	//listen to UDP packets
	memcpy(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS, temp, cSize);
	int receivedSize = udpSocket[ithread]->ReceiveDataOnly(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS + cSize, lSize + cSize);

	//throw away packets that is not one packet size, need to check status if socket is shut down
	while(status != TRANSMITTING && myDetectorType == EIGER && receivedSize != onePacketSize) {
		if(receivedSize != EIGER_HEADER_LENGTH)
			cprintf(RED,"Listening_Thread %d: Listened to a weird packet size %d\n",receivedSize);
#ifdef DEBUG
		else
			cprintf(BLUE,"Listening_Thread %d: Listened to a header packet\n",ithread);
#endif
		receivedSize = udpSocket[ithread]->ReceiveDataOnly(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS);
	}

#ifdef DEBUG
	cprintf(BLUE, "Listening_Thread %d : Received bytes: %d. Expected bytes: %d\n", ithread, receivedSize, expected-cSize);
#endif
	return receivedSize;
}


void UDPStandardImplementation::startFrameIndices(int ithread){
	FILE_LOG(logDEBUG1) << __AT__ << " called";

	//determine startFrameIndex
	switch(myDetectorType){
	case EIGER:
		startFrameIndex = 0;	//frame number always resets
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
		cprintf(BLUE,"Info: Thread %d: startAcquisitionIndex:%d\n",ithread,startAcquisitionIndex);
	}

	//set start of scan/real time measurement
	cprintf(BLUE,"Info: Thread %d: startFrameIndex: %d\n", ithread,startFrameIndex);
	measurementStarted = true;
}





void UDPStandardImplementation::stopListening(int ithread, int numbytes){
	FILE_LOG(logDEBUG1) << __AT__ << " called";

	cout << "Info: Stop Listening. Status:" << slsDetectorBase::runStatusType(status) << endl;


	//less than 1 packet size (especially for eiger), ignore the buffer (so that 2 dummy buffers are not sent with pc=0)
	if(numbytes < onePacketSize)
		numbytes = 0;


	//free empty buffer
	if(numbytes <= 0){
		cprintf(BLUE,"Info: Thread %d :End of Acquisition for Listening Thread\n", ithread);
		while(!fifoFree[ithread]->push(buffer[ithread]));
#ifdef FIFODEBUG
		cprintf(BLUE,"Listening_Thread %d :Listener push empty buffer into fifofree %p\n", ithread, (void*)(buffer[ithread]));
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
#ifdef FIFODEBUG
		cprintf(BLUE,"Listening_Thread %d: Listener Last Buffer pushed into fifo %p\n",  ithread,(void*)(buffer[ithread]));
#endif
	}

	//push dummy-end buffer into fifo for all writer threads
	for(int i=0; i<numberofWriterThreads; ++i){
		fifoFree[ithread]->pop(buffer[ithread]);
		//creating dummy-end buffer with pc=0xFFFF
		(*((uint32_t*)(buffer[ithread]))) = dummyPacketValue;
		while(!fifo[ithread]->push(buffer[ithread]));
#ifdef FIFODEBUG
		cprintf(BLUE,"Listening_Thread %d: Listener pushed dummy-end buffer into fifo %p\n", ithread,(void*)(buffer[ithread]));
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
	FILE_LOG(logDEBUG1) << __AT__ << " called";

	int lastPacketOffset;		//the offset of the last packet
	int lastFrameHeader;		//frame number of last packet in buffer
	uint32_t packetCount = (packetsPerFrame/numberofListeningThreads) * numberofJobsPerBuffer;		//packets received
	cSize = 0;					//reset size

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

	default:
		cprintf(RED,"Listening_Thread %d: Error: This detector is not implemented in the receiver" +
				slsDetectorBase::getDetectorType(myDetectorType).c_str() + "\n");
		break;
	}

#ifdef DEBUG4
	cprintf(BLUE,"Listening_Thread %d: PacketCount:%d CarryonBufferSize:%d\n",ithread, packetCount, cSize);
#endif

	return packetCount;
}






void UDPStandardImplementation::startWriting(){
	FILE_LOG(logDEBUG1) << __AT__ << " called";

	//set current thread value  index
	int ithread = currentThreadIndex;
	//let calling function know thread started and obtained current
	threadStarted = 1;

	//variable definitions
	char* wbuf[numberofListeningThreads];				//buffer popped from FIFO
	sfilefd = NULL;										//file pointer
	bool popReady[numberofListeningThreads];			//if the FIFO can be popped
	uint32_t numPackets[numberofListeningThreads];		//number of packets popped from the FIFO
	//eiger specific
	int MAX_NUM_PACKETS = 1024;							//highest 32 bit has 1024 number of packets
	char* toFreePointers[MAX_NUM_PACKETS];				//pointers to free for each frame
	int toFreePointersOffset[numberofListeningThreads];	//offset of pointers to free added for each thread



	/* outer loop - loops once for each acquisition */
	//infinite loop, exited only to change dynamic range, 10G parameters etc (then recreated again)
	while(true){

		//--reset parameters before acquisition
		for(int i=0; i<numberofListeningThreads; ++i){
			wbuf[i] = NULL;
			popReady[i] = true;
			numPackets[i] = 0;
			toFreePointersOffset[i] = (i*packetsPerFrame/numberofListeningThreads);
		}
		for(int i=0; i<MAX_NUM_PACKETS; ++i){
				toFreePointers[i] = NULL;
		}
		//--end of reset parameters before acquisition

		/* inner loop - loop for each buffer */
		//until mask unset (udp sockets shut down by client)
		while((1 << ithread) & writerThreadsMask){


			//pop fifo and if end of acquisition
			if(popAndCheckEndofAcquisition(ithread, wbuf, popReady, numPackets,toFreePointers,toFreePointersOffset)){
#ifdef DEBUG4
				cprintf(GREEN,"Writing_Thread %d: All dummy-end buffers popped\n", ithread);
#endif
				//finish missing packets

				if(myDetectorType == EIGER &&
						 ((tempoffset[0]!=0) || (tempoffset[1]!=(packetsPerFrame/numberofListeningThreads))));
				else{
					stopWriting(ithread,wbuf);
					continue;
				}
			}

			switch(myDetectorType){
			case EIGER:
				processWritingBufferPacketByPacket();
				break;
			default:
				if(!dataCompressionEnable)
					handleWithoutDataCompression(ithread, wbuf, numPackets[0]);
				else
					handleDataCompression(ithread,wbuf,d, xmax, ymax, nf);
				break;
			}

		}/*--end of loop for each buffer (inner loop)*/


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
			//free resources at exit
			for(int i=0; i<MAX_NUMBER_OF_LISTENING_THREADS; ++i)
				if(wbuf[i]) delete[] wbuf[i];
			pthread_exit(NULL);
		}
#ifdef DEBUG4
		cprintf(GREEN,"Writing_Thread %d: Got 1st post. Creating File\n", ithread);
#endif


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
			for(int i=0; i<MAX_NUMBER_OF_LISTENING_THREADS; ++i)
				if(wbuf[i]) delete[] wbuf[i];
			pthread_exit(NULL);
		}
#ifdef DEBUG4
		cprintf(GREEN,"Writing_Thread %d: Got 2nd post. Restarting Acquisition\n", ithread);
#endif

	}/*--end of loop for each acquisition (outer loop) */

}




bool UDPStandardImplementation::popAndCheckEndofAcquisition(int ithread, char* wbuffer[], bool ready[], uint32_t nP[],char* toFree[],int toFreeOffset[]){
	FILE_LOG(logDEBUG1) << __AT__ << " called";

	bool endofAcquisition = true;
	int val;
	for(int i=0; i<numberofListeningThreads; ++i){
		//pop if ready
		if(ready[i]){
			fifo[i]->pop(wbuffer[i]);
#ifdef FIFODEBUG
			cprintf(GREEN,"Writing_Thread %d: Popped %p from FIFO %d\n", ithread, (void*)(wbuffer[i]),i);
#endif
			val = (uint32_t)(*((uint32_t*)wbuffer[i]));
			if(val < 0)
				cprintf(BG_RED,"Error: Negative packet numbers: %d for FIFO %d\n",val,i);
			nP[i] = abs(val);
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
				switch(myDetectorType){
				case EIGER:
					eiger_packet_footer_t* wbuf_footer = (eiger_packet_footer_t*)(wbuffer[i] + footerOffset + HEADER_SIZE_NUM_TOT_PACKETS);
					//cprintf(BLUE,"footer value:0x%x\n",i,(uint64_t)(*( (uint64_t*) wbuf_footer)));
					cprintf(BLUE,"Fnum[%d]:%d\n",i,(uint32_t)(*( (uint64_t*) wbuf_footer)));
					cprintf(BLUE,"Pnum[%d]:%d\n",i,*( (uint16_t*) wbuf_footer->packetNumber));
					break;
				default: break;
				}
#endif
				if(myDetectorType == EIGER){
					toFree[toFreeOffset[i]] = wbuffer[i];
					toFreeOffset[i]++;
				}
			}
		}
	}

	return endofAcquisition;
}



void UDPStandardImplementation::stopWriting(int ithread, char* wbuffer[]){
	FILE_LOG(logDEBUG1) << __AT__ << " called";

	cprintf(GREEN,"Info: Writing_Thread %d: End of Acquisition\n",ithread);

	//free fifo
	for(int i=0; i<numberofListeningThreads; ++i){
		while(!fifoFree[i]->push(wbuffer[i]));
#ifdef FIFODEBUG
	cprintf(GREEN,"Writing_Thread %d: Freeing dummy-end buffer. Pushed into fifofree %p for listener %d\n", ithread,(void*)(wbuffer[i]),i);
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
		cprintf(GREEN, "Status: Run Finished\n");
		if(!totalPacketsCaught){
			cprintf(RED, "Total Missing Packets padded:%d\n",numTotMissingPackets);
			cprintf(RED, "Total Packets Caught: 0\n");
			cprintf(RED, "Total Frames Caught: 0\n");
		}else{
			cprintf(GREEN, "Total Missing Packets padded:%d\n",numTotMissingPackets);
			cprintf(GREEN, "Total Packets Caught:%d\n", totalPacketsCaught);
			cprintf(GREEN, "Total Frames Caught:%d\n",(totalPacketsCaught/packetsPerFrame));
		}
		//acquisition end
		if (acquisitionFinishedCallBack)
			acquisitionFinishedCallBack((totalPacketsCaught/packetsPerFrame), pAcquisitionFinished);
	}
}




void UDPStandardImplementation::handleWithoutDataCompression(int ithread, char* wbuffer[],int npackets){
	FILE_LOG(logDEBUG1) << __AT__ << " called";

	int i;

	//get frame number (eiger already gets it when it does packet to packet processing)
	if (myDetectorType != EIGER){
		uint64_t tempframenumber = ((uint32_t)(*((uint32_t*)(wbuffer[0] + HEADER_SIZE_NUM_TOT_PACKETS))));
		//for gotthard and normal frame, increment frame number to separate fnum and pnum
		if (myDetectorType == PROPIX ||(myDetectorType == GOTTHARD && shortFrameEnable == -1))
			tempframenumber++;
		//get frame number
		currentFrameNumber = (tempframenumber & frameIndexMask) >> frameIndexOffset;
		//set indices
		acquisitionIndex = currentFrameNumber - startAcquisitionIndex;
		frameIndex = currentFrameNumber - startFrameIndex;
	}


	//callback to write data
	if (cbAction < DO_EVERYTHING){
		switch(myDetectorType){
		case EIGER:
			for(i=0;i<npackets;++i)
				rawDataReadyCallBack(currentFrameNumber, wbuffer[i], onePacketSize, sfilefd, guiData, pRawDataReady);
			break;
		default:
			rawDataReadyCallBack(currentFrameNumber, wbuffer[0] + HEADER_SIZE_NUM_TOT_PACKETS, npackets * onePacketSize,
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
#ifdef FIFODEBUG
			cprintf(GREEN,"Writing_Thread %d: Freed buffer, pushed into fifofree %p for listener 0\n",ithread, (void*)(wbuffer[0]));
#endif
	}
}




void UDPStandardImplementation::writeFileWithoutCompression(char* wbuffer[],int numpackets){
	FILE_LOG(logDEBUG1) << __AT__ << " called";

	int i;

	//create headers for eiger
#ifdef WRITE_HEADERS
	if (myDetectorType == EIGER && cbAction == DO_EVERYTHING)
		createHeaders(wbuffer);
#endif

	//if write enabled
	if((fileWriteEnable) && (sfilefd)){
		int offset = HEADER_SIZE_NUM_TOT_PACKETS; 		//offset (not eiger) to keep track of how many packets saved
		int packetsToSave;								//how many packets to save at a time
		volatile uint64_t tempframenumber;
		int lastpacket;

		//loop to take care of creating new files when it reaches max packets per file
		while(numpackets > 0){

			//to create new file when max reached
			packetsToSave = maxPacketsPerFile - packetsInFile;
			if(packetsToSave > numpackets)
				packetsToSave = numpackets;

			//write to file
			if(cbAction == DO_EVERYTHING){
				switch(myDetectorType){
				case EIGER:
					for(i=0; i<packetsToSave; ++i)
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

			//new file
			if(packetsInFile >= (uint32_t)maxPacketsPerFile){
				//for packet loss, because currframenum is the latest one for eiger
				if(myDetectorType != EIGER){
					lastpacket = (((packetsToSave - 1) * onePacketSize) + offset);

					//get frame number (eiger already gets it when it does packet to packet processing)
					tempframenumber = ((uint32_t)(*((uint32_t*)(wbuffer[0] + lastpacket))));
					//for gotthard and normal frame, increment frame number to separate fnum and pnum
					if (myDetectorType == PROPIX ||(myDetectorType == GOTTHARD && shortFrameEnable == -1))
						tempframenumber++;
					//get frame number
					currentFrameNumber = (tempframenumber & frameIndexMask) >> frameIndexOffset;
					//set indices
					acquisitionIndex = currentFrameNumber - startAcquisitionIndex;
					frameIndex = currentFrameNumber - startFrameIndex;
				}
#ifdef DEBUG3
				cprintf(GREEN,"Writing_Thread: Current Frame Number:%d\n",currentFrameNumber);
#endif
				createNewFile();
			}

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

	eiger_packet_header_t* wbuf_header=0;
	eiger_packet_footer_t* wbuf_footer=0;
	int port = 0, missingPacket;

	for (int i = 0; i < packetsPerFrame; i++){

		wbuf_header = (eiger_packet_header_t*) wbuffer[i];
		wbuf_footer = (eiger_packet_footer_t*)(wbuffer[i] + footerOffset);
#ifdef DEBUG4
		cprintf(GREEN, "Loop index:%d Pnum:%d\n",i,*( (uint16_t*) wbuf_footer->packetNumber));
#endif
		//which port
		if (i ==(packetsPerFrame/2))	port = 1;

		//missing packet
		if (*( (uint16_t*) wbuf_header->missingPacket)== missingPacketValue){
#ifdef DEBUG4
			cprintf(GREEN,"Missing packet at %d\n", i+1);
#endif
			missingPacket = 1;
			//add frame and packet numbers
			*( (uint64_t*) wbuf_footer) = (uint64_t)((currentFrameNumber+1));
			*( (uint16_t*) wbuf_footer->packetNumber) = (i+1);
		}
		//normal packet
		else{
			missingPacket = 0;
			//correct the packet numbers of port2 so that port1 and 2 are not the same
			if(port)  *( (uint16_t*) wbuf_footer->packetNumber) = (*( (uint16_t*) wbuf_footer->packetNumber))+(packetsPerFrame/2);
		}
		//DEBUGGING
		if(*( (uint16_t*) wbuf_footer->packetNumber) != (i+1)){
			cprintf(BG_RED, "Packet Number Mismatch! i:%d pnum:%d fnum:%d missingPacket:%d\n",
					i,*( (uint16_t*) wbuf_footer->packetNumber),currentFrameNumber,*( (uint16_t*) wbuf_header->missingPacket));
			exit(-1);
		}
		//overwriting port number and dynamic range
		*( (uint8_t*) wbuf_header->portIndex) = port;
		*( (uint8_t*) wbuf_header->dynamicRange) = dynamicRange;
	}
}


void UDPStandardImplementation::copyFrameToGui(char* buffer[]){
	FILE_LOG(logDEBUG1) << __AT__ << " called";

	int i;

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
			for(int i=0; i<packetsPerFrame; ++i)
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

void UDPStandardImplementation::processWritingBufferPacketByPacket(int ithread, char* wbuffer[], uint32_t nP[]){
	FILE_LOG(logDEBUG1) << __AT__ << " called";



}















































int UDPStandardImplementation::createCompressionFile(int ithr, int iframe){
	FILE_LOG(logDEBUG1) << __AT__ << " called";

#ifdef MYROOT1
	char temp[MAX_STR_LENGTH];
		//create file name for gui purposes, and set up acquistion parameters
		sprintf(temp, "%s/%s_fxxx_%d_%d.root", filePath,fileName,fileIndex,ithr);
		//file
		myFile[ithr] = new TFile(temp,"RECREATE");/** later  return error if it exists */
		cout << "Thread " << ithr << ": created File: "<< temp << endl;
		//tree
		sprintf(temp, "%s_fxxx_%d_%d",fileName,fileIndex,ithr);
		myTree[ithr]=singlePhotonDet[ithr]->initEventTree(temp, &iframe);
		//resets the pedestalSubtraction array and the commonModeSubtraction
		singlePhotonDet[ithr]->newDataSet();
		if(myFile[ithr]==NULL){
			cout<<"file null"<<endl;
			return FAIL;
		}
		if(!myFile[ithr]->IsOpen()){
			cout<<"file not open"<<endl;
			return FAIL;
		}
#endif
	return OK;
}










int UDPStandardImplementation::startWriting(){
	FILE_LOG(logDEBUG1) << __AT__ << " called";



	char *d=new char[bufferSize*numListeningThreads];
	int xmax=0,ymax=0;
	int ret,i,j;

	bool endofacquisition;
	int  nf;
	bool fullframe[numListeningThreads];
	volatile uint32_t tempframenum[numListeningThreads];
	uint32_t presentframenum;
	uint32_t lastpacketheader[numListeningThreads], currentpacketheader[numListeningThreads];
	int numberofmissingpackets[numListeningThreads];



	char* tempbuffer[MAX_VALUE];
	char* blankframe[MAX_VALUE];
	int tempoffset[numListeningThreads];
	int blankoffset;
	for(i=0;i<MAX_VALUE;++i){
		tempbuffer[i] = 0;
		blankframe[i] = 0;
	}


	uint32_t LAST_PACKET_VALUE;

	eiger_packet_footer_t* wbuf_footer=0;

	eiger_packet_header_t* tempframe_header=0;
	eiger_packet_footer_t* tempframe_footer=0;

	eiger_packet_header_t* blankframe_header=0;
	unsigned char* blankframe_data=0;


	while(1){


		nf = 0;
		if(myDetectorType == MOENCH){
			xmax = MOENCH_PIXELS_IN_ONE_ROW-1;
			ymax = MOENCH_PIXELS_IN_ONE_ROW-1;
		}else{
			if(shortFrame == -1){
				xmax = GOTTHARD_PIXELS_IN_ROW-1;
				ymax = GOTTHARD_PIXELS_IN_COL-1;
			}else{
				xmax = GOTTHARD_SHORT_PIXELS_IN_ROW-1;
				ymax = GOTTHARD_SHORT_PIXELS_IN_COL-1;
			}
		}

		//so that the first frame is always copied
		guiData = latestData;

		//blank frame
		if(myDetectorType == EIGER){
			for(i=0;i<packetsPerFrame;++i){
				if(blankframe[i]){delete [] blankframe[i]; blankframe[i] = 0;}

				//blank frame for each packet
				blankframe[i] = new char[onePacketSize];
				blankframe_header = (eiger_packet_header_t*) blankframe[i];
				//set missing packet to 0xff
				*( (uint16_t*) blankframe_header->missingpacket) = missingPacketValue;

				//set each value inside blank frame to 0xff
				for(j=0;j<(oneDataSize);++j){
					blankframe_data = (unsigned char*)blankframe[i] + sizeof(eiger_packet_header_t) + j;
					*(blankframe_data) = 0xFF;
				}
				//verify
				if (*( (uint16_t*) blankframe_header->missingpacket)  != missingPacketValue){
					cprintf(RED,"blank frame not detected at %d: 0x%x\n",i,*( (uint16_t*) blankframe_header->missingpacket) );
					exit(-1);
				}
#ifdef FIFO_DEBUG
				cprintf(GREEN,"packet %d blank frame 0x%x\n",i,(void*)(blankframe[i]));
#endif
			}

			//last packet numbers for different dynamic ranges
			LAST_PACKET_VALUE = (packetsPerFrame/numListeningThreads);
		}




		//allow them all to be popped initially
		for(i=0;i<numListeningThreads;++i){
			fullframe[i] = false;
			tempoffset[i] = (i*packetsPerFrame/numListeningThreads);
			blankoffset = 0;
			lastpacketheader[i] = 0;
			currentpacketheader[i] = 0;
			numberofmissingpackets[i] = 0;

			numpackets[i] = 0;
			tempframenum[i] = 0;
		}
		endofacquisition = false;
		presentframenum = 0;


		while((1<<ithread)&writerthreads_mask){








			if(myDetectorType == EIGER){



				//NOT FULL FRAME
				if(!fullframe[0] || !fullframe[1]){
					for(i=0;i<numListeningThreads;++i){



						//anything that is not a data packet of right size
						if(numpackets[i] != onePacketSize){
							//header packet
							if(numpackets[i] == EIGER_HEADER_LENGTH) {cprintf(BG_RED,"weird, header frame packet recieved. shouldnt\n"); exit(-1);}
							//dummy packet
							else if(!numpackets[i]){
#ifdef EIGER_DEBUG3
								cprintf(RED, "Dummy packet: %d from fifo %d\n", numpackets[i],i);
#endif
								//cout<<"tempoffset["<<i<<"]:"<<tempoffset[i]<<" checking against:"<<(((i+1)*packetsPerFrame/numListeningThreads))<<endl;
								//cannot check for full frame as it will be false  if its done with all packets OR waiting for packets
								if(tempoffset[i]!= (((i+1)*packetsPerFrame/numListeningThreads))){
#ifdef VERYDEBUG
									cprintf(RED, "Dummy packet: Adding missing packets\n");
#endif
									//add missing packets
									numberofmissingpackets[i] = (LAST_PACKET_VALUE - lastpacketheader[i]);
									//to decrement from packetsInFile to calculate packet loss
									for(j=0;j<numberofmissingpackets[i];++j){
										tempbuffer[tempoffset[i]] = blankframe[blankoffset];

										tempframe_header = (eiger_packet_header_t*) tempbuffer[tempoffset[i]];
										blankframe_header = (eiger_packet_header_t*) blankframe[blankoffset];
										if (*( (uint16_t*) tempframe_header->missingpacket)!= missingPacketValue){
											cprintf(BG_RED, "dummy blank mismatch num4 earlier2! "
													"i:%d pnum:%d fnum:%d missingpacket:0x%x actual missingpacket:0x%x\n",
													i,tempoffset[i],tempframenum[i],
													*( (uint16_t*) tempframe_header->missingpacket),
													*( (uint16_t*) blankframe_header->missingpacket));
											exit(-1);
										}else
#ifdef PADDING
											cprintf(GREEN, "blank packet i:%d pnum:%d fnum:%d missingpacket:0x%x\n",i,
													tempoffset[i],tempframenum[i],*( (uint16_t*) tempframe_header->missingpacket));
#endif
										tempoffset[i]++;
										blankoffset++;
									}
									//set fullframe and dont let fifo pop over it until written
									fullframe[i] = true;
									popready[i] = false;
								}
							}
#ifdef EIGER_DEBUG3
							else{
								cprintf(RED, "WARNING: Got a weird packet size: %d from fifo %d\n", numpackets[i],i);
								continue;
							}
#endif
						}





						//not a full frame
						if(!fullframe[i]){
							wbuf_footer = (eiger_packet_footer_t*)(wbuf[i] + footer_offset + HEADER_SIZE_NUM_TOT_PACKETS);
#ifdef EIGER_DEBUG3
							cprintf(GREEN,"**pnum of %d: %d\n",i,(*( (uint16_t*) wbuf_footer->packetnum)));
#endif
							//update frame number
							if(!((uint32_t)(*( (uint64_t*) wbuf_footer)))){
								cprintf(BG_RED,"%d VERY WEIRD frame number=%d and popready:%d\n",
										i,(uint32_t)(*( (uint64_t*) wbuf_footer)),popready[i]);
								popready[i]=true;
								continue;
							}
							tempframenum[i] =(uint32_t)(*( (uint64_t*) wbuf_footer));
							tempframenum[i] +=	(startFrameIndex-1);


							//WRONG FRAME - leave
							if(tempframenum[i] != presentframenum){
#ifdef PADDING
								cout<<"wrong packet"<<endl;
#endif

#ifdef EIGER_DEBUG3
								cprintf(RED,"fifo:%d packet from next frame %d, add missing packets to the right one %d\n",
										i,tempframenum[i],presentframenum );
								cprintf(RED,"current wrong frame:%d wrong frame packet number:%d\n",
										(uint32_t)(*( (uint64_t*) wbuf_footer)),
										*( (uint16_t*) wbuf_footer->packetnum));

#endif
								tempframenum[i] = presentframenum;
								//add missing packets
								numberofmissingpackets[i] = (LAST_PACKET_VALUE - lastpacketheader[i]);
#ifdef VERYDEBUG
								if(numberofmissingpackets[i]>0)
									cprintf(BG_RED,"fifo:%d missing packet from: %d now\n",i,lastpacketheader[i]);
#endif
								//to decrement from packetsInFile to calculate packet loss
								for(j=0;j<numberofmissingpackets[i];++j){
									tempbuffer[tempoffset[i]] = blankframe[blankoffset];

									tempframe_header = (eiger_packet_header_t*) tempbuffer[tempoffset[i]];
									blankframe_header = (eiger_packet_header_t*) blankframe[blankoffset];
									if (*( (uint16_t*) tempframe_header->missingpacket)!= missingPacketValue){
										cprintf(BG_RED, "wrong blank mismatch num4 earlier2! "
												"i:%d pnum:%d fnum:%d missingpacket:0x%x actual missingpacket:0x%x add:0x%p\n",
												i,tempoffset[i],tempframenum[i],
												*( (uint16_t*) tempframe_header->missingpacket),
												*( (uint16_t*) blankframe_header->missingpacket),
												(void*)(tempbuffer[tempoffset[i]]));
										exit(-1);
									}else
#ifdef PADDING
										cprintf(GREEN, "blank packet i:%d pnum:%d fnum:%d missingpacket:0x%x add:0x%x\n",
												i,tempoffset[i],tempframenum[i],
												*( (uint16_t*) tempframe_header->missingpacket),
												(void*)(tempbuffer[tempoffset[i]]));
#endif
									tempoffset[i] ++;
									blankoffset ++;
								}
								//set fullframe and dont let fifo pop over it until written
								fullframe[i] = true;
								popready[i] = false;
							}


							//CORRECT FRAME - continue building frame
							else {
#ifdef PADDING
								cout<<"correct packet"<<endl;
#endif
#ifdef EIGER_DEBUG3
								cprintf(GREEN,"**tempfraemnum of %d: %d\n",i,tempframenum[i]);
#endif
								//update current packet
								currentpacketheader[i] = *( (uint16_t*) wbuf_footer->packetnum);
#ifdef VERYVERBOSE
								cprintf(GREEN,"**fifo:%d currentpacketheader: %d lastpacketheader %d tempoffset:%d\n",
										i,currentpacketheader[i],lastpacketheader[i], tempoffset[i]);
#endif
								//add missing packets
								numberofmissingpackets[i] = (currentpacketheader[i] - lastpacketheader[i] -1);
#ifdef VERYDEBUG
								if(numberofmissingpackets[i]>0)
									cprintf(BG_RED,"fifo:%d missing packet from: %d now at :%d tempoffset:%d\n",
											i,lastpacketheader[i],currentpacketheader[i],tempoffset[i]);
#endif
								//to decrement from packetsInFile to calculate packet loss
								for(j=0;j<numberofmissingpackets[i];++j){
									tempbuffer[tempoffset[i]] = blankframe[blankoffset];

									tempframe_header = (eiger_packet_header_t*) tempbuffer[tempoffset[i]];
									blankframe_header = (eiger_packet_header_t*) blankframe[blankoffset];
									if (*( (uint16_t*) tempframe_header->missingpacket)!= missingPacketValue){
										cprintf(BG_RED, "correct blank mismatch num4 earlier2! "
												"i:%d pnum:%d fnum:%d missingpacket:0x%x actual missingpacket:0x%x add:0x%p\n",
												i,tempoffset[i],tempframenum[i],
												*( (uint16_t*) tempframe_header->missingpacket),
												*( (uint16_t*) blankframe_header->missingpacket),
												(void*)(tempbuffer[tempoffset[i]]));
										exit(-1);
									}else
#ifdef PADDING
										cprintf(GREEN, "blank packet i:%d pnum:%d fnum:%d missingpacket:0x%x add:0x%x\n",
												i,tempoffset[i],tempframenum[i],
												*( (uint16_t*) tempframe_header->missingpacket),
												(void*)(tempbuffer[tempoffset[i]]));
#endif
									tempoffset[i] ++;
									blankoffset ++;
								}
								//add current packet
								if(currentpacketheader[i] != (uint32_t)(tempoffset[i]-(i*packetsPerFrame/numListeningThreads))+1){
									cprintf(BG_RED, "correct pnum mismatch earlier! tempoffset[%d]:%d pnum:%d fnum:%d rfnum:%d\n",
											i,tempoffset[i],currentpacketheader[i],
											tempframenum[i],(uint32_t)(*( (uint64_t*) wbuf_footer)));
									exit(-1);
								}

								tempbuffer[tempoffset[i]] = wbuf[i] + HEADER_SIZE_NUM_TOT_PACKETS;
								tempframe_header = (eiger_packet_header_t*) tempbuffer[tempoffset[i]];
								tempframe_footer = (eiger_packet_footer_t*) (tempbuffer[tempoffset[i]] + footer_offset);
#ifdef EIGER_DEBUG3
								cprintf(GREEN,"**fifo:%d currentpacketheader: %d tempoffset:%d\n",
										i,*( (uint16_t*) tempframe_footer->packetnum),tempoffset[i]);
#endif
								if(*( (uint16_t*) tempframe_footer->packetnum)!= (tempoffset[i]-(i*packetsPerFrame/numListeningThreads))+1){
									cprintf(BG_RED, "pnum mismatch num4 earlier! i:%d pnum:%d pnum orig:%d fnum:%d add:0x%p\n",
											i,*( (uint16_t*) tempframe_footer->packetnum),*( (uint16_t*) wbuf_footer->packetnum),
											tempframenum[i],(void*)(tempbuffer[tempoffset[i]]));
									exit(-1);
								}
#ifdef PADDING
								cprintf(GREEN, "normal packet i:%d pnum:%d fnum:%d missingpacket:0x%x add:0x%x\n",
										i,tempoffset[i],tempframenum[i],
										*( (uint16_t*) tempframe_header->missingpacket),
										(void*)(tempbuffer[tempoffset[i]]));
#endif
								tempoffset[i] ++;
								//update last packet
								lastpacketheader[i] = currentpacketheader[i];
								popready[i] = true;
								//last frame got, this will save time and also for last frames, it doesnt wait for stop receiver
								if(currentpacketheader[i] == LAST_PACKET_VALUE){
#ifdef EIGER_DEBUG3
									cprintf(GREEN, "Got last packet\n");
#endif
									fullframe[i] = true;
									popready[i] = false;
								}
							}
						}
					}
				}



				//FULL FRAME
				if(fullframe[0] && fullframe[1]){

					//determine frame number
					if(tempframenum[0] != tempframenum[1])
						cprintf(RED,"Frame numbers mismatch!!! %d %d\n",tempframenum[0],tempframenum[1]);
					currframenum = tempframenum[0];
					numMissingPackets += (numberofmissingpackets[0]+numberofmissingpackets[1]);
					numTotMissingPacketsInFile += numMissingPackets;
					numTotMissingPackets += numMissingPackets;
#ifdef EIGER_DEBUG2
					cprintf(GREEN,"**fnum:%d**\n",currframenum);
#endif
#ifdef EIGER_DEBUG3
					if(numberofmissingpackets[0])
						cprintf(RED, "fifo 0 missing packets:%d fnum:%d\n",numberofmissingpackets[0],currframenum);
					if(numberofmissingpackets[1])
						cprintf(RED, "fifo 1 missing packets:%d fnum:%d\n",numberofmissingpackets[1],currframenum);
					if(numMissingPackets){
						cprintf(RED, "numMissingPackets:%d fnum:%d\n",numMissingPackets,currframenum);

						for (j=0;j<packetsPerFrame;++j){
							tempframe_header = (eiger_packet_header_t*) tempbuffer[j];
							if (*( (uint16_t*) tempframe_header->missingpacket)==missingPacketValue)
								cprintf(RED,"found the missing packet at pnum:%d\n",j);
						}
					}
#endif


					//write and copy to gui
					handleWithoutDataCompression(ithread,tempbuffer,packetsPerFrame);


					//freeing
					for(j=0;j<tofreeoffset[0];++j){
						while(!fifoFree[0]->push(tofree[j]));
#ifdef FIFO_DEBUG
						cprintf(GREEN,"%d writer freed pushed into fifofree %x for listener %d\n",ithread, (void*)(tofree[j]),0);
#endif
					}
					for(j=(packetsPerFrame/numListeningThreads);j<tofreeoffset[1];++j){
						while(!fifoFree[1]->push(tofree[j]));
#ifdef FIFO_DEBUG
						cprintf(GREEN,"%d writer freed pushed into fifofree %x for listener %d\n",ithread, (void*)(tofree[j]),1);
#endif
					}
#ifdef VERYDEBUG
					cprintf(GREEN,"finished freeing\n");
#endif


					//reset a few stuff
					presentframenum =  tempframenum[0]+1;
					for(int i=0;i<numListeningThreads;i++){
						fullframe[i] = false;
						//no dummy packet and is the last packet (if not last packet, next frame, dont pop over it)
						if((numpackets[i]) && (currentpacketheader[i] == LAST_PACKET_VALUE))
							popready[i] = true;
						/*cprintf(GREEN,"popready[%d]:%d\n",i,popready[i]);*/
						tempoffset[i] = (i*packetsPerFrame/numListeningThreads);
						tofreeoffset[i] = (i*packetsPerFrame/numListeningThreads);
						blankoffset = 0;
						lastpacketheader[i] = 0;
						currentpacketheader[i] = 0;
						numberofmissingpackets[i] = 0;
					}

				}
#ifdef EIGER_DEBUG3
				for(int i=0;i<numListeningThreads;i++){
					wbuf_footer = (eiger_packet_footer_t*)(wbuf[i] + footer_offset + HEADER_SIZE_NUM_TOT_PACKETS);
					cprintf(GREEN," end of loop popready[%d]:%d add:0x%x\n",i,popready[i],(void*)(wbuf[i]));
					cprintf(GREEN,"tempframenum[%d]:%d\n",i,(uint32_t)(*( (uint64_t*) wbuf_footer)));
					cprintf(GREEN,"packetnum[%d]:%d\n",i,*( (uint16_t*) wbuf_footer->packetnum));
				}
#endif
			}




		}

	}


	return OK;
}


















void UDPStandardImplementation::handleDataCompression(int ithread, char* wbuffer[], char* data, int xmax, int ymax, int &nf){
	FILE_LOG(logDEBUG1) << __AT__ << " called";

	uint64_t tempframenumber = ((uint32_t)(*((uint32_t*)(wbuffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS))));
	//for gotthard and normal frame, increment frame number to separate fnum and pnum
	if (myDetectorType == PROPIX ||(myDetectorType == GOTTHARD && shortFrameEnable == -1))
		tempframenumber++;
	//get frame number
	tempframenumber = (tempframenumber & frameIndexMask) >> frameIndexOffset;


		pthread_mutex_lock(&progressMutex);
		if(tempframenumber > currentFrameNumber)
			currentFrameNumber = tempframenumber;
		pthread_mutex_unlock(&progressMutex);
		//set indices
		acquisitionIndex = currentFrameNumber - startAcquisitionIndex;
		frameIndex = currentFrameNumber - startFrameIndex;

#if defined(MYROOT1) && defined(ALLFILE_DEBUG)
				writeToFile_withoutCompression(wbuf[0], numpackets,currframenum);
#endif
				int npackets = (uint32_t)(*((uint32_t*)wbuffer[0]));
				eventType thisEvent = PEDESTAL;
				int ndata;
				char* buff = 0;
				data = wbuffer[0]+ HEADER_SIZE_NUM_TOT_PACKETS;
				int remainingsize = npackets * onePacketSize;
				int np;
				int once = 0;
				double tot, tl, tr, bl, br;
				int xmin = 1, ymin = 1, ix, iy;


				while(buff = receiverdata[ithread]->findNextFrame(data,ndata,remainingsize)){
					np = ndata/onePacketSize;

					//cout<<"buff framnum:"<<ithread <<":"<< ((((uint32_t)(*((uint32_t*)buff)))& (frameIndexMask)) >> frameIndexOffset)<<endl;

					if ((np == packetsPerFrame) && (buff!=NULL)){
						if(nf == 1000) cout << "Thread " << ithread << ": pedestal done " << endl;


						singlePhotonDet[ithread]->newFrame();

						//only for moench
						if(commonModeSubtractionEnable){
							for(ix = xmin - 1; ix < xmax+1; ix++){
								for(iy = ymin - 1; iy < ymax+1; iy++){
									thisEvent = singlePhotonDet[ithread]->getEventType(buff, ix, iy, 0);
								}
							}
						}


						for(ix = xmin - 1; ix < xmax+1; ix++)
							for(iy = ymin - 1; iy < ymax+1; iy++){
								thisEvent=singlePhotonDet[ithread]->getEventType(buff, ix, iy, commonModeSubtractionEnable);
								if (nf>1000) {
									tot=0;
									tl=0;
									tr=0;
									bl=0;
									br=0;
									if (thisEvent==PHOTON_MAX) {
										receiverdata[ithread]->getFrameNumber(buff);
										//iFrame=receiverdata[ithread]->getFrameNumber(buff);
#ifdef MYROOT1
										myTree[ithread]->Fill();
										//cout << "Fill in event: frmNr: " << iFrame <<  " ix " << ix << " iy " << iy << " type " <<  thisEvent << endl;
#else
										pthread_mutex_lock(&write_mutex);
										if((enableFileWrite) && (sfilefd))
											singlePhotonDet[ithread]->writeCluster(sfilefd);
										pthread_mutex_unlock(&write_mutex);
#endif
									}
								}
							}

						nf++;
#ifndef ALLFILE
						pthread_mutex_lock(&progress_mutex);
						packetsInFile += packetsPerFrame;
						packetsCaught += packetsPerFrame;
						totalPacketsCaught += packetsPerFrame;
						if(packetsInFile >= (uint32_t)maxPacketsPerFile)
							createNewFile();
						pthread_mutex_unlock(&progress_mutex);

#endif
						if(!once){
							copyFrameToGui(NULL,buff);
							once = 1;
						}
					}

					remainingsize -= ((buff + ndata) - data);
					data = buff + ndata;
					if(data > (wbuffer[0] + HEADER_SIZE_NUM_TOT_PACKETS + npackets * onePacketSize) )
						cprintf(BG_RED,"ERROR SHOULD NOT COME HERE, Error 142536!\n");

				}

				while(!fifoFree[0]->push(wbuffer[0]));
#ifdef FIFO_DEBUG
				cprintf(BLUE,"%d writer compression free pushed into fifofree %x for listerner 0\n", ithread, (void*)(wbuffer[0]));
#endif

}

