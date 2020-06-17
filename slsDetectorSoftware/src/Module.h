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
    runStatus getRunStatus() const;
    runStatus getReceiverStatus() const;
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
    bool getStoreInRamMode();
    void setStoreInRamMode(const bool enable);
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

    /**
     * Set Detector offset in shared memory in dimension d
     * @param det detector size
     */

    int setControlPort(int port_number);

    /**
     * Returns the detector TCP control port  \sa sharedSlsDetector
     * @returns the detector TCP control port
     */
    int getControlPort() const;

    int setStopPort(int port_number);

    /**
     * Returns the detector TCP stop port  \sa sharedSlsDetector
     * @returns the detector TCP stop port
     */
    int getStopPort() const;

    /**
     * Lock server for this client IP
     * @param p 0 to unlock, 1 to lock (-1 gets)
     * @returns true for locked or false for unlocked
     */
    bool lockServer(int lock = -1);

    /**
     * Get last client IP saved on detector server
     * @returns last client IP saved on detector server
     */
    sls::IpAddr getLastClientIP();

    /**
     * Exit detector server
     */
    void exitServer();

    /**
     * Executes a system command on the detector server
     * e.g. mount an nfs disk, reboot and returns answer etc.
     * @param cmd command to be executed
     */
    void execCommand(const std::string &cmd);

    /**
     * Get detector specific commands to write into config file
     * @returns vector of strings with commands
     */
    std::vector<std::string> getConfigFileCommands();

    /**
     * Set threshold energy and settings (Eiger only)
     * @param e_eV threshold in eV
     * @param isettings ev. change settings
     * @param tb 1 to include trimbits, 0 to exclude
     */
    void setThresholdEnergyAndSettings(int e_eV, detectorSettings isettings,
                                       bool trimbits = true);

    /**
     * Start detector acquisition and read all data (Blocking until end of
     * acquisition)
     */
    void startAndReadAll();

    /**
     * Start readout (without exposure or interrupting exposure) (Eiger store in
     * ram)
     */
    void startReadOut();

    /**
     * Requests and  receives all data from the detector (Eiger store in ram)
     */
    void readAll();

    /**
     * Configures in detector the destination for UDP packets
     */
    void configureMAC();

    /** [Gotthard2] only in burst mode and in auto timing mode */
    int64_t getNumberOfBursts();

    /** [Gotthard2] only in burst mode and in auto timing mode */
    void setNumberOfBursts(int64_t value);

    /** [CTB][Moench] */
    int getNumberOfAnalogSamples();

    /** [CTB][Moench] */
    void setNumberOfAnalogSamples(int value);

    /** [CTB] */
    int getNumberOfDigitalSamples();

    /** [CTB] */
    void setNumberOfDigitalSamples(int value);

    /** [Mythen3] */
    int getNumberOfGates();

    /** [Mythen3] */
    void setNumberOfGates(int value);

    /** [Mythen3] for all gates */
    std::array<time::ns, 3> getExptimeForAllGates();

    /** [Mythen3] gatIndex: 0-2 */
    int64_t getGateDelay(int gateIndex);

    /** [Mythen3] gatIndex: -1 for all, 0-2 */
    void setGateDelay(int gateIndex, int64_t value);

    /** [Mythen3] for all gates */
    std::array<time::ns, 3> getGateDelayForAllGates();

    /** [Gotthard2] only in burst mode and in auto timing mode */
    int64_t getBurstPeriod();

    /** [Gotthard2] only in burst mode and in auto timing mode */
    void setBurstPeriod(int64_t value);

    /** [Jungfrau][CTB][Moench][Mythen3]
     * [Gotthard2] only in continuous mode */
    int64_t getNumberOfFramesFromStart() const;

    /** [Jungfrau][CTB][Moench][Mythen3] Get time from detector start
     * [Gotthard2] only in continuous mode */
    int64_t getActualTime() const;

    /** [Jungfrau][CTB][Moench][Mythen3] Get timestamp at a frame start
     * [Gotthard2] only in continuous mode */
    int64_t getMeasurementTime() const;

    /**
     * [Ctb]
     * @param mode readout mode Options: ANALOG_ONLY, DIGITAL_ONLY,
     * ANALOG_AND_DIGITAL
     */
    void setReadoutMode(const readoutMode mode);

    /**
     * [Ctb]
     * @returns readout mode
     */
    readoutMode getReadoutMode();

    /**
     * Write in a register. For Advanced users
     * @param addr address of register
     * @param val value to write into register
     * @returns value read after writing
     */
    uint32_t writeRegister(uint32_t addr, uint32_t val);

    /**
     * Read from a register. For Advanced users
     * @param addr address of register
     * @returns value read from register
     */
    uint32_t readRegister(uint32_t addr);

    /**
     * Set bit in a register. For Advanced users
     * @param addr address of register
     * @param n nth bit
     * @returns value read from register
     */
    uint32_t setBit(uint32_t addr, int n);

    /**
     * Clear bit in a register. For Advanced users
     * @param addr address of register
     * @param n nth bit
     * @returns value read from register
     */
    uint32_t clearBit(uint32_t addr, int n);

    void test();

    int getReceiverProgress() const;

    /** update receiver stremaing ip from shm to receiver
     * if empty, use rx_hostname ip
     */
    void updateReceiverStreamingIP();

    /** empty vector deletes entire additional json header */
    void setAdditionalJsonHeader(
        const std::map<std::string, std::string> &jsonHeader);
    std::map<std::string, std::string> getAdditionalJsonHeader();

    /**
     * Sets the value for the additional json header parameter key if found,
     * else append it. If value empty, then deletes parameter */
    void setAdditionalJsonParameter(const std::string &key,
                                    const std::string &value);
    std::string getAdditionalJsonParameter(const std::string &key);

    /** [Gotthard][Jungfrau][CTB][Moench] */
    void executeFirmwareTest();

    /** [Gotthard][Jungfrau][CTB][Moench] */
    void executeBusTest();

    /** [Gotthard2] */
    std::array<int, 2> getInjectChannel();

    /** [Gotthard2]
     * @param offsetChannel starting channel to be injected
     * @param incrementChannel determines succeeding channels to be injected */
    void setInjectChannel(const int offsetChannel, const int incrementChannel);

    /** [Gotthard2] asic input */
    std::vector<int> getVetoPhoton(const int chipIndex);

    /** [Gotthard2] energy in keV */
    void setVetoPhoton(const int chipIndex, const int numPhotons,
                       const int energy, const std::string &fname);

    void setVetoReference(const int gainIndex, const int value);

    /** [Gotthard2]  */
    burstMode getBurstMode();

    /** [Gotthard2] BURST_OFF, BURST_INTERNAL (default), BURST_EXTERNAL */
    void setBurstMode(burstMode value);

    /** [Gotthard2] */
    bool getCurrentSource();

    /** default disabled */
    void setCurrentSource(bool value);

    /** [Gotthard2] */
    slsDetectorDefs::timingSourceType getTimingSource();

    /** [Gotthard2] Options: TIMING_INTERNAL, TIMING_EXTERNAL */
    void setTimingSource(slsDetectorDefs::timingSourceType value);

    /** [Gotthard2] */
    bool getVeto();

    /** default disabled */
    void setVeto(bool enable);

    /**
     * Set ADC Enable Mask (CTB, Moench)
     * @param mask ADC Enable mask
     */
    void setADCEnableMask(uint32_t mask);

    /**
     * Get ADC Enable Mask (CTB, Moench)
     * @returns ADC Enable mask
     */
    uint32_t getADCEnableMask();

    /**
     * Set 10Gb  ADC Enable Mask (CTB, Moench)
     * @param mask ADC Enable mask
     */
    void setTenGigaADCEnableMask(uint32_t mask);

    /**
     * Get 10Gb ADC Enable Mask (CTB, Moench)
     * @returns ADC Enable mask
     */
    uint32_t getTenGigaADCEnableMask();

    /**
     * Set ADC invert register (CTB, Moench, Jungfrau)
     * @param value ADC invert value
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void setADCInvert(uint32_t value);

    /**
     * Get ADC invert register (CTB, Moench, Jungfrau)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns ADC invert value
     */
    uint32_t getADCInvert();

    /**
     * Set external sampling source (CTB only)
     * @param value external sampling source (Option: 0-63)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns external sampling source
     */
    int setExternalSamplingSource(int value);

    /**
     * Get external sampling source (CTB only)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns external sampling source
     */
    int getExternalSamplingSource();

    void setExternalSampling(bool value);

    bool getExternalSampling();

    /** digital data bits enable (CTB only) */
    void setReceiverDbitList(const std::vector<int> &list);
    std::vector<int> getReceiverDbitList() const;

    /** Set digital data offset in bytes (CTB only) */
    void setReceiverDbitOffset(int value);
    int getReceiverDbitOffset();

    /**
     * Write to ADC register (Gotthard, Jungfrau, ChipTestBoard). For expert
     * users
     * @param addr address of adc register
     * @param val value
     */
    void writeAdcRegister(uint32_t addr, uint32_t val);

    /**
     * Set storage cell that stores first acquisition of the series (Jungfrau)
     * @param value storage cell index. Value can be 0 to 15. (-1 gets)
     * @returns the storage cell that stores the first acquisition of the series
     */

    /**
     * [Jungfau][Ctb] Programs FPGA with raw file from pof file
     * [Mythen3][Gotthard2] Programs FPGA with raw file from rbf file
     * @param buffer programming file in memory
     */
    void programFPGA(std::vector<char> buffer);

    /** [Jungfau][Ctb] */
    void programFPGAviaBlackfin(std::vector<char> buffer);

    /** [Mythen3][Gotthard2] */
    void programFPGAviaNios(std::vector<char> buffer);
    /**
     * Resets FPGA (Jungfrau)
     */
    void resetFPGA();

    /**
     * Copies detector server from tftp and changes respawn server (Not Eiger)
     * @param fname name of detector server binary
     * @param hostname name of pc to tftp from
     */
    void copyDetectorServer(const std::string &fname,
                            const std::string &hostname);

    /**
     * [Jungfrau][Ctb][Gotthard][Mythen3][Gotthard2]
     * Reboot detector controller (blackfin/ powerpc)
     */
    void rebootController();

    /**
     * Get trimbit filename with path for settings and energy
     *
     */
    std::string getTrimbitFilename(detectorSettings settings, int e_eV);

    /**
     * Configure Module (Eiger)
     * Called for loading trimbits and settings settings to the detector
     * @param module module to be set - must contain correct module number and
     * also channel and chip registers
     * @param tb 1 to include trimbits, 0 to exclude (used for eiger)
     * \sa ::sls_detector_module
     */
    void setModule(sls_detector_module &module, bool trimbits = true);

    /**
     * Get module structure from detector (all detectors)
     * @returns pointer to module structure (which has been created and must
     * then be deleted)
     */
    sls_detector_module getModule();

    /**
     * Update rate correction according to dynamic range (Eiger)
     * If rate correction enabled and dr is 8 or 16, it will throw
     * Otherwise update ratecorrection if enabled
     */
    void updateRateCorrection();

    /**
     * Exits the receiver TCP server
     */
    void exitReceiver();

    /**
     * Gets the current frame index of receiver
     * @returns current frame index of receiver
     */
    uint64_t getReceiverCurrentFrameIndex() const;

    /**
     * If data streaming in receiver is enabled,
     * restream the stop dummy packet from receiver
     * Used usually for Moench,
     * in case it is lost in network due to high data rate
     */
    void restreamStopFromReceiver();

    /**
     * Opens pattern file and sends pattern to CTB
     * @param fname pattern file to open
     */
    void setPattern(const std::string &fname);

    /**
     * Sets pattern IO control (CTB/ Moench)
     * @param word 64bit word to be written, -1 gets
     * @returns actual value
     */
    uint64_t setPatternIOControl(uint64_t word = -1);

    /**
     * Sets pattern clock control (CTB/ Moench)
     * @param word 64bit word to be written, -1 gets
     * @returns actual value
     */
    uint64_t setPatternClockControl(uint64_t word = -1);

    /**
     * Writes a pattern word (CTB/ Moench/ Mythen3)
     * @param addr address of the word
     * @param word 64bit word to be written, -1 reads the addr (same as
     * executing the pattern for ctb)
     * @returns actual value
     */
    uint64_t setPatternWord(int addr, uint64_t word);

    /**
     * Sets the pattern or loop limits (CTB/ Moench/ Mythen3)
     * @param level -1 complete pattern, 0,1,2, loop level
     * @param start start address for level 0-2, -1 gets
     * @param stop stop address for level 0-2, -1 gets
     * @returns array of start addr and stop addr
     */
    std::array<int, 2> setPatternLoopAddresses(int level = -1, int start = -1,
                                               int stop = -1);

    /**
     * Sets the pattern or loop limits (CTB/ Moench/ Mythen3)
     * @param level -1 complete pattern, 0,1,2, loop level
     * @param n number of loops for level 0-2, -1 gets
     * @returns number of loops
     */
    int setPatternLoopCycles(int level = -1, int n = -1);

    /**
     * Sets the wait address (CTB/ Moench/ Mythen3)
     * @param level  0,1,2, wait level
     * @param addr wait address, -1 gets
     * @returns actual value
     */
    int setPatternWaitAddr(int level, int addr = -1);

    /**
     * Sets the wait time (CTB/ Moench/ Mythen3)
     * @param level  0,1,2, wait level
     * @param t wait time, -1 gets
     * @returns actual value
     */
    uint64_t setPatternWaitTime(int level, uint64_t t = -1);

    /**
     * Sets the mask applied to every pattern (CTB/ Moench/ Mythen3)
     * @param mask mask to be applied
     */
    void setPatternMask(uint64_t mask);

    /**
     * Gets the mask applied to every pattern (CTB/ Moench/ Mythen3)
     * @returns mask set
     */
    uint64_t getPatternMask();

    /**
     * Selects the bits that the mask will be applied to for every pattern (CTB/
     * Moench/ Mythen3)
     * @param mask mask to select bits
     */
    void setPatternBitMask(uint64_t mask);

    /**
     * Gets the bits that the mask will be applied to for every pattern (CTB/
     * Moench/ Mythen3)
     * @returns mask  of bits selected
     */
    uint64_t getPatternBitMask();

    /** [Mythen3] */
    void startPattern();

    /**
     * Set LED Enable (Moench, CTB only)
     * @param enable 1 to switch on, 0 to switch off, -1 gets
     * @returns LED enable
     */
    int setLEDEnable(int enable = -1);

    /**
     * Set Digital IO Delay (Moench, CTB only)
     * @param digital IO mask to select the pins
     * @param delay delay in ps(1 bit=25ps, max of 775 ps)
     */
    void setDigitalIODelay(uint64_t pinMask, int delay);

    /** [Ctb][Moench] */
    int getPipeline(int clkIndex);

    /** [Ctb][Moench] */
    void setPipeline(int clkIndex, int value);

    /** [Mythen3] */
    void setCounterMask(uint32_t countermask);

    /** [Mythen3] */
    uint32_t getCounterMask();

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

    /**
     * Send function parameters to detector (stop server)
     * @param fnum function enum
     * @param args argument pointer
     * @param args_size size of argument
     * @param retval return pointers
     * @param retval_size size of return value
     */
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

    /**
     * Send function parameters to receiver
     * @param fnum function enum
     * @param args argument pointer
     * @param args_size size of argument
     * @param retval return pointers
     * @param retval_size size of return value
     */
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

    /**
     * Send a sls_detector_module structure over socket
     * @param myMod module structure to send
     * @returns number of bytes sent to the detector
     */
    int sendModule(sls_detector_module *myMod, sls::ClientSocket &client);

    /**
     * Receive a sls_detector_module structure over socket
     * @param myMod module structure to receive
     * @returns number of bytes received from the detector
     */
    int receiveModule(sls_detector_module *myMod, sls::ClientSocket &client);

    /**
     * Get MAC from the receiver using udpip and
     * set up UDP connection in detector
     */
    void setUDPConnection();

    /*
     * Template function to do linear interpolation between two points (Eiger
     * only)
     */
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

    /**
     * reads a trim/settings file
     * @param fname name of the file to be read
     * @param myMod pointer to the module structure which has to be set. <BR>
     * If it is NULL a new module structure will be created
     * @param tb 1 to include trimbits, 0 to exclude (used for eiger)
     * @returns the pointer to myMod or NULL if reading the file failed
     */

    sls_detector_module readSettingsFile(const std::string &fname,
                                         bool trimbits = true);

    /** Module Id or position in the detectors list */
    const int moduleId;

    /** Shared Memory object */
    mutable sls::SharedMemory<sharedSlsDetector> shm{0, 0};
};

} // namespace sls