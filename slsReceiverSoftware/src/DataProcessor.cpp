// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
/************************************************
 * @file DataProcessor.cpp
 * @short creates data processor thread that
 * pulls pointers to memory addresses from  fifos
 * and processes data stored in them & writes them to file
 ***********************************************/

#include "DataProcessor.h"
#include "BinaryDataFile.h"
#include "Fifo.h"
#include "GeneralData.h"
#include "MasterAttributes.h"
#include "MasterFileUtility.h"
#ifdef HDF5C
#include "HDF5DataFile.h"
#endif
#include "DataStreamer.h"
#include "sls/container_utils.h"
#include "sls/sls_detector_exceptions.h"

#include <cerrno>
#include <cstring>
#include <iostream>

namespace sls {

// TODO: move somewhere else
template <typename T> struct AlignedData {
    T *ptr; // Aligned data pointer
    std::unique_ptr<std::aligned_storage_t<sizeof(T), alignof(T)>[]> buffer;

    AlignedData(
        T *p,
        std::unique_ptr<std::aligned_storage_t<sizeof(T), alignof(T)>[]> buf)
        : ptr(p), buffer(std::move(buf)) {}
};

// TODO: should not be in this file any suggestions to move it to a more
// appropriate file?
// TODO: Add unit test
/*
 * AlignData
 * Aligns data to a given type T with proper alignment
 * @param data: pointer to data
 * @param size: size of data to align in bytes
 * @return: aligned data
 */
template <typename T>
void AlignData(AlignedData<T> &aligneddata, char *data, size_t size) {
    using AlignedBuffer =
        typename std::aligned_storage<sizeof(T), alignof(T)>::type;
    std::unique_ptr<AlignedBuffer[]> tempbuffer;

    if (reinterpret_cast<uintptr_t>(data) % alignof(uint64_t) == 0) {
        // If aligned directly cast to pointer

        aligneddata.ptr = reinterpret_cast<T *>(data);

    } else {
        // Allocate a temporary buffer with proper alignment
        tempbuffer = std::make_unique<AlignedBuffer[]>(size / sizeof(T));
        // size = ctbDigitaldbt;

        std::memcpy(tempbuffer.get(), data, size);
        aligneddata.buffer = std::move(tempbuffer);
        aligneddata.ptr = reinterpret_cast<T *>(aligneddata.buffer.get());
    }
}

const std::string DataProcessor::typeName = "DataProcessor";

DataProcessor::DataProcessor(int index) : ThreadObject(index, typeName) {

    LOG(logDEBUG) << "DataProcessor " << index << " created";
}

DataProcessor::~DataProcessor() { DeleteFiles(); }

bool DataProcessor::GetStartedFlag() const { return startedFlag; }

void DataProcessor::SetFifo(Fifo *f) { fifo = f; }

void DataProcessor::SetGeneralData(GeneralData *g) { generalData = g; }

void DataProcessor::SetUdpPortNumber(const uint16_t portNumber) {
    udpPortNumber = portNumber;
}

void DataProcessor::SetActivate(bool enable) { activated = enable; }

void DataProcessor::SetReceiverROI(ROI roi) {
    receiverRoi = roi;
    receiverRoiEnabled = receiverRoi.completeRoi() ? false : true;
    receiverNoRoi = receiverRoi.noRoi();
}

void DataProcessor::SetDataStreamEnable(bool enable) {
    dataStreamEnable = enable;
}

void DataProcessor::SetStreamingFrequency(uint32_t value) {
    streamingFrequency = value;
}

void DataProcessor::SetStreamingTimerInMs(uint32_t value) {
    streamingTimerInMs = value;
}

void DataProcessor::SetStreamingStartFnum(uint32_t value) {
    streamingStartFnum = value;
}

void DataProcessor::SetFramePadding(bool enable) { framePadding = enable; }

void DataProcessor::SetCtbDbitList(std::vector<int> value) {
    ctbDbitList = value;
}

void DataProcessor::SetCtbDbitOffset(int value) { ctbDbitOffset = value; }

void DataProcessor::SetCtbDbitReorder(bool value) { ctbDbitReorder = value; }

void DataProcessor::SetQuadEnable(bool value) { quadEnable = value; }

void DataProcessor::SetFlipRows(bool fd) {
    flipRows = fd;
    // flip only right port of quad
    if (quadEnable) {
        flipRows = (index == 1 ? true : false);
    }
}

void DataProcessor::SetNumberofTotalFrames(uint64_t value) {
    nTotalFrames = value;
}

void DataProcessor::SetAdditionalJsonHeader(
    const std::map<std::string, std::string> &json) {
    std::lock_guard<std::mutex> lock(additionalJsonMutex);
    additionalJsonHeader = json;
    isAdditionalJsonUpdated = true;
}

void DataProcessor::ResetParametersforNewAcquisition() {
    StopRunning();
    startedFlag = false;
    numFramesCaught = 0;
    firstIndex = 0;
    currentFrameIndex = 0;
    firstStreamerFrame = true;
    streamCurrentFrame = false;
    completeImageToStreamBeforeCropping =
        make_unique<char[]>(generalData->imageSize);
}

void DataProcessor::RecordFirstIndex(uint64_t fnum) {
    // listen to this fnum, later +1
    currentFrameIndex = fnum;
    startedFlag = true;
    firstIndex = fnum;
    LOG(logDEBUG1) << index << " First Index:" << firstIndex;
}

void DataProcessor::CloseFiles() {
    if (dataFile)
        dataFile->CloseFile();
}

void DataProcessor::DeleteFiles() {
    CloseFiles();
    delete dataFile;
    dataFile = nullptr;
}
void DataProcessor::SetupFileWriter(const bool filewriteEnable,
                                    const fileFormat fileFormatType,
                                    std::mutex *hdf5LibMutex) {
    DeleteFiles();
    if (filewriteEnable) {
        switch (fileFormatType) {
#ifdef HDF5C
        case HDF5:
            dataFile = new HDF5DataFile(index, hdf5LibMutex);
            break;
#endif
        case BINARY:
            dataFile = new BinaryDataFile(index);
            break;
        default:
            throw RuntimeError("Unknown file format (compile with hdf5 flags");
        }
    }
}

void DataProcessor::CreateFirstFiles(const std::string &fileNamePrefix,
                                     const uint64_t fileIndex,
                                     const bool overWriteEnable,
                                     const bool silentMode,
                                     const bool detectorDataStream) {
    if (dataFile == nullptr) {
        throw RuntimeError("file object not contstructed");
    }
    CloseFiles();

    // deactivated (half module/ single port or no roi), dont write file
    if (!activated || !detectorDataStream || receiverNoRoi) {
        return;
    }

#ifdef HDF5C
    int nx = generalData->nPixelsX;
    int ny = generalData->nPixelsY;
    if (receiverRoiEnabled) {
        nx = receiverRoi.xmax - receiverRoi.xmin + 1;
        ny = receiverRoi.ymax - receiverRoi.ymin + 1;
        if (receiverRoi.ymax == -1 || receiverRoi.ymin == -1) {
            ny = 1;
        }
    }
#endif
    switch (dataFile->GetFileFormat()) {
#ifdef HDF5C
    case HDF5:
        dataFile->CreateFirstHDF5DataFile(
            fileNamePrefix, fileIndex, overWriteEnable, silentMode,
            udpPortNumber, generalData->framesPerFile, nTotalFrames, nx, ny,
            generalData->dynamicRange);
        break;
#endif
    case BINARY:
        dataFile->CreateFirstBinaryDataFile(
            fileNamePrefix, fileIndex, overWriteEnable, silentMode,
            udpPortNumber, generalData->framesPerFile);
        break;
    default:
        throw RuntimeError("Unknown file format (compile with hdf5 flags");
    }
}

#ifdef HDF5C
uint32_t DataProcessor::GetFilesInAcquisition() const {
    if (dataFile == nullptr) {
        throw RuntimeError("No data file object created to get number of "
                           "files in acquiistion");
    }
    return dataFile->GetFilesInAcquisition();
}

std::string DataProcessor::CreateVirtualFile(
    const std::string &filePath, const std::string &fileNamePrefix,
    const uint64_t fileIndex, const bool overWriteEnable, const bool silentMode,
    const int modulePos, const int numModX, const int numModY,
    std::mutex *hdf5LibMutex) {

    if (receiverRoiEnabled) {
        throw std::runtime_error(
            "Skipping virtual hdf5 file since rx_roi is enabled.");
    }

    bool gotthard25um = ((generalData->detType == GOTTHARD ||
                          generalData->detType == GOTTHARD2) &&
                         (numModX * numModY) == 2);

    // 0 for infinite files
    uint32_t framesPerFile =
        ((generalData->framesPerFile == 0) ? numFramesCaught
                                           : generalData->framesPerFile);

    // TODO: assumption 1: create virtual file even if no data in other
    // files (they exist anyway) assumption2: virtual file max frame index
    // is from R0 P0 (difference from others when missing frames or for a
    // stop acquisition)
    return masterFileUtility::CreateVirtualHDF5File(
        filePath, fileNamePrefix, fileIndex, overWriteEnable, silentMode,
        modulePos, generalData->numUDPInterfaces, framesPerFile,
        generalData->nPixelsX, generalData->nPixelsY, generalData->dynamicRange,
        numFramesCaught, numModX, numModY, dataFile->GetPDataType(),
        dataFile->GetParameterNames(), dataFile->GetParameterDataTypes(),
        hdf5LibMutex, gotthard25um);
}

void DataProcessor::LinkFileInMaster(const std::string &masterFileName,
                                     const std::string &virtualFileName,
                                     const bool silentMode,
                                     std::mutex *hdf5LibMutex) {

    if (receiverRoiEnabled) {
        throw std::runtime_error(
            "Should not be here, roi with hdf5 virtual should throw.");
    }
    std::string fname{virtualFileName}, masterfname{masterFileName};
    // if no virtual file, link data file
    if (virtualFileName.empty()) {
        fname = dataFile->GetFileName();
    }
    masterFileUtility::LinkHDF5FileInMaster(masterfname, fname,
                                            dataFile->GetParameterNames(),
                                            silentMode, hdf5LibMutex);
}
#endif

std::string DataProcessor::CreateMasterFile(
    const std::string &filePath, const std::string &fileNamePrefix,
    const uint64_t fileIndex, const bool overWriteEnable, bool silentMode,
    const fileFormat fileFormatType, MasterAttributes *attr,
    std::mutex *hdf5LibMutex) {

    attr->framesInFile = numFramesCaught;

    std::unique_ptr<File> masterFile{nullptr};
    switch (fileFormatType) {
#ifdef HDF5C
    case HDF5:
        return masterFileUtility::CreateMasterHDF5File(
            filePath, fileNamePrefix, fileIndex, overWriteEnable, silentMode,
            attr, hdf5LibMutex);
#endif
    case BINARY:
        return masterFileUtility::CreateMasterBinaryFile(
            filePath, fileNamePrefix, fileIndex, overWriteEnable, silentMode,
            attr);
    default:
        throw RuntimeError("Unknown file format (compile with hdf5 flags");
    }
}

void DataProcessor::ThreadExecution() {
    char *buffer = nullptr;
    fifo->PopAddress(buffer);
    LOG(logDEBUG5) << "DataProcessor " << index << ", " << std::hex
                   << static_cast<void *>(buffer) << std::dec << ":" << buffer;
    auto *memImage = reinterpret_cast<image_structure *>(buffer);

    // check dummy
    LOG(logDEBUG1) << "DataProcessor " << index
                   << ", Numbytes:" << memImage->size;
    if (memImage->size == DUMMY_PACKET_VALUE) {
        StopProcessing(buffer);
        return;
    }

    try {
        ProcessAnImage(memImage->header, memImage->size, memImage->firstIndex,
                       memImage->data);
    } catch (const std::exception &e) {
        fifo->FreeAddress(buffer);
        return;
    }

    // stream (if time/freq to stream) or free
    if (streamCurrentFrame) {
        // copy the complete image back if roi enabled
        if (receiverRoiEnabled) {
            memImage->size = generalData->imageSize;
            memcpy(memImage->data, &completeImageToStreamBeforeCropping[0],
                   generalData->imageSize);
        }
        fifo->PushAddressToStream(buffer);
    } else {
        fifo->FreeAddress(buffer);
    }
}

void DataProcessor::StopProcessing(char *buf) {
    LOG(logDEBUG1) << "DataProcessing " << index << ": Dummy";

    // stream or free
    if (dataStreamEnable)
        fifo->PushAddressToStream(buf);
    else
        fifo->FreeAddress(buf);

    CloseFiles();
    StopRunning();
    LOG(logDEBUG1) << index << ": Processing Completed";
}

void DataProcessor::ProcessAnImage(sls_receiver_header &header, size_t &size,
                                   size_t &firstImageIndex, char *data) {
    uint64_t fnum = header.detHeader.frameNumber;
    LOG(logDEBUG1) << "DataProcessing " << index << ": fnum:" << fnum;
    currentFrameIndex = fnum;
    numFramesCaught++;
    uint32_t nump = header.detHeader.packetNumber;

    if (!startedFlag) {
        RecordFirstIndex(fnum);
        if (dataStreamEnable) {
            // restart timer
            clock_gettime(CLOCK_REALTIME, &timerbegin);
            timerbegin.tv_sec -= streamingTimerInMs / 1000;
            timerbegin.tv_nsec -= (streamingTimerInMs % 1000) * 1000000;

            // to send first image
            currentFreqCount = streamingFrequency - streamingStartFnum;
        }
    }

    // frame padding
    if (framePadding && nump < generalData->packetsPerFrame)
        PadMissingPackets(header, data);

    // rearrange ctb digital bits
    if (!ctbDbitList.empty()) {
        ArrangeDbitData(size, data);
    } else if (ctbDbitReorder) {
        Reorder(size, data);
    } else if (ctbDbitOffset > 0) {
        RemoveTrailingBits(size, data);
    }

    // 'stream Image' check has to be done here before crop image
    // stream (if time/freq to stream) or free
    if (dataStreamEnable && SendToStreamer()) {
        if (firstStreamerFrame) {
            firstStreamerFrame = false;
            // write to memory structure of first streamer frame
            firstImageIndex = firstIndex;
        }
        streamCurrentFrame = true;
    } else {
        streamCurrentFrame = false;
    }

    if (receiverRoiEnabled) {
        // copy the complete image to stream before cropping
        if (streamCurrentFrame) {
            memcpy(&completeImageToStreamBeforeCropping[0], data,
                   generalData->imageSize);
        }
        CropImage(size, data);
    }

    try {
        // callbacks
        if (rawDataReadyCallBack != nullptr) {

            uint64_t frameIndex = fnum - firstIndex;
            // update local copy only if it was updated (to prevent locking each
            // time)
            if (isAdditionalJsonUpdated) {
                std::lock_guard<std::mutex> lock(additionalJsonMutex);
                localAdditionalJsonHeader = additionalJsonHeader;
                isAdditionalJsonUpdated = false;
            }

            dataCallbackHeader callbackHeader = {
                udpPortNumber,
                {static_cast<int>(generalData->nPixelsX),
                 static_cast<int>(generalData->nPixelsY)},
                fnum,
                frameIndex,
                (100 * ((double)(frameIndex + 1) / (double)(nTotalFrames))),
                (nump == generalData->packetsPerFrame ? true : false),
                flipRows,
                localAdditionalJsonHeader};

            rawDataReadyCallBack(header, callbackHeader, data, size,
                                 pRawDataReady);
        }
    } catch (const std::exception &e) {
        throw RuntimeError("Get Data Callback Error: " + std::string(e.what()));
    }

    // write to file
    if (dataFile) {
        try {
            dataFile->WriteToFile(data, header, size, fnum - firstIndex, nump);
        } catch (const RuntimeError &e) {
            ; // ignore write exception for now (TODO: send error message
              // via stopReceiver tcp)
        }
    }
}

bool DataProcessor::SendToStreamer() {
    // skip
    if (streamingFrequency == 0u) {
        if (!CheckTimer())
            return false;
    } else {
        if (!CheckCount())
            return false;
    }
    return true;
}

bool DataProcessor::CheckTimer() {
    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end);

    auto elapsed_s = (end.tv_sec - timerbegin.tv_sec) +
                     (end.tv_nsec - timerbegin.tv_nsec) / 1e9;
    double timer_s = streamingTimerInMs / 1e3;

    LOG(logDEBUG1) << index << " Timer elapsed time:" << elapsed_s
                   << " seconds";

    // still less than streaming timer, keep waiting
    if (elapsed_s < timer_s)
        return false;

    // restart timer
    clock_gettime(CLOCK_REALTIME, &timerbegin);
    return true;
}

bool DataProcessor::CheckCount() {
    if (currentFreqCount == streamingFrequency) {
        currentFreqCount = 1;
        return true;
    }
    currentFreqCount++;
    return false;
}

void DataProcessor::registerCallBackRawDataReady(
    void (*func)(sls_receiver_header &, dataCallbackHeader, char *, size_t &,
                 void *),
    void *arg) {
    rawDataReadyCallBack = func;
    pRawDataReady = arg;
}

void DataProcessor::PadMissingPackets(sls_receiver_header header, char *data) {
    LOG(logDEBUG) << index << ": Padding Missing Packets";

    uint32_t pperFrame = generalData->packetsPerFrame;

    uint32_t nmissing = pperFrame - header.detHeader.packetNumber;
    sls_bitset pmask = header.packetsMask;

    uint32_t dsize = generalData->dataSize;
    if (generalData->detType == GOTTHARD2 && index != 0) {
        dsize = generalData->vetoDataSize;
    }
    uint32_t corrected_dsize =
        dsize - ((pperFrame * dsize) - generalData->imageSize);
    LOG(logDEBUG1) << "bitmask: " << pmask.to_string();

    for (unsigned int pnum = 0; pnum < pperFrame; ++pnum) {

        // not missing packet
        if (pmask[pnum])
            continue;

        // done with padding, exit loop earlier
        if (nmissing == 0u)
            break;

        LOG(logDEBUG) << "padding for " << index << " for pnum: " << pnum
                      << std::endl;

        // missing packet
        switch (generalData->detType) {
        // for gotthard, 1st packet: 4 bytes fnum, CACA + CACA, 639*2 bytes
        // data
        //              2nd packet: 4 bytes fnum, previous 1*2 bytes data  +
        //              640*2 bytes data !!
        case GOTTHARD:
            if (pnum == 0u)
                memset(data + (pnum * dsize), 0xFF, dsize - 2);
            else
                memset(data + (pnum * dsize), 0xFF, dsize + 2);
            break;
        case CHIPTESTBOARD:
        case XILINX_CHIPTESTBOARD:
            if (pnum == (pperFrame - 1))
                memset(data + (pnum * dsize), 0xFF, corrected_dsize);
            else
                memset(data + (pnum * dsize), 0xFF, dsize);
            break;
        default:
            memset(data + (pnum * dsize), 0xFF, dsize);
            break;
        }
        --nmissing;
    }
}

void DataProcessor::RemoveTrailingBits(size_t &size, char *data) {
    const size_t nAnalogDataBytes = generalData->GetNumberOfAnalogDatabytes();
    const size_t nDigitalDataBytes = generalData->GetNumberOfDigitalDatabytes();
    const size_t nTransceiverDataBytes =
        generalData->GetNumberOfTransceiverDatabytes();

    const size_t ctbDigitalDataBytes = nDigitalDataBytes - ctbDbitOffset;

    // no digital data
    if (ctbDigitalDataBytes == 0) {
        LOG(logWARNING)
            << "No digital data for call back, yet ctbDbitOffset is non zero.";
        return;
    }

    // update size and copy data
    memmove(data + nAnalogDataBytes, data + nAnalogDataBytes + ctbDbitOffset,
            ctbDigitalDataBytes + nTransceiverDataBytes);

    size = nAnalogDataBytes + ctbDigitalDataBytes + nTransceiverDataBytes;
}

void DataProcessor::Reorder(size_t &size, char *data) {
    const size_t nAnalogDataBytes = generalData->GetNumberOfAnalogDatabytes();
    const size_t nDigitalDataBytes = generalData->GetNumberOfDigitalDatabytes();
    const size_t nTransceiverDataBytes =
        generalData->GetNumberOfTransceiverDatabytes();

    const size_t ctbDigitalDataBytes = nDigitalDataBytes - ctbDbitOffset;

    // no digital data
    if (ctbDigitalDataBytes == 0) {
        LOG(logWARNING)
            << "No digital data for call back, yet reorder is set to 1.";
        return;
    }

    // make sure data is aligned to 8 bytes before casting to uint64_t
    AlignedData<uint64_t> aligned_data(nullptr, nullptr);
    AlignData<uint64_t>(aligned_data, data + nAnalogDataBytes + ctbDbitOffset,
                        ctbDigitalDataBytes);

    uint64_t *source = aligned_data.ptr;

    const size_t numDigitalSamples = (ctbDigitalDataBytes / sizeof(uint64_t));

    size_t numBytesPerBit =
        (numDigitalSamples % 8 == 0)
            ? numDigitalSamples / 8
            : numDigitalSamples / 8 +
                  1; // number of bytes per bit in digital data after reordering

    size_t totalNumBytes =
        numBytesPerBit *
        64; // number of bytes for digital data after reordering

    std::vector<uint8_t> result(totalNumBytes, 0);
    uint8_t *dest = &result[0];

    int bitoffset = 0;
    // reorder
    for (size_t bi = 0; bi < 64; ++bi) {

        if (bitoffset != 0) {
            bitoffset = 0;
            ++dest;
        }

        for (auto *ptr = source; ptr < (source + numDigitalSamples); ++ptr) {
            uint8_t bit = (*ptr >> bi) & 1;
            *dest |= bit << bitoffset; // most significant bits will be padded
            ++bitoffset;
            if (bitoffset == 8) {
                bitoffset = 0;
                ++dest;
            }
        }
    }

    // move transceiver data to not overwrite and avoid gap in memory
    if (totalNumBytes != nDigitalDataBytes)
        memmove(data + nAnalogDataBytes + totalNumBytes * sizeof(uint8_t),
                data + nAnalogDataBytes + nDigitalDataBytes,
                nTransceiverDataBytes);

    // copy back to memory and update size
    size = totalNumBytes * sizeof(uint8_t) + nAnalogDataBytes +
           nTransceiverDataBytes;

    memcpy(data + nAnalogDataBytes, result.data(),
           totalNumBytes * sizeof(uint8_t));

    LOG(logDEBUG1) << "totalNumBytes: " << totalNumBytes
                   << " nAnalogDataBytes:" << nAnalogDataBytes
                   << " ctbDbitOffset:" << ctbDbitOffset
                   << " nTransceiverDataBytes:" << nTransceiverDataBytes
                   << " size:" << size;
}

/** ctb specific */
void DataProcessor::ArrangeDbitData(size_t &size, char *data) {
    size_t nAnalogDataBytes = generalData->GetNumberOfAnalogDatabytes();
    size_t nDigitalDataBytes = generalData->GetNumberOfDigitalDatabytes();
    size_t nTransceiverDataBytes =
        generalData->GetNumberOfTransceiverDatabytes();
    // TODO! (Erik) Refactor and add tests
    int ctbDigitalDataBytes = nDigitalDataBytes - ctbDbitOffset;

    // no digital data
    if (ctbDigitalDataBytes == 0) {
        LOG(logWARNING)
            << "No digital data for call back, yet dbitlist is not empty.";
        return;
    }

    AlignedData<uint64_t> aligned_data(nullptr, nullptr);
    AlignData<uint64_t>(aligned_data, data + nAnalogDataBytes + ctbDbitOffset,
                        ctbDigitalDataBytes);

    uint64_t *source = aligned_data.ptr;

    // auto *source = (uint64_t *)(data + nAnalogDataBytes + ctbDbitOffset);

    const int numDigitalSamples = (ctbDigitalDataBytes / sizeof(uint64_t));

    int totalNumBytes =
        0; // number of bytes for selected digital data given by dtbDbitList

    // store each selected bit from all samples consecutively
    if (ctbDbitReorder) {
        size_t numBitsPerDbit =
            numDigitalSamples; // num bits per selected digital
                               // Bit for all samples
        if ((numBitsPerDbit % 8) != 0)
            numBitsPerDbit += (8 - (numDigitalSamples % 8));
        totalNumBytes = (numBitsPerDbit / 8) * ctbDbitList.size();
    }
    // store all selected bits from one sample consecutively
    else {
        size_t numBitsPerSample =
            ctbDbitList.size(); // num bits for all selected bits per sample
        if ((numBitsPerSample % 8) != 0)
            numBitsPerSample += (8 - (numBitsPerSample % 8));
        totalNumBytes = (numBitsPerSample / 8) * numDigitalSamples;
    }

    std::vector<uint8_t> result(totalNumBytes, 0);
    uint8_t *dest = &result[0];

    if (ctbDbitReorder) {
        // loop through digital bit enable vector
        int bitoffset = 0;
        for (auto bi : ctbDbitList) {
            // where numbits * numDigitalSamples is not a multiple of 8
            if (bitoffset != 0) {
                bitoffset = 0;
                ++dest;
            }

            // loop through the frame digital data
            for (auto *ptr = source; ptr < (source + numDigitalSamples);) {
                // get selected bit from each 8 bit
                uint8_t bit = (*ptr++ >> bi) & 1;
                *dest |= bit << bitoffset; // stored as least significant
                ++bitoffset;
                // extract destination in 8 bit batches
                if (bitoffset == 8) {
                    bitoffset = 0;
                    ++dest;
                }
            }
        }
    } else {
        // loop through the digital data
        int bitoffset = 0;
        for (auto *ptr = source; ptr < (source + numDigitalSamples); ++ptr) {
            // where bit enable vector size is not a multiple of 8
            if (bitoffset != 0) {
                bitoffset = 0;
                ++dest;
            }

            // loop through digital bit enable vector
            for (auto bi : ctbDbitList) {
                // get selected bit from each 64 bit
                uint8_t bit = (*ptr >> bi) & 1;
                *dest |= bit << bitoffset;
                ++bitoffset;
                // extract destination in 8 bit batches
                if (bitoffset == 8) {
                    bitoffset = 0;
                    ++dest;
                }
            }
        }
    }

    size = totalNumBytes * sizeof(uint8_t) + nAnalogDataBytes +
           nTransceiverDataBytes;

    // check if size changed, if so move transceiver data to avoid gap in memory
    if (size != nAnalogDataBytes + nDigitalDataBytes + nTransceiverDataBytes)
        memmove(data + nAnalogDataBytes + totalNumBytes * sizeof(uint8_t),
                data + nAnalogDataBytes + nDigitalDataBytes,
                nTransceiverDataBytes);

    // copy back to memory and update size
    memcpy(data + nAnalogDataBytes, result.data(),
           totalNumBytes * sizeof(uint8_t));

    LOG(logDEBUG1) << "totalNumBytes: " << totalNumBytes
                   << " nAnalogDataBytes:" << nAnalogDataBytes
                   << " ctbDbitOffset:" << ctbDbitOffset
                   << " nTransceiverDataBytes:" << nTransceiverDataBytes
                   << " size:" << size;
}

void DataProcessor::CropImage(size_t &size, char *data) {
    LOG(logDEBUG) << "Cropping Image to ROI " << ToString(receiverRoi);
    int nPixelsX = generalData->nPixelsX;
    int xmin = receiverRoi.xmin;
    int xmax = receiverRoi.xmax;
    int ymin = receiverRoi.ymin;
    int ymax = receiverRoi.ymax;
    int xwidth = xmax - xmin + 1;
    int ywidth = ymax - ymin + 1;
    if (ymin == -1 || ymax == -1) {
        ywidth = 1;
        ymin = 0;
    }

    // calculate total roi size
    double bytesPerPixel = generalData->dynamicRange / 8.00;
    int startOffset = (int)((nPixelsX * ymin + xmin) * bytesPerPixel);

    // write size into memory
    std::size_t roiImageSize = xwidth * ywidth * bytesPerPixel;
    LOG(logDEBUG) << "roiImageSize:" << roiImageSize;
    size = roiImageSize;

    // copy the roi to the beginning of the image
    char *dstOffset = data;
    char *srcOffset = dstOffset + startOffset;

    // entire width
    if (xwidth == nPixelsX) {
        memcpy(dstOffset, srcOffset, roiImageSize);
    }
    // width is cropped
    else {
        for (int y = 0; y != ywidth; ++y) {
            memcpy(dstOffset, srcOffset, xwidth * bytesPerPixel);
            dstOffset += (int)(xwidth * bytesPerPixel);
            srcOffset += (int)(generalData->nPixelsX * bytesPerPixel);
        }
    }
}

} // namespace sls
