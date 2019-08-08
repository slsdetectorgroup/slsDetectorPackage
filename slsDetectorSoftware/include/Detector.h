#pragma once
#include "Result.h"
#include "sls_detector_defs.h"
#include <chrono>
#include <memory>
#include <vector>

class multiSlsDetector;
namespace sls {
using ns = std::chrono::nanoseconds;
using Positions = const std::vector<int> &;
using defs = slsDetectorDefs;

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

    // Acquisition

    /**
     * Blocking call, starts the receiver and detector. Acquired
     * the number of frames set.
     */
    void acquire();
    void startReceiver(Positions pos = {});
    void stopReceiver(Positions pos = {});

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

    /** Read back the run status of the receiver */
    Result<defs::runStatus> getReceiverStatus(Positions pos = {});

    // Configuration

    /**
     * Frees the shared memory of this detector and all modules
     * belonging to it.
     */
    void freeSharedMemory();
    void setConfig(const std::string &fname);
    Result<std::string> getHostname(Positions pos = {}) const;
    // void setHostname(Positions pos = {});

    Result<uint64_t> getStartingFrameNumber(Positions pos = {}) const;
    void setStartingFrameNumber(uint64_t value, Positions pos);

    // Bits and registers

    /**
     * Clears (sets to 0) bit number bitnr in register at addr.
     * @param addr address of the register
     * @param bitnr bit number to clear
     * @param pos detector position
     */
    void clearBit(uint32_t addr, int bitnr, Positions pos = {});

    /**
     * Sets bit number bitnr in register at addr to 1
     * @param addr address of the register
     * @param bitnr bit number to clear
     * @param pos detector position
     */
    void setBit(uint32_t addr, int bitnr, Positions pos = {});

    /**
     * Reads 32 bit register from detector
     * @param addr address of the register
     * @returns value read from register
     */
    Result<uint32_t> getRegister(uint32_t addr, Positions pos = {});

    /**************************************************
     *                                                *
     *    FILE, anything concerning file writing or   *
     *    reading goes here                           *
     *                                                *
     * ************************************************/

    /**
     * Returns receiver file name prefix. The actual file name
     * contains module id and file index as well.
     * @param pos detector positions
     * @returns file name prefix
     */
    Result<std::string> getFileName(Positions pos = {}) const;

    /**
     * Sets the receiver file name prefix
     * @param fname file name prefix
     */
    void setFileName(const std::string &fname, Positions pos = {});
    Result<std::string> getFilePath(Positions pos = {}) const;
    void setFilePath(const std::string &fname, Positions pos = {});
    Result<bool> getFileWrite(Positions pos = {}) const;
    void setFileWrite(bool value, Positions pos = {});
    Result<bool> getFileOverWrite(Positions pos = {}) const;
    void setFileOverWrite(bool value, Positions pos = {});

    // Time
    Result<ns> getExptime(Positions pos = {}) const;
    void setExptime(ns t, Positions pos = {});
    Result<ns> getSubExptime(Positions pos = {}) const;
    void setSubExptime(ns t, Positions pos = {});
    Result<ns> getPeriod(Positions pos = {}) const;
    void setPeriod(ns t, Positions pos = {});

    // dhanya
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
     * Frees shared memory and adds detectors to the list
     * Also updates local detector cache
     * @param name hostnames for the positions given
     */
    void setHostname(const std::vector<std::string> &value);

    /**
     * Get Detector type as an enum
     * @returns detector type
     */
    defs::detectorType getDetectorTypeAsEnum() const;

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
    int getTotalNumberOfDetectors() const;

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
    void setThresholdEnergy(int value, defs::detectorSettings sett = defs::GET_SETTINGS,
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

    // Erik

    Result<int> getFramesCaughtByReceiver(Positions pos = {}) const;

    Result<uint64_t> getReceiverCurrentFrameIndex(Positions pos = {}) const;

    void resetFramesCaught(Positions pos = {});

    // TODO!
    // int createReceivingDataSockets(const bool destroy = false);
    // void readFrameFromReceiver();

    void setMasterFileWrite(bool value, Positions pos = {});

    Result<bool> getMasterFileWrite(Positions pos = {}) const;

    void setReceiverStreamingFrequency(int freq = -1, int detPos = -1);
    /**
     * [All] If receiver streaming frequency is 0, then this timer between each
     * data stream is set. Default is 500 ms.
     */
    void setReceiverStreamingTimer(int time_in_ms = 500, Positions pos = {});

    /** [All] */
    Result<int> getReceiverStreamingTimer(Positions pos = {}) const;

    // TODO!
    // int enableDataStreamingToClient(int enable = -1);
    // int enableDataStreamingFromReceiver(int enable = -1, int detPos = -1)

    /** [TODO! All?] */
    void setTenGigaEnabled(bool value, Positions pos = {});

    /** [TODO! All?] */
    Result<bool> getTenGigaEnabled(Positions pos = {}) const;

    /** [All] */
    void setReceiverFifoDepth(int nframes, Positions pos = {});

    /** [All] */
    Result<int> getReceiverFifoDepth(Positions pos = {}) const;

    /** [All] */
    void setReceiverSilentMode(bool value, Positions pos = {});

    /** [All] */
    Result<bool> getReceiverSilentMode(Positions pos = {}) const;

    /** [CTB] */
    void setPattern(const std::string &fname, Positions pos = {});

    /** [CTB] */
    void setPatternIOControl(uint64_t word, Positions pos = {});

    /** [CTB] */
    Result<uint64_t> getPatternIOControl(Positions pos = {}) const;

    /** [CTB] */
    void setPatternClockControl(uint64_t word, Positions pos = {});

    /** [CTB] */
    Result<uint64_t> getPatternClockControl(Positions pos = {}) const;

    /**
     * [CTB] Writes a pattern word
     * @param addr address of the word
     * @param word word to be written, -1  reads the addr (same as
     * executing the pattern)
     * @param pos detector position
     * @returns actual value
     */
    void setPatternWord(int addr, uint64_t word, Positions pos = {});

    // TODO! Does this have side effects?
    // uint64_t getPatternWord()...

    /**
     * [CTB] Sets the pattern or loop limits.
     * @param level -1 complete pattern, 0,1,2, loop level
     * @param start start address for level 0-2
     * @param stop stop address for level 0-2
     * @param n number of loops for level 0-2
     * @param pos detector position
     */
    void setPatternLoops(int level, int start, int stop, int n,
                         Positions pos = {});

    /**
     * [CTB] Gets the pattern loop limits
     * @param level  -1 complete pattern, 0,1,2, loop level
     * @param pos detector positions
     * @returns array of start address, stop address and number of loops
     */
    Result<std::array<int, 3>> getPatternLoops(int level,
                                               Positions pos = {}) const;

    /**
     * [CTB] Sets the wait address
     * @param level  0,1,2, wait level
     * @param addr wait address
     * @param pos detector position
     * @returns actual value
     */
    void setPatternWaitAddr(int level, int addr, Positions pos = {});

    /* [CTB] */
    Result<int> getPatternWaitAddr(int level, Positions pos = {}) const;

    /**
     * [CTB] Sets the wait time
     * @param level  0,1,2, wait level
     * @param t wait time
     * @param pos detector position
     */
    void setPatternWaitTime(int level, uint64_t t, Positions pos = {});

    /** [CTB]  */
    Result<uint64_t> getPatternWaitTime(int level, Positions pos = {}) const;

    /** [CTB] Sets the mask applied to every pattern. */
    void setPatternMask(uint64_t mask, Positions pos = {});

    /** [CTB] Gets the mask applied to every pattern. */
    Result<uint64_t> getPatternMask(Positions pos = {});

    /**
     * [CTB] Sets the bitmask that the mask will be applied to for every
     * pattern.
     * @param mask mask to select bits
     */
    void setPatternBitMask(uint64_t mask, Positions pos = {});

    /**
     * [CTB] Gets the bits that the mask will be applied to for every
     * pattern
     */
    Result<uint64_t> getPatternBitMask(Positions pos = {}) const;

    /** [CTB] Enable or disable the LED */
    void setLEDEnable(bool enable, Positions pos = {});

    /** [CTB] Get LED enable. */
    Result<bool> getLEDEnable(Positions pos = {}) const;

    /**
     * [CTB] Set Digital IO Delay
     * @param digital IO mask to select the pins
     * @param delay delay in ps(1 bit=25ps, max of 775 ps)
     */
    void setDigitalIODelay(uint64_t pinMask, int delay, Positions pos = {});

    Result<int> getFileIndex(Positions pos = {}) const;

    void setFileIndex(int i, Positions pos = {});

    Result<defs::fileFormat> getFileFormat(Positions pos = {}) const;

    void setFileFormat(defs::fileFormat f, Positions pos = {});

    Result<bool> getPartialFramesPadding(Positions pos = {}) const;

    void setPartialFramesPadding(bool value, Positions pos = {});

    void setReceiverFrameDiscardPolicy(defs::frameDiscardPolicy f,
                                       Positions pos = {});

    Result<defs::frameDiscardPolicy>
    getReceiverFrameDiscardPolicy(Positions pos = {}) const;

    void setFramesPerFile(int n, Positions pos = {});

    Result<int> getFramesPerFile(Positions pos = {}) const;

    // void execReceiverCommand(const std::string &cmd, int detPos = -1);
    // void exitReceiver(int detPos = -1);

    Result<std::string> getReceiverLastClientIP(Positions pos = {}) const;

    void setReceiverLock(bool value, Positions pos = {});

    Result<bool> getReceiverLock(Positions pos = {});

    /** true when receiver is used otherwise false */
    Result<bool> getUseReceiverFlag(Positions pos = {}) const;

    void printReceiverConfiguration(Positions pos = {}) const;

    /** [Eiger]
     * @returns deadtime in ns, 0 = disabled
     */
    Result<int64_t> getRateCorrection(Positions pos = {}) const;

    /**
     * [Eiger] Set Rate correction
     * 0 disable correction, <0 set to default, >0 deadtime in ns
     */
    void setRateCorrection(int64_t dead_time_ns, Positions pos = {});

    /** [Jungfrau] TODO??? fix docs */
    void setAutoCompDisable(bool value, Positions pos = {});

    Result<bool> getAutoCompDisable(Positions pos = {}) const;

    void setPowerChip(bool on, Positions pos = {});

    Result<bool> getPowerChip(Positions pos = {}) const;

    /**
     * Updates the firmware, detector server and then reboots detector
     * controller blackfin. (Not Eiger)
     * @param sname name of detector server binary
     * @param hostname name of pc to tftp from
     * @param fname programming file name
     * @param pos detector positions
     */
    void updateFirmwareAndServer(const std::string &sname,
                                 const std::string &hostname,
                                 const std::string &fname, Positions pos = {});

    /** [not Eiger] TODO! is this needed?*/
    void rebootController(Positions pos = {});

    /** Copy detector server to detector */
    void copyDetectorServer(const std::string &fname,
                            const std::string &hostname, Positions pos = {});

    /** [not Eiger] */
    void resetFPGA(Positions pos = {});

    /** [not Eiger] */
    void programFPGA(const std::string &fname, Positions pos = {});

    /**
     * [Jungfrau] Set first storage cell of the series (Jungfrau)
     * @param value storage cell index. Value can be 0 to 15.
     */
    void setStoragecellStart(int cell, Positions pos = {});

    Result<int> getStorageCellStart(Positions pos = {}) const;

    /** [Jungfrau] 1 there was an temperature event */
    void setTemperatureEvent(int val, Positions pos = {});

    /** [Jungfrau] */
    Result<int> getTemperatureEvent(Positions pos = {}) const;

    /** [Jungfrau] */
    void setTemperatureControl(bool enable, Positions pos = {});

    /** [Jungfrau] */
    Result<bool> getTemperatureControl(Positions pos = {}) const;

    /**
     * [Jungfrau]Set threshold temperature
     * @param val value in millidegrees TODO! Verify
     */
    void setThresholdTemperature(int temp, Positions pos = {});

    Result<int> getThresholdTemperature(Positions pos = {}) const;

    /** [Eiger] */
    void pulseChip(int n, Positions pos = {});

    /** 
     * [Eiger] Pulse Pixel and move by a relative value
     * @param n is number of times to pulse
     * @param x is relative x value
     * @param y is relative y value
     */
    void pulsePixelNMove(int n, int x, int y, Positions pos = {});

    /**
     * [Eiger] Pulse Pixel
     * @param n is number of times to pulse
     * @param x is x coordinate
     * @param y is y coordinate
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void pulsePixel(int n, int x, int y, Positions pos = {});

    /**[Eiger] Returns energies in eV where the module is trimmed */
    Result<std::vector<int>> getTrimEn(Positions pos = {}) const;

    /** [Eiger] Set the energies where the detector is trimmed */
    void setTrimEn(std::vector<int> energies, Positions pos = {});

    /** 
     * [Eiger] not 4 bit mode 
     * Fills in gap pixels in data
    */
    void setGapPixelsEnable(bool enable, Positions pos = {});

    Result<bool> getGapPixelEnable(Positions pos = {}) const;
};

} // namespace sls