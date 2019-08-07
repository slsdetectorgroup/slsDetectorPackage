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

    
};

} // namespace sls