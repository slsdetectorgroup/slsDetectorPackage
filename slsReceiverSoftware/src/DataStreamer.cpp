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

const std::string DataStreamer::TypeName = "DataStreamer";


DataStreamer::DataStreamer(int ind, Fifo*& f, uint32_t* dr, std::vector<ROI>* r,
		uint64_t* fi, int* fd, char* ajh, bool* sm) :
		ThreadObject(ind),
		runningFlag(0),
		generalData(0),
		fifo(f),
		zmqSocket(0),
		dynamicRange(dr),
		roi(r),
		adcConfigured(-1),
		fileIndex(fi),
		flippedData(fd),
		additionJsonHeader(ajh),
		acquisitionStartedFlag(false),
		measurementStartedFlag(false),
		firstAcquisitionIndex(0),
		firstMeasurementIndex(0),
		completeBuffer(0)
{
    if(ThreadObject::CreateThread() == FAIL)
        throw std::exception();

    FILE_LOG(logDEBUG) << "DataStreamer " << ind << " created";

	memset(fileNametoStream, 0, MAX_STR_LENGTH);
}


DataStreamer::~DataStreamer() {
	CloseZmqSocket();
	if (completeBuffer) delete [] completeBuffer;
	ThreadObject::DestroyThread();
}

/** getters */
std::string DataStreamer::GetType(){
	return TypeName;
}

bool DataStreamer::IsRunning() {
	return runningFlag;
}


/** setters */
void DataStreamer::StartRunning() {
    runningFlag = true;
}


void DataStreamer::StopRunning() {
    runningFlag = false;
}

void DataStreamer::SetFifo(Fifo*& f) {
	fifo = f;
}

void DataStreamer::ResetParametersforNewAcquisition() {
	firstAcquisitionIndex = 0;
	acquisitionStartedFlag = false;
}

void DataStreamer::ResetParametersforNewMeasurement(char* fname){
    runningFlag = false;
	firstMeasurementIndex = 0;
	measurementStartedFlag = false;
	strcpy(fileNametoStream, fname);
	if (completeBuffer) {
		delete [] completeBuffer;
		completeBuffer = 0;
	}
	if (roi->size()) {
		if (generalData->myDetectorType == GOTTHARD) {
			adcConfigured = generalData->GetAdcConfigured(index, roi);
		}
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


void DataStreamer::CreateZmqSockets(int* nunits, uint32_t port, const char* srcip) {
	uint32_t portnum = port + index;

	try {
		zmqSocket = new ZmqSocket(portnum, (strlen(srcip)?srcip:NULL));
	} catch (...) {
		cprintf(RED, "Error: Could not create Zmq socket on port %d for Streamer %d\n", portnum, index);
		throw;
	}
	FILE_LOG(logINFO) << index << " Streamer: Zmq Server started at " << zmqSocket->GetZmqServerAddress();
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
	sls_receiver_header* header = (sls_receiver_header*) (buf);
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

	sls_receiver_header* header = (sls_receiver_header*) (buf + FIFO_HEADER_NUMBYTES);
	uint64_t fnum = header->detHeader.frameNumber;
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

		//disregarding the size modified from callback (always using imageSizeComplete
		// instead of buf (32 bit) because gui needs imagesizecomplete and listener
		//write imagesize

		if (!SendHeader(header, generalData->imageSizeComplete,
				generalData->nPixelsXComplete, generalData->nPixelsYComplete, false))
			cprintf(RED,"Error: Could not send zmq header for fnum %lld and streamer %d\n",
					(long long int) fnum, index);

		memcpy(completeBuffer + ((generalData->imageSize) * adcConfigured),
				buf + FIFO_HEADER_NUMBYTES + sizeof(sls_receiver_header),
				(uint32_t)(*((uint32_t*)buf)) );

		if (!zmqSocket->SendData(completeBuffer, generalData->imageSizeComplete))
			cprintf(RED,"Error: Could not send zmq data for fnum %lld and streamer %d\n",
					(long long int) fnum, index);
	}


	//normal
	else {

		if (!SendHeader(header, (uint32_t)(*((uint32_t*)buf)),
				generalData->nPixelsX, generalData->nPixelsY, false)) // new size possibly from callback
			cprintf(RED,"Error: Could not send zmq header for fnum %lld and streamer %d\n",
					(long long int) fnum, index);

		if (!zmqSocket->SendData(buf + FIFO_HEADER_NUMBYTES + sizeof(sls_receiver_header),
				(uint32_t)(*((uint32_t*)buf)) )) // new size possibly from callback
			cprintf(RED,"Error: Could not send zmq data for fnum %lld and streamer %d\n",
					(long long int) fnum, index);
	}
}



int DataStreamer::SendHeader(sls_receiver_header* rheader, uint32_t size, uint32_t nx, uint32_t ny, bool dummy) {

	if (dummy)
		return  zmqSocket->SendHeaderData(index, dummy,SLS_DETECTOR_JSON_HEADER_VERSION);

	sls_detector_header header = rheader->detHeader;

	uint64_t frameIndex = header.frameNumber - firstMeasurementIndex;
	uint64_t acquisitionIndex = header.frameNumber - firstAcquisitionIndex;

	return zmqSocket->SendHeaderData(index, dummy, SLS_DETECTOR_JSON_HEADER_VERSION, *dynamicRange, *fileIndex,
			nx, ny, size,
			acquisitionIndex, frameIndex, fileNametoStream,
			header.frameNumber, header.expLength, header.packetNumber, header.bunchId, header.timestamp,
			header.modId, header.row, header.column, header.reserved,
			header.debug, header.roundRNumber,
			header.detType, header.version,
			flippedData,
			additionJsonHeader
			);
}



int DataStreamer::RestreamStop() {
	//send dummy header
	int ret = zmqSocket->SendHeaderData(index, true, SLS_DETECTOR_JSON_HEADER_VERSION);
	if (!ret) {
		FILE_LOG(logERROR) << "Could not Restream Dummy Header via ZMQ for port " << zmqSocket->GetPortNumber();
		return FAIL;
	}
	return OK;
}


