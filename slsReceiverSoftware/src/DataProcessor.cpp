/************************************************
 * @file DataProcessor.h
 * @short creates data processor thread that
 * pulls pointers to memory addresses from fifos
 * and processes data stored in them & writes them to file
 ***********************************************/


#include "DataProcessor.h"
#include "GeneralData.h"
#include "Fifo.h"
#include "BinaryFile.h"
#ifdef HDF5C
#include "HDF5File.h"
#endif

#include <iostream>
#include <cstring>
using namespace std;

const string DataProcessor::TypeName = "DataProcessor";

int DataProcessor::NumberofDataProcessors(0);

uint64_t DataProcessor::ErrorMask(0x0);

uint64_t DataProcessor::RunningMask(0x0);

pthread_mutex_t DataProcessor::Mutex = PTHREAD_MUTEX_INITIALIZER;


DataProcessor::DataProcessor(Fifo*& f, runStatus* s, pthread_mutex_t* m, fileFormat* ftype, bool* fwenable,
		int* cbaction,
		void (*dataReadycb)(int, char*, int, FILE*, char*, void*),
		void *pDataReadycb) :
		ThreadObject(NumberofDataProcessors),
		generalData(0),
		fifo(f),
		acquisitionStartedFlag(false),
		measurementStartedFlag(false),
		status(s),
		statusMutex(m),
		numTotalFramesCaught(0),
		numFramesCaught(0),
		firstAcquisitionIndex(0),
		firstMeasurementIndex(0),
		currentFrameIndex(0),
		file(0),
		fileFormatType(ftype),
		fileWriteEnable(fwenable),
		callbackAction(cbaction),
		rawDataReadyCallBack(dataReadycb),
		pRawDataReady(pDataReadycb)
{
	if(ThreadObject::CreateThread()){
		pthread_mutex_lock(&Mutex);
		ErrorMask ^= (1<<index);
		pthread_mutex_unlock(&Mutex);
	}

	NumberofDataProcessors++;
	FILE_LOG (logDEBUG) << "Number of DataProcessors: " << NumberofDataProcessors << endl;
}


DataProcessor::~DataProcessor() {
	if (file) delete file;
	ThreadObject::DestroyThread();
	NumberofDataProcessors--;
}

/** static functions */

uint64_t DataProcessor::GetErrorMask() {
	return ErrorMask;
}

uint64_t DataProcessor::GetRunningMask() {
	return RunningMask;
}

/** non static functions */
/** getters */
string DataProcessor::GetType(){
	return TypeName;
}

bool DataProcessor::IsRunning() {
	return ((1 << index) & RunningMask);
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

uint64_t DataProcessor::GetProcessedAcquisitionIndex() {
	return currentFrameIndex - firstAcquisitionIndex;
}




/** setters */
void DataProcessor::StartRunning() {
	pthread_mutex_lock(&Mutex);
	RunningMask |= (1<<index);
	pthread_mutex_unlock(&Mutex);
}


void DataProcessor::StopRunning() {
	pthread_mutex_lock(&Mutex);
	RunningMask ^= (1<<index);
	pthread_mutex_unlock(&Mutex);
}


void DataProcessor::SetFifo(Fifo*& f) {
	fifo = f;
}

void DataProcessor::ResetParametersforNewAcquisition() {
	numTotalFramesCaught = 0;
	firstAcquisitionIndex = 0;
	currentFrameIndex = 0;
	acquisitionStartedFlag = false;
}

void DataProcessor::ResetParametersforNewMeasurement(){
	numFramesCaught = 0;
	firstMeasurementIndex = 0;
	measurementStartedFlag = false;
	if(RunningMask){
		pthread_mutex_lock(&Mutex);
		RunningMask = 0x0;
		pthread_mutex_unlock(&Mutex);
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

#ifdef VERBOSE
	cprintf(BLUE,"%d First Acquisition Index:%lld\tFirst Measurement Index:%lld\n",
			index, (long long int)firstAcquisitionIndex, (long long int)firstMeasurementIndex);
#endif
}


void DataProcessor::SetGeneralData(GeneralData* g) {
	generalData = g;
#ifdef VERY_VERBOSE
	generalData->Print();
#endif
	if (!file) {
		cprintf(RED, "Error Calling SetGeneralData with no file object. Should not be here\n");
		return;
	}
	if (file->GetFileType() == BINARY)
		file->SetMaxFramesPerFile(generalData->maxFramesPerFile);
	else if (file->GetFileType() == HDF5) {
		file->SetNumberofPixels(generalData->nPixelsX, generalData->nPixelsY);
	}
}


void DataProcessor::SetFileFormat(const fileFormat f) {
	if (file->GetFileType() != f) {
		//remember the pointer values before they are destroyed
		char* fname=0; char* fpath=0; uint64_t* findex=0; bool* frindexenable=0;
		bool* owenable=0; int* dindex=0; int* nunits=0; uint64_t* nf = 0; uint32_t* dr = 0;
		file->GetMemberPointerValues(fname, fpath, findex, frindexenable, owenable, dindex, nunits, nf, dr);
		//create file writer with same pointers
		SetupFileWriter(fname, fpath, findex, frindexenable, owenable, dindex, nunits, nf, dr);
	}
}



void DataProcessor::SetupFileWriter(char* fname, char* fpath, uint64_t* findex,
		bool* frindexenable, bool* owenable, int* dindex, int* nunits, uint64_t* nf, uint32_t* dr, GeneralData* g)
{
	if (g)
		generalData = g;

	if (file)
		delete file;

	switch(*fileFormatType){
#ifdef HDF5C
	case HDF5:
		file = new HDF5File(index, fname, fpath, findex,
				frindexenable, owenable, dindex, nunits, nf, dr, generalData->nPixelsX, generalData->nPixelsY);
		break;
#endif
	default:
		file = new BinaryFile(index, fname, fpath, findex,
				frindexenable, owenable, dindex, nunits, nf, dr, generalData->maxFramesPerFile);
		break;
	}
}


int DataProcessor::CreateNewFile(bool en, uint64_t nf, uint64_t at, uint64_t ap) {
	file->CloseAllFiles();
	if (file->CreateCommonFiles(en,	generalData->imageSize, generalData->nPixelsX, generalData->nPixelsY,
			at, ap) == FAIL)
		return FAIL;
	if (file->CreateFile(currentFrameIndex) == FAIL)
		return FAIL;
	return OK;
}


void DataProcessor::CloseFiles() {
	if (file)
		file->CloseAllFiles();
}


void DataProcessor::ThreadExecution() {
	char* buffer=0;
	fifo->PopAddress(buffer);
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



void DataProcessor::StopProcessing(char* buf) {
	fifo->FreeAddress(buf);
	file->CloseCurrentFile();
	StopRunning();
	cprintf(BLUE,"%d: Processing Completed\n", index);
}


void DataProcessor::ProcessAnImage(char* buf) {
	numFramesCaught++;
	numTotalFramesCaught++;


	uint64_t fnum = (*((uint64_t*)buf));
#ifdef VERBOSE
	if (!index) cprintf(BLUE,"DataProcessing %d: fnum:%lld\n", index, (long long int)fnum);
#endif

	if (!measurementStartedFlag) {
#ifdef VERBOSE
		if (!index) cprintf(BLUE,"DataProcessing %d: fnum:%lld\n", index, (long long int)fnum);
#endif
		RecordFirstIndices(fnum);
	}

	if (fileWriteEnable && *callbackAction == DO_EVERYTHING)
		file->WriteToFile(buf + FIFO_HEADER_NUMBYTES, generalData->fifoBufferSize + FILE_FRAME_HEADER_SIZE, fnum-firstMeasurementIndex);
}

