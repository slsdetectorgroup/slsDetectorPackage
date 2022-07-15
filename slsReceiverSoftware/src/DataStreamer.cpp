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

namespace sls {

const std::string DataStreamer::TypeName = "DataStreamer";

DataStreamer::DataStreamer(int ind, Fifo *f, uint32_t *dr, ROI *r, uint64_t *fi,
                           bool fr, slsDetectorDefs::xy np, bool *qe,
                           uint64_t *tot)
    : ThreadObject(ind, TypeName), fifo(f), dynamicRange(dr), detectorRoi(r),
      fileIndex(fi), flipRows(fr), numPorts(np), quadEnable(qe),
      totalNumFrames(tot) {

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

void DataStreamer::RecordFirstIndex(uint64_t fnum, size_t firstStreamerIndex) {
    startedFlag = true;
    // streamer first index needn't be the very first index
    firstIndex = fnum - firstStreamerIndex;
    LOG(logDEBUG1) << index << " First Index: " << firstIndex
                   << ", First Streamer Index:" << fnum;
}

void DataStreamer::SetGeneralData(GeneralData *g) { generalData = g; }

void DataStreamer::SetNumberofPorts(xy np) { numPorts = np; }

void DataStreamer::SetFlipRows(bool fd) { flipRows = fd; }

void DataStreamer::SetAdditionalJsonHeader(
    const std::map<std::string, std::string> &json) {
    std::lock_guard<std::mutex> lock(additionalJsonMutex);
    additionalJsonHeader = json;
    isAdditionalJsonUpdated = true;
}

void DataStreamer::CreateZmqSockets(int *nunits, uint32_t port,
                                    const IpAddr ip, int hwm) {
    uint32_t portnum = port + index;
    std::string sip = ip.str();
    try {
        zmqSocket = new ZmqSocket(portnum, (ip != 0 ? sip.c_str() : nullptr));
        // set if custom
        if (hwm >= 0) {
            zmqSocket->SetSendHighWaterMark(hwm);
            if (zmqSocket->GetSendHighWaterMark() != hwm) {
                throw RuntimeError(
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
    LOG(logDEBUG5) << "DataStreamer " << index << ", pop 0x"
                   << std::hex << (void *)(buffer) << std::dec << ":" << buffer;
    auto *memImage = reinterpret_cast<image_structure *>(buffer);

    // check dummy
    LOG(logDEBUG1) << "DataStreamer " << index << ", Numbytes:" << memImage->size ;
    if (memImage->size == DUMMY_PACKET_VALUE) {
        StopProcessing(buffer);
        return;
    }

    ProcessAnImage(memImage->header.detHeader, memImage->size, memImage->firstStreamerIndex, memImage->data);

    // free
    fifo->FreeAddress(buffer);
}

void DataStreamer::StopProcessing(char *buf) {
    LOG(logDEBUG1) << "DataStreamer " << index << ": Dummy";
    // send dummy header and data
    if (!SendDummyHeader()) {
        LOG(logERROR) << "Could not send zmq dummy header for streamer for port " 
                    << zmqSocket->GetPortNumber();
    }

    fifo->FreeAddress(buf);
    StopRunning();
    LOG(logDEBUG1) << index << ": Streaming Completed";
}

/** buf includes only the standard header */
void DataStreamer::ProcessAnImage(sls_detector_header header, size_t size, size_t firstStreamerIndex, char* data) {
    
    uint64_t fnum = header.frameNumber;
    LOG(logDEBUG1) << "DataStreamer " << index << ": fnum:" << fnum;
    if (!startedFlag) {
        RecordFirstIndex(fnum, firstStreamerIndex);
    }
    // shortframe gotthard
    if (completeBuffer) {
        // disregarding the size modified from callback (always using
        // imageSizeComplete instead of size because gui needs 
        // imagesizecomplete and listener writes imagesize to size

        if (!SendDataHeader(header, generalData->imageSizeComplete,
                        generalData->nPixelsXComplete, generalData->nPixelsYComplete)) {
            LOG(logERROR) << "Could not send zmq header for fnum " << fnum
                          << " and streamer " << index;
        }
        memcpy(completeBuffer + ((generalData->imageSize) * adcConfigured),
               data, size);

        if (!zmqSocket->SendData(completeBuffer, generalData->imageSizeComplete)) {
            LOG(logERROR) << "Could not send zmq data for fnum " << fnum
                          << " and streamer " << index;
        }
    }

    // normal
    else {

        if (!SendDataHeader(header, size, generalData->nPixelsX, generalData->nPixelsY)) {
            LOG(logERROR) << "Could not send zmq header for fnum " << fnum
                          << " and streamer " << index;
        }
        if (!zmqSocket->SendData(data, size)) {
            LOG(logERROR) << "Could not send zmq data for fnum " << fnum
                          << " and streamer " << index;
        }
    }
}

int DataStreamer::SendDummyHeader() {
    zmqHeader zHeader;
    zHeader.data = false;
    zHeader.jsonversion = SLS_DETECTOR_JSON_HEADER_VERSION;
    return zmqSocket->SendHeader(index, zHeader);
}

int DataStreamer::SendDataHeader(sls_detector_header header, uint32_t size,
                             uint32_t nx, uint32_t ny) {
    zmqHeader zHeader;
    zHeader.data = true;
    zHeader.jsonversion = SLS_DETECTOR_JSON_HEADER_VERSION;

    uint64_t frameIndex = header.frameNumber - firstIndex;
    uint64_t acquisitionIndex = header.frameNumber;

    zHeader.dynamicRange = *dynamicRange;
    zHeader.fileIndex = *fileIndex;
    zHeader.ndetx = numPorts.x;
    zHeader.ndety = numPorts.y;
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
    if (!SendDummyHeader()) {
        throw RuntimeError(
            "Could not restream Dummy Header via ZMQ for port " +
            std::to_string(zmqSocket->GetPortNumber()));
    }
}

} // namespace sls
