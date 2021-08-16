/************************************************
 * @file Listener.cpp
 * @short creates the listener thread that
 * listens to udp sockets, writes data to memory
 * & puts pointers to their memory addresses into fifos
 ***********************************************/

#include "Listener.h"
#include "Fifo.h"
#include "GeneralData.h"
#include "sls/UdpRxSocket.h"
#include "sls/container_utils.h" // For sls::make_unique<>
#include "sls/network_utils.h"
#include "sls/sls_detector_exceptions.h"

#include <cerrno>
#include <cstring>
#include <iostream>

const std::string Listener::TypeName = "Listener";

Listener::Listener(int ind, detectorType dtype, Fifo *f,
                   std::atomic<runStatus> *s, uint32_t *portno, std::string *e,
                   uint64_t *nf, int *us, int *as, uint32_t *fpf,
                   frameDiscardPolicy *fdp, bool *act, bool* detds, bool *depaden, bool *sm)
    : ThreadObject(ind, TypeName), fifo(f), myDetectorType(dtype), status(s),
      udpPortNumber(portno), eth(e), numImages(nf), udpSocketBufferSize(us),
      actualUDPSocketBufferSize(as), framesPerFile(fpf), frameDiscardMode(fdp),
      activated(act), detectorDataStream(detds), deactivatedPaddingEnable(depaden), silentMode(sm) {
    LOG(logDEBUG) << "Listener " << ind << " created";
}

Listener::~Listener() = default;

uint64_t Listener::GetPacketsCaught() const { return numPacketsCaught; }

uint64_t Listener::GetLastFrameIndexCaught() const {
    return lastCaughtFrameIndex;
}

uint64_t Listener::GetNumMissingPacket(bool stoppedFlag,
                                       uint64_t numPackets) const {
    if (!stoppedFlag) {
        return (numPackets - numPacketsCaught);
    }
    if (numPacketsCaught == 0) {
        return numPacketsCaught;
    }
    return (lastCaughtFrameIndex - firstIndex + 1) *
               generalData->packetsPerFrame -
           numPacketsCaught;
}

void Listener::SetFifo(Fifo *f) { fifo = f; }

void Listener::ResetParametersforNewAcquisition() {
    StopRunning();
    startedFlag = false;
    numPacketsCaught = 0;
    firstIndex = 0;
    currentFrameIndex = 0;
    lastCaughtFrameIndex = 0;
    carryOverFlag = false;
    uint32_t packetSize = generalData->packetSize;
    if (myDetectorType == GOTTHARD2 && index != 0) {
        packetSize = generalData->vetoPacketSize;
    }
    carryOverPacket = sls::make_unique<char[]>(packetSize);
    memset(carryOverPacket.get(), 0, packetSize);
    listeningPacket = sls::make_unique<char[]>(packetSize);
    memset(carryOverPacket.get(), 0, packetSize);

    numPacketsStatistic = 0;
    numFramesStatistic = 0;
    // reset fifo statistic
    fifo->GetMaxLevelForFifoBound();
    fifo->GetMinLevelForFifoFree();
}

void Listener::RecordFirstIndex(uint64_t fnum) {
    // listen to this fnum, later +1
    currentFrameIndex = fnum;

    startedFlag = true;
    firstIndex = fnum;

    if (!(*silentMode)) {
        if (!index) {
            LOG(logINFOBLUE) << index << " First Index: " << firstIndex;
        }
    }
}

void Listener::SetGeneralData(GeneralData *g) { generalData = g; }

void Listener::CreateUDPSockets() {
    if (!(*activated) || !(*detectorDataStream)) {
        return;
    }

    // if eth is mistaken with ip address
    if ((*eth).find('.') != std::string::npos) {
        (*eth) = "";
    }
    if (!(*eth).length()) {
        LOG(logWARNING) << "eth is empty. Listening to all";
    }

    ShutDownUDPSocket();

    uint32_t packetSize = generalData->packetSize;
    if (myDetectorType == GOTTHARD2 && index != 0) {
        packetSize = generalData->vetoPacketSize;
    }

    // InterfaceNameToIp(eth).str().c_str()
    try {
        udpSocket = sls::make_unique<sls::UdpRxSocket>(
            *udpPortNumber, packetSize,
            ((*eth).length() ? sls::InterfaceNameToIp(*eth).str().c_str()
                             : nullptr),
            *udpSocketBufferSize);
        LOG(logINFO) << index << ": UDP port opened at port " << *udpPortNumber;
    } catch (...) {
        throw sls::RuntimeError("Could not create UDP socket on port " +
                                std::to_string(*udpPortNumber));
    }

    udpSocketAlive = true;

    // doubled due to kernel bookkeeping (could also be less due to permissions)
    *actualUDPSocketBufferSize = udpSocket->getBufferSize();
}

void Listener::ShutDownUDPSocket() {
    if (udpSocket) {
        udpSocketAlive = false;
        udpSocket->Shutdown();
        LOG(logINFO) << "Shut down of UDP port " << *udpPortNumber;
    }
}

void Listener::CreateDummySocketForUDPSocketBufferSize(int s) {
    LOG(logINFO) << "Testing UDP Socket Buffer size " << s << " with test port "
                 << *udpPortNumber;

    if (!(*activated) || !(*detectorDataStream)) {
        *actualUDPSocketBufferSize = (s * 2);
        return;
    }

    int temp = *udpSocketBufferSize;
    *udpSocketBufferSize = s;

    // if eth is mistaken with ip address
    if ((*eth).find('.') != std::string::npos) {
        (*eth) = "";
    }

    uint32_t packetSize = generalData->packetSize;
    if (myDetectorType == GOTTHARD2 && index != 0) {
        packetSize = generalData->vetoPacketSize;
    }

    // create dummy socket
    try {
        sls::UdpRxSocket g(*udpPortNumber, packetSize,
                           ((*eth).length()
                                ? sls::InterfaceNameToIp(*eth).str().c_str()
                                : nullptr),
                           *udpSocketBufferSize);

        // doubled due to kernel bookkeeping (could also be less due to
        // permissions)
        *actualUDPSocketBufferSize = g.getBufferSize();
        if (*actualUDPSocketBufferSize == -1) {
            *udpSocketBufferSize = temp;
        } else {
            *udpSocketBufferSize = (*actualUDPSocketBufferSize) / 2;
        }

    } catch (...) {
        throw sls::RuntimeError("Could not create a test UDP socket on port " +
                                std::to_string(*udpPortNumber));
    }
}

void Listener::SetHardCodedPosition(uint16_t r, uint16_t c) {
    row = r;
    column = c;
}

void Listener::ThreadExecution() {
    char *buffer;
    int rc = 0;

    fifo->GetNewAddress(buffer);
    LOG(logDEBUG5) << "Listener " << index
                   << ", "
                      "pop 0x"
                   << std::hex << (void *)(buffer) << std::dec << ":" << buffer;

    // udpsocket doesnt exist
    if (*activated && *detectorDataStream && !udpSocketAlive && !carryOverFlag) {
        // LOG(logERROR) << "Listening_Thread " << index << ": UDP Socket not
        // created or shut down earlier";
        (*((uint32_t *)buffer)) = 0;
        StopListening(buffer);
        return;
    }

    // get data
    if ((*status != TRANSMITTING && (!(*activated) || !(*detectorDataStream) || udpSocketAlive)) ||
        carryOverFlag) {
        rc = ListenToAnImage(buffer);
    }

    // error check, (should not be here) if not transmitting yet (previous if)
    // rc should be > 0
    if (rc == 0) {
        if (!udpSocketAlive) {
            (*((uint32_t *)buffer)) = 0;
            StopListening(buffer);
        } else
            fifo->FreeAddress(buffer);
        return;
    }

    // discarding image
    else if (rc < 0) {
        LOG(logDEBUG) << index << " discarding fnum:" << currentFrameIndex;
        fifo->FreeAddress(buffer);
        currentFrameIndex++;
        return;
    }

    (*((uint32_t *)buffer)) = rc;
    (*((uint64_t *)(buffer + FIFO_HEADER_NUMBYTES))) =
        currentFrameIndex; // for those returning earlier
    currentFrameIndex++;

    // push into fifo
    fifo->PushAddress(buffer);

    // Statistics
    if (!(*silentMode)) {
        numFramesStatistic++;
        if (numFramesStatistic >=
            // second condition also for infinite #number of frames
            (((*framesPerFile) == 0) ? STATISTIC_FRAMENUMBER_INFINITE
                                     : (*framesPerFile)))
            PrintFifoStatistics();
    }
}

void Listener::StopListening(char *buf) {
    (*((uint32_t *)buf)) = DUMMY_PACKET_VALUE;
    fifo->PushAddress(buf);
    StopRunning();
    LOG(logDEBUG1) << index << ": Listening Packets (" << *udpPortNumber
                   << ") : " << numPacketsCaught;
    LOG(logDEBUG1) << index << ": Listening Completed";
}

/* buf includes the fifo header and packet header */
uint32_t Listener::ListenToAnImage(char *buf) {

    int rc = 0;
    uint64_t fnum = 0;
    uint32_t pnum = 0;
    uint64_t bnum = 0;
    uint32_t numpackets = 0;
    uint32_t dsize = generalData->dataSize;
    uint32_t imageSize = generalData->imageSize;
    uint32_t packetSize = generalData->packetSize;
    uint32_t hsize = generalData->headerSizeinPacket;
    uint32_t fifohsize = generalData->fifoBufferHeaderSize;
    bool standardheader = generalData->standardheader;
    if (myDetectorType == GOTTHARD2 && index != 0) {
        dsize = generalData->vetoDataSize;
        imageSize = generalData->vetoImageSize;
        packetSize = generalData->vetoPacketSize;
        hsize = generalData->vetoHsize;
        standardheader = false;
    }
    uint32_t pperFrame = generalData->packetsPerFrame;
    bool isHeaderEmpty = true;
    sls_detector_header *old_header = nullptr;
    sls_receiver_header *new_header = nullptr;
    uint32_t corrected_dsize = dsize - ((pperFrame * dsize) - imageSize);

    // reset to -1
    memset(buf, 0, fifohsize);
    new_header = (sls_receiver_header *)(buf + FIFO_HEADER_NUMBYTES);

    // deactivated port (eiger)
    if (!(*detectorDataStream)) {
        return 0;
    }
    // deactivated (eiger)
    if (!(*activated)) {
        // no padding
        if (!(*deactivatedPaddingEnable))
            return 0;
        // padding without setting bitmask (all missing packets padded in
        // dataProcessor)
        if (currentFrameIndex >= *numImages)
            return 0;

        //(eiger) first fnum starts at 1
        if (!currentFrameIndex) {
            ++currentFrameIndex;
        }
        new_header->detHeader.frameNumber = currentFrameIndex;
        new_header->detHeader.row = row;
        new_header->detHeader.column = column;
        new_header->detHeader.detType = (uint8_t)generalData->myDetectorType;
        new_header->detHeader.version = (uint8_t)SLS_DETECTOR_HEADER_VERSION;
        return imageSize;
    }

    // look for carry over
    if (carryOverFlag) {
        LOG(logDEBUG3) << index << "carry flag";
        // check if its the current image packet
        // -------------------------- new header
        // ----------------------------------------------------------------------
        if (standardheader) {
            old_header = (sls_detector_header *)(&carryOverPacket[0]);
            fnum = old_header->frameNumber;
            pnum = old_header->packetNumber;
        }
        // -------------------old header
        // -----------------------------------------------------------------------------
        else {
            generalData->GetHeaderInfo(index, &carryOverPacket[0],
                                       oddStartingPacket, fnum, pnum, bnum);
        }
        //------------------------------------------------------------------------------------------------------------
        if (fnum != currentFrameIndex) {
            if (fnum < currentFrameIndex) {
                LOG(logERROR)
                    << "(Weird), With carry flag: Frame number " << fnum
                    << " less than current frame number " << currentFrameIndex;
                carryOverFlag = false;
                return 0;
            }
            switch (*frameDiscardMode) {
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
            if (isHeaderEmpty) {
                new_header->detHeader.row = row;
                new_header->detHeader.column = column;
            }
            return imageSize;
        }

        // copy packet
        switch (myDetectorType) {
        // for gotthard, 1st packet: 4 bytes fnum, CACA
        // + CACA, 639*2 bytes data 				2nd packet: 4
        // bytes fnum, previous 1*2 bytes data  + 640*2 bytes data !!
        case GOTTHARD:
            if (!pnum)
                memcpy(buf + fifohsize, &carryOverPacket[hsize + 4], dsize - 2);
            else
                memcpy(buf + fifohsize + dsize - 2, &carryOverPacket[hsize],
                       dsize + 2);
            break;
        case CHIPTESTBOARD:
        case MOENCH:
            if (pnum == (pperFrame - 1))
                memcpy(buf + fifohsize + (pnum * dsize),
                       &carryOverPacket[hsize], corrected_dsize);
            else
                memcpy(buf + fifohsize + (pnum * dsize),
                       &carryOverPacket[hsize], dsize);
            break;
        default:
            memcpy(buf + fifohsize + (pnum * dsize), &carryOverPacket[hsize],
                   dsize);
            break;
        }

        carryOverFlag = false;
        ++numpackets; // number of packets in this image (each time its copied
                      // to buf)
        new_header->packetsMask[(
            (pnum < MAX_NUM_PACKETS) ? pnum : MAX_NUM_PACKETS - 1)] = 1;

        // writer header
        if (isHeaderEmpty) {
            // -------------------------- new header
            // ----------------------------------------------------------------------
            if (standardheader) {
                memcpy((char *)new_header, (char *)old_header,
                       sizeof(sls_detector_header));
            }
            // -------------------old header
            // ------------------------------------------------------------------------------
            else {
                new_header->detHeader.frameNumber = fnum;
                new_header->detHeader.bunchId = bnum;
                new_header->detHeader.row = row;
                new_header->detHeader.column = column;
                new_header->detHeader.detType =
                    (uint8_t)generalData->myDetectorType;
                new_header->detHeader.version =
                    (uint8_t)SLS_DETECTOR_HEADER_VERSION;
            }
            //------------------------------------------------------------------------------------------------------------
            isHeaderEmpty = false;
        }
    }

    // until last packet isHeaderEmpty to account for gotthard short frame, else
    // never entering this loop)
    while (numpackets < pperFrame) {
        // listen to new packet
        rc = 0;
        if (udpSocketAlive) {
            rc = udpSocket->ReceiveDataOnly(&listeningPacket[0]);
        }
        // end of acquisition
        if (rc <= 0) {
            if (numpackets == 0)
                return 0; // empty image

            switch (*frameDiscardMode) {
            case DISCARD_EMPTY_FRAMES:
                if (!numpackets)
                    return -1;
                break;
            case DISCARD_PARTIAL_FRAMES:
                return -1;
            default:
                break;
            }
            new_header->detHeader.packetNumber =
                numpackets; // number of packets caught
            if (isHeaderEmpty) {
                new_header->detHeader.row = row;
                new_header->detHeader.column = column;
            }
            return imageSize; // empty packet now, but not empty image
        }

        // update parameters
        numPacketsCaught++; // record immediately to get more time before socket
                            // shutdown
        numPacketsStatistic++;

        // -------------------------- new header
        // ----------------------------------------------------------------------
        if (standardheader) {
            old_header = (sls_detector_header *)(&listeningPacket[0]);
            fnum = old_header->frameNumber;
            pnum = old_header->packetNumber;
        }
        // -------------------old header
        // -----------------------------------------------------------------------------
        else {
            // set first packet to be odd or even (check required when switching
            // from roi to no roi)
            if (myDetectorType == GOTTHARD && !startedFlag) {
                oddStartingPacket = generalData->SetOddStartingPacket(
                    index, &listeningPacket[0]);
            }

            generalData->GetHeaderInfo(index, &listeningPacket[0],
                                       oddStartingPacket, fnum, pnum, bnum);
        }
        //------------------------------------------------------------------------------------------------------------

        // Eiger Firmware in a weird state
        if (myDetectorType == EIGER && fnum == 0) {
            LOG(logERROR) << "[" << *udpPortNumber
                          << "]: Got Frame Number "
                             "Zero from Firmware. Discarding Packet";
            numPacketsCaught--;
            return 0;
        }

        lastCaughtFrameIndex = fnum;

        LOG(logDEBUG1) << "Listening " << index
                       << ": currentfindex:" << currentFrameIndex
                       << ", fnum:" << fnum << ", pnum:" << pnum
                       << ", numpackets:" << numpackets;

        if (!startedFlag)
            RecordFirstIndex(fnum);

        if (pnum >= pperFrame) {
            LOG(logERROR) << "Bad packet " << pnum << "(fnum: " << fnum
                          << "), throwing away. "
                             "Packets caught so far: "
                          << numpackets;
            return 0; // bad packet
        }

        // future packet	by looking at image number  (all other
        // detectors)
        if (fnum != currentFrameIndex) {
            carryOverFlag = true;
            memcpy(carryOverPacket.get(), &listeningPacket[0], packetSize);

            switch (*frameDiscardMode) {
            case DISCARD_EMPTY_FRAMES:
                if (!numpackets)
                    return -1;
                break;
            case DISCARD_PARTIAL_FRAMES:
                return -1;
            default:
                break;
            }
            new_header->detHeader.packetNumber =
                numpackets; // number of packets caught
            if (isHeaderEmpty) {
                new_header->detHeader.row = row;
                new_header->detHeader.column = column;
            }
            return imageSize;
        }

        // copy packet
        switch (myDetectorType) {
        // for gotthard, 1st packet: 4 bytes fnum, CACA
        // + CACA, 639*2 bytes data 				2nd packet: 4
        // bytes fnum, previous 1*2 bytes data  + 640*2 bytes data !!
        case GOTTHARD:
            if (!pnum)
                memcpy(buf + fifohsize + (pnum * dsize),
                       &listeningPacket[hsize + 4], dsize - 2);
            else
                memcpy(buf + fifohsize + (pnum * dsize) - 2,
                       &listeningPacket[hsize], dsize + 2);
            break;
        case CHIPTESTBOARD:
        case MOENCH:
            if (pnum == (pperFrame - 1))
                memcpy(buf + fifohsize + (pnum * dsize),
                       &listeningPacket[hsize], corrected_dsize);
            else
                memcpy(buf + fifohsize + (pnum * dsize),
                       &listeningPacket[hsize], dsize);
            break;
        default:
            memcpy(buf + fifohsize + (pnum * dsize), &listeningPacket[hsize],
                   dsize);
            break;
        }
        ++numpackets; // number of packets in this image (each time its copied
                      // to buf)
        new_header->packetsMask[(
            (pnum < MAX_NUM_PACKETS) ? pnum : MAX_NUM_PACKETS - 1)] = 1;

        if (isHeaderEmpty) {
            // -------------------------- new header
            // ----------------------------------------------------------------------
            if (standardheader) {
                memcpy((char *)new_header, (char *)old_header,
                       sizeof(sls_detector_header));
            }
            // -------------------old header
            // ------------------------------------------------------------------------------
            else {
                new_header->detHeader.frameNumber = fnum;
                new_header->detHeader.bunchId = bnum;
                new_header->detHeader.row = row;
                new_header->detHeader.column = column;
                new_header->detHeader.detType =
                    (uint8_t)generalData->myDetectorType;
                new_header->detHeader.version =
                    (uint8_t)SLS_DETECTOR_HEADER_VERSION;
            }
            //------------------------------------------------------------------------------------------------------------
            isHeaderEmpty = false;
        }
    }

    // complete image
    new_header->detHeader.packetNumber = numpackets; // number of packets caught
    return imageSize;
}

void Listener::PrintFifoStatistics() {
    LOG(logDEBUG1) << "numFramesStatistic:" << numFramesStatistic
                   << " numPacketsStatistic:" << numPacketsStatistic
                   << " packetsperframe:" << generalData->packetsPerFrame;

    // calculate packet loss
    int64_t totalP = numFramesStatistic * (generalData->packetsPerFrame);
    int64_t loss = totalP - numPacketsStatistic;
    int lossPercent = ((double)loss / (double)totalP) * 100.00;
    numPacketsStatistic = 0;
    numFramesStatistic = 0;

    const auto color = loss ? logINFORED : logINFOGREEN;
    LOG(color) << "[" << *udpPortNumber
               << "]:  "
                  "Packet_Loss:"
               << loss << " (" << lossPercent << "%)"
               << "  Used_Fifo_Max_Level:" << fifo->GetMaxLevelForFifoBound()
               << " \tFree_Slots_Min_Level:" << fifo->GetMinLevelForFifoFree()
               << " \tCurrent_Frame#:" << currentFrameIndex;
}
