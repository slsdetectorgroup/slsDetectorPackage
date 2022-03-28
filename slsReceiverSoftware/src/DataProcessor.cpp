// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
/************************************************
 * @file DataProcessor.cpp
 * @short creates data processor thread that
 * pulls pointers to memory addresses from fifos
 * and processes data stored in them & writes them to file
 ***********************************************/

#include "DataProcessor.h"
#include "BinaryDataFile.h"
#include "BinaryMasterFile.h"
#include "Fifo.h"
#include "GeneralData.h"
#include "MasterAttributes.h"
#ifdef HDF5C
#include "HDF5DataFile.h"
#include "HDF5MasterFile.h"
#include "HDF5VirtualFile.h"
#endif
#include "DataStreamer.h"
#include "sls/container_utils.h"
#include "sls/sls_detector_exceptions.h"

#include <cerrno>
#include <cstring>
#include <iostream>

const std::string DataProcessor::typeName_ = "DataProcessor";

DataProcessor::DataProcessor(int index, detectorType detectorType, Fifo *fifo,
                             bool *activated, bool *dataStreamEnable,
                             uint32_t *streamingFrequency,
                             uint32_t *streamingTimerInMs,
                             uint32_t *streamingStartFnum, bool *framePadding,
                             std::vector<int> *ctbDbitList, int *ctbDbitOffset,
                             int *ctbAnalogDataBytes)
    : ThreadObject(index, typeName_), fifo_(fifo), detectorType_(detectorType),
      dataStreamEnable_(dataStreamEnable), activated_(activated),
      streamingFrequency_(streamingFrequency),
      streamingTimerInMs_(streamingTimerInMs),
      streamingStartFnum_(streamingStartFnum), framePadding_(framePadding),
      ctbDbitList_(ctbDbitList), ctbDbitOffset_(ctbDbitOffset),
      ctbAnalogDataBytes_(ctbAnalogDataBytes), firstStreamerFrame_(false) {

    LOG(logDEBUG) << "DataProcessor " << index << " created";

    memset((void *)&timerbegin_, 0, sizeof(timespec));
}

DataProcessor::~DataProcessor() { DeleteFiles(); }

/** getters */

bool DataProcessor::GetStartedFlag() const { return startedFlag_; }

void DataProcessor::SetFifo(Fifo *fifo) { fifo_ = fifo; }

void DataProcessor::ResetParametersforNewAcquisition() {
    StopRunning();
    startedFlag_ = false;
    numFramesCaught_ = 0;
    firstIndex_ = 0;
    currentFrameIndex_ = 0;
    firstStreamerFrame_ = true;
}

void DataProcessor::RecordFirstIndex(uint64_t fnum) {
    // listen to this fnum, later +1
    currentFrameIndex_ = fnum;

    startedFlag_ = true;
    firstIndex_ = fnum;

    LOG(logDEBUG1) << index << " First Index:" << firstIndex_;
}

void DataProcessor::SetGeneralData(GeneralData *generalData) {
    generalData_ = generalData;
}

void DataProcessor::CloseFiles() {
    if (dataFile_)
        dataFile_->CloseFile();
#ifdef HDF5C
    if (virtualFile_)
        virtualFile_->CloseFile();
#endif
}

void DataProcessor::DeleteFiles() {
    CloseFiles();
    if (dataFile_) {
        delete dataFile_;
        dataFile_ = nullptr;
    }
#ifdef HDF5C
    if (virtualFile_) {
        delete virtualFile_;
        virtualFile_ = nullptr;
    }
#endif
}
void DataProcessor::SetupFileWriter(const bool filewriteEnable,
                                    const fileFormat fileFormatType,
                                    std::mutex *hdf5LibMutex) {
    DeleteFiles();
    if (filewriteEnable) {
        switch (fileFormatType) {
#ifdef HDF5C
        case HDF5:
            dataFile_ = new HDF5DataFile(index, hdf5LibMutex);
            break;
#endif
        case BINARY:
            dataFile_ = new BinaryDataFile(index);
            break;
        default:
            throw sls::RuntimeError(
                "Unknown file format (compile with hdf5 flags");
        }
    }
}

void DataProcessor::CreateFirstFiles(
    const std::string filePath, const std::string fileNamePrefix,
    const uint64_t fileIndex, const bool overWriteEnable, const bool silentMode,
    const int modulePos, const int numUnitsPerReadout,
    const uint32_t udpPortNumber, const uint32_t maxFramesPerFile,
    const uint64_t numImages, const uint32_t dynamicRange,
    const bool detectorDataStream) {
    if (dataFile_ == nullptr) {
        throw sls::RuntimeError("file object not contstructed");
    }
    CloseFiles();

    // deactivated (half module/ single port), dont write file
    if ((!*activated_) || (!detectorDataStream)) {
        return;
    }

    switch (dataFile_->GetFileFormat()) {
#ifdef HDF5C
    case HDF5:
        dataFile_->CreateFirstHDF5DataFile(
            filePath, fileNamePrefix, fileIndex, overWriteEnable, silentMode,
            modulePos, numUnitsPerReadout, udpPortNumber, maxFramesPerFile,
            numImages, generalData_->nPixelsX, generalData_->nPixelsY,
            dynamicRange);
        break;
#endif
    case BINARY:
        dataFile_->CreateFirstBinaryDataFile(
            filePath, fileNamePrefix, fileIndex, overWriteEnable, silentMode,
            modulePos, numUnitsPerReadout, udpPortNumber, maxFramesPerFile);
        break;
    default:
        throw sls::RuntimeError("Unknown file format (compile with hdf5 flags");
    }
}

#ifdef HDF5C
uint32_t DataProcessor::GetFilesInAcquisition() const {
    if (dataFile_ == nullptr) {
        throw sls::RuntimeError("No data file object created to get number of "
                                "files in acquiistion");
    }
    return dataFile_->GetFilesInAcquisition();
}

void DataProcessor::CreateVirtualFile(
    const std::string filePath, const std::string fileNamePrefix,
    const uint64_t fileIndex, const bool overWriteEnable, const bool silentMode,
    const int modulePos, const int numUnitsPerReadout,
    const uint32_t maxFramesPerFile, const uint64_t numImages,
    const uint32_t dynamicRange, const int numModX, const int numModY,
    std::mutex *hdf5LibMutex) {

    if (virtualFile_) {
        delete virtualFile_;
    }
    bool gotthard25um =
        ((detectorType_ == GOTTHARD || detectorType_ == GOTTHARD2) &&
         (numModX * numModY) == 2);
    virtualFile_ = new HDF5VirtualFile(hdf5LibMutex, gotthard25um);

    // maxframesperfile = 0 for infinite files
    uint32_t framesPerFile =
        ((maxFramesPerFile == 0) ? numFramesCaught_ : maxFramesPerFile);

    // TODO: assumption 1: create virtual file even if no data in other
    // files (they exist anyway) assumption2: virtual file max frame index
    // is from R0 P0 (difference from others when missing frames or for a
    // stop acquisition)
    virtualFile_->CreateVirtualFile(
        filePath, fileNamePrefix, fileIndex, overWriteEnable, silentMode,
        modulePos, numUnitsPerReadout, framesPerFile, numImages,
        generalData_->nPixelsX, generalData_->nPixelsY, dynamicRange,
        numFramesCaught_, numModX, numModY, dataFile_->GetPDataType(),
        dataFile_->GetParameterNames(), dataFile_->GetParameterDataTypes());
}

void DataProcessor::LinkDataInMasterFile(const string &masterFileName,
                                         const bool silentMode,
                                         std::mutex *hdf5LibMutex) {
    std::string fname, datasetName;
    if (virtualFile_) {
        auto res = virtualFile_->GetFileAndDatasetName();
        fname = res[0];
        datasetName = res[1];
    } else {
        auto res = dataFile_->GetFileAndDatasetName();
        fname = res[0];
        datasetName = res[1];
    }
    // link in master
    HDF5MasterFile::LinkDataFile(masterFileName, fname, datasetName,
                                 dataFile_->GetParameterNames(), silentMode,
                                 &hdf5LibMutex);
}
#endif

std::string DataProcessor::CreateMasterFile(
    const std::string &filePath, const std::string &fileNamePrefix,
    const uint64_t fileIndex, const bool overWriteEnable, bool silentMode,
    const fileFormat fileFormatType, MasterAttributes *attr,
    std::mutex *hdf5LibMutex) {

    attr->framesInFile = numFramesCaught_;

    std::unique_ptr<File> masterFile{nullptr};
    switch (fileFormatType) {
#ifdef HDF5C
    case HDF5:
        return HDF5MasterFile::CreateMasterFile(filePath, fileNamePrefix,
                                                fileIndex, overWriteEnable,
                                                silentMode, attr, hdf5LibMutex);
#endif
    case BINARY:
        return BinaryMasterFile::CreateMasterFile(filePath, fileNamePrefix,
                                                  fileIndex, overWriteEnable,
                                                  silentMode, attr);
    default:
        throw sls::RuntimeError("Unknown file format (compile with hdf5 flags");
    }
}

void DataProcessor::ThreadExecution() {
    char *buffer = nullptr;
    fifo_->PopAddress(buffer);
    LOG(logDEBUG5) << "DataProcessor " << index
                   << ", "
                      "pop 0x"
                   << std::hex << (void *)(buffer) << std::dec << ":" << buffer;

    // check dummy
    auto numBytes = (uint32_t)(*((uint32_t *)buffer));
    LOG(logDEBUG1) << "DataProcessor " << index << ", Numbytes:" << numBytes;
    if (numBytes == DUMMY_PACKET_VALUE) {
        StopProcessing(buffer);
        return;
    }

    uint64_t fnum = 0;
    try {
        fnum = ProcessAnImage(buffer);
    } catch (const std::exception &e) {
        fifo_->FreeAddress(buffer);
        return;
    }
    // stream (if time/freq to stream) or free
    if (*dataStreamEnable_ && SendToStreamer()) {
        // if first frame to stream, add frame index to fifo header (might
        // not be the first)
        if (firstStreamerFrame_) {
            firstStreamerFrame_ = false;
            (*((uint32_t *)(buffer + FIFO_DATASIZE_NUMBYTES))) =
                (uint32_t)(fnum - firstIndex_);
        }
        fifo_->PushAddressToStream(buffer);
    } else {
        fifo_->FreeAddress(buffer);
    }
}

void DataProcessor::StopProcessing(char *buf) {
    LOG(logDEBUG1) << "DataProcessing " << index << ": Dummy";

    // stream or free
    if (*dataStreamEnable_)
        fifo_->PushAddressToStream(buf);
    else
        fifo_->FreeAddress(buf);

    CloseFiles();
    StopRunning();
    LOG(logDEBUG1) << index << ": Processing Completed";
}

uint64_t DataProcessor::ProcessAnImage(char *buf) {

    auto *rheader = (sls_receiver_header *)(buf + FIFO_HEADER_NUMBYTES);
    sls_detector_header header = rheader->detHeader;
    uint64_t fnum = header.frameNumber;
    currentFrameIndex_ = fnum;
    numFramesCaught_++;
    uint32_t nump = header.packetNumber;

    LOG(logDEBUG1) << "DataProcessing " << index << ": fnum:" << fnum;

    if (!startedFlag_) {
        RecordFirstIndex(fnum);
        if (*dataStreamEnable_) {
            // restart timer
            clock_gettime(CLOCK_REALTIME, &timerbegin_);
            timerbegin_.tv_sec -= (*streamingTimerInMs_) / 1000;
            timerbegin_.tv_nsec -= ((*streamingTimerInMs_) % 1000) * 1000000;

            // to send first image
            currentFreqCount_ = *streamingFrequency_ - *streamingStartFnum_;
        }
    }

    // frame padding
    if (*activated_ && *framePadding_ && nump < generalData_->packetsPerFrame)
        PadMissingPackets(buf);

    // rearrange ctb digital bits (if ctbDbitlist is not empty)
    if (!(*ctbDbitList_).empty()) {
        RearrangeDbitData(buf);
    }

    try {
        // normal call back
        if (rawDataReadyCallBack != nullptr) {
            rawDataReadyCallBack((char *)rheader,
                                 buf + FIFO_HEADER_NUMBYTES +
                                     sizeof(sls_receiver_header),
                                 (uint32_t)(*((uint32_t *)buf)), pRawDataReady);
        }

        // call back with modified size
        else if (rawDataModifyReadyCallBack != nullptr) {
            auto revsize = (uint32_t)(*((uint32_t *)buf));
            rawDataModifyReadyCallBack((char *)rheader,
                                       buf + FIFO_HEADER_NUMBYTES +
                                           sizeof(sls_receiver_header),
                                       revsize, pRawDataReady);
            (*((uint32_t *)buf)) = revsize;
        }
    } catch (const std::exception &e) {
        throw sls::RuntimeError("Get Data Callback Error: " +
                                std::string(e.what()));
    }

    // write to file
    if (dataFile_) {
        try {
            dataFile_->WriteToFile(
                buf + FIFO_HEADER_NUMBYTES,
                sizeof(sls_receiver_header) +
                    (uint32_t)(*((uint32_t *)buf)), //+ size of data (resizable
                                                    // from previous call back
                fnum - firstIndex_, nump);
        } catch (const sls::RuntimeError &e) {
            ; // ignore write exception for now (TODO: send error message
              // via stopReceiver tcp)
        }
    }
    return fnum;
}

bool DataProcessor::SendToStreamer() {
    // skip
    if ((*streamingFrequency_) == 0u) {
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

    LOG(logDEBUG1) << index << " Timer elapsed time:"
                   << ((end.tv_sec - timerbegin_.tv_sec) +
                       (end.tv_nsec - timerbegin_.tv_nsec) / 1000000000.0)
                   << " seconds";
    // still less than streaming timer, keep waiting
    if (((end.tv_sec - timerbegin_.tv_sec) +
         (end.tv_nsec - timerbegin_.tv_nsec) / 1000000000.0) <
        ((double)*streamingTimerInMs_ / 1000.00))
        return false;

    // restart timer
    clock_gettime(CLOCK_REALTIME, &timerbegin_);
    return true;
}

bool DataProcessor::CheckCount() {
    if (currentFreqCount_ == *streamingFrequency_) {
        currentFreqCount_ = 1;
        return true;
    }
    currentFreqCount_++;
    return false;
}

void DataProcessor::registerCallBackRawDataReady(void (*func)(char *, char *,
                                                              uint32_t, void *),
                                                 void *arg) {
    rawDataReadyCallBack = func;
    pRawDataReady = arg;
}

void DataProcessor::registerCallBackRawDataModifyReady(
    void (*func)(char *, char *, uint32_t &, void *), void *arg) {
    rawDataModifyReadyCallBack = func;
    pRawDataReady = arg;
}

void DataProcessor::PadMissingPackets(char *buf) {
    LOG(logDEBUG) << index << ": Padding Missing Packets";

    uint32_t pperFrame = generalData_->packetsPerFrame;
    auto *header = (sls_receiver_header *)(buf + FIFO_HEADER_NUMBYTES);
    uint32_t nmissing = pperFrame - header->detHeader.packetNumber;
    sls_bitset pmask = header->packetsMask;

    uint32_t dsize = generalData_->dataSize;
    if (detectorType_ == GOTTHARD2 && index != 0) {
        dsize = generalData_->vetoDataSize;
    }
    uint32_t fifohsize = generalData_->fifoBufferHeaderSize;
    uint32_t corrected_dsize =
        dsize - ((pperFrame * dsize) - generalData_->imageSize);
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
        switch (detectorType_) {
        // for gotthard, 1st packet: 4 bytes fnum, CACA + CACA, 639*2 bytes
        // data
        //              2nd packet: 4 bytes fnum, previous 1*2 bytes data  +
        //              640*2 bytes data !!
        case GOTTHARD:
            if (pnum == 0u)
                memset(buf + fifohsize + (pnum * dsize), 0xFF, dsize - 2);
            else
                memset(buf + fifohsize + (pnum * dsize), 0xFF, dsize + 2);
            break;
        case CHIPTESTBOARD:
        case MOENCH:
            if (pnum == (pperFrame - 1))
                memset(buf + fifohsize + (pnum * dsize), 0xFF, corrected_dsize);
            else
                memset(buf + fifohsize + (pnum * dsize), 0xFF, dsize);
            break;
        default:
            memset(buf + fifohsize + (pnum * dsize), 0xFF, dsize);
            break;
        }
        --nmissing;
    }
}

/** ctb specific */
void DataProcessor::RearrangeDbitData(char *buf) {
    // TODO! (Erik) Refactor and add tests
    int totalSize = (int)(*((uint32_t *)buf));
    int ctbDigitalDataBytes =
        totalSize - (*ctbAnalogDataBytes_) - (*ctbDbitOffset_);

    // no digital data
    if (ctbDigitalDataBytes == 0) {
        LOG(logWARNING)
            << "No digital data for call back, yet dbitlist is not empty.";
        return;
    }

    const int numSamples = (ctbDigitalDataBytes / sizeof(uint64_t));
    const int digOffset = FIFO_HEADER_NUMBYTES + sizeof(sls_receiver_header) +
                          (*ctbAnalogDataBytes_);

    // ceil as numResult8Bits could be decimal
    const int numResult8Bits =
        ceil((double)(numSamples * (*ctbDbitList_).size()) / 8.00);
    std::vector<uint8_t> result(numResult8Bits);
    uint8_t *dest = &result[0];

    auto *source = (uint64_t *)(buf + digOffset + (*ctbDbitOffset_));

    // loop through digital bit enable vector
    int bitoffset = 0;
    for (auto bi : (*ctbDbitList_)) {
        // where numbits * numsamples is not a multiple of 8
        if (bitoffset != 0) {
            bitoffset = 0;
            ++dest;
        }

        // loop through the frame digital data
        for (auto ptr = source; ptr < (source + numSamples);) {
            // get selected bit from each 8 bit
            uint8_t bit = (*ptr++ >> bi) & 1;
            *dest |= bit << bitoffset;
            ++bitoffset;
            // extract destination in 8 bit batches
            if (bitoffset == 8) {
                bitoffset = 0;
                ++dest;
            }
        }
    }

    // copy back to buf and update size
    memcpy(buf + digOffset, result.data(), numResult8Bits * sizeof(uint8_t));
    (*((uint32_t *)buf)) = numResult8Bits * sizeof(uint8_t);
}
