#pragma once
#include "SharedMemory.h"
#include "sls/ClientSocket.h"
#include "sls/StaticVector.h"
#include "sls/logger.h"
#include "sls/network_utils.h"
#include "sls/sls_detector_defs.h"
#include "sls/Pattern.h"

#include <array>
#include <cmath>
#include <map>
#include <vector>

class ServerInterface;

#define SLS_SHMAPIVERSION 0x190726
#define SLS_SHMVERSION    0x200402

namespace sls {

/**
 * @short structure allocated in shared memory to store detector settings for
 * IPC and cache
 */
struct sharedSlsDetector {

    /* FIXED PATTERN FOR STATIC FUNCTIONS. DO NOT CHANGE, ONLY APPEND ------*/

    int shmversion;
    char hostname[MAX_STR_LENGTH];
    slsDetectorDefs::detectorType myDetectorType;

    /** END OF FIXED PATTERN -----------------------------------------------*/

    slsDetectorDefs::xy numberOfDetector;
    int controlPort;
    int stopPort;
    char settingsDir[MAX_STR_LENGTH];
    /** list of the energies at which the detector has been trimmed  */
    sls::StaticVector<int, MAX_TRIMEN> trimEnergies;
    /**  number of channels per chip */
    slsDetectorDefs::xy nChan;
    slsDetectorDefs::xy nChip;
    int nDacs;
    char rxHostname[MAX_STR_LENGTH];
    int rxTCPPort;
    /** if rxHostname and rxTCPPort can be connected to */
    bool useReceiverFlag;
    /** Listening tcp port from gui (only data) */
    int zmqport;
    /**  Listening tcp ip address from gui (only data) **/
    sls::IpAddr zmqip;
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
    explicit Module(detectorType type, int det_id = 0, int module_id = 0,
                    bool verify = true);

    /** opening existing shared memory
    verify is if shared memory version matches existing one */
    explicit Module(int det_id = 0, int module_id = 0, bool verify = true);

    virtual ~Module();

    /** Frees shared memory and deletes shared memory structure
    Safe to call only if detector shm also deleted or its numberOfDetectors is
    updated */
    void freeSharedMemory();
    bool isFixedPatternSharedMemoryCompatible() const;
    std::string getHostname() const;

    /** initialChecks is enable or disable initial compatibility checks and
    other server start up checks. Enabled by default. Disable only for advanced
    users! */
    void setHostname(const std::string &hostname, const bool initialChecks);

    int64_t getFirmwareVersion() const;
    int64_t getDetectorServerVersion() const;
    int64_t getSerialNumber() const;
    int64_t getReceiverSoftwareVersion() const;
    static detectorType getTypeFromDetector(const std::string &hostname,
                                            int cport = DEFAULT_PORTNO);

    /** Get Detector type from shared memory */
    detectorType getDetectorType() const;
    void updateNumberOfChannels();
    slsDetectorDefs::xy getNumberOfChannels() const;
    void updateNumberOfDetector(slsDetectorDefs::xy det);
    detectorSettings getSettings() const;
    void setSettings(detectorSettings isettings);
    void loadSettingsFile(const std::string &fname);
    int getAllTrimbits() const;
    void setAllTrimbits(int val);
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
    int getClockDivider(int clkIndex) const;
    void setClockDivider(int clkIndex, int value);
    int getClockPhase(int clkIndex, bool inDegrees) const;
    void setClockPhase(int clkIndex, int value, bool inDegrees);
    int getMaxClockPhaseShift(int clkIndex) const;
    int getClockFrequency(int clkIndex) const;
    void setClockFrequency(int clkIndex, int value);
    /** [Eiger][Jungfrau][Moench][Gotthard][Gotthard2][Mythen3] */
    void setDefaultDacs();
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
    int64_t getFramesCaughtByReceiver() const;
    std::vector<uint64_t> getNumMissingPackets() const;
    uint64_t getNextFrameNumber() const;
    void setNextFrameNumber(uint64_t value);
    void sendSoftwareTrigger();
    defs::scanParameters getScan() const;
    void setScan(const defs::scanParameters t);
    std::string getScanErrorMessage() const;

    /**************************************************
     *                                                 *
     *    Network Configuration (Detector<->Receiver)  *
     *                                                 *
     * ************************************************/
    int getNumberofUDPInterfacesFromShm() const;
    int getNumberofUDPInterfaces() const;
    void setNumberofUDPInterfaces(int n);
    int getSelectedUDPInterface() const;
    void selectUDPInterface(int n);
    sls::IpAddr getSourceUDPIP() const;
    void setSourceUDPIP(const sls::IpAddr ip);
    sls::IpAddr getSourceUDPIP2() const;
    void setSourceUDPIP2(const sls::IpAddr ip);
    sls::MacAddr getSourceUDPMAC() const;
    void setSourceUDPMAC(const sls::MacAddr mac);
    sls::MacAddr getSourceUDPMAC2() const;
    void setSourceUDPMAC2(const sls::MacAddr mac);
    sls::IpAddr getDestinationUDPIP() const;
    void setDestinationUDPIP(const sls::IpAddr ip);
    sls::IpAddr getDestinationUDPIP2() const;
    void setDestinationUDPIP2(const sls::IpAddr ip);
    sls::MacAddr getDestinationUDPMAC() const;
    void setDestinationUDPMAC(const sls::MacAddr mac);
    sls::MacAddr getDestinationUDPMAC2() const;
    void setDestinationUDPMAC2(const sls::MacAddr mac);
    int getDestinationUDPPort() const;
    void setDestinationUDPPort(int udpport);
    int getDestinationUDPPort2() const;
    void setDestinationUDPPort2(int udpport);
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
    void setReceiverHostname(const std::string &receiver);
    int getReceiverPort() const;
    int setReceiverPort(int port_number);
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
    sls::IpAddr getReceiverLastClientIP() const;
    std::array<pid_t, NUM_RX_THREAD_IDS> getReceiverThreadIds() const;

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
    int getReceiverStreamingPort() const;
    void setReceiverStreamingPort(int port);
    sls::IpAddr getReceiverStreamingIP() const;
    void setReceiverStreamingIP(const sls::IpAddr ip);
    int getClientStreamingPort() const;
    void setClientStreamingPort(int port);
    sls::IpAddr getClientStreamingIP() const;
    void setClientStreamingIP(const sls::IpAddr ip);
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
    int getThresholdEnergy() const;
    void setThresholdEnergy(int e_eV, detectorSettings isettings,
                            bool trimbits);
    std::string getSettingsDir() const;
    std::string setSettingsDir(const std::string &dir);
    bool getOverFlowMode() const;
    void setOverFlowMode(const bool enable);
    bool getFlippedDataX() const;
    void setFlippedDataX(bool value);
    std::vector<int> getTrimEn() const;
    int setTrimEn(const std::vector<int> &energies = {});
    int64_t getRateCorrection() const;
    void setDefaultRateCorrection();
    void setRateCorrection(int64_t t = 0);
    void sendReceiverRateCorrections(const std::vector<int64_t> &t);
    int getReadNLines() const;
    void setReadNLines(const int value);
    bool getInterruptSubframe() const;
    void setInterruptSubframe(const bool enable);
    int64_t getMeasuredPeriod() const;
    int64_t getMeasuredSubFramePeriod() const;
    bool getActivate() const;
    void setActivate(const bool enable);
    bool getDeactivatedRxrPaddingMode() const;
    void setDeactivatedRxrPaddingMode(bool padding);
    bool getCounterBit() const;
    void setCounterBit(bool cb);
    void pulsePixel(int n = 0, int x = 0, int y = 0);
    void pulsePixelNMove(int n = 0, int x = 0, int y = 0);
    void pulseChip(int n_pulses = 0);
    bool getQuad() const;
    void setQuad(const bool enable);

    /**************************************************
     *                                                *
     *    Jungfrau Specific                           *
     *                                                *
     * ************************************************/
    int getThresholdTemperature() const;
    void setThresholdTemperature(int val);
    bool getTemperatureControl() const;
    void setTemperatureControl(bool val);
    int getTemperatureEvent() const;
    void resetTemperatureEvent();
    bool getAutoComparatorDisableMode() const;
    void setAutoComparatorDisableMode(bool val);
    int getNumberOfAdditionalStorageCells() const;
    void setNumberOfAdditionalStorageCells(int value);
    int getStorageCellStart() const;
    void setStorageCellStart(int pos);
    int64_t getStorageCellDelay() const;
    void setStorageCellDelay(int64_t value);

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
    int getFilter() const;
    void setFilter(int value);
    bool getCurrentSource() const;
    void setCurrentSource(bool value);
    slsDetectorDefs::timingSourceType getTimingSource() const;
    void setTimingSource(slsDetectorDefs::timingSourceType value);
    bool getVeto() const;
    void setVeto(bool enable);
    int getADCConfiguration(const int chipIndex, const int adcIndex) const;
    void setADCConfiguration(const int chipIndex, const int adcIndex,
                             int value);
    void getBadChannels(const std::string &fname) const;
    void setBadChannels(const std::string &fname);

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

    /**************************************************
     *                                                *
     *    CTB / Moench Specific                       *
     *                                                *
     * ************************************************/
    int getNumberOfAnalogSamples() const;
    void setNumberOfAnalogSamples(int value);
    int getPipeline(int clkIndex) const;
    void setPipeline(int clkIndex, int value);
    uint32_t getADCEnableMask() const;
    void setADCEnableMask(uint32_t mask);
    uint32_t getTenGigaADCEnableMask() const;
    void setTenGigaADCEnableMask(uint32_t mask);

    /**************************************************
     *                                                *
     *    CTB Specific                                *
     *                                                *
     * ************************************************/
    int getNumberOfDigitalSamples() const;
    void setNumberOfDigitalSamples(int value);
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
     *    Pattern                                     *
     *                                                *
     * ************************************************/
    void setPattern(const Pattern& pat);
    Pattern getPattern();
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
     *    Moench                                      *
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
    void programFPGA(std::vector<char> buffer);
    void resetFPGA();
    void copyDetectorServer(const std::string &fname,
                            const std::string &hostname);
    void rebootController();
    uint32_t readRegister(uint32_t addr) const;
    uint32_t writeRegister(uint32_t addr, uint32_t val);
    void setBit(uint32_t addr, int n);
    void clearBit(uint32_t addr, int n);
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
    int getControlPort() const;
    void setControlPort(int port_number);
    int getStopPort() const;
    void setStopPort(int port_number);
    bool getLockDetector() const;
    void setLockDetector(bool lock);
    sls::IpAddr getLastClientIP() const;
    std::string execCommand(const std::string &cmd);
    int64_t getNumberOfFramesFromStart() const;
    int64_t getActualTime() const;
    int64_t getMeasurementTime() const;
    uint64_t getReceiverCurrentFrameIndex() const;

  private:
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

    /** Initialize detector structure to defaults,
    Called when new shared memory is created */
    void initializeDetectorStructure(detectorType type);

    void checkDetectorVersionCompatibility();
    void checkReceiverVersionCompatibility();
    void setModule(sls_detector_module &module, bool trimbits = true);
    int sendModule(sls_detector_module *myMod, sls::ClientSocket &client);
    void updateReceiverStreamingIP();

    void updateRateCorrection();
    void setThresholdEnergyAndSettings(int e_eV, detectorSettings isettings,
                                       bool trimbits = true);
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
    void programFPGAviaBlackfin(std::vector<char> buffer);
    void programFPGAviaNios(std::vector<char> buffer);

    const int moduleId;
    mutable sls::SharedMemory<sharedSlsDetector> shm{0, 0};
};

} // namespace sls