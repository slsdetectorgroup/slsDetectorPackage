// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
/************************************************
 * @file DataProcessor.h
 * @short creates data processor thread that
 * pulls pointers to memory addresses from fifos
 * and processes data stored in them & writes them to file
 ***********************************************/
/**
 *@short creates & manages a data processor thread each
 */

#include "ThreadObject.h"
#include "receiver_defs.h"

#include <atomic>
#include <mutex>
#include <vector>

namespace sls {

class GeneralData;
class Fifo;
class File;
class DataStreamer;
struct MasterAttributes;

class DataProcessor : private virtual slsDetectorDefs, public ThreadObject {

  public:
    DataProcessor(int index, detectorType detectorType, Fifo *fifo,
                  bool *dataStreamEnable, uint32_t *streamingFrequency,
                  uint32_t *streamingTimerInMs, uint32_t *streamingStartFnum,
                  bool *framePadding, std::vector<int> *ctbDbitList,
                  int *ctbDbitOffset, int *ctbAnalogDataBytes);

    ~DataProcessor() override;

    bool GetStartedFlag() const;

    void SetFifo(Fifo *f);
    void SetActivate(bool enable);
    void SetReceiverROI(ROI roi);
    void ResetParametersforNewAcquisition();
    void SetGeneralData(GeneralData *generalData);

    void CloseFiles();
    void DeleteFiles();
    void SetupFileWriter(const bool filewriteEnable,
                         const fileFormat fileFormatType,
                         std::mutex *hdf5LibMutex);

    void CreateFirstFiles(const std::string &filePath,
                          const std::string &fileNamePrefix,
                          const uint64_t fileIndex, const bool overWriteEnable,
                          const bool silentMode, const int modulePos,
                          const int numUnitsPerReadout,
                          const uint32_t udpPortNumber,
                          const uint32_t maxFramesPerFile,
                          const uint64_t numImages, const uint32_t dynamicRange,
                          const bool detectorDataStream);
#ifdef HDF5C
    uint32_t GetFilesInAcquisition() const;
    std::string CreateVirtualFile(
        const std::string &filePath, const std::string &fileNamePrefix,
        const uint64_t fileIndex, const bool overWriteEnable,
        const bool silentMode, const int modulePos,
        const int numUnitsPerReadout, const uint32_t maxFramesPerFile,
        const uint64_t numImages, const int numModX, const int numModY,
        const uint32_t dynamicRange, std::mutex *hdf5LibMutex);
    void LinkFileInMaster(const std::string &masterFileName,
                          const std::string &virtualFileName,
                          const bool silentMode, std::mutex *hdf5LibMutex);
#endif

    std::string CreateMasterFile(const std::string &filePath,
                                 const std::string &fileNamePrefix,
                                 const uint64_t fileIndex,
                                 const bool overWriteEnable, bool silentMode,
                                 const fileFormat fileFormatType,
                                 MasterAttributes *attr,
                                 std::mutex *hdf5LibMutex);

    /** params: sls_receiver_header pointer, pointer to data, image size */
    void registerCallBackRawDataReady(void (*func)(sls_receiver_header *,
                                                   char *, size_t, void *),
                                      void *arg);

    /** params: sls_receiver_header pointer, pointer to data, reference to image
     * size */
    void registerCallBackRawDataModifyReady(void (*func)(sls_receiver_header *,
                                                         char *, size_t &,
                                                         void *),
                                            void *arg);

  private:
    void RecordFirstIndex(uint64_t fnum);

    /**
     * Thread Exeution for DataProcessor Class
     * Pop bound addresses, process them,
     * write to file if needed & free the address
     */
    void ThreadExecution() override;

    /**
     * Frees dummy buffer,
     * reset running mask by calling StopRunning()
     */
    void StopProcessing(char *buf);

    /**
     * Process an image popped from fifo,
     * write to file if fw enabled & update parameters
     */
    void ProcessAnImage(char *buf);

    /**
     * Calls CheckTimer and CheckCount for streaming frequency and timer
     * and determines if the current image should be sent to streamer
     * @returns true if it should to streamer, else false
     */
    bool SendToStreamer();

    /**
     * This function should be called only in random frequency mode
     * Checks if timer is done and ready to send to stream
     * @returns true if ready to send to stream, else false
     */
    bool CheckTimer();

    /**
     * This function should be called only in non random frequency mode
     * Checks if count is done and ready to send to stream
     * @returns true if ready to send to stream, else false
     */
    bool CheckCount();

    void PadMissingPackets(char *buf);

    /**
     * Align corresponding digital bits together (CTB only if ctbDbitlist is not
     * empty)
     */
    void RearrangeDbitData(char *buf);

    void CropImage(char *buf);

    static const std::string typeName_;

    const GeneralData *generalData_{nullptr};
    Fifo *fifo_;
    detectorType detectorType_;
    bool *dataStreamEnable_;
    bool activated_{false};
    ROI receiverRoi_{};
    bool receiverRoiEnabled_{false};
    std::unique_ptr<char[]> completeImageToStreamBeforeCropping;
    /** if 0, sending random images with a timer */
    uint32_t *streamingFrequency_;
    uint32_t *streamingTimerInMs_;
    uint32_t *streamingStartFnum_;
    uint32_t currentFreqCount_{0};
    struct timespec timerbegin_ {};
    bool *framePadding_;
    std::vector<int> *ctbDbitList_;
    int *ctbDbitOffset_;
    int *ctbAnalogDataBytes_;
    std::atomic<bool> startedFlag_{false};
    std::atomic<uint64_t> firstIndex_{0};

    // for statistics
    uint64_t numFramesCaught_{0};

    /** Frame Number of latest processed frame number */
    std::atomic<uint64_t> currentFrameIndex_{0};

    /** first streamer frame to add frame index in fifo header */
    bool firstStreamerFrame_{false};

    bool streamCurrentFrame_{false};

    File *dataFile_{nullptr};

    // call back
    /**
     * Call back for raw data
     * args to raw data ready callback are
     * sls_receiver_header frame metadata
     * dataPointer is the pointer to the data
     * dataSize in bytes is the size of the data in bytes.
     */
    void (*rawDataReadyCallBack)(sls_receiver_header *, char *, size_t,
                                 void *) = nullptr;

    /**
     * Call back for raw data (modified)
     * args to raw data ready callback are
     * sls_receiver_header frame metadata
     * dataPointer is the pointer to the data
     * revDatasize is the reference of data size in bytes. Can be modified to
     * the new size to be written/streamed. (only smaller value).
     */
    void (*rawDataModifyReadyCallBack)(sls_receiver_header *, char *, size_t &,
                                       void *) = nullptr;

    void *pRawDataReady{nullptr};
};

} // namespace sls
