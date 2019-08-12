#pragma once
#include "Result.h"
#include "sls_detector_defs.h"
#include <chrono>
#include <memory>
#include <vector>

class multiSlsDetector;
namespace sls {
using ns = std::chrono::nanoseconds;
class MacAddr;
class IpAddr;

/**
 * \class Detector
 */
class Detector {
    std::unique_ptr<multiSlsDetector> pimpl;

  public:
    /**
     * @param multi_id multi detector shared memory id
     */
    Detector(int multi_id = 0);
    ~Detector();


    /**************************************************
     *                                                *
     *    CONFIG                                      *
     *                                                *
     * ************************************************/
    /**
     * Frees the shared memory of this detector and all modules
     * belonging to it.
     */
    void freeSharedMemory();
    void setConfig(const std::string &fname);
    Result<std::string> getHostname(Positions pos = {}) const;
    /**
     * Frees shared memory and adds detectors to the list
     * Also updates local detector cache
     * @param name hostnames for the positions given
     */
    void setHostname(const std::vector<std::string> &value);

    /**
     * Get multidetector Id
     * @returns multidetector Id
     */
    int getMultiId() const;

    /**
     * Check version compatibility with detector software
     * @param pos detector position
     */
    void checkDetectorVersionCompatibility(Positions pos = {}) const;

    /**
     * Check version compatibility with receiver software
     * @param pos detector position
     */
    void checkReceiverVersionCompatibility(Positions pos = {}) const;

    /**
     * Get detector firmware version
     * @param pos detector position
     * @returns detector firmware version
     */
    Result<int64_t> getDetectorFirmwareVersion(Positions pos = {}) const;

    /**
     * Get detector server version
     * @param pos detector position
     * @returns detector server version
     */
    Result<int64_t> getDetectorServerVersion(Positions pos = {}) const;

    /**
     * Get detector serial number
     * @param pos detector position
     * @returns detector serial number
     */
    Result<int64_t> getDetectorSerialNumber(Positions pos = {}) const;

    /**
     * Get Client Software version
     * @returns client software version
     */
    int64_t getClientSoftwareVersion() const;

    /**
     * Get Receiver software version
     * @param pos detector position
     * @return receiver software version
     */
    Result<int64_t> getReceiverSoftwareVersion(Positions pos = {}) const;

    /**
     * Get user details of shared memory
     * @returns string with user details
     */
    std::string getUserDetails() const;

    /**
     * Get Detector type as an enum
     * @returns detector type
     */
    defs::detectorType getDetectorType() const;

    /**
     * Get Detector type as an enum
     * @param pos detector position
     * @returns detector type
     */
    Result<defs::detectorType> getDetectorTypeAsEnum(Positions pos = {}) const;

    /**
     * Returns detector type as a string
     * @param pos detector position
     * @returns detector type as string
     */
    Result<std::string> getDetectorTypeAsString(Positions pos = {}) const;

    /**
     * Returns the total number of detectors in the multidetector structure
     * @returns total number of detectors in the multidetector structure
     */
    int size() const;

    /**
     * Returns the number of detectors in the multidetector structure
     * @returns number of detectors in the multidetector structure
     */
    defs::coordinates getNumberOfDetectors() const;

    /**
     * Returns the number of channels
     * @param pos detector position
     * @returns the number of channels
     */
    Result<defs::coordinates> getNumberOfChannels(Positions pos = {}) const;

    /**
     * Returns the number of channels including gap pixels
     * @param pos detector position
     * @returns the number of channels including gap pixels
     */
    Result<defs::coordinates>
    getNumberOfChannelsInclGapPixels(Positions pos = {}) const;

    /**
     * Returns the maximum number of channels of complete detector in both
     * dimensions. -1 means no limit in this dimension. This value is used to
     * calculate row and column offsets for each module.
     * @returns the maximum number of channels of complete detector
     */
    defs::coordinates getMaxNumberOfChannels() const;

    /**
     * Sets the maximum number of channels of complete detector in both
     * dimensions. -1 means no limit in this dimension. This value is used to
     * calculate row and column offsets for each module.
     * @param value the maximum number of channels of complete detector
     */
    void setMaxNumberOfChannels(const defs::coordinates value);

    /**
     * Get Detector offset from shared memory (Gotthard only)
     * @param pos detector position
     * @returns offset in both dimensions
     */
    Result<defs::coordinates> getDetectorOffsets(Positions pos = {}) const;

    /**
     * Set Detector offset in shared memory for each module
     * @param value offset for detector in both dimensions
     * @param pos detector position
     */
    void setDetectorOffsets(defs::coordinates value, Positions pos = {});

    /**
     * Get Quad Type (Only for Eiger Quad detector hardware)
     * @param pos detector position
     * @returns quad type
     */
    Result<bool> getQuad(Positions pos = {}) const;

    /**
     * Set Quad Type (Only for Eiger Quad detector hardware)
     * @param enable true if quad type set, else false
     * @param pos detector position
     */
    void setQuad(const bool enable, Positions pos = {});

    /**
     * Get number of rows to read out (Only for Eiger)
     * @param pos detector position
     * @returns number of lines
     */
    Result<int> getReadNLines(Positions pos = {}) const;

    /**
     * Set number of rows to read out (Only for Eiger)
     * @param value number of lines
     * @param pos detector position
     */
    void setReadNLines(const int value, Positions pos = {});

    /**
     * Get Detector Control TCP port (for client communication with Detector
     * control server)
     * @param pos detector position
     * @returns control TCP port
     */
    Result<int> getControlPort(Positions pos = {}) const;

    /**
     * Set Detector Control TCP port (for client communication with Detector
     * control server)
     * @param value port number
     * @param pos detector position
     */
    void setControlPort(int value, Positions pos = {});

    /**
     * Get Detector Stop TCP port (for client communication with Detector Stop
     * server)
     * @param pos detector position
     * @returns Stop TCP port
     */
    Result<int> getStopPort(Positions pos = {}) const;

    /**
     * Set Detector Stop TCP port (for client communication with Detector Stop
     * server)
     * @param value port number
     * @param pos detector position
     */
    void setStopPort(int value, Positions pos = {});

    /**
     * Get Receiver TCP port (for client communication with Receiver)
     * @param pos detector position
     * @returns Receiver TCP port
     */
    Result<int> getReceiverPort(Positions pos = {}) const;

    /**
     * Set Receiver TCP port (for client communication with Receiver)
     * @param value port number
     * @param pos detector position
     */
    void setReceiverPort(int value, Positions pos = {});

    /**
     * Gets Lock for detector control server to this client IP
     * @param pos detector position
     * @returns lock
     */
    Result<bool> getLockServer(Positions pos = {}) const;

    /**
     * Sets Lock for detector control server to this client IP
     * @param value lock
     * @param pos detector position
     */
    void setLockServer(bool value, Positions pos = {});

    /**
     * Get last client IP saved on detector server
     * @param pos detector position
     * @returns last client IP saved on detector server
     */
    Result<std::string> getLastClientIP(Positions pos = {}) const;

    /**
     * Exit detector server
     * @param pos detector position
     */
    void exitServer(Positions pos = {});

    /**
     * Execute a command on the detector server
     * @param value command
     * @param pos detector position
     */
    void execCommand(const std::string &value, Positions pos = {});

    /**
     * Write current configuration to a file
     * @param value configuration file name
     */
    void writeConfigurationFile(const std::string &value);

    /**
     * Get detector settings
     * @param pos detector position
     * @returns current settings
     */
    Result<defs::detectorSettings> getSettings(Positions pos = {}) const;

    /**
     * Load detector settings from the settings file picked from the
     * trimdir/settingsdir
     * Eiger only stores in shared memory ( a get will
     * overwrite this) For Eiger, one must use threshold
     * @param value settings
     * @param pos detector position
     */
    void setSettings(defs::detectorSettings value, Positions pos = {});

    /**
     * Get threshold energy (Eiger)
     * @param pos detector position
     * @returns current threshold value for imod in ev (-1 failed)
     */
    Result<int> getThresholdEnergy(Positions pos = {}) const;

    /**
     * Set threshold energy (Eiger)
     * @param value threshold in eV
     * @param sett ev. change settings
     * @param tb 1 to include trimbits, 0 to exclude
     * @param pos detector position
     */
    void setThresholdEnergy(int value,
                            defs::detectorSettings sett = defs::GET_SETTINGS,
                            int tb = 1, Positions pos = {});

    /**
     * Returns the detector trimbit/settings directory
     * @param pos detector position
     * @returns the trimbit/settings directory
     */
    Result<std::string> getSettingsDir(Positions pos = {}) const;

    /**
     * Sets the detector trimbit/settings directory
     * @param value trimbits/settings directory
     * @param pos detector position
     */
    void setSettingsDir(const std::string &value, Positions pos = {});

    /**
     * Loads the modules settings/trimbits reading from a specific file
     * file name extension is automatically generated.
     * @param value specific settings/trimbits file
     * @param pos detector position
     */
    void loadSettingsFile(const std::string &value, Positions pos = {});

    /**
     * Saves the modules settings/trimbits to a specific file
     * file name extension is automatically generated.
     * @param value specific settings/trimbits file
     * @param pos detector position
     */
    void saveSettingsFile(const std::string &value, Positions pos = {});

    /**
     * Get Detector run status
     * @param pos detector position
     * @returns status
     */
    Result<defs::runStatus> getRunStatus(Positions pos = {});

    /**
     * Prepares detector for acquisition (Eiger)
     */
    void prepareAcquisition();

    /**
     * Start detector acquisition (Non blocking)
     */
    void startAcquisition();

    /**
     * Stop detector acquisition
     */
    void stopAcquisition();

    /**
     * Give an internal software trigger to the detector (Eiger only)
     * @param pos detector position
     */
    void sendSoftwareTrigger(Positions pos = {});

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
     * @param pos detector position
     */
    void configureMAC(Positions pos = {});

    /**
     * Get number of Frames
     * @returns number of Frames
     */
    Result<int64_t> getNumberOfFrames() const;

    /**
     * Set number of Frames
     * @param value number of Frames
     */
    void setNumberOfFrames(int64_t value);

    /**
     * Get number of Cycles
     * @returns number of Cycles
     */
    Result<int64_t> getNumberOfCycles() const;

    /**
     * Set number of Cycles
     * @param value number of Cycles
     */
    void setNumberOfCycles(int64_t value);

    /**
     * Get number of additional storage cells (Jungfrau)
     * @returns number of additional storage cells
     */
    Result<int64_t> getNumberOfStorageCells() const;

    /**
     * Set number of additional storage cells (Jungfrau)
     * @param value number of additional storage cells
     */
    void setNumberOfStorageCells(int64_t value);

    /** [Jungfrau] */
    Result<int> getStorageCellStart(Positions pos = {}) const;

    /**
     * [Jungfrau] Sets the storage cell storing the first acquisition of the series
     * Options: 0-15
     */
    void setStoragecellStart(int cell, Positions pos = {});

    /**
     * Get number of analog samples (CTB)
     * @param pos detector position
     * @returns number of analog samples
     */
    Result<int64_t> getNumberOfAnalogSamples(Positions pos = {}) const;

    /**
     * Set number of analog samples (CTB)
     * @param value number of analog samples (CTB)
     * @param pos detector position
     */
    void setNumberOfAnalogSamples(int64_t value, Positions pos = {});

    /**
     * Get number of digital samples (CTB)
     * @param pos detector position
     * @returns number of digital samples
     */
    Result<int64_t> getNumberOfDigitalSamples(Positions pos = {}) const;

    /**
     * Set number of digital samples (CTB)
     * @param value number of digital samples (CTB)
     * @param pos detector position
     */
    void setNumberOfDigitalSamples(int64_t value, Positions pos = {});

    /**
     * Get exposure time in ns
     * @param pos detector position
     * @returns exposure time in ns
     */
    Result<ns> getExptime(Positions pos = {}) const;

    /**
     * Set exposure time in ns
     * @param value exposure time in ns
     * @param pos detector position
     */
    void setExptime(ns t, Positions pos = {});

    /**
     * Get period in ns
     * @param pos detector position
     * @returns period in ns
     */
    Result<ns> getPeriod(Positions pos = {}) const;

    /**
     * Set period in ns
     * @param value period in ns
     * @param pos detector position
     */
    void setPeriod(ns t, Positions pos = {});
    /**
     * Get delay after trigger in ns(Gotthard, Jungfrau)
     * @param pos detector position
     * @returns delay after trigger in ns
     */
    Result<ns> getDelayAfterTrigger(Positions pos = {}) const;

    /**
     * Set delay after trigger (Gotthard, Jungfrau)
     * @param value delay after trigger in ns
     * @param pos detector position
     */
    void setDelayAfterTrigger(ns value, Positions pos = {});

    /**
     * Get sub frame exposure time in ns (Eiger in 32 bit mode)
     * @param pos detector position
     * @returns sub frame exposure time in ns
     */
    Result<ns> getSubExptime(Positions pos = {}) const;

    /**
     * Set sub frame exposure time after trigger (Eiger in 32 bit mode)
     * @param value sub frame exposure time in ns
     * @param pos detector position
     */
    void setSubExptime(ns t, Positions pos = {});
    /**
     * Get sub frame dead time in ns (Eiger in 32 bit mode)
     * @param pos detector position
     * @returns sub frame dead time in ns
     */
    Result<ns> getSubDeadTime(Positions pos = {}) const;

    /**
     * Set sub frame dead time after trigger (Eiger in 32 bit mode)
     * @param value sub frame dead time in ns
     * @param pos detector position
     */
    void setSubDeadTime(ns value, Positions pos = {});

    /**
     * Get storage cell delay (Jungfrau)
     * @param pos detector position
     * @returns storage cell delay in ns. Range: (0-1638375 ns (resolution of
     * 25ns)
     */
    Result<ns> getStorageCellDelay(Positions pos = {}) const;

    /**
     * Set storage cell delay (Jungfrau)
     * @param value storage cell delay in ns. Range: (0-1638375 ns (resolution
     * of 25ns)
     * @param pos detector position
     */
    void setStorageCellDelay(ns value, Positions pos = {});

    /**
     * Get number of Frames left (Gotthard, Jungfrau, CTB)
     * @param pos detector position
     * @returns number of Frames left
     */
    Result<int64_t> getNumberOfFramesLeft(Positions pos = {}) const;

    /**
     * Get number of Cycles left (Gotthard, Jungfrau, CTB)
     * @param pos detector position
     * @returns number of Cycles left
     */
    Result<int64_t> getNumberOfCyclesLeft(Positions pos = {}) const;
    /**
     * Get exposure time left in ns (Gotthard)
     * @param pos detector position
     * @returns exposure time left in ns
     */
    Result<ns> getExptimeLeft(Positions pos = {}) const;

    /**
     * Get period left in ns (Gotthard, Jungfrau, CTB)
     * @param pos detector position
     * @returns period left in ns
     */
    Result<ns> getPeriodLeft(Positions pos = {}) const;

    /**
     * Get delay after trigger left in ns(Gotthard, Jungfrau, CTB)
     * @param pos detector position
     * @returns delay after trigger left in ns
     */
    Result<ns> getDelayAfterTriggerLeft(Positions pos = {}) const;

    /**
     * Get number of frames from start up of detector (Jungfrau, CTB)
     * @param pos detector position
     * @returns number of frames from start up of detector
     */
    Result<int64_t> getNumberOfFramesFromStart(Positions pos = {}) const;

    /**
     * Get time from detector start in ns (Jungfrau, CTB)
     * @param pos detector position
     * @returns time from detector start in ns
     */
    Result<ns> getActualTime(Positions pos = {}) const;

    /**
     * Get timestamp at a frame start in ns(Jungfrau, CTB)
     * @param pos detector position
     * @returns timestamp at a frame start in ns
     */
    Result<ns> getMeasurementTime(Positions pos = {}) const;

    /**
     * Get measured period between previous two frames in ns (Eiger)
     * @param pos detector position
     * @returns measured period between previous two frames in ns
     */
    Result<ns> getMeasuredPeriod(Positions pos = {}) const;

    /**
     * Get measured sub frame period between previous two frames in ns (Eiger in
     * 32 bit mode)
     * @param pos detector position
     * @returns measured sub frame period between previous two frames in ns
     */
    Result<ns> getMeasuredSubFramePeriod(Positions pos = {}) const;

    /**
     * Get speed (Eiger, Jungfrau)
     * @param pos detector position
     * @returns speed (0 full speed, 1 half speed, 2 quarter speed)
     */
    Result<int> getSpeed(Positions pos = {}) const;

    /**
     * Set speed (Eiger, Jungfrau)
     * @param value speed (0 full speed, 1 half speed, 2 quarter speed)
     * @param pos detector position
     */
    void setSpeed(int value, Positions pos = {});

    /**
     * Get ADC Phase (Gotthard, Jungfrau, CTB)
     * @param inDeg in degrees (Jungfrau, CTB)
     * @returns ADC Phase
     */
    Result<int> getADCPhase(bool inDeg, Positions pos = {}) const;

    /**
     * Set sADC Phase (Gotthard, Jungfrau, CTB)
     * @param value ADC Phase
     * @param inDeg in degrees (Jungfrau, CTB)
     */
    void setADCPhase(int value, bool inDeg, Positions pos = {});

    /**
     * Get Max ADC Phase Shift (Jungfrau, CTB)
     * @returns Max ADC Phase Shift
     */
    Result<int> getMaxADCPhaseShift(Positions pos = {}) const;

    /**
     * Get DBIT Phase (Jungfrau, CTB)
     * @param inDeg in degrees
     * @returns DBIT Phase
     */
    Result<int> getDBITPhase(bool inDeg, Positions pos = {}) const;

    /**
     * Set DBIT Phase (CTB)
     * @param value DBIT Phase
     * @param inDeg in degrees
     */
    void setDBITPhase(int value, bool inDeg, Positions pos = {});

    /**
     * Get Max DBIT Phase Shift  (CTB)
     * @returns Max DBIT Phase Shift
     */
    Result<int> getMaxDBITPhaseShift(Positions pos = {}) const;

    /**
     * Get ADC Clock in MHz (CTB)
     * @returns ADC Clock in MHz
     */
    Result<int> getADCClock(Positions pos = {}) const;

    /**
     * Set ADC Clock in MHz (CTB)
     * @param value ADC Clock in MHz
     */
    void setADCClock(int value, Positions pos = {});

    /**
     * Get DBIT Clock in MHz (CTB)
     * @returns DBIT Clock in MHz
     */
    Result<int> getDBITClock(Positions pos = {}) const;

    /**
     * Set DBIT Clock in MHz (CTB)
     * @param value DBIT Clock in MHz
     */
    void setDBITClock(int value, Positions pos = {});

    /**
     * Get RUN Clock in MHz (CTB)
     * @returns RUN Clock in MHz
     */
    Result<int> getRUNClock(Positions pos = {}) const;

    /**
     * Set RUN Clock in MHz (CTB)
     * @param value RUN Clock in MHz
     */
    void setRUNClock(int value, Positions pos = {});

    /**
     * Get SYNC Clock in MHz (CTB)
     * @returns SYNC Clock in MHz
     */
    Result<int> getSYNCClock(Positions pos = {}) const;

    /**
     * Get ADC Pipeline (CTB)
     * @returns ADC Pipeline
     */
    Result<int> getADCPipeline(Positions pos = {}) const;

    /**
     * Set ADC Pipeline (CTB)
     * @param value ADC Pipeline
     */
    void setADCPipeline(int value, Positions pos = {});

    /**
     * Get DBIT Pipeline (CTB)
     * @returns DBIT Pipeline
     */
    Result<int> getDBITPipeline(Positions pos = {}) const;

    /**
     * Set  DBIT Pipeline (CTB)
     * @param value  DBIT Pipeline
     */
    void setDBITPipeline(int value, Positions pos = {});

    Result<int> getDynamicRange(Positions pos = {}) const;

    /**
     * (Eiger:
     * Options: 4, 8, 16, 32
     * If i is 32, also sets clkdivider to 2, if 16, sets clkdivider to 1)
     */
    void setDynamicRange(int value);

    Result<int> getHighVoltage(Positions pos = {}) const;

    /**
     * (Gotthard Options: 0, 90, 110, 120, 150, 180, 200)
     * (Jungfrau, CTB Options: 0, 60 - 200)
     * (Eiger Options: 0 - 200)
     */
    void setHighVoltage(int value, Positions pos = {});

    /**
     * (Eiger)
     */
    Result<int> getIODelay(Positions pos = {}) const;

    /**
     * (Eiger)
     */
    void setIODelay(int value, Positions pos = {});

    /**
     * (Degrees)
     * (Gotthard Options: TEMPERATURE_ADC, TEMPERATURE_FPGA)
     * (Jungfrau Options: TEMPERATURE_ADC, TEMPERATURE_FPGA)
     * (Eiger Options: TEMPERATURE_FPGA, TEMPERATURE_FPGAEXT, TEMPERATURE_10GE,
     * TEMPERATURE_DCDC, TEMPERATURE_SODL, TEMPERATURE_SODR, TEMPERATURE_FPGA2,
     * TEMPERATURE_FPGA3) (CTB Options: SLOW_ADC_TEMP)
     */
    Result<int> getTemp(defs::dacIndex index, Positions pos = {}) const;

    /**
     * (CTB mV)
     */
    Result<int> getVrefVoltage(bool mV, Positions pos = {}) const;

    /**
     * (CTB mV)
     */
    void setVrefVoltage(int value, bool mV, Positions pos = {});

    /**
     * (CTB mV Options: V_LIMIT, V_POWER_A, V_POWER_B, V_POWER_C,
     * V_POWER_D, V_POWER_IO, V_POWER_CHIP))
     */
    Result<int> getVoltage(defs::dacIndex index, Positions pos = {}) const;

    /**
     * (CTB mV Options: V_LIMIT, V_POWER_A, V_POWER_B, V_POWER_C,
     * V_POWER_D, V_POWER_IO, V_POWER_CHIP)
     */
    void setVoltage(int value, defs::dacIndex index, Positions pos = {});

    /**
     * (CTB mV Options: V_POWER_A, V_POWER_B, V_POWER_C, V_POWER_D, V_POWER_IO,
     * V_POWER_CHIP)
     */
    Result<int> getMeasuredVoltage(defs::dacIndex index,
                                   Positions pos = {}) const;

    /**
     * (CTB mA Options: I_POWER_A, I_POWER_B, I_POWER_C, I_POWER_D, I_POWER_IO)
     */
    Result<int> getMeasuredCurrent(defs::dacIndex index,
                                   Positions pos = {}) const;

    /**
     * (CTB Options: SLOW_ADC0 - SLOW_ADC7)
     */
    Result<int> getSlowADC(defs::dacIndex index, Positions pos = {}) const;

    Result<int> getDAC(defs::dacIndex index, bool mV, Positions pos = {}) const;

    void setDAC(int value, defs::dacIndex index, bool mV, Positions pos = {});

    Result<defs::externalCommunicationMode> getTimingMode(Positions pos = {}) const;

    /**
     * (Gotthard, Jungfrau, CTB Options: AUTO_TIMING, TRIGGER_EXPOSURE)
     * (Eiger Options: AUTO_TIMING, TRIGGER_EXPOSURE, GATED, BURST_TRIGGER)
     */
    void setTimingMode(defs::externalCommunicationMode value, Positions pos = {});

    /**
     * (Gotthard)
     */
    Result<defs::externalSignalFlag> getExternalSignalFlags(Positions pos = {}) const;

    /**
     * (Gotthard Options: TRIGGER_IN_RISING_EDGE, TRIGGER_IN_FALLING_EDGE)
     */
    void setExternalSignalFlags(defs::externalSignalFlag value, Positions pos = {});

    /**
     * (Eiger)
     */
    Result<bool> getParallelMode(Positions pos = {}) const;

    /**
     * (Eiger)
     */
    void setParallelMode(bool value, Positions pos = {});

    /**
     * (Eiger)
     */
    Result<bool> getOverFlowMode(Positions pos = {}) const;

    /**
     * (Eiger)
     */
    void setOverFlowMode(bool value, Positions pos = {});

    /**
     * (CTB Options: NORMAL_READOUT = 0, DIGITAL_ONLY = 1, ANALOG_AND_DIGITAL = 2)
     */
    Result<int> getSignalType(Positions pos = {}) const;

    /**
     * (CTB Options: NORMAL_READOUT = 0, DIGITAL_ONLY = 1, ANALOG_AND_DIGITAL = 2)
     */
    void setSignalType(int value, Positions pos = {});

    // Erik

    /** [Eiger] */
    Result<bool> getInterruptSubframe(Positions pos = {}) const;

    /** [Eiger] when set,  the last subframe is interrupted at end of acq */
    void setInterruptSubframe(const bool enable, Positions pos = {});

    Result<uint32_t> readRegister(uint32_t addr, Positions pos = {}) const;

    void writeRegister(uint32_t addr, uint32_t val, Positions pos = {});

    void setBit(uint32_t addr, int bitnr, Positions pos = {});

    void clearBit(uint32_t addr, int bitnr, Positions pos = {});

    Result<MacAddr> getDetectorMAC(Positions pos = {}) const;

    void setDetectorMAC(const std::string &detectorMAC, Positions pos = {});

    /** [Jungfrau] bottom half */
    Result<MacAddr> getDetectorMAC2(Positions pos = {}) const;

    /** [Jungfrau] bottom half */
    void setDetectorMAC2(const std::string &detectorMAC, Positions pos = {});

    Result<IpAddr> getDetectorIP(Positions pos = {}) const;

    void setDetectorIP(const std::string &detectorIP, Positions pos = {});

    /** [Jungfrau] bottom half */
    Result<IpAddr> getDetectorIP2(Positions pos = {}) const;

    /** [Jungfrau] bottom half */
    void setDetectorIP2(const std::string &detectorIP, Positions pos = {});

    Result<std::string> getReceiverHostname(Positions pos = {}) const;

    /**
     * Validates and sets the receiver.
     * Updates local receiver cache parameters
     * Configures the detector to the receiver as UDP destination
     * @param receiver receiver hostname or IP address 
     */
    void setReceiverHostname(const std::string &receiver, Positions pos = {});

    Result<IpAddr> getReceiverUDPIP(Positions pos = {}) const;
    void setReceiverUDPIP(const std::string &udpip, Positions pos = {});

    /** [Jungfrau bottom half] */
    Result<IpAddr> getReceiverUDPIP2(Positions pos = {}) const;

    /** [Jungfrau bottom half] */
    void setReceiverUDPIP2(const std::string &udpip, Positions pos = {});

    Result<MacAddr> getReceiverUDPMAC(Positions pos = {}) const;
    void setReceiverUDPMAC(const std::string &udpmac, Positions pos = {});

    /** [Jungfrau bottom half] */
    Result<MacAddr> getReceiverUDPMAC2(Positions pos = {}) const;
    /** [Jungfrau bottom half] */
    void setReceiverUDPMAC2(const std::string &udpmac, Positions pos = {});

    Result<int> getReceiverUDPPort(Positions pos = {}) const;
    void setReceiverUDPPort(int udpport, Positions pos = {});

    /** [Eiger right port][Jungfrau bottom half] */
    Result<int> getReceiverUDPPort2(Positions pos = {}) const;
    /** [Eiger right port][Jungfrau bottom half] */
    void setReceiverUDPPort2(int udpport, Positions pos = {});

    /** [Jungfrau] */
    Result<int> getNumberofUDPInterfaces(Positions pos = {}) const;
    /** [Jungfrau] Also restarts client and receiver sockets */
    void setNumberofUDPInterfaces(int n, Positions pos = {});

    /** [Jungfrau] */
    Result<int> getSelectedUDPInterface(Positions pos = {}) const;
    /** 
     * [Jungfrau:
     * Effective only when number of interfaces is 1.
     * Options: 0 (outer, default), 1(inner)] 
     */
    void selectUDPInterface(int interface, Positions pos = {});

    Result<int> getClientStreamingPort(Positions pos = {}) const;
    /** 
     * pos can be for a single module or all modules, not a subset
     * If pos for all modules, ports for each module is calculated (increments)
     * Restarts client zmq sockets
     */
    void setClientDataStreamingInPort(int port, Positions pos = {});
 
    Result<int> getReceiverStreamingPort(Positions pos = {}) const;
    /** 
     * pos can be for a single module or all modules, not a subset
     * If pos for all modules, ports for each module is calculated (increments)
     * Restarts receiver zmq sockets
     */
    void setReceiverDataStreamingOutPort(int port, Positions pos = {});

    Result<std::string> getClientStreamingIP(Positions pos = {}) const;
    // TODO these should probably be the same ?? same as what?
    void setClientDataStreamingInIP(const std::string &ip, Positions pos = {});

    Result<std::string> getReceiverStreamingIP(Positions pos = {}) const;
    void setReceiverDataStreamingOutIP(const std::string &ip,
                                       Positions pos = {});

    /** [Eiger, Jungfrau] */
    Result<bool> getFlowControl10G(Positions pos = {}) const;
    /** [Eiger, Jungfrau] */
    void setFlowControl10G(bool enable, Positions pos = {});

    /** [Eiger, Jungfrau] */
    Result<int> getTransmissionDelayFrame(Positions pos = {}) const;
    /** 
     * [Jungfrau: Sets the transmission delay of the first UDP packet being streamed out of the module
     * Options: 0 - 31, each value represenets 1 ms ] 
     * [Eiger: Sets the transmission delay of entire frame streamed out for both left and right UDP ports]
     */
    void setTransmissionDelayFrame(int value, Positions pos = {});

    /** [Eiger] */
    Result<int> getTransmissionDelayLeft(Positions pos = {}) const;
    /** 
     * [Eiger]
     * Sets the transmission delay of first packet streamed ut of the left UDP port
     */
    void setTransmissionDelayLeft(int value, Positions pos = {});

    /** [Eiger] */
    Result<int> getTransmissionDelayRight(Positions pos = {}) const;
    /** 
     * [Eiger]
     * Sets the transmission delay of first packet streamed ut of the right UDP port
     */
    void setTransmissionDelayRight(int value, Positions pos = {});
    
    /** [Moench] */
    Result<std::string> getAdditionalJsonHeader(Positions pos = {}) const;
    /** [Moench] */
    void setAdditionalJsonHeader(const std::string &jsonheader,
                                 Positions pos = {});

    /** [Moench] */
    Result<std::string> getAdditionalJsonParameter(const std::string &key,
                                                   Positions pos = {}) const;
    /** 
     * [Moench]
     * Sets the value for additional json header parameter if found, 
     * else appends the parameter key and value 
     * The value cannot be empty
     */
    void setAdditionalJsonParameter(const std::string &key,
                                    const std::string &value,
                                    Positions pos = {});

    /** [Moench] TODO! How do we do this best??? Can be refactored to something
     * else? Use a generic zmq message passing system...
     * For now limiting to all detectors working the same*/
    /** [Moench: -1 if not found or cannot convert to int] */
    Result<int> getDetectorMinMaxEnergyThreshold(const bool isEmax, Positions pos = {}) const;
    /** [Moench] */
    void setDetectorMinMaxEnergyThreshold(const bool isEmax, const int value, Positions pos = {});
    /** [Moench: -1 if unknown mode] */
    Result<int> getFrameMode(Positions pos = {}) const;
    /** [Moench] */    
    void setFrameMode(defs::frameModeType value, Positions pos = {});
    /** [Moench: -1 if unknown mode] */
    Result<int> getDetectorMode(Positions pos = {}) const;    
    /** [Moench] */
    void setDetectorMode(defs::detectorModeType value, Positions pos = {});

    /** [Gotthard] */
    Result<int> getDigitalTestBit(Positions pos = {});
    /** [Gotthard] */
    Result<int> setDigitalTestBit(const int value, Positions pos = {});
    /** [Gotthard, Jungfrau, CTB] */
    Result<int> executeFirmwareTest(Positions pos = {});
    /** [Gotthard, Jungfrau, CTB] */
    Result<int> executeBusTest(Positions pos = {});
    /** [Gotthard] subset modules not allowed */
    void loadDarkImage(const std::string &fname, Positions pos = {});
    /** [Gotthard] subset modules not allowed */
    void loadGainImage(const std::string &fname, Positions pos = {});
    /**
     * [Gotthard] subset modules not allowed 
     * @param startACQ if start acq after reading counter
     */
    void getCounterMemoryBlock(const std::string &fname, bool startACQ, Positions pos = {});
    /**
     * [Gotthard] 
     * @param startACQ if start acq after resetting counter
     * TODO! does it make sense to call one detector?
     */
    void resetCounterBlock(bool startACQ, Positions pos = {});

    /** [Eiger] */
    Result<bool> getCounterBit(Positions pos = {}) const;
    /** [Eiger] If it is set, it resets chips completely (else partially) before an acquisition TODO: if it makes sense */
    void setCounterBit(bool value, Positions pos = {});

    /** [Gotthard, CTB]*/
    Result<std::vector<defs::ROI>> getROI(Positions pos = {}) const;
    /** 
     * [Gotthard Options: Only a single chip or all chips, only 1 ROI allowed]
     * [CTB: multiple ROIs allowed] 
     * subset modules not allowed 
     */
    void setROI(std::vector<defs::ROI> value, Positions pos = {});

    /** [CTB]*/
    Result<uint32_t> getADCEnableMask(Positions pos = {}) const;

    /** [CTB]*/
    void setADCEnableMask(uint32_t mask, Positions pos = {});

    /** [CTB] */
    Result<uint32_t> getADCInvert(Positions pos = {}) const;

    /** [CTB]*/
    void setADCInvert(uint32_t value, Positions pos = {});

    /** [CTB] */
    Result<int> getExternalSamplingSource(Positions pos = {}) const;

    /** [CTB] Value between 0-63 */
    void setExternalSamplingSource(int value, Positions pos = {});

    /** [CTB] */
    Result<int> getExternalSampling(Positions pos = {}) const;

    /** [CTB] */
    void setExternalSampling(bool value, Positions pos = {});

    /** [CTB] */
    Result<std::vector<int>> getReceiverDbitList(Positions pos = {}) const;

    /** [CTB] list contains the set of bits (0-63) to save */
    void setReceiverDbitList(std::vector<int> list, Positions pos = {});

    /** [CTB] */
    Result<int> getReceiverDbitOffset(Positions pos = {}) const;

    /** [CTB] Set number of bytes of digital data to skip in the Receiver */
    void setReceiverDbitOffset(int value, Positions pos = {});

    /** [Gotthard][Jungfrau][CTB] not possible to read back*/
    void writeAdcRegister(uint32_t addr, uint32_t value, Positions pos = {});

    /** [Eiger] */
    Result<bool> getActive(Positions pos = {}) const;

    /** [Eiger] */
    void setActive(bool active, Positions pos = {});

    /** [Eiger] */
    Result<bool> getBottom(Positions pos = {}) const;

    /** [Eiger] for gui purposes */
    void setBottom(bool value, Positions pos = {});   

    /** [Eiger] 
     * @returns -1 if they are all different
    */
    Result<int> getAllTrimbits(Positions pos = {}) const;

    /**[Eiger] */
    void setAllTrimbits(int value, Positions pos = {});

    /**[Eiger] */
    Result<bool> getGapPixelsEnable(Positions pos = {}) const;

    /**
     * [Eiger] 
     * 4 bit mode not implemented in Receiver, but in client data call back
     * Fills in gap pixels in data
     */
    void setGapPixelsEnable(bool enable);

    /**[Eiger] Returns energies in eV where the module is trimmed */
    Result<std::vector<int>> getTrimEnergies(Positions pos = {}) const;

    /** [Eiger] Set the energies where the detector is trimmed */
    void setTrimEnergies(std::vector<int> energies, Positions pos = {});

    /**
     * [Eiger] Pulse Pixel n times at x and y coordinates
     */
    void pulsePixel(int n, int x, int y, Positions pos = {});

    /** [Eiger] Pulse Pixel n times and move by a relative value of x and y coordinates */
    void pulsePixelNMove(int n, int x, int y, Positions pos = {});

    /** [Eiger] Pulse chip n times */
    void pulseChip(int n, Positions pos = {});

    /** [Jungfrau] */
    Result<int> getThresholdTemperature(Positions pos = {}) const;

    /**
     * [Jungfrau]Set threshold temperature
     * If temperature crosses threshold temperature
     * and temperature control is enabled, 
     * power to chip will be switched off and 
     * temperature event will be set
     * @param val value in millidegrees TODO! Verify 
     */
    void setThresholdTemperature(int temp, Positions pos = {});

    /** [Jungfrau] */
    Result<bool> getTemperatureControl(Positions pos = {}) const;

    /** [Jungfrau] */
    void setTemperatureControl(bool enable, Positions pos = {});

    /** [Jungfrau] */
    Result<int> getTemperatureEvent(Positions pos = {}) const;

    /** [Jungfrau] */
    void ResetTemperatureEvent(Positions pos = {});

    /** [Jungfrau][CTB] */
    void programFPGA(const std::string &fname, Positions pos = {});

    /** [Jungfrau][CTB] */
    void resetFPGA(Positions pos = {});

    /** 
     * Copy detector server fname from tftp folder of hostname to detector 
     * Also changes respawn server, which is effective after a reboot.
    */
    void copyDetectorServer(const std::string &fname,
                            const std::string &hostname, Positions pos = {});

    /** [Jungfrau][Gotthard][CTB] */
    void rebootController(Positions pos = {});

    /**
     * [Jungfrau][Gotthard][CTB]
     * Updates the firmware, detector server and then reboots detector
     * controller blackfin. 
     * @param sname name of detector server binary found on tftp folder of host pc
     * @param hostname name of pc to tftp from
     * @param fname programming file name
     * @param pos detector positions
     */
    void updateFirmwareAndServer(const std::string &sname,
                                 const std::string &hostname,
                                 const std::string &fname, Positions pos = {});

    /** [Jungfrau] */
    Result<bool> getPowerChip(Positions pos = {}) const;

    /** [Jungfrau] */
    void setPowerChip(bool on, Positions pos = {});    

    /** [Jungfrau] */
    Result<bool> getAutoCompDisable(Positions pos = {}) const;

    /** [Jungfrau] TODO??? fix docs ? */
    void setAutoCompDisable(bool value, Positions pos = {});

    /** [Eiger]
     * @returns deadtime in ns, 0 = disabled
     */
    Result<int64_t> getRateCorrection(Positions pos = {}) const;

    /**
     * [Eiger] Set Rate correction
     * 0 disable correction, <0 set to default, >0 deadtime in ns
     */
    void setRateCorrection(int64_t dead_time_ns, Positions pos = {});

    /** [Eiger][Jungfrau] */
    Result<uint64_t> getStartingFrameNumber(Positions pos = {}) const;
    /** [Eiger][Jungfrau] */
    void setStartingFrameNumber(uint64_t value, Positions pos);

    /** [Eiger] */
    Result<bool> getTenGigaEnabled(Positions pos = {}) const;
    /** [Eiger] */
    void setTenGigaEnabled(bool value, Positions pos = {});

    /** [CTB] */
    Result<bool> getLEDEnable(Positions pos = {}) const;
    /** [CTB] */
    void setLEDEnable(bool enable, Positions pos = {});

    /**
     * [CTB] Set Digital IO Delay
     * cannot get
     * @param digital IO mask to select the pins
     * @param delay delay in ps(1 bit=25ps, max of 775 ps)
     */
    void setDigitalIODelay(uint64_t pinMask, int delay, Positions pos = {});

    /**************************************************
     *                                                *
     *    FILE, anything concerning file writing or   *
     *    reading goes here                           *
     *                                                *
     * ************************************************/
    Result<defs::fileFormat> getFileFormat(Positions pos = {}) const;
    /** default binary, Options: BINARY, HDF5 (library must be compiled with this option) */
    void setFileFormat(defs::fileFormat f, Positions pos = {});
    Result<std::string> getFilePath(Positions pos = {}) const;
    void setFilePath(const std::string &fname, Positions pos = {});
    Result<std::string> getFileNamePrefix(Positions pos = {}) const;
    /** default run */
    void setFileNamePrefix(const std::string &fname, Positions pos = {});
    Result<int> getFileIndex(Positions pos = {}) const;
    void setFileIndex(int i, Positions pos = {});
    Result<bool> getFileWrite(Positions pos = {}) const;
    /** default writes */ 
    void setFileWrite(bool value, Positions pos = {});
    Result<bool> getMasterFileWrite(Positions pos = {}) const;
    void setMasterFileWrite(bool value, Positions pos = {});
    Result<bool> getFileOverWrite(Positions pos = {}) const;
    /** default overwites */
    void setFileOverWrite(bool value, Positions pos = {});
    Result<int> getFramesPerFile(Positions pos = {}) const;
    void setFramesPerFile(int n, Positions pos = {});


    /**************************************************
     *                                                *
     *    RECEIVER CONFIG                             *
     *                                                *
     * ************************************************/
    /** true when slsReceiver is used */
    Result<bool> getUseReceiverFlag(Positions pos = {}) const;

    Result<std::string> printReceiverConfiguration(Positions pos = {}) const;

    Result<bool> getReceiverLock(Positions pos = {});

    /** locks receiver server to client IP */
    void setReceiverLock(bool value, Positions pos = {});

    Result<std::string> getReceiverLastClientIP(Positions pos = {}) const;

    void exitReceiver(Positions pos = {});

    void execReceiverCommand(const std::string &cmd, Positions pos = {});

    Result<int> getReceiverStreamingFrequency(Positions pos = {}) const;
    /** @param freq nth frame streamed out of receiver. 
     * If 0, streaming timer is the timeout,
     * after which current frame sent out. Default is 0 at 200 ms.
     * For every frame, set freq to 1.
     */
    void setReceiverStreamingFrequency(int freq, Positions pos = {});

    Result<int> getReceiverStreamingTimer(Positions pos = {}) const;
    /**
     * If receiver streaming frequency is 0 (default), then this timer between each
     * data stream is set. Default is 200 ms.
     */
    void setReceiverStreamingTimer(int time_in_ms, Positions pos = {});

    bool getDataStreamingToClient() const;
    void setDataStreamingToClient(bool value);
    Result<bool> getDataStreamingFromReceiver(Positions pos = {}) const;
    void setDataStreamingFromReceiver(bool value, Positions pos = {});

    Result<int> getReceiverFifoDepth(Positions pos = {}) const;
    void setReceiverFifoDepth(int nframes, Positions pos = {});
    Result<bool> getReceiverSilentMode(Positions pos = {}) const;
    void setReceiverSilentMode(bool value, Positions pos = {});

   Result<defs::frameDiscardPolicy>
    getReceiverFrameDiscardPolicy(Positions pos = {}) const;
    /** 
     * default NO_DISCARD 
     * Options: NO_DISCARD, DISCARD_EMPTY_FRAMES, DISCARD_PARTIAL_FRAMES
     */
    void setReceiverFrameDiscardPolicy(defs::frameDiscardPolicy f,
                                       Positions pos = {});
    Result<bool> getPartialFramesPadding(Positions pos = {}) const;
    /** padding enabled */
    void setPartialFramesPadding(bool value, Positions pos = {});

    /** [Eiger] */
    Result<bool> getRxPadDeactivatedMod(Positions pos = {}) const;

    /**
     * [Eiger] Set deactivated Receiver padding mode
     */
    void setRxPadDeactivatedMod(bool pad, Positions pos = {});


    Result<int64_t> getReceiverUDPSocketBufferSize(Positions pos = {}) const;

    void setReceiverUDPSocketBufferSize(int64_t udpsockbufsize,
                                        Positions pos = {});
    Result<int64_t>
    getReceiverRealUDPSocketBufferSize(Positions pos = {}) const;

    /**************************************************
     *                                                *
     *    ACQUISITION                                 *
     *                                                *
     * ************************************************/
    /**
     * Blocking call, starts the receiver and detector. 
     * Increments file index if file write enabled. 
     * Acquired the number of frames set.
     */
    void acquire();

    /**
     * Get the acquiring flag. When true the detector blocks
     * any attempt to start a new acquisition.
     */
    bool getAcquiringFlag() const;

    /**
     * Set the acquiring flag. This might have to done manually
     * after an acquisition was aborted.
     */
    void setAcquiringFlag(bool value);

    /**  Receiver starts listening to UDP packets from detector */
    void startReceiver(Positions pos = {});
    /**  Receiver stops listening to UDP packets from detector */
    void stopReceiver(Positions pos = {});
    /** Read back the run status of the receiver */
    Result<defs::runStatus> getReceiverStatus(Positions pos = {});

    Result<int> getFramesCaughtByReceiver(Positions pos = {}) const;
    Result<uint64_t> getReceiverCurrentFrameIndex(Positions pos = {}) const;
    void resetFramesCaught(Positions pos = {});

    /**************************************************
     *                                                *
     *    PATTERN                                     *
     *                                                *
     * ************************************************/
   
    /** [CTB] */
    void setPattern(const std::string &fname, Positions pos = {});

    /** [CTB] */
    Result<uint64_t> getPatternIOControl(Positions pos = {}) const;

    /** [CTB] */
    void setPatternIOControl(uint64_t word, Positions pos = {});

    /** [CTB] */
    Result<uint64_t> getPatternClockControl(Positions pos = {}) const;

    /** [CTB] */
    void setPatternClockControl(uint64_t word, Positions pos = {});

    /**
     * [CTB] 
     * Caution: If word is  -1  reads the addr (same as
     * executing the pattern)
     */
    void setPatternWord(int addr, uint64_t word, Positions pos = {});

    /**
     * [CTB] 
     * Options: level: -1 (complete pattern) and 0-2 levels
     * @returns array of start address, stop address and number of loops
     */
    Result<std::array<int, 3>> getPatternLoops(int level,
                                               Positions pos = {}) const;

    /**
     * [CTB] 
     * Options: start, stop, n : 0-2
     * level: -1 (complete pattern) and 0-2 levels
     */
    void setPatternLoops(int level, int start, int stop, int n,
                         Positions pos = {});

    /* [CTB] */
    Result<int> getPatternWaitAddr(int level, Positions pos = {}) const;

    /**
     * [CTB] 
     * Options: level 0-2 
     */
    void setPatternWaitAddr(int level, int addr, Positions pos = {});

    /** [CTB]  */
    Result<uint64_t> getPatternWaitTime(int level, Positions pos = {}) const;

    /**
     * [CTB] 
     * Options: level 0-2 
     */
    void setPatternWaitTime(int level, uint64_t t, Positions pos = {});

    /** [CTB] */
    Result<uint64_t> getPatternMask(Positions pos = {});

    /** [CTB] Sets the mask applied to every pattern to the selected bit mask */
    void setPatternMask(uint64_t mask, Positions pos = {});

    /** [CTB]  */
    Result<uint64_t> getPatternBitMask(Positions pos = {}) const;

    /**
     * [CTB] Sets the bitmask that the mask will be applied to for every
     * pattern
     */
    void setPatternBitMask(uint64_t mask, Positions pos = {});






  

 





































 
};

} // namespace sls