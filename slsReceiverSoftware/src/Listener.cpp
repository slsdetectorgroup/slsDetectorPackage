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
#include <unistd.h>

namespace sls {

const std::string Listener::TypeName = "Listener";

Listener::Listener(int index, std::atomic<runStatus> *status)
    : ThreadObject(index, TypeName), status(status) {
    LOG(logDEBUG) << "Listener " << index << " created";
}

Listener::~Listener() = default;

bool Listener::isPortDisabled() const { return disabledPort; }

uint64_t Listener::GetPacketsCaught() const { return numPacketsCaught; }

uint64_t Listener::GetNumCompleteFramesCaught() const {
    return numCompleteFramesCaught;
}

uint64_t Listener::GetLastFrameIndexCaught() const {
    return lastCaughtFrameIndex;
}

int64_t Listener::GetNumMissingPacket(bool stoppedFlag,
                                      uint64_t numPackets) const {
    if (disabledPort) {
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

void Listener::SetGeneralData(GeneralData *g) { generalData = g; }

void Listener::SetUdpPortNumber(const uint16_t portNumber) {
    udpPortNumber = portNumber;
}

void Listener::SetEthernetInterface(const std::string e) {
    eth = e;
    // if eth is mistaken with ip address
    if (eth.find('.') != std::string::npos) {
        eth = "";
    }
}

void Listener::SetActivate(bool enable) {
    activated = enable;
    disabledPort = (!activated || !detectorDataStream || noRoi);
}

void Listener::SetDetectorDatastream(bool enable) {
    detectorDataStream = enable;
    disabledPort = (!activated || !detectorDataStream || noRoi);
}

void Listener::SetNoRoi(bool enable) {
    noRoi = enable;
    disabledPort = (!activated || !detectorDataStream || noRoi);
}

void Listener::SetSilentMode(bool enable) { silentMode = enable; }

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
    if (generalData->detType == GOTTHARD2 && index != 0) {
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

    if (!silentMode) {
        if (!index) {
            LOG(logINFOBLUE) << index << " First Index: " << firstIndex;
        }
    }
}

void Listener::CreateUDPSocket(int &actualSize) {
    try {
        if (disabledPort) {
            return;
        }
        uint32_t packetSize = generalData->packetSize;
        if (generalData->detType == GOTTHARD2 && index != 0) {
            packetSize = generalData->vetoPacketSize;
        }

        std::string ip;
        if (eth.length() > 0)
            ip = InterfaceNameToIp(eth).str();
        udpSocket = nullptr;
        udpSocket = make_unique<UdpRxSocket>(
            udpPortNumber, packetSize, (ip.empty() ? nullptr : ip.c_str()),
            generalData->udpSocketBufferSize);
        LOG(logINFO) << index << ": UDP port opened at port " << udpPortNumber
                     << " (" << (ip.length() ? ip : "any") << ')';

        udpSocketAlive = true;

        // doubled due to kernel bookkeeping (could also be less due to
        // permissions)
        actualSize = udpSocket->getBufferSize();

    } catch (std::exception &e) {
        std::ostringstream oss;
        oss << "Could not create UDP socket on port " << udpPortNumber << " ["
            << e.what() << ']';
        throw RuntimeError(oss.str());
    }
}

void Listener::ShutDownUDPSocket() {
    if (udpSocket) {
        udpSocketAlive = false;
        // give other thread time after udpSocketAlive is changed
        usleep(0);
        udpSocket->Shutdown();
        LOG(logINFO) << "Shut down of UDP port " << udpPortNumber;
    }
}

void Listener::DeleteUDPSocket() {
    if (udpSocket) {
        udpSocket.reset();
        LOG(logINFO) << "Closed UDP port " << udpPortNumber;
    }
}

void Listener::CreateDummySocketForUDPSocketBufferSize(int s, int &actualSize) {
    // custom setup (s != 0)
    // default setup at startup (s = 0)
    int size = (s == 0 ? generalData->udpSocketBufferSize : s);
    LOG(logINFO) << "Testing UDP Socket Buffer size " << size
                 << " with test port " << udpPortNumber;
    int previousSize = generalData->udpSocketBufferSize;
    generalData->udpSocketBufferSize = size;

    if (disabledPort) {
        actualSize = (generalData->udpSocketBufferSize * 2);
        return;
    }

    uint32_t packetSize = generalData->packetSize;
    if (generalData->detType == GOTTHARD2 && index != 0) {
        packetSize = generalData->vetoPacketSize;
    }

    // create dummy socket
    try {
        // to allowe ports to be bound from udpsocket
        udpSocket.reset();

        std::string ip;
        if (eth.length() > 0)
            ip = InterfaceNameToIp(eth).str();
        UdpRxSocket g(udpPortNumber, packetSize,
                      (ip.empty() ? nullptr : ip.c_str()),
                      generalData->udpSocketBufferSize);

        // doubled due to kernel bookkeeping (could also be less due to
        // permissions)
        actualSize = g.getBufferSize();
        if (actualSize == -1) {
            generalData->udpSocketBufferSize = previousSize;
        } else {
            generalData->udpSocketBufferSize = actualSize / 2;
        }
        // to allow udp sockets to be able to bind in the future
    } catch (...) {
        throw RuntimeError("Could not create a test UDP socket on port " +
                           std::to_string(udpPortNumber));
    }

    // custom and didnt set, throw error
    if (s != 0 && static_cast<int>(generalData->udpSocketBufferSize) != s) {
        throw RuntimeError("Could not set udp socket buffer size. (No "
                           "CAP_NET_ADMIN privileges?)");
    }
}

void Listener::SetHardCodedPosition(uint16_t r, uint16_t c) {
    row = r;
    column = c;
    LOG(logDEBUG1) << "Setting hardcoded position [" << index
                   << "] (row: " << row << ", col: " << column << ")";
}

std::pair<uint16_t, uint16_t> Listener::GetHardCodedPosition() {
    return std::make_pair(row, column);
}

void Listener::ThreadExecution() {
    char *buffer;
    fifo->GetNewAddress(buffer);
    LOG(logDEBUG5) << "Listener " << index << ", pop 0x" << std::hex
                   << (void *)(buffer) << std::dec << ":" << buffer;
    auto *memImage = reinterpret_cast<image_structure *>(buffer);

    // udpsocket doesnt exist
    if ((*status == TRANSMITTING || !udpSocketAlive) && !carryOverFlag) {
        StopListening(buffer, memImage->size);
        return;
    }

    // reset header and size and get data
    memset(memImage, 0, IMAGE_STRUCTURE_HEADER_SIZE);
    int rc = ListenToAnImage(memImage->header, memImage->data);

    // end of acquisition or discarding image
    if (rc <= 0) {
        fifo->FreeAddress(buffer);
        return;
    }

    // valid image, set size and push into fifo
    memImage->size = rc;
    fifo->PushAddress(buffer);

    // Statistics
    if (!silentMode) {
        numFramesStatistic++;
        if (numFramesStatistic >=
            // second condition also for infinite #number of frames
            (generalData->framesPerFile == 0 ? STATISTIC_FRAMENUMBER_INFINITE
                                             : generalData->framesPerFile))
            PrintFifoStatistics();
    }
}

void Listener::StopListening(char *buf, size_t &size) {
    size = DUMMY_PACKET_VALUE;
    fifo->PushAddress(buf);
    StopRunning();
    LOG(logDEBUG1) << index << ": Listening Completed. Packets ("
                   << udpPortNumber << ") : " << numPacketsCaught;
}

/* buf includes the fifo header and packet header */
uint32_t Listener::ListenToAnImage(sls_receiver_header &dstHeader,
                                   char *dstData) {

    uint64_t fnum = 0;
    uint32_t pnum = 0;
    uint64_t bnum = 0;
    uint32_t numpackets = 0;

    uint32_t dsize = generalData->dataSize;
    uint32_t imageSize = generalData->imageSize;
    uint32_t packetSize = generalData->packetSize;
    uint32_t hsize = generalData->headerSizeinPacket;
    bool standardHeader = generalData->standardheader;
    if (generalData->detType == GOTTHARD2 && index != 0) {
        dsize = generalData->vetoDataSize;
        imageSize = generalData->vetoImageSize;
        packetSize = generalData->vetoPacketSize;
        hsize = generalData->vetoHsize;
        standardHeader = false;
    }
    uint32_t pperFrame = generalData->packetsPerFrame;
    bool isHeaderEmpty = true;
    uint32_t corrected_dsize = dsize - ((pperFrame * dsize) - imageSize);
    sls_detector_header *srcDetHeader = nullptr;

    // carry over packet
    if (carryOverFlag) {
        LOG(logDEBUG3) << index << "carry flag";
        GetPacketIndices(fnum, pnum, bnum, standardHeader,
                         carryOverPacket.get(), srcDetHeader);

        // future packet
        if (fnum != currentFrameIndex) {
            if (fnum < currentFrameIndex) {
                LOG(logERROR)
                    << "(Weird), With carry flag: Frame number " << fnum
                    << " less than current frame number " << currentFrameIndex;
                carryOverFlag = false;
                return 0;
            }
            return HandleFuturePacket(false, numpackets, fnum, isHeaderEmpty,
                                      imageSize, dstHeader);
        }

        CopyPacket(dstData, carryOverPacket.get(), dsize, hsize,
                   corrected_dsize, numpackets, isHeaderEmpty, standardHeader,
                   dstHeader, srcDetHeader, pnum, bnum);
        carryOverFlag = false;
    }

    // until last packet isHeaderEmpty to account for gotthard short frame, else
    // never entering this loop)
    while (numpackets < pperFrame) {
        // listen to new packet
        if (!udpSocketAlive || !udpSocket->ReceivePacket(&listeningPacket[0])) {
            // end of acquisition
            if (numpackets == 0)
                return 0;
            return HandleFuturePacket(true, numpackets, fnum, isHeaderEmpty,
                                      imageSize, dstHeader);
        }
        numPacketsCaught++;
        numPacketsStatistic++;
        GetPacketIndices(fnum, pnum, bnum, standardHeader,
                         listeningPacket.get(), srcDetHeader);

        // Eiger Firmware in a weird state
        if (generalData->detType == EIGER && fnum == 0) {
            LOG(logERROR) << "[" << udpPortNumber
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

        // bad packet
        if (pnum >= pperFrame) {
            LOG(logERROR) << "Bad packet " << pnum << "(fnum: " << fnum
                          << "), throwing away. Packets caught so far: "
                          << numpackets;
            return 0;
        }

        // future packet
        if (fnum != currentFrameIndex) {
            carryOverFlag = true;
            memcpy(carryOverPacket.get(), &listeningPacket[0], packetSize);
            return HandleFuturePacket(false, numpackets, fnum, isHeaderEmpty,
                                      imageSize, dstHeader);
        }
        CopyPacket(dstData, listeningPacket.get(), dsize, hsize,
                   corrected_dsize, numpackets, isHeaderEmpty, standardHeader,
                   dstHeader, srcDetHeader, pnum, bnum);
    }

    // complete image
    dstHeader.detHeader.packetNumber = numpackets;
    if (numpackets == pperFrame) {
        ++numCompleteFramesCaught;
    }
    ++currentFrameIndex;
    return imageSize;
}

size_t Listener::HandleFuturePacket(bool EOA, uint32_t numpackets,
                                    uint64_t fnum, bool isHeaderEmpty,
                                    size_t imageSize,
                                    sls_receiver_header &dstHeader) {
    switch (generalData->frameDiscardMode) {
    case DISCARD_EMPTY_FRAMES:
        if (!numpackets) {
            if (!EOA) {
                LOG(logDEBUG) << index << " Skipped fnum:" << currentFrameIndex;
                currentFrameIndex = fnum;
            }
            return -1;
        }
        break;
    case DISCARD_PARTIAL_FRAMES:
        LOG(logDEBUG) << index << " discarding fnum:" << currentFrameIndex;
        if (!EOA) {
            currentFrameIndex = fnum;
        }
        return -1;
    default:
        break;
    }
    dstHeader.detHeader.packetNumber = numpackets;
    // for empty frames (padded)
    if (isHeaderEmpty) {
        dstHeader.detHeader.frameNumber = currentFrameIndex;
        // no packet to get bnum
        dstHeader.detHeader.row = row;
        dstHeader.detHeader.column = column;
        dstHeader.detHeader.detType =
            static_cast<uint8_t>(generalData->detType);
        dstHeader.detHeader.version =
            static_cast<uint8_t>(SLS_DETECTOR_HEADER_VERSION);
    }
    if (!EOA) {
        ++currentFrameIndex;
    }
    return imageSize;
}

void Listener::CopyPacket(char *dst, char *src, uint32_t dataSize,
                          uint32_t detHeaderSize, uint32_t correctedDataSize,
                          uint32_t &numpackets, bool &isHeaderEmpty,
                          bool standardHeader, sls_receiver_header &dstHeader,
                          sls_detector_header *srcDetHeader, uint32_t pnum,
                          uint64_t bnum) {

    // copy packet data
    switch (generalData->detType) {
    // for gotthard,
    // 1st packet: 4 bytes fnum, CACA + CACA, 639*2 bytes data
    // 2nd packet: 4 bytes fnum, previous 1*2 bytes data  + 640*2 bytes data
    case GOTTHARD:
        if (!pnum)
            memcpy(dst, &src[detHeaderSize + 2], dataSize - 2);
        else
            memcpy(dst + dataSize - 2, &src[detHeaderSize - 2], dataSize + 2);
        break;
    case CHIPTESTBOARD:
    case XILINX_CHIPTESTBOARD:
        if (pnum == (generalData->packetsPerFrame - 1))
            memcpy(dst + (pnum * dataSize), &src[detHeaderSize],
                   correctedDataSize);
        else
            memcpy(dst + (pnum * dataSize), &src[detHeaderSize], dataSize);
        break;
    default:
        memcpy(dst + (pnum * dataSize), &src[detHeaderSize], dataSize);
        break;
    }

    ++numpackets;
    dstHeader
        .packetsMask[((pnum < MAX_NUM_PACKETS) ? pnum : MAX_NUM_PACKETS - 1)] =
        1;

    // writer header
    if (isHeaderEmpty) {
        if (standardHeader) {
            memcpy((char *)&dstHeader, (char *)srcDetHeader,
                   sizeof(sls_detector_header));
        } else {
            dstHeader.detHeader.frameNumber = currentFrameIndex;
            dstHeader.detHeader.detSpec1 = bnum;
            dstHeader.detHeader.row = row;
            dstHeader.detHeader.column = column;
            dstHeader.detHeader.detType =
                static_cast<uint8_t>(generalData->detType);
            dstHeader.detHeader.version =
                static_cast<uint8_t>(SLS_DETECTOR_HEADER_VERSION);
        }
        isHeaderEmpty = false;
    }
}

void Listener::GetPacketIndices(uint64_t &fnum, uint32_t &pnum, uint64_t &bnum,
                                bool standardHeader, char *packet,
                                sls_detector_header *&header) {
    if (standardHeader) {
        header = (sls_detector_header *)(&packet[0]);
        fnum = header->frameNumber;
        pnum = header->packetNumber;
    } else {
        // set first packet to be odd or even (check required when switching
        // from roi to no roi)
        if (generalData->detType == GOTTHARD && !startedFlag) {
            oddStartingPacket =
                generalData->SetOddStartingPacket(index, &packet[0]);
        }
        generalData->GetHeaderInfo(index, &packet[0], oddStartingPacket, fnum,
                                   pnum, bnum);
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
    LOG(color) << "[" << udpPortNumber
               << "]:  "
                  "Packet_Loss:"
               << loss << " (" << lossPercent << "%)"
               << "  Used_Fifo_Max_Level:" << fifo->GetMaxLevelForFifoBound()
               << " \tFree_Slots_Min_Level:" << fifo->GetMinLevelForFifoFree()
               << " \tCurrent_Frame#:" << currentFrameIndex;
}

} // namespace sls
