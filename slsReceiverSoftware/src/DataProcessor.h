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
class MasterAttributes;

class DataProcessor : private virtual slsDetectorDefs, public ThreadObject {

  public:
    DataProcessor(int index);
    ~DataProcessor() override;

    bool GetStartedFlag() const;

    void SetFifo(Fifo *f);
    void SetGeneralData(GeneralData *generalData);

    void SetUdpPortNumber(const uint16_t portNumber);
    void SetActivate(bool enable);
    void SetReceiverROI(ROI roi);
    void SetDataStreamEnable(bool enable);
    void SetStreamingFrequency(uint32_t value);
    void SetStreamingTimerInMs(uint32_t value);
    void SetStreamingStartFnum(uint32_t value);
    void SetFramePadding(bool enable);
    void SetCtbDbitList(std::vector<int> value);
    void SetCtbDbitOffset(int value);
    void SetQuadEnable(bool value);
    void SetFlipRows(bool fd);
    void SetNumberofTotalFrames(uint64_t value);
    void
    SetAdditionalJsonHeader(const std::map<std::string, std::string> &json);

    void ResetParametersforNewAcquisition();
    void CloseFiles();
    void DeleteFiles();
    void SetupFileWriter(const bool filewriteEnable,
                         const fileFormat fileFormatType,
                         std::mutex *hdf5LibMutex);

    void CreateFirstFiles(const std::string &fileNamePrefix,
                          const uint64_t fileIndex, const bool overWriteEnable,
                          const bool silentMode, const bool detectorDataStream);
#ifdef HDF5C
    uint32_t GetFilesInAcquisition() const;
    std::string CreateVirtualFile(const std::string &filePath,
                                  const std::string &fileNamePrefix,
                                  const uint64_t fileIndex,
                                  const bool overWriteEnable,
                                  const bool silentMode, const int modulePos,
                                  const int numModX, const int numModY,
                                  std::mutex *hdf5LibMutex);
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

    /** params: sls_receiver_header, pointer to data, image size */
    void registerCallBackRawDataReady(void (*func)(sls_receiver_header &,
                                                   dataCallbackHeader, char *,
                                                   size_t &, void *),
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
    void ProcessAnImage(sls_receiver_header &header, size_t &size,
                        size_t &firstImageIndex, char *data);

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

    void PadMissingPackets(sls_receiver_header header, char *data);

    /**
     * Align corresponding digital bits together (CTB only if ctbDbitlist is not
     * empty)
     */
    void RearrangeDbitData(size_t &size, char *data);

    void CropImage(size_t &size, char *data);

    static const std::string typeName;

    GeneralData *generalData{nullptr};
    Fifo *fifo;

    uint16_t udpPortNumber{0};
    bool dataStreamEnable;
    bool activated{false};
    ROI receiverRoi{};
    bool receiverRoiEnabled{false};
    bool receiverNoRoi{false};
    std::unique_ptr<char[]> completeImageToStreamBeforeCropping;
    /** if 0, sending random images with a timer */
    uint32_t streamingFrequency;
    uint32_t streamingTimerInMs;
    uint32_t streamingStartFnum;
    uint32_t currentFreqCount{0};
    struct timespec timerbegin {};
    bool framePadding;
    std::vector<int> ctbDbitList;
    int ctbDbitOffset;
    std::atomic<bool> startedFlag{false};
    std::atomic<uint64_t> firstIndex{0};
    bool quadEnable{false};
    bool flipRows{false};
    uint64_t nTotalFrames{0};

    std::map<std::string, std::string> additionalJsonHeader;
    /** Used by streamer thread to update local copy (reduce number of locks
     * during streaming) */
    std::atomic<bool> isAdditionalJsonUpdated{false};
    /** mutex to update json and to read and update local copy */
    mutable std::mutex additionalJsonMutex;
    /** local copy of additional json header  (it can be update on the fly) */
    std::map<std::string, std::string> localAdditionalJsonHeader;

    // for statistics
    uint64_t numFramesCaught{0};

    /** Frame Number of latest processed frame number */
    std::atomic<uint64_t> currentFrameIndex{0};

    /** first streamer frame to add frame index in fifo header */
    bool firstStreamerFrame{false};

    bool streamCurrentFrame{false};

    File *dataFile{nullptr};

    // call back
    /**
     * Call back for raw data
     * args to raw data ready callback are
     * sls_receiver_header frame metadata
     * dataPointer is the pointer to the data
     * dataSize in bytes is the size of the data in bytes.
     */
    void (*rawDataReadyCallBack)(sls_receiver_header &, dataCallbackHeader,
                                 char *, size_t &, void *) = nullptr;

    void *pRawDataReady{nullptr};
};

} // namespace sls
