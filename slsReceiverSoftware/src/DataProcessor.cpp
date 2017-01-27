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


DataProcessor::DataProcessor(Fifo*& f) : ThreadObject(NumberofDataProcessors), fifo(f) {
	FILE_LOG(logDEBUG) << __AT__ << " called";

	if(ThreadObject::CreateThread()){
		pthread_mutex_lock(&Mutex);
		ErrorMask ^= (1<<index);
		pthread_mutex_unlock(&Mutex);
	}
	NumberofDataProcessors++;
	FILE_LOG(logDEBUG) << "Number of DataProcessors: " << NumberofDataProcessors << endl;
}


DataProcessor::~DataProcessor() {
	FILE_LOG(logDEBUG) << __AT__ << " called";
	ThreadObject::DestroyThread();
	NumberofDataProcessors--;
}

/** static functions */


uint64_t DataProcessor::GetErrorMask() {
	FILE_LOG(logDEBUG) << __AT__ << " called";
	return ErrorMask;
}


void DataProcessor::ResetRunningMask() {
	FILE_LOG(logDEBUG) << __AT__ << " called";
	pthread_mutex_lock(&Mutex);
	RunningMask = 0x0;
	pthread_mutex_unlock(&Mutex);
}


/** non static functions */


string DataProcessor::GetType(){
	return TypeName;
}


bool DataProcessor::IsRunning() {
	FILE_LOG(logDEBUG) << __AT__ << " called";
	return ((1 << index) & RunningMask);
}


void DataProcessor::StartRunning() {
	FILE_LOG(logDEBUG) << __AT__ << " called";
	pthread_mutex_lock(&Mutex);
	RunningMask |= (1<<index);
	pthread_mutex_unlock(&Mutex);
}


void DataProcessor::StopRunning() {
	FILE_LOG(logDEBUG) << __AT__ << " called";
	pthread_mutex_lock(&Mutex);
	RunningMask ^= (1<<index);
	pthread_mutex_unlock(&Mutex);
}


void DataProcessor::SetFifo(Fifo*& f) {
	FILE_LOG(logDEBUG) << __AT__ << " called";
	fifo = f;
}


void DataProcessor::ThreadExecution() {
	FILE_LOG(logDEBUG) << __AT__ << " called";

	char* buffer=0;
	fifo->PopAddress(buffer);
#ifdef FIFODEBUG
	cprintf(BLUE,"DataProcessor %d, pop 0x%p buffer:%s\n", index,(void*)(buffer),buffer);
#endif

	if(!strcmp(buffer,"done")){
		StopRunning();
	}
	fifo->FreeAddress(buffer);
}
