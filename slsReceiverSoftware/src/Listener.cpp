/************************************************
 * @file Listener.cpp
 * @short creates the listener thread that
 * listens to udp sockets, writes data to memory
 * & puts pointers to their memory addresses into fifos
 ***********************************************/


#include "Listener.h"
#include "GeneralData.h"
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

const GeneralData* Listener::generalData(0);

bool Listener::acquisitionStartedFlag(false);

bool Listener::measurementStartedFlag(false);

Listener::Listener(Fifo*& f, runStatus* s, uint32_t* portno) :
		ThreadObject(NumberofListeners),
		fifo(f),
		status(s),
		udpSocket(0),
		udpPortNumber(portno),
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

void Listener::SetGeneralData(GeneralData*& g) {
	FILE_LOG (logDEBUG) << __AT__ << " called";
	generalData = g;
//#ifdef VERY_VERBOSE
	generalData->Print();
//#endif
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
	if (!index) cprintf(GREEN,"Listener %d, pop 0x%p buffer:%s\n", index,(void*)(buffer),buffer);
#endif

	int rc = 0;

	//udpsocket doesnt exist
	if (!udpSocket) {
		FILE_LOG(logERROR) << "Listening_Thread " << index << ": UDP Socket not created or shut down earlier";
		(*((uint32_t*)buffer)) = 0;
		StopListening(buffer);
		return;
	}

	//get data
	if (*status != TRANSMITTING) {
		rc = udpSocket->ReceiveDataOnly(buffer + generalData->fifoBufferHeaderSize,generalData->fifoBufferSize);
		if (!index) cprintf(GREEN,"Listening %d: rc: %d\n",index,rc);
		(*((uint32_t*)buffer)) = ((rc <= 0) ? 0 : rc);
	}

	//done acquiring
	if (*status == TRANSMITTING) {
		StopListening(buffer);
		return;
	}

	uint64_t fnum; uint32_t pnum; uint32_t snum; uint64_t bcid=0;
	generalData->GetHeaderInfo(index,buffer + generalData->fifoBufferHeaderSize,16,fnum,pnum,snum,bcid);
	if (!index) cprintf(GREEN,"Listening %d: fnum:%lld, pnum:%d\n", index, (long long int)fnum, pnum);

	//push into fifo
	fifo->PushAddress(buffer);
}



void Listener::StopListening(char* buf) {
	FILE_LOG (logDEBUG) << __AT__ << " called";

	cprintf(BLUE,"%d: End of Listening\n", index);

	uint32_t numPackets = (uint32_t)(*((uint32_t*)buf));

	//incomplete packets
	if (numPackets > 0) {
		fifo->PushAddress(buf);
		fifo->GetNewAddress(buf);
#ifdef FIFODEBUG
		if (!index) cprintf(GREEN,"Listener %d, Got incomplete, for dummy: pop 0x%p buffer:%s\n", index,(void*)(buf),buf);
#endif
	}

	//dummy
	(*((uint32_t*)buf)) = DUMMY_PACKET_VALUE;
	fifo->PushAddress(buf);

	StopRunning();

//#ifdef DEBUG4
	if (!index) FILE_LOG(logINFO) << "Listening Thread (" << *udpPortNumber << ")"
			" Packets caught: " << numPacketsCaught;
//#endif

}



int Listener::CreateUDPSockets(const char* eth) {
	FILE_LOG (logDEBUG) << __AT__ << " called";

	udpSocket = new genericSocket(*udpPortNumber, genericSocket::UDP,
			generalData->packetSize, eth, generalData->headerPacketSize);
	int iret = udpSocket->getErrorStatus();
	if(!iret){
		cout << "UDP port opened at port " << *udpPortNumber << endl;
	}else{
		FILE_LOG(logERROR) << "Could not create UDP socket on port " << *udpPortNumber << " error: " << iret;
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

