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


Listener::Listener(detectorType dtype, Fifo*& f, runStatus* s, uint32_t* portno, char* e, int* act, uint64_t* nf, uint32_t* dr) :
		ThreadObject(NumberofListeners),
		generalData(0),
		fifo(f),
		myDetectorType(dtype),
		status(s),
		udpSocket(0),
		udpPortNumber(portno),
		eth(e),
		activated(act),
		numImages(nf),
		dynamicRange(dr),
		acquisitionStartedFlag(false),
		measurementStartedFlag(false),
		firstAcquisitionIndex(0),
		firstMeasurementIndex(0),
		numPacketsCaught(0),
		lastCaughtFrameIndex(0),
		currentFrameIndex(0),
		carryOverFlag(0),
		carryOverPacket(0),
		listeningPacket(0),
		udpSocketAlive(0)
{
	if(ThreadObject::CreateThread()){
		pthread_mutex_lock(&Mutex);
		ErrorMask ^= (1<<index);
		pthread_mutex_unlock(&Mutex);
	}
	NumberofListeners++;
	FILE_LOG (logDEBUG) << "Number of Listeners: " << NumberofListeners;

	switch(myDetectorType){
	case JUNGFRAU:
	case EIGER:
		standardheader = true;
		break;
	default:
		standardheader = false;
		break;
	}
}


Listener::~Listener() {
	if (udpSocket) delete udpSocket;
	if (carryOverPacket) delete [] carryOverPacket;
	if (listeningPacket) delete [] listeningPacket;
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

uint64_t Listener::GetPacketsCaught() {
	return numPacketsCaught;
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
	firstAcquisitionIndex = 0;
	currentFrameIndex = 0;
	lastCaughtFrameIndex = 0;
}


void Listener::ResetParametersforNewMeasurement() {
	measurementStartedFlag = false;
	numPacketsCaught = 0;
	firstMeasurementIndex = 0;
	carryOverFlag = false;
	if (carryOverPacket)
		delete [] carryOverPacket;
	carryOverPacket = new char[generalData->packetSize];
	memset(carryOverPacket,0,generalData->packetSize);
	if (listeningPacket)
		delete [] listeningPacket;
	listeningPacket = new char[generalData->packetSize];
	memset(listeningPacket,0,generalData->packetSize);

	numPacketsStatistic = 0;
	numFramesStatistic = 0;
	//reset fifo statistic
	fifo->GetMaxLevelForFifoBound();
	fifo->GetMinLevelForFifoFree();
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
	if (!index) bprintf(BLUE,"%d First Acquisition Index:%lu\n"
							  "%d First Measurement Index:%lu\n",
			index, firstAcquisitionIndex,
			index, firstMeasurementIndex);
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
	if (pthread_setschedparam(thread, SCHED_FIFO, &param) == EPERM)
		return FAIL;
	FILE_LOG(logINFO) << "Listener Thread Priority set to " << priority;
	return OK;
}

int Listener::CreateUDPSockets() {

	if (!(*activated))
		return OK;

	//if eth is mistaken with ip address
	if (strchr(eth,'.') != NULL){
		strncpy(eth,"", MAX_STR_LENGTH);
	}
	if(!strlen(eth)){
		FILE_LOG(logWARNING) << "eth is empty. Listening to all";
	}

	ShutDownUDPSocket();
	udpSocket = new genericSocket(*udpPortNumber, genericSocket::UDP,
			generalData->packetSize, (strlen(eth)?eth:NULL), generalData->headerPacketSize);
	int iret = udpSocket->getErrorStatus();
	if(!iret){
		FILE_LOG(logINFO) << index << ": UDP port opened at port " << *udpPortNumber;
	}else{
		FILE_LOG(logERROR) << "Could not create UDP socket on port " << *udpPortNumber << " error: " << iret;
		return FAIL;
	}
	udpSocketAlive = true;
	return OK;
}



void Listener::ShutDownUDPSocket() {
	if(udpSocket){
		udpSocketAlive = false;
		udpSocket->ShutDownSocket();
		FILE_LOG(logINFO) << "Shut down of UDP port " << *udpPortNumber;
		fflush(stdout);
		//delete socket at stoplistening
	}
}



void Listener::ThreadExecution() {
	char* buffer;
	int rc = 0;

	fifo->GetNewAddress(buffer);
#ifdef FIFODEBUG
	bprintf(GREEN,"Listener %d, pop 0x%p buffer:%s\n", index,(void*)(buffer),buffer);
#endif

	//udpsocket doesnt exist
	if (*activated && !udpSocketAlive && !carryOverFlag) {
		//FILE_LOG(logERROR) << "Listening_Thread " << index << ": UDP Socket not created or shut down earlier";
		(*((uint32_t*)buffer)) = 0;
		StopListening(buffer);
		return;
	}

	//get data
	if ((*status != TRANSMITTING && udpSocketAlive) || carryOverFlag) {
		if (*activated)
			rc = ListenToAnImage(buffer);
		else
			rc = CreateAnImage(buffer + generalData->fifoBufferHeaderSize);
	}


	/*//done acquiring (removing this, else the last incomplete image will not be sent, directly going to dummy msg)
	if ((*status == TRANSMITTING) || ( (!(*activated)) && (rc == 0)) ) {
		StopListening(buffer);
		return;
	}*/

	//error check, (should not be here) if not transmitting yet (previous if) rc should be > 0
	if (rc <= 0) {
		//cprintf(RED,"%d Socket shut down while waiting for future packet. udpsocketalive:%d\n",index, udpSocketAlive );
		if (!udpSocketAlive) {
			(*((uint32_t*)buffer)) = 0;
			StopListening(buffer);
		}else
			fifo->FreeAddress(buffer);
		return;
	}

	(*((uint32_t*)buffer)) = rc;
	(*((uint64_t*)(buffer + FIFO_HEADER_NUMBYTES ))) = currentFrameIndex;		//for those returning earlier
	currentFrameIndex++;

	//push into fifo
	fifo->PushAddress(buffer);

	//Statistics
	numFramesStatistic++;
	if (numFramesStatistic >=  generalData->maxFramesPerFile)
		PrintFifoStatistics();
}



void Listener::StopListening(char* buf) {
	(*((uint32_t*)buf)) = DUMMY_PACKET_VALUE;
	fifo->PushAddress(buf);
	StopRunning();
	if (udpSocket) {
		delete udpSocket;
		udpSocket = 0;
	}
#ifdef VERBOSE
	bprintf(GREEN,"%d: Listening Packets (%u) : %llu\n", index, *udpPortNumber, numPacketsCaught);
	bprintf(GREEN,"%d: Listening Completed\n", index);
#endif
}


/* buf includes the fifo header and packet header */
uint32_t Listener::ListenToAnImage(char* buf) {
	int rc = 0;
	uint64_t fnum = 0, bid = 0;
	uint32_t pnum = 0, snum = 0;
	uint32_t numpackets = 0;
	uint32_t dsize = generalData->dataSize;
	uint32_t hsize = generalData->headerSizeinPacket; //(includes empty header)
	uint32_t esize = generalData->emptyHeader;
	uint32_t fifohsize = generalData->fifoBufferHeaderSize;
	uint32_t pperFrame = generalData->packetsPerFrame;
	bool isHeaderEmpty = true;
	sls_detector_header* old_header = 0;
	sls_detector_header* new_header = 0;


	//reset to -1
	memset(buf, 0, fifohsize);
	memset(buf + fifohsize, 0xFF, generalData->imageSize);
	new_header = (sls_detector_header*) (buf + FIFO_HEADER_NUMBYTES);


	//look for carry over
	if (carryOverFlag) {
		 //bprintf(RED,"%d carry flag\n",index);
		//check if its the current image packet
		// -------------------------- new header ----------------------------------------------------------------------
		if (standardheader) {
			old_header = (sls_detector_header*) (carryOverPacket + esize);
			fnum = old_header->frameNumber;
			pnum = old_header->packetNumber;
		}
		// -------------------old header -----------------------------------------------------------------------------
		else {
			generalData->GetHeaderInfo(index, carryOverPacket + esize,
					*dynamicRange, fnum, pnum, snum, bid);
		}
		//------------------------------------------------------------------------------------------------------------
		if (fnum != currentFrameIndex) {
			if (fnum < currentFrameIndex) {
				bprintf(RED,"Error:(Weird), With carry flag: Frame number %lu less than current frame number %lu\n", fnum, currentFrameIndex);
				return 0;
			}
			new_header->packetNumber = numpackets;
			return generalData->imageSize;
		}

		//copy packet
		switch(myDetectorType) {
		//for gotthard, 1st packet: 4 bytes fnum, CACA					   + CACA, 639*2 bytes data
		//				2nd packet: 4 bytes fnum, previous 1*2 bytes data  + 640*2 bytes data !!
		case GOTTHARD:
			if(!pnum)
				memcpy(buf + fifohsize , carryOverPacket + hsize+4, dsize-2);
			else
				memcpy(buf + fifohsize + dsize - 2, carryOverPacket + hsize, dsize+2);
			break;
		default:
			memcpy(buf + fifohsize + (pnum * dsize), carryOverPacket + hsize, dsize);
			break;
		}

		carryOverFlag = false;
		numpackets++;					//number of packets in this image (each time its copied to buf)

		//writer header
		if(isHeaderEmpty) {
			// -------------------------- new header ----------------------------------------------------------------------
			if (standardheader) {
				memcpy((char*)new_header, (char*)old_header, sizeof(sls_detector_header));
			}
			// -------------------old header ------------------------------------------------------------------------------
			else {
				memset(new_header, 0, sizeof(sls_detector_header));
				new_header->frameNumber = fnum;
				new_header->packetNumber = pperFrame;
				/*new_header->xCoord = index;  given by det packet, also for ycoord, zcoord */
				new_header->detType = (uint8_t) generalData->myDetectorType;
				new_header->version = (uint8_t) SLS_DETECTOR_HEADER_VERSION;
			}
			//------------------------------------------------------------------------------------------------------------
			isHeaderEmpty = false;
		}

	}




	//until last packet isHeaderEmpty to account for gotthard short frame, else never entering this loop)
	while ( numpackets < pperFrame) {
		//listen to new packet
		rc = 0;
		if (udpSocketAlive){
			rc = udpSocket->ReceiveDataOnly(listeningPacket);
		}
		if(rc <= 0) {
			if (numpackets == 0) return 0;	//empty image

			new_header->packetNumber = numpackets; 	//number of packets caught
			return generalData->imageSize;	//empty packet now, but not empty image
		}

		//update parameters
		numPacketsCaught++;					//record immediately to get more time before socket shutdown
		numPacketsStatistic++;

		// -------------------------- new header ----------------------------------------------------------------------
		if (standardheader) {
			old_header = (sls_detector_header*) (listeningPacket + esize);
			fnum = old_header->frameNumber;
			pnum = old_header->packetNumber;
		}
		// -------------------old header -----------------------------------------------------------------------------
		else {
			generalData->GetHeaderInfo(index, listeningPacket + esize,
					*dynamicRange, fnum, pnum, snum, bid);
		}
		//------------------------------------------------------------------------------------------------------------

		// Eiger Firmware in a weird state
		if (myDetectorType == EIGER && fnum == 0) {
			cprintf(RED,"[%u]: Got Frame Number Zero from Firmware. Discarding Packet\n", *udpPortNumber);
			numPacketsCaught--;
			return 0;
		}

		lastCaughtFrameIndex = fnum;


#ifdef VERBOSE
		//if (!index)
		bprintf(GREEN,"Listening %d: currentfindex:%lu, fnum:%lu,   pnum:%u numpackets:%u\n",
				index,currentFrameIndex, fnum, pnum, numpackets);
#endif
		if (!measurementStartedFlag)
			RecordFirstIndices(fnum);

		//future packet	by looking at image number  (all other detectors)
		if (fnum != currentFrameIndex) {
			//bprintf(RED,"setting carry over flag to true num:%llu nump:%u\n",fnum, numpackets );
			carryOverFlag = true;
			memcpy(carryOverPacket,listeningPacket, generalData->packetSize);

			new_header->packetNumber = numpackets; 	//number of packets caught
			return generalData->imageSize;
		}

		//copy packet
		switch(myDetectorType) {
		//for gotthard, 1st packet: 4 bytes fnum, CACA					   + CACA, 639*2 bytes data
		//				2nd packet: 4 bytes fnum, previous 1*2 bytes data  + 640*2 bytes data !!
		case GOTTHARD:
			if(!pnum)
				memcpy(buf + fifohsize + (pnum * dsize), listeningPacket + hsize+4, dsize-2);
			else
				memcpy(buf + fifohsize + (pnum * dsize) - 2, listeningPacket + hsize, dsize+2);
			break;
		default:
			memcpy(buf + fifohsize + (pnum * dsize), listeningPacket + hsize, dsize);
			break;
		}
		numpackets++;			//number of packets in this image (each time its copied to buf)
		if(isHeaderEmpty) {
			// -------------------------- new header ----------------------------------------------------------------------
			if (standardheader) {
				memcpy((char*)new_header, (char*)old_header, sizeof(sls_detector_header));
			}
			// -------------------old header ------------------------------------------------------------------------------
			else {
				memset(new_header, 0, sizeof(sls_detector_header));
				new_header->frameNumber = fnum;
				new_header->packetNumber = pperFrame;
				/*new_header->xCoord = index;  given by det packet, also for ycoord, zcoord */
				new_header->detType = (uint8_t) generalData->myDetectorType;
				new_header->version = (uint8_t) SLS_DETECTOR_HEADER_VERSION;
			}
			//------------------------------------------------------------------------------------------------------------
			isHeaderEmpty = false;
		}
	}

	new_header->packetNumber = numpackets; 	//number of packets caught
	return generalData->imageSize;
}


uint32_t Listener::CreateAnImage(char* buf) {

	if (!measurementStartedFlag)
		RecordFirstIndices(0);

	if (currentFrameIndex == *numImages)
		return 0;

	//update parameters
	numPacketsCaught++;		//record immediately to get more time before socket shutdown

	//reset data to -1
	memset(buf, 0xFF, generalData->dataSize);

	return generalData->imageSize;
}


void Listener::PrintFifoStatistics() {
#ifdef VERBOSE
	cout << "numFramesStatistic:" << numFramesStatistic << " numPacketsStatistic:" << numPacketsStatistic << endl;
#endif
	//calculate packet loss
	int64_t loss = -1;
	loss = (numFramesStatistic*(generalData->packetsPerFrame)) - numPacketsStatistic;
	numPacketsStatistic = 0;
	numFramesStatistic = 0;

	if (loss)
		bprintf(RED,"[%u]:  Packet_Loss:%lu  Used_Fifo_Max_Level:%d \tFree_Slots_Min_Level:%d \tCurrent_Frame#:%lu\n",
				*udpPortNumber,loss, fifo->GetMaxLevelForFifoBound() , fifo->GetMinLevelForFifoFree(), currentFrameIndex);
	else
		bprintf(GREEN,"[%u]:  Packet_Loss:%lu  Used_Fifo_Max_Level:%d  \tFree_Slots_Min_Level:%d \tCurrent_Frame#:%lu\n",
				*udpPortNumber,loss, fifo->GetMaxLevelForFifoBound(), fifo->GetMinLevelForFifoFree(), currentFrameIndex);
}
