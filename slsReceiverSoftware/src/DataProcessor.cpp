/************************************************
 * @file DataProcessor.cpp
 * @short creates data processor thread that
 * pulls pointers to memory addresses from fifos
 * and processes data stored in them & writes them to file
 ***********************************************/


#include "DataProcessor.h"
#include "BinaryFile.h"
#include "Fifo.h"
#include "GeneralData.h"
#ifdef HDF5C
#include "HDF5File.h"
#endif
#include "DataStreamer.h"
#include "sls_detector_exceptions.h"

#include <cerrno>
#include <cstring>
#include <iostream>

const std::string DataProcessor::TypeName = "DataProcessor";


DataProcessor::DataProcessor(int ind, detectorType dtype, Fifo* f,
		fileFormat* ftype, bool fwenable, bool* mfwenable, 
		bool* dsEnable, bool* gpEnable, uint32_t* dr,
		uint32_t* freq, uint32_t* timer,
		bool* fp, bool* act, bool* depaden, bool* sm, bool* qe,
		std::vector <int> * cdl, int* cdo, int* cad) :

		ThreadObject(ind),
		runningFlag(false),
		generalData(nullptr),
		fifo(f),
		myDetectorType(dtype),
		file(nullptr),
		dataStreamEnable(dsEnable),
		fileFormatType(ftype),
		fileWriteEnable(fwenable),
		masterFileWriteEnable(mfwenable),
		gapPixelsEnable(gpEnable),
		dynamicRange(dr),
		streamingFrequency(freq),
		streamingTimerInMs(timer),
		currentFreqCount(0),
		tempBuffer(nullptr),
		activated(act),
		deactivatedPaddingEnable(depaden),
        silentMode(sm),
		quadEnable(qe),
		framePadding(fp),
		ctbDbitList(cdl),
		ctbDbitOffset(cdo),
		ctbAnalogDataBytes(cad),
		acquisitionStartedFlag(false),
		measurementStartedFlag(false),
		firstAcquisitionIndex(0),
		firstMeasurementIndex(0),
		numTotalFramesCaught(0),
		numFramesCaught(0),
		currentFrameIndex(0),
		rawDataReadyCallBack(nullptr),
		rawDataModifyReadyCallBack(nullptr),
		pRawDataReady(nullptr)
{
     if(ThreadObject::CreateThread() == FAIL)
         throw sls::RuntimeError("Could not create processing thread");

    FILE_LOG(logDEBUG) << "DataProcessor " << ind << " created";

	memset((void*)&timerBegin, 0, sizeof(timespec));
}


DataProcessor::~DataProcessor() {
	delete file;
	delete [] tempBuffer;
	ThreadObject::DestroyThread();
}

/** getters */
std::string DataProcessor::GetType(){
	return TypeName;
}

bool DataProcessor::IsRunning() {
	return runningFlag;
}

bool DataProcessor::GetAcquisitionStartedFlag(){
	return acquisitionStartedFlag;
}

bool DataProcessor::GetMeasurementStartedFlag(){
	return measurementStartedFlag;
}

uint64_t DataProcessor::GetNumTotalFramesCaught() {
	return numTotalFramesCaught;
}

uint64_t DataProcessor::GetNumFramesCaught() {
	return numFramesCaught;
}

uint64_t DataProcessor::GetActualProcessedAcquisitionIndex() {
	return currentFrameIndex;
}

uint64_t DataProcessor::GetProcessedAcquisitionIndex() {
	return currentFrameIndex - firstAcquisitionIndex;
}

uint64_t DataProcessor::GetProcessedMeasurementIndex() {
	return currentFrameIndex - firstMeasurementIndex;
}



/** setters */
void DataProcessor::StartRunning() {
    runningFlag = true;
}


void DataProcessor::StopRunning() {
    runningFlag = false;
}

void DataProcessor::SetFifo(Fifo* f) {
	fifo = f;
}

void DataProcessor::ResetParametersforNewAcquisition() {
	numTotalFramesCaught = 0;
	firstAcquisitionIndex = 0;
	currentFrameIndex = 0;
	acquisitionStartedFlag = false;
}

void DataProcessor::ResetParametersforNewMeasurement(){
    runningFlag = false;
	numFramesCaught = 0;
	firstMeasurementIndex = 0;
	measurementStartedFlag = false;

	if (tempBuffer != nullptr) {
		delete [] tempBuffer;
		tempBuffer = nullptr;
	}
	if (*gapPixelsEnable) {
		tempBuffer = new char[generalData->imageSize];
		memset(tempBuffer, 0, generalData->imageSize);
	}
}


void DataProcessor::RecordFirstIndices(uint64_t fnum) {
	//listen to this fnum, later +1
	currentFrameIndex = fnum;

	measurementStartedFlag = true;
	firstMeasurementIndex = fnum;

	//start of entire acquisition
	if (!acquisitionStartedFlag) {
		acquisitionStartedFlag = true;
		firstAcquisitionIndex = fnum;
	}

	FILE_LOG(logDEBUG1) << index << " First Acquisition Index:" << firstAcquisitionIndex <<
			"\tFirst Measurement Index:" << firstMeasurementIndex;
}


void DataProcessor::SetGeneralData(GeneralData* g) {
	generalData = g;
	generalData->Print();
	if (file != nullptr) {
		if (file->GetFileType() == HDF5) {
			file->SetNumberofPixels(generalData->nPixelsX, generalData->nPixelsY);
		}
	}
}


int DataProcessor::SetThreadPriority(int priority) {
	struct sched_param param;
	param.sched_priority = priority;
	if (pthread_setschedparam(thread, SCHED_FIFO, &param) == EPERM)
		return FAIL;
	FILE_LOG(logINFO) << "Processor Thread Priority set to " << priority;
	return OK;
}


void DataProcessor::SetFileFormat(const fileFormat f) {
	if ((file != nullptr) && file->GetFileType() != f) {
		//remember the pointer values before they are destroyed
		int nd[MAX_DIMENSIONS];nd[0] = 0; nd[1] = 0;
		uint32_t* maxf = nullptr;
		char* fname=nullptr; char* fpath=nullptr; uint64_t* findex=nullptr;
		bool* owenable=nullptr; int* dindex=nullptr; int* nunits=nullptr; uint64_t* nf = nullptr;
		uint32_t* dr = nullptr; uint32_t* port = nullptr;
		file->GetMemberPointerValues(nd, maxf, fname, fpath, findex,
				owenable, dindex, nunits, nf, dr, port);
		//create file writer with same pointers
		SetupFileWriter(fileWriteEnable, nd, maxf, fname, fpath, findex,
				owenable, dindex, nunits, nf, dr, port);
	}
}


void DataProcessor::SetupFileWriter(bool fwe, int* nd, uint32_t* maxf,
		char* fname, char* fpath, uint64_t* findex,
		bool* owenable, int* dindex, int* nunits, uint64_t* nf, uint32_t* dr,
		uint32_t* portno,
		GeneralData* g)
{
	fileWriteEnable = fwe;
	if (g != nullptr)
		generalData = g;


	if (file != nullptr) {
		delete file; file = nullptr;
	}

	if (fileWriteEnable) {
		switch(*fileFormatType){
#ifdef HDF5C
	case HDF5:
		file = new HDF5File(index, maxf,
				nd, fname, fpath, findex, owenable,
				dindex, nunits, nf, dr, portno,
				generalData->nPixelsX, generalData->nPixelsY, silentMode);
		break;
#endif
	default:
		file = new BinaryFile(index, maxf,
				nd, fname, fpath, findex, owenable,
				dindex, nunits, nf, dr, portno, silentMode);
		break;
		}
	}
}

// only the first file
int DataProcessor::CreateNewFile(bool en, uint64_t nf, uint64_t at, uint64_t st,
		uint64_t sp, uint64_t ap) {
	if (file == nullptr)
		return FAIL;
	file->CloseAllFiles();
	file->resetSubFileIndex();
	if (file->CreateMasterFile(*masterFileWriteEnable, en,	generalData->imageSize,
			generalData->nPixelsX, generalData->nPixelsY,
			at, st, sp, ap) == FAIL)
		return FAIL;
	if (file->CreateFile() == FAIL)
		return FAIL;
	return OK;
}


void DataProcessor::CloseFiles() {
	if (file != nullptr)
		file->CloseAllFiles();
}

void DataProcessor::EndofAcquisition(bool anyPacketsCaught, uint64_t numf) {
	if ((file != nullptr) && file->GetFileType() == HDF5) {
		file->EndofAcquisition(anyPacketsCaught, numf);
	}
}


void DataProcessor::ThreadExecution() {
	char* buffer=nullptr;
	fifo->PopAddress(buffer);
	FILE_LOG(logDEBUG5) << "DataProcessor " << index << ", "
			"pop 0x" << std::hex << (void*)(buffer) << std::dec << ":" << buffer;

	//check dummy
	auto numBytes = (uint32_t)(*((uint32_t*)buffer));
	FILE_LOG(logDEBUG1) << "DataProcessor " << index << ", Numbytes:" << numBytes;
	if (numBytes == DUMMY_PACKET_VALUE) {
		StopProcessing(buffer);
		return;
	}

	ProcessAnImage(buffer);

	//stream (if time/freq to stream) or free
	if (*dataStreamEnable && SendToStreamer())
		fifo->PushAddressToStream(buffer);
	else
		fifo->FreeAddress(buffer);
}


void DataProcessor::StopProcessing(char* buf) {
	FILE_LOG(logDEBUG1) << "DataProcessing " << index << ": Dummy";

	//stream or free
	if (*dataStreamEnable)
		fifo->PushAddressToStream(buf);
	else
		fifo->FreeAddress(buf);

	if (file != nullptr)
		file->CloseCurrentFile();
	StopRunning();
	FILE_LOG(logDEBUG1) << index << ": Processing Completed";
}


void DataProcessor::ProcessAnImage(char* buf) {

	auto* rheader = (sls_receiver_header*) (buf + FIFO_HEADER_NUMBYTES);
	sls_detector_header header = rheader->detHeader;
	uint64_t fnum = header.frameNumber;
	currentFrameIndex = fnum;
	uint32_t nump = header.packetNumber;
	if (nump == generalData->packetsPerFrame) {
		numFramesCaught++;
		numTotalFramesCaught++;
	}

	FILE_LOG(logDEBUG1) << "DataProcessing " << index << ": fnum:" << fnum;

	if (!measurementStartedFlag) {
		RecordFirstIndices(fnum);

		if (*dataStreamEnable) {
			//restart timer
			clock_gettime(CLOCK_REALTIME, &timerBegin);
			timerBegin.tv_sec -= (*streamingTimerInMs) / 1000;
			timerBegin.tv_nsec -= ((*streamingTimerInMs) % 1000) * 1000000;

			//to send first image
			currentFreqCount = *streamingFrequency;
		}
	}

	if (*gapPixelsEnable && (*dynamicRange!=4))
		InsertGapPixels(buf + FIFO_HEADER_NUMBYTES + sizeof(sls_receiver_header),
				*dynamicRange);


    // frame padding
    if (*activated && *framePadding && nump < generalData->packetsPerFrame)
        PadMissingPackets(buf);

    // deactivated and padding enabled
    else if (!(*activated) && *deactivatedPaddingEnable)
		PadMissingPackets(buf);

	// rearrange ctb digital bits (if ctbDbitlist is not empty)
	if (!(*ctbDbitList).empty()) {
		RearrangeDbitData(buf);
	}

	// normal call back
	if (rawDataReadyCallBack != nullptr) {
		rawDataReadyCallBack(
				(char*)rheader,
				buf + FIFO_HEADER_NUMBYTES + sizeof(sls_receiver_header),
				(uint32_t)(*((uint32_t*)buf)),
				pRawDataReady);
	}

	// call back with modified size
	else if (rawDataModifyReadyCallBack != nullptr) {
        auto revsize = (uint32_t)(*((uint32_t*)buf));
        rawDataModifyReadyCallBack(
        		(char*)rheader,
                buf + FIFO_HEADER_NUMBYTES + sizeof(sls_receiver_header),
                revsize,
                pRawDataReady);
        (*((uint32_t*)buf)) =  revsize;
    }
	

	// write to file
	if (file != nullptr)
		file->WriteToFile(buf + FIFO_HEADER_NUMBYTES,
				sizeof(sls_receiver_header) + (uint32_t)(*((uint32_t*)buf)), //+ size of data (resizable from previous call back
				fnum-firstMeasurementIndex, nump);

}



bool DataProcessor::SendToStreamer() {
	//skip
	if ((*streamingFrequency) == 0u) {
		if (!CheckTimer())
			return false;
	} else {
		if (!CheckCount())
			return false;
	}
	return true;
}


bool DataProcessor::CheckTimer() {
	struct timespec end;
	clock_gettime(CLOCK_REALTIME, &end);

	FILE_LOG(logDEBUG1) << index << " Timer elapsed time:" <<
			(( end.tv_sec - timerBegin.tv_sec ) + ( end.tv_nsec - timerBegin.tv_nsec ) / 1000000000.0)
			<< " seconds";
	//still less than streaming timer, keep waiting
	if((( end.tv_sec - timerBegin.tv_sec )	+ ( end.tv_nsec - timerBegin.tv_nsec )
			/ 1000000000.0) < ((double)*streamingTimerInMs/1000.00))
		return false;

	//restart timer
	clock_gettime(CLOCK_REALTIME, &timerBegin);
	return true;
}


bool DataProcessor::CheckCount() {
	if (currentFreqCount == *streamingFrequency ) {
		currentFreqCount = 1;
		return true;
	}
	currentFreqCount++;
	return false;
}


void DataProcessor::SetPixelDimension() {
	if (file != nullptr) {
		if (file->GetFileType() == HDF5) {
			file->SetNumberofPixels(generalData->nPixelsX, generalData->nPixelsY);
		}
	}
}

void DataProcessor::registerCallBackRawDataReady(void (*func)(char* ,
		char*, uint32_t, void*),void *arg) {
	rawDataReadyCallBack=func;
	pRawDataReady=arg;
}

void DataProcessor::registerCallBackRawDataModifyReady(void (*func)(char* ,
		char*, uint32_t&, void*),void *arg) {
	rawDataModifyReadyCallBack=func;
	pRawDataReady=arg;
}

void DataProcessor::PadMissingPackets(char* buf) {
	FILE_LOG(logDEBUG) << index << ": Padding Missing Packets";

	uint32_t pperFrame = generalData->packetsPerFrame;
	auto* header = (sls_receiver_header*) (buf + FIFO_HEADER_NUMBYTES);
	uint32_t nmissing = pperFrame - header->detHeader.packetNumber;
	sls_bitset pmask = header->packetsMask;

	uint32_t dsize = generalData->dataSize;
	uint32_t fifohsize = generalData->fifoBufferHeaderSize;
	uint32_t corrected_dsize = dsize - ((pperFrame * dsize) - generalData->imageSize);
	FILE_LOG(logDEBUG1) << "bitmask: " << pmask.to_string();

	for (unsigned int pnum = 0; pnum < pperFrame; ++pnum) {

		// not missing packet
		if (pmask[pnum])
			continue;

		// done with padding, exit loop earlier
		if (nmissing == 0u)
			break;

		FILE_LOG(logDEBUG) << "padding for " << index << " for pnum: " << pnum << std::endl;

		// missing packet
		switch(myDetectorType) {
		//for gotthard, 1st packet: 4 bytes fnum, CACA                     + CACA, 639*2 bytes data
		//              2nd packet: 4 bytes fnum, previous 1*2 bytes data  + 640*2 bytes data !!
		case GOTTHARD:
			if(pnum == 0u)
				memset(buf + fifohsize + (pnum * dsize), 0xFF, dsize-2);
			else
				memset(buf + fifohsize + (pnum * dsize), 0xFF, dsize+2);
			break;
		case CHIPTESTBOARD:
			if (pnum == (pperFrame-1))
				memset(buf + fifohsize + (pnum * dsize), 0xFF, corrected_dsize);
			else
				memset(buf + fifohsize + (pnum * dsize), 0xFF, dsize);
			break;
		default:
			memset(buf + fifohsize + (pnum * dsize), 0xFF, dsize);
			break;
		}
		--nmissing;
	}
}

/** ctb specific */
void DataProcessor::RearrangeDbitData(char* buf) {
	int totalSize = (int)(*((uint32_t*)buf));
	int ctbDigitalDataBytes = totalSize - (*ctbAnalogDataBytes) - (*ctbDbitOffset);

	// no digital data      
    if (ctbDigitalDataBytes == 0) {
        FILE_LOG(logWARNING) << "No digital data for call back, yet dbitlist is not empty.";
        return;
	}

	const int numSamples = (ctbDigitalDataBytes / sizeof(uint64_t));
	const int digOffset = FIFO_HEADER_NUMBYTES + sizeof(sls_receiver_header) + (*ctbAnalogDataBytes);

	// ceil as numResult8Bits could be decimal 
	const int numResult8Bits = ceil((double)(numSamples * (*ctbDbitList).size()) / 8.00); 
	uint8_t result[numResult8Bits];
	memset(result, 0, numResult8Bits * sizeof(uint8_t));
	uint8_t* dest = result;

	auto* source = (uint64_t*)(buf + digOffset + (*ctbDbitOffset));

	// loop through digital bit enable vector
	int bitoffset = 0;
    for (auto bi : (*ctbDbitList)) {
		// where numbits * numsamples is not a multiple of 8
		if (bitoffset != 0) {
			bitoffset = 0;
			++dest;
		}
		
		// loop through the frame digital data
        for (auto ptr = source; ptr < (source + numSamples);) {
			// get selected bit from each 8 bit
			uint8_t bit = (*ptr++ >> bi) & 1;
			*dest |= bit << bitoffset;
			++bitoffset;
			// extract destination in 8 bit batches
			if (bitoffset == 8) {
				bitoffset = 0;
				++dest;
			}
	    }
    }


	// copy back to buf and update size
	memcpy(buf + digOffset, result, numResult8Bits * sizeof(uint8_t));
	(*((uint32_t*)buf)) = numResult8Bits * sizeof(uint8_t);
}

/** eiger specific */
void DataProcessor::InsertGapPixels(char* buf, uint32_t dr) {

	memset(tempBuffer, 0xFF, generalData->imageSize);

	int rightChip = ((*quadEnable) ? 0 : index);	// quad enable, then faking both to be left chips
	const uint32_t nx = generalData->nPixelsX;
	const uint32_t ny = generalData->nPixelsY;
	const uint32_t npx = nx * ny;
	bool group3 = (*quadEnable) ? false : true; // if quad enabled, no last line for left chips
	char* srcptr = nullptr;
	char* dstptr = nullptr;

	const uint32_t b1px = generalData->imageSize / (npx); // not double as not dealing with 4 bit mode
	const uint32_t b2px = 2 * b1px;
	const uint32_t b1pxofst = (rightChip == 0 ? 0 : b1px); // left fpga (rightChip 0) has no extra 1px offset, but right fpga has
	const uint32_t b1chip = 256 * b1px;
	const uint32_t b1line = (nx * b1px);
	const uint32_t bgroup3chip = b1chip + (group3 ? b1px : 0);

	// copying line by line
	srcptr = buf;
	dstptr = tempBuffer + b1line + b1pxofst;		// left fpga (rightChip 0) has no extra 1px offset, but right fpga has
	for (uint32_t i = 0; i < (ny-1); ++i) {
		memcpy(dstptr, srcptr, b1chip);
		srcptr += b1chip;
		dstptr += (b1chip + b2px);
		memcpy(dstptr, srcptr, b1chip);
		srcptr += b1chip;
		dstptr += bgroup3chip;
	}

	// vertical filling of values
	{
		char* srcgp1 = nullptr; char* srcgp2 = nullptr; char* srcgp3 = nullptr;
		char* dstgp1 = nullptr; char* dstgp2 = nullptr; char* dstgp3 = nullptr;
		const uint32_t b3px = 3 * b1px;

		srcptr = tempBuffer + b1line;
		dstptr = tempBuffer + b1line;

		for (uint32_t i = 0; i < (ny-1); ++i) {
			srcgp1 = srcptr + b1pxofst + b1chip - b1px;
			dstgp1 = srcgp1 + b1px;
			srcgp2 = srcgp1 + b3px;
			dstgp2 = dstgp1 + b1px;
			if (group3) {
				if (rightChip == 0u) {
					srcgp3 = srcptr + b1line - b2px;
					dstgp3 = srcgp3 + b1px;
				} else {
					srcgp3 = srcptr + b1px;
					dstgp3 = srcptr;
				}
			}
			switch (dr) {
			case 8:
				(*((uint8_t*)srcgp1)) = (*((uint8_t*)srcgp1))/2;	(*((uint8_t*)dstgp1)) = (*((uint8_t*)srcgp1));
				(*((uint8_t*)srcgp2)) = (*((uint8_t*)srcgp2))/2;	(*((uint8_t*)dstgp2)) = (*((uint8_t*)srcgp2));
				if (group3) {
					(*((uint8_t*)srcgp3)) = (*((uint8_t*)srcgp3))/2;	(*((uint8_t*)dstgp3)) = (*((uint8_t*)srcgp3));
				}
				break;
			case 16:
				(*((uint16_t*)srcgp1)) = (*((uint16_t*)srcgp1))/2;	(*((uint16_t*)dstgp1)) = (*((uint16_t*)srcgp1));
				(*((uint16_t*)srcgp2)) = (*((uint16_t*)srcgp2))/2;	(*((uint16_t*)dstgp2)) = (*((uint16_t*)srcgp2));
				if (group3) {
					(*((uint16_t*)srcgp3)) = (*((uint16_t*)srcgp3))/2;	(*((uint16_t*)dstgp3)) = (*((uint16_t*)srcgp3));
				}
				break;
			default:
				(*((uint32_t*)srcgp1)) = (*((uint32_t*)srcgp1))/2;	(*((uint32_t*)dstgp1)) = (*((uint32_t*)srcgp1));
				(*((uint32_t*)srcgp2)) = (*((uint32_t*)srcgp2))/2;	(*((uint32_t*)dstgp2)) = (*((uint32_t*)srcgp2));
				if (group3) {
					(*((uint32_t*)srcgp3)) = (*((uint32_t*)srcgp3))/2;	(*((uint32_t*)dstgp3)) = (*((uint32_t*)srcgp3));
				}
				break;
			}
			srcptr += b1line;
			dstptr += b1line;
		}

	}

	// horizontal filling of values
	srcptr = tempBuffer + b1line;
	dstptr = tempBuffer;
	for (uint32_t i = 0; i < nx; ++i) {
		switch (dr) {
		case 8:	(*((uint8_t*)srcptr)) = (*((uint8_t*)srcptr))/2; (*((uint8_t*)dstptr)) = (*((uint8_t*)srcptr)); break;
		case 16:(*((uint16_t*)srcptr)) = (*((uint16_t*)srcptr))/2; (*((uint16_t*)dstptr)) = (*((uint16_t*)srcptr)); break;
		default:(*((uint32_t*)srcptr)) = (*((uint32_t*)srcptr))/2; (*((uint32_t*)dstptr)) = (*((uint32_t*)srcptr)); break;
		}
		srcptr += b1px;
		dstptr += b1px;
	}

	memcpy(buf, tempBuffer, generalData->imageSize);
	return;
}
