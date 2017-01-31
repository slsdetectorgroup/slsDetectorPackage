/************************************************
 * @file DataStreamer.h
 * @short streams data from receiver via ZMQ
 ***********************************************/


#include "DataStreamer.h"

#include <iostream>
using namespace std;

const string DataStreamer::TypeName = "DataStreamer";

int DataStreamer::NumberofDataStreamers(0);

uint64_t DataStreamer::ErrorMask(0x0);

uint64_t DataStreamer::RunningMask(0x0);

pthread_mutex_t DataStreamer::Mutex = PTHREAD_MUTEX_INITIALIZER;


DataStreamer::DataStreamer() :
		ThreadObject(NumberofDataStreamers)
{
	FILE_LOG (logDEBUG) << __AT__ << " called";

	if(ThreadObject::CreateThread()){
		pthread_mutex_lock(&Mutex);
		ErrorMask ^= (1<<index);
		pthread_mutex_unlock(&Mutex);
	}

	NumberofDataStreamers++;
	FILE_LOG (logDEBUG) << "Number of DataStreamers: " << NumberofDataStreamers << endl;
}


DataStreamer::~DataStreamer() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	ThreadObject::DestroyThread();
	NumberofDataStreamers--;
}

/** static functions */


uint64_t DataStreamer::GetErrorMask() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	return ErrorMask;
}


void DataStreamer::ResetRunningMask() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	pthread_mutex_lock(&Mutex);
	RunningMask = 0x0;
	pthread_mutex_unlock(&Mutex);
}


/** non static functions */


string DataStreamer::GetType(){
	return TypeName;
}


bool DataStreamer::IsRunning() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	return ((1 << index) & RunningMask);
}


void DataStreamer::StartRunning() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	pthread_mutex_lock(&Mutex);
	RunningMask |= (1<<index);
	pthread_mutex_unlock(&Mutex);
}


void DataStreamer::StopRunning() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	pthread_mutex_lock(&Mutex);
	RunningMask ^= (1<<index);
	pthread_mutex_unlock(&Mutex);
}

void DataStreamer::ThreadExecution() {
	FILE_LOG (logDEBUG) << __AT__ << " called";

}


