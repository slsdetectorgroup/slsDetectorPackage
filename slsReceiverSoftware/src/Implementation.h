// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "Arping.h"
#include "receiver_defs.h"
#include "sls/container_utils.h"
#include "sls/logger.h"
#include "sls/network_utils.h"
#include "sls/sls_detector_defs.h"

#include <atomic>
#include <chrono>
#include <exception>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

namespace sls {
using ns = std::chrono::nanoseconds;

class GeneralData;
class Listener;
class DataProcessor;
class DataStreamer;
class Fifo;

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
    xy getDetectorSize() const;
    void setDetectorSize(const xy size);
    int getModulePositionId() const;
    void setModulePositionId(const int id);
    void setRow(const int value);
    void setColumn(const int value);
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
    bool getArping() const;
    pid_t getArpingProcessId() const;
    void setArping(const bool i, const std::vector<std::string> ips);
    ROI getReceiverROI() const;
    void setReceiverROI(const ROI arg);
    void setReceiverROIMetadata(const ROI arg);

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
    std::vector<int64_t> getFramesCaught() const;
    std::vector<int64_t> getCurrentFrameIndex() const;
    double getProgress() const;
    std::vector<int64_t> getNumMissingPackets() const;
    void setScan(slsDetectorDefs::scanParameters s);
    void startReceiver();
    void setStoppedFlag(bool stopped);
    void stopReceiver();
    void startReadout();
    void shutDownUDPSockets();
    void restreamStop();

    /**************************************************
     *                                                 *
     *   Network Configuration (UDP)                   *
     *                                                 *
     * ************************************************/
    int getNumberofUDPInterfaces() const;
    /* [Jungfrau][Moench] */
    void setNumberofUDPInterfaces(const int n);
    std::string getEthernetInterface() const;
    void setEthernetInterface(const std::string &c);
    std::string getEthernetInterface2() const;
    /* [Jungfrau][Moench] */
    void setEthernetInterface2(const std::string &c);
    uint16_t getUDPPortNumber() const;
    void setUDPPortNumber(const uint16_t i);
    uint16_t getUDPPortNumber2() const;
    /* [Eiger][Jungfrau][Moench] */
    void setUDPPortNumber2(const uint16_t i);
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
    uint16_t getStreamingPort() const;
    void setStreamingPort(const uint16_t i);
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
    /**[Ctb] */
    void setNumberofAnalogSamples(const uint32_t i);
    uint32_t getNumberofDigitalSamples() const;
    /**[Ctb] */
    void setNumberofDigitalSamples(const uint32_t i);
    uint32_t getNumberofTransceiverSamples() const;
    /**[Ctb] */
    void setNumberofTransceiverSamples(const uint32_t i);
    uint32_t getCounterMask() const;
    /** [Mythen3] */
    void setCounterMask(const uint32_t i);
    uint32_t getDynamicRange() const;
    void setDynamicRange(const uint32_t i);
    ROI getROI() const;
    /* [Gotthard] */
    void setDetectorROI(ROI arg);
    bool getTenGigaEnable() const;
    /* [Eiger][Ctb] */
    void setTenGigaEnable(const bool b);
    bool getFlipRows() const;
    void setFlipRows(bool enable);
    bool getQuad() const;
    /* [Eiger] */
    void setQuad(const bool b);
    bool getActivate() const;
    /** [Eiger] If deactivated, receiver will create dummy data if deactivated
     * padding is enabled (as it will receive nothing from detector) */
    void setActivate(const bool enable);
    bool getDetectorDataStream(const portPosition port) const;
    /** [Eiger] If datastream is disabled, receiver will create dummy data if
     * deactivated
     * padding for that port is enabled (as it will receive nothing from
     * detector) */
    void setDetectorDataStream(const portPosition port, const bool enable);
    int getReadNRows() const;
    /* [Eiger][Jungfrau][Moench] */
    void setReadNRows(const int value);
    /** [Eiger] */
    void setThresholdEnergy(const int value);
    void setThresholdEnergy(const std::array<int, 3> value);
    /* [Eiger] */
    void setRateCorrections(const std::vector<int64_t> &t);
    readoutMode getReadoutMode() const;
    /* [Ctb] */
    void setReadoutMode(const readoutMode f);
    uint32_t getADCEnableMask() const;
    /* [Ctb] */
    void setADCEnableMask(const uint32_t mask);
    uint32_t getTenGigaADCEnableMask() const;
    /* [Ctb] */
    void setTenGigaADCEnableMask(const uint32_t mask);
    std::vector<int> getDbitList() const;
    /* [Ctb] */
    void setDbitList(const std::vector<int> &v);
    int getDbitOffset() const;
    /* [Ctb] */
    void setDbitOffset(const int s);
    uint32_t getTransceiverEnableMask() const;
    /* [Ctb] */
    void setTransceiverEnableMask(const uint32_t mask);

    /**************************************************
     *                                                *
     *    Callbacks                                   *
     *                                                *
     * ************************************************/
    /** params: file path, file name, file index, image size */
    void registerCallBackStartAcquisition(int (*func)(const startCallbackHeader,
                                                      void *),
                                          void *arg);
    /** params: total frames caught */
    void registerCallBackAcquisitionFinished(
        void (*func)(const endCallbackHeader, void *), void *arg);
    /** params: sls_receiver_header, pointer to data, image size */
    void registerCallBackRawDataReady(void (*func)(sls_receiver_header &,
                                                   dataCallbackHeader, char *,
                                                   size_t &, void *),
                                      void *arg);

  private:
    void SetLocalNetworkParameters();
    void SetThreadPriorities();
    void SetupFifoStructure();

    const xy GetPortGeometry() const;
    const ROI GetMaxROIPerPort() const;
    void ResetParametersforNewAcquisition();
    void CreateUDPSockets();
    void SetupWriter();
    void StartMasterWriter();
    void StartRunning();
    void SetupListener(int i);
    void SetupDataProcessor(int i);
    void SetupDataStreamer(int i);

    /**************************************************
     *                                                *
     *    Class Members                               *
     *                                                *
     * ************************************************/

    // config parameters
    xy numModules{1, 1};
    xy numPorts{1, 1};
    int modulePos{0};
    std::string detHostname;
    bool silentMode{false};
    bool framePadding{true};
    pid_t parentThreadId;
    pid_t tcpThreadId;
    ROI receiverRoi{};
    std::array<ROI, 2> portRois{};
    // receiver roi for complete detector for metadata
    ROI receiverRoiMetadata{};

    // file parameters
    fileFormat fileFormatType{BINARY};
    std::string filePath{"/"};
    std::string fileName{"run"};
    uint64_t fileIndex{0};
    bool fileWriteEnable{false};
    bool masterFileWriteEnable{true};
    bool overwriteEnable{true};

    // acquisition
    std::atomic<runStatus> status{IDLE};
    std::atomic<bool> stoppedFlag{false};
    scanParameters scanParams{};

    // network configuration (UDP)
    std::array<std::string, MAX_NUMBER_OF_LISTENING_THREADS> eth;
    std::array<uint16_t, MAX_NUMBER_OF_LISTENING_THREADS> udpPortNum{
        {DEFAULT_UDP_DST_PORTNO, DEFAULT_UDP_DST_PORTNO + 1}};
    int actualUDPSocketBufferSize{0};

    // zmq parameters
    bool dataStreamEnable{false};
    uint32_t streamingFrequency{1};
    uint32_t streamingTimerInMs{DEFAULT_STREAMING_TIMER_IN_MS};
    uint32_t streamingStartFnum{0};
    uint16_t streamingPort{0};
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
    bool flipRows{false};
    bool quadEnable{false};
    bool activated{true};
    std::array<bool, 2> detectorDataStream = {{true, true}};
    std::array<bool, 2> detectorDataStream10GbE = {{true, true}};
    int readNRows{0};
    int thresholdEnergyeV{-1};
    std::array<int, 3> thresholdAllEnergyeV = {{-1, -1, -1}};
    std::vector<int64_t> rateCorrections;
    std::vector<int> ctbDbitList;
    int ctbDbitOffset{0};

    // callbacks
    int (*startAcquisitionCallBack)(const startCallbackHeader, void *){nullptr};
    void *pStartAcquisition{nullptr};
    void (*acquisitionFinishedCallBack)(const endCallbackHeader,
                                        void *){nullptr};
    void *pAcquisitionFinished{nullptr};
    void (*rawDataReadyCallBack)(sls_receiver_header &, dataCallbackHeader,
                                 char *, size_t &, void *){nullptr};
    void *pRawDataReady{nullptr};

    // class objects
    GeneralData *generalData{nullptr};
    std::vector<std::unique_ptr<Listener>> listener;
    std::vector<std::unique_ptr<DataProcessor>> dataProcessor;
    std::vector<std::unique_ptr<DataStreamer>> dataStreamer;
    std::vector<std::unique_ptr<Fifo>> fifo;
    Arping arping;

    // mutex shared across all hdf5 virtual, master and data files
    std::mutex hdf5LibMutex;
};

} // namespace sls
