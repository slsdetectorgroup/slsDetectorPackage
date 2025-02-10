// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "SharedMemory.h"
#include "sls/ClientSocket.h"
#include "sls/Pattern.h"
#include "sls/StaticVector.h"
#include "sls/logger.h"
#include "sls/network_utils.h"
#include "sls/sls_detector_defs.h"

#include <array>
#include <cmath>
#include <map>
#include <vector>

namespace sls {

class ServerInterface;

#define MODULE_SHMAPIVERSION 0x190726
#define MODULE_SHMVERSION    0x230913

/**
 * @short structure allocated in shared memory to store Module settings for
 * IPC and cache
 */
struct sharedModule {

    /* FIXED PATTERN FOR STATIC FUNCTIONS. DO NOT CHANGE, ONLY APPEND ------*/

    int shmversion;
    char hostname[MAX_STR_LENGTH];
    slsDetectorDefs::detectorType detType;

    /** END OF FIXED PATTERN -----------------------------------------------*/

    slsDetectorDefs::xy numberOfModule;
    uint16_t controlPort;
    uint16_t stopPort;
    char settingsDir[MAX_STR_LENGTH];
    /** list of the energies at which the Module has been trimmed  */
    StaticVector<int, MAX_TRIMEN> trimEnergies;
    /**  number of channels per chip */
    slsDetectorDefs::xy nChan;
    slsDetectorDefs::xy nChip;
    int nDacs;
    char rxHostname[MAX_STR_LENGTH];
    uint16_t rxTCPPort;
    /** if rxHostname and rxTCPPort can be connected to */
    bool useReceiverFlag;
    /** Listening tcp port from gui (only data) */
    uint16_t zmqport;
    /**  Listening tcp ip address from gui (only data) **/
    IpAddr zmqip;
    int numUDPInterfaces;
    /** to inform rxr when stopping rxr */
    bool stoppedFlag;
};

class Module : public virtual slsDetectorDefs {
  public:
    /**************************************************
     *                                                *
     *    Configuration                               *
     *                                                *
     * ************************************************/

    /** creating new shared memory
    verify is if shared memory version matches existing one */
    explicit Module(detectorType type, int det_id = 0, int module_index = 0,
                    bool verify = true);

    /** opening existing shared memory
    verify is if shared memory version matches existing one */
    explicit Module(int det_id = 0, int module_index = 0, bool verify = true);

    bool isFixedPatternSharedMemoryCompatible() const;
    std::string getHostname() const;

    /** initialChecks is enable or disable initial compatibility checks and
    other server start up checks. Enabled by default. Disable only for advanced
    users! */
    void setHostname(const std::string &hostname, const bool initialChecks);

    int64_t getFirmwareVersion() const;
    int64_t getFrontEndFirmwareVersion(const fpgaPosition fpgaPosition) const;
    std::string getControlServerLongVersion() const;
    std::string getStopServerLongVersion() const;
    void throwDeprecatedServerVersion() const;
    std::string getDetectorServerVersion() const;
    std::string getHardwareVersion() const;
    std::string getKernelVersion() const;
    int64_t getSerialNumber() const;
    int getModuleId() const;
    std::string getReceiverSoftwareVersion() const;
    static detectorType
    getTypeFromDetector(const std::string &hostname,
                        uint16_t cport = DEFAULT_TCP_CNTRL_PORTNO);

    /** Get Detector type from shared memory */
    detectorType getDetectorType() const;
    void updateNumberOfChannels();
    slsDetectorDefs::xy getNumberOfChannels() const;
    void updateNumberOfModule(slsDetectorDefs::xy det);
    detectorSettings getSettings() const;
    void setSettings(detectorSettings isettings);
    int getThresholdEnergy() const;
    std::array<int, 3> getAllThresholdEnergy() const;
    void setThresholdEnergy(int e_eV, detectorSettings isettings,
                            bool trimbits);
    void setAllThresholdEnergy(std::array<int, 3> e_eV,
                               detectorSettings isettings, bool trimbits);
    std::string getSettingsDir() const;
    std::string setSettingsDir(const std::string &dir);
    void loadTrimbits(const std::string &fname);
    void saveTrimbits(const std::string &fname);
    int getAllTrimbits() const;
    void setAllTrimbits(int val);
    std::vector<int> getTrimEn() const;
    int setTrimEn(const std::vector<int> &energies = {});
    bool getFlipRows() const;
    void setFlipRows(bool value);
    bool isMaster() const;
    void setMaster(const bool master);
    bool getSynchronization() const;
    bool getSynchronizationFromStopServer() const;
    void setSynchronization(const bool value);
    std::vector<int> getBadChannels() const;
    void setBadChannels(std::vector<int> list);
    int getRow() const;
    void setRow(const int value);
    int getColumn() const;
    void setColumn(const int value);

    bool isVirtualDetectorServer() const;

    /**************************************************
     *                                                *
     *    Acquisition Parameters                      *
     *                                                *
     * ************************************************/
    int64_t getNumberOfFrames() const;
    void setNumberOfFrames(int64_t value);
    int64_t getNumberOfTriggers() const;
    void setNumberOfTriggers(int64_t value);
    /** [Mythen3] gatIndex: 0-2, [Others]: -1 always */
    int64_t getExptime(int gateIndex) const;
    /** [Mythen3] gatIndex: -1 for all, 0-2, [Others]: -1 always */
    void setExptime(int gateIndex, int64_t value);
    int64_t getPeriod() const;
    void setPeriod(int64_t value);
    int64_t getDelayAfterTrigger() const;
    void setDelayAfterTrigger(int64_t value);
    int64_t getNumberOfFramesLeft() const;
    int64_t getNumberOfTriggersLeft() const;
    int64_t getDelayAfterTriggerLeft() const;
    int64_t getPeriodLeft() const;
    int getDynamicRange() const;
    void setDynamicRange(int dr);
    timingMode getTimingMode() const;
    void setTimingMode(timingMode value);
    speedLevel getReadoutSpeed() const;
    void setReadoutSpeed(speedLevel value);
    int getClockDivider(int clkIndex) const;
    void setClockDivider(int clkIndex, int value);
    int getClockPhase(int clkIndex, bool inDegrees) const;
    void setClockPhase(int clkIndex, int value, bool inDegrees);
    int getMaxClockPhaseShift(int clkIndex) const;
    int getClockFrequency(int clkIndex) const;
    void setClockFrequency(int clkIndex, int value);
    int getDefaultDac(slsDetectorDefs::dacIndex index,
                      slsDetectorDefs::detectorSettings sett);
    void setDefaultDac(slsDetectorDefs::dacIndex index, int defaultValue,
                       defs::detectorSettings sett);
    void resetToDefaultDacs(const bool hardReset);
    int getDAC(dacIndex index, bool mV) const;
    void setDAC(int val, dacIndex index, bool mV);
    bool getPowerChip() const;
    void setPowerChip(bool on);
    int getImageTestMode() const;
    void setImageTestMode(const int value);
    /* temperature in millidegrees */
    int getADC(dacIndex index) const;
    int getOnChipDAC(slsDetectorDefs::dacIndex index, int chipIndex) const;
    void setOnChipDAC(slsDetectorDefs::dacIndex index, int chipIndex,
                      int value);
    externalSignalFlag getExternalSignalFlags(int signalIndex) const;
    void setExternalSignalFlags(int signalIndex, externalSignalFlag type);
    bool getParallelMode() const;
    void setParallelMode(const bool enable);
    int getFilterResistor() const;
    void setFilterResistor(int value);
    defs::currentSrcParameters getCurrentSource() const;
    void setCurrentSource(defs::currentSrcParameters par);
    int getDBITPipeline() const;
    void setDBITPipeline(int value);
    int getReadNRows() const;
    void setReadNRows(const int value);

    /**************************************************
     *                                                *
     *    Acquisition                                 *
     *                                                *
     * ************************************************/
    void startReceiver();
    void stopReceiver();
    void startAcquisition();
    void startReadout();
    void stopAcquisition();
    void restreamStopFromReceiver();
    void startAndReadAll();
    runStatus getRunStatus() const;
    runStatus getReceiverStatus() const;
    double getReceiverProgress() const;
    std::vector<int64_t> getFramesCaughtByReceiver() const;
    std::vector<int64_t> getNumMissingPackets() const;
    std::vector<int64_t> getReceiverCurrentFrameIndex() const;
    uint64_t getNextFrameNumber() const;
    void setNextFrameNumber(uint64_t value);
    void sendSoftwareTrigger(const bool block);
    defs::scanParameters getScan() const;
    void setScan(const defs::scanParameters t);
    std::string getScanErrorMessage() const;

    /**************************************************
     *                                                 *
     *    Network Configuration (Detector<->Receiver)  *
     *                                                 *
     * ************************************************/
    int getNumberofUDPInterfacesFromShm() const;
    void updateNumberofUDPInterfaces();
    void setNumberofUDPInterfaces(int n);
    int getSelectedUDPInterface() const;
    void selectUDPInterface(int n);
    IpAddr getSourceUDPIP() const;
    void setSourceUDPIP(const IpAddr ip);
    IpAddr getSourceUDPIP2() const;
    void setSourceUDPIP2(const IpAddr ip);
    MacAddr getSourceUDPMAC() const;
    void setSourceUDPMAC(const MacAddr mac);
    MacAddr getSourceUDPMAC2() const;
    void setSourceUDPMAC2(const MacAddr mac);
    UdpDestination getDestinationUDPList(const uint32_t entry) const;
    void setDestinationUDPList(const UdpDestination dest);
    int getNumberofUDPDestinations() const;
    void clearUDPDestinations();
    int getFirstUDPDestination() const;
    void setFirstUDPDestination(const int value);
    IpAddr getDestinationUDPIP() const;
    void setDestinationUDPIP(const IpAddr ip);
    IpAddr getDestinationUDPIP2() const;
    void setDestinationUDPIP2(const IpAddr ip);
    MacAddr getDestinationUDPMAC() const;
    void setDestinationUDPMAC(const MacAddr mac);
    MacAddr getDestinationUDPMAC2() const;
    void setDestinationUDPMAC2(const MacAddr mac);
    uint16_t getDestinationUDPPort() const;
    void setDestinationUDPPort(uint16_t udpport);
    uint16_t getDestinationUDPPort2() const;
    void setDestinationUDPPort2(uint16_t udpport);
    void reconfigureUDPDestination();
    void validateUDPConfiguration();
    std::string printReceiverConfiguration();
    bool getTenGiga() const;
    void setTenGiga(bool value);
    bool getTenGigaFlowControl() const;
    void setTenGigaFlowControl(bool enable);
    int getTransmissionDelayFrame() const;
    void setTransmissionDelayFrame(int value);
    int getTransmissionDelayLeft() const;
    void setTransmissionDelayLeft(int value);
    int getTransmissionDelayRight() const;
    void setTransmissionDelayRight(int value);

    /**************************************************
     *                                                *
     *    Receiver Config                             *
     *                                                *
     * ************************************************/
    bool getUseReceiverFlag() const;
    std::string getReceiverHostname() const;
    void setReceiverHostname(const std::string &hostname, const uint16_t port,
                             const bool initialChecks);
    uint16_t getReceiverPort() const;
    void setReceiverPort(uint16_t port_number);
    int getReceiverFifoDepth() const;
    void setReceiverFifoDepth(int n_frames);
    bool getReceiverSilentMode() const;
    void setReceiverSilentMode(bool enable);
    frameDiscardPolicy getReceiverFramesDiscardPolicy() const;
    void setReceiverFramesDiscardPolicy(frameDiscardPolicy f);
    bool getPartialFramesPadding() const;
    void setPartialFramesPadding(bool padding);
    int getReceiverUDPSocketBufferSize() const;
    int getReceiverRealUDPSocketBufferSize() const;
    void setReceiverUDPSocketBufferSize(int udpsockbufsize);
    bool getReceiverLock() const;
    void setReceiverLock(bool lock);
    IpAddr getReceiverLastClientIP() const;
    std::array<pid_t, NUM_RX_THREAD_IDS> getReceiverThreadIds() const;
    bool getRxArping() const;
    void setRxArping(bool enable);
    defs::ROI getRxROI() const;
    void setRxROI(const slsDetectorDefs::ROI arg);
    void setRxROIMetadata(const slsDetectorDefs::ROI arg);

    /**************************************************
     *                                                *
     *    File                                        *
     *                                                *
     * ************************************************/
    fileFormat getFileFormat() const;
    void setFileFormat(fileFormat f);
    std::string getFilePath() const;
    void setFilePath(const std::string &path);
    std::string getFileName() const;
    void setFileName(const std::string &fname);
    int64_t getFileIndex() const;
    void setFileIndex(int64_t file_index);
    void incrementFileIndex();
    bool getFileWrite() const;
    void setFileWrite(bool value);
    bool getMasterFileWrite() const;
    void setMasterFileWrite(bool value);
    bool getFileOverWrite() const;
    void setFileOverWrite(bool value);
    int getFramesPerFile() const;
    /** 0 will set frames per file to unlimited */
    void setFramesPerFile(int n_frames);

    /**************************************************
     *                                                *
     *    ZMQ Streaming Parameters (Receiver<->Client)*
     *                                                *
     * ************************************************/
    bool getReceiverStreaming() const;
    void setReceiverStreaming(bool enable);
    int getReceiverStreamingFrequency() const;
    /** Option: nth frame streamed out, if 0, streamed out at a timer of 200 */
    void setReceiverStreamingFrequency(int freq);
    int getReceiverStreamingTimer() const;
    void setReceiverStreamingTimer(int time_in_ms = 200);
    int getReceiverStreamingStartingFrame() const;
    void setReceiverStreamingStartingFrame(int fnum);
    uint16_t getReceiverStreamingPort() const;
    void setReceiverStreamingPort(uint16_t port);
    uint16_t getClientStreamingPort() const;
    void setClientStreamingPort(uint16_t port);
    IpAddr getClientStreamingIP() const;
    void setClientStreamingIP(const IpAddr ip);
    int getReceiverStreamingHwm() const;
    void setReceiverStreamingHwm(const int limit);

    /**************************************************
     *                                                *
     *    Eiger Specific                              *
     *                                                *
     * ************************************************/
    int64_t getSubExptime() const;
    void setSubExptime(int64_t value);
    int64_t getSubDeadTime() const;
    void setSubDeadTime(int64_t value);
    bool getOverFlowMode() const;
    void setOverFlowMode(const bool enable);
    int64_t getRateCorrection() const;
    void setDefaultRateCorrection();
    void setRateCorrection(int64_t t = 0);
    void sendReceiverRateCorrections(const std::vector<int64_t> &t);
    bool getInterruptSubframe() const;
    void setInterruptSubframe(const bool enable);
    int64_t getMeasuredPeriod() const;
    int64_t getMeasuredSubFramePeriod() const;
    bool getActivate() const;
    void setActivate(const bool enable);
    bool getCounterBit() const;
    void setCounterBit(bool cb);
    void pulsePixel(int n = 0, int x = 0, int y = 0);
    void pulsePixelNMove(int n = 0, int x = 0, int y = 0);
    void pulseChip(int n_pulses = 0);
    bool getQuad() const;
    void setQuad(const bool enable);
    bool getDataStream(const portPosition port) const;
    void setDataStream(const portPosition port, const bool enable);
    bool getTop() const;
    void setTop(bool value);

    /**************************************************
     *                                                *
     *    Jungfrau/Moench Specific                    *
     *                                                *
     * ************************************************/
    double getChipVersion() const;
    int getThresholdTemperature() const;
    void setThresholdTemperature(int val);
    bool getTemperatureControl() const;
    void setTemperatureControl(bool val);
    int getTemperatureEvent() const;
    void resetTemperatureEvent();
    bool getAutoComparatorDisableMode() const;
    void setAutoComparatorDisableMode(bool val);
    int64_t getComparatorDisableTime() const;
    void setComparatorDisableTime(int64_t value);
    int getNumberOfAdditionalStorageCells() const;
    void setNumberOfAdditionalStorageCells(int value);
    int getStorageCellStart() const;
    void setStorageCellStart(int pos);
    int64_t getStorageCellDelay() const;
    void setStorageCellDelay(int64_t value);
    gainMode getGainMode() const;
    void setGainMode(const gainMode mode);
    int getNumberOfFilterCells() const;
    void setNumberOfFilterCells(int value);
    defs::pedestalParameters getPedestalMode() const;
    void setPedestalMode(defs::pedestalParameters par);
    defs::timingInfoDecoder getTimingInfoDecoder() const;
    void setTimingInfoDecoder(const defs::timingInfoDecoder enable);
    defs::collectionMode getCollectionMode() const;
    void setCollectionMode(const defs::collectionMode enable);

    /**************************************************
     *                                                *
     *    Gotthard Specific                           *
     *                                                *
     * ************************************************/
    slsDetectorDefs::ROI getROI() const;
    void setROI(slsDetectorDefs::ROI arg);
    void clearROI();
    int64_t getExptimeLeft() const;

    /**************************************************
     *                                                *
     *    Gotthard2 Specific                          *
     *                                                *
     * ************************************************/
    int64_t getNumberOfBursts() const;
    void setNumberOfBursts(int64_t value);
    int64_t getBurstPeriod() const;
    void setBurstPeriod(int64_t value);
    int64_t getNumberOfBurstsLeft() const;
    std::array<int, 2> getInjectChannel() const;
    void setInjectChannel(const int offsetChannel, const int incrementChannel);
    void sendVetoPhoton(const int chipIndex,
                        const std::vector<int> &gainIndices,
                        const std::vector<int> &values);
    void getVetoPhoton(const int chipIndex, const std::string &fname) const;
    void setVetoPhoton(const int chipIndex, const int numPhotons,
                       const int energy, const std::string &fname);
    void setVetoReference(const int gainIndex, const int value);
    void setVetoFile(const int chipIndex, const std::string &fname);
    burstMode getBurstMode() const;
    void setBurstMode(burstMode value);
    bool getCDSGain() const;
    void setCDSGain(bool value);
    slsDetectorDefs::timingSourceType getTimingSource() const;
    void setTimingSource(slsDetectorDefs::timingSourceType value);
    bool getVeto() const;
    void setVeto(bool enable);
    bool getVetoStream() const;
    void setVetoStream(const bool value);
    slsDetectorDefs::vetoAlgorithm
    getVetoAlgorithm(const slsDetectorDefs::streamingInterface interface) const;
    void setVetoAlgorithm(const slsDetectorDefs::vetoAlgorithm alg,
                          const slsDetectorDefs::streamingInterface interface);
    int getADCConfiguration(const int chipIndex, const int adcIndex) const;
    void setADCConfiguration(const int chipIndex, const int adcIndex,
                             int value);

    /**************************************************
     *                                                *
     *    Mythen3 Specific                            *
     *                                                *
     * ************************************************/
    uint32_t getCounterMask() const;
    void setCounterMask(uint32_t countermask);
    int getNumberOfGates() const;
    void setNumberOfGates(int value);
    std::array<time::ns, 3> getExptimeForAllGates() const;
    int64_t getGateDelay(int gateIndex) const;
    void setGateDelay(int gateIndex, int64_t value);
    std::array<time::ns, 3> getGateDelayForAllGates() const;
    int getChipStatusRegister() const;
    void setGainCaps(int caps);
    int getGainCaps();
    defs::polarity getPolarity() const;
    void setPolarity(const defs::polarity enable);
    bool getInterpolation() const;
    void setInterpolation(const bool enable);
    bool getPumpProbe() const;
    void setPumpProbe(const bool enable);
    bool getAnalogPulsing() const;
    void setAnalogPulsing(const bool enable);
    bool getDigitalPulsing() const;
    void setDigitalPulsing(const bool enable);

    /**************************************************
     *                                                *
     *    CTB  Specific                               *
     *                                                *
     * ************************************************/
    int getNumberOfAnalogSamples() const;
    void setNumberOfAnalogSamples(int value);
    uint32_t getADCEnableMask() const;
    void setADCEnableMask(uint32_t mask);
    uint32_t getTenGigaADCEnableMask() const;
    void setTenGigaADCEnableMask(uint32_t mask);
    uint32_t getTransceiverEnableMask() const;
    void setTransceiverEnableMask(uint32_t mask);
    int getNumberOfDigitalSamples() const;
    void setNumberOfDigitalSamples(int value);
    int getNumberOfTransceiverSamples() const;
    void setNumberOfTransceiverSamples(int value);
    readoutMode getReadoutMode() const;
    void setReadoutMode(const readoutMode mode);
    int getExternalSamplingSource();
    int setExternalSamplingSource(int value);
    bool getExternalSampling() const;
    void setExternalSampling(bool value);
    std::vector<int> getReceiverDbitList() const;
    void setReceiverDbitList(std::vector<int> list);
    int getReceiverDbitOffset() const;
    void setReceiverDbitOffset(int value);
    void setDigitalIODelay(uint64_t pinMask, int delay);
    bool getLEDEnable() const;
    void setLEDEnable(bool enable);

    /**************************************************
     *                                                *
     *    Xilinx Ctb Specific                         *
     *                                                *
     * ************************************************/
    void configureTransceiver();

    /**************************************************
     *                                                *
     *    Pattern                                     *
     *                                                *
     * ************************************************/
    std::string getPatterFileName() const;
    void setPattern(const Pattern &pat, const std::string &fname);
    Pattern getPattern();
    void loadDefaultPattern();
    uint64_t getPatternIOControl() const;
    void setPatternIOControl(uint64_t word);
    uint64_t getPatternWord(int addr) const;
    void setPatternWord(int addr, uint64_t word);
    std::array<int, 2> getPatternLoopAddresses(int level) const;
    void setPatternLoopAddresses(int level, int start, int stop);
    int getPatternLoopCycles(int level) const;
    void setPatternLoopCycles(int level, int n);
    int getPatternWaitAddr(int level) const;
    void setPatternWaitAddr(int level, int addr);
    uint64_t getPatternWaitTime(int level) const;
    void setPatternWaitTime(int level, uint64_t t);
    uint64_t getPatternMask() const;
    void setPatternMask(uint64_t mask);
    uint64_t getPatternBitMask() const;
    void setPatternBitMask(uint64_t mask);
    void startPattern();

    /**************************************************
     *                                                *
     *    Json Header specific                        *
     *                                                *
     * ************************************************/
    std::map<std::string, std::string> getAdditionalJsonHeader() const;
    void setAdditionalJsonHeader(
        const std::map<std::string, std::string> &jsonHeader);
    std::string getAdditionalJsonParameter(const std::string &key) const;
    void setAdditionalJsonParameter(const std::string &key,
                                    const std::string &value);

    /**************************************************
     *                                                *
     *    Advanced                                    *
     *                                                *
     * ************************************************/
    int getADCPipeline() const;
    void setADCPipeline(int value);
    void programFPGA(std::vector<char> buffer,
                     const bool forceDeleteNormalFile);
    void resetFPGA();
    void updateDetectorServer(std::vector<char> buffer,
                              const std::string &serverName);
    void updateKernel(std::vector<char> buffer);
    void rebootController();
    bool getUpdateMode() const;
    void setUpdateMode(const bool updatemode);
    uint32_t readRegister(uint32_t addr) const;
    void writeRegister(uint32_t addr, uint32_t val, bool validate);
    void setBit(uint32_t addr, int n, bool validate);
    void clearBit(uint32_t addr, int n, bool validate);
    int getBit(uint32_t addr, int n);
    void executeFirmwareTest();
    void executeBusTest();
    void writeAdcRegister(uint32_t addr, uint32_t val);
    uint32_t getADCInvert() const;
    void setADCInvert(uint32_t value);

    /**************************************************
     *                                                *
     *    Insignificant                               *
     *                                                *
     * ************************************************/
    uint16_t getControlPort() const;
    void setControlPort(uint16_t port_number);
    uint16_t getStopPort() const;
    void setStopPort(uint16_t port_number);
    bool getLockDetector() const;
    void setLockDetector(bool lock);
    IpAddr getLastClientIP() const;
    std::string executeCommand(const std::string &cmd);
    int64_t getNumberOfFramesFromStart() const;
    int64_t getActualTime() const;
    int64_t getMeasurementTime() const;

  private:
    std::string getReceiverLongVersion() const;

    void checkArgs(const void *args, size_t args_size, void *retval,
                   size_t retval_size) const;

    /**
     * Send function parameters to detector (control server)
     * @param fnum function enum
     * @param args argument pointer
     * @param args_size size of argument
     * @param retval return pointers
     * @param retval_size size of return value
     */
    void sendToDetector(int fnum, const void *args, size_t args_size,
                        void *retval, size_t retval_size);

    void sendToDetector(int fnum, const void *args, size_t args_size,
                        void *retval, size_t retval_size) const;

    template <typename Arg, typename Ret>
    void sendToDetector(int fnum, const Arg &args, Ret &retval);

    template <typename Arg, typename Ret>
    void sendToDetector(int fnum, const Arg &args, Ret &retval) const;

    template <typename Arg>
    void sendToDetector(int fnum, const Arg &args, std::nullptr_t);

    template <typename Arg>
    void sendToDetector(int fnum, const Arg &args, std::nullptr_t) const;

    template <typename Ret>
    void sendToDetector(int fnum, std::nullptr_t, Ret &retval);

    template <typename Ret>
    void sendToDetector(int fnum, std::nullptr_t, Ret &retval) const;

    void sendToDetector(int fnum);
    void sendToDetector(int fnum) const;

    template <typename Ret> Ret sendToDetector(int fnum);
    template <typename Ret> Ret sendToDetector(int fnum) const;

    template <typename Ret, typename Arg>
    Ret sendToDetector(int fnum, const Arg &args);

    template <typename Ret, typename Arg>
    Ret sendToDetector(int fnum, const Arg &args) const;

    /** Send function parameters to detector (stop server) */
    void sendToDetectorStop(int fnum, const void *args, size_t args_size,
                            void *retval, size_t retval_size);

    void sendToDetectorStop(int fnum, const void *args, size_t args_size,
                            void *retval, size_t retval_size) const;

    template <typename Arg, typename Ret>
    void sendToDetectorStop(int fnum, const Arg &args, Ret &retval);

    template <typename Arg, typename Ret>
    void sendToDetectorStop(int fnum, const Arg &args, Ret &retval) const;

    template <typename Arg>
    void sendToDetectorStop(int fnum, const Arg &args, std::nullptr_t);

    template <typename Arg>
    void sendToDetectorStop(int fnum, const Arg &args, std::nullptr_t) const;

    template <typename Ret>
    void sendToDetectorStop(int fnum, std::nullptr_t, Ret &retval);

    template <typename Ret>
    void sendToDetectorStop(int fnum, std::nullptr_t, Ret &retval) const;

    void sendToDetectorStop(int fnum);

    void sendToDetectorStop(int fnum) const;

    template <typename Ret> Ret sendToDetectorStop(int fnum);

    template <typename Ret> Ret sendToDetectorStop(int fnum) const;

    template <typename Ret, typename Arg>
    Ret sendToDetectorStop(int fnum, const Arg &args);

    template <typename Ret, typename Arg>
    Ret sendToDetectorStop(int fnum, const Arg &args) const;

    /** Send function parameters to receiver */
    void sendToReceiver(int fnum, const void *args, size_t args_size,
                        void *retval, size_t retval_size);

    void sendToReceiver(int fnum, const void *args, size_t args_size,
                        void *retval, size_t retval_size) const;

    template <typename Arg, typename Ret>
    void sendToReceiver(int fnum, const Arg &args, Ret &retval);

    template <typename Arg, typename Ret>
    void sendToReceiver(int fnum, const Arg &args, Ret &retval) const;

    template <typename Arg>
    void sendToReceiver(int fnum, const Arg &args, std::nullptr_t);

    template <typename Arg>
    void sendToReceiver(int fnum, const Arg &args, std::nullptr_t) const;

    template <typename Ret>
    void sendToReceiver(int fnum, std::nullptr_t, Ret &retval);

    template <typename Ret>
    void sendToReceiver(int fnum, std::nullptr_t, Ret &retval) const;

    template <typename Ret> Ret sendToReceiver(int fnum);

    template <typename Ret> Ret sendToReceiver(int fnum) const;

    void sendToReceiver(int fnum);

    void sendToReceiver(int fnum) const;

    template <typename Ret, typename Arg>
    Ret sendToReceiver(int fnum, const Arg &args);

    template <typename Ret, typename Arg>
    Ret sendToReceiver(int fnum, const Arg &args) const;

    /** Get Detector Type from Shared Memory
    verify is if shm size matches existing one */
    detectorType getDetectorTypeFromShm(int det_id, bool verify = true);

    /** Initialize shared memory
    verify is if shm size matches existing one  */
    void initSharedMemory(detectorType type, int det_id, bool verify = true);

    /** Initialize module structure to defaults,
    Called when new shared memory is created */
    void initializeModuleStructure(detectorType type);

    void initialDetectorServerChecks();
    const std::string getDetectorAPI() const;
    void checkDetectorVersionCompatibility();
    void checkReceiverVersionCompatibility();
    void setModule(sls_detector_module &module, bool trimbits = true);
    sls_detector_module getModule();
    void sendModule(sls_detector_module *myMod, ClientSocket &client);
    void receiveModule(sls_detector_module *myMod, ClientSocket &client);
    void updateClientStreamingIP();

    void updateRateCorrection();
    /** Template function to do linear interpolation between two points (Eiger
     only) */
    template <typename E, typename V>
    V linearInterpolation(const E x, const E x1, const E x2, const V y1,
                          const V y2) {
        double k = static_cast<double>(y2 - y1) / (x2 - x1);
        double m = y1 - k * x1;
        int y = round(k * x + m);
        return static_cast<V>(y);
    }

    /**
     * interpolates dacs and trimbits between 2 trim files
     * @param a first module structure
     * @param b second module structure
     * @param energy energy to trim at
     * @param e1 reference trim value
     * @param e2 reference trim value
     * @param trimbits 1 to include trimbits, 0 to exclude (used for eiger)
     * @returns  the pointer to the module structure with interpolated values or
     * NULL if error
     */
    sls_detector_module interpolateTrim(sls_detector_module *a,
                                        sls_detector_module *b,
                                        const int energy, const int e1,
                                        const int e2, bool trimbits = true);

    std::string getTrimbitFilename(detectorSettings settings, int e_eV);
    sls_detector_module readSettingsFile(const std::string &fname,
                                         bool trimbits = true);
    void saveSettingsFile(sls_detector_module &myMod, const std::string &fname);
    void sendProgram(bool blackfin, std::vector<char> buffer,
                     const int functionEnum, const std::string &functionType,
                     const std::string serverName = "",
                     const bool forceDeleteNormalFile = false);
    void simulatingActivityinDetector(const std::string &functionType,
                                      const int timeRequired);

    const int moduleIndex;
    mutable SharedMemory<sharedModule> shm{0, 0};
    static const int BLACKFIN_ERASE_FLASH_TIME = 65;
    static const int BLACKFIN_WRITE_TO_FLASH_TIME = 30;
    static const int NIOS_ERASE_FLASH_TIME_FPGA = 10;
    static const int NIOS_WRITE_TO_FLASH_TIME_FPGA = 45;
    static const int NIOS_ERASE_FLASH_TIME_KERNEL = 9;
    static const int NIOS_WRITE_TO_FLASH_TIME_KERNEL = 40;

    enum mythen3_DacIndex {
        M_VCASSH,
        M_VTH2,
        M_VRSHAPER,
        M_VRSHAPER_N,
        M_VIPRE_OUT,
        M_VTH3,
        M_VTH1,
        M_VICIN,
        M_VCAS,
        M_VRPREAMP,
        M_VCAL_N,
        M_VIPRE,
        M_VISHAPER,
        M_VCAL_P,
        M_VTRIM,
        M_VDCSH
    };

    enum eiger_DacIndex {
        E_SVP,
        E_VTR,
        E_VRF,
        E_VRS,
        E_SVN,
        E_VTGSTV,
        E_VCMP_LL,
        E_VCMP_LR,
        E_CAL,
        E_VCMP_RL,
        E_RXB_RB,
        E_RXB_LB,
        E_VCMP_RR,
        E_VCP,
        E_VCN,
        E_VIS
    };
};

} // namespace sls