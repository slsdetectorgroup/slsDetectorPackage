#pragma once
#include "sls/container_utils.h"
#include "sls/logger.h"
#include "sls/network_utils.h"
#include "receiver_defs.h"
class GeneralData;
class Listener;
class DataProcessor;
class DataStreamer;
class Fifo;
class slsDetectorDefs;

#include <atomic>
#include <chrono>
#include <exception>
#include <map>
#include <memory>
#include <vector>
using ns = std::chrono::nanoseconds;

class Implementation : private virtual slsDetectorDefs {
  public:
    explicit Implementation(const detectorType d);
    virtual ~Implementation();

    /**************************************************
     *                                                 *
     *   Configuration Parameters                      *
     *                                                 *
     * ************************************************/

    void setDetectorType(const detectorType d);
    int *getDetectorSize() const;
    void setDetectorSize(const int *size);
    int getModulePositionId() const;
    void setModulePositionId(const int id);
    std::string getDetectorHostname() const;
    void setDetectorHostname(const std::string &c);
    bool getSilentMode() const;
    void setSilentMode(const bool i);
    uint32_t getFifoDepth() const;
    void setFifoDepth(const uint32_t i);
    frameDiscardPolicy getFrameDiscardPolicy() const;
    void setFrameDiscardPolicy(const frameDiscardPolicy i);
    bool getFramePaddingEnable() const;
    void setFramePaddingEnable(const bool i);
    void setThreadIds(const pid_t parentTid, const pid_t tcpTid);
    std::array<pid_t, NUM_RX_THREAD_IDS> getThreadIds() const;

    /**************************************************
     *                                                 *
     *   File Parameters                               *
     *                                                 *
     * ************************************************/
    fileFormat getFileFormat() const;
    void setFileFormat(slsDetectorDefs::fileFormat f);
    std::string getFilePath() const;
    /* check for existence */
    void setFilePath(const std::string &c);
    std::string getFileName() const;
    void setFileName(const std::string &c);
    uint64_t getFileIndex() const;
    void setFileIndex(const uint64_t i);
    bool getFileWriteEnable() const;
    void setFileWriteEnable(const bool b);
    bool getMasterFileWriteEnable() const;
    void setMasterFileWriteEnable(const bool b);
    bool getOverwriteEnable() const;
    void setOverwriteEnable(const bool b);
    uint32_t getFramesPerFile() const;
    /* 0 means infinite */
    void setFramesPerFile(const uint32_t i);

    /**************************************************
     *                                                 *
     *   Acquisition                                   *
     *                                                 *
     * ************************************************/
    runStatus getStatus() const;
    uint64_t getFramesCaught() const;
    uint64_t getAcquisitionIndex() const;
    double getProgress() const;
    std::vector<uint64_t> getNumMissingPackets() const;
    void setScan(slsDetectorDefs::scanParameters s);
    void startReceiver();
    void setStoppedFlag(bool stopped);
    void stopReceiver();
    void startReadout();
    void shutDownUDPSockets();
    void closeFiles();
    void restreamStop();

    /**************************************************
     *                                                 *
     *   Network Configuration (UDP)                   *
     *                                                 *
     * ************************************************/
    int getNumberofUDPInterfaces() const;
    /* [Jungfrau] */
    void setNumberofUDPInterfaces(const int n);
    std::string getEthernetInterface() const;
    void setEthernetInterface(const std::string &c);
    std::string getEthernetInterface2() const;
    /* [Jungfrau] */
    void setEthernetInterface2(const std::string &c);
    uint32_t getUDPPortNumber() const;
    void setUDPPortNumber(const uint32_t i);
    uint32_t getUDPPortNumber2() const;
    /* [Eiger][Jungfrau] */
    void setUDPPortNumber2(const uint32_t i);
    int getUDPSocketBufferSize() const;
    void setUDPSocketBufferSize(const int s);
    int getActualUDPSocketBufferSize() const;

    /**************************************************
     *                                                 *
     *   ZMQ Streaming Parameters (ZMQ)                *
     *                                                 *
     * ************************************************/
    bool getDataStreamEnable() const;
    void setDataStreamEnable(const bool enable);
    uint32_t getStreamingFrequency() const;
    /* 0 for timer */
    void setStreamingFrequency(const uint32_t freq);
    uint32_t getStreamingTimer() const;
    void setStreamingTimer(const uint32_t time_in_ms);
    uint32_t getStreamingStartingFrameNumber() const;
    void setStreamingStartingFrameNumber(const uint32_t fnum);
    uint32_t getStreamingPort() const;
    void setStreamingPort(const uint32_t i);
    sls::IpAddr getStreamingSourceIP() const;
    void setStreamingSourceIP(const sls::IpAddr ip);
    int getStreamingHwm() const;
    void setStreamingHwm(const int i);
    std::map<std::string, std::string> getAdditionalJsonHeader() const;
    void setAdditionalJsonHeader(const std::map<std::string, std::string> &c);
    std::string getAdditionalJsonParameter(const std::string &key) const;
    void setAdditionalJsonParameter(const std::string &key,
                                    const std::string &value);

    /**************************************************
     *                                                 *
     *   Detector Parameters                           *
     *                                                 *
     * ************************************************/
    void updateTotalNumberOfFrames();
    uint64_t getNumberOfFrames() const;
    void setNumberOfFrames(const uint64_t i);
    uint64_t getNumberOfTriggers() const;
    void setNumberOfTriggers(const uint64_t i);
    uint64_t getNumberOfBursts() const;
    /** [Gottthard2] */
    void setNumberOfBursts(const uint64_t i);
    int getNumberOfAdditionalStorageCells() const;
    /** [Jungfrau] */
    void setNumberOfAdditionalStorageCells(const int i);
    /** [Mythen3] */
    void setNumberOfGates(const int i);
    timingMode getTimingMode() const;
    void setTimingMode(const timingMode i);
    burstMode getBurstMode() const;
    /** [Gottthard2] */
    void setBurstMode(const burstMode i);
    ns getAcquisitionTime() const;
    void setAcquisitionTime(const ns i);
    /** [Mythen3] */
    void updateAcquisitionTime();
    /** [Mythen3] */
    void setAcquisitionTime1(const ns i);
    /** [Mythen3] */
    void setAcquisitionTime2(const ns i);
    /** [Mythen3] */
    void setAcquisitionTime3(const ns i);
    /** [Mythen3] */
    void setGateDelay1(const ns i);
    /** [Mythen3] */
    void setGateDelay2(const ns i);
    /** [Mythen3] */
    void setGateDelay3(const ns i);
    ns getAcquisitionPeriod() const;
    void setAcquisitionPeriod(const ns i);
    ns getSubExpTime() const;
    /* [Eiger] */
    void setSubExpTime(const ns i);
    ns getSubPeriod() const;
    /* [Eiger] */
    void setSubPeriod(const ns i);
    uint32_t getNumberofAnalogSamples() const;
    /**[Ctb][Moench] */
    void setNumberofAnalogSamples(const uint32_t i);
    uint32_t getNumberofDigitalSamples() const;
    /**[Ctb] */
    void setNumberofDigitalSamples(const uint32_t i);
    uint32_t getCounterMask() const;
    /** [Mythen3] */
    void setCounterMask(const uint32_t i);
    uint32_t getDynamicRange() const;
    void setDynamicRange(const uint32_t i);
    ROI getROI() const;
    /* [Gotthard] */
    void setROI(ROI arg);
    bool getTenGigaEnable() const;
    /* [Eiger][Ctb] */
    void setTenGigaEnable(const bool b);
    int getFlippedDataX() const;
    void setFlippedDataX(int enable = -1);
    bool getQuad() const;
    /* [Eiger] */
    void setQuad(const bool b);
    bool getActivate() const;
    /** [Eiger] If deactivated, receiver will create dummy data if deactivated
     * padding is enabled (as it will receive nothing from detector) */
    bool setActivate(const bool enable);
    bool getDeactivatedPadding() const;
    /* [Eiger] */
    void setDeactivatedPadding(const bool enable);
    int getReadNLines() const;
    /* [Eiger] */
    void setReadNLines(const int value);
    /** [Eiger] */
    void setThresholdEnergy(const int value);
    /* [Eiger] */
    void setRateCorrections(const std::vector<int64_t> &t);
    readoutMode getReadoutMode() const;
    /* [Ctb] */
    void setReadoutMode(const readoutMode f);
    uint32_t getADCEnableMask() const;
    /* [Ctb][Moench] */
    void setADCEnableMask(const uint32_t mask);
    uint32_t getTenGigaADCEnableMask() const;
    /* [Ctb][Moench] */
    void setTenGigaADCEnableMask(const uint32_t mask);
    std::vector<int> getDbitList() const;
    /* [Ctb] */
    void setDbitList(const std::vector<int> &v);
    int getDbitOffset() const;
    /* [Ctb] */
    void setDbitOffset(const int s);

    /**************************************************
     *                                                *
     *    Callbacks                                   *
     *                                                *
     * ************************************************/
    void registerCallBackStartAcquisition(int (*func)(std::string, std::string,
                                                      uint64_t, uint32_t,
                                                      void *),
                                          void *arg);
    void registerCallBackAcquisitionFinished(void (*func)(uint64_t, void *),
                                             void *arg);
    void registerCallBackRawDataReady(void (*func)(char *, char *, uint32_t,
                                                   void *),
                                      void *arg);
    void registerCallBackRawDataModifyReady(void (*func)(char *, char *,
                                                         uint32_t &, void *),
                                            void *arg);

  private:
    void SetLocalNetworkParameters();
    void SetThreadPriorities();
    void SetupFifoStructure();

    void ResetParametersforNewAcquisition();
    void CreateUDPSockets();
    void SetupWriter();
    void StartRunning();

    /**************************************************
     *                                                *
     *    Class Members                               *
     *                                                *
     * ************************************************/

    // config parameters
    int numThreads{1};
    detectorType myDetectorType{GENERIC};
    int numDet[MAX_DIMENSIONS] = {0, 0};
    int modulePos{0};
    std::string detHostname;
    bool silentMode{false};
    uint32_t fifoDepth{0};
    frameDiscardPolicy frameDiscardMode{NO_DISCARD};
    bool framePadding{true};
    pid_t parentThreadId;
    pid_t tcpThreadId;

    // file parameters
    fileFormat fileFormatType{BINARY};
    std::string filePath{"/"};
    std::string fileName{"run"};
    uint64_t fileIndex{0};
    bool fileWriteEnable{true};
    bool masterFileWriteEnable{true};
    bool overwriteEnable{true};
    uint32_t framesPerFile{0};

    // acquisition
    std::atomic<runStatus> status{IDLE};
    bool stoppedFlag{false};
    scanParameters scanParams{};

    // network configuration (UDP)
    int numUDPInterfaces{1};
    std::array<std::string, MAX_NUMBER_OF_LISTENING_THREADS> eth;
    std::array<uint32_t, MAX_NUMBER_OF_LISTENING_THREADS> udpPortNum{
        {DEFAULT_UDP_PORTNO, DEFAULT_UDP_PORTNO + 1}};
    int udpSocketBufferSize{0};
    int actualUDPSocketBufferSize{0};

    // zmq parameters
    bool dataStreamEnable{false};
    uint32_t streamingFrequency{1};
    uint32_t streamingTimerInMs{DEFAULT_STREAMING_TIMER_IN_MS};
    uint32_t streamingStartFnum{0};
    uint32_t streamingPort{0};
    sls::IpAddr streamingSrcIP = sls::IpAddr{};
    int streamingHwm{-1};
    std::map<std::string, std::string> additionalJsonHeader;

    // detector parameters
    uint64_t numberOfTotalFrames{0};
    uint64_t numberOfFrames{1};
    uint64_t numberOfTriggers{1};
    uint64_t numberOfBursts{1};
    int numberOfAdditionalStorageCells{0};
    int numberOfGates{0};
    timingMode timingMode{AUTO_TIMING};
    burstMode burstMode{BURST_INTERNAL};
    ns acquisitionPeriod = std::chrono::nanoseconds(SAMPLE_TIME_IN_NS);
    ns acquisitionTime = std::chrono::nanoseconds(0);
    ns acquisitionTime1 = std::chrono::nanoseconds(0);
    ns acquisitionTime2 = std::chrono::nanoseconds(0);
    ns acquisitionTime3 = std::chrono::nanoseconds(0);
    ns gateDelay1 = std::chrono::nanoseconds(0);
    ns gateDelay2 = std::chrono::nanoseconds(0);
    ns gateDelay3 = std::chrono::nanoseconds(0);
    ns subExpTime = std::chrono::nanoseconds(0);
    ns subPeriod = std::chrono::nanoseconds(0);
    uint32_t numberOfAnalogSamples{0};
    uint32_t numberOfDigitalSamples{0};
    uint32_t counterMask{0};
    uint32_t dynamicRange{16};
    ROI roi{};
    bool tengigaEnable{false};
    int flippedDataX{0};
    bool quadEnable{false};
    bool activated{true};
    bool deactivatedPaddingEnable{true};
    int numLinesReadout{MAX_EIGER_ROWS_PER_READOUT};
    int thresholdEnergyeV{-1};
    std::vector<int64_t> rateCorrections;
    readoutMode readoutType{ANALOG_ONLY};
    uint32_t adcEnableMaskOneGiga{BIT32_MASK};
    uint32_t adcEnableMaskTenGiga{BIT32_MASK};
    std::vector<int> ctbDbitList;
    int ctbDbitOffset{0};
    int ctbAnalogDataBytes{0};

    // callbacks
    int (*startAcquisitionCallBack)(std::string, std::string, uint64_t,
                                    uint32_t, void *){nullptr};
    void *pStartAcquisition{nullptr};
    void (*acquisitionFinishedCallBack)(uint64_t, void *){nullptr};
    void *pAcquisitionFinished{nullptr};
    void (*rawDataReadyCallBack)(char *, char *, uint32_t, void *){nullptr};
    void (*rawDataModifyReadyCallBack)(char *, char *, uint32_t &,
                                       void *){nullptr};
    void *pRawDataReady{nullptr};

    // class objects
    GeneralData *generalData;
    std::vector<std::unique_ptr<Listener>> listener;
    std::vector<std::unique_ptr<DataProcessor>> dataProcessor;
    std::vector<std::unique_ptr<DataStreamer>> dataStreamer;
    std::vector<std::unique_ptr<Fifo>> fifo;
};
