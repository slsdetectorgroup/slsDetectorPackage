/************************************************
 * @file DataProcessor.h
 * @short creates data processor thread that
 * pulls pointers to memory addresses from fifos
 * and processes data stored in them & writes them to file
 ***********************************************/


#include "DataProcessor.h"
#include "Fifo.h"

#include <iostream>
#include <cstring>
using namespace std;

const string DataProcessor::TypeName = "DataProcessor";

int DataProcessor::NumberofDataProcessors(0);

uint64_t DataProcessor::ErrorMask(0x0);

uint64_t DataProcessor::RunningMask(0x0);

pthread_mutex_t DataProcessor::Mutex = PTHREAD_MUTEX_INITIALIZER;

bool DataProcessor::acquisitionStartedFlag(false);

bool DataProcessor::measurementStartedFlag(false);


DataProcessor::DataProcessor(Fifo*& f) :
		ThreadObject(NumberofDataProcessors),
		fifo(f),
		numTotalFramesCaught(0),
		numFramesCaught(0),
		firstAcquisitionIndex(0),
		firstMeasurementIndex(0),
		currentFrameIndex(0)
{
	FILE_LOG (logDEBUG) << __AT__ << " called";

	if(ThreadObject::CreateThread()){
		pthread_mutex_lock(&Mutex);
		ErrorMask ^= (1<<index);
		pthread_mutex_unlock(&Mutex);
	}
	NumberofDataProcessors++;
	FILE_LOG (logDEBUG) << "Number of DataProcessors: " << NumberofDataProcessors << endl;
}


DataProcessor::~DataProcessor() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	ThreadObject::DestroyThread();
	NumberofDataProcessors--;
}

/** static functions */


uint64_t DataProcessor::GetErrorMask() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	return ErrorMask;
}


bool DataProcessor::GetAcquisitionStartedFlag(){
	FILE_LOG (logDEBUG) << __AT__ << " called";
	return acquisitionStartedFlag;
}


bool DataProcessor::GetMeasurementStartedFlag(){
	FILE_LOG (logDEBUG) << __AT__ << " called";
	return measurementStartedFlag;
}


/** non static functions */

string DataProcessor::GetType(){
	return TypeName;
}

uint64_t DataProcessor::GetNumTotalFramesCaught() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	return numTotalFramesCaught;
}


uint64_t DataProcessor::GetNumFramesCaught() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	return numFramesCaught;
}


uint64_t DataProcessor::GetProcessedAcquisitionIndex() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	return currentFrameIndex - firstAcquisitionIndex;
}


bool DataProcessor::IsRunning() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	return ((1 << index) & RunningMask);
}


void DataProcessor::StartRunning() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	pthread_mutex_lock(&Mutex);
	RunningMask |= (1<<index);
	pthread_mutex_unlock(&Mutex);
}


void DataProcessor::StopRunning() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	pthread_mutex_lock(&Mutex);
	RunningMask ^= (1<<index);
	pthread_mutex_unlock(&Mutex);
}


void DataProcessor::SetFifo(Fifo*& f) {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	fifo = f;
}


void DataProcessor::ResetParametersforNewAcquisition() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	numTotalFramesCaught = 0;
	firstAcquisitionIndex = 0;
	currentFrameIndex = 0;
	if(acquisitionStartedFlag){
		pthread_mutex_lock(&Mutex);
		acquisitionStartedFlag = false;
		pthread_mutex_unlock(&Mutex);
	}
}

void DataProcessor::ResetParametersforNewMeasurement(){
	FILE_LOG (logDEBUG) << __AT__ << " called";
	numFramesCaught = 0;
	firstMeasurementIndex = 0;
	if(measurementStartedFlag){
		pthread_mutex_lock(&Mutex);
		measurementStartedFlag = false;
		pthread_mutex_unlock(&Mutex);
	}
	if(RunningMask){
		pthread_mutex_lock(&Mutex);
		RunningMask = 0x0;
		pthread_mutex_unlock(&Mutex);
	}
}


void DataProcessor::ThreadExecution() {
	FILE_LOG (logDEBUG) << __AT__ << " called";

	char* buffer=0;
	fifo->PopAddress(buffer);
#ifdef FIFODEBUG
	cprintf(BLUE,"DataProcessor %d, pop 0x%p buffer:%s\n", index,(void*)(buffer),buffer);
#endif
	uint32_t numPackets = (uint32_t)(*((uint32_t*)buffer));

	if(numPackets == DUMMY_PACKET_VALUE){
		cprintf(GREEN,"DataProcessing %d: Got dummy value*****\n");
		StopRunning();
		fifo->FreeAddress(buffer);
		return;
	}

	uint64_t fnum; uint32_t pnum; uint32_t snum; uint64_t bcid;
	GetHeaderInfo(index,buffer+generalData->fifoBufferHeaderSize,16,fnum,pnum,snum,bcid);
	cprintf(GREEN,"DataProcessing %d: fnum:%lld, pnum:%d\n",(long long int)fnum, pnum);

	fifo->FreeAddress(buffer);
}


int DataProcessor::CreateNewFile() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	//create file fileWriter.push_back(new BinaryFileWriter(fileName))
	return OK;
}



void DataProcessor::CloseFile() {
	FILE_LOG (logDEBUG) << __AT__ << " called";

}
