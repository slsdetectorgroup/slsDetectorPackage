/********************************************//**
 * @file UDPStandardImplementation.cpp
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 ***********************************************/


#include "UDPStandardImplementation.h"

#include "moench02ModuleData.h"
#include "gotthardModuleData.h"
#include "gotthardShortModuleData.h"


#include <signal.h>  		// SIGINT
#include <sys/stat.h> 		// stat
#include <sys/socket.h>		// socket(), bind(), listen(), accept(), shut down
#include <arpa/inet.h>		// sock_addr_in, htonl, INADDR_ANY
#include <stdlib.h>			// exit()
#include <iomanip>			//set precision
#include <sys/mman.h>		//munmap



#include <string.h>
#include <iostream>

using namespace std;

#define EIGER_32BIT_INITIAL_CONSTANT 0x17c




UDPStandardImplementation::UDPStandardImplementation()
//:
//thread_started(0),
//eth(NULL),
//latestData(NULL),
//guiFileName(NULL),
//guiFrameNumber(0),
//tengigaEnable(0)
{
	
	thread_started = 0;
	eth = NULL;
	latestData = NULL;
	guiFileName = NULL;
	guiFrameNumber = NULL;
	tengigaEnable = 0;
	for(int i=0;i<MAX_NUM_LISTENING_THREADS;i++){
		udpSocket[i] = NULL;
		server_port[i] = DEFAULT_UDP_PORTNO+i;
		mem0[i] = NULL;
		fifo[i] = NULL;
		fifoFree[i] = NULL;
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


void UDPStandardImplementation::configure(map<string, string> config_map){
	FILE_LOG(logWARNING) << __AT__ << " called";

	map<string, string>::const_iterator pos;
	pos = config_map.find("mode");
	if (pos != config_map.end() ){
		int b;
		if(!sscanf(pos->second.c_str(), "%d", &b)){
			cout << "Warning: Could not parse mode. Assuming top mode." << endl;
			b = 0;
		}
		bottom = b!= 0;
		cout << "bottom:"<< bottom << endl;
	}
};

void UDPStandardImplementation::initializeMembers(){
	myDetectorType = GENERIC;
	maxPacketsPerFile = 0;
	enableFileWrite = 1;
	overwrite = 1;
	fileIndex = 0;
	scanTag = 0;
	frameIndexNeeded = 0;
	acqStarted = false;
	measurementStarted = false;
	startFrameIndex = 0;
	frameIndex = 0;
	packetsCaught = 0;
	totalPacketsCaught = 0;
	packetsInFile = 0;
	startAcquisitionIndex = 0;
	acquisitionIndex = 0;
	packetsPerFrame = 0;
	frameIndexMask = 0;
	packetIndexMask = 0;
	frameIndexOffset = 0;
	acquisitionPeriod = SAMPLE_TIME_IN_NS;
	numberOfFrames = 0;
	dynamicRange = 16;
	shortFrame = -1;
	currframenum = 0;
	prevframenum = 0;
	frameSize = 0;
	bufferSize = 0;
	onePacketSize = 0;
	guiDataReady = 0;
	nFrameToGui = 0;
	fifosize = 0;
	numJobsPerThread = -1;
	dataCompression = false;
	numListeningThreads = 1;
	numWriterThreads = 1;
	thread_started = 0;
	currentListeningThreadIndex = -1;
	currentWriterThreadIndex = -1;
	for(int i=0;i<numWriterThreads;i++)
		totalListeningFrameCount[i] = 0;
	listeningthreads_mask = 0x0;
	writerthreads_mask = 0x0;
	killAllListeningThreads = 0;
	killAllWritingThreads = 0;
	cbAction = DO_EVERYTHING;
	tengigaEnable = 0;

	for(int i=0;i<numListeningThreads;i++){
		udpSocket[i] = NULL;
		mem0[i] = NULL;
		fifo[i] = NULL;
		fifoFree[i] = NULL;
		buffer[i] = NULL;
	}
	eth = NULL;
	latestData = NULL;
	guiFileName = NULL;
	guiData = NULL;
	guiFrameNumber = 0;
	sfilefd = NULL;
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

	guiFileName = new char[MAX_STR_LENGTH];
	eth = new char[MAX_STR_LENGTH];
	strcpy(eth,"");
	strcpy(detHostname,"");
	strcpy(guiFileName,"");
	strcpy(savefilename,"");
	
	setFileName("run");
	setFilePath("");
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
	if(guiFileName) 	{delete [] guiFileName;	guiFileName = NULL;}
	for(int i=0;i<numListeningThreads;i++){
		if(mem0[i])			{free(mem0[i]);			mem0[i] = NULL;}
		if(fifo[i])			{delete fifo[i];		fifo[i] = NULL;}
		if(fifoFree[i])		{delete fifoFree[i];	fifoFree[i] = NULL;}
	}
}






int UDPStandardImplementation::setDetectorType(detectorType det){ 	FILE_LOG(logDEBUG) << __AT__ << " called";

	cout << "Setting Receiver Type " << endl;

	deleteMembers();
	initializeMembers();

	myDetectorType = det;

	switch(myDetectorType){
	case GOTTHARD:
		cout << endl << "***** This is a GOTTHARD Receiver *****" << endl << endl;
		break;
	case MOENCH:
		cout << endl << "***** This is a MOENCH Receiver *****" << endl << endl;
		break;
	case EIGER:
		cout << endl << "***** This is a EIGER Receiver *****" << endl << endl;
		break;
	default:
		cout << endl << "***** Unknown Receiver *****" << endl << endl;
	return FAIL;
		break;
	}

	//moench variables
	if(myDetectorType == GOTTHARD){
		fifosize 			= GOTTHARD_FIFO_SIZE;
		packetsPerFrame 	= GOTTHARD_PACKETS_PER_FRAME;
		onePacketSize		= GOTTHARD_ONE_PACKET_SIZE;
		frameSize			= GOTTHARD_BUFFER_SIZE;
		bufferSize 			= GOTTHARD_BUFFER_SIZE;
		maxPacketsPerFile 	= MAX_FRAMES_PER_FILE * GOTTHARD_PACKETS_PER_FRAME;
		frameIndexMask 		= GOTTHARD_FRAME_INDEX_MASK;
		frameIndexOffset 	= GOTTHARD_FRAME_INDEX_OFFSET;
		packetIndexMask 	= GOTTHARD_PACKET_INDEX_MASK;
	}else if(myDetectorType == MOENCH){
		fifosize 			= MOENCH_FIFO_SIZE;
		packetsPerFrame 	= MOENCH_PACKETS_PER_FRAME;
		onePacketSize		= MOENCH_ONE_PACKET_SIZE;
		frameSize			= MOENCH_BUFFER_SIZE;
		bufferSize 			= MOENCH_BUFFER_SIZE;
		maxPacketsPerFile 	= MOENCH_MAX_FRAMES_PER_FILE * MOENCH_PACKETS_PER_FRAME;
		frameIndexMask 		= MOENCH_FRAME_INDEX_MASK;
		frameIndexOffset 	= MOENCH_FRAME_INDEX_OFFSET;
		packetIndexMask 	= MOENCH_PACKET_INDEX_MASK;
	}
	else if(myDetectorType == EIGER){
		fifosize 			= EIGER_FIFO_SIZE;
		packetsPerFrame 	= EIGER_ONE_GIGA_CONSTANT * dynamicRange * EIGER_MAX_PORTS;
		onePacketSize		= EIGER_ONE_GIGA_ONE_PACKET_SIZE;
		frameSize			= onePacketSize * packetsPerFrame;
		bufferSize 			= (frameSize/EIGER_MAX_PORTS) + EIGER_HEADER_LENGTH;//everything one port gets (img header plus packets)
		maxPacketsPerFile 	= EIGER_MAX_FRAMES_PER_FILE * packetsPerFrame;
		frameIndexMask 		= EIGER_FRAME_INDEX_MASK;
		frameIndexOffset 	= EIGER_FRAME_INDEX_OFFSET;
		packetIndexMask 	= EIGER_PACKET_INDEX_MASK;

		pthread_mutex_lock(&status_mutex);
		listeningthreads_mask = 0x0;
		pthread_mutex_unlock(&(status_mutex));
		if(thread_started)
			createListeningThreads(true);

		numListeningThreads = MAX_NUM_LISTENING_THREADS;
	}
	latestData = new char[frameSize];


	setupFifoStructure();

	if(createListeningThreads() == FAIL){
		cprintf(BG_RED,"ERROR: Could not create listening thread\n");
		exit (-1);
	}

	if(createWriterThreads() == FAIL){
		cprintf(BG_RED,"ERROR: Could not create writer threads\n");
		exit (-1);
	}

	setThreadPriorities();

	cout << "Ready..." << endl;

	return OK;
}





/*Frame indices and numbers caught*/

//bool UDPStandardImplementation::getAcquistionStarted(){return acqStarted;};

//bool UDPStandardImplementation::getMeasurementStarted(){return measurementStarted;};

//int UDPStandardImplementation::getFramesCaught(){return (packetsCaught/packetsPerFrame);}

//int UDPStandardImplementation::getTotalFramesCaught(){return (totalPacketsCaught/packetsPerFrame);}

//uint32_t UDPStandardImplementation::getStartAcquisitionIndex(){return startAcquisitionIndex;}

//uint32_t UDPStandardImplementation::getStartFrameIndex(){return startFrameIndex;}

/*
uint32_t UDPStandardImplementation::getFrameIndex(){
	if(!packetsCaught)
		frameIndex=-1;
	else
		frameIndex = currframenum - startFrameIndex;
	return frameIndex;
}
*/

/*
uint32_t UDPStandardImplementation::getAcquisitionIndex(){
	if(!totalPacketsCaught)
		acquisitionIndex=-1;
	else
		acquisitionIndex = currframenum - startAcquisitionIndex;
	return acquisitionIndex;
}
*/

/*
void UDPStandardImplementation::resetTotalFramesCaught(){
	acqStarted = false;
	startAcquisitionIndex = 0;
	totalPacketsCaught = 0;
}
*/








/*file parameters*/
/*
char* UDPStandardImplementation::getFilePath() const{
		return (char*)filePath;
}
*/
/*
char* UDPStandardImplementation::setFilePath(const char c[]){
	if(strlen(c)){
		//check if filepath exists
		struct stat st;
		if(stat(c,&st) == 0)
			strcpy(filePath,c);
		else{
			strcpy(filePath,"");
			cout << "FilePath does not exist:" << filePath << endl;
		}
	}
	return getFilePath();
}
*/

/*
char* UDPStandardImplementation::getFileName() const{
	return (char*)fileName;
}
*/

/*
char* UDPStandardImplementation::setFileName(const char c[]){
	if(strlen(c))
		strcpy(fileName,c);
	return getFileName();
}
*/

/*
int UDPStandardImplementation::getFileIndex(){
	return fileIndex;
}
*/

/*
int UDPStandardImplementation::setFileIndex(int i){
	if(i>=0)
		fileIndex = i;
	return getFileIndex();
}
*/

/*
int UDPStandardImplementation::setFrameIndexNeeded(int i){
	frameIndexNeeded = i;
	return frameIndexNeeded;
}
*/

/*
int UDPStandardImplementation::getEnableFileWrite()  const{
	return enableFileWrite;
}
*/

/*
int UDPStandardImplementation::setEnableFileWrite(int i){
	enableFileWrite=i;
	return getEnableFileWrite();
}
*/

/*
int UDPStandardImplementation::getEnableOverwrite()  const{
	return overwrite;
}
*/

/*
int UDPStandardImplementation::setEnableOverwrite(int i){
	overwrite=i;
	return getEnableOverwrite();
}
*/




/*other parameters*/

slsReceiverDefs::runStatus UDPStandardImplementation::getStatus() const{
	FILE_LOG(logDEBUG) << __AT__ << " called, status: " << status;

	
	return status;
}


void UDPStandardImplementation::initialize(const char *detectorHostName){
	if(strlen(detectorHostName))
		strcpy(detHostname,detectorHostName);
}


char *UDPStandardImplementation::getDetectorHostname() const{
	return (char*)detHostname;
}

void UDPStandardImplementation::setEthernetInterface(char* c){ 	FILE_LOG(logDEBUG) << __AT__ << " called";

	strcpy(eth,c);
}


void UDPStandardImplementation::setUDPPortNo(int p){
FILE_LOG(logDEBUG) << __AT__ << " called";
	server_port[0] = p;
}


void UDPStandardImplementation::setUDPPortNo2(int p){
FILE_LOG(logDEBUG) << __AT__ << " called";
	server_port[1] = p;
}


int UDPStandardImplementation::getNumberOfFrames() const {
	return numberOfFrames;
}


int32_t UDPStandardImplementation::setNumberOfFrames(int32_t fnum){ 	FILE_LOG(logDEBUG) << __AT__ << " called";

	if(fnum >= 0)
		numberOfFrames = fnum;

	return getNumberOfFrames();
}

int UDPStandardImplementation::getScanTag() const{
	return scanTag;
}


int32_t UDPStandardImplementation::setScanTag(int32_t stag){ 	FILE_LOG(logDEBUG) << __AT__ << " called";

	if(stag >= 0)
		scanTag = stag;

	return getScanTag();
}


int UDPStandardImplementation::getDynamicRange() const{
	return dynamicRange;
}

int32_t UDPStandardImplementation::setDynamicRange(int32_t dr){ 	FILE_LOG(logDEBUG) << __AT__ << " called";

	cout << "Setting Dynamic Range" << endl;

	int olddr = dynamicRange;
	if(dr >= 0){
		dynamicRange = dr;

		if(myDetectorType == EIGER){


			if(!tengigaEnable)
				packetsPerFrame 	= EIGER_ONE_GIGA_CONSTANT * dynamicRange * EIGER_MAX_PORTS;
			else
				packetsPerFrame 	= EIGER_TEN_GIGA_CONSTANT * dynamicRange * EIGER_MAX_PORTS;
			frameSize			= onePacketSize * packetsPerFrame;
			bufferSize 			= (frameSize/EIGER_MAX_PORTS) + EIGER_HEADER_LENGTH;//everything one port gets (img header plus packets)
			maxPacketsPerFile 	= EIGER_MAX_FRAMES_PER_FILE * packetsPerFrame;



			if(olddr != dr){

				//del
				if(thread_started){
					createListeningThreads(true);
					createWriterThreads(true);
				}
				for(int i=0;i<numListeningThreads;i++){
					if(mem0[i])			{free(mem0[i]);			mem0[i] = NULL;}
					if(fifo[i])			{delete fifo[i];		fifo[i] = NULL;}
					if(fifoFree[i])		{delete fifoFree[i];	fifoFree[i] = NULL;}
					buffer[i] = NULL;
				}
				if(latestData) 		{delete [] latestData;	latestData = NULL;}
				latestData = new char[frameSize];

				numJobsPerThread = -1;
				setupFifoStructure();

				if(createListeningThreads() == FAIL){
					cprintf(BG_RED,"ERROR: Could not create listening thread\n");
					exit (-1);
				}

				if(createWriterThreads() == FAIL){
					cprintf(BG_RED,"ERROR: Could not create writer threads\n");
					exit (-1);
				}

				setThreadPriorities();
			}
		}

	}

	return getDynamicRange();
}



int UDPStandardImplementation::setShortFrame(int i){ 	FILE_LOG(logDEBUG) << __AT__ << " called";

	shortFrame=i;

	if(shortFrame!=-1){
		bufferSize = GOTTHARD_SHORT_ONE_PACKET_SIZE;
		frameSize = GOTTHARD_SHORT_BUFFER_SIZE;
		maxPacketsPerFile = SHORT_MAX_FRAMES_PER_FILE * GOTTHARD_SHORT_PACKETS_PER_FRAME;
		packetsPerFrame = GOTTHARD_SHORT_PACKETS_PER_FRAME;
		frameIndexMask = GOTTHARD_SHORT_FRAME_INDEX_MASK;
		frameIndexOffset = GOTTHARD_SHORT_FRAME_INDEX_OFFSET;

	}else{
		onePacketSize = GOTTHARD_ONE_PACKET_SIZE;
		bufferSize = GOTTHARD_BUFFER_SIZE;
		frameSize = GOTTHARD_BUFFER_SIZE;
		maxPacketsPerFile = MAX_FRAMES_PER_FILE * GOTTHARD_PACKETS_PER_FRAME;
		packetsPerFrame = GOTTHARD_PACKETS_PER_FRAME;
		frameIndexMask = GOTTHARD_FRAME_INDEX_MASK;
		frameIndexOffset = GOTTHARD_FRAME_INDEX_OFFSET;
	}


	deleteFilter();
	if(dataCompression)
		setupFilter();

	return shortFrame;
}


int UDPStandardImplementation::setNFrameToGui(int i){ 	FILE_LOG(logDEBUG) << __AT__ << " called";

	if(i>=0){
		nFrameToGui = i;
		setupFifoStructure();
	}
	return nFrameToGui;
}



int64_t UDPStandardImplementation::setAcquisitionPeriod(int64_t index){ 	FILE_LOG(logDEBUG) << __AT__ << " called";


	if(index >= 0){
		if(index != acquisitionPeriod){
			acquisitionPeriod = index;
			setupFifoStructure();
		}
	}
	return acquisitionPeriod;
}


bool UDPStandardImplementation::getDataCompression(){ 	FILE_LOG(logDEBUG) << __AT__ << " called";
return dataCompression;}

int UDPStandardImplementation::enableDataCompression(bool enable){ 	FILE_LOG(logDEBUG) << __AT__ << " called";

	cout << "Data compression ";
	if(enable)
		cout << "enabled" << endl;
	else
		cout << "disabled" << endl;
#ifdef MYROOT1
	cout << " WITH ROOT" << endl;
#else
	cout << " WITHOUT ROOT" << endl;
#endif
	//delete filter for the current number of threads
	deleteFilter();

	dataCompression = enable;
	pthread_mutex_lock(&status_mutex);
	writerthreads_mask = 0x0;
	pthread_mutex_unlock(&(status_mutex));

	createWriterThreads(true);

	if(enable)
		numWriterThreads = MAX_NUM_WRITER_THREADS;
	else
		numWriterThreads = 1;

	if(createWriterThreads() == FAIL){
		cprintf(BG_RED,"ERROR: Could not create writer threads\n");
		return FAIL;
	}
	setThreadPriorities();


	if(enable)
		setupFilter();

	return OK;
}












/*other functions*/


void UDPStandardImplementation::deleteFilter(){ 	FILE_LOG(logDEBUG) << __AT__ << " called";

	int i;
	cmSub=NULL;

	for(i=0;i<numWriterThreads;i++){
		if(singlePhotonDet[i]){
			delete singlePhotonDet[i];
			singlePhotonDet[i] = NULL;
		}
		if(receiverdata[i]){
			delete receiverdata[i];
			receiverdata[i] = NULL;
		}
	}
}


void UDPStandardImplementation::setupFilter(){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	double hc = 0;
	double sigma = 5;
	int sign = 1;
	int csize;
	int i;

	if (commonModeSubtractionEnable)
		cmSub=new moenchCommonMode();

	switch(myDetectorType){
	case MOENCH:
		csize = 3;
		for(i=0;i<numWriterThreads;i++)
			receiverdata[i]=new moench02ModuleData(hc);
		break;
	default:
		csize = 1;
		if(shortFrame == -1){
			for(i=0;i<numWriterThreads;i++)
				receiverdata[i]=new gotthardModuleData(hc);
		}else{
			for(i=0;i<numWriterThreads;i++)
				receiverdata[i]=new gotthardShortModuleData(hc);
		}
		break;
	}

	for(i=0;i<numWriterThreads;i++)
		singlePhotonDet[i]=new singlePhotonDetector<uint16_t>(receiverdata[i], csize, sigma, sign, cmSub);

}



//LEO: it is not clear to me..
void UDPStandardImplementation::setupFifoStructure(){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	int64_t i;
	int oldn = numJobsPerThread;

	//if every nth frame mode
	if(nFrameToGui)
		numJobsPerThread = nFrameToGui;

	//random nth frame mode
	else{
		if(!acquisitionPeriod)
			i = SAMPLE_TIME_IN_NS;
		else
			i = SAMPLE_TIME_IN_NS/acquisitionPeriod;
		if (i > MAX_JOBS_PER_THREAD)
			numJobsPerThread = MAX_JOBS_PER_THREAD;
		else if (i < 1)
			numJobsPerThread = 1;
		else
			numJobsPerThread = i;
	}

	//if same, return
	if(oldn == numJobsPerThread)
		return;

	if(myDetectorType == EIGER)
		numJobsPerThread = 1;

	//otherwise memory too much if numjobsperthread is at max = 1000
	fifosize = GOTTHARD_FIFO_SIZE;
	if(myDetectorType == MOENCH)
		fifosize = MOENCH_FIFO_SIZE;
	else if(myDetectorType == EIGER)
		fifosize = EIGER_FIFO_SIZE;

	if(fifosize % numJobsPerThread)
		fifosize = (fifosize/numJobsPerThread)+1;
	else
		fifosize = fifosize/numJobsPerThread;


	cout << "Number of Frames per buffer:" << numJobsPerThread << endl;
	cout << "Fifo Size:" << fifosize << endl;

	/*
	//for testing
	 numJobsPerThread = 3; fifosize = 11;
	 */

	for(int i=0;i<numListeningThreads;i++){
		//deleting old structure and creating fifo structure
		if(fifoFree[i]){
			while(!fifoFree[i]->isEmpty())
				fifoFree[i]->pop(buffer[i]);
#ifdef FIFO_DEBUG
			//cprintf(GREEN,"%d fifostructure popped from fifofree %x\n", i, (void*)(buffer[i]));
#endif
			delete fifoFree[i];
		}
		if(fifo[i])	delete fifo[i];
		if(mem0[i]) 	free(mem0[i]);
		fifoFree[i] 	= new CircularFifo<char>(fifosize);
		fifo[i] 		= new CircularFifo<char>(fifosize);


		//allocate memory
		mem0[i]=(char*)malloc((bufferSize * numJobsPerThread + HEADER_SIZE_NUM_TOT_PACKETS)*fifosize);
		/** shud let the client know about this */
		if (mem0[i]==NULL){
			cout<<"++++++++++++++++++++++ COULD NOT ALLOCATE MEMORY FOR LISTENING !!!!!!!+++++++++++++++++++++" << endl;
			exit(-1);
		}
		buffer[i]=mem0[i];
		//push the addresses into freed fifoFree and writingFifoFree
		while (buffer[i]<(mem0[i]+(bufferSize * numJobsPerThread + HEADER_SIZE_NUM_TOT_PACKETS)*(fifosize-1))) {
			fifoFree[i]->push(buffer[i]);
#ifdef FIFO_DEBUG
			cprintf(BLUE,"%d fifostructure free pushed into fifofree %x\n", i, (void*)(buffer[i]));
#endif
			buffer[i]+=(bufferSize * numJobsPerThread + HEADER_SIZE_NUM_TOT_PACKETS);
		}
	}
	cout << "Fifo structure(s) reconstructed" << endl;
}







/** acquisition functions */
void UDPStandardImplementation::readFrame(char* c,char** raw, uint32_t &fnum, uint32_t &startAcquisitionIndex, uint32_t &startFrameIndex){
	FILE_LOG(logDEBUG) << __AT__ << " called";
	//point to gui data
	if (guiData == NULL){
		guiData = latestData;
#ifdef VERY_VERY_DEBUG
		cout << "gui data not null anymore" << endl;
#endif
	}

	//copy data and filename
	strcpy(c,guiFileName);
	fnum = guiFrameNumber;
	startAcquisitionIndex = getStartAcquisitionIndex();
	startFrameIndex = getStartFrameIndex();


	//could not get gui data
	if(!guiDataReady){
#ifdef VERY_VERY_DEBUG
		cout << "gui data not ready" << endl;
#endif
		*raw = NULL;
	}
	//data ready, set guidata to receive new data
	else{
#ifdef VERY_VERY_DEBUG
		cout << "gui data ready" << endl;
#endif
		*raw = guiData;
		guiData = NULL;

		/*pthread_mutex_lock(&dataReadyMutex); WHY WAS THIS HERE IN THE FIRST PLACE
		guiDataReady = 0;
		pthread_mutex_unlock(&dataReadyMutex);*/
		if((nFrameToGui) && (writerthreads_mask)){
#ifdef VERY_VERY_DEBUG
			cout << "gonna post" << endl;
#endif
		/*if(nFrameToGui){*/
			//release after getting data
			sem_post(&smp);
		}
#ifdef VERY_VERY_DEBUG
		cout << "done post" << endl;
#endif
	}
}





void UDPStandardImplementation::copyFrameToGui(char* startbuf[], uint32_t fnum, char* buf){
	FILE_LOG(logDEBUG) << __AT__ << " called";
#ifdef VERY_VERY_DEBUG
cout << "copyframe" << endl;
#endif

	//random read when gui not ready , also command line doesnt have nthframetogui
	//else guidata always null as guidataready is always 1 after 1st frame, and seccond data never gets copied
	if((!nFrameToGui) && (!guiData)){
#ifdef VERY_VERY_DEBUG
		cout << "doing nothing" << endl;
#endif
		pthread_mutex_lock(&dataReadyMutex);
		guiDataReady=0;
		pthread_mutex_unlock(&dataReadyMutex);
	}

	//random read or nth frame read, gui needs data now or it is the first frame
	else{
#ifdef VERY_VERY_DEBUG
		cout << "gui needs data now or 1st frame" << endl;
#endif
		pthread_mutex_lock(&dataReadyMutex);
		guiDataReady=0;
#ifdef VERY_VERY_DEBUG
		cout << "guidataready is  0, copying data" << endl;
#endif
		//eiger
		if(startbuf != NULL){
			int offset = 0;
			int size = frameSize/EIGER_MAX_PORTS;
			for(int j=0;j<numListeningThreads;++j){
				memcpy((((char*)latestData)+offset) ,startbuf[j] + (HEADER_SIZE_NUM_TOT_PACKETS + EIGER_HEADER_LENGTH),size);
				offset += size;
			}

			/*
			for(int j=25;j<27;++j)
			for(int i=1000;i<1010;i=i+2)
				//cout<<"startbuf:"<<dec<<i<<hex<<":\t0x"<<htonl((uint32_t)(*((uint32_t*)(startbuf[1] + HEADER_SIZE_NUM_TOT_PACKETS+ EIGER_HEADER_LENGTH+8+ i))))<<endl;
			cout<<"startbuf:"<<dec<<i<<hex<<":\t0x"<<((uint16_t)(*((uint16_t*)(startbuf[1] + 2+ 48+ j*1040+8+ i))))<<endl;
			 */

			guiFrameNumber = fnum;
		}else//other detectors
			memcpy(latestData,buf,bufferSize);


		strcpy(guiFileName,savefilename);
		guiDataReady=1;
		pthread_mutex_unlock(&dataReadyMutex);
#ifdef VERY_VERY_DEBUG
		cout << "guidataready = 1" << endl;
#endif
		//nth frame read, block current process if the guireader hasnt read it yet
		if(nFrameToGui){
#ifdef VERY_VERY_DEBUG
			cout<<"waiting after copying"<<endl;
#endif
			sem_wait(&smp);
#ifdef VERY_VERY_DEBUG
			cout<<"done waiting"<<endl;
#endif
		}

	}
}





int UDPStandardImplementation::createUDPSockets(){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	int port[2];
	port[0] = server_port[0];
	port[1] = server_port[1];

	/** eiger specific */

	if(bottom){
		port[0] = server_port[1];
		port[1] = server_port[0];
	}

	//if eth is mistaken with ip address
	if (strchr(eth,'.')!=NULL)
		strcpy(eth,"");

	shutDownUDPSockets();

	//if no eth, listen to all
	if(!strlen(eth)){
		cout<<"warning:eth is empty.listening to all"<<endl;

		for(int i=0;i<numListeningThreads;i++)
			udpSocket[i] = new genericSocket(port[i],genericSocket::UDP,bufferSize);
	}
	//normal socket
	else{
		cout<<"eth:"<<eth<<endl;

		for(int i=0;i<numListeningThreads;i++)
			udpSocket[i] = new genericSocket(port[i],genericSocket::UDP,bufferSize,eth);
	}

	//error
	int iret;
	for(int i=0;i<numListeningThreads;i++){
		iret = udpSocket[i]->getErrorStatus();
		if(!iret)
			cout << "UDP port opened at port " << port[i] << endl;
		else{
#ifdef VERBOSE
			cprintf(BG_RED,"Could not create UDP socket on port %d error: %d\n", port[i], iret);
#endif
			return FAIL;
		}
	}

	return OK;
}







int UDPStandardImplementation::shutDownUDPSockets(){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	for(int i=0;i<numListeningThreads;i++){
		if(udpSocket[i]){
			udpSocket[i]->ShutDownSocket();
			delete udpSocket[i];
			udpSocket[i] = NULL;
		}
	}
	return OK;
}




// TODO: add a destroyListeningThreads
int UDPStandardImplementation::createListeningThreads(bool destroy){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	int i;
	void* status;

	killAllListeningThreads = 0;

	pthread_mutex_lock(&status_mutex);
	listeningthreads_mask = 0x0;
	pthread_mutex_unlock(&(status_mutex));

	FILE_LOG(logDEBUG) << "Starting " << __func__ << endl;

	if(!destroy){

		//start listening threads
		cout << "Creating Listening Threads(s)";

		currentListeningThreadIndex = -1;

		for(i = 0; i < numListeningThreads; ++i){
			sem_init(&listensmp[i],1,0);
			thread_started = 0;
			currentListeningThreadIndex = i;
			if(pthread_create(&listening_thread[i], NULL,startListeningThread, (void*) this)){
				cout << "Could not create listening thread with index " << i << endl;
				return FAIL;
			}
			while(!thread_started);
			cout << ".";
			cout << flush;
		}
#ifdef VERBOSE
		cout << "Listening thread(s) created successfully." << endl;
#else
		cout << endl;
#endif
	}else{
		cout<<"Destroying Listening Thread(s)"<<endl;
		killAllListeningThreads = 1;
		for(i = 0; i < numListeningThreads; ++i){
			sem_post(&listensmp[i]);
			pthread_join(listening_thread[i], &status);
			cout <<"."<<flush;
		}
		killAllListeningThreads = 0;
		thread_started = 0;
		cout << "Listening thread(s) destroyed" << endl;
	}

	return OK;
}






int UDPStandardImplementation::createWriterThreads(bool destroy){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	int i;
	void* status;

	killAllWritingThreads = 0;

	pthread_mutex_lock(&status_mutex);
	writerthreads_mask = 0x0;
	createfile_mask = 0x0;
	pthread_mutex_unlock(&(status_mutex));


	if(!destroy){

		//start writer threads
		cout << "Creating Writer Thread(s)";

		currentWriterThreadIndex = -1;

		for(i = 0; i < numWriterThreads; ++i){
			sem_init(&writersmp[i],1,0);
			thread_started = 0;
			currentWriterThreadIndex = i;
			if(pthread_create(&writing_thread[i], NULL,startWritingThread, (void*) this)){
				cout << "Could not create writer thread with index " << i << endl;
				return FAIL;
			}
			while(!thread_started);
			cout << ".";
			cout << flush;
		}
#ifdef VERBOSE
		cout << endl << "Writer thread(s) created successfully." << endl;
#else
		cout << endl;
#endif

	}else{
		cout << "Destroying Writer Thread(s)" << endl;
		killAllWritingThreads = 1;
		for(i = 0; i < numWriterThreads; ++i){
			sem_post(&writersmp[i]);
			pthread_join(writing_thread[i],&status);
			cout <<"."<<flush;
		}
		killAllWritingThreads = 0;
		thread_started = 0;
		cout << endl << "Writer thread(s) destroyed" << endl;
	}

	return OK;
}









void UDPStandardImplementation::setThreadPriorities(){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	int i;
	//assign priorities
	struct sched_param tcp_param, listen_param, write_param;
	int policy= SCHED_RR;
	bool rights = true;

	tcp_param.sched_priority = 50;
	listen_param.sched_priority = 99;
	write_param.sched_priority = 90;

	for(i = 0; i < numListeningThreads; ++i){
		if (pthread_setschedparam(listening_thread[i], policy, &listen_param) == EPERM){
			rights = false;
			break;
		}
	}
	for(i = 0; i < numWriterThreads; ++i){
		if(rights)
			if (pthread_setschedparam(writing_thread[i], policy, &write_param) == EPERM){
				rights = false;
				break;
			}
	}
	if (pthread_setschedparam(pthread_self(),5 , &tcp_param) == EPERM)
		rights = false;

	if(!rights)
		cout << "WARNING: Could not prioritize threads. You need to be super user for that." << endl;

}






int UDPStandardImplementation::setupWriter(){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	//reset writing thread variables
	packetsInFile=0;
	packetsCaught=0;
	frameIndex=0;
	if(sfilefd) sfilefd=NULL;
	guiData = NULL;
	guiDataReady=0;
	strcpy(guiFileName,"");
	guiFrameNumber = 0;
	cbAction = DO_EVERYTHING;

	pthread_mutex_lock(&status_mutex);
	writerthreads_mask = 0x0;
	createfile_mask = 0x0;
	ret_createfile = OK;
	pthread_mutex_unlock(&status_mutex);

	//printouts
	cout << "Max Packets Per File:" << maxPacketsPerFile << endl;
	if (rawDataReadyCallBack)
		cout << "Note: Data Write has been defined exernally" << endl;
	if (dataCompression)
		cout << "Data Compression is enabled with " << numJobsPerThread << " number of jobs per thread" << endl;
	if(nFrameToGui)
		cout << "Sending every " << nFrameToGui << "th frame to gui" <<  endl;



	//acquisition start call back returns enable write
	if (startAcquisitionCallBack)
		cbAction=startAcquisitionCallBack(filePath,fileName,fileIndex,bufferSize,pStartAcquisition);

	if(cbAction < DO_EVERYTHING)
		cout << endl << "Note: Call back activated. Data saving must be taken care of by user in call back." << endl;
	else if(enableFileWrite==0)
		cout << endl << "Note: Data will not be saved" << endl;



	//creating first file

	//mask
	pthread_mutex_lock(&status_mutex);
	for(int i=0;i<numWriterThreads;i++)
		createfile_mask|=(1<<i);
	pthread_mutex_unlock(&status_mutex);

	for(int i=0;i<numWriterThreads;i++){
#ifdef VERYDEBUG
		cout << i << " gonna post 1st sem" << endl;
#endif
		sem_post(&writersmp[i]);
	}
	//wait till its created
	while(createfile_mask){
		//cout<<"*"<<flush;
		usleep(5000);
	}
	if (createfile_mask)
		cout <<"*********************************************sooo weird:"<<createfile_mask<<endl;


	if(dataCompression){
#if (defined(MYROOT1) && defined(ALLFILE_DEBUG)) || !defined(MYROOT1)
		if(ret_createfile != FAIL){
			int ret = createNewFile();
			if(ret == FAIL)
				ret_createfile = FAIL;
		}
#endif
	}

	return ret_createfile;

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
	else
		sprintf(savefilename, "%s/%s_f%012d_%d.raw", filePath,fileName,(packetsCaught/packetsPerFrame),fileIndex);

#ifdef VERBOSE
	cout << filePath << " + " << fileName << endl;
#endif

	//if filewrite and we are allowed to write
	if(enableFileWrite && cbAction > DO_NOTHING){
		//close
		if(sfilefd){
			fclose(sfilefd);
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
					(int)((((currframenum-prevframenum)-(packetsInFile/packetsPerFrame))/(double)(currframenum-prevframenum))*100.000)
					<< "%\tframenum "
					<< dec << currframenum //<< "\t\t p " << prevframenum
					<< "\tindex " << dec << gt
					<< "\tlost " << dec << (((int)(currframenum-prevframenum))-(packetsInFile/packetsPerFrame)) << endl;

		}
	}

	//reset counters for each new file
	if(packetsCaught){
		prevframenum = currframenum;
		packetsInFile = 0;
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
			cout << "sfield:" << (int)sfilefd << endl;
#endif
			fclose(sfilefd);
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
			fclose(sfilefd);
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
 * Pre:
 * Post: eiger req. time for 32bit before acq start
 * */

int UDPStandardImplementation::startReceiver(char message[]){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	int i;


// #ifdef VERBOSE
	cout << "Starting Receiver" << endl;
//#endif


	//reset listening thread variables
	measurementStarted = false;
	//should be set to zero as its added to get next start frame indices for scans for eiger
	if(!acqStarted)		currframenum = 0;
	startFrameIndex = 0;

	for(int i = 0; i < numListeningThreads; ++i)
		totalListeningFrameCount[i] = 0;

	//udp socket
	if(createUDPSockets() == FAIL){
		strcpy(message,"Could not create UDP Socket(s).\n");
		cout << endl << message << endl;
		return FAIL;
	}
	cout << "UDP socket(s) created successfully." << endl;


	if(setupWriter() == FAIL){
		//stop udp socket
		shutDownUDPSockets();

		sprintf(message,"Could not create file %s.\n",savefilename);
		return FAIL;
	}
	cout << "Successfully created file(s)" << endl;

	//done to give the gui some proper name instead of always the last file name
	if(dataCompression)
		sprintf(savefilename, "%s/%s_fxxx_%d_xx.root", filePath,fileName,fileIndex);

	//initialize semaphore
	sem_init(&smp,1,0);

	//status
	pthread_mutex_lock(&status_mutex);
	status = RUNNING;
	for(i=0;i<numListeningThreads;i++)
		listeningthreads_mask|=(1<<i);
	for(i=0;i<numWriterThreads;i++)
		writerthreads_mask|=(1<<i);
	pthread_mutex_unlock(&(status_mutex));


	//start listening /writing
	for(i=0;i<numListeningThreads;i++)
		sem_post(&listensmp[i]);
	for(i=0; i < numWriterThreads; ++i)
		sem_post(&writersmp[i]);

	//usleep(5000000);
	cout << "Receiver Started.\nStatus:" << status << endl;

	return OK;
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

		while(status == TRANSMITTING)
			usleep(5000);

		//semaphore destroy
		sem_post(&smp);
		sem_destroy(&smp);

		//change status
		pthread_mutex_lock(&status_mutex);
		status = IDLE;
		pthread_mutex_unlock(&(status_mutex));

		cout << "Receiver Stopped.\nStatus:" << status << endl << endl;
	}else cout <<" Not idle to stop receiver" << endl;

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
	cout << "In startListening() " << endl;
#endif

	thread_started = 1;

	int total;
	int lastpacketoffset, expected, rc,packetcount, maxBufferSize, carryonBufferSize;
	uint32_t lastframeheader;// for moench to check for all the packets in last frame
	char* tempchar = NULL;
	int imageheader = 0;
	if(myDetectorType==EIGER)
		imageheader = EIGER_IMAGE_HEADER_SIZE;


	while(1){
		//variables that need to be checked/set before each acquisition
		carryonBufferSize = 0;
		//if more than 1 listening thread, listen one packet at a time, else need to interleaved frame later
		maxBufferSize = bufferSize * numJobsPerThread;
#ifdef VERYDEBUG
		cout << " maxBufferSize:" << maxBufferSize << ",carryonBufferSize:" << carryonBufferSize << endl;
#endif

		if(tempchar) {delete [] tempchar;tempchar = NULL;}
		if(myDetectorType != EIGER)
			tempchar = new char[onePacketSize * ((packetsPerFrame/numListeningThreads) - 1)]; //gotthard: 1packet size, moench:39 packet size


		while((1<<ithread)&listeningthreads_mask){

#ifdef VERYDEBUG
			cout << ithread << " ***waiting to pop out of listeningfifo" << endl;
#endif
			//pop
			fifoFree[ithread]->pop(buffer[ithread]);
#ifdef FIFO_DEBUG
			cprintf(GREEN,"%d listener popped from fifofree %x\n", ithread, (void*)(buffer[ithread]));
#endif


			//receive
			if(udpSocket[ithread] == NULL){
				rc = 0;
				cout << ithread << "UDP Socket is NULL" << endl;
			}
			//normal listening
			else if(!carryonBufferSize){

			/*	if(!ithread){*/
				rc = udpSocket[ithread]->ReceiveDataOnly(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS, maxBufferSize);
				//cout<<"value:"<<htonl(*(unsigned int*)((eiger_image_header *)((char*)(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS)))->fnum)<<endl;
				expected = maxBufferSize;
				/*}else{
					while(1) usleep(100000000);
				}
*/
			}
			//the remaining packets from previous buffer
			else{
#ifdef VERYDEBUG
				cout <<  ithread << " ***carry on buffer" << carryonBufferSize << endl;
				cout <<  ithread << " framennum in temochar:"<<((((uint32_t)(*((uint32_t*)tempchar)))
						& (frameIndexMask)) >> frameIndexOffset)<<endl;
				cout <<  ithread << " temochar packet:"<< ((((uint32_t)(*((uint32_t*)(tempchar)))))
											& (packetIndexMask)) << endl;
#endif
				//if there is a packet from previous buffer, copy it and listen to n less frame
				memcpy(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS, tempchar, carryonBufferSize);
				rc = udpSocket[ithread]->ReceiveDataOnly((buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS + carryonBufferSize),maxBufferSize - carryonBufferSize);
				expected = maxBufferSize - carryonBufferSize;
			}

#ifdef EIGER_DEBUG
			cout << ithread << " *** rc:" << dec << rc << ". expected:" << dec << expected << endl;
#endif

/*
			//start indices for each start of scan/acquisition - eiger does it before
				if((!measurementStarted) && (rc > 0) && (!ithread))
					startFrameIndices(ithread);
*/

			//problem in receiving or end of acquisition
			if((rc < expected)||(rc <= 0)){
				if(myDetectorType != EIGER){
					//start indices for each start of scan/acquisition - this should be done earlier for normal detectors
					if((!measurementStarted) && (rc > 0) && (!ithread))
						startFrameIndices(ithread);
				}
				stopListening(ithread,rc,packetcount,total);
				continue;
			}

///*
			//eiger - start indices for each start of scan/acquisition - this should be done after to ignore first incomplete frames
			if((!measurementStarted) && (rc > 0) && (!ithread))
				startFrameIndices(ithread);
//*/

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
				if(shortFrame == -1){
					lastpacketoffset = (((numJobsPerThread * packetsPerFrame - 1) * onePacketSize) + HEADER_SIZE_NUM_TOT_PACKETS);
#ifdef VERYDEBUG
					cout << "last packet offset:" << lastpacketoffset << endl;
#endif

					if((unsigned int)(packetsPerFrame -1) != ((((uint32_t)(*((uint32_t*)(buffer[ithread]+lastpacketoffset))))+1) & (packetIndexMask))){
						memcpy(tempchar,buffer[ithread]+lastpacketoffset, onePacketSize);
#ifdef VERYDEBUG
						cout << "tempchar header:" << (((((uint32_t)(*((uint32_t*)(tempchar))))+1)
								& (frameIndexMask)) >> frameIndexOffset) << endl;
#endif
						carryonBufferSize = onePacketSize;
						--packetcount;
					}
				}
#ifdef VERYDEBUG
			cout << "header:" << (((((uint32_t)(*((uint32_t*)(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS))))+1)
										& (frameIndexMask)) >> frameIndexOffset) << endl;
#endif
				break;
			default:

				break;

			}


		//	cout<<"*********** "<<ithread<<" tempnum:"<< htonl(*(unsigned int*)((eiger_image_header *)((char*)(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS)))->fnum)<<endl;



#ifdef VERYDEBUG
			cout << "*** packetcount:" << packetcount << " carryonbuffer:" << carryonBufferSize << endl;
#endif
			//write packet count and push
			(*((uint16_t*)(buffer[ithread]))) = packetcount;
			totalListeningFrameCount[ithread] += packetcount;
#ifdef VERYDEBUG
			cout<<dec<<ithread<<" listener going to push fifo:"<<(void*)(buffer[ithread])<<endl;
#endif
			while(!fifo[ithread]->push(buffer[ithread]));
#ifdef FIFO_DEBUG
			//if(!ithread)
			cprintf(RED, "%d listener pushed into fifo %x\n",ithread, (void*)(buffer[ithread]));
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
	cout << ithread << "In startWriting()" <<endl;
#endif

	thread_started = 1;

	int numpackets, nf;
	uint32_t tempframenum;
	char* wbuf[numListeningThreads];//interleaved
	char *d=new char[bufferSize*numListeningThreads];
	int xmax=0,ymax=0;
	int ret,i;
	int packetsPerThread = packetsPerFrame/numListeningThreads;

	while(1){


		nf = 0;
		packetsPerThread = packetsPerFrame/numListeningThreads;
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


		while((1<<ithread)&writerthreads_mask){
#ifdef VERYDEBUG
			cout << ithread << " ***waiting to pop out of writing fifo" << endl;
#endif
			//pop
			for(i=0;i<numListeningThreads;++i){
#ifdef VERYDEBUG
				cout << "writer gonna pop from fifo:" << i << endl;
#endif
				fifo[i]->pop(wbuf[i]);
#ifdef FIFO_DEBUG
				cprintf(MAGENTA,"%d writer poped from fifo %x\n", ithread, (void*)(wbuf[i]));
#endif
				numpackets = (uint16_t)(*((uint16_t*)wbuf[i]));
#ifdef VERYDEBUG
				cout << i << " numpackets:" << dec << numpackets << "for fifo :"<< i << endl;
#endif
			}


			//last dummy packet
			if(numpackets == 0xFFFF){
#ifdef VERYDEBUG
				cout << "**LAST dummy packet" << endl;
#endif
				stopWriting(ithread,wbuf);
				continue;
			}
#ifdef VERYDEBUG
			else cout <<"**NOT a dummy packet"<<endl;
#endif



			//for progress
			if(myDetectorType == EIGER){
				tempframenum = htonl(*(unsigned int*)((eiger_image_header *)((char*)(wbuf[ithread] + HEADER_SIZE_NUM_TOT_PACKETS)))->fnum);
				if(dynamicRange != 32)
					tempframenum += (startFrameIndex-1); //eiger frame numbers start at 1, so need to -1
				else{
					cout << " 32 bit eiger could be "<< dec << htonl(*(unsigned int*)((eiger_image_header *)((char*)(wbuf[ithread] + HEADER_SIZE_NUM_TOT_PACKETS)))->fnum) << endl;
					cout << " 32 bit eiger32 could be "<< dec << htonl(*(unsigned int*)((eiger_image_header32 *)((char*)(wbuf[ithread] + HEADER_SIZE_NUM_TOT_PACKETS)))->fnum) << endl;
					tempframenum = ((tempframenum / EIGER_32BIT_INITIAL_CONSTANT) + startFrameIndex)-1;//eiger 32 bit mode is a multiple of 17c. +startframeindex for scans
				}
			}else if ((myDetectorType == GOTTHARD) && (shortFrame == -1))
				tempframenum = (((((uint32_t)(*((uint32_t*)(wbuf[ithread] + HEADER_SIZE_NUM_TOT_PACKETS))))+1)& (frameIndexMask)) >> frameIndexOffset);
			else
				tempframenum = ((((uint32_t)(*((uint32_t*)(wbuf[ithread] + HEADER_SIZE_NUM_TOT_PACKETS))))& (frameIndexMask)) >> frameIndexOffset);

			if(numWriterThreads == 1)
				currframenum = tempframenum;
			else{
				pthread_mutex_lock(&progress_mutex);
				if(tempframenum > currframenum)
					currframenum = tempframenum;
				pthread_mutex_unlock(&progress_mutex);
			}
#ifdef VERYDEBUG
			if(myDetectorType == EIGER)
				cout << endl <<ithread << " tempframenum:" << dec << tempframenum << " curframenum:" << currframenum << endl;
#endif


			//without datacompression: write datacall back, or write data, free fifo
			if(!dataCompression){

				if (cbAction < DO_EVERYTHING){
					for(i=0;i<numListeningThreads;++i)
						/* for eiger 32 bit mode, currframenum like gotthard, does not start from 0 or 1 */
						rawDataReadyCallBack(currframenum, wbuf[i], numpackets * onePacketSize, sfilefd, guiData,pRawDataReady);
				}else if (numpackets > 0){
					for(i=0;i<numListeningThreads;++i)
						writeToFile_withoutCompression(wbuf[i], numpackets,currframenum);
#ifdef VERYDEBUG
					cout << "written everyting" << endl;
#endif
				}


				if(myDetectorType == EIGER) {
#ifdef VERYDEBUG
					cout << "gonna copy frame" << endl;
#endif
					copyFrameToGui(wbuf,currframenum);
#ifdef VERYDEBUG
					cout << "copied frame" << endl;
#endif
					for(i=0;i<numListeningThreads;++i){
						while(!fifoFree[i]->push(wbuf[i]));
#ifdef FIFO_DEBUG
						cprintf(BLUE,"%d writer freed pushed into fifofree %x for listener %d\n",ithread, (void*)(wbuf[i]),i);
#endif
					}


				}
				else{
					//copy to gui
					if(numpackets >= packetsPerFrame){//min 1 frame, but neednt be
					//if(numpackets == packetsPerFrame * numJobsPerThread){ //only full frames
						copyFrameToGui(NULL,-1,wbuf[0]+HEADER_SIZE_NUM_TOT_PACKETS);
#ifdef VERYVERBOSE
						cout << ithread << " finished copying" << endl;
#endif
					}//else cout << "unfinished buffersize" << endl;
					while(!fifoFree[0]->push(wbuf[0]));
#ifdef FIFO_DEBUG
					cprintf(BLUE,"%d writer freed pushed into fifofree %x for listener 0\n",ithread, (void*)(wbuf[0]));
#endif
				}
			}
			//data compression
			else
				handleDataCompression(ithread,wbuf,numpackets,d, xmax, ymax, nf);





		}
#ifdef VERYVERBOSE
		cout << ithread << " gonna wait for 1st sem" << endl;
#endif
		//wait
		sem_wait(&writersmp[ithread]);
		if(killAllWritingThreads){
			cout << ithread << " good bye writing thread" << endl;
			closeFile(ithread);
			pthread_exit(NULL);
		}
#ifdef VERYVERBOSE
		cout << ithread << " got 1st post" << endl;
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
		cout << ithread << " gonna wait for 2nd sem" << endl;
#endif
		//wait
		sem_wait(&writersmp[ithread]);
		if(killAllWritingThreads){
			cout << ithread << " Goodbye thread" << endl;
			closeFile(ithread);
			pthread_exit(NULL);
		}
#ifdef VERYVERBOSE
		cout << ithread << " got 2nd post" << endl;
#endif
	}

	delete [] d;

	return OK;
}




void UDPStandardImplementation::startFrameIndices(int ithread){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	if (myDetectorType == EIGER)
		//add currframenum later  in this method for scans
		startFrameIndex = htonl(*(unsigned int*)((eiger_image_header *)((char*)(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS)))->fnum);
	//gotthard has +1 for frame number and not a short frame
	else if ((myDetectorType == GOTTHARD) && (shortFrame == -1))
		startFrameIndex = (((((uint32_t)(*((uint32_t*)(buffer[ithread] + HEADER_SIZE_NUM_TOT_PACKETS))))+1)
				& (frameIndexMask)) >> frameIndexOffset);
	else
		startFrameIndex = ((((uint32_t)(*((uint32_t*)(buffer[ithread]+HEADER_SIZE_NUM_TOT_PACKETS))))
				& (frameIndexMask)) >> frameIndexOffset);


	//start of acquisition
	if(!acqStarted){
		startAcquisitionIndex=startFrameIndex;
		currframenum = startAcquisitionIndex;
		acqStarted = true;
		cout << "startAcquisitionIndex:" << hex << startAcquisitionIndex<<endl;
	}
	//for scans, cuz currfraenum resets
	else if (myDetectorType == EIGER){
		if(dynamicRange == 32)
			startFrameIndex = (currframenum + 1);// to be added later for scans
		else
			startFrameIndex += currframenum;

	}


	cout << "startFrameIndex:" << startFrameIndex<<endl;
	prevframenum=startFrameIndex;
	measurementStarted = true;

}



void UDPStandardImplementation::stopListening(int ithread, int rc, int &pc, int &t){
	FILE_LOG(logDEBUG) << __AT__ << " called";


int i;

#ifdef VERYVERBOSE
				cerr << ithread << " recvfrom() failed:"<<endl;
#endif
				if(status != TRANSMITTING){
					cout << ithread << " *** shoule never be here********* status not transmitting***********************"<<endl;/**/
					fifoFree[ithread]->push(buffer[ithread]);
#ifdef FIFO_DEBUG
					cprintf(BLUE,"%d listener not txm free pushed into fifofree %x\n", ithread,(void*)(buffer[ithread]));
#endif
					exit(-1);
				}

				//free buffer
				if(rc <= 0){
					cout << ithread << "Discarding empty frame" << endl;
					fifoFree[ithread]->push(buffer[ithread]);
#ifdef FIFO_DEBUG
					cprintf(BLUE,"%d listener empty buffer pushed into fifofree %x\n", ithread, (void*)(buffer[ithread]));
#endif
				}
				//push the last buffer into fifo
				else{
					//eiger (incomplete frames) - throw away
					if((myDetectorType == EIGER) && (rc < (bufferSize * numJobsPerThread)) ){
						if(rc == 266240)
							cprintf(GREEN, "%d Start of detector: Received test frame of 266240 bytes.\n",ithread);
						cout << ithread << "Discarding incomplete frame" << endl;
						fifoFree[ithread]->push(buffer[ithread]);
#ifdef FIFO_DEBUG
						cprintf(BLUE,"%d listener last buffer free pushed into fifofree %x\n", ithread,(void*)(buffer[ithread]));
#endif
					}
					//eiger (complete frames) + other detectors
					else{
						pc = (rc/onePacketSize);
#ifdef VERYDEBUG
						cout << ithread << " last rc:"<<rc<<endl;
						cout << ithread <<  " *** last packetcount:" << pc << endl;
#endif
						(*((uint16_t*)(buffer[ithread]))) = pc;
						totalListeningFrameCount[ithread] += pc;
						while(!fifo[ithread]->push(buffer[ithread]));
#ifdef FIFO_DEBUG
						cprintf(RED,"%d listener last buffer pushed into fifo %x\n",  ithread,(void*)(buffer[ithread]));
#endif
					}
				}



				//push dummy buffer to all writer threads
				for(i=0;i<numWriterThreads;++i){
					fifoFree[ithread]->pop(buffer[ithread]);
#ifdef FIFO_DEBUG
					cprintf(GREEN,"%d listener popped dummy buffer from fifofree %x\n", ithread,(void*)(buffer[ithread]));
#endif
					(*((uint16_t*)(buffer[ithread]))) = 0xFFFF;
#ifdef VERYDEBUG
					cout << ithread << " dummy buffer num packets:"<< (*((uint16_t*)(buffer[ithread]))) << endl;
#endif
					while(!fifo[ithread]->push(buffer[ithread]));
#ifdef FIFO_DEBUG
					cprintf(RED,"%d listener pushed dummy buffer into fifo %x\n", ithread,(void*)(buffer[ithread]));
#endif
				}

				//reset mask and exit loop
				pthread_mutex_lock(&status_mutex);
				listeningthreads_mask^=(1<<ithread);
#ifdef VERYDEBUG
				cout << ithread << " Resetting mask of current listening thread. New Mask: " << listeningthreads_mask << endl;
#endif
				pthread_mutex_unlock(&(status_mutex));

#ifdef VERYDEBUG
				cout << ithread << ": Frames listened to " << dec << ((totalListeningFrameCount[ithread]*numListeningThreads)/packetsPerFrame) << endl;
#endif

				//waiting for all listening threads to be done, to print final count of frames listened to
				if(ithread == 0){
#ifdef VERYDEBUG
					if(numListeningThreads > 1)
						cout << "Waiting for listening to be done.. current mask:" << hex << listeningthreads_mask << endl;
#endif
					while(listeningthreads_mask)
						usleep(5000);
#ifdef VERYDEBUG
					t = 0;
					for(i=0;i<numListeningThreads;++i)
						t += totalListeningFrameCount[i];
					cout << "Total frames listened to " << dec <<(t/packetsPerFrame) << endl;
#endif
				}

}










void UDPStandardImplementation::stopWriting(int ithread, char* wbuffer[]){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	int i,j;
#ifdef VERYDEBUG
	cout << ithread << " **********************popped last dummy frame:" << (void*)wbuffer[0] << endl;
#endif

	//free fifo
	for(i=0;i<numListeningThreads;++i){
		while(!fifoFree[i]->push(wbuffer[i]));
#ifdef FIFO_DEBUG
		cprintf(BLUE,"%d writer free dummy pushed into fifofree %x for listener %d\n", ithread,(void*)(wbuffer[i]),i);
#endif
	}



	//all threads need to close file, reset mask and exit loop
	closeFile(ithread);
	pthread_mutex_lock(&status_mutex);
	writerthreads_mask^=(1<<ithread);
#ifdef VERYDEBUG
	cout << ithread << " Resetting mask of current writing thread. New Mask: " << writerthreads_mask << endl;
#endif
	pthread_mutex_unlock(&status_mutex);



	//only thread 0 needs to do this
	//check if all jobs are done and wait
	//change status to run finished
	if(ithread == 0){
		if(dataCompression){
			cout << "Waiting for jobs to be done.. current mask:" << hex << writerthreads_mask << endl;
			while(writerthreads_mask){
				/*cout << "." << flush;*/
				usleep(50000);
			}
			cout<<" Jobs Done!"<<endl;
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
			cprintf(RED, "Total Packets Caught:%d\n", totalPacketsCaught);
			cprintf(RED, "Total Frames Caught:%d\n",(totalPacketsCaught/packetsPerFrame));
		}else{
			cprintf(GREEN, "Total Packets Caught:%d\n", totalPacketsCaught);
			cprintf(GREEN, "Total Frames Caught:%d\n",(totalPacketsCaught/packetsPerFrame));
		}
		//acquisition end
		if (acquisitionFinishedCallBack)
			acquisitionFinishedCallBack((totalPacketsCaught/packetsPerFrame), pAcquisitionFinished);

	}
}











void UDPStandardImplementation::writeToFile_withoutCompression(char* buf,int numpackets, uint32_t framenum){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	int packetsToSave, offset,lastpacket;
	uint32_t tempframenum = framenum;

	//file write
	if((enableFileWrite) && (sfilefd)){

		offset = HEADER_SIZE_NUM_TOT_PACKETS;
		if(myDetectorType == EIGER)
			offset += EIGER_HEADER_LENGTH;
		while(numpackets > 0){

			//for progress and packet loss calculation(new files)
			if(myDetectorType == EIGER);
			else if ((myDetectorType == GOTTHARD) && (shortFrame == -1))
				tempframenum = (((((uint32_t)(*((uint32_t*)(buf + HEADER_SIZE_NUM_TOT_PACKETS))))+1)& (frameIndexMask)) >> frameIndexOffset);
			else
				tempframenum = ((((uint32_t)(*((uint32_t*)(buf + HEADER_SIZE_NUM_TOT_PACKETS))))& (frameIndexMask)) >> frameIndexOffset);

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
			fwrite(buf+offset, 1, packetsToSave * onePacketSize, sfilefd);
			packetsInFile += packetsToSave;
			packetsCaught += packetsToSave;
			totalPacketsCaught += packetsToSave;
#ifdef VERYDEBUG
			cout << "/totalPacketsCaught:" << dec << totalPacketsCaught <<endl;
#endif
			//new file
			if(packetsInFile >= maxPacketsPerFile){
				//for packet loss
				lastpacket = (((packetsToSave - 1) * onePacketSize) + offset);
				if(myDetectorType == EIGER);
				else if ((myDetectorType == GOTTHARD) && (shortFrame == -1))
					tempframenum = (((((uint32_t)(*((uint32_t*)(buf + lastpacket))))+1)& (frameIndexMask)) >> frameIndexOffset);
				else
					tempframenum = ((((uint32_t)(*((uint32_t*)(buf + lastpacket))))& (frameIndexMask)) >> frameIndexOffset);

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


			offset += (packetsToSave * onePacketSize);
			numpackets -= packetsToSave;
		}

	}
	else{
		if(numWriterThreads > 1)
			pthread_mutex_lock(&write_mutex);
		packetsInFile += numpackets;
		packetsCaught += numpackets;
		totalPacketsCaught += numpackets;
		if(numWriterThreads > 1)
			pthread_mutex_unlock(&write_mutex);
	}
}














void UDPStandardImplementation::handleDataCompression(int ithread, char* wbuffer[], int &npackets, char* data, int xmax, int ymax, int &nf){
	FILE_LOG(logDEBUG) << __AT__ << " called";

#if defined(MYROOT1) && defined(ALLFILE_DEBUG)
				writeToFile_withoutCompression(wbuf[0], numpackets,currframenum);
#endif

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
						if(packetsInFile >= maxPacketsPerFile)
							createNewFile();
						pthread_mutex_unlock(&progress_mutex);

#endif
						if(!once){
							copyFrameToGui(NULL,-1,buff);
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





int UDPStandardImplementation::enableTenGiga(int enable){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	cout << "Enabling 10Gbe to" << enable << endl;

	int oldtengiga = tengigaEnable;
	if(enable >= 0){

		tengigaEnable = enable;

		if(myDetectorType == EIGER){

			if(!tengigaEnable){
				packetsPerFrame = EIGER_ONE_GIGA_CONSTANT * dynamicRange * EIGER_MAX_PORTS;
				onePacketSize  	= EIGER_ONE_GIGA_ONE_PACKET_SIZE;
				maxPacketsPerFile 	= EIGER_MAX_FRAMES_PER_FILE * packetsPerFrame;
			}else{
				packetsPerFrame = EIGER_TEN_GIGA_CONSTANT * dynamicRange * EIGER_MAX_PORTS;
				onePacketSize  	= EIGER_TEN_GIGA_ONE_PACKET_SIZE;
				maxPacketsPerFile 	= EIGER_MAX_FRAMES_PER_FILE * packetsPerFrame*4;
			}
			frameSize			= onePacketSize * packetsPerFrame;
			bufferSize 			= (frameSize/EIGER_MAX_PORTS) + EIGER_HEADER_LENGTH;//everything one port gets (img header plus packets)
			//maxPacketsPerFile 	= EIGER_MAX_FRAMES_PER_FILE * packetsPerFrame;


			cout<<"packetsPerFrame:"<<dec<<packetsPerFrame<<endl;
			cout<<"onePacketSize:"<<onePacketSize<<endl;
			cout<<"framsize:"<<frameSize<<endl;
			cout<<"bufferSize:"<<bufferSize<<endl;
			cout<<"maxPacketsPerFile:"<<maxPacketsPerFile<<endl;


			if(oldtengiga != enable){

				//del
				if(thread_started){
					createListeningThreads(true);
					createWriterThreads(true);
				}
				for(int i=0;i<numListeningThreads;i++){
					if(mem0[i])			{free(mem0[i]);			mem0[i] = NULL;}
					if(fifo[i])			{delete fifo[i];		fifo[i] = NULL;}
					if(fifoFree[i])		{delete fifoFree[i];	fifoFree[i] = NULL;}
					buffer[i] = NULL;
				}
				if(latestData) 		{delete [] latestData;	latestData = NULL;}
				latestData = new char[frameSize];

				numJobsPerThread = -1;
				setupFifoStructure();

				if(createListeningThreads() == FAIL){
					cprintf(BG_RED,"ERROR: Could not create listening thread\n");
					exit (-1);
				}

				if(createWriterThreads() == FAIL){
					cprintf(BG_RED,"ERROR: Could not create writer threads\n");
					exit (-1);
				}

				setThreadPriorities();
			}
		}

	}

	return tengigaEnable;
}



