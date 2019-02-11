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

const std::string Listener::TypeName = "Listener";


Listener::Listener(int ind, detectorType dtype, Fifo*& f, runStatus* s,
        uint32_t* portno, char* e, uint64_t* nf, uint32_t* dr,
        uint32_t* us, uint32_t* as, uint32_t* fpf,
		frameDiscardPolicy* fdp, bool* act, bool* depaden, bool* sm) :
		ThreadObject(ind),
		runningFlag(0),
		generalData(0),
		fifo(f),
		myDetectorType(dtype),
		status(s),
		udpSocket(0),
		udpPortNumber(portno),
		eth(e),
		numImages(nf),
		dynamicRange(dr),
		udpSocketBufferSize(us),
		actualUDPSocketBufferSize(as),
		framesPerFile(fpf),
		frameDiscardMode(fdp),
		activated(act),
		deactivatedPaddingEnable(depaden),
		silentMode(sm),
		row(0),
		column(0),
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
		udpSocketAlive(0),
		numPacketsStatistic(0),
		numFramesStatistic(0),
		oddStartingPacket(true)
{
	if(ThreadObject::CreateThread() == FAIL)
	    throw std::exception();

	FILE_LOG(logDEBUG) << "Listener " << ind << " created";
}


Listener::~Listener() {
	if (udpSocket){
		delete udpSocket;
		sem_post(&semaphore_socket);
    	sem_destroy(&semaphore_socket);
	} 
	if (carryOverPacket) delete [] carryOverPacket;
	if (listeningPacket) delete [] listeningPacket;
	ThreadObject::DestroyThread();
}

/** getters */
std::string Listener::GetType(){
	return TypeName;
}

bool Listener::IsRunning() {
	return runningFlag;
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
    runningFlag = true;
}


void Listener::StopRunning() {
    runningFlag = false;
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
    runningFlag = false;
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

	if(!(*silentMode)) {
		if (!index) cprintf(BLUE,"%d First Acquisition Index:%lu\n"
				"%d First Measurement Index:%lu\n",
				index, firstAcquisitionIndex,
				index, firstMeasurementIndex);
	}
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

    if (!(*activated)) {
    	return OK;
    }

	//if eth is mistaken with ip address
	if (strchr(eth,'.') != NULL){
	    memset(eth, 0, MAX_STR_LENGTH);
	}
	if(!strlen(eth)){
		FILE_LOG(logWARNING) << "eth is empty. Listening to all";
	}

	ShutDownUDPSocket();

	try{
		genericSocket* g = new genericSocket(*udpPortNumber, genericSocket::UDP,
				generalData->packetSize, (strlen(eth)?eth:NULL), generalData->headerPacketSize,
				*udpSocketBufferSize);
		udpSocket = g;
		FILE_LOG(logINFO) << index << ": UDP port opened at port " << *udpPortNumber;
	} catch (...) {
		FILE_LOG(logERROR) << "Could not create UDP socket on port " << *udpPortNumber;
		return FAIL;
	}

	udpSocketAlive = true;
    sem_init(&semaphore_socket,1,0);

    // doubled due to kernel bookkeeping (could also be less due to permissions)
    *actualUDPSocketBufferSize = udpSocket->getActualUDPSocketBufferSize();

	return OK;
}



void Listener::ShutDownUDPSocket() {
	if(udpSocket){
		udpSocketAlive = false;
		udpSocket->ShutDownSocket();
		FILE_LOG(logINFO) << "Shut down of UDP port " << *udpPortNumber;
		fflush(stdout);
		// wait only if the threads have started as it is the threads that
		//give a post to semaphore(at stopListening)
		if (runningFlag)
		    sem_wait(&semaphore_socket);
        delete udpSocket;
        udpSocket = 0;
	    sem_destroy(&semaphore_socket);
	}
}


int Listener::CreateDummySocketForUDPSocketBufferSize(uint32_t s) {
    FILE_LOG(logINFO) << "Testing UDP Socket Buffer size with test port " << *udpPortNumber;

    if (!(*activated)) {
    	*actualUDPSocketBufferSize = (s*2);
    	return OK;
    }

    uint32_t temp = *udpSocketBufferSize;
    *udpSocketBufferSize = s;

    //if eth is mistaken with ip address
    if (strchr(eth,'.') != NULL){
        memset(eth, 0, MAX_STR_LENGTH);
    }

    // shutdown if any open
    if(udpSocket){
        udpSocket->ShutDownSocket();
        delete udpSocket;
        udpSocket = 0;
    }

    //create dummy socket
    try {
    	udpSocket = new genericSocket(*udpPortNumber, genericSocket::UDP,
            generalData->packetSize, (strlen(eth)?eth:NULL), generalData->headerPacketSize,
            *udpSocketBufferSize);
    } catch (...) {
        FILE_LOG(logERROR) << "Could not create a test UDP socket on port " << *udpPortNumber;
        return FAIL;
    }


    // doubled due to kernel bookkeeping (could also be less due to permissions)
    *actualUDPSocketBufferSize = udpSocket->getActualUDPSocketBufferSize();
    if (*actualUDPSocketBufferSize != (s*2)) {
        *udpSocketBufferSize = temp;
    }


    // shutdown socket
    if(udpSocket){
        udpSocketAlive = false;
        udpSocket->ShutDownSocket();
        delete udpSocket;
        udpSocket = 0;
    }

    return OK;
}

void Listener::SetHardCodedPosition(uint16_t r, uint16_t c) {
	row = r;
	column = c;
}

void Listener::ThreadExecution() {
	char* buffer;
	int rc = 0;

	fifo->GetNewAddress(buffer);
#ifdef FIFODEBUG
	cprintf(GREEN,"Listener %d, pop 0x%p buffer:%s\n", index,(void*)(buffer),buffer);
#endif

	//udpsocket doesnt exist
	if (*activated && !udpSocketAlive && !carryOverFlag) {
		//FILE_LOG(logERROR) << "Listening_Thread " << index << ": UDP Socket not created or shut down earlier";
		(*((uint32_t*)buffer)) = 0;
		StopListening(buffer);
		return;
	}

	//get data
	if ((*status != TRANSMITTING && (!(*activated) || udpSocketAlive)) || carryOverFlag) {
		rc = ListenToAnImage(buffer);
	}


	//error check, (should not be here) if not transmitting yet (previous if) rc should be > 0
	if (rc == 0) {
		//cprintf(RED,"%d Socket shut down while waiting for future packet. udpsocketalive:%d\n",index, udpSocketAlive );
		if (!udpSocketAlive) {
			(*((uint32_t*)buffer)) = 0;
			StopListening(buffer);
		}else
			fifo->FreeAddress(buffer);
		return;
	}

	// discarding image
	else if (rc < 0) {
		FILE_LOG(logDEBUG) <<  index << " discarding fnum:" << currentFrameIndex;
		fifo->FreeAddress(buffer);
		currentFrameIndex++;
		return;
	}

	(*((uint32_t*)buffer)) = rc;
	(*((uint64_t*)(buffer + FIFO_HEADER_NUMBYTES ))) = currentFrameIndex;		//for those returning earlier
	currentFrameIndex++;

	//push into fifo
	fifo->PushAddress(buffer);

	//Statistics
	if(!(*silentMode)) {
		numFramesStatistic++;
		if (numFramesStatistic >=
				//second condition also for infinite #number of frames
				(((*framesPerFile) == 0) ? STATISTIC_FRAMENUMBER_INFINITE : (*framesPerFile)) )
			PrintFifoStatistics();
	}
}



void Listener::StopListening(char* buf) {
	(*((uint32_t*)buf)) = DUMMY_PACKET_VALUE;
	fifo->PushAddress(buf);
	StopRunning();

	 sem_post(&semaphore_socket);
#ifdef VERBOSE
	cprintf(GREEN,"%d: Listening Packets (%u) : %llu\n", index, *udpPortNumber, numPacketsCaught);
	cprintf(GREEN,"%d: Listening Completed\n", index);
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
	sls_receiver_header* new_header = 0;
	bool standardheader = generalData->standardheader;
	uint32_t corrected_dsize = dsize - ((pperFrame * dsize) - generalData->imageSize);


	//reset to -1
	memset(buf, 0, fifohsize);
	/*memset(buf + fifohsize, 0xFF, generalData->imageSize);*/
	new_header = (sls_receiver_header*) (buf + FIFO_HEADER_NUMBYTES);

	// deactivated (eiger)
	if (!(*activated)) {
		// no padding
		if (!(*deactivatedPaddingEnable))
			return 0;
		// padding without setting bitmask (all missing packets padded in dataProcessor)
		if (currentFrameIndex >= *numImages)
			return 0;

		//(eiger) first fnum starts at 1
		if (!currentFrameIndex) {
			++currentFrameIndex;
		}
		new_header->detHeader.frameNumber = currentFrameIndex;
		new_header->detHeader.row = row;
		new_header->detHeader.column = column;
		new_header->detHeader.detType = (uint8_t) generalData->myDetectorType;
		new_header->detHeader.version = (uint8_t) SLS_DETECTOR_HEADER_VERSION;
		return generalData->imageSize;
	}



	//look for carry over
	if (carryOverFlag) {
		 //cprintf(RED,"%d carry flag\n",index);
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
					*dynamicRange, oddStartingPacket, fnum, pnum, snum, bid);
		}
		//------------------------------------------------------------------------------------------------------------
		if (fnum != currentFrameIndex) {
			if (fnum < currentFrameIndex) {
				cprintf(RED,"Error:(Weird), With carry flag: Frame number %lu less than current frame number %lu\n", fnum, currentFrameIndex);
				return 0;
			}
			switch(*frameDiscardMode) {
			case DISCARD_EMPTY_FRAMES:
				if (!numpackets)
					return -1;
				break;
			case DISCARD_PARTIAL_FRAMES:
				return -1;
			default:
				break;
			}
			new_header->detHeader.packetNumber = numpackets;
			if(isHeaderEmpty) {
				new_header->detHeader.row = row;
				new_header->detHeader.column = column;
			}
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
		case JUNGFRAUCTB:
			if (pnum == (pperFrame-1))
				memcpy(buf + fifohsize + (pnum * dsize), carryOverPacket + hsize, corrected_dsize);
			else
				memcpy(buf + fifohsize + (pnum * dsize), carryOverPacket + hsize,  dsize);
			break;
		default:
			memcpy(buf + fifohsize + (pnum * dsize), carryOverPacket + hsize, dsize);
			break;
		}

		carryOverFlag = false;
		++numpackets;					//number of packets in this image (each time its copied to buf)
		new_header->packetsMask[pnum] = 1;

		//writer header
		if(isHeaderEmpty) {
			// -------------------------- new header ----------------------------------------------------------------------
			if (standardheader) {
				memcpy((char*)new_header, (char*)old_header, sizeof(sls_detector_header));
			}
			// -------------------old header ------------------------------------------------------------------------------
			else {
				new_header->detHeader.frameNumber = fnum;
				new_header->detHeader.row = row;
				new_header->detHeader.column = column;
				new_header->detHeader.detType = (uint8_t) generalData->myDetectorType;
				new_header->detHeader.version = (uint8_t) SLS_DETECTOR_HEADER_VERSION;
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
		// end of acquisition
		if(rc <= 0) {
			if (numpackets == 0) return 0;	//empty image

			switch(*frameDiscardMode) {
			case DISCARD_EMPTY_FRAMES:
				if (!numpackets)
					return -1;
				break;
			case DISCARD_PARTIAL_FRAMES:
				return -1;
			default:
				break;
			}
			new_header->detHeader.packetNumber = numpackets; 	//number of packets caught
			if(isHeaderEmpty) {
				new_header->detHeader.row = row;
				new_header->detHeader.column = column;
			}
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
		    // set first packet to be odd or even (check required when switching from roi to no roi)
		    if (myDetectorType == GOTTHARD && !measurementStartedFlag) {
		        oddStartingPacket = generalData->SetOddStartingPacket(index, listeningPacket + esize);
		    }

			generalData->GetHeaderInfo(index, listeningPacket + esize,
					*dynamicRange, oddStartingPacket, fnum, pnum, snum, bid);
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
		cprintf(GREEN,"Listening %d: currentfindex:%lu, fnum:%lu,   pnum:%u numpackets:%u\n",
				index,currentFrameIndex, fnum, pnum, numpackets);
#endif
		if (!measurementStartedFlag)
			RecordFirstIndices(fnum);

		//future packet	by looking at image number  (all other detectors)
		if (fnum != currentFrameIndex) {
			//cprintf(RED,"setting carry over flag to true num:%llu nump:%u\n",fnum, numpackets );
			carryOverFlag = true;
			memcpy(carryOverPacket,listeningPacket, generalData->packetSize);

			switch(*frameDiscardMode) {
			case DISCARD_EMPTY_FRAMES:
				if (!numpackets)
					return -1;
				break;
			case DISCARD_PARTIAL_FRAMES:
				return -1;
			default:
				break;
			}
			new_header->detHeader.packetNumber = numpackets; 	//number of packets caught
			if(isHeaderEmpty) {
				new_header->detHeader.row = row;
				new_header->detHeader.column = column;
			}
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
		case JUNGFRAUCTB:
			if (pnum == (pperFrame-1))
				memcpy(buf + fifohsize + (pnum * dsize), listeningPacket + hsize, corrected_dsize);
			else
				memcpy(buf + fifohsize + (pnum * dsize), listeningPacket + hsize, dsize);
			break;
		default:
			memcpy(buf + fifohsize + (pnum * dsize), listeningPacket + hsize, dsize);
			break;
		}
		++numpackets;			//number of packets in this image (each time its copied to buf)
		new_header->packetsMask[pnum] = 1;

		if(isHeaderEmpty) {
			// -------------------------- new header ----------------------------------------------------------------------
			if (standardheader) {
				memcpy((char*)new_header, (char*)old_header, sizeof(sls_detector_header));
			}
			// -------------------old header ------------------------------------------------------------------------------
			else {
				new_header->detHeader.frameNumber = fnum;
				new_header->detHeader.row = row;
				new_header->detHeader.column = column;
				new_header->detHeader.detType = (uint8_t) generalData->myDetectorType;
				new_header->detHeader.version = (uint8_t) SLS_DETECTOR_HEADER_VERSION;
			}
			//------------------------------------------------------------------------------------------------------------
			isHeaderEmpty = false;
		}
	}

	// complete image
	new_header->detHeader.packetNumber = numpackets; 	//number of packets caught
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
		cprintf(RED,"[%u]:  Packet_Loss:%lu  Used_Fifo_Max_Level:%d \tFree_Slots_Min_Level:%d \tCurrent_Frame#:%lu\n",
				*udpPortNumber,loss, fifo->GetMaxLevelForFifoBound() , fifo->GetMinLevelForFifoFree(), currentFrameIndex);
	else
		cprintf(GREEN,"[%u]:  Packet_Loss:%lu  Used_Fifo_Max_Level:%d  \tFree_Slots_Min_Level:%d \tCurrent_Frame#:%lu\n",
				*udpPortNumber,loss, fifo->GetMaxLevelForFifoBound(), fifo->GetMinLevelForFifoFree(), currentFrameIndex);
}
