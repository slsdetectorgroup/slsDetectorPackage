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
#include <errno.h>
#include <cstring>
using namespace std;

const string Listener::TypeName = "Listener";

int Listener::NumberofListeners(0);

uint64_t Listener::ErrorMask(0x0);

uint64_t Listener::RunningMask(0x0);

pthread_mutex_t Listener::Mutex = PTHREAD_MUTEX_INITIALIZER;

const GeneralData* Listener::generalData(0);


Listener::Listener(Fifo*& f, runStatus* s, uint32_t* portno, char* e, int* act, uint64_t* nf) :
		ThreadObject(NumberofListeners),
		fifo(f),
		acquisitionStartedFlag(false),
		measurementStartedFlag(false),
		status(s),
		udpSocket(0),
		udpPortNumber(portno),
		eth(e),
		activated(act),
		numImages(nf),
		numTotalPacketsCaught(0),
		numPacketsCaught(0),
		firstAcquisitionIndex(0),
		firstMeasurementIndex(0),
		currentFrameIndex(0),
		lastCaughtFrameIndex(0),
		carryOverFlag(0),
		carryOverPacket(0),
		listeningPacket(0)
{
	if(ThreadObject::CreateThread()){
		pthread_mutex_lock(&Mutex);
		ErrorMask ^= (1<<index);
		pthread_mutex_unlock(&Mutex);
	}
	NumberofListeners++;
	FILE_LOG (logDEBUG) << "Number of Listeners: " << NumberofListeners;
}


Listener::~Listener() {
	if (carryOverPacket) delete carryOverPacket;
	if (listeningPacket) delete listeningPacket;
	ThreadObject::DestroyThread();
	NumberofListeners--;
}

/** static functions */

uint64_t Listener::GetErrorMask() {
	return ErrorMask;
}

uint64_t Listener::GetRunningMask() {
	return RunningMask;
}

void Listener::ResetRunningMask() {
	RunningMask = 0x0;
}


/** non static functions */
/** getters */
string Listener::GetType(){
	return TypeName;
}

bool Listener::IsRunning() {
	return ((1 << index) & RunningMask);
}

bool Listener::GetAcquisitionStartedFlag(){
	return acquisitionStartedFlag;
}

bool Listener::GetMeasurementStartedFlag(){
	return measurementStartedFlag;
}

uint64_t Listener::GetTotalPacketsCaught() {
	return numTotalPacketsCaught;
}

uint64_t Listener::GetLastFrameIndexCaught() {
	return lastCaughtFrameIndex;
}

/** setters */
void Listener::StartRunning() {
	pthread_mutex_lock(&Mutex);
	RunningMask |= (1<<index);
	pthread_mutex_unlock(&Mutex);
}


void Listener::StopRunning() {
	pthread_mutex_lock(&Mutex);
	RunningMask ^= (1<<index);
	pthread_mutex_unlock(&Mutex);
}


void Listener::SetFifo(Fifo*& f) {
	fifo = f;
}


void Listener::ResetParametersforNewAcquisition() {
	acquisitionStartedFlag = false;
	numTotalPacketsCaught = 0;
	firstAcquisitionIndex = 0;
	currentFrameIndex = 0;
	lastCaughtFrameIndex = 0;
}


void Listener::ResetParametersforNewMeasurement(){
	measurementStartedFlag = false;
	numPacketsCaught = 0;
	firstMeasurementIndex = 0;
	carryOverFlag = false;
	if (carryOverPacket)
		delete carryOverPacket;
	carryOverPacket = new char[generalData->packetSize];
	if (listeningPacket)
		delete listeningPacket;
	listeningPacket = new char[generalData->packetSize];
}



void Listener::RecordFirstIndices(uint64_t fnum) {
	//listen to this fnum, later +1
	currentFrameIndex = fnum;

	measurementStartedFlag = true;
	firstMeasurementIndex = fnum;

	//start of entire acquisition
	if (!acquisitionStartedFlag) {
		acquisitionStartedFlag = true;
		firstAcquisitionIndex = fnum;
	}
	if (!index) cprintf(BLUE,"%d First Acquisition Index:%lld\n"
							  "%d First Measurement Index:%lld\n",
			index, (long long int)firstAcquisitionIndex,
			index, (long long int)firstMeasurementIndex);
}


void Listener::SetGeneralData(GeneralData*& g) {
	generalData = g;
#ifdef VERY_VERBOSE
	generalData->Print();
#endif
}


int Listener::SetThreadPriority(int priority) {
	struct sched_param param;
	param.sched_priority = priority;
	if (pthread_setschedparam(thread, SCHED_RR, &param) == EPERM)
		return FAIL;
	return OK;
}

int Listener::CreateUDPSockets() {
	ShutDownUDPSocket();

	if (!(*activated))
		return OK;

	//if eth is mistaken with ip address
	if (strchr(eth,'.') != NULL){
		strcpy(eth,"");
	}
	if(!strlen(eth)){
		FILE_LOG(logWARNING) << "eth is empty. Listening to all";
	}

	udpSocket = new genericSocket(*udpPortNumber, genericSocket::UDP,
			generalData->packetSize, (strlen(eth)?eth:NULL), generalData->headerPacketSize);
	int iret = udpSocket->getErrorStatus();
	if(!iret){
		cout << index << ": UDP port opened at port " << *udpPortNumber << endl;
	}else{
		FILE_LOG(logERROR) << "Could not create UDP socket on port " << *udpPortNumber << " error: " << iret;
		return FAIL;
	}
	return OK;
}



void Listener::ShutDownUDPSocket() {
	if(udpSocket){
		udpSocket->ShutDownSocket();
		FILE_LOG(logINFO) << "Shut down of UDP port " << *udpPortNumber;
		delete udpSocket;
		udpSocket = 0;
	}
}



void Listener::ThreadExecution() {
	char* buffer;
	int rc = 0;

	fifo->GetNewAddress(buffer);
#ifdef FIFODEBUG
	if (!index) cprintf(GREEN,"Listener %d, pop 0x%p buffer:%s\n", index,(void*)(buffer),buffer);
#endif

	//udpsocket doesnt exist
	if (*activated && !udpSocket) {
		FILE_LOG(logERROR) << "Listening_Thread " << index << ": UDP Socket not created or shut down earlier";
		(*((uint32_t*)buffer)) = 0;
		StopListening(buffer);
		return;
	}

	//get data
	if (*status != TRANSMITTING) {
		if (*activated)
			rc = ListenToAnImage(buffer + generalData->fifoBufferHeaderSize);
		else
			rc = CreateAnImage(buffer + generalData->fifoBufferHeaderSize);
	}

	//done acquiring
	if (*status == TRANSMITTING || ((!(*activated)) && (rc == 0))) {
		StopListening(buffer);
		return;
	}

	//error check
	if (rc <= 0) {
		cprintf(BG_RED,"Error:(Weird), UDP Sockets not shut down, but received nothing\n");
		fifo->FreeAddress(buffer);
		return;
	}

	(*((uint32_t*)buffer)) = rc;
	//for those returning earlier
	(*((uint64_t*)(buffer + FIFO_HEADER_NUMBYTES ))) = currentFrameIndex;
	currentFrameIndex++;

	//push into fifo
	fifo->PushAddress(buffer);
}



void Listener::StopListening(char* buf) {
	(*((uint32_t*)buf)) = DUMMY_PACKET_VALUE;
	fifo->PushAddress(buf);
	StopRunning();
#ifdef VERBOSE
	cprintf(GREEN,"%d: Listening Packets (%d) : %d\n", index, *udpPortNumber, numPacketsCaught);
	printf("%d: Listening Completed\n", index);
#endif
}



uint32_t Listener::ListenToAnImage(char* buf) {
	uint32_t rc = 0;
	uint64_t fnum = 0, bid = 0;
	uint32_t pnum = 0, snum = 0;
	int dsize = generalData->dataSize;
	bool isHeaderEmpty = true;


	//reset to -1
	memset(buf,0xFF,dsize);


	//look for carry over
	if (carryOverFlag) {
		//check if its the current image packet
		generalData->GetHeaderInfo(index, carryOverPacket, fnum, pnum, snum, bid);
		if (fnum != currentFrameIndex) {
			return generalData->imageSize;
		}
		carryOverFlag = false;
		memcpy(buf + (pnum * dsize), carryOverPacket + generalData->headerSizeinPacket, dsize);
		if(isHeaderEmpty) {
			(*((uint64_t*)(buf - FILE_FRAME_HEADER_SIZE))) = fnum;
			(*((uint64_t*)(buf - FILE_FRAME_HEADER_SIZE + FILE_FRAME_HDR_FNUM_SIZE))) = snum;
			(*((uint64_t*)(buf - FILE_FRAME_HEADER_SIZE + FILE_FRAME_HDR_FNUM_SIZE + FILE_FRAME_HDR_SNUM_SIZE))) = bid;
			isHeaderEmpty = false;
		}
	}


	while (pnum < (generalData->packetsPerFrame-1)) {

		//listen to new packet
		rc += udpSocket->ReceiveDataOnly(listeningPacket);
		if (rc <= 0) return 0;


		//update parameters
		numPacketsCaught++;		//record immediately to get more time before socket shutdown
		numTotalPacketsCaught++;
		generalData->GetHeaderInfo(index, listeningPacket, fnum, pnum, snum, bid);
		lastCaughtFrameIndex = fnum;
#ifdef VERBOSE
		if (!index && !pnum) cprintf(GREEN,"Listening %d: fnum:%lld, pnum:%d\n", index, (long long int)fnum, pnum);
#endif
		if (!measurementStartedFlag)
			RecordFirstIndices(fnum);

		//future packet
		if (fnum != currentFrameIndex) {
			carryOverFlag = true;
			memcpy(carryOverPacket,listeningPacket, generalData->packetSize);
			return generalData->imageSize;
		}

		//copy packet
		memcpy(buf + (pnum * dsize), listeningPacket + generalData->headerSizeinPacket, dsize);
		if(isHeaderEmpty) {
			(*((uint64_t*)(buf - FILE_FRAME_HEADER_SIZE))) = fnum;
			(*((uint64_t*)(buf - FILE_FRAME_HEADER_SIZE + FILE_FRAME_HDR_FNUM_SIZE))) = snum;
			(*((uint64_t*)(buf - FILE_FRAME_HEADER_SIZE + FILE_FRAME_HDR_FNUM_SIZE + FILE_FRAME_HDR_SNUM_SIZE))) = bid;
			isHeaderEmpty = false;
		}
	}

	return generalData->imageSize;
}


uint32_t Listener::CreateAnImage(char* buf) {

	if (!measurementStartedFlag)
		RecordFirstIndices(0);

	if (currentFrameIndex == *numImages)
		return 0;

	//update parameters
	numPacketsCaught++;		//record immediately to get more time before socket shutdown
	numTotalPacketsCaught++;

	//reset data to -1
	memset(buf, 0xFF, generalData->dataSize);

	return generalData->imageSize;
}
