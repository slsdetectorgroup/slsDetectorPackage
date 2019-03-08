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
#include "container_utils.h" // For sls::make_unique<>

#include <iostream>
#include <errno.h>
#include <cstring>

const std::string Listener::TypeName = "Listener";


Listener::Listener(int ind, detectorType dtype, Fifo* f, runStatus* s,
        uint32_t* portno, char* e, uint64_t* nf, uint32_t* dr,
        uint64_t* us, uint64_t* as, uint32_t* fpf,
		frameDiscardPolicy* fdp, bool* act, bool* depaden, bool* sm) :
		ThreadObject(ind),
		runningFlag(0),
		generalData(nullptr),
		fifo(f),
		myDetectorType(dtype),
		status(s),
		udpSocket(nullptr),
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
		sem_post(&semaphore_socket);
    	sem_destroy(&semaphore_socket);
	} 

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


void Listener::SetFifo(Fifo* f) {
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
	carryOverPacket = sls::make_unique<char[]>(generalData->packetSize);
	memset(carryOverPacket.get(),0,generalData->packetSize);
	listeningPacket = sls::make_unique<char[]>(generalData->packetSize);
	memset(carryOverPacket.get(),0,generalData->packetSize);

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
		if (!index) {
			FILE_LOG(logINFOBLUE) << index <<
					" First Acquisition Index: " << firstAcquisitionIndex;
			FILE_LOG(logDEBUG1) << index << " First Measurement Index: " << firstMeasurementIndex;
		}
	}
}


void Listener::SetGeneralData(GeneralData* g) {
	generalData = g;
	generalData->Print();
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
	if (strchr(eth,'.') != nullptr){
	    memset(eth, 0, MAX_STR_LENGTH);
	}
	if(!strlen(eth)){
		FILE_LOG(logWARNING) << "eth is empty. Listening to all";
	}

	ShutDownUDPSocket();

	try{
	    udpSocket = sls::make_unique<genericSocket>(*udpPortNumber, genericSocket::UDP,
				generalData->packetSize, (strlen(eth)?eth:nullptr), generalData->headerPacketSize,
				*udpSocketBufferSize);
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
	    sem_destroy(&semaphore_socket);
	}
}


int Listener::CreateDummySocketForUDPSocketBufferSize(uint64_t s) {
    FILE_LOG(logINFO) << "Testing UDP Socket Buffer size with test port " << *udpPortNumber;

    if (!(*activated)) {
    	*actualUDPSocketBufferSize = (s*2);
    	return OK;
    }

    uint64_t temp = *udpSocketBufferSize;
    *udpSocketBufferSize = s;

    //if eth is mistaken with ip address
    if (strchr(eth,'.') != nullptr){
        memset(eth, 0, MAX_STR_LENGTH);
    }

    //create dummy socket
    try {
    	genericSocket g(*udpPortNumber, genericSocket::UDP,
            generalData->packetSize, (strlen(eth)?eth:nullptr), generalData->headerPacketSize,
            *udpSocketBufferSize);

        // doubled due to kernel bookkeeping (could also be less due to permissions)
        *actualUDPSocketBufferSize = g.getActualUDPSocketBufferSize();
        if (*actualUDPSocketBufferSize != (s*2)) {
            *udpSocketBufferSize = temp;
        }

    } catch (...) {
        FILE_LOG(logERROR) << "Could not create a test UDP socket on port " << *udpPortNumber;
        return FAIL;
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
	FILE_LOG(logDEBUG5) << "Listener " << index << ", "
			"pop 0x" << std::hex << (void*)(buffer) << std::dec << ":" << buffer;

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
	 FILE_LOG(logDEBUG1) << index << ": Listening Packets (" << *udpPortNumber << ") : " << numPacketsCaught;
	 FILE_LOG(logDEBUG1) << index << ": Listening Completed";
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
	sls_detector_header* old_header = nullptr;
	sls_receiver_header* new_header = nullptr;
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
		FILE_LOG(logDEBUG3) << index << "carry flag";
		//check if its the current image packet
		// -------------------------- new header ----------------------------------------------------------------------
		if (standardheader) {
			old_header = (sls_detector_header*) (&carryOverPacket[esize]);
			fnum = old_header->frameNumber;
			pnum = old_header->packetNumber;
		}
		// -------------------old header -----------------------------------------------------------------------------
		else {
			generalData->GetHeaderInfo(index, &carryOverPacket[esize],
					*dynamicRange, oddStartingPacket, fnum, pnum, snum, bid);
		}
		//------------------------------------------------------------------------------------------------------------
		if (fnum != currentFrameIndex) {
			if (fnum < currentFrameIndex) {
				FILE_LOG(logERROR) << "(Weird), With carry flag: Frame number " <<
						fnum << " less than current frame number " << currentFrameIndex;
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
				memcpy(buf + fifohsize , &carryOverPacket[hsize+4], dsize-2);
			else
				memcpy(buf + fifohsize + dsize - 2, &carryOverPacket[hsize], dsize+2);
			break;
		case CHIPTESTBOARD:
		case MOENCH:
			if (pnum == (pperFrame-1))
				memcpy(buf + fifohsize + (pnum * dsize), &carryOverPacket[hsize], corrected_dsize);
			else
				memcpy(buf + fifohsize + (pnum * dsize), &carryOverPacket[hsize],  dsize);
			break;
		default:
			memcpy(buf + fifohsize + (pnum * dsize), &carryOverPacket[hsize], dsize);
			break;
		}

		carryOverFlag = false;
		++numpackets;					//number of packets in this image (each time its copied to buf)
		new_header->packetsMask[((pnum < MAX_NUM_PACKETS) ? pnum : MAX_NUM_PACKETS)] = 1;

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
			rc = udpSocket->ReceiveDataOnly(&listeningPacket[0]);
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
			old_header = (sls_detector_header*) (&listeningPacket[esize]);
			fnum = old_header->frameNumber;
			pnum = old_header->packetNumber;
		}
		// -------------------old header -----------------------------------------------------------------------------
		else {
            // set first packet to be odd or even (check required when switching from roi to no roi)
            if (myDetectorType == GOTTHARD && !measurementStartedFlag) {
                oddStartingPacket = generalData->SetOddStartingPacket(index, &listeningPacket[esize]);
            }

			generalData->GetHeaderInfo(index, &listeningPacket[esize],
					*dynamicRange, oddStartingPacket, fnum, pnum, snum, bid);
		}
		//------------------------------------------------------------------------------------------------------------

		// Eiger Firmware in a weird state
		if (myDetectorType == EIGER && fnum == 0) {
			FILE_LOG(logERROR) << "[" << *udpPortNumber << "]: Got Frame Number "
					"Zero from Firmware. Discarding Packet";
			numPacketsCaught--;
			return 0;
		}

		lastCaughtFrameIndex = fnum;

		FILE_LOG(logDEBUG5) << "Listening " << index << ": currentfindex:" << currentFrameIndex <<
				", fnum:" << fnum << ", pnum:" << pnum << ", numpackets:" << numpackets;

		if (!measurementStartedFlag)
			RecordFirstIndices(fnum);

        if (pnum >= pperFrame ) {
            FILE_LOG(logERROR) << "Bad packet " << pnum <<
                    "(fnum: " << fnum << "), throwing away. "
                    "Packets caught so far: " << numpackets;
          return 0;   // bad packet
        }

		//future packet	by looking at image number  (all other detectors)
		if (fnum != currentFrameIndex) {
			carryOverFlag = true;
			memcpy(carryOverPacket.get(), &listeningPacket[0], generalData->packetSize);

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
				memcpy(buf + fifohsize + (pnum * dsize), &listeningPacket[hsize+4], dsize-2);
			else
				memcpy(buf + fifohsize + (pnum * dsize) - 2, &listeningPacket[hsize], dsize+2);
			break;
		case CHIPTESTBOARD:
		case MOENCH:
			if (pnum == (pperFrame-1))
				memcpy(buf + fifohsize + (pnum * dsize), &listeningPacket[hsize], corrected_dsize);
			else
				memcpy(buf + fifohsize + (pnum * dsize), &listeningPacket[hsize], dsize);
			break;
		default:
			memcpy(buf + fifohsize + (pnum * dsize), &listeningPacket[hsize], dsize);
			break;
		}
		++numpackets;			//number of packets in this image (each time its copied to buf)
		new_header->packetsMask[((pnum < MAX_NUM_PACKETS) ? pnum : MAX_NUM_PACKETS)] = 1;

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
	FILE_LOG(logDEBUG1) << "numFramesStatistic:" << numFramesStatistic << " numPacketsStatistic:" << numPacketsStatistic;

	//calculate packet loss
	int64_t loss = -1;
	loss = (numFramesStatistic*(generalData->packetsPerFrame)) - numPacketsStatistic;
	numPacketsStatistic = 0;
	numFramesStatistic = 0;

	FILE_LOG(loss ? logINFORED : logINFOGREEN) << "[" << *udpPortNumber << "]:  "
			"Packet_Loss:" << loss <<
			"  Used_Fifo_Max_Level:" << fifo->GetMaxLevelForFifoBound() <<
			" \tFree_Slots_Min_Level:" << fifo->GetMinLevelForFifoFree() <<
			" \tCurrent_Frame#:" << currentFrameIndex;

}
