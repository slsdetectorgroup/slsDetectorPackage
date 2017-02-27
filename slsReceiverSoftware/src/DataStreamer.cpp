/************************************************
 * @file DataStreamer.h
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

const char* DataStreamer::jsonHeaderFormat_part1 =
		"{"
		"\"htype\":[\"chunk-1.0\"], "
		"\"type\":\"%s\", "
		"\"shape\":%s, ";

const char* DataStreamer::jsonHeaderFormat =
		"%s"
		"\"acqIndex\":%lld, "
		"\"fIndex\":%lld, "
		"\"subfnum\":%lld, "
		"\"fname\":\"%s\"}";


DataStreamer::DataStreamer(Fifo*& f, uint32_t* dr, uint32_t* freq, uint32_t* timer) :
		ThreadObject(NumberofDataStreamers),
		generalData(0),
		fifo(f),
		zmqSocket(0),
		dynamicRange(dr),
		streamingFrequency(freq),
		streamingTimerInMs(timer),
		currentFreqCount(0),
		currentHeader(0),
		acquisitionStartedFlag(false),
		measurementStartedFlag(false),
		firstAcquisitionIndex(0),
		firstMeasurementIndex(0)
{
	memset(timerBegin, 0xFF, sizeof(timespec));
	if(ThreadObject::CreateThread()){
		pthread_mutex_lock(&Mutex);
		ErrorMask ^= (1<<index);
		pthread_mutex_unlock(&Mutex);
	}

	NumberofDataStreamers++;
	FILE_LOG (logDEBUG) << "Number of DataStreamers: " << NumberofDataStreamers;

}


DataStreamer::~DataStreamer() {
	CloseZmqSocket();
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

void DataStreamer::ResetParametersforNewMeasurement(){
	firstMeasurementIndex = 0;
	measurementStartedFlag = false;

	CreateHeaderPart1();
}


void DataStreamer::CreateHeaderPart1() {
	char *type;
	switch (*dynamicRange) {
	case 4:		strcpy(type, "uint8");	break;
	case 8:		strcpy(type, "uint8");	break;
	case 16:	strcpy(type, "uint16");	break;
	case 32:	strcpy(type, "uint32");	break;
	default:
		strcpy(type, "unknown");
		cprintf(RED," Unknown datatype in json format\n");
		break;
	}

	char *shape= "[%d, %d]";
	sprintf(shape, shape, generalData->nPixelsX, generalData->nPixelsY);

	sprintf(currentHeader, jsonHeaderFormat_part1, type, shape);
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
	if (pthread_setschedparam(thread, SCHED_RR, &param) == EPERM)
		return FAIL;
	return OK;
}


int DataStreamer::CreateZmqSockets(int* dindex, int* nunits) {
	uint32_t portnum = DEFAULT_ZMQ_PORTNO + ((*dindex) * (*nunits) + index);
	printf("%d Streamer: Port number: %d\n", index, portnum);

	zmqSocket = new ZmqSocket(portnum);
	if (zmqSocket->GetErrorStatus()) {
		cprintf(RED, "Error: Could not create Zmq socket on port %d for Streamer %d\n", portnum, index);
		return FAIL;
	}
	printf("%d Streamer: Zmq Server started at %s\n",zmqSocket->GetZmqServerAddress());
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
	if (!index) cprintf(BLUE,"DataProcessor %d, pop 0x%p buffer:%s\n", index,(void*)(buffer),buffer);
#endif

	//check dummy
	uint32_t numBytes = (uint32_t)(*((uint32_t*)buffer));
	if (numBytes == DUMMY_PACKET_VALUE) {
		StopProcessing(buffer);
		return;
	}

	ProcessAnImage(buffer + FIFO_HEADER_NUMBYTES);

	//free
	fifo->FreeAddress(buffer);

}



void DataStreamer::StopProcessing(char* buf) {
	fifo->FreeAddress(buf);
	StopRunning();
	cprintf(MAGENTA,"%d: Streaming Completed\n", index);
}


void DataStreamer::ProcessAnImage(char* buf) {
	uint64_t fnum = (*((uint64_t*)buf));
#ifdef VERBOSE
	if (!index) cprintf(MAGENTA,"DataStreamer %d: fnum:%lld\n", index, (long long int)fnum);
#endif

	if (!measurementStartedFlag) {
#ifdef VERBOSE
		if (!index) cprintf(MAGENTA,"DataStreamer %d: fnum:%lld\n", index, (long long int)fnum);
#endif
		RecordFirstIndices(fnum);
		//restart timer
		clock_gettime(CLOCK_REALTIME, &timerBegin);
		//to send first image
		currentFreqCount = *streamingFrequency;
	}

	//skip
	if (!(*streamingFrequency)) {
		if (!CheckTimer())
			return;
	} else {
		if (!CheckCount())
			return;
	}

	if(!SendHeader(fnum))
		cprintf(RED,"Error: Could not send zmq header for fnum %lld and streamer %d\n",
				(long long int) fnum, index);


	Send Datat();
}

bool DataStreamer::CheckTimer() {
	struct timespec end;
	clock_gettime(CLOCK_REALTIME, &end);
#ifdef VERBOSE
	cprintf(BLUE,"%d Timer elapsed time:%f seconds\n", index, ( end.tv_sec - timerBegin.tv_sec ) + ( end.tv_nsec - timerBegin.tv_nsec ) / 1000000000.0);
#endif
	//still less than streaming timer, keep waiting
	if((( end.tv_sec - timerBegin.tv_sec )	+ ( end.tv_nsec - timerBegin.tv_nsec ) / 1000000000.0) < (streamingTimerInMs/1000))
		return false;

	//restart timer
	clock_gettime(CLOCK_REALTIME, &timerBegin);
	return true;
}

bool DataStreamer::CheckCount() {
	if (currentFreqCount == *streamingFrequency ) {
		currentFreqCount = 1;
		return true;
	}
	currentFreqCount++;
	return false;
}

int DataStreamer::SendHeader(uint64_t fnum) {
	uint64_t frameIndex = fnum - firstMeasurementIndex;
	uint64_t acquisitionIndex = fnum - firstAcquisitionIndex;
	uint64_t subframeIndex = -1; /* subframe to be included in fifo buffer? */
	char buf[1000];
	int len = sprintf(buf, jsonHeaderFormat, jsonHeaderFormat_part1, acquisitionIndex, frameIndex, subframeIndex,completeFileName[ithread]);
	return zmqSocket->SendDataOnly(buf, len);
}
