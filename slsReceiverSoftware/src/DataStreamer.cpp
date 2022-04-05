// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
/************************************************
 * @file DataStreamer.cpp
 * @short streams data from receiver via ZMQ
 ***********************************************/

#include "DataStreamer.h"
#include "Fifo.h"
#include "GeneralData.h"
#include "sls/ZmqSocket.h"
#include "sls/sls_detector_exceptions.h"

#include <cerrno>
#include <iostream>

const std::string DataStreamer::TypeName = "DataStreamer";

DataStreamer::DataStreamer(int ind, Fifo *f, uint32_t *dr, ROI *r, uint64_t *fi,
                           bool fr, slsDetectorDefs::xy nm, bool *qe,
                           uint64_t *tot)
    : ThreadObject(ind, TypeName), fifo(f), dynamicRange(dr), detectorRoi(r),
      fileIndex(fi), flipRows(fr), quadEnable(qe), totalNumFrames(tot) {
    numMods.x = nm.x;
    numMods.y = nm.y;

    LOG(logDEBUG) << "DataStreamer " << ind << " created";
}

DataStreamer::~DataStreamer() {
    CloseZmqSocket();
    delete[] completeBuffer;
}

void DataStreamer::SetFifo(Fifo *f) { fifo = f; }

void DataStreamer::ResetParametersforNewAcquisition(const std::string &fname) {
    StopRunning();
    startedFlag = false;
    firstIndex = 0;

    fileNametoStream = fname;
    if (completeBuffer) {
        delete[] completeBuffer;
        completeBuffer = nullptr;
    }
    if (generalData->myDetectorType == GOTTHARD && detectorRoi->xmin != -1) {
        adcConfigured = generalData->GetAdcConfigured(index, *detectorRoi);
        completeBuffer = new char[generalData->imageSizeComplete];
        memset(completeBuffer, 0, generalData->imageSizeComplete);
    }
}

void DataStreamer::RecordFirstIndex(uint64_t fnum, char *buf) {
    startedFlag = true;
    // streamer first index needn't be
    uint64_t firstVal = fnum - (*((uint32_t *)(buf + FIFO_DATASIZE_NUMBYTES)));

    firstIndex = firstVal;
    LOG(logDEBUG1) << index << " First Index: " << firstIndex
                   << ", First Streamer Index:" << fnum;
}

void DataStreamer::SetGeneralData(GeneralData *g) { generalData = g; }

void DataStreamer::SetNumberofModules(xy nm) {
    numMods.x = nm.x;
    numMods.y = nm.y;
}

void DataStreamer::SetFlipRows(bool fd) { flipRows = fd; }

void DataStreamer::SetAdditionalJsonHeader(
    const std::map<std::string, std::string> &json) {
    std::lock_guard<std::mutex> lock(additionalJsonMutex);
    additionalJsonHeader = json;
    isAdditionalJsonUpdated = true;
}

void DataStreamer::CreateZmqSockets(int *nunits, uint32_t port,
                                    const sls::IpAddr ip, int hwm) {
    uint32_t portnum = port + index;
    std::string sip = ip.str();
    try {
        zmqSocket = new ZmqSocket(portnum, (ip != 0 ? sip.c_str() : nullptr));
        // set if custom
        if (hwm >= 0) {
            zmqSocket->SetSendHighWaterMark(hwm);
            if (zmqSocket->GetSendHighWaterMark() != hwm) {
                throw sls::RuntimeError(
                    "Could not set zmq send high water mark to " +
                    std::to_string(hwm));
            }
        }
    } catch (...) {
        LOG(logERROR) << "Could not create Zmq socket on port " << portnum
                      << " for Streamer " << index;
        throw;
    }
    LOG(logINFO) << index << " Streamer: Zmq Server started at "
                 << zmqSocket->GetZmqServerAddress()
                 << "[hwm: " << zmqSocket->GetSendHighWaterMark() << "]";
}

void DataStreamer::CloseZmqSocket() {
    if (zmqSocket) {
        delete zmqSocket;
        zmqSocket = nullptr;
    }
}

void DataStreamer::ThreadExecution() {
    char *buffer = nullptr;
    fifo->PopAddressToStream(buffer);
    LOG(logDEBUG5) << "DataStreamer " << index
                   << ", "
                      "pop 0x"
                   << std::hex << (void *)(buffer) << std::dec << ":" << buffer;

    // check dummy
    uint32_t numBytes = (uint32_t)(*((uint32_t *)buffer));
    LOG(logDEBUG1) << "DataStreamer " << index << ", Numbytes:" << numBytes;
    if (numBytes == DUMMY_PACKET_VALUE) {
        StopProcessing(buffer);
        return;
    }

    ProcessAnImage(buffer);

    // free
    fifo->FreeAddress(buffer);
}

void DataStreamer::StopProcessing(char *buf) {
    LOG(logDEBUG1) << "DataStreamer " << index << ": Dummy";

    sls_receiver_header *header = (sls_receiver_header *)(buf);
    // send dummy header and data
    if (!SendHeader(header, 0, 0, 0, true)) {
        LOG(logERROR) << "Could not send zmq dummy header for streamer "
                      << index;
    }

    fifo->FreeAddress(buf);
    StopRunning();
    LOG(logDEBUG1) << index << ": Streaming Completed";
}

/** buf includes only the standard header */
void DataStreamer::ProcessAnImage(char *buf) {

    sls_receiver_header *header =
        (sls_receiver_header *)(buf + FIFO_HEADER_NUMBYTES);
    uint64_t fnum = header->detHeader.frameNumber;
    LOG(logDEBUG1) << "DataStreamer " << index << ": fnum:" << fnum;

    if (!startedFlag) {
        RecordFirstIndex(fnum, buf);
    }

    // shortframe gotthard
    if (completeBuffer) {
        // disregarding the size modified from callback (always using
        // imageSizeComplete
        // instead of buf (32 bit) because gui needs imagesizecomplete and
        // listener
        // write imagesize

        if (!SendHeader(header, generalData->imageSizeComplete,
                        generalData->nPixelsXComplete,
                        generalData->nPixelsYComplete, false)) {
            LOG(logERROR) << "Could not send zmq header for fnum " << fnum
                          << " and streamer " << index;
        }
        memcpy(completeBuffer + ((generalData->imageSize) * adcConfigured),
               buf + FIFO_HEADER_NUMBYTES + sizeof(sls_receiver_header),
               (uint32_t)(*((uint32_t *)buf)));

        if (!zmqSocket->SendData(completeBuffer,
                                 generalData->imageSizeComplete)) {
            LOG(logERROR) << "Could not send zmq data for fnum " << fnum
                          << " and streamer " << index;
        }
    }

    // normal
    else {

        if (!SendHeader(header, (uint32_t)(*((uint32_t *)buf)),
                        generalData->nPixelsX, generalData->nPixelsY,
                        false)) { // new size possibly from callback
            LOG(logERROR) << "Could not send zmq header for fnum " << fnum
                          << " and streamer " << index;
        }
        if (!zmqSocket->SendData(
                buf + FIFO_HEADER_NUMBYTES + sizeof(sls_receiver_header),
                (uint32_t)(*(
                    (uint32_t *)buf)))) { // new size possibly from callback
            LOG(logERROR) << "Could not send zmq data for fnum " << fnum
                          << " and streamer " << index;
        }
    }
}

int DataStreamer::SendHeader(sls_receiver_header *rheader, uint32_t size,
                             uint32_t nx, uint32_t ny, bool dummy) {

    zmqHeader zHeader;
    zHeader.data = !dummy;
    zHeader.jsonversion = SLS_DETECTOR_JSON_HEADER_VERSION;

    if (dummy) {
        return zmqSocket->SendHeader(index, zHeader);
    }

    sls_detector_header header = rheader->detHeader;

    uint64_t frameIndex = header.frameNumber - firstIndex;
    uint64_t acquisitionIndex = header.frameNumber;

    zHeader.dynamicRange = *dynamicRange;
    zHeader.fileIndex = *fileIndex;
    zHeader.ndetx = numMods.x;
    zHeader.ndety = numMods.y;
    zHeader.npixelsx = nx;
    zHeader.npixelsy = ny;
    zHeader.imageSize = size;
    zHeader.acqIndex = acquisitionIndex;
    zHeader.frameIndex = frameIndex;
    zHeader.progress =
        100 * ((double)(frameIndex + 1) / (double)(*totalNumFrames));
    zHeader.fname = fileNametoStream;
    zHeader.frameNumber = header.frameNumber;
    zHeader.expLength = header.expLength;
    zHeader.packetNumber = header.packetNumber;
    zHeader.bunchId = header.bunchId;
    zHeader.timestamp = header.timestamp;
    zHeader.modId = header.modId;
    zHeader.row = header.row;
    zHeader.column = header.column;
    zHeader.reserved = header.reserved;
    zHeader.debug = header.debug;
    zHeader.roundRNumber = header.roundRNumber;
    zHeader.detType = header.detType;
    zHeader.version = header.version;
    zHeader.flipRows = static_cast<int>(flipRows);
    zHeader.quad = *quadEnable;
    zHeader.completeImage =
        (header.packetNumber < generalData->packetsPerFrame ? false : true);

    // update local copy only if it was updated (to prevent locking each time)
    if (isAdditionalJsonUpdated) {
        std::lock_guard<std::mutex> lock(additionalJsonMutex);
        localAdditionalJsonHeader = additionalJsonHeader;
        isAdditionalJsonUpdated = false;
    }
    zHeader.addJsonHeader = localAdditionalJsonHeader;

    return zmqSocket->SendHeader(index, zHeader);
}

void DataStreamer::RestreamStop() {
    // send dummy header
    zmqHeader zHeader;
    zHeader.data = false;
    zHeader.jsonversion = SLS_DETECTOR_JSON_HEADER_VERSION;
    int ret = zmqSocket->SendHeader(index, zHeader);
    if (!ret) {
        throw sls::RuntimeError(
            "Could not restream Dummy Header via ZMQ for port " +
            std::to_string(zmqSocket->GetPortNumber()));
    }
}
