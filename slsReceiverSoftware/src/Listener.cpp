/************************************************
 * @file Listener.cpp
 * @short creates the listener thread that
 * listens to udp sockets, writes data to memory
 * & puts pointers to their memory addresses into fifos
 ***********************************************/


#include "Listener.h"
#include "Fifo.h"
#include "genericSocket.h"

#include <iostream>
#include <cstring>
using namespace std;

const string Listener::TypeName = "Listener";

int Listener::NumberofListeners(0);

uint64_t Listener::ErrorMask(0x0);

uint64_t Listener::RunningMask(0x0);

pthread_mutex_t Listener::Mutex = PTHREAD_MUTEX_INITIALIZER;

bool Listener::acquisitionStartedFlag(false);

bool Listener::measurementStartedFlag(false);

Listener::Listener(Fifo*& f) :
		ThreadObject(NumberofListeners),
		fifo(f),
		numTotalPacketsCaught(0),
		numPacketsCaught(0),
		firstAcquisitionIndex(0),
		firstMeasurementIndex(0)
{
	FILE_LOG (logDEBUG) << __AT__ << " called";

	if(ThreadObject::CreateThread()){
		pthread_mutex_lock(&Mutex);
		ErrorMask ^= (1<<index);
		pthread_mutex_unlock(&Mutex);
	}
	NumberofListeners++;
	FILE_LOG (logDEBUG) << "Number of Listeners: " << NumberofListeners << endl;
}


Listener::~Listener() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	ThreadObject::DestroyThread();
	NumberofListeners--;
}

/** static functions */


uint64_t Listener::GetErrorMask() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	return ErrorMask;
}


bool Listener::GetAcquisitionStartedFlag(){
	FILE_LOG (logDEBUG) << __AT__ << " called";
	return acquisitionStartedFlag;
}


bool Listener::GetMeasurementStartedFlag(){
	FILE_LOG (logDEBUG) << __AT__ << " called";
	return measurementStartedFlag;
}


/** non static functions */

string Listener::GetType(){
	return TypeName;
}


uint64_t Listener::GetTotalPacketsCaught() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	return numTotalPacketsCaught;
}

uint64_t Listener::GetNumReceivedinUDPBuffer() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	if(!udpSocket)
		return 0;
	return udpSocket->getCurrentTotalReceived();
}

bool Listener::IsRunning() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	return ((1 << index) & RunningMask);
}


void Listener::StartRunning() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	pthread_mutex_lock(&Mutex);
	RunningMask |= (1<<index);
	pthread_mutex_unlock(&Mutex);
}


void Listener::StopRunning() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	pthread_mutex_lock(&Mutex);
	RunningMask ^= (1<<index);
	pthread_mutex_unlock(&Mutex);
}


void Listener::SetFifo(Fifo*& f) {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	fifo = f;
}


void Listener::ResetParametersforNewAcquisition() {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	numTotalPacketsCaught = 0;
	firstAcquisitionIndex = 0;
	if(acquisitionStartedFlag){
		pthread_mutex_lock(&Mutex);
		acquisitionStartedFlag = false;
		pthread_mutex_unlock(&Mutex);
	}
}


void Listener::ResetParametersforNewMeasurement(){
	FILE_LOG (logDEBUG) << __AT__ << " called";
	numPacketsCaught = 0;
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


void Listener::ThreadExecution() {
	FILE_LOG (logDEBUG) << __AT__ << " called";

	char* buffer;
	fifo->GetNewAddress(buffer);
#ifdef FIFODEBUG
	cprintf(GREEN,"Listener %d, pop 0x%p buffer:%s\n", index,(void*)(buffer),buffer);
#endif

	int rc;

	while ((rc>0 && rc < generalData->packetSize)) {
		rc = udpSocket->ReceiveDataOnly(buffer + generalData->fifoBufferHeaderSize,fifoBufferSize);
		cprintf(BLUE,"Listening %d: rc: %d\n",index,rc);
		uint64_t fnum; uint32_t pnum; uint32_t snum; uint64_t bcid;
		GetHeaderInfo(index,buffer,16,fnum,pnum,snum,bcid);
		cprintf(BLUE,"Listening %d: fnum:%lld, pnum:%d\n",(long long int)fnum, pnum);
		*((uint32_t*)(buffer[ithread])) = (rc/generalData->packetSize);
	}

	if(rc <=0 ){
		cprintf(BLUE,"Listening %d: Gonna send dummy value*****\n");
		(*((uint32_t*)buffer)) = DUMMY_PACKET_VALUE;
		StopRunning();
	}

	fifo->PushAddress(buffer);
}


int Listener::CreateUDPSockets(uint32_t portnumber, uint32_t packetSize, const char* eth, uint32_t headerPacketSize) {
	FILE_LOG (logDEBUG) << __AT__ << " called";

	udpSocket = new genericSocket(portnumber, genericSocket::UDP, packetSize, eth, headerPacketSize);
	int iret = udpSocket->getErrorStatus();
	if(!iret){
		cout << "UDP port opened at port " << portnumber << endl;
	}else{
		FILE_LOG(logERROR) << "Could not create UDP socket on port " << portnumber << " error: " << iret;
		return FAIL;
	}
	return OK;
}


void Listener::ShutDownUDPSocket() {
	FILE_LOG (logDEBUG) << __AT__ << " called";

	if(udpSocket){
		udpSocket->ShutDownSocket();
		FILE_LOG(logINFO) << "Shut down UDP Socket " << index;
		delete udpSocket;
		udpSocket = 0;
	}
}

