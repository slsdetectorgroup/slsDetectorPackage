#pragma once
#include "ClientSocket.h"
#include "SharedMemory.h"
#include "StaticVector.h"
#include "logger.h"
#include "network_utils.h"
#include "sls_detector_defs.h"

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
    @param verify if shared memory version matches existing one */
    explicit Module(detectorType type, int det_id = 0, int module_id = 0,
                    bool verify = true);

    /** opening existing shared memory
    @param verify if shared memory version matches existing one */
    explicit Module(int det_id = 0, int module_id = 0, bool verify = true);

    virtual ~Module();

    /** Frees shared memory and deletes shared memory structure
    Safe to call only if detector shm also deleted or its numberOfDetectors is
    updated */
    void freeSharedMemory();
    bool isFixedPatternSharedMemoryCompatible();
    std::string getHostname() const;

    /** @param initialChecks enable or disable initial compatibility checks and
    other server start up checks. Enabled by default. Disable only for advanced
    users! */
    void setHostname(const std::string &hostname, const bool initialChecks);

    int64_t getFirmwareVersion();
    int64_t getDetectorServerVersion();
    int64_t getSerialNumber();
    int64_t getReceiverSoftwareVersion() const;
    static detectorType getTypeFromDetector(const std::string &hostname,
                                            int cport = DEFAULT_PORTNO);

    /** Get Detector type from shared memory */
    detectorType getDetectorType() const;
    void updateNumberOfChannels();
    slsDetectorDefs::xy getNumberOfChannels() const;
    void updateNumberOfDetector(slsDetectorDefs::xy det);
    detectorSettings getSettings();
    void setSettings(detectorSettings isettings);
    void loadSettingsFile(const std::string &fname);
    int getAllTrimbits();
    void setAllTrimbits(int val);

    /**************************************************
     *                                                *
     *    Acquisition Parameters                      *
     *                                                *
     * ************************************************/
    int64_t getNumberOfFrames();
    void setNumberOfFrames(int64_t value);
    int64_t getNumberOfTriggers();
    void setNumberOfTriggers(int64_t value);
    /** [Mythen3] gatIndex: 0-2, [Others]: -1 always */
    int64_t getExptime(int gateIndex);
    /** [Mythen3] gatIndex: -1 for all, 0-2, [Others]: -1 always */
    void setExptime(int gateIndex, int64_t value);
    int64_t getPeriod();
    void setPeriod(int64_t value);
    int64_t getDelayAfterTrigger();
    void setDelayAfterTrigger(int64_t value);
    int64_t getNumberOfFramesLeft() const;
    int64_t getNumberOfTriggersLeft() const;
    int64_t getDelayAfterTriggerLeft() const;
    int64_t getPeriodLeft() const;
    timingMode getTimingMode();
    void setTimingMode(timingMode value);
    int getClockDivider(int clkIndex);
    void setClockDivider(int clkIndex, int value);
    int getClockPhase(int clkIndex, bool inDegrees);
    void setClockPhase(int clkIndex, int value, bool inDegrees);
    int getMaxClockPhaseShift(int clkIndex);
    int getClockFrequency(int clkIndex);
    void setClockFrequency(int clkIndex, int value);
    int getDAC(dacIndex index, bool mV);
    void setDAC(int val, dacIndex index, bool mV);
    bool getPowerChip();
    void setPowerChip(bool on);
    int getImageTestMode();
    void setImageTestMode(const int value);
    /* temperature in millidegrees */
    int getADC(dacIndex index);
    int getOnChipDAC(slsDetectorDefs::dacIndex index, int chipIndex);
    void setOnChipDAC(slsDetectorDefs::dacIndex index, int chipIndex,
                      int value);
    externalSignalFlag getExternalSignalFlags(int signalIndex);
    void setExternalSignalFlags(int signalIndex, externalSignalFlag type);

    /**************************************************
     *                                                *
     *    Acquisition                                 *
     *                                                *
     * ************************************************/
    void startReceiver();
    void stopReceiver();
    void prepareAcquisition();
    void startAcquisition();
    void stopAcquisition();
    void startAndReadAll();
    runStatus getRunStatus() const;
    runStatus getReceiverStatus() const;
    int getReceiverProgress() const;
    int64_t getFramesCaughtByReceiver() const;
    std::vector<uint64_t> getNumMissingPackets() const;
    uint64_t getStartingFrameNumber();
    void setStartingFrameNumber(uint64_t value);
    void sendSoftwareTrigger();

    /**************************************************
     *                                                 *
     *    Network Configuration (Detector<->Receiver)  *
     *                                                 *
     * ************************************************/
    int getNumberofUDPInterfacesFromShm();
    int getNumberofUDPInterfaces();
    void setNumberofUDPInterfaces(int n);
    int getSelectedUDPInterface();
    void selectUDPInterface(int n);
    sls::IpAddr getSourceUDPIP();
    void setSourceUDPIP(const sls::IpAddr ip);
    sls::IpAddr getSourceUDPIP2();
    void setSourceUDPIP2(const sls::IpAddr ip);
    sls::MacAddr getSourceUDPMAC();
    void setSourceUDPMAC(const sls::MacAddr mac);
    sls::MacAddr getSourceUDPMAC2();
    void setSourceUDPMAC2(const sls::MacAddr mac);
    sls::IpAddr getDestinationUDPIP();
    void setDestinationUDPIP(const sls::IpAddr ip);
    sls::IpAddr getDestinationUDPIP2();
    void setDestinationUDPIP2(const sls::IpAddr ip);
    sls::MacAddr getDestinationUDPMAC();
    void setDestinationUDPMAC(const sls::MacAddr mac);
    sls::MacAddr getDestinationUDPMAC2();
    void setDestinationUDPMAC2(const sls::MacAddr mac);
    int getDestinationUDPPort();
    void setDestinationUDPPort(int udpport);
    int getDestinationUDPPort2();
    void setDestinationUDPPort2(int udpport);
    std::string printReceiverConfiguration();
    bool getTenGiga();
    void setTenGiga(bool value);
    bool getTenGigaFlowControl();
    void setTenGigaFlowControl(bool enable);
    int getTransmissionDelayFrame();
    void setTransmissionDelayFrame(int value);
    int getTransmissionDelayLeft();
    void setTransmissionDelayLeft(int value);
    int getTransmissionDelayRight();
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
    int getReceiverFifoDepth();
    void setReceiverFifoDepth(int n_frames);
    bool getReceiverSilentMode();
    void setReceiverSilentMode(bool enable);
    frameDiscardPolicy getReceiverFramesDiscardPolicy();
    void setReceiverFramesDiscardPolicy(frameDiscardPolicy f);
    bool getPartialFramesPadding();
    void setPartialFramesPadding(bool padding);
    int64_t getReceiverUDPSocketBufferSize() const;
    int64_t getReceiverRealUDPSocketBufferSize() const;
    void setReceiverUDPSocketBufferSize(int64_t udpsockbufsize);
    bool getReceiverLock();
    void setReceiverLock(bool lock);
    sls::IpAddr getReceiverLastClientIP() const;
    std::array<pid_t, NUM_RX_THREAD_IDS> getReceiverThreadIds() const;

    /**************************************************
     *                                                *
     *    File                                        *
     *                                                *
     * ************************************************/
    fileFormat getFileFormat();
    void setFileFormat(fileFormat f);
    std::string getFilePath();
    void setFilePath(const std::string &path);
    std::string getFileName();
    void setFileName(const std::string &fname);
    int64_t getFileIndex();
    void setFileIndex(int64_t file_index);
    void incrementFileIndex();
    bool getFileWrite();
    void setFileWrite(bool value);
    bool getMasterFileWrite();
    void setMasterFileWrite(bool value);
    bool getFileOverWrite();
    void setFileOverWrite(bool value);
    int getFramesPerFile();
    /** 0 will set frames per file to unlimited */
    void setFramesPerFile(int n_frames);

    /**************************************************
     *                                                *
     *    ZMQ Streaming Parameters (Receiver<->Client)*
     *                                                *
     * ************************************************/
    bool getReceiverStreaming();
    void setReceiverStreaming(bool enable);
    int getReceiverStreamingFrequency();
    /** Option: nth frame streamed out, if 0, streamed out at a timer of 200 */
    void setReceiverStreamingFrequency(int freq);
    int getReceiverStreamingTimer();
    void setReceiverStreamingTimer(int time_in_ms = 200);
    int getReceiverStreamingPort();
    void setReceiverStreamingPort(int port);
    sls::IpAddr getReceiverStreamingIP();
    void setReceiverStreamingIP(const sls::IpAddr ip);
    int getClientStreamingPort();
    void setClientStreamingPort(int port);
    sls::IpAddr getClientStreamingIP();
    void setClientStreamingIP(const sls::IpAddr ip);

    /**************************************************
     *                                                *
     *    Eiger Specific                              *
     *                                                *
     * ************************************************/
    int getDynamicRange();
    void setDynamicRange(int n);
    int64_t getSubExptime();
    void setSubExptime(int64_t value);
    int64_t getSubDeadTime();
    void setSubDeadTime(int64_t value);
    int getThresholdEnergy();
    void setThresholdEnergy(int e_eV, detectorSettings isettings = GET_SETTINGS,
                            bool trimbits = true);
    std::string getSettingsDir();
    std::string setSettingsDir(const std::string &dir);
    bool getParallelMode();
    void setParallelMode(const bool enable);
    bool getOverFlowMode();
    void setOverFlowMode(const bool enable);
    bool getFlippedDataX();
    void setFlippedDataX(bool value);
    std::vector<int> getTrimEn();
    int setTrimEn(const std::vector<int> &energies = {});
    int64_t getRateCorrection();
    void setDefaultRateCorrection();
    void setRateCorrection(int64_t t = 0);
    int getReadNLines();
    void setReadNLines(const int value);
    bool getInterruptSubframe();
    void setInterruptSubframe(const bool enable);
    int64_t getMeasuredPeriod() const;
    int64_t getMeasuredSubFramePeriod() const;
    bool getActivate();
    void setActivate(const bool enable);
    bool getDeactivatedRxrPaddingMode();
    void setDeactivatedRxrPaddingMode(bool padding);
    bool getCounterBit();
    void setCounterBit(bool cb);
    void pulsePixel(int n = 0, int x = 0, int y = 0);
    void pulsePixelNMove(int n = 0, int x = 0, int y = 0);
    void pulseChip(int n_pulses = 0);
    bool getQuad();
    void setQuad(const bool enable);

    /**************************************************
     *                                                *
     *    Jungfrau Specific                           *
     *                                                *
     * ************************************************/
    int getThresholdTemperature();
    void setThresholdTemperature(int val);
    bool getTemperatureControl();
    void setTemperatureControl(bool val);
    int getTemperatureEvent();
    void resetTemperatureEvent();
    bool getAutoComparatorDisableMode();
    void setAutoComparatorDisableMode(bool val);
    int getNumberOfAdditionalStorageCells();
    void setNumberOfAdditionalStorageCells(int value);
    int getStorageCellStart();
    void setStorageCellStart(int pos);
    int64_t getStorageCellDelay();
    void setStorageCellDelay(int64_t value);

    /**************************************************
     *                                                *
     *    Gotthard Specific                           *
     *                                                *
     * ************************************************/
    slsDetectorDefs::ROI getROI();
    void setROI(slsDetectorDefs::ROI arg);
    void clearROI();
    int64_t getExptimeLeft() const;

    /**************************************************
     *                                                *
     *    Gotthard2 Specific                          *
     *                                                *
     * ************************************************/
    int64_t getNumberOfBursts();
    void setNumberOfBursts(int64_t value);
    int64_t getBurstPeriod();
    void setBurstPeriod(int64_t value);
    std::array<int, 2> getInjectChannel();
    void setInjectChannel(const int offsetChannel, const int incrementChannel);
    std::vector<int> getVetoPhoton(const int chipIndex);
    void setVetoPhoton(const int chipIndex, const int numPhotons,
                       const int energy, const std::string &fname);
    void setVetoReference(const int gainIndex, const int value);
    burstMode getBurstMode();
    void setBurstMode(burstMode value);
    bool getCurrentSource();
    void setCurrentSource(bool value);
    slsDetectorDefs::timingSourceType getTimingSource();
    void setTimingSource(slsDetectorDefs::timingSourceType value);
    bool getVeto();
    void setVeto(bool enable);

    /**************************************************
     *                                                *
     *    Mythen3 Specific                            *
     *                                                *
     * ************************************************/
    uint32_t getCounterMask();
    void setCounterMask(uint32_t countermask);
    int getNumberOfGates();
    void setNumberOfGates(int value);
    std::array<time::ns, 3> getExptimeForAllGates();
    int64_t getGateDelay(int gateIndex);
    void setGateDelay(int gateIndex, int64_t value);
    std::array<time::ns, 3> getGateDelayForAllGates();

    /**************************************************
     *                                                *
     *    CTB / Moench Specific                       *
     *                                                *
     * ************************************************/
    int getNumberOfAnalogSamples();
    void setNumberOfAnalogSamples(int value);
    int getPipeline(int clkIndex);
    void setPipeline(int clkIndex, int value);
    uint32_t getADCEnableMask();
    void setADCEnableMask(uint32_t mask);
    uint32_t getTenGigaADCEnableMask();
    void setTenGigaADCEnableMask(uint32_t mask);

    /**************************************************
     *                                                *
     *    CTB Specific                                *
     *                                                *
     * ************************************************/
    int getNumberOfDigitalSamples();
    void setNumberOfDigitalSamples(int value);
    readoutMode getReadoutMode();
    void setReadoutMode(const readoutMode mode);
    int getExternalSamplingSource();
    int setExternalSamplingSource(int value);
    bool getExternalSampling();
    void setExternalSampling(bool value);
    std::vector<int> getReceiverDbitList() const;
    void setReceiverDbitList(const std::vector<int> &list);
    int getReceiverDbitOffset();
    void setReceiverDbitOffset(int value);
    void setDigitalIODelay(uint64_t pinMask, int delay);
    bool getLEDEnable();
    void setLEDEnable(bool enable);

    /**************************************************
     *                                                *
     *    Pattern                                     *
     *                                                *
     * ************************************************/
    void setPattern(const std::string &fname);
    uint64_t getPatternIOControl();
    void setPatternIOControl(uint64_t word);
    uint64_t getPatternClockControl();
    void setPatternClockControl(uint64_t word);
    uint64_t getPatternWord(int addr);
    void setPatternWord(int addr, uint64_t word);
    std::array<int, 2> getPatternLoopAddresses(int level);
    void setPatternLoopAddresses(int level, int start, int stop);
    int getPatternLoopCycles(int level);
    void setPatternLoopCycles(int level, int n);
    int getPatternWaitAddr(int level);
    void setPatternWaitAddr(int level, int addr);
    uint64_t getPatternWaitTime(int level);
    void setPatternWaitTime(int level, uint64_t t);
    uint64_t getPatternMask();
    void setPatternMask(uint64_t mask);
    uint64_t getPatternBitMask();
    void setPatternBitMask(uint64_t mask);
    void startPattern();

    /**************************************************
     *                                                *
     *    Moench                                      *
     *                                                *
     * ************************************************/
    std::map<std::string, std::string> getAdditionalJsonHeader();
    void setAdditionalJsonHeader(
        const std::map<std::string, std::string> &jsonHeader);
    std::string getAdditionalJsonParameter(const std::string &key);
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
    uint32_t readRegister(uint32_t addr);
    uint32_t writeRegister(uint32_t addr, uint32_t val);
    uint32_t setBit(uint32_t addr, int n);
    uint32_t clearBit(uint32_t addr, int n);
    void executeFirmwareTest();
    void executeBusTest();
    void writeAdcRegister(uint32_t addr, uint32_t val);
    uint32_t getADCInvert();
    void setADCInvert(uint32_t value);

    /**************************************************
     *                                                *
     *    Insignificant                               *
     *                                                *
     * ************************************************/
    int getControlPort() const;
    int setControlPort(int port_number);
    int getStopPort() const;
    int setStopPort(int port_number);
    bool getLockDetector();
    void setLockDetector(bool lock);
    sls::IpAddr getLastClientIP();
    std::string execCommand(const std::string &cmd);
    int64_t getNumberOfFramesFromStart() const;
    int64_t getActualTime() const;
    int64_t getMeasurementTime() const;
    uint64_t getReceiverCurrentFrameIndex() const;

  private:
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

    template <typename Arg, typename Ret>
    void sendToDetector(int fnum, const Arg &args, Ret &retval);
    template <typename Arg>
    void sendToDetector(int fnum, const Arg &args, std::nullptr_t);
    template <typename Ret>
    void sendToDetector(int fnum, std::nullptr_t, Ret &retval);
    void sendToDetector(int fnum);

    template <typename Ret> Ret sendToDetector(int fnum);

    template <typename Ret, typename Arg>
    Ret sendToDetector(int fnum, const Arg &args);

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

    template <typename Ret, typename Arg>
    Ret sendToReceiver(int fnum, const Arg &args);

    template <typename Ret, typename Arg>
    Ret sendToReceiver(int fnum, const Arg &args) const;

    /** Get Detector Type from Shared Memory
    @param verify if shm size matches existing one */
    detectorType getDetectorTypeFromShm(int det_id, bool verify = true);

    /** Initialize shared memory
     @param verify if shm size matches existing one  */
    void initSharedMemory(detectorType type, int det_id, bool verify = true);

    /** Initialize detector structure to defaults,
    Called when new shared memory is created */
    void initializeDetectorStructure(detectorType type);

    void checkDetectorVersionCompatibility();
    void checkReceiverVersionCompatibility();
    void restreamStopFromReceiver();
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
     * @param tb 1 to include trimbits, 0 to exclude (used for eiger)
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