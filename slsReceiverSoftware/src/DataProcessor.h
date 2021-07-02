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

class GeneralData;
class Fifo;
class File;
class DataStreamer;
struct MasterAttributes;

#include <atomic>
#include <mutex>
#include <vector>

class DataProcessor : private virtual slsDetectorDefs, public ThreadObject {

  public:
    DataProcessor(int index, detectorType detectorType, Fifo *fifo,
                  bool *activated, bool *deactivatedPaddingEnable,
                  bool *dataStreamEnable, uint32_t *streamingFrequency,
                  uint32_t *streamingTimerInMs, uint32_t *streamingStartFnum,
                  bool *framePadding, std::vector<int> *ctbDbitList,
                  int *ctbDbitOffset, int *ctbAnalogDataBytes,
                  std::mutex *hdf5Lib);

    ~DataProcessor() override;

    bool GetStartedFlag();
    uint64_t GetNumFramesCaught();
    /** (-1 if no frames have been caught */
    uint64_t GetCurrentFrameIndex();
    /** (-1 if no frames have been caught) */
    uint64_t GetProcessedIndex();

    void SetFifo(Fifo *f);
    void ResetParametersforNewAcquisition();
    void SetGeneralData(GeneralData *generalData);

    void CloseFiles();
    void DeleteFiles();
    void SetupFileWriter(const bool filewriteEnable,
                         const bool masterFilewriteEnable,
                         const fileFormat fileFormatType, const int modulePos);

    void CreateFirstFiles(MasterAttributes *attr, const std::string filePath,
                          const std::string fileNamePrefix,
                          const uint64_t fileIndex, const bool overWriteEnable,
                          const bool silentMode, const int modulePos,
                          const int numUnitsPerReadout,
                          const uint32_t udpPortNumber,
                          const uint32_t maxFramesPerFile,
                          const uint64_t numImages,
                          const uint32_t dynamicRange);
#ifdef HDF5C
    void CreateVirtualFile(const std::string filePath,
                           const std::string fileNamePrefix,
                           const uint64_t fileIndex, const bool overWriteEnable,
                           const bool silentMode, const int modulePos,
                           const int numUnitsPerReadout,
                           const uint32_t maxFramesPerFile,
                           const uint64_t numImages,
                           const uint32_t dynamicRange, const int numModX,
                           const int numModY);
#endif
    /**
     * Call back for raw data
     * args to raw data ready callback are
     * sls_receiver_header frame metadata
     * dataPointer is the pointer to the data
     * dataSize in bytes is the size of the data in bytes.
     */
    void registerCallBackRawDataReady(void (*func)(char *, char *, uint32_t,
                                                   void *),
                                      void *arg);

    /**
     * Call back for raw data (modified)
     * args to raw data ready callback are
     * sls_receiver_header frame metadata
     * dataPointer is the pointer to the data
     * revDatasize is the reference of data size in bytes.
     * Can be modified to the new size to be written/streamed. (only smaller
     * value).
     */
    void registerCallBackRawDataModifyReady(void (*func)(char *, char *,
                                                         uint32_t &, void *),
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
     * @returns frame number
     */
    uint64_t ProcessAnImage(char *buf);

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

    static const std::string typeName_;

    const GeneralData *generalData_{nullptr};
    Fifo *fifo_;
    detectorType detectorType_;
    bool *dataStreamEnable_;
    bool *activated_;
    bool *deactivatedPaddingEnable_;
    /** if 0, sending random images with a timer */
    uint32_t *streamingFrequency_;
    uint32_t *streamingTimerInMs_;
    uint32_t *streamingStartFnum_;
    uint32_t currentFreqCount_{0};
    struct timespec timerbegin_;
    bool *framePadding_;
    std::vector<int> *ctbDbitList_;
    int *ctbDbitOffset_;
    int *ctbAnalogDataBytes_;
    std::atomic<bool> startedFlag_{false};
    std::atomic<uint64_t> firstIndex_{0};

    // for statistics
    /** Number of complete frames caught */
    uint64_t numFramesCaught_{0};

    /** Frame Number of latest processed frame number */
    std::atomic<uint64_t> currentFrameIndex_{0};

    /** first streamer frame to add frame index in fifo header */
    bool firstStreamerFrame_{false};

    File *dataFile_{nullptr};
    File *masterFile_{nullptr};
    std::mutex *hdf5Lib_;
#ifdef HDF5C
    File *virtualFile_{nullptr};
#endif

    // call back
    /**
     * Call back for raw data
     * args to raw data ready callback are
     * sls_receiver_header frame metadata
     * dataPointer is the pointer to the data
     * dataSize in bytes is the size of the data in bytes.
     */
    void (*rawDataReadyCallBack)(char *, char *, uint32_t, void *) = nullptr;

    /**
     * Call back for raw data (modified)
     * args to raw data ready callback are
     * sls_receiver_header frame metadata
     * dataPointer is the pointer to the data
     * revDatasize is the reference of data size in bytes. Can be modified to
     * the new size to be written/streamed. (only smaller value).
     */
    void (*rawDataModifyReadyCallBack)(char *, char *, uint32_t &,
                                       void *) = nullptr;

    void *pRawDataReady{nullptr};
};
