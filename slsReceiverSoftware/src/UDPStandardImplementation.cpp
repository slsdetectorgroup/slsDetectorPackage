/********************************************//**
 * @file UDPStandardImplementation.cpp
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 ***********************************************/


#include "UDPStandardImplementation.h"
#include "GeneralData.h"
#include "Listener.h"
#include "DataProcessor.h"
#include "DataStreamer.h"
#include "Fifo.h"

#include <cstdlib>			//system
#include <cstring>			//strcpy
using namespace std;


/** cosntructor & destructor */

UDPStandardImplementation::UDPStandardImplementation() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	InitializeMembers();

}


UDPStandardImplementation::~UDPStandardImplementation() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	DeleteMembers();
}


void UDPStandardImplementation::DeleteMembers() {
	FILE_LOG (logDEBUG) << __AT__ << " starting";

	if (generalData) { delete generalData; generalData=0;}
	for (vector<Listener*>::const_iterator it = listener.begin(); it != listener.end(); ++it)
		delete(*it);
	listener.clear();
	for (vector<DataProcessor*>::const_iterator it = dataProcessor.begin(); it != dataProcessor.end(); ++it)
		delete(*it);
	dataProcessor.clear();
	for (vector<DataStreamer*>::const_iterator it = dataStreamer.begin(); it != dataStreamer.end(); ++it)
		delete(*it);
	dataStreamer.clear();
	for (vector<Fifo*>::const_iterator it = fifo.begin(); it != fifo.end(); ++it)
		delete(*it);
	fifo.clear();
}


void UDPStandardImplementation::InitializeMembers() {
	FILE_LOG (logDEBUG) << __AT__ << " starting";

	UDPBaseImplementation::initializeMembers();
	acquisitionPeriod = SAMPLE_TIME_IN_NS;

	//*** detector parameters ***
	detID = -1;

	//*** receiver parameters ***
	numThreads = 1;
	numberofJobs = 1;

	//*** mutex ***
	pthread_mutex_init(&statusMutex,NULL);

	//** class objects ***
	generalData = 0;
}



/*** Overloaded Functions called by TCP Interface ***/

uint64_t UDPStandardImplementation::getTotalFramesCaught() const {
	FILE_LOG (logDEBUG) << __AT__ << " starting";
	uint64_t sum = 0;
	vector<DataProcessor*>::const_iterator it;
	for (vector<DataProcessor*>::const_iterator it = dataProcessor.begin(); it != dataProcessor.end(); ++it)
		sum += (*it)->GetNumTotalFramesCaught();
	return (sum/dataProcessor.size());
}

uint64_t UDPStandardImplementation::getFramesCaught() const {
	FILE_LOG (logDEBUG) << __AT__ << " starting";
	uint64_t sum = 0;
	for (vector<DataProcessor*>::const_iterator it = dataProcessor.begin(); it != dataProcessor.end(); ++it)
		sum += (*it)->GetNumFramesCaught();
	return (sum/dataProcessor.size());
}

int64_t UDPStandardImplementation::getAcquisitionIndex() const {
	FILE_LOG (logDEBUG) << __AT__ << " starting";
	//no data processed
	if(!DataProcessor::GetAcquisitionStartedFlag())
		return -1;
	uint64_t sum = 0;
	for (vector<DataProcessor*>::const_iterator it = dataProcessor.begin(); it != dataProcessor.end(); ++it)
		sum += (*it)->GetProcessedAcquisitionIndex();
	return (sum/dataProcessor.size());
}


void UDPStandardImplementation::setFileName(const char c[]) {
	FILE_LOG (logDEBUG) << __AT__ << " starting";

	if (strlen(c)) {
		strcpy(fileName, c); //automatically update fileName in Filewriter (pointer)
		int detindex = -1;
		string tempname(fileName);
		size_t uscore=tempname.rfind("_");
		if (uscore!=string::npos) {
			if (sscanf(tempname.substr(uscore+1, tempname.size()-uscore-1).c_str(), "d%d", &detindex)) {
				detID = detindex;
			}
		}
		if (detindex == -1)
			detID = 0;
	}
	FILE_LOG (logINFO) << "File name:" << fileName;
}


int UDPStandardImplementation::setShortFrameEnable(const int i) {
	FILE_LOG (logDEBUG) << __AT__ << " called";

	if (myDetectorType != GOTTHARD) {
		cprintf(RED, "Error: Can not set short frame for this detector\n");
		return FAIL;
	}

	if (shortFrameEnable != i) {
		shortFrameEnable = i;

		if (generalData)
			delete generalData;
		if (i != -1)
			generalData = new ShortGotthardData();
		else
			generalData = new GotthardData();
		numberofJobs = -1; //changes to imagesize has to be noted to recreate fifo structure
		if (SetupFifoStructure() == FAIL)
			return FAIL;

		Listener::SetGeneralData(generalData);
		DataProcessor::SetGeneralData(generalData);
	}
	FILE_LOG (logINFO) << "Short Frame Enable: " << shortFrameEnable;
	return OK;
}


int UDPStandardImplementation::setFrameToGuiFrequency(const uint32_t freq) {
	FILE_LOG (logDEBUG) << __AT__ << " called";

	if (frameToGuiFrequency != freq) {
		frameToGuiFrequency = freq;

		//only the ones lisening to more than 1 frame at a time needs to change fifo structure
		switch (myDetectorType) {
		case GOTTHARD:
		case PROPIX:
		if (SetupFifoStructure() == FAIL)
			return FAIL;
		break;
		default:
			break;
		}
	}
	FILE_LOG (logINFO) << "Frame to Gui Frequency: " << frameToGuiFrequency;
	return OK;
}


int UDPStandardImplementation::setDataStreamEnable(const bool enable) {
	FILE_LOG (logDEBUG) << __AT__ << " called";

	if (dataStreamEnable != enable) {
		dataStreamEnable = enable;

		//data sockets have to be created again as the client ones are
		for (vector<DataStreamer*>::const_iterator it = dataStreamer.begin(); it != dataStreamer.end(); ++it)
			delete(*it);
		dataStreamer.clear();

		if (enable) {
			for ( int i=0; i < numThreads; ++i ) {
				dataStreamer.push_back(new DataStreamer());
				if (DataStreamer::GetErrorMask()) {
					cprintf(BG_RED,"Error: Could not create data callback threads\n");
					for (vector<DataStreamer*>::const_iterator it = dataStreamer.begin(); it != dataStreamer.end(); ++it)
						delete(*it);
					dataStreamer.clear();
					return FAIL;
				}
			}
		}
	}
	FILE_LOG (logINFO) << "Data Send to Gui: " << dataStreamEnable;
	return OK;
}


int UDPStandardImplementation::setAcquisitionPeriod(const uint64_t i) {
	FILE_LOG (logDEBUG) << __AT__ << " called";

	if (acquisitionPeriod != i) {
		acquisitionPeriod = i;

		//only the ones lisening to more than 1 frame at a time needs to change fifo structure
		switch (myDetectorType) {
		case GOTTHARD:
		case PROPIX:
		if (SetupFifoStructure() == FAIL)
			return FAIL;
		break;
		default:
			break;
		}
	}
	FILE_LOG (logINFO) << "Acquisition Period: " << (double)acquisitionPeriod/(1E9) << "s";
	return OK;
}


int UDPStandardImplementation::setAcquisitionTime(const uint64_t i) {
	FILE_LOG (logDEBUG) << __AT__ << " called";

	if (acquisitionTime != i) {
		acquisitionTime = i;

		//only the ones lisening to more than 1 frame at a time needs to change fifo structure
		switch (myDetectorType) {
		case GOTTHARD:
		case PROPIX:
		if (SetupFifoStructure() == FAIL)
			return FAIL;
		break;
		default:
			break;
		}
	}
	FILE_LOG (logINFO) << "Acquisition Period: " << (double)acquisitionTime/(1E9) << "s";
	return OK;
}


int UDPStandardImplementation::setNumberOfFrames(const uint64_t i) {
	FILE_LOG (logDEBUG) << __AT__ << " called";

	if (numberOfFrames != i) {
		numberOfFrames = i;

		//only the ones lisening to more than 1 frame at a time needs to change fifo structure
		switch (myDetectorType) {
		case GOTTHARD:
		case PROPIX:
		if (SetupFifoStructure() == FAIL)
			return FAIL;
		break;
		default:
			break;
		}
	}
	FILE_LOG (logINFO) << "Number of Frames:" << numberOfFrames;
	return OK;
}


int UDPStandardImplementation::setDynamicRange(const uint32_t i) {
	FILE_LOG (logDEBUG) << __AT__ << " called";

	if (dynamicRange != i) {
		dynamicRange = i;
		//side effects
		generalData->SetDynamicRange(i,tengigaEnable);

		numberofJobs = -1; //changes to imagesize has to be noted to recreate fifo structure
		if (SetupFifoStructure() == FAIL)
			return FAIL;
	}
	FILE_LOG (logINFO) << "Dynamic Range: " << dynamicRange;
	return OK;
}


int UDPStandardImplementation::setTenGigaEnable(const bool b) {
	FILE_LOG (logDEBUG) << __AT__ << " called";

	if (tengigaEnable != b) {
		tengigaEnable = b;
		//side effects
		generalData->SetTenGigaEnable(tengigaEnable,b);

		numberofJobs = -1; //changes to imagesize has to be noted to recreate fifo structure
		if (SetupFifoStructure() == FAIL)
			return FAIL;
	}
	FILE_LOG (logINFO) << "Ten Giga: " << stringEnable(tengigaEnable);
	return OK;
}


int UDPStandardImplementation::setFifoDepth(const uint32_t i) {
	FILE_LOG (logDEBUG) << __AT__ << " called";

	if (fifoDepth != i) {
		fifoDepth = i;

		numberofJobs = -1; //changes to imagesize has to be noted to recreate fifo structure
		if (SetupFifoStructure() == FAIL)
			return FAIL;
	}
	FILE_LOG (logINFO) << "Fifo Depth: " << i << endl;
	return OK;
}



int UDPStandardImplementation::setDetectorType(const detectorType d) {
	FILE_LOG (logDEBUG) << __AT__ << " starting";

	FILE_LOG (logDEBUG) << "Setting receiver type";

	DeleteMembers();cout<<"size of fifo:"<<fifo.size()<<endl;
	InitializeMembers();
	myDetectorType = d;
	switch(myDetectorType) {
	case GOTTHARD:
	case PROPIX:
	case MOENCH:
	case EIGER:
	case JUNGFRAUCTB:
	case JUNGFRAU:
		FILE_LOG (logINFO) << " ***** " << getDetectorType(d) << " Receiver *****";
		break;
	default:
		FILE_LOG (logERROR) << "This is an unknown receiver type " << (int)d;
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
	Listener::SetGeneralData(generalData);
	DataProcessor::SetGeneralData(generalData);
	numThreads = generalData->threadsPerReceiver;
	fifoDepth = generalData->defaultFifoDepth;

	//create fifo structure
	numberofJobs = -1;
	if (SetupFifoStructure() == FAIL) {
		FILE_LOG (logERROR) << "Error: Could not allocate memory for fifo structure";
		return FAIL;
	}

	//create threads
	for ( int i=0; i < numThreads; ++i ) {
		listener.push_back(new Listener(fifo[i], &status, &udpPortNum[i]));
		dataProcessor.push_back(new DataProcessor(fifo[i], &status, &statusMutex));
		if (Listener::GetErrorMask() || DataProcessor::GetErrorMask()) {
			FILE_LOG (logERROR) << "Error: Could not creates listener/dataprocessor threads (index:" << i << ")";
			for (vector<Listener*>::const_iterator it = listener.begin(); it != listener.end(); ++it)
				delete(*it);
			listener.clear();
			for (vector<DataProcessor*>::const_iterator it = dataProcessor.begin(); it != dataProcessor.end(); ++it)
				delete(*it);
			dataProcessor.clear();
			return FAIL;
		}
	}

	//local network parameters
	SetLocalNetworkParameters();

	FILE_LOG (logDEBUG) << " Detector type set to " << getDetectorType(d);
	return OK;
}



void UDPStandardImplementation::resetAcquisitionCount() {
	FILE_LOG (logDEBUG) << __AT__ << " starting";

	for (vector<Listener*>::const_iterator it = listener.begin(); it != listener.end(); ++it)
		(*it)->ResetParametersforNewAcquisition();

	for (vector<DataProcessor*>::const_iterator it = dataProcessor.begin(); it != dataProcessor.end(); ++it)
		(*it)->ResetParametersforNewAcquisition();

	FILE_LOG (logINFO) << "Acquisition Count has been reset";
}



int UDPStandardImplementation::startReceiver(char *c) {
	FILE_LOG (logDEBUG) << __AT__ << " called";

	ResetParametersforNewMeasurement();

	if (CreateUDPSockets() == FAIL) {
		strcpy(c,"Could not create UDP Socket(s).");
		FILE_LOG(logERROR) << c;
		return FAIL;
	}

	if(fileWriteEnable){
		if (SetupWriter() == FAIL) {
			strcpy(c,"Could not create file.");
			FILE_LOG(logERROR) << c;
			return FAIL;
		}
	}

	//change status
	pthread_mutex_lock(&statusMutex);
	status = RUNNING;
	pthread_mutex_unlock(&(statusMutex));

	//Let Threads continue to be ready for acquisition
	StartRunning();

	FILE_LOG(logINFO)  << "Receiver Started";
	FILE_LOG(logINFO)  << "Status: " << runStatusType(status);
}



void UDPStandardImplementation::stopReceiver(){
	FILE_LOG(logDEBUG) << __AT__ << " called";
	FILE_LOG(logINFO)  << "Stopping Receiver";

	//set status to transmitting
	startReadout();

	//wait until status is run_finished
	while(status == TRANSMITTING){
		usleep(5000);
	}

	//change status
	pthread_mutex_lock(&statusMutex);
	status = IDLE;
	pthread_mutex_unlock(&(statusMutex));

	FILE_LOG(logINFO)  << "Receiver Stopped";
	FILE_LOG(logINFO)  << "Status: " << runStatusType(status);
	cout << endl << endl;
}



void UDPStandardImplementation::startReadout(){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	if(status == RUNNING){

		//needs to wait for packets only if activated
		if(activated){

			//current packets caught
			int totalP = 0,prev=-1;
			for (vector<Listener*>::const_iterator it = listener.begin(); it != listener.end(); ++it)
				totalP += (*it)->GetTotalPacketsCaught();

				//current udp buffer received
			int currentReceivedInBuffer=0,prevReceivedInBuffer=-1;
			for (vector<Listener*>::const_iterator it = listener.begin(); it != listener.end(); ++it)
				currentReceivedInBuffer += (*it)->GetNumReceivedinUDPBuffer();

			//wait for all packets
			if((unsigned long long int)totalP!=numberOfFrames*generalData->packetsPerFrame*listener.size()){

				//wait as long as there is change from prev totalP,
				//and also change from received in buffer to previous value
				//(as one listens to many at a time, shouldnt cut off in between)
				while((prev != totalP) || (prevReceivedInBuffer!= currentReceivedInBuffer)){
#ifdef VERY_VERBOSE
					cprintf(MAGENTA,"waiting for all packets prevP:%d totalP:%d PrevBuffer:%d currentBuffer:%d\n",prev,totalP,prevReceivedInBuffer,currentReceivedInBuffer);

#endif
					//usleep(2*1000*1000);
					usleep(5*1000);/* Need to find optimal time **/

					prev = totalP;
					totalP = 0;
					prevReceivedInBuffer = currentReceivedInBuffer;
					currentReceivedInBuffer = 0;

					for (vector<Listener*>::const_iterator it = listener.begin(); it != listener.end(); ++it) {
						totalP += (*it)->GetTotalPacketsCaught();
						currentReceivedInBuffer += (*it)->GetNumReceivedinUDPBuffer();
					}
#ifdef VERY_VERBOSE
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

	//shut down udp sockets so as to make listeners push dummy (end) packets for processors
	shutDownUDPSockets();
}


void UDPStandardImplementation::shutDownUDPSockets() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	for (vector<Listener*>::const_iterator it = listener.begin(); it != listener.end(); ++it)
		(*it)->ShutDownUDPSocket();
}



void UDPStandardImplementation::closeFiles() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	for (vector<DataProcessor*>::const_iterator it = dataProcessor.begin(); it != dataProcessor.end(); ++it)
		(*it)->CloseFile();
}



void UDPStandardImplementation::SetLocalNetworkParameters() {
	FILE_LOG (logDEBUG) << __AT__ << " called";

	//to increase socket receiver buffer size and max length of input queue by changing kernel settings
	if (myDetectorType == EIGER)
		return;

	char command[255];

	//to increase Socket Receiver Buffer size
	sprintf(command,"echo $((%d)) > /proc/sys/net/core/rmem_max",RECEIVE_SOCKET_BUFFER_SIZE);
	if (system(command)) {
		FILE_LOG (logWARNING) << "No root permission to change Socket Receiver Buffer size (/proc/sys/net/core/rmem_max)";
		return;
	}
	FILE_LOG (logINFO) << "Socket Receiver Buffer size (/proc/sys/net/core/rmem_max) modified to " << RECEIVE_SOCKET_BUFFER_SIZE ;


	// to increase Max length of input packet queue
	sprintf(command,"echo %d > /proc/sys/net/core/netdev_max_backlog",MAX_SOCKET_INPUT_PACKET_QUEUE);
	if (system(command)) {
		FILE_LOG (logWARNING) << "No root permission to change Max length of input packet queue (/proc/sys/net/core/netdev_max_backlog)";
		return;
	}
	FILE_LOG (logINFO) << "Max length of input packet queue (/proc/sys/net/core/netdev_max_backlog) modified to " << MAX_SOCKET_INPUT_PACKET_QUEUE ;
}



int UDPStandardImplementation::SetupFifoStructure() {
	FILE_LOG (logDEBUG) << __AT__ << " called";


	//recalculate number of jobs &  fifodepth, return if no change
	if ((myDetectorType == GOTTHARD) || (myDetectorType == PROPIX)) {

		int oldnumberofjobs = numberofJobs;
		//listen to only n jobs at a time
		if (frameToGuiFrequency)
			numberofJobs = frameToGuiFrequency;
		else {
			//random freq depends on acquisition period/time (calculate upto 100ms/period)
			int i = ((acquisitionPeriod > 0) ?
					(SAMPLE_TIME_IN_NS/acquisitionPeriod):
					((acquisitionTime > 0) ? (SAMPLE_TIME_IN_NS/acquisitionTime) : SAMPLE_TIME_IN_NS));
			//must be > 0 and < max jobs
			numberofJobs = ((i < 1) ? 1 : ((i > MAX_JOBS_PER_THREAD) ? MAX_JOBS_PER_THREAD : i));
		}
		FILE_LOG (logINFO) << "Number of Jobs Per Thread:" << numberofJobs << endl;

		uint32_t oldfifodepth = fifoDepth;
		//reduce fifo depth if numberofJobsPerBuffer > 1 (to save memory)
		if (numberofJobs >1) {
			fifoDepth = ((fifoDepth % numberofJobs) ?
					((fifoDepth/numberofJobs)+1) : //if not directly divisible
					(fifoDepth/numberofJobs));
		}
		FILE_LOG (logINFO) << "Total Fifo Depth Recalculated:" << fifoDepth;

		//no change, return
		if ((oldnumberofjobs == numberofJobs) && (oldfifodepth == fifoDepth))
			return OK;
	}else
		numberofJobs = 1;


	for (vector<Fifo*>::const_iterator it = fifo.begin(); it != fifo.end(); ++it)
		delete(*it);
	fifo.clear();
	for ( int i = 0; i < numThreads; i++ ) {
		//create fifo structure
		bool success = true;
		fifo.push_back( new Fifo (
				(generalData->fifoBufferSize) * numberofJobs + (generalData->fifoBufferHeaderSize),
				fifoDepth, success));
		if (!success) {
			cprintf(BG_RED,"Error: Could not allocate memory for fifo structure of index %d\n", i);
			for (vector<Fifo*>::const_iterator it = fifo.begin(); it != fifo.end(); ++it)
				delete(*it);
			fifo.clear();
			return FAIL;
		}
		//set the listener & dataprocessor threads to point to the right fifo
		if(listener.size())listener[i]->SetFifo(fifo[i]);
		if(dataProcessor.size())dataProcessor[i]->SetFifo(fifo[i]);
	}

	FILE_LOG (logINFO) << "Fifo structure(s) reconstructed";
	return OK;
}



void UDPStandardImplementation::ResetParametersforNewMeasurement() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	for (vector<Listener*>::const_iterator it = listener.begin(); it != listener.end(); ++it)
		(*it)->ResetParametersforNewMeasurement();
	for (vector<DataProcessor*>::const_iterator it = dataProcessor.begin(); it != dataProcessor.end(); ++it)
		(*it)->ResetParametersforNewMeasurement();
}



int UDPStandardImplementation::CreateUDPSockets() {
	FILE_LOG (logDEBUG) << __AT__ << " called";

	shutDownUDPSockets();

	//if eth is mistaken with ip address
	if (strchr(eth,'.') != NULL){
		strcpy(eth,"");
	}
	if(!strlen(eth)){
		FILE_LOG(logWARNING) << "eth is empty. Listening to all";
	}
	bool error = false;
	for (unsigned int i = 0; i < listener.size(); ++i)
		if (listener[i]->CreateUDPSockets((strlen(eth)?eth:NULL)) == FAIL) {
			error = true;
			break;
		}
	if (error) {
		shutDownUDPSockets();
		return FAIL;
	}

	FILE_LOG(logDEBUG) << "UDP socket(s) created successfully.";
	cout << "Listener Ready ..." << endl;
	return OK;
}


int UDPStandardImplementation::SetupWriter() {
	FILE_LOG (logDEBUG) << __AT__ << " called";

	bool error = false;
	for (unsigned int i = 0; i < dataProcessor.size(); ++i)
		if (dataProcessor[i]->CreateNewFile() == FAIL) {
			error = true;
			break;
		}
	if (error) {
		shutDownUDPSockets();
		closeFiles();
		return FAIL;
	}

	cout << "Writer Ready ..." << endl;
	return OK;
}


void UDPStandardImplementation::StartRunning() {
	FILE_LOG (logDEBUG) << __AT__ << " called";

	//set running mask and post semaphore to start the inner loop in execution thread
	for (vector<Listener*>::const_iterator it = listener.begin(); it != listener.end(); ++it) {
		(*it)->StartRunning();
		(*it)->Continue();
	}
	for (vector<DataProcessor*>::const_iterator it = dataProcessor.begin(); it != dataProcessor.end(); ++it){
		(*it)->StartRunning();
		(*it)->Continue();
	}
}
