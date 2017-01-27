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
#include "BinaryFileWriter.h"
#ifdef HDF5C
#include "HDF5FileWriter.h"
#endif

#include <cstdlib>			//system
#include <cstring>			//strcpy
using namespace std;


/** cosntructor & destructor */

UDPStandardImplementation::UDPStandardImplementation() {
	FILE_LOG(logDEBUG) << __AT__ << " called";
	InitializeMembers();

}


UDPStandardImplementation::~UDPStandardImplementation(){
	FILE_LOG(logDEBUG) << __AT__ << " called";
	DeleteMembers();
}


void UDPStandardImplementation::DeleteMembers() {
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	if (generalData){ delete generalData; generalData=0;}
	listener.clear();
	dataProcessor.clear();
	dataStreamer.clear();
	fifo.clear();
	fileWriter.clear();
}


void UDPStandardImplementation::InitializeMembers(){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	UDPBaseImplementation::initializeMembers();
	acquisitionPeriod = SAMPLE_TIME_IN_NS;

	//*** detector parameters ***
	detID = -1;

	//*** receiver parameters ***
	numThreads = 1;
	numberofJobs = 1;

	//** class objects ***
	generalData = 0;
	listener.clear();
	dataProcessor.clear();
	dataStreamer.clear();
	fifo.clear();
	fileWriter.clear();
}


int UDPStandardImplementation::setDetectorType(const detectorType d) {
	FILE_LOG(logDEBUG) << __AT__ << " starting";


	numThreads = EIGER_PORTS_PER_READOUT;
	numberofJobs = 1;
	//killing all threads, deleting members etc.


	//generalData = new GotthardData();



	//only at start or changing parameters
	for ( int i=0; i < 1; i++ ) {//numthreads; i++ ) {

		bool success = true;
		fifo.push_back(new Fifo(1024*256,5, success));
		if (!success) cprintf(RED,"not successful\n");


		listener.push_back(new Listener(fifo[i]));
		dataProcessor.push_back(new DataProcessor(fifo[i]));
		//dataStreamer.push_back(new DataStreamer(fifo[i]));


		//listener[i]->SetFifo(fifo[i]);
		//dataProcessor[i]->SetFifo(fifo[i]);

		fileWriter.push_back(new BinaryFileWriter(fileName));

	}


	if (Listener::GetErrorMask() || DataProcessor::GetErrorMask()){
		cprintf(RED, "Error in creating threads\n");
	}


	//start receiver functions
	//create udp sockets
	//create file
	//reset status
	//reset all masks
	Listener::ResetRunningMask();
	DataProcessor::ResetRunningMask();
	//DataStreamer::ResetRunningMask();


	for( unsigned int i=0; i < listener.size();i++ ) {
		listener[i]->StartRunning();
		dataProcessor[i]->StartRunning();
		listener[i]->Continue();
		dataProcessor[i]->Continue();
	 }


	// for (vector<Listener*>::iterator it = listener.begin(); it != listener.end(); ++it) {
	//*it->StartRunning();

	usleep (5 * 1000 * 1000);


	SetLocalNetworkParameters();

	return OK;
}


uint64_t UDPStandardImplementation::getTotalFramesCaught() const{
	FILE_LOG(logDEBUG) << __AT__ << " starting";
	//preventing divide by 0 using ternary operator
	return (totalPacketsCaught/(packetsPerFrame*(listener.size()>0?listener.size():1)));
}

uint64_t UDPStandardImplementation::getFramesCaught() const{
	FILE_LOG(logDEBUG) << __AT__ << " starting";
	//preventing divide by 0 using ternary operator
	return (packetsCaught/(packetsPerFrame*(listener.size()>0?listener.size():1)));
}


void UDPStandardImplementation::setFileName(const char c[]){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

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
	FILE_LOG(logINFO) << "File name:" << fileName;
}


int UDPStandardImplementation::setShortFrameEnable(const int i){
	FILE_LOG(logDEBUG) << __AT__ << " called";

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
		if(SetupFifoStructure() == FAIL)
			return FAIL;
	}
	FILE_LOG(logINFO) << "Short Frame Enable: " << shortFrameEnable;
	return OK;
}


int UDPStandardImplementation::setFrameToGuiFrequency(const uint32_t freq){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	if (frameToGuiFrequency != freq){
		frameToGuiFrequency = freq;

		//only the ones lisening to more than 1 frame at a time needs to change fifo structure
		switch (myDetectorType) {
		case GOTTHARD:
		case PROPIX:
		if(SetupFifoStructure() == FAIL)
			return FAIL;
		break;
		default:
			break;
		}
	}
	FILE_LOG(logINFO) << "Frame to Gui Frequency: " << frameToGuiFrequency;
	return OK;
}


int UDPStandardImplementation::setDataStreamEnable(const bool enable){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	if (dataStreamEnable != enable) {
		dataStreamEnable = enable;

		//data sockets have to be created again as the client ones are
		if(dataStreamer.size())
			dataStreamer.clear();

		if(enable){
			for ( int i=0; i < numThreads; ++i ) {
				dataStreamer.push_back(new DataStreamer());
				if (DataStreamer::GetErrorMask()) {
					cprintf(BG_RED,"Error: Could not create data callback threads\n");
					return FAIL;
				}
			}
		}
	}
	FILE_LOG(logINFO) << "Data Send to Gui: " << dataStreamEnable;
	return OK;
}


int UDPStandardImplementation::setAcquisitionPeriod(const uint64_t i){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	if (acquisitionPeriod != i) {
		acquisitionPeriod = i;

		//only the ones lisening to more than 1 frame at a time needs to change fifo structure
		switch (myDetectorType) {
		case GOTTHARD:
		case PROPIX:
		if(setupFifoStructure() == FAIL)
			return FAIL;
		break;
		case EIGER:
			if (fileFormatType == BINARY)
				for (int i=0; i<numThreads; ++i )
					updateFileHeader(i); /*????????*/
			break;
		default:
			break;
		}
	}
	FILE_LOG(logINFO) << "Acquisition Period: " << (double)acquisitionPeriod/(1E9) << "s";
	return OK;
}




void UDPStandardImplementation::SetLocalNetworkParameters(){
	FILE_LOG(logDEBUG) << __AT__ << " called";

	//to increase socket receiver buffer size and max length of input queue by changing kernel settings
	if (myDetectorType == EIGER)
		return;

	char command[255];

	//to increase Socket Receiver Buffer size
	sprintf(command,"echo $((%d)) > /proc/sys/net/core/rmem_max",RECEIVE_SOCKET_BUFFER_SIZE);
	if (system(command)){
		FILE_LOG(logWARNING) << "No root permission to change Socket Receiver Buffer size (/proc/sys/net/core/rmem_max)";
		return;
	}
	FILE_LOG(logINFO) << "Socket Receiver Buffer size (/proc/sys/net/core/rmem_max) modified to " << RECEIVE_SOCKET_BUFFER_SIZE ;


	// to increase Max length of input packet queue
	sprintf(command,"echo %d > /proc/sys/net/core/netdev_max_backlog",MAX_SOCKET_INPUT_PACKET_QUEUE);
	if (system(command)){
		FILE_LOG(logWARNING) << "No root permission to change Max length of input packet queue (/proc/sys/net/core/netdev_max_backlog)";
		return;
	}
	FILE_LOG(logINFO) << "Max length of input packet queue (/proc/sys/net/core/netdev_max_backlog) modified to " << MAX_SOCKET_INPUT_PACKET_QUEUE ;
}



int UDPStandardImplementation::SetupFifoStructure(){
	FILE_LOG(logDEBUG) << __AT__ << " called";


	//recalculate number of jobs &  fifodepth, return if no change
	if ((myDetectortype == GOTTHARD) || (myDetectortype = PROPIX)) {

		int oldnumberofjobs = numberofJobs;
		//listen to only n jobs at a time
		if (frameToGuiFrequency)
			numberofJobs = frameToGuiFrequency;
		else {
			//random freq depends on acquisition period/time (calculate upto 100ms/period)
			i = ((acquisitionPeriod > 0) ?
					(SAMPLE_TIME_IN_NS/acquisitionPeriod):
					((acquisitionTime > 0) ? (SAMPLE_TIME_IN_NS/acquisitionTime) : SAMPLE_TIME_IN_NS));
			//must be > 0 and < max jobs
			numberofJobs = ((i < 1) ? 1 : ((i > MAX_JOBS_PER_THREAD) ? MAX_JOBS_PER_THREAD : i));
		}
		FILE_LOG(logINFO) << "Number of Jobs Per Thread:" << numberofJobs << endl;


		uint32_t oldfifodepth = fifoDepth;
		//reduce fifo depth if numberofJobsPerBuffer > 1 (to save memory)
		if(numberofJobsPerBuffer >1){
			fifoDepth = ((fifoDepth % numberofJobsPerBuffer) ?
					((fifoDepth/numberofJobsPerBuffer)+1) : //if not directly divisible
					(fifoDepth/numberofJobsPerBuffer));
		}
		FILE_LOG(logINFO) << "Total Fifo Size:" << fifoSize;

		//no change, return
		if ((oldnumberofjobs == numberofJobs) && (oldfifodepth == fifoDepth))
			return OK;
	}


	//delete fifostructure
	fifo.clear();
	for ( int i=0; i < numThreads; i++ ) {

		//create fifo structure
		bool success = true;
		fifo.push_back( new Fifo ((generalData->fifoBufferSize) * numberofJobs + (generalData->fifoBufferHeaderSize), success));
		if (!success){
			cprintf(BG_RED,"Error: Could not allocate memory for listening \n");
			return FAIL;
		}

		//set the listener & dataprocessor threads to point to the right fifo
		listener[i]->SetFifo(fifo[i]);
		dataProcessor[i]->SetFifo(fifo[i]);
	}
	FILE_LOG(logINFO) << "Fifo structure(s) reconstructed";
	return OK;
}

