/************************************************
 * @file DataStreamer.cpp
 * @short streams data from receiver via ZMQ
 ***********************************************/


#include "DataStreamer.h"
#include "GeneralData.h"
#include "Fifo.h"
#include "ZmqSocket.h"

#include <iostream>
#include <errno.h>
using namespace std;

const string DataStreamer::TypeName = "DataStreamer";

int DataStreamer::NumberofDataStreamers(0);

uint64_t DataStreamer::ErrorMask(0x0);

uint64_t DataStreamer::RunningMask(0x0);

pthread_mutex_t DataStreamer::Mutex = PTHREAD_MUTEX_INITIALIZER;

bool DataStreamer::SilentMode(false);


DataStreamer::DataStreamer(Fifo*& f, uint32_t* dr, int* sEnable, uint64_t* fi, int* fd, char* ajh) :
		ThreadObject(NumberofDataStreamers),
		generalData(0),
		fifo(f),
		zmqSocket(0),
		dynamicRange(dr),
		shortFrameEnable(sEnable),
		fileIndex(fi),
		acquisitionStartedFlag(false),
		measurementStartedFlag(false),
		firstAcquisitionIndex(0),
		firstMeasurementIndex(0),
		completeBuffer(0),
		flippedData(fd),
		additionJsonHeader(ajh)
{
	if(ThreadObject::CreateThread()){
		pthread_mutex_lock(&Mutex);
		ErrorMask ^= (1<<index);
		pthread_mutex_unlock(&Mutex);
	}

	NumberofDataStreamers++;
	FILE_LOG(logDEBUG) << "Number of DataStreamers: " << NumberofDataStreamers;

	strcpy(fileNametoStream, "");
}


DataStreamer::~DataStreamer() {
	CloseZmqSocket();
	if (completeBuffer) delete [] completeBuffer;
	ThreadObject::DestroyThread();
	NumberofDataStreamers--;
}

/** static functions */

uint64_t DataStreamer::GetErrorMask() {
	return ErrorMask;
}

uint64_t DataStreamer::GetRunningMask() {
	return RunningMask;
}

void DataStreamer::ResetRunningMask() {
	RunningMask = 0x0;
}

void DataStreamer::SetSilentMode(bool mode) {
	SilentMode = mode;
}


/** non static functions */
/** getters */
string DataStreamer::GetType(){
	return TypeName;
}

bool DataStreamer::IsRunning() {
	return ((1 << index) & RunningMask);
}


/** setters */
void DataStreamer::StartRunning() {
	pthread_mutex_lock(&Mutex);
	RunningMask |= (1<<index);
	pthread_mutex_unlock(&Mutex);
}


void DataStreamer::StopRunning() {
	pthread_mutex_lock(&Mutex);
	RunningMask ^= (1<<index);
	pthread_mutex_unlock(&Mutex);
}

void DataStreamer::SetFifo(Fifo*& f) {
	fifo = f;
}

void DataStreamer::ResetParametersforNewAcquisition() {
	firstAcquisitionIndex = 0;
	acquisitionStartedFlag = false;
}

void DataStreamer::ResetParametersforNewMeasurement(char* fname){
	firstMeasurementIndex = 0;
	measurementStartedFlag = false;
	strcpy(fileNametoStream, fname);
	if (completeBuffer) {
		delete [] completeBuffer;
		completeBuffer = 0;
	}
	if (*shortFrameEnable >= 0) {
		completeBuffer = new char[generalData->imageSizeComplete];
		memset(completeBuffer, 0, generalData->imageSizeComplete);
	}
}


void DataStreamer::RecordFirstIndices(uint64_t fnum) {
	measurementStartedFlag = true;
	firstMeasurementIndex = fnum;

	//start of entire acquisition
	if (!acquisitionStartedFlag) {
		acquisitionStartedFlag = true;
		firstAcquisitionIndex = fnum;
	}

#ifdef VERBOSE
	cprintf(BLUE,"%d First Acquisition Index:%lld\tFirst Measurement Index:%lld\n",
			index, (long long int)firstAcquisitionIndex, (long long int)firstMeasurementIndex);
#endif
}


void DataStreamer::SetGeneralData(GeneralData* g) {
	generalData = g;
#ifdef VERY_VERBOSE
	generalData->Print();
#endif
}

int DataStreamer::SetThreadPriority(int priority) {
	struct sched_param param;
	param.sched_priority = priority;
	if (pthread_setschedparam(thread, SCHED_FIFO, &param) == EPERM)
		return FAIL;
	FILE_LOG(logINFO) << "Streamer Thread Priority set to " << priority;
	return OK;
}


int DataStreamer::CreateZmqSockets(int* nunits, uint32_t port, const char* srcip) {
	uint32_t portnum = port + index;

	zmqSocket = new ZmqSocket(portnum, (strlen(srcip)?srcip:NULL));
	if (zmqSocket->IsError()) {
		cprintf(RED, "Error: Could not create Zmq socket on port %d for Streamer %d\n", portnum, index);
		return FAIL;
	}
	FILE_LOG(logINFO) << index << " Streamer: Zmq Server started at " << zmqSocket->GetZmqServerAddress();
	return OK;
}


void DataStreamer::CloseZmqSocket() {
	if (zmqSocket) {
		delete zmqSocket;
		zmqSocket = 0;
	}
}


void DataStreamer::ThreadExecution() {
	char* buffer=0;
	fifo->PopAddressToStream(buffer);
#ifdef FIFODEBUG
	if (!index) cprintf(BLUE,"DataStreamer %d, pop 0x%p buffer:%s\n", index,(void*)(buffer),buffer);
#endif

	//check dummy
	uint32_t numBytes = (uint32_t)(*((uint32_t*)buffer));
#ifdef VERBOSE
	cprintf(GREEN,"DataStreamer %d, Numbytes:%u\n", index,numBytes);
#endif
	if (numBytes == DUMMY_PACKET_VALUE) {
		StopProcessing(buffer);
		return;
	}

	ProcessAnImage(buffer);

	//free
	fifo->FreeAddress(buffer);

}



void DataStreamer::StopProcessing(char* buf) {
#ifdef VERBOSE
	if (!index)
		cprintf(RED,"DataStreamer %d: Dummy\n", index);
#endif
	sls_detector_header* header = (sls_detector_header*) (buf);
	//send dummy header and data
	if (!SendHeader(header, 0, 0, 0, true))
		cprintf(RED,"Error: Could not send zmq dummy header for streamer %d\n", index);

	fifo->FreeAddress(buf);
	StopRunning();
#ifdef VERBOSE
	FILE_LOG(logINFO) << index << ": Streaming Completed";
#endif
}

/** buf includes only the standard header */
void DataStreamer::ProcessAnImage(char* buf) {

	sls_detector_header* header = (sls_detector_header*) (buf + FIFO_HEADER_NUMBYTES);
	uint64_t fnum = header->frameNumber;
#ifdef VERBOSE
	cprintf(MAGENTA,"DataStreamer %d: fnum:%lu\n", index,fnum);
#endif

	if (!measurementStartedFlag) {
#ifdef VERBOSE
		if (!index) cprintf(MAGENTA,"DataStreamer %d: fnum:%lu\n", index, fnum);
#endif
		RecordFirstIndices(fnum);
	}

	//shortframe gotthard
	if (completeBuffer) {

		if (!SendHeader(header, (uint32_t)(*((uint32_t*)buf)), generalData->nPixelsXComplete, generalData->nPixelsYComplete, false))
			cprintf(RED,"Error: Could not send zmq header for fnum %lld and streamer %d\n",
					(long long int) fnum, index);

		memcpy(completeBuffer + ((generalData->imageSize)**shortFrameEnable), buf + FIFO_HEADER_NUMBYTES + sizeof(sls_detector_header), (uint32_t)(*((uint32_t*)buf)) ); // new size possibly from callback
		if (!zmqSocket->SendData(completeBuffer, generalData->imageSizeComplete))
			cprintf(RED,"Error: Could not send zmq data for fnum %lld and streamer %d\n",
					(long long int) fnum, index);
	}


	//normal
	else {

		if (!SendHeader(header, (uint32_t)(*((uint32_t*)buf)), generalData->nPixelsX, generalData->nPixelsY, false)) // new size possibly from callback
			cprintf(RED,"Error: Could not send zmq header for fnum %lld and streamer %d\n",
					(long long int) fnum, index);

		if (!zmqSocket->SendData(buf + FIFO_HEADER_NUMBYTES + sizeof(sls_detector_header), (uint32_t)(*((uint32_t*)buf)) )) // new size possibly from callback
			cprintf(RED,"Error: Could not send zmq data for fnum %lld and streamer %d\n",
					(long long int) fnum, index);
	}
}



int DataStreamer::SendHeader(sls_detector_header* header, uint32_t size, uint32_t nx, uint32_t ny, bool dummy) {

	if (dummy)
		return  zmqSocket->SendHeaderData(index, dummy,SLS_DETECTOR_JSON_HEADER_VERSION);

	uint64_t frameIndex = header->frameNumber - firstMeasurementIndex;
	uint64_t acquisitionIndex = header->frameNumber - firstAcquisitionIndex;

	return zmqSocket->SendHeaderData(index, dummy, SLS_DETECTOR_JSON_HEADER_VERSION, *dynamicRange, *fileIndex,
			nx, ny, size,
			acquisitionIndex, frameIndex, fileNametoStream,
			header->frameNumber, header->expLength, header->packetNumber, header->bunchId, header->timestamp,
			header->modId, header->xCoord, header->yCoord, header->zCoord,
			header->debug, header->roundRNumber,
			header->detType, header->version,
			flippedData,
			additionJsonHeader
			);
}



int DataStreamer::restreamStop() {
	//send dummy header
	int ret = zmqSocket->SendHeaderData(index, true, SLS_DETECTOR_JSON_HEADER_VERSION);
	if (!ret) {
		FILE_LOG(logERROR) << "Could not Restream Dummy Header via ZMQ for port " << zmqSocket->GetPortNumber();
		return FAIL;
	}
	return OK;
}
