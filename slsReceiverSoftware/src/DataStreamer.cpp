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

DataStreamer::DataStreamer(int index) : ThreadObject(index, TypeName) {
    LOG(logDEBUG) << "DataStreamer " << index << " created";
}

DataStreamer::~DataStreamer() {
    CloseZmqSocket();
    delete[] completeBuffer;
}

void DataStreamer::SetFifo(Fifo *f) { fifo = f; }

void DataStreamer::SetGeneralData(GeneralData *g) { generalData = g; }

void DataStreamer::SetFileIndex(uint64_t value) { fileIndex = value; }

void DataStreamer::SetNumberofPorts(xy np) { numPorts = np; }

void DataStreamer::SetFlipRows(bool fd) {
    flipRows = fd;
    // flip only right port of quad
    if (quadEnable) {
        flipRows = (index == 1 ? true : false);
    }
}

void DataStreamer::SetQuadEnable(bool value) { quadEnable = value; }

void DataStreamer::SetNumberofTotalFrames(uint64_t value) {
    nTotalFrames = value;
}

void DataStreamer::SetAdditionalJsonHeader(
    const std::map<std::string, std::string> &json) {
    std::lock_guard<std::mutex> lock(additionalJsonMutex);
    additionalJsonHeader = json;
    isAdditionalJsonUpdated = true;
}

void DataStreamer::SetReceiverROI(ROI roi) { receiverRoi = roi; }

void DataStreamer::ResetParametersforNewAcquisition(const std::string &fname) {
    StopRunning();
    startedFlag = false;
    firstIndex = 0;

    fileNametoStream = fname;
    if (completeBuffer) {
        delete[] completeBuffer;
        completeBuffer = nullptr;
    }
    if (generalData->detType == GOTTHARD &&
        generalData->detectorRoi.xmin != -1) {
        adcConfigured =
            generalData->GetAdcConfigured(index, generalData->detectorRoi);
        completeBuffer = new char[generalData->imageSizeComplete];
        memset(completeBuffer, 0, generalData->imageSizeComplete);
    }
}

void DataStreamer::RecordFirstIndex(uint64_t fnum, size_t firstImageIndex) {
    startedFlag = true;
    firstIndex = firstImageIndex;
    LOG(logDEBUG1) << index << " First Index: " << firstIndex
                   << ", First Streamer Index:" << fnum;
}

void DataStreamer::CreateZmqSockets(uint16_t port, int hwm) {
    uint16_t portnum = port + index;
    try {
        zmqSocket = new ZmqSocket(portnum);

        // set if custom
        if (hwm >= 0) {
            zmqSocket->SetSendHighWaterMark(hwm);
            // needed, or HWL is not taken
            zmqSocket->Rebind();
        }
    } catch (std::exception &e) {
        std::ostringstream oss;
        oss << "Could not create zmq pub socket on port " << portnum;
        oss << " [" << e.what() << ']';
        throw RuntimeError(oss.str());
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
    LOG(logDEBUG5) << "DataStreamer " << index << ", pop 0x" << std::hex
                   << (void *)(buffer) << std::dec << ":" << buffer;
    auto *memImage = reinterpret_cast<image_structure *>(buffer);

    // check dummy
    LOG(logDEBUG1) << "DataStreamer " << index
                   << ", Numbytes:" << memImage->size;
    if (memImage->size == DUMMY_PACKET_VALUE) {
        StopProcessing(buffer);
        return;
    }

    // streamer first index needn't be the very first index
    if (!startedFlag) {
        RecordFirstIndex(memImage->header.detHeader.frameNumber,
                         memImage->firstIndex);
    }

    ProcessAnImage(memImage->header.detHeader, memImage->size, memImage->data);

    // free
    fifo->FreeAddress(buffer);
}

void DataStreamer::StopProcessing(char *buf) {
    LOG(logDEBUG1) << "DataStreamer " << index << ": Dummy";
    if (!SendDummyHeader()) {
        LOG(logERROR)
            << "Could not send zmq dummy header for streamer for port "
            << zmqSocket->GetPortNumber();
    }

    fifo->FreeAddress(buf);
    StopRunning();
    LOG(logDEBUG1) << index << ": Streaming Completed";
}

/** buf includes only the standard header */
void DataStreamer::ProcessAnImage(sls_detector_header header, size_t size,
                                  char *data) {

    uint64_t fnum = header.frameNumber;
    LOG(logDEBUG1) << "DataStreamer " << index << ": fnum:" << fnum;

    // shortframe gotthard
    if (completeBuffer) {
        // disregarding the size modified from callback (always using
        // imageSizeComplete instead of size because gui needs
        // imagesizecomplete and listener writes imagesize to size

        if (!SendDataHeader(header, generalData->imageSizeComplete,
                            generalData->nPixelsXComplete,
                            generalData->nPixelsYComplete)) {
            LOG(logERROR) << "Could not send zmq header for fnum " << fnum
                          << " and streamer " << index;
        }
        memcpy(completeBuffer + ((generalData->imageSize) * adcConfigured),
               data, size);

        if (!zmqSocket->SendData(completeBuffer,
                                 generalData->imageSizeComplete)) {
            LOG(logERROR) << "Could not send zmq data for fnum " << fnum
                          << " and streamer " << index;
        }
    }

    // normal
    else {

        if (!SendDataHeader(header, size, generalData->nPixelsX,
                            generalData->nPixelsY)) {
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

    zHeader.dynamicRange = generalData->dynamicRange;
    zHeader.fileIndex = fileIndex;
    zHeader.ndetx = numPorts.x;
    zHeader.ndety = numPorts.y;
    zHeader.npixelsx = nx;
    zHeader.npixelsy = ny;
    zHeader.imageSize = size;
    zHeader.acqIndex = acquisitionIndex;
    zHeader.frameIndex = frameIndex;
    zHeader.progress =
        100 * ((double)(frameIndex + 1) / (double)(nTotalFrames));
    zHeader.fname = fileNametoStream;
    zHeader.frameNumber = header.frameNumber;
    zHeader.expLength = header.expLength;
    zHeader.packetNumber = header.packetNumber;
    zHeader.detSpec1 = header.detSpec1;
    zHeader.timestamp = header.timestamp;
    zHeader.modId = header.modId;
    zHeader.row = header.row;
    zHeader.column = header.column;
    zHeader.detSpec2 = header.detSpec2;
    zHeader.detSpec3 = header.detSpec3;
    zHeader.detSpec4 = header.detSpec4;
    zHeader.detType = header.detType;
    zHeader.version = header.version;
    zHeader.flipRows = static_cast<int>(flipRows);
    zHeader.quad = quadEnable;
    zHeader.completeImage =
        (header.packetNumber < generalData->packetsPerFrame ? false : true);

    // update local copy only if it was updated (to prevent locking each time)
    if (isAdditionalJsonUpdated) {
        std::lock_guard<std::mutex> lock(additionalJsonMutex);
        localAdditionalJsonHeader = additionalJsonHeader;
        isAdditionalJsonUpdated = false;
    }
    zHeader.addJsonHeader = localAdditionalJsonHeader;
    zHeader.rx_roi = receiverRoi.getIntArray();

    return zmqSocket->SendHeader(index, zHeader);
}

void DataStreamer::RestreamStop() {
    if (!SendDummyHeader()) {
        throw RuntimeError("Could not restream Dummy Header via ZMQ for port " +
                           std::to_string(zmqSocket->GetPortNumber()));
    }
}

} // namespace sls
