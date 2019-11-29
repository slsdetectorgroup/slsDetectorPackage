/************************************************
 * @file DataStreamer.cpp
 * @short streams data from receiver via ZMQ
 ***********************************************/


#include "DataStreamer.h"
#include "Fifo.h"
#include "GeneralData.h"
#include "ZmqSocket.h"
#include "sls_detector_exceptions.h"

#include <cerrno>
#include <iostream>

const std::string DataStreamer::TypeName = "DataStreamer";


DataStreamer::DataStreamer(int ind, Fifo* f, uint32_t* dr, ROI* r,
		uint64_t* fi, int fd, std::string* ajh, int* nd, bool* gpEnable, bool* qe) :
		ThreadObject(ind, TypeName),
		runningFlag(0),
		generalData(nullptr),
		fifo(f),
		zmqSocket(nullptr),
		dynamicRange(dr),
		roi(r),
		adcConfigured(-1),
		fileIndex(fi),
		flippedDataX(fd),
		additionJsonHeader(ajh),
		startedFlag(false),
		firstIndex(0),
		completeBuffer(nullptr),
		gapPixelsEnable(gpEnable),
		quadEnable(qe)
{
	numDet[0] = nd[0];
	numDet[1] = nd[1];
	
    FILE_LOG(logDEBUG) << "DataStreamer " << ind << " created";
}


DataStreamer::~DataStreamer() {
	CloseZmqSocket();
	delete [] completeBuffer;
}

/** getters */

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

void DataStreamer::SetFifo(Fifo* f) {
	fifo = f;
}

void DataStreamer::ResetParametersforNewAcquisition(const std::string& fname){
    runningFlag = false;
	startedFlag = false;
	firstIndex = 0;

    fileNametoStream = fname;
    if (completeBuffer) {
            delete[] completeBuffer;
            completeBuffer = nullptr;
	}
	if (roi->xmin != -1) {
		if (generalData->myDetectorType == GOTTHARD) {
			adcConfigured = generalData->GetAdcConfigured(index, *roi);
		}
		completeBuffer = new char[generalData->imageSizeComplete];
		memset(completeBuffer, 0, generalData->imageSizeComplete);
	}
}

void DataStreamer::RecordFirstIndex(uint64_t fnum) {
	startedFlag = true;
	firstIndex = fnum;

	FILE_LOG(logDEBUG1) << index << " First Index: " << firstIndex;
}

void DataStreamer::SetGeneralData(GeneralData* g) {
	generalData = g;
	generalData->Print();
}

void DataStreamer::SetNumberofDetectors(int* nd) {
	numDet[0] = nd[0];
	numDet[1] = nd[1];
}

void DataStreamer::SetFlippedDataX(int fd) {
	flippedDataX = fd;
}

void DataStreamer::CreateZmqSockets(int* nunits, uint32_t port, const sls::IpAddr ip) {
	uint32_t portnum = port + index;
	std::string sip = ip.str();
	try {
		zmqSocket = new ZmqSocket(portnum, (ip != 0? sip.c_str(): nullptr));
	} catch (...) {
		FILE_LOG(logERROR) << "Could not create Zmq socket on port " << portnum << " for Streamer " << index;
		throw;
	}
	FILE_LOG(logINFO) << index << " Streamer: Zmq Server started at " << zmqSocket->GetZmqServerAddress();
}


void DataStreamer::CloseZmqSocket() {
	if (zmqSocket) {
		delete zmqSocket;
		zmqSocket = nullptr;
	}
}


void DataStreamer::ThreadExecution() {
	char* buffer=nullptr;
	fifo->PopAddressToStream(buffer);
	FILE_LOG(logDEBUG5) << "DataStreamer " << index << ", "
			"pop 0x" << std::hex << (void*)(buffer) << std::dec << ":" << buffer;


	//check dummy
	uint32_t numBytes = (uint32_t)(*((uint32_t*)buffer));
	FILE_LOG(logDEBUG1) << "DataStreamer " << index << ", Numbytes:" << numBytes;
	if (numBytes == DUMMY_PACKET_VALUE) {
		StopProcessing(buffer);
		return;
	}

	ProcessAnImage(buffer);

	//free
	fifo->FreeAddress(buffer);

}



void DataStreamer::StopProcessing(char* buf) {
	FILE_LOG(logDEBUG1) << "DataStreamer " << index << ": Dummy";

	sls_receiver_header* header = (sls_receiver_header*) (buf);
	//send dummy header and data
	if (!SendHeader(header, 0, 0, 0, true)) {
		FILE_LOG(logERROR) << "Could not send zmq dummy header for streamer " << index;
	}

	fifo->FreeAddress(buf);
	StopRunning();
	FILE_LOG(logDEBUG1) << index << ": Streaming Completed";
}

/** buf includes only the standard header */
void DataStreamer::ProcessAnImage(char* buf) {

	sls_receiver_header* header = (sls_receiver_header*) (buf + FIFO_HEADER_NUMBYTES);
	uint64_t fnum = header->detHeader.frameNumber;
	FILE_LOG(logDEBUG1) << "DataStreamer " << index << ": fnum:" << fnum;

	if (!startedFlag) {
		RecordFirstIndex(fnum);
	}

	//shortframe gotthard
	if (completeBuffer) {

		//disregarding the size modified from callback (always using imageSizeComplete
		// instead of buf (32 bit) because gui needs imagesizecomplete and listener
		//write imagesize

		if (!SendHeader(header, generalData->imageSizeComplete,
				generalData->nPixelsXComplete, generalData->nPixelsYComplete, false)) {
			FILE_LOG(logERROR) << "Could not send zmq header for fnum " << fnum << " and streamer " << index;
		}
		memcpy(completeBuffer + ((generalData->imageSize) * adcConfigured),
				buf + FIFO_HEADER_NUMBYTES + sizeof(sls_receiver_header),
				(uint32_t)(*((uint32_t*)buf)) );

		if (!zmqSocket->SendData(completeBuffer, generalData->imageSizeComplete)) {
			FILE_LOG(logERROR) << "Could not send zmq data for fnum " << fnum << " and streamer " << index;
		}
	}


	//normal
	else {

		if (!SendHeader(header, (uint32_t)(*((uint32_t*)buf)),
				generalData->nPixelsX, generalData->nPixelsY, false))  {// new size possibly from callback
			FILE_LOG(logERROR) << "Could not send zmq header for fnum " << fnum << " and streamer " << index;
		}
		if (!zmqSocket->SendData(buf + FIFO_HEADER_NUMBYTES + sizeof(sls_receiver_header),
				(uint32_t)(*((uint32_t*)buf)) )) {// new size possibly from callback
			FILE_LOG(logERROR) << "Could not send zmq data for fnum " << fnum << " and streamer " << index;
		}
	}
}



int DataStreamer::SendHeader(sls_receiver_header* rheader, uint32_t size, uint32_t nx, uint32_t ny, bool dummy) {

	if (dummy)
		return  zmqSocket->SendHeaderData(index, dummy,SLS_DETECTOR_JSON_HEADER_VERSION);

	sls_detector_header header = rheader->detHeader;

	uint64_t frameIndex = header.frameNumber - firstIndex;
	uint64_t acquisitionIndex = header.frameNumber;

	return zmqSocket->SendHeaderData(index, dummy, SLS_DETECTOR_JSON_HEADER_VERSION, *dynamicRange, *fileIndex,
			numDet[0], numDet[1], nx, ny, size,
			acquisitionIndex, frameIndex, fileNametoStream.c_str(),
			header.frameNumber, header.expLength, header.packetNumber, header.bunchId, header.timestamp,
			header.modId, header.row, header.column, header.reserved,
			header.debug, header.roundRNumber,
			header.detType, header.version,
			*gapPixelsEnable ? 1 : 0, flippedDataX, *quadEnable,
			additionJsonHeader
			);
}



void DataStreamer::RestreamStop() {
	//send dummy header
	int ret = zmqSocket->SendHeaderData(index, true, SLS_DETECTOR_JSON_HEADER_VERSION);
	if (!ret) {
		throw sls::RuntimeError("Could not restream Dummy Header via ZMQ for port " + std::to_string(zmqSocket->GetPortNumber()));
	}
}


