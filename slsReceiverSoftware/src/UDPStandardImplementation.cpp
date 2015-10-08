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
using namespace std;

#define WRITE_HEADERS

/*************************************************************************
 * Constructor & Destructor **********************************************
 * They access local cache of configuration or detector parameters *******
 *************************************************************************/

UDPStandardImplementation::UDPStandardImplementation(){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	initializeMembers();
}

UDPStandardImplementation::~UDPStandardImplementation(){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	deleteMembers();
}


/*************************************************************************
 * Setters ***************************************************************
 * They modify the local cache of configuration or detector parameters ***
 *************************************************************************/

/***initial parameters***/
void UDPStandardImplementation::deleteBaseMembers(){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	UDPBaseImplementation::~UDPBaseImplementation();
}

void UDPStandardImplementation::deleteMembers(){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

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
	UDPBaseImplementation::UDPBaseImplementation();
}


void UDPStandardImplementation::initializeMembers(){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

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
	maxPacketsPerFile = 0;
	fileCreateSuccess = false;

	//***acquisition indices parameters***
	startAcquisitionIndex = 0;
	startFrameIndex = 0;
	frameIndex = 0;
	currentFrameNumber = 0;
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
	numberofListeningThreads = 1;
	currentListeningThreadIndex = -1;
	listeningThreadsMask = 0x0;
	killAllListeningThreads = false;

	//***writer thread parameters***
	numberofWriterThreads = 1;
	currentWriterThreadIndex = -1;
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

	//***mutex***
	pthread_mutex_init(&status_mutex,NULL);

	//***callback***
	cbAction = DO_EVERYTHING;
}



void UDPStandardImplementation::initializeFilter(){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

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



int UDPStandardImplementation::createListeningThreads(bool destroy){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	//reset masks
	killAllListeningThreads = false;
	pthread_mutex_lock(&status_mutex);
	listeningThreadsMask = 0x0;
	pthread_mutex_unlock(&(status_mutex));

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
		currentListeningThreadIndex = -1;

		for(int i = 0; i < numberofListeningThreads; ++i){
			sem_init(&listenSemaphore[i],1,0);
			threadStarted = false;
			currentListeningThreadIndex = i;
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
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	//reset masks
	killAllWritingThreads = false;
	pthread_mutex_lock(&status_mutex);
	writerThreadsMask = 0x0;
	createFileMask = 0x0;
	pthread_mutex_unlock(&(status_mutex));

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
		currentWriterThreadIndex = -1;

		for(int i = 0; i < numberofWriterThreads; ++i){
			sem_init(&writerSemaphore[i],1,0);
			threadStarted = false;
			currentWriterThreadIndex = i;
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

	if(!rights)
		cout << "Warning: No root permission to prioritize threads." << endl;

}



int UDPStandardImplementation::setupFifoStructure(){
	FILE_LOG(logDEBUG) << __AT__ << " called";

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




int UDPStandardImplementation::createUDPSockets(){
	FILE_LOG(logDEBUG) << __AT__ << " called";

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
	FILE_LOG(logDEBUG) << __AT__ << " starting";

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
	pthread_mutex_lock(&status_mutex);
	for(int i=0; i<numberofWriterThreads; i++)
		createFileMask|=(1<<i);
	pthread_mutex_unlock(&status_mutex);

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
		cout << "Info: Bottom Enable: " << stringEnable(bottomEnable) << endl;
	}

}


/***file parameters***/
int UDPStandardImplementation::setDataCompressionEnable(const bool b){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	cout << "Info: Setting up Data Compression Enable to " << stringEnable(b);
#ifdef MYROOT1
	cout << " WITH ROOT" << endl;
#else
	cout << " WITHOUT ROOT" << endl;
#endif

	//set data compression enable
	dataCompressionEnable = b;

	//-- create writer threads depending on enable
	pthread_mutex_lock(&status_mutex);
	writerThreadsMask = 0x0;
	pthread_mutex_unlock(&(status_mutex));

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

	cout << "Info: Short Frame Enable set to " << shortFrameEnable << endl;
}


int UDPStandardImplementation::setFrameToGuiFrequency(const uint32_t i){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	if(i >= 0){
		FrameToGuiFrequency = i;
		if(setupFifoStructure() == FAIL)
			return FAIL;
	}

	cout << "Info: Frame to Gui Frequency set to " << FrameToGuiFrequency << endl;

	return OK;
}


int UDPStandardImplementation::setAcquisitionPeriod(int64_t i){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	if(i >= 0){
		acquisitionPeriod = i;
		if(setupFifoStructure() == FAIL)
			return FAIL;
	}

	cout << "Info: Acquisition Period set to " << acquisitionPeriod << endl;


	return OK;
}

int UDPStandardImplementation::setDynamicRange(const uint32_t i){
	FILE_LOG(logDEBUG) << __AT__ << " called";

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
	FILE_LOG(logDEBUG) << __AT__ << " called";

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
	FILE_LOG(logDEBUG) << __AT__ << " called";

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
		cout << "Error: This is an unknown receiver type " << (int)d << endl;
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
		pthread_mutex_lock(&status_mutex);
		listeningThreadsMask = 0x0;
		pthread_mutex_unlock(&(status_mutex));
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
		exit (-1);
	}
	if(createWriterThreads() == FAIL){
		cprintf(BG_RED,"Error: Could not create writer threads\n");
		exit (-1);
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
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	totalPacketsCaught = 0;
	acqStarted = false;
	startAcquisitionIndex = 0;

	cout << "Info: Acquisition Count has been reset" << endl;
}


int UDPStandardImplementation::startReceiver(char *c=NULL){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	cout << "Info: Starting Receiver" << endl;


	//RESET
	//reset measurement variables
	measurementStarted = false;
	startFrameIndex = 0;
	frameIndex = 0;
	if(!acqStarted)
		currentFrameNumber = 0;		//has to be zero to add to startframeindex for each scan
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
	pthread_mutex_lock(&status_mutex);
	writerThreadsMask = 0x0;
	createFileMask = 0x0;
	fileCreateSuccess = false;
	pthread_mutex_unlock(&status_mutex);


	//Print Receiver Configuration
	cout << "Info: ***Receiver Configuration***" << endl;
	cout << "Info: Max Packets Per File:" << maxPacketsPerFile << endl;
	cout << "Info: Data Compression has been " << stringEnable(dataCompressionEnable) << endl;
	if(myDetectorType != EIGER)
		cout << "Info: Number of Jobs Per Buffer: " << numberofJobsPerBuffer << endl;
	if(FrameToGuiFrequency)
		cout << "Info: Frequency of frames sent to gui" << FrameToGuiFrequency << endl;



	//create UDP sockets
	if(createUDPSockets() == FAIL){
		strcpy(c,"Could not create UDP Socket(s).\n");
		cout << endl;
		cout << "Error: "<< c << endl;
		return FAIL;
	}

	if(setupWriter() == FAIL){
		//stop udp socket
		shutDownUDPSockets();
		sprintf(c,"Could not create file %s.\n",savefilename);
		cout << endl;
		cout << "Error: "<< c << endl;
		return FAIL;
	}


	//For compression, done to give the gui some proper name instead of always the last file name
	if(dataCompressionEnable)
		sprintf(savefilename, "%s/%s_fxxx_%d_xx.root", filePath,fileName,fileIndex);

	//initialize semaphore to synchronize between writer and gui reader threads
	sem_init(&writerGuiSemaphore,1,0);

	//status and thread masks
	pthread_mutex_lock(&status_mutex);
	status = RUNNING;
	for(int i=0;i<numberofListeningThreads;i++)
		listeningThreadsMask|=(1<<i);
	for(int i=0;i<numberofWriterThreads;i++)
		writerThreadsMask|=(1<<i);
	pthread_mutex_unlock(&(status_mutex));


	//start listening /writing
	for(int i=0;i<numberofListeningThreads;i++)
		sem_post(&listenSemaphore[i]);
	for(int i=0; i < numberofWriterThreads; i++)
		sem_post(&writerSemaphore[i]);

	//usleep(5000000);
	cout << "Info: Receiver Started." << endl;
	cout << "Info: Status:" << status << endl;

	return OK;
}



int UDPStandardImplementation::shutDownUDPSockets(){
	FILE_LOG(logDEBUG) << __AT__ << " called";

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






































UDPStandardImplementation::UDPStandardImplementation(){
	thread_started = 0;
	eth = NULL;

	tengigaEnable = 0;

	for(int i=0;i<MAX_NUM_LISTENING_THREADS;i++){

		server_port[i] = DEFAULT_UDP_PORTNO+i;

	}

	for(int i=0;i<MAX_NUM_WRITER_THREADS;i++){
		singlePhotonDet[i] = NULL;
		receiverdata[i] = NULL;
	}

	startAcquisitionCallBack = NULL;
	pStartAcquisition = NULL;
	acquisitionFinishedCallBack = NULL;
	pAcquisitionFinished = NULL;
	rawDataReadyCallBack = NULL;
	pRawDataReady = NULL;


	//mutex
	pthread_mutex_init(&dataReadyMutex,NULL);
	pthread_mutex_init(&status_mutex,NULL);
	pthread_mutex_init(&progress_mutex,NULL);
	pthread_mutex_init(&write_mutex,NULL);

	initializeMembers();

	//to increase socket receiver buffer size and max length of input queue by changing kernel settings
	if(system("echo $((100*1024*1024)) > /proc/sys/net/core/rmem_max"))
	  cout << "\nWARNING: Could not change socket receiver buffer size in file /proc/sys/net/core/rmem_max" << endl;
	else if(system("echo 250000 > /proc/sys/net/core/netdev_max_backlog"))
	  cout << "\nWARNING: Could not change max length of input queue in file /proc/sys/net/core/netdev_max_backlog" << endl;

	/** permanent setting heiner
	    net.core.rmem_max = 104857600 # 100MiB
	    net.core.netdev_max_backlog = 250000
	    sysctl -p
	    // from the manual
	    sysctl -w net.core.rmem_max=16777216
	    sysctl -w net.core.netdev_max_backlog=250000
	*/
		}



void UDPStandardImplementation::initializeMembers(){
	myDetectorType = GENERIC;
	enableFileWrite = 1;
	overwrite = 1;
	fileIndex = 0;
	scanTag = 0;
	frameIndexNeeded = 0;



	packetsCaught = 0;
	totalPacketsCaught = 0;

	startAcquisitionIndex = 0;
	acquisitionIndex = 0;

	frameIndexMask = 0;
	packetIndexMask = 0;
	frameIndexOffset = 0;
	acquisitionPeriod = SAMPLE_TIME_IN_NS;
	numberOfFrames = 0;
	dynamicRange = 16;
	shortFrame = -1;
	currframenum = 0;
	prevframenum = 0;


	nFrameToGui = 0;
	dataCompression = false;
	numListeningThreads = 1;
	numWriterThreads = 1;
	thread_started = 0;



	tengigaEnable = 0;



	eth = NULL;



	cmSub = NULL;


	//diff threads
	for(int i=0;i<numWriterThreads;i++){
		commonModeSubtractionEnable = false;
		singlePhotonDet[i] = NULL;
		receiverdata[i] = NULL;
#ifdef MYROOT1
		myTree[i] = (NULL);
		myFile[i] = (NULL);
#endif
	}




	strcpy(savefilename,"");
	

	//strcpy(filePath,"");
	//strcpy(fileName,"run");



	//status
	pthread_mutex_lock(&status_mutex);
	status = IDLE;
	pthread_mutex_unlock(&(status_mutex));

}



UDPStandardImplementation::~UDPStandardImplementation(){ 	FILE_LOG(logDEBUG) << __AT__ << " called";

	createListeningThreads(true);
	createWriterThreads(true);
	deleteMembers();
}




void UDPStandardImplementation::deleteMembers(){ 	FILE_LOG(logDEBUG) << __AT__ << " called";

	//kill threads
	if(thread_started){
		createListeningThreads(true);
		createWriterThreads(true);
	}

	for(int i=0;i<numWriterThreads;i++){
		if(singlePhotonDet[i]){
			delete singlePhotonDet[i];
			singlePhotonDet[i] = NULL;
		}
		if(receiverdata[i]){
			delete receiverdata[i];
			receiverdata[i] = NULL;
		}
	}
	shutDownUDPSockets();
	if(eth) 			{delete [] eth;			eth = NULL;}
	if(latestData) 		{delete [] latestData;	latestData = NULL;}

	for(int i=0;i<numListeningThreads;i++){
		if(mem0[i])			{free(mem0[i]);			mem0[i] = NULL;}
		if(fifo[i])			{delete fifo[i];		fifo[i] = NULL;}
		if(fifoFree[i])		{delete fifoFree[i];	fifoFree[i] = NULL;}
	}

}


/*Frame indices and numbers caught*/

//bool UDPStandardImplementation::getAcquistionStarted(){return acqStarted;};

//bool UDPStandardImplementation::getMeasurementStarted(){return measurementStarted;};

//uint32_t UDPStandardImplementation::getStartAcquisitionIndex(){return startAcquisitionIndex;}

//uint32_t UDPStandardImplementation::getStartFrameIndex(){return startFrameIndex;}

/*
	acquisitionIndex = currframenum - startAcquisitionIndex
		frameIndex = currframenum - startFrameIndex;

}
*/



















/*other parameters*/




/** acquisition functions */
void UDPStandardImplementation::readFrame(char* c,char** raw, uint32_t &startAcquisitionIndex, uint32_t &startFrameIndex){
	FILE_LOG(logDEBUG) << __AT__ << " called";
	//point to gui data
	if (guiData == NULL){
		guiData = latestData;
#ifdef VERY_VERY_DEBUG
		cprintf(CYAN,"gui data not null anymore\n");
#endif
	}

	//copy data and filename
	strcpy(c,guiFileName);
	startAcquisitionIndex = getStartAcquisitionIndex();
	startFrameIndex = getStartFrameIndex();


	//could not get gui data
	if(!guiDataReady){
#ifdef VERY_VERY_DEBUG
		cprintf(CYAN,"gui data not ready\n");
#endif
		*raw = NULL;
	}
	//data ready, set guidata to receive new data
	else{
#ifdef VERY_VERY_DEBUG
		cprintf(CYAN,"gui data ready\n");
#endif
		*raw = guiData;
		guiData = NULL;

		/*pthread_mutex_lock(&dataReadyMutex); WHY WAS THIS HERE IN THE FIRST PLACE
		guiDataReady = 0;
		pthread_mutex_unlock(&dataReadyMutex);*/
		if((nFrameToGui) && (writerthreads_mask)){
#ifdef VERY_VERY_DEBUG
			cprintf(CYAN,"gonna post\n");
#endif
		/*if(nFrameToGui){*/
			//release after getting data
			sem_post(&smp);
		}
#ifdef VERY_VERY_DEBUG
		cprintf(CYAN,"done post\n");
#endif
	}
}





void UDPStandardImplementation::copyFrameToGui(char* startbuf[], char* buf){
	FILE_LOG(logDEBUG) << __AT__ << " called";
#ifdef VERY_VERY_DEBUG
cout << "copyframe" << endl;
#endif

	//random read when gui not ready , also command line doesnt have nthframetogui
	//else guidata always null as guidataready is always 1 after 1st frame, and seccond data never gets copied
	if((!nFrameToGui) && (!guiData)){
#ifdef VERY_VERY_DEBUG
		cprintf(GREEN,"doing nothing\n");
#endif
		pthread_mutex_lock(&dataReadyMutex);
		guiDataReady=0;
		pthread_mutex_unlock(&dataReadyMutex);
	}

	//random read or nth frame read, gui needs data now or it is the first frame
	else{
#ifdef VERY_VERY_DEBUG
		cprintf(GREEN,"gui needs data now or 1st frame\n");
#endif
		pthread_mutex_lock(&dataReadyMutex);
		guiDataReady=0;
#ifdef VERY_VERY_DEBUG
		cprintf(GREEN,"guidataready is  0, copying data\n");
#endif
		//eiger
		if(startbuf != NULL){

			for(int j=0;j<packetsPerFrame;++j)
				memcpy((((char*)latestData)+j * onePacketSize) ,startbuf[j],onePacketSize);

		}else//other detectors
			memcpy(latestData,buf,bufferSize);


		strcpy(guiFileName,savefilename);
		guiDataReady=1;
		pthread_mutex_unlock(&dataReadyMutex);
#ifdef VERY_VERY_DEBUG
		cprintf(GREEN,"guidataready = 1\n");
#endif
		//nth frame read, block current process if the guireader hasnt read it yet
		if(nFrameToGui){
#ifdef VERY_VERY_DEBUG
			cprintf(GREEN,"waiting after copying\n");
#endif
			sem_wait(&smp);
#ifdef VERY_VERY_DEBUG
			cprintf(GREEN,"done waiting\n");
#endif
		}

	}
}





int UDPStandardImplementation::createCompressionFile(int ithr, int iframe){
	FILE_LOG(logDEBUG) << __AT__ << " called";

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




int UDPStandardImplementation::createNewFile(){
	FILE_LOG(logDEBUG) << __AT__ << " called";

 int gt = getFrameIndex();
 if(gt==-1) gt=0;
	//create file name
	if(frameIndexNeeded==-1)
		sprintf(savefilename, "%s/%s_%d.raw", filePath,fileName,fileIndex);
	else if (myDetectorType == EIGER)
		sprintf(savefilename, "%s/%s_f%012d_%d.raw", filePath,fileName,currframenum,fileIndex);
	else
		sprintf(savefilename, "%s/%s_f%012d_%d.raw", filePath,fileName,(packetsCaught/packetsPerFrame),fileIndex);

#ifdef VERBOSE
	cout << filePath << " + " << fileName << endl;
#endif

	//if filewrite and we are allowed to write
	if(enableFileWrite && cbAction > DO_NOTHING){
		//close
		if(sfilefd){
			if(fclose(sfilefd)){
				cprintf(RED, "file close problem %d\n",fileno(sfilefd));
				fclose(sfilefd);
			}
			sfilefd = NULL;
		}

		//open file
		if(!overwrite){
			if (NULL == (sfilefd = fopen((const char *) (savefilename), "wx"))){
				cprintf(BG_RED,"Error: Could not create new file %s\n",savefilename);
				return FAIL;
			}
		}else if (NULL == (sfilefd = fopen((const char *) (savefilename), "w"))){
			cprintf(BG_RED,"Error: Could not create file %s\n",savefilename);
			return FAIL;
		}
		//setting buffer
		setvbuf(sfilefd,NULL,_IOFBF,BUF_SIZE);


		//printing packet losses and file names
		if(!packetsCaught)
			cout << savefilename << endl;
		else{
			cout << savefilename
					<< "\tpacket loss "
					<< setw(4)<<fixed << setprecision(4)<< dec <<
					(int)((((currframenum-prevframenum)-((packetsInFile-numTotMissingPacketsInFile)/packetsPerFrame))/(double)(currframenum-prevframenum))*100.000)
					<< "%\tframenum "
					<< dec << currframenum //<< "\t\t p " << prevframenum
					<< "\tindex " << dec << gt
					<< "\tlost " << dec << (((int)(currframenum-prevframenum))-((packetsInFile-numTotMissingPacketsInFile)/packetsPerFrame)) << endl;

		}
	}

	//reset counters for each new file
	if(packetsCaught){
		prevframenum = currframenum;
		packetsInFile = 0;
		numTotMissingPacketsInFile = 0;
	}

	return OK;
}








void UDPStandardImplementation::closeFile(int ithr){
	FILE_LOG(logDEBUG) << __AT__ << " called";

#ifdef VERBOSE
	cout << "In closeFile for thread " << ithr << endl;
#endif

	if(!dataCompression){
		if(sfilefd){
#ifdef VERBOSE
			cprintf(YELLOW, "gonna close file:%d\n",fileno(sfilefd));
#endif
			if(fclose(sfilefd))
				perror("file close ERROR");
			sfilefd = NULL;
		}
	}
	//compression
	else{
#if (defined(MYROOT1) && defined(ALLFILE_DEBUG)) || !defined(MYROOT1)
		if(sfilefd){
#ifdef VERBOSE
			cout << "sfield:" << (int)sfilefd << endl;
#endif
			if(fclose(sfilefd))
				perror("close ERRROR");
			sfilefd = NULL;
		}
#endif

#ifdef MYROOT1
		pthread_mutex_lock(&write_mutex);
		//write to file
		if(myTree[ithr] && myFile[ithr]){
			myFile[ithr] = myTree[ithr]->GetCurrentFile();

			if(myFile[ithr]->Write())
				//->Write(tall->GetName(),TObject::kOverwrite);
				cout << "Thread " << ithr <<": wrote frames to file" << endl;
			else
				cout << "Thread " << ithr << ": could not write frames to file" << endl;

		}else
			cout << "Thread " << ithr << ": could not write frames to file: No file or No Tree" << endl;
		//close file
		if(myTree[ithr] && myFile[ithr])
			myFile[ithr] = myTree[ithr]->GetCurrentFile();
		if(myFile[ithr] != NULL)
			myFile[ithr]->Close();
		myFile[ithr] = NULL;
		myTree[ithr] = NULL;
		pthread_mutex_unlock(&write_mutex);

#endif
	}
}




/**
 * Pre: status is running, semaphores have been instantiated,
 * Post: udp sockets shut down, status is idle, sempahores destroyed
 * */

int UDPStandardImplementation::stopReceiver(){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	if(status != IDLE){
		//#ifdef VERBOSE
		cout << "Stopping Receiver" << endl;
		//#endif

		startReadout();

		while(status == TRANSMITTING){
			sem_post(&smp);
			usleep(5000);
		}

		//semaphore destroy
		sem_destroy(&smp);

		//change status
		pthread_mutex_lock(&status_mutex);
		status = IDLE;
		pthread_mutex_unlock(&(status_mutex));

		cout << "Receiver Stopped.\nStatus:" << status << endl << endl;
	}else cout <<" Not idle to stop receiver" << endl;


	//sem_post(&smp);

	return OK;
}




/**
 * Pre: status is running, udp sockets have been initialized,
 * stop receiver initiated
 * Post:udp sockets closed, status is transmitting
 * */
void UDPStandardImplementation::startReadout(){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	//#ifdef VERBOSE
		cout << "Start Receiver Readout" << endl;
	//#endif

	if(status == RUNNING){

		//wait so that all packets which take time has arrived
		usleep(5000);

		/********************************************/
		//usleep(10000000);
		//usleep(2000000);
		uint32_t prev = totalPacketsCaught;
		usleep(50000);
		while(prev!=totalPacketsCaught){
			prev=totalPacketsCaught;
			usleep(50000);
		}
		pthread_mutex_lock(&status_mutex);
		status = TRANSMITTING;
		pthread_mutex_unlock(&status_mutex);
		cout << "Status: Transmitting" << endl;
	}

	//kill udp socket to tell the listening thread to push last packet
	shutDownUDPSockets();

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






int UDPStandardImplementation::startListening(){
	FILE_LOG(logDEBUG) << __AT__ << " called";
	int ithread = currentListeningThreadIndex;
#ifdef VERYVERBOSE
	cprintf(BLUE, "In startListening()\n ");
#endif

	thread_started = 1;

	int total;
	int lastpacketoffset, expected, rc,packetcount, maxBufferSize, carryonBufferSize;
	uint32_t lastframeheader;// for moench to check for all the packets in last frame
	char* tempchar = NULL;

	while(1){
		//variables that need to be checked/set before each acquisition
		carryonBufferSize = 0;
		maxBufferSize = bufferSize * numJobsPerThread;
#ifdef VERYDEBUG
		cprintf(BLUE, "%d maxBufferSize:%d carryonBufferSize:%d\n", ithread,maxBufferSize,carryonBufferSize);
#endif

		//missing packets compensation in listening thread
		if(tempchar) {delete [] tempchar;tempchar = NULL;}
		if(myDetectorType != EIGER)
			tempchar = new char[onePacketSize * ((packetsPerFrame/numListeningThreads) - 1)]; //gotthard: 1packet size, moench:39 packet size
		else
			maxBufferSize = 0;


		while((1<<ithread)&listeningthreads_mask){


			//pop
#ifdef VERYDEBUG
			cprintf(BLUE, "%d waiting to pop out of listeningfifo\n",ithread);
#endif
			fifoFree[ithread]->pop(buffer[ithread]);
#ifdef FIFO_DEBUG
			cprintf(BLUE,"%d listener popped from fifofree %x\n", ithread, (void*)(buffer[ithread]));
#endif



			//ensure udpsocket exists
			if(udpSocket[ithread] == NULL){
				rc = 0;
				cprintf(BLUE, "%d UDP Socket is NULL\n",ithread);
			}


			//normal listening
			else if(!carryonBufferSize){
#ifdef SOCKET_DEBUG
				if(!ithread){
#endif
					rc = udpSocket[ithread]->ReceiveDataOnly(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS, maxBufferSize);
					if(rc == EIGER_HEADER_LENGTH && myDetectorType == EIGER) {
						while(rc == EIGER_HEADER_LENGTH){
							rc = udpSocket[ithread]->ReceiveDataOnly(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS, maxBufferSize);
						}
					}
					expected = maxBufferSize;
#ifdef SOCKET_DEBUG
				}else{
					while(1) usleep(100000000);
				}
#endif
			}


			//the remaining packets from previous buffer, copy it and listen to n less frame
			else{
#ifdef VERYDEBUG
				cprintf(BLUE, "%d carry on buffer size:%d\n",ithread,carryonBufferSize);
				cprintf(BLUE, "%d framennum in tempchar:%d\n",((((uint32_t)(*((uint32_t*)tempchar)))
						& (frameIndexMask)) >> frameIndexOffset));
				cprintf(BLUE, "%d tempchar packet:%d\n", ((((uint32_t)(*((uint32_t*)(tempchar)))))
						& (packetIndexMask)));
#endif
				memcpy(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS, tempchar, carryonBufferSize);
				rc = udpSocket[ithread]->ReceiveDataOnly((buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS + carryonBufferSize),maxBufferSize - carryonBufferSize);
				expected = maxBufferSize - carryonBufferSize;
			}


#ifdef EIGER_DEBUG
			cprintf(BLUE, "%d rc: %d. expected: %d\n", ithread, rc, expected);
#endif


			//start indices for each start of scan/acquisition
			if((!measurementStarted) && (rc > 0)){
				pthread_mutex_lock(&progress_mutex);
				if(!measurementStarted)
					startFrameIndices(ithread, rc);
				pthread_mutex_unlock(&progress_mutex);
			}


			//problem in receiving or end of acquisition
			if (status == TRANSMITTING){
				stopListening(ithread,rc,packetcount,total);
				continue;
			}


			//reset
			packetcount = (packetsPerFrame/numListeningThreads) * numJobsPerThread;
			carryonBufferSize = 0;



			//check if last packet valid and calculate packet count
			switch(myDetectorType){
			case MOENCH:
				lastpacketoffset = (((numJobsPerThread * packetsPerFrame - 1) * onePacketSize) + HEADER_SIZE_NUM_TOT_PACKETS);
#ifdef VERYDEBUG
				cout <<"first packet:"<< ((((uint32_t)(*((uint32_t*)(buffer[ithread]+HEADER_SIZE_NUM_TOT_PACKETS))))) & (packetIndexMask)) << endl;
				cout <<"first header:"<< (((((uint32_t)(*((uint32_t*)(buffer[ithread]+HEADER_SIZE_NUM_TOT_PACKETS))))) & (frameIndexMask)) >> frameIndexOffset) << endl;
				cout << "last packet offset:" << lastpacketoffset << endl;
				cout <<"last packet:"<< ((((uint32_t)(*((uint32_t*)(buffer[ithread]+lastpacketoffset))))) & (packetIndexMask)) << endl;
				cout <<"last header:"<< (((((uint32_t)(*((uint32_t*)(buffer[ithread]+lastpacketoffset))))) & (frameIndexMask)) >> frameIndexOffset) << endl;
#endif
				//moench last packet value is 0
				if( ((((uint32_t)(*((uint32_t*)(buffer[ithread]+lastpacketoffset))))) & (packetIndexMask))){
					lastframeheader = ((((uint32_t)(*((uint32_t*)(buffer[ithread]+lastpacketoffset))))) & (frameIndexMask)) >> frameIndexOffset;
					carryonBufferSize += onePacketSize;
					lastpacketoffset -= onePacketSize;
					--packetcount;
					while (lastframeheader == (((((uint32_t)(*((uint32_t*)(buffer[ithread]+lastpacketoffset))))) & (frameIndexMask)) >> frameIndexOffset)){
						carryonBufferSize += onePacketSize;
						lastpacketoffset -= onePacketSize;
						--packetcount;
					}
					memcpy(tempchar, buffer[ithread]+(lastpacketoffset+onePacketSize), carryonBufferSize);
#ifdef VERYDEBUG
					cout << "tempchar header:" << (((((uint32_t)(*((uint32_t*)(tempchar)))))
							& (frameIndexMask)) >> frameIndexOffset) << endl;
					cout <<"tempchar packet:"<< ((((uint32_t)(*((uint32_t*)(tempchar)))))
							& (packetIndexMask)) << endl;
#endif
				}
				break;

			case GOTTHARD:
			case PROPIX:
				if(shortFrame == -1){
					lastpacketoffset = (((numJobsPerThread * packetsPerFrame - 1) * onePacketSize) + HEADER_SIZE_NUM_TOT_PACKETS);
#ifdef VERYDEBUG
					cprintf(BLUE, "%d last packet offset:%d\n",ithread, lastpacketoffset);
#endif
					//if not last packet
					if((unsigned int)(packetsPerFrame -1) != ((((uint32_t)(*((uint32_t*)(buffer[ithread]+lastpacketoffset))))+1) & (packetIndexMask))){
						memcpy(tempchar,buffer[ithread]+lastpacketoffset, onePacketSize);
#ifdef VERYDEBUG
						cprintf(BLUE, "%d tempchar header:%d\n",ithread,(((((uint32_t)(*((uint32_t*)(tempchar))))+1)
								& (frameIndexMask)) >> frameIndexOffset));
#endif
						carryonBufferSize = onePacketSize;
						--packetcount;
					}
				}
#ifdef VERYDEBUG
				cprintf(BLUE, "%d header:%d\n", (((((uint32_t)(*((uint32_t*)(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS))))+1)
						& (frameIndexMask)) >> frameIndexOffset));
#endif
				break;



			case EIGER:
				//because even headers might be included, so not packet count
				(*((uint32_t*)(buffer[ithread]))) = rc;
				packetcount = 1;
				break;

			default:
				break;
			}



			//write packet count and push
#ifdef VERYDEBUG
			cprintf(BLUE, "%d packetcount:%d carryonbuffer:%d\n", ithread, packetcount, carryonBufferSize);
#endif
			if(myDetectorType != EIGER)
				(*((uint32_t*)(buffer[ithread]))) = packetcount;
			totalListeningFrameCount[ithread] += packetcount;
#ifdef VERYDEBUG
			cprintf(BLUE,"%d listener going to push fifo: 0x%x\n", ithread,(void*)(buffer[ithread]));
#endif
			while(!fifo[ithread]->push(buffer[ithread]));
#ifdef FIFO_DEBUG
			cprintf(BLUE, "%d listener pushed into fifo %x\n",ithread, (void*)(buffer[ithread]));
#endif



		}

		sem_wait(&listensmp[ithread]);

		//make sure its not exiting thread
		if(killAllListeningThreads){
			cout << ithread << " good bye listening thread" << endl;
			if(tempchar) {delete [] tempchar;tempchar = NULL;}
			pthread_exit(NULL);
		}

		if(tempchar) {delete [] tempchar;tempchar = NULL;}
	}

	return OK;
}













int UDPStandardImplementation::startWriting(){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	int ithread = currentWriterThreadIndex;
#ifdef VERYVERBOSE
	cprintf(GREEN,"%d In startWriting()\n", ithread);
#endif

	thread_started = 1;

	char* wbuf[numListeningThreads];//interleaved
	char *d=new char[bufferSize*numListeningThreads];
	int xmax=0,ymax=0;
	int ret,i,j;

	bool endofacquisition;
	int numpackets[numListeningThreads], nf;
	bool fullframe[numListeningThreads],popready[numListeningThreads];
	volatile uint32_t tempframenum[numListeningThreads];
	uint32_t presentframenum;
	uint32_t lastpacketheader[numListeningThreads], currentpacketheader[numListeningThreads];
	int numberofmissingpackets[numListeningThreads];

	int MAX_VALUE = 1024;//32 bit number of packets
	char* tofree[MAX_VALUE];
	char* tempbuffer[MAX_VALUE];
	char* blankframe[MAX_VALUE];
	int tofreeoffset[numListeningThreads];
	int tempoffset[numListeningThreads];
	int blankoffset;
	for(i=0;i<MAX_VALUE;++i){
		tempbuffer[i] = 0;
		tofree[i] = 0;
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
			popready[i] = true;
			tempoffset[i] = (i*packetsPerFrame/numListeningThreads);
			tofreeoffset[i] = (i*packetsPerFrame/numListeningThreads);
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



			//pop
			endofacquisition = true;
			for(i=0;i<numListeningThreads;++i){
				if(popready[i]){
					fifo[i]->pop(wbuf[i]);
#ifdef FIFO_DEBUG
					cprintf(GREEN,"%d writer poped 0x%x from fifo %d\n", ithread, (void*)(wbuf[i]), i);
#endif
					numpackets[i] = (uint32_t)(*((uint32_t*)wbuf[i]));

#ifdef VERYDEBUG
					cprintf(GREEN,"%d numpackets: %d for fifo :%d\n", ithread, numpackets[i], i);
#endif
					if(numpackets < 0){
						cprintf(BG_RED,"negative numpackets[%d]%d\n",i,numpackets[i]);
						exit(-1);
					}
					//dont pop again if dummy packet
					else if(numpackets[i] == 0){
						popready[i] = false;
#ifdef EIGER_DEBUG3
						cprintf(GREEN,"%d Dummy frame popped out of fifo %d",ithread, i);
#endif
					}else{
						endofacquisition = false;
						if(numpackets[i] == onePacketSize){;
#ifdef EIGER_DEBUG3
							wbuf_footer = (eiger_packet_footer_t*)(wbuf[i] + footer_offset + HEADER_SIZE_NUM_TOT_PACKETS);
							//cprintf(BLUE,"footer value:0x%x\n",i,(uint64_t)(*( (uint64_t*) wbuf_footer)));
							cprintf(BLUE,"tempframenum[%d]:%d\n",i,(uint32_t)(*( (uint64_t*) wbuf_footer)));
							cprintf(BLUE,"packetnum[%d]:%d\n",i,*( (uint16_t*) wbuf_footer->packetnum));
#endif
						}else if(numpackets[i] == EIGER_HEADER_LENGTH){
							cprintf(BG_RED, "got header in writer, weirdd packetsize:%d\n",numpackets[i]);
							exit(-1);
						}
#ifdef EIGER_DEBUG3
						else {
							cprintf(BG_RED, "got weird in writer, weirdd packetsize:%d\n",numpackets[i]);
						}
#endif
						if(myDetectorType == EIGER){
							tofree[tofreeoffset[i]] = wbuf[i];
							tofreeoffset[i]++;
						}
					}

				}
			}



			//END OF ACQUISITION
			if(endofacquisition){
#ifdef EIGER_DEBUG3
				cprintf(GREEN,"%d Both dummy frames\n", ithread);
#endif
				//remaining packets to be written
				if((myDetectorType == EIGER) &&
						((tempoffset[0]!=0) || (tempoffset[1]!=(packetsPerFrame/numListeningThreads))));
				else{

					stopWriting(ithread,wbuf);
					continue;
				}
			}



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


			//other detectors other than eiger
			else{

				//frame number for progress
				if ((myDetectorType == PROPIX) ||((myDetectorType == GOTTHARD) && (shortFrame == -1)))
					tempframenum[0] = (((((uint32_t)(*((uint32_t*)(wbuf[ithread] + HEADER_SIZE_NUM_TOT_PACKETS))))+1)& (frameIndexMask)) >> frameIndexOffset);
				else
					tempframenum[0] = ((((uint32_t)(*((uint32_t*)(wbuf[ithread] + HEADER_SIZE_NUM_TOT_PACKETS))))& (frameIndexMask)) >> frameIndexOffset);

				if(numWriterThreads == 1)
					currframenum = tempframenum[0];
				else{
					pthread_mutex_lock(&progress_mutex);
					if(tempframenum[0] > currframenum)
						currframenum = tempframenum[0];
					pthread_mutex_unlock(&progress_mutex);
				}


				//without datacompression: write datacall back, or write data, free fifo
				if(!dataCompression)    handleWithoutDataCompression(ithread,wbuf, numpackets[0]);
				//data compression
				else					handleDataCompression(ithread,wbuf,d, xmax, ymax, nf);

			}

		}
#ifdef VERYVERBOSE
		cprintf(GREEN,"%d gonna wait for 1st sem\n", ithread);
#endif
		//wait
		sem_wait(&writersmp[ithread]);
		if(killAllWritingThreads){
			for(i=0;i<packetsPerFrame;++i)
				if (blankframe[i]) {delete [] blankframe[i]; blankframe[i]=0;}
			cprintf(GREEN,"%d  good bye writing thread\n", ithread);
			closeFile(ithread);
			pthread_exit(NULL);
		}
#ifdef VERYVERBOSE
		cprintf(GREEN,"%d got 1st post\n", ithread);
#endif


		if((1<<ithread)&createfile_mask){
			if(dataCompression){
#ifdef MYROOT1
				pthread_mutex_lock(&write_mutex);
				ret = createCompressionFile(ithread,0);
				pthread_mutex_unlock(&write_mutex);
				if(ret == FAIL)
					ret_createfile = FAIL;
#endif
			}else{
				ret = createNewFile();
				if(ret == FAIL)
					ret_createfile = FAIL;
			}

			//let tcp know
			pthread_mutex_lock(&status_mutex);
			createfile_mask^=(1<<ithread);
			pthread_mutex_unlock(&status_mutex);
		}


#ifdef VERYVERBOSE
		cprintf(GREEN,"%d gonna wait for 2nd sem\n", ithread);
#endif
		//wait
		sem_wait(&writersmp[ithread]);
		if(killAllWritingThreads){
			for(i=0;i<packetsPerFrame;++i)
				if (blankframe[i]) {delete [] blankframe[i]; blankframe[i]=0;}
			cprintf(GREEN,"%d Goodbye thread\n", ithread);
			closeFile(ithread);
			pthread_exit(NULL);
		}
#ifdef VERYVERBOSE
		cprintf(GREEN,"%d got 2nd post\n", ithread);
#endif

	}

	delete [] d;
	for(i=0;i<packetsPerFrame;++i)
		if (blankframe[i]) {delete [] blankframe[i]; blankframe[i]=0;}

	return OK;
}




void UDPStandardImplementation::startFrameIndices(int ithread, int numbytes){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	//add currframenum later  in this method for scans
	if (myDetectorType == EIGER)
		startFrameIndex = 0;
	//gotthard has +1 for frame number and not a short frame
	else if ((myDetectorType == PROPIX) || ((myDetectorType == GOTTHARD) && (shortFrame == -1)))
		startFrameIndex = (((((uint32_t)(*((uint32_t*)(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS))))+1)
				& (frameIndexMask)) >> frameIndexOffset);
	else
		startFrameIndex = ((((uint32_t)(*((uint32_t*)(buffer[ithread]+HEADER_SIZE_NUM_TOT_PACKETS))))
				& (frameIndexMask)) >> frameIndexOffset);


	//start of acquisition
	if(!acqStarted){
		startAcquisitionIndex=startFrameIndex;
		//currframenum = startAcquisitionIndex;
		acqStarted = true;
		cprintf(BLUE,"%d startAcquisitionIndex:%d\n", ithread, startAcquisitionIndex);
	}

	cprintf(BLUE,"%d startFrameIndex: %d\n", ithread,startFrameIndex);
	prevframenum=startFrameIndex-1; //so that there is no packet loss, when currframenum(max,20) - prevframenum(1)
	measurementStarted = true;

}



void UDPStandardImplementation::stopListening(int ithread, int rc, int &pc, int &t){
	FILE_LOG(logDEBUG) << __AT__ << " called";


	int i;

#ifdef VERYVERBOSE
	cprintf(BLUE, "%d Stop Listening\n", ithread);
#endif


	if(status != TRANSMITTING){
		cprintf(BG_RED,"%d *** udp socket not shut down from client ***********************\n", ithread);
		while(!fifoFree[ithread]->push(buffer[ithread]));
		exit(-1);
	}


	//free buffer
	if(rc <= 0){
		cprintf(BLUE,"%d End of acquisition for Listening Thread\n", ithread);
		while(!fifoFree[ithread]->push(buffer[ithread]));
#ifdef FIFO_DEBUG
		cprintf(BLUE,"%d listener empty buffer pushed into fifofree %x\n", ithread, (void*)(buffer[ithread]));
#endif
	}


	//push the last buffer into fifo
	else{
		if(myDetectorType == EIGER){
			(*((uint32_t*)(buffer[ithread]))) = rc;
			pc = 1;
		}else{
			pc = (rc/onePacketSize);
			(*((uint32_t*)(buffer[ithread]))) = pc;
		}
#ifdef VERYDEBUG
		cprintf(BLUE,"%d last rc:%d\n",ithread, rc);
		cprintf(BLUE,"%d last packetcount:%d\n", ithread, pc);
#endif

		totalListeningFrameCount[ithread] += pc;
		while(!fifo[ithread]->push(buffer[ithread]));
#ifdef FIFO_DEBUG
		cprintf(BLUE,"%d listener last buffer pushed into fifo %x\n",  ithread,(void*)(buffer[ithread]));
#endif
	}




	//push dummy buffer to all writer threads
	for(i=0;i<numWriterThreads;++i){
		fifoFree[ithread]->pop(buffer[ithread]);
#ifdef FIFO_DEBUG
		cprintf(BLUE,"%d listener popped dummy buffer from fifofree %x\n", ithread,(void*)(buffer[ithread]));
#endif
		(*((uint32_t*)(buffer[ithread]))) = 0x0;
#ifdef VERYDEBUG
		cprintf(BLUE,"%d dummy buffer num packets:%d\n", ithread(*((uint16_t*)(buffer[ithread]))));
#endif
		while(!fifo[ithread]->push(buffer[ithread]));
#ifdef FIFO_DEBUG
		cprintf(BLUE,"%d listener pushed dummy buffer into fifo %x\n", ithread,(void*)(buffer[ithread]));
#endif
	}



	//reset mask and exit loop
	pthread_mutex_lock(&status_mutex);
	listeningthreads_mask^=(1<<ithread);
#ifdef VERYDEBUG
	cprintf(BLUE,"%d Resetting mask of current listening thread. New Mask: 0x%x", ithread, listeningthreads_mask);
#endif
	pthread_mutex_unlock(&(status_mutex));

#ifdef VERYDEBUG
	cprintf(BLUE,"%d: Frames listened to %d\n",ithread, ((totalListeningFrameCount[ithread]*numListeningThreads)/packetsPerFrame));
#endif



	//waiting for all listening threads to be done, to print final count of frames listened to
	if(ithread == 0){
#ifdef VERYDEBUG
		if(numListeningThreads > 1)
			cprintf(BLUE,"%d Waiting for listening to be done.. current mask:0x%x\n", ithread, listeningthreads_mask);
#endif
		while(listeningthreads_mask)
			usleep(5000);
#ifdef VERYDEBUG
		t = 0;
		for(i=0;i<numListeningThreads;++i)
			t += totalListeningFrameCount[i];
		cprintf(BLUE,"%d Total frames listened to %d\n", ithread,(t/packetsPerFrame));
#endif
	}

}










void UDPStandardImplementation::stopWriting(int ithread, char* wbuffer[]){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	cprintf(GREEN,"%d End of Acquisition for Writing Thread\n",ithread);

	int i;
	//free fifo
	for(i=0;i<numListeningThreads;++i){
		while(!fifoFree[i]->push(wbuffer[i]));
#ifdef FIFO_DEBUG
		cprintf(GREEN,"%d writer free dummy pushed into fifofree %x for listener %d\n", ithread,(void*)(wbuffer[i]),i);
#endif
	}



	//all threads need to close file, reset mask and exit loop
	closeFile(ithread);
	pthread_mutex_lock(&status_mutex);
	writerthreads_mask^=(1<<ithread);
#ifdef VERYDEBUG
	cprintf(GREEN,"%d Resetting mask of current writing thread. New Mask: 0x%x\n", ithread,writerthreads_mask );
#endif
	pthread_mutex_unlock(&status_mutex);



	//only thread 0 needs to do this
	//check if all jobs are done and wait
	//change status to run finished
	if(ithread == 0){
		if(dataCompression){
			cprintf(GREEN,"%d Waiting for jobs to be done.. current mask:0x%x\n",ithread, writerthreads_mask);
			while(writerthreads_mask){
				/*cout << "." << flush;*/
				usleep(50000);
			}
			cprintf(GREEN," Jobs Done!\n");
		}
		//to make sure listening threads are done before you update status, as that returns to client
		while(listeningthreads_mask)
			usleep(5000);
		//update status
		pthread_mutex_lock(&status_mutex);
		status = RUN_FINISHED;
		pthread_mutex_unlock(&(status_mutex));
		//report

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











void UDPStandardImplementation::writeToFile_withoutCompression(char* buf[],int numpackets, uint32_t framenum){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	int packetsToSave, offset,lastpacket,i;
	uint32_t tempframenum = framenum;

	//file write
	if((enableFileWrite) && (sfilefd)){

		offset = HEADER_SIZE_NUM_TOT_PACKETS;

		while(numpackets > 0){

			//for progress and packet loss calculation(new files)
			if(myDetectorType == EIGER);
			else if ((myDetectorType == PROPIX)||((myDetectorType == GOTTHARD) && (shortFrame == -1)))
				tempframenum = (((((uint32_t)(*((uint32_t*)(buf[0] + HEADER_SIZE_NUM_TOT_PACKETS))))+1)& (frameIndexMask)) >> frameIndexOffset);
			else
				tempframenum = ((((uint32_t)(*((uint32_t*)(buf[0] + HEADER_SIZE_NUM_TOT_PACKETS))))& (frameIndexMask)) >> frameIndexOffset);

			if(numWriterThreads == 1)
				currframenum = tempframenum;
			else{
				if(tempframenum > currframenum)
					currframenum = tempframenum;
			}
#ifdef VERYDEBUG
			cout << "tempframenum:" << dec << tempframenum << " curframenum:" << currframenum << endl;
#endif

			//lock
			if(numWriterThreads > 1)
				pthread_mutex_lock(&write_mutex);


			//to create new file when max reached
			packetsToSave = maxPacketsPerFile - packetsInFile;
			if(packetsToSave > numpackets)
				packetsToSave = numpackets;
/**next time offset is still plus header length*/
			if(myDetectorType == EIGER){
				for(i=0;i<packetsToSave;++i)
					fwrite((void*)buf[i], 1, onePacketSize, sfilefd);
				//fwrite((void*)buf, 1, packetsToSave * onePacketSize, sfilefd);
			}else
				fwrite(buf[0]+offset, 1, packetsToSave * onePacketSize, sfilefd);
			packetsInFile += packetsToSave;
#ifdef EIGER_DEBUG3
			cprintf(GREEN,"packetscaught earlier:%d packetstosave:%d numMissingPackets:%d addingon:%d\n",
					packetsCaught,packetsToSave,numMissingPackets,(packetsToSave - numMissingPackets));
#endif
			packetsCaught += (packetsToSave - numMissingPackets);
			totalPacketsCaught += (packetsToSave - numMissingPackets);
			numMissingPackets = 0;
#ifdef EIGER_DEBUG3
			cprintf(GREEN,"packetscaught:%d\n", packetsCaught);
			cprintf(GREEN,"totalPacketsCaught:%d\n", totalPacketsCaught);
#endif
			//new file
			if(packetsInFile >= (uint32_t)maxPacketsPerFile){

				//for packet loss, because currframenum is the latest one for eiger
				if(myDetectorType != EIGER){
					lastpacket = (((packetsToSave - 1) * onePacketSize) + offset);

					if ((myDetectorType == PROPIX)||((myDetectorType == GOTTHARD) && (shortFrame == -1)))

						tempframenum = (((((uint32_t)(*((uint32_t*)(buf[0] + lastpacket))))+1)& (frameIndexMask)) >> frameIndexOffset);
					else
						tempframenum = ((((uint32_t)(*((uint32_t*)(buf[0] + lastpacket))))& (frameIndexMask)) >> frameIndexOffset);
				}
				if(numWriterThreads == 1)
					currframenum = tempframenum;
				else{
					if(tempframenum > currframenum)
						currframenum = tempframenum;
				}
#ifdef VERYDEBUG
				cout << "tempframenum:" << dec << tempframenum << " curframenum:" << currframenum << endl;
#endif
				//create
				createNewFile();
			}

			//unlock
			if(numWriterThreads > 1)
				pthread_mutex_unlock(&write_mutex);

			if(myDetectorType != EIGER)
				offset += (packetsToSave * onePacketSize);
			numpackets -= packetsToSave;
		}

	}
	else{
		if(numWriterThreads > 1)
			pthread_mutex_lock(&write_mutex);
		packetsInFile += numpackets;
		packetsCaught += (numpackets - numMissingPackets);
		totalPacketsCaught += (numpackets - numMissingPackets);
		numMissingPackets = 0;
		if(numWriterThreads > 1)
			pthread_mutex_unlock(&write_mutex);
	}
}








void UDPStandardImplementation::handleWithoutDataCompression(int ithread, char* wbuffer[],int npackets){
	int i, missingpacket,port = 0;


	if (cbAction < DO_EVERYTHING){
		if (myDetectorType == EIGER){
			for(i=0;i<npackets;++i)
				rawDataReadyCallBack(currframenum, wbuffer[i], onePacketSize, sfilefd, guiData,pRawDataReady);
		}else
			rawDataReadyCallBack(currframenum, wbuffer[0] + HEADER_SIZE_NUM_TOT_PACKETS, npackets * onePacketSize, sfilefd, guiData,pRawDataReady);
	}

	else {
		if (npackets > 0){

#ifdef WRITE_HEADERS
			if (myDetectorType == EIGER){

				eiger_packet_header_t* wbuf_header=0;
				eiger_packet_footer_t* wbuf_footer=0;


				for (i = 0; i < packetsPerFrame; i++){

					wbuf_header = (eiger_packet_header_t*) wbuffer[i];
					wbuf_footer = (eiger_packet_footer_t*)(wbuffer[i] + footer_offset);
#ifdef EIGER_DEBUG3
					cprintf(GREEN, "i:%d pnum:%d \n",i,*( (uint16_t*) wbuf_footer->packetnum));
#endif
					//which port
					if (i ==(packetsPerFrame/2))
						port = 1;



					//missing packet
					if (*( (uint16_t*) wbuf_header->missingpacket)== missingPacketValue){
#ifdef VERY_VERBOSE
						cprintf(GREEN,"missing packet at %d\n", i+1);
#endif
						missingpacket = 1;
						//add frame and packet numbers
						 *( (uint64_t*) wbuf_footer) = (uint64_t)((currframenum+1));
						 *( (uint16_t*) wbuf_footer->packetnum) = (i+1);
					}else{
						missingpacket = 0;

						if(*( (uint16_t*) wbuf_footer->packetnum)!= (i-(port*packetsPerFrame/numListeningThreads))+1){
							cprintf(BG_RED, "pnum mismatch num4! i:%d pnum:%d fnum:%d\n",
									i,*( (uint16_t*) wbuf_footer->packetnum),currframenum);
							exit(-1);
						}

							//move packet numbers to num2, and compensate for port1 starting pnum from 0
							if(port)
								*( (uint16_t*) wbuf_footer->packetnum) = (*( (uint16_t*) wbuf_footer->packetnum))+(packetsPerFrame/2);
					}

					if(*( (uint16_t*) wbuf_footer->packetnum) != (i+1)){
						cprintf(BG_RED, "pnum mismatch! i:%d pnum:%d fnum:%d\n",
								i,*( (uint16_t*) wbuf_footer->packetnum),currframenum);
						if (*( (uint16_t*) wbuf_header->missingpacket) == missingPacketValue)
							cprintf(BG_RED,"missing packet though\n");
						exit(-1);
					}

					//overwriting port number and dynamic range
					*( (uint8_t*) wbuf_header->portnum) = port;
					*( (uint8_t*) wbuf_header->dynamicrange) = dynamicRange;



#ifdef VERYDEBUG
					if((i==0)||(i==1)){
						cprintf(GREEN, "%d packet header:0x%016llx missingpacket:0x%x\n",i,
								(uint64_t)()*( (uint64_t*) wbuf_header)), *( (uint16_t*) wbuf_header->missingpacket));

						cprintf(GREEN, "%d - 0x%x - %d\n", i,
								 *( (uint16_t*) wbuf_header->missingpacket), *( (uint16_t*) wbuf_footer->packetnum));
					}
#endif

				}
			}
#endif

			writeToFile_withoutCompression(wbuffer, npackets,currframenum);
		}

#ifdef VERYDEBUG
		cprintf(GREEN,"written everyting\n");
#endif
	}


	if(myDetectorType == EIGER) {

#ifdef VERYDEBUG
		cprintf(GREEN,"gonna copy frame\n");
#endif
		copyFrameToGui(wbuffer);
#ifdef VERYDEBUG
		cprintf(GREEN,"copied frame\n");
#endif
	}else{
		//copy to gui
		if(npackets >= packetsPerFrame){//min 1 frame, but neednt be
			//if(npackets == packetsPerFrame * numJobsPerThread){ //only full frames
			copyFrameToGui(NULL,wbuffer[0]+HEADER_SIZE_NUM_TOT_PACKETS);
#ifdef VERYVERBOSE
			cout << ithread << " finished copying" << endl;
#endif
		}

		//else cout << "unfinished buffersize" << endl;
		while(!fifoFree[0]->push(wbuffer[0]));
#ifdef FIFO_DEBUG
		cprintf(GREEN,"%d writer freed pushed into fifofree %x for listener 0\n",ithread, (void*)(wbuffer[0]));
#endif

	}
}








void UDPStandardImplementation::handleDataCompression(int ithread, char* wbuffer[], char* data, int xmax, int ymax, int &nf){
	FILE_LOG(logDEBUG) << __AT__ << " called";

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

