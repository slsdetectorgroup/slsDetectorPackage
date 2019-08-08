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

    // Erik

    /** CTB only.Sets the mask applied to every pattern. */
    void setPatternMask(uint64_t mask, Positions pos = {});

    /** CTB only. Gets the mask applied to every pattern. */
    Result<uint64_t> getPatternMask(Positions pos = {});

    /**
     * CTB only. Sets the bitmask that the mask will be applied to for every
     * pattern.
     * @param mask mask to select bits
     */
    void setPatternBitMask(uint64_t mask, Positions pos = {});

    /**
     * CTB only. Gets the bits that the mask will be applied to for every
     * pattern
     */
    Result<uint64_t> getPatternBitMask(Positions pos = {}) const;

    /**
     * CTB only. Enable or disable the LED
     * @param enable true to switch on, false to switch off
     */
    void setLEDEnable(bool enable, Positions pos = {});

    /**
     * CTB only. Get LED enable.
     */
    Result<bool> getLEDEnable(Positions pos = {}) const;

    /**
     * CTB only. Set Digital IO Delay
     * @param digital IO mask to select the pins
     * @param delay delay in ps(1 bit=25ps, max of 775 ps)
     */
    void setDigitalIODelay(uint64_t pinMask, int delay, Positions pos = {});
};

} // namespace sls