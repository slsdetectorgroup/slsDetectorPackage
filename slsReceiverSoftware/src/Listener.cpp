// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
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
#include "sls/container_utils.h" // For make_unique<>
#include "sls/network_utils.h"
#include "sls/sls_detector_exceptions.h"

#include <cerrno>
#include <cstring>
#include <iostream>

namespace sls {

const std::string Listener::TypeName = "Listener";

Listener::Listener(int ind, detectorType dtype, Fifo *f,
                   std::atomic<runStatus> *s, uint32_t *portno, std::string *e,
                   int *us, int *as, uint32_t *fpf, frameDiscardPolicy *fdp,
                   bool *detds, bool *sm)
    : ThreadObject(ind, TypeName), fifo(f), myDetectorType(dtype), status(s),
      udpPortNumber(portno), eth(e), udpSocketBufferSize(us),
      actualUDPSocketBufferSize(as), framesPerFile(fpf), frameDiscardMode(fdp),
      detectorDataStream(detds), silentMode(sm) {
    LOG(logDEBUG) << "Listener " << ind << " created";
}

Listener::~Listener() = default;

uint64_t Listener::GetPacketsCaught() const { return numPacketsCaught; }

uint64_t Listener::GetNumCompleteFramesCaught() const {
    return numCompleteFramesCaught;
}

uint64_t Listener::GetLastFrameIndexCaught() const {
    return lastCaughtFrameIndex;
}

int64_t Listener::GetNumMissingPacket(bool stoppedFlag,
                                      uint64_t numPackets) const {
    if (!activated) {
        return 0;
    }
    if (!(*detectorDataStream)) {
        return 0;
    }
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

bool Listener::GetStartedFlag() const { return startedFlag; }

uint64_t Listener::GetCurrentFrameIndex() const { return lastCaughtFrameIndex; }

uint64_t Listener::GetListenedIndex() const {
    return lastCaughtFrameIndex - firstIndex;
}

void Listener::SetFifo(Fifo *f) { fifo = f; }

void Listener::ResetParametersforNewAcquisition() {
    StopRunning();
    startedFlag = false;
    numPacketsCaught = 0;
    numCompleteFramesCaught = 0;
    firstIndex = 0;
    currentFrameIndex = 0;
    lastCaughtFrameIndex = 0;
    carryOverFlag = false;
    uint32_t packetSize = generalData->packetSize;
    if (myDetectorType == GOTTHARD2 && index != 0) {
        packetSize = generalData->vetoPacketSize;
    }
    carryOverPacket = make_unique<char[]>(packetSize);
    memset(carryOverPacket.get(), 0, packetSize);
    listeningPacket = make_unique<char[]>(packetSize);
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
    lastCaughtFrameIndex = fnum;

    startedFlag = true;
    firstIndex = fnum;

    if (!(*silentMode)) {
        if (!index) {
            LOG(logINFOBLUE) << index << " First Index: " << firstIndex;
        }
    }
}

void Listener::SetGeneralData(GeneralData *g) { generalData = g; }

void Listener::SetActivate(bool enable) { activated = enable; }

void Listener::CreateUDPSockets() {
    if (!activated || !(*detectorDataStream)) {
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
        udpSocket = make_unique<UdpRxSocket>(
            *udpPortNumber, packetSize,
            ((*eth).length() ? InterfaceNameToIp(*eth).str().c_str()
                             : nullptr),
            *udpSocketBufferSize);
        LOG(logINFO) << index << ": UDP port opened at port " << *udpPortNumber;
    } catch (...) {
        throw RuntimeError("Could not create UDP socket on port " +
                                std::to_string(*udpPortNumber));
    }

    udpSocketAlive = true;

    // doubled due to kernel bookkeeping (could also be less due to permissions)
    *actualUDPSocketBufferSize = udpSocket->getBufferSize();
}

void Listener::ShutDownUDPSocket() {
    if (udpSocket) {
        udpSocketAlive = false;
        usleep(0);
        udpSocket->Shutdown();
        LOG(logINFO) << "Shut down of UDP port " << *udpPortNumber;
    }
}

void Listener::CreateDummySocketForUDPSocketBufferSize(int s) {
    LOG(logINFO) << "Testing UDP Socket Buffer size " << s << " with test port "
                 << *udpPortNumber;

    if (!activated || !(*detectorDataStream)) {
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
        UdpRxSocket g(*udpPortNumber, packetSize,
                           ((*eth).length()
                                ? InterfaceNameToIp(*eth).str().c_str()
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
        throw RuntimeError("Could not create a test UDP socket on port " +
                                std::to_string(*udpPortNumber));
    }
}

void Listener::SetHardCodedPosition(uint16_t r, uint16_t c) {
    row = r;
    column = c;
    LOG(logDEBUG1) << "Setting hardcoded position [" << index
                   << "] (row: " << row << ", col: " << column << ")";
}

void Listener::ThreadExecution() {
    char *buffer;
    fifo->GetNewAddress(buffer);
    LOG(logDEBUG5) << "Listener " << index
                   << ", "
                      "pop 0x"
                   << std::hex << (void *)(buffer) << std::dec << ":" << buffer;

    // udpsocket doesnt exist
    if ((*status == TRANSMITTING || !udpSocketAlive) && !carryOverFlag) {
        StopListening(buffer);
        return;
    }

    // get data
    int rc = ListenToAnImage(buffer);

    // end of acquisition or discarding image
    if (rc <= 0) {
       fifo->FreeAddress(buffer);
       return;
    }

    // valid image, set size and push into fifo
    (*((uint32_t *)buffer)) = rc;
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
    ////////////uint32_t fifohsize = generalData->fifoBufferHeaderSize;
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
    uint32_t corrected_dsize = dsize - ((pperFrame * dsize) - imageSize);

    auto *memImage = reinterpret_cast<fifo_image_structure *>(buf);
    auto rxHeader = &memImage->header;
    auto data = memImage->data;
    sls_detector_header *detHeader = nullptr;

    // reset to -1
    memImage->imageSize = 0;
    memset(&rxHeader, 0, sizeof(memImage->header));

    // carry over packet
    if (carryOverFlag) {
        LOG(logDEBUG3) << index << "carry flag";
        if (standardheader) {
            auto *carryStructure = reinterpret_cast<listening_packet_structure *>(carryOverPacket.get());
            detHeader = &carryStructure->header;
            fnum = detHeader->frameNumber;
            pnum = detHeader->packetNumber;
        } else {
            generalData->GetHeaderInfo(index, &carryOverPacket[0],
                                       oddStartingPacket, fnum, pnum, bnum);
        }
        // future packet
        if (fnum != currentFrameIndex) {
            if (fnum < currentFrameIndex) {
                LOG(logERROR)
                    << "(Weird), With carry flag: Frame number " << fnum
                    << " less than current frame number " << currentFrameIndex;
                carryOverFlag = false;
                return 0;
            }
            return HandleFuturePacket(false, numpackets, fnum, isHeaderEmpty, imageSize, rxHeader);
        }

        CopyPacket(data, carryOverPacket.get(), dsize, hsize, corrected_dsize, numpackets, isHeaderEmpty, rxHeader, detHeader, pnum, bnum);
        carryOverFlag = false;
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
            // empty image
            if (numpackets == 0)
                return 0; 
            return HandleFuturePacket(true, numpackets, fnum, isHeaderEmpty, imageSize, rxHeader);
        }

        // record immediately to get more time before socket shutdown
        numPacketsCaught++; 
        numPacketsStatistic++;

        if (standardheader) {
            detHeader = (sls_detector_header *)(&listeningPacket[0]);
            fnum = detHeader->frameNumber;
            pnum = detHeader->packetNumber;
        } else {
            // set first packet to be odd or even (check required when switching
            // from roi to no roi)
            if (myDetectorType == GOTTHARD && !startedFlag) {
                oddStartingPacket = generalData->SetOddStartingPacket(
                    index, &listeningPacket[0]);
            }
            generalData->GetHeaderInfo(index, &listeningPacket[0],
                                       oddStartingPacket, fnum, pnum, bnum);
        }

        // Eiger Firmware in a weird state
        if (myDetectorType == EIGER && fnum == 0) {
            LOG(logERROR) << "[" << *udpPortNumber
                          << "]: Got Frame Number "
                             "Zero from Firmware. Discarding Packet";
            numPacketsCaught--;
            numPacketsStatistic--;
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

        // future packet
        if (fnum != currentFrameIndex) {
            carryOverFlag = true;
            memcpy(carryOverPacket.get(), &listeningPacket[0], packetSize);
            return HandleFuturePacket(false, numpackets, fnum, isHeaderEmpty, imageSize, rxHeader);
        }
        CopyPacket(data, listeningPacket.get(), dsize, hsize, corrected_dsize, numpackets, isHeaderEmpty, rxHeader, detHeader, pnum, bnum);
    }

    // complete image
    rxHeader->detHeader.packetNumber = numpackets; // number of packets caught
    rxHeader->detHeader.frameNumber = currentFrameIndex;
    if (numpackets == pperFrame) {
        ++numCompleteFramesCaught;
    }
    ++currentFrameIndex;
    return imageSize;
}

size_t Listener::HandleFuturePacket(bool EOA, uint32_t numpackets, uint64_t frameNumber, bool isHeaderEmpty, size_t imageSize, sls_receiver_header* rxHeader) {
    switch (*frameDiscardMode) {
    case DISCARD_EMPTY_FRAMES:
        if (!numpackets) {
            if (!EOA) {
                LOG(logDEBUG)
                    << index << " Skipped fnum:" << currentFrameIndex;
                currentFrameIndex = frameNumber;
            }
            return -1;
        }
        break;
    case DISCARD_PARTIAL_FRAMES:
        LOG(logDEBUG)
            << index << " discarding fnum:" << currentFrameIndex;
        if (!EOA) {
            currentFrameIndex = frameNumber;
        }
        return -1;
    default:
        break;
    }
    // replacing with number of packets caught
    rxHeader->detHeader.packetNumber = numpackets; 
    if (isHeaderEmpty) {
        rxHeader->detHeader.row = row;
        rxHeader->detHeader.column = column;
    }
    rxHeader->detHeader.frameNumber = currentFrameIndex;
    if (!EOA) {
        ++currentFrameIndex;
    }
    return imageSize;
}

void Listener::CopyPacket(char* dst, char* src, uint32_t dataSize, uint32_t detHeaderSize, 
    uint32_t correctedDataSize, uint32_t &numpackets, bool &isHeaderEmpty, 
    sls_receiver_header* rxHeader, sls_detector_header* detHeader, uint32_t pnum, uint64_t bnum) {

    // copy packet data
    switch (myDetectorType) {
    // for gotthard, 1st packet: 4 bytes fnum, CACA
    // + CACA, 639*2 bytes data 				2nd packet: 4
    // bytes fnum, previous 1*2 bytes data  + 640*2 bytes data !!
    case GOTTHARD:
        if (!pnum)
            memcpy(dst, &src[detHeaderSize + 4], dataSize - 2);
        else
            memcpy(dst + dataSize - 2, &src[detHeaderSize], dataSize + 2);
        break;
    case CHIPTESTBOARD:
    case MOENCH:
        if (pnum == (generalData->packetsPerFrame - 1))
            memcpy(dst + (pnum * dataSize), &src[detHeaderSize], correctedDataSize);
        else
            memcpy(dst + (pnum * dataSize), &src[detHeaderSize], dataSize);
        break;
    default:
        memcpy(dst + (pnum * dataSize), &src[detHeaderSize], dataSize);
        break;
    }

    ++numpackets; 
    rxHeader->packetsMask[(
        (pnum < MAX_NUM_PACKETS) ? pnum : MAX_NUM_PACKETS - 1)] = 1;

    // writer header
    if (isHeaderEmpty) {
        if (standardheader) {
            memcpy((char *)rxHeader, (char *)detHeader, sizeof(sls_detector_header));
        } else {
            rxHeader->detHeader.frameNumber = currentFrameIndex;
            rxHeader->detHeader.bunchId = bnum;
            rxHeader->detHeader.row = row;
            rxHeader->detHeader.column = column;
            rxHeader->detHeader.detType =
                (uint8_t)generalData->myDetectorType;
            rxHeader->detHeader.version =
                (uint8_t)SLS_DETECTOR_HEADER_VERSION;
        }
        isHeaderEmpty = false;
    }
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

} // namespace sls
