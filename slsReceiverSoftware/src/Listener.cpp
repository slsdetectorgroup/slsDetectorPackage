/************************************************
 * @file Listener.cpp
 * @short creates the listener thread that
 * listens to udp sockets, writes data to memory
 * & puts pointers to their memory addresses into fifos
 ***********************************************/


#include "Listener.h"
#include "Fifo.h"

#include <iostream>
#include <cstring>
using namespace std;

const string Listener::TypeName = "Listener";

int Listener::NumberofListeners(0);

uint64_t Listener::ErrorMask(0x0);

uint64_t Listener::RunningMask(0x0);

pthread_mutex_t Listener::Mutex = PTHREAD_MUTEX_INITIALIZER;

Listener::Listener(Fifo*& f) : ThreadObject(NumberofListeners), fifo(f) {
	FILE_LOG(logDEBUG) << __AT__ << " called";

	if(ThreadObject::CreateThread()){
		pthread_mutex_lock(&Mutex);
		ErrorMask ^= (1<<index);
		pthread_mutex_unlock(&Mutex);
	}
	count = 0;
	NumberofListeners++;
	FILE_LOG(logDEBUG) << "Number of Listeners: " << NumberofListeners << endl;
}


Listener::~Listener() {
	FILE_LOG(logDEBUG) << __AT__ << " called";
	ThreadObject::DestroyThread();
	NumberofListeners--;
}

/** static functions */


uint64_t Listener::GetErrorMask() {
	FILE_LOG(logDEBUG) << __AT__ << " called";
	return ErrorMask;
}


void Listener::ResetRunningMask() {
	FILE_LOG(logDEBUG) << __AT__ << " called";
	pthread_mutex_lock(&Mutex);
	RunningMask = 0x0;
	pthread_mutex_unlock(&Mutex);
}


/** non static functions */

string Listener::GetType(){
	return TypeName;
}


bool Listener::IsRunning() {
	FILE_LOG(logDEBUG) << __AT__ << " called";
	return ((1 << index) & RunningMask);
}


void Listener::StartRunning() {
	FILE_LOG(logDEBUG) << __AT__ << " called";
	pthread_mutex_lock(&Mutex);
	RunningMask |= (1<<index);
	pthread_mutex_unlock(&Mutex);
}


void Listener::StopRunning() {
	FILE_LOG(logDEBUG) << __AT__ << " called";
	pthread_mutex_lock(&Mutex);
	RunningMask ^= (1<<index);
	pthread_mutex_unlock(&Mutex);
}


void Listener::SetFifo(Fifo*& f) {
	FILE_LOG(logDEBUG) << __AT__ << " called";
	fifo = f;
}


void Listener::ThreadExecution() {
	FILE_LOG(logDEBUG) << __AT__ << " called";

	char* buffer;
	fifo->GetNewAddress(buffer);
#ifdef FIFODEBUG
	cprintf(GREEN,"Listener %d, pop 0x%p buffer:%s\n", index,(void*)(buffer),buffer);
#endif

	strcpy(buffer,"changed");

	if(count == 3){
		strcpy(buffer,"done\0");
		StopRunning();
	}

	fifo->PushAddress(buffer);
	count++;
}


