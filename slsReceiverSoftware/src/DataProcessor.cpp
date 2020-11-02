/************************************************
 * @file DataProcessor.cpp
 * @short creates data processor thread that
 * pulls pointers to memory addresses from fifos
 * and processes data stored in them & writes them to file
 ***********************************************/

#include "DataProcessor.h"
#include "BinaryFile.h"
#include "Fifo.h"
#include "GeneralData.h"
#include "MasterAttributes.h"
#ifdef HDF5C
#include "HDF5File.h"
#endif
#include "DataStreamer.h"
#include "sls/sls_detector_exceptions.h"

#include <cerrno>
#include <cstring>
#include <iostream>

const std::string DataProcessor::TypeName = "DataProcessor";

DataProcessor::DataProcessor(int ind, detectorType dtype, Fifo *f,
                             fileFormat *ftype, bool fwenable, bool *mfwenable,
                             bool *dsEnable, uint32_t *freq,
                             uint32_t *timer, uint32_t *sfnum, bool *fp,
                             bool *act, bool *depaden, bool *sm,
                             std::vector<int> *cdl, int *cdo, int *cad)
    : ThreadObject(ind, TypeName), fifo(f), myDetectorType(dtype),
      dataStreamEnable(dsEnable), fileFormatType(ftype),
      fileWriteEnable(fwenable), masterFileWriteEnable(mfwenable),
      streamingFrequency(freq), streamingTimerInMs(timer),
      streamingStartFnum(sfnum), activated(act),
      deactivatedPaddingEnable(depaden), silentMode(sm),
      framePadding(fp), ctbDbitList(cdl), ctbDbitOffset(cdo),
      ctbAnalogDataBytes(cad), firstStreamerFrame(false) {
    LOG(logDEBUG) << "DataProcessor " << ind << " created";
    memset((void *)&timerBegin, 0, sizeof(timespec));
}

DataProcessor::~DataProcessor() { delete file; }

/** getters */

bool DataProcessor::GetStartedFlag() { return startedFlag; }

uint64_t DataProcessor::GetNumFramesCaught() { return numFramesCaught; }

uint64_t DataProcessor::GetCurrentFrameIndex() { return currentFrameIndex; }

uint64_t DataProcessor::GetProcessedIndex() {
    return currentFrameIndex - firstIndex;
}

void DataProcessor::SetFifo(Fifo *f) { fifo = f; }

void DataProcessor::ResetParametersforNewAcquisition() {
    StopRunning();
    startedFlag = false;
    numFramesCaught = 0;
    firstIndex = 0;
    currentFrameIndex = 0;
    firstStreamerFrame = true;
}

void DataProcessor::RecordFirstIndex(uint64_t fnum) {
    // listen to this fnum, later +1
    currentFrameIndex = fnum;

    startedFlag = true;
    firstIndex = fnum;

    LOG(logDEBUG1) << index << " First Index:" << firstIndex;
}

void DataProcessor::SetGeneralData(GeneralData *g) {
    generalData = g;
    if (file != nullptr) {
        if (file->GetFileType() == HDF5) {
            file->SetNumberofPixels(generalData->nPixelsX,
                                    generalData->nPixelsY);
        }
    }
}

void DataProcessor::SetFileFormat(const fileFormat f) {
    if ((file != nullptr) && file->GetFileType() != f) {
        // remember the pointer values before they are destroyed
        int nd[MAX_DIMENSIONS];
        nd[0] = 0;
        nd[1] = 0;
        uint32_t *maxf = nullptr;
        std::string *fname = nullptr;
        std::string *fpath = nullptr;
        uint64_t *findex = nullptr;
        bool *owenable = nullptr;
        int *dindex = nullptr;
        int *nunits = nullptr;
        uint64_t *nf = nullptr;
        uint32_t *dr = nullptr;
        uint32_t *port = nullptr;
        file->GetMemberPointerValues(nd, maxf, fname, fpath, findex, owenable,
                                     dindex, nunits, nf, dr, port);
        // create file writer with same pointers
        SetupFileWriter(fileWriteEnable, nd, maxf, fname, fpath, findex,
                        owenable, dindex, nunits, nf, dr, port);
    }
}

void DataProcessor::SetupFileWriter(bool fwe, int *nd, uint32_t *maxf,
                                    std::string *fname, std::string *fpath,
                                    uint64_t *findex, bool *owenable,
                                    int *dindex, int *nunits, uint64_t *nf,
                                    uint32_t *dr, uint32_t *portno,
                                    GeneralData *g) {
    fileWriteEnable = fwe;
    if (g != nullptr)
        generalData = g;

    if (file != nullptr) {
        delete file;
        file = nullptr;
    }

    if (fileWriteEnable) {
        switch (*fileFormatType) {
#ifdef HDF5C
        case HDF5:
            file = new HDF5File(index, maxf, nd, fname, fpath, findex, owenable,
                                dindex, nunits, nf, dr, portno,
                                generalData->nPixelsX, generalData->nPixelsY,
                                silentMode);
            break;
#endif
        default:
            file =
                new BinaryFile(index, maxf, nd, fname, fpath, findex, owenable,
                               dindex, nunits, nf, dr, portno, silentMode);
            break;
        }
    }
}

// only the first file
void DataProcessor::CreateNewFile(MasterAttributes *attr) {
    if (file == nullptr) {
        throw sls::RuntimeError("file object not contstructed");
    }
    file->CloseAllFiles();
    file->resetSubFileIndex();
    file->CreateMasterFile(*masterFileWriteEnable, attr);
    file->CreateFile();
}

void DataProcessor::CloseFiles() {
    if (file != nullptr)
        file->CloseAllFiles();
}

void DataProcessor::EndofAcquisition(bool anyPacketsCaught, uint64_t numf) {
    if ((file != nullptr) && file->GetFileType() == HDF5) {
        try {
            file->EndofAcquisition(anyPacketsCaught, numf);
        } catch (const sls::RuntimeError &e) {
            ; // ignore for now //TODO: send error to client via stop receiver
        }
    }
}

void DataProcessor::ThreadExecution() {
    char *buffer = nullptr;
    fifo->PopAddress(buffer);
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

    uint64_t fnum = ProcessAnImage(buffer);

    // stream (if time/freq to stream) or free
    if (*dataStreamEnable && SendToStreamer()) {
        // if first frame to stream, add frame index to fifo header (might not
        // be the first)
        if (firstStreamerFrame) {
            firstStreamerFrame = false;
            (*((uint32_t *)(buffer + FIFO_DATASIZE_NUMBYTES))) =
                (uint32_t)(fnum - firstIndex);
        }
        fifo->PushAddressToStream(buffer);
    } else {
        fifo->FreeAddress(buffer);
    }
}

void DataProcessor::StopProcessing(char *buf) {
    LOG(logDEBUG1) << "DataProcessing " << index << ": Dummy";

    // stream or free
    if (*dataStreamEnable)
        fifo->PushAddressToStream(buf);
    else
        fifo->FreeAddress(buf);

    if (file != nullptr)
        file->CloseCurrentFile();
    StopRunning();
    LOG(logDEBUG1) << index << ": Processing Completed";
}

uint64_t DataProcessor::ProcessAnImage(char *buf) {

    auto *rheader = (sls_receiver_header *)(buf + FIFO_HEADER_NUMBYTES);
    sls_detector_header header = rheader->detHeader;
    uint64_t fnum = header.frameNumber;
    currentFrameIndex = fnum;
    uint32_t nump = header.packetNumber;
    if (nump == generalData->packetsPerFrame) {
        numFramesCaught++;
    }

    LOG(logDEBUG1) << "DataProcessing " << index << ": fnum:" << fnum;

    if (!startedFlag) {
        RecordFirstIndex(fnum);
        if (*dataStreamEnable) {
            // restart timer
            clock_gettime(CLOCK_REALTIME, &timerBegin);
            timerBegin.tv_sec -= (*streamingTimerInMs) / 1000;
            timerBegin.tv_nsec -= ((*streamingTimerInMs) % 1000) * 1000000;

            // to send first image
            currentFreqCount = *streamingFrequency - *streamingStartFnum;
        }
    }

    // frame padding
    if (*activated && *framePadding && nump < generalData->packetsPerFrame)
        PadMissingPackets(buf);

    // deactivated and padding enabled
    else if (!(*activated) && *deactivatedPaddingEnable)
        PadMissingPackets(buf);

    // rearrange ctb digital bits (if ctbDbitlist is not empty)
    if (!(*ctbDbitList).empty()) {
        RearrangeDbitData(buf);
    }

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

    // write to file
    if (file != nullptr) {
        try {
            file->WriteToFile(
                buf + FIFO_HEADER_NUMBYTES,
                sizeof(sls_receiver_header) +
                    (uint32_t)(*((uint32_t *)buf)), //+ size of data (resizable
                                                    // from previous call back
                fnum - firstIndex, nump);
        } catch (const sls::RuntimeError &e) {
            ; // ignore write exception for now (TODO: send error message via
              // stopReceiver tcp)
        }
    }
    return fnum;
}

bool DataProcessor::SendToStreamer() {
    // skip
    if ((*streamingFrequency) == 0u) {
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
                   << ((end.tv_sec - timerBegin.tv_sec) +
                       (end.tv_nsec - timerBegin.tv_nsec) / 1000000000.0)
                   << " seconds";
    // still less than streaming timer, keep waiting
    if (((end.tv_sec - timerBegin.tv_sec) +
         (end.tv_nsec - timerBegin.tv_nsec) / 1000000000.0) <
        ((double)*streamingTimerInMs / 1000.00))
        return false;

    // restart timer
    clock_gettime(CLOCK_REALTIME, &timerBegin);
    return true;
}

bool DataProcessor::CheckCount() {
    if (currentFreqCount == *streamingFrequency) {
        currentFreqCount = 1;
        return true;
    }
    currentFreqCount++;
    return false;
}

void DataProcessor::SetPixelDimension() {
    if (file != nullptr) {
        if (file->GetFileType() == HDF5) {
            file->SetNumberofPixels(generalData->nPixelsX,
                                    generalData->nPixelsY);
        }
    }
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

    uint32_t pperFrame = generalData->packetsPerFrame;
    auto *header = (sls_receiver_header *)(buf + FIFO_HEADER_NUMBYTES);
    uint32_t nmissing = pperFrame - header->detHeader.packetNumber;
    sls_bitset pmask = header->packetsMask;

    uint32_t dsize = generalData->dataSize;
    if (myDetectorType == GOTTHARD2 && index != 0) {
        dsize = generalData->vetoDataSize;
    }
    uint32_t fifohsize = generalData->fifoBufferHeaderSize;
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
        switch (myDetectorType) {
        // for gotthard, 1st packet: 4 bytes fnum, CACA                     +
        // CACA, 639*2 bytes data
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
        totalSize - (*ctbAnalogDataBytes) - (*ctbDbitOffset);

    // no digital data
    if (ctbDigitalDataBytes == 0) {
        LOG(logWARNING)
            << "No digital data for call back, yet dbitlist is not empty.";
        return;
    }

    const int numSamples = (ctbDigitalDataBytes / sizeof(uint64_t));
    const int digOffset = FIFO_HEADER_NUMBYTES + sizeof(sls_receiver_header) +
                          (*ctbAnalogDataBytes);

    // ceil as numResult8Bits could be decimal
    const int numResult8Bits =
        ceil((double)(numSamples * (*ctbDbitList).size()) / 8.00);
    std::vector<uint8_t> result(numResult8Bits);
    uint8_t *dest = &result[0];

    auto *source = (uint64_t *)(buf + digOffset + (*ctbDbitOffset));

    // loop through digital bit enable vector
    int bitoffset = 0;
    for (auto bi : (*ctbDbitList)) {
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
