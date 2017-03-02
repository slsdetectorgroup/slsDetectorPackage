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
		"\"version\":%.1f, "
		"\"type\":\"%s\", "
		"\"shape\":[%d, %d], ";

const char* DataStreamer::jsonHeaderFormat =
		"%s"
		"\"acqIndex\":%lld, "
		"\"fIndex\":%lld, "
		"\"subfnum\":%u, "
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
	if(ThreadObject::CreateThread()){
		pthread_mutex_lock(&Mutex);
		ErrorMask ^= (1<<index);
		pthread_mutex_unlock(&Mutex);
	}

	NumberofDataStreamers++;
	FILE_LOG (logDEBUG) << "Number of DataStreamers: " << NumberofDataStreamers;

	memset((void*)&timerBegin, 0, sizeof(timespec));
	currentHeader = new char[255];
	strcpy(fileNametoStream, "");
}


DataStreamer::~DataStreamer() {
	CloseZmqSocket();
	delete currentHeader;
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

void DataStreamer::ResetParametersforNewMeasurement(char* fname){
	firstMeasurementIndex = 0;
	measurementStartedFlag = false;
	strcpy(fileNametoStream, fname);
	cprintf(BLUE,"fname:%s\n",fname);

	CreateHeaderPart1();
}


void DataStreamer::CreateHeaderPart1() {
	char type[10] = "";
	switch (*dynamicRange) {
	case 4:		strcpy(type, "uint4");	break;
	case 8:		strcpy(type, "uint8");	break;
	case 16:	strcpy(type, "uint16");	break;
	case 32:	strcpy(type, "uint32");	break;
	default:
		strcpy(type, "unknown");
		cprintf(RED," Unknown datatype in json format %d\n", *dynamicRange);
		break;
	}

	sprintf(currentHeader, jsonHeaderFormat_part1,
			STREAMER_VERSION, type, generalData->nPixelsX, generalData->nPixelsY);
#ifdef VERBOSE
	cprintf(BLUE, "%d currentheader: %s\n", index, currentHeader);
#endif
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

	zmqSocket = new ZmqSocket(portnum);
	if (zmqSocket->IsError()) {
		cprintf(RED, "Error: Could not create Zmq socket on port %d for Streamer %d\n", portnum, index);
		return FAIL;
	}
	printf("%d Streamer: Zmq Server started at %s\n",index, zmqSocket->GetZmqServerAddress());
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

	//send dummy header and data
	if (!SendHeader(0, true))
		cprintf(RED,"Error: Could not send zmq dummy header for streamer %d\n", index);

	if (!zmqSocket->SendData((char*)DUMMY_MSG, DUMMY_MSG_SIZE))
		cprintf(RED,"Error: Could not send zmq dummy message for streamer %d\n", index);

	fifo->FreeAddress(buf);
	StopRunning();
#ifdef VERBOSE
	printf("%d: Streaming Completed\n", index);
#endif
}


void DataStreamer::ProcessAnImage(char* buf) {
	uint64_t fnum = (*((uint64_t*)buf));
	uint32_t snum = (*((uint32_t*)(buf + FILE_FRAME_HDR_FNUM_SIZE)));
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

	if (!SendHeader(fnum, snum))
		cprintf(RED,"Error: Could not send zmq header for fnum %lld and streamer %d\n",
				(long long int) fnum, index);

	if (!zmqSocket->SendData(buf + FILE_FRAME_HEADER_SIZE, generalData->imageSize))
		cprintf(RED,"Error: Could not send zmq data for fnum %lld and streamer %d\n",
						(long long int) fnum, index);
}



bool DataStreamer::CheckTimer() {
	struct timespec end;
	clock_gettime(CLOCK_REALTIME, &end);
#ifdef VERBOSE
	cprintf(BLUE,"%d Timer elapsed time:%f seconds\n", index, ( end.tv_sec - timerBegin.tv_sec ) + ( end.tv_nsec - timerBegin.tv_nsec ) / 1000000000.0);
#endif
	//still less than streaming timer, keep waiting
	if((( end.tv_sec - timerBegin.tv_sec )	+ ( end.tv_nsec - timerBegin.tv_nsec ) / 1000000000.0) < (*streamingTimerInMs/1000))
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


int DataStreamer::SendHeader(uint64_t fnum, uint32_t snum, bool dummy) {
	uint64_t frameIndex = -1;
	uint64_t acquisitionIndex = -1;
	uint32_t subframeIndex = -1;
	char buf[1000] = "";

	if (!dummy) {
		frameIndex = fnum - firstMeasurementIndex;
		acquisitionIndex = fnum - firstAcquisitionIndex;
		subframeIndex = snum;
	}

	int len = sprintf(buf, jsonHeaderFormat, currentHeader, acquisitionIndex, frameIndex, subframeIndex, fileNametoStream);
#ifdef VERBOSE
	printf("%d Streamer: buf:%s\n", index, buf);
#endif
	return zmqSocket->SendHeaderData(buf, len);
}
