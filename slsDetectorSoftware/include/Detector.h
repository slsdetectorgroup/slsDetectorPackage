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
class Detector {
    std::unique_ptr<multiSlsDetector> pimpl;

  public:
    Detector(int multi_id = 0);
    ~Detector();

    // Acquisition
    void acquire();
    void startReceiver(Positions pos = {});
    void stopReceiver(Positions pos = {});
    Result<defs::runStatus> getReceiverStatus(Positions pos = {});

    // Configuration
    void freeSharedMemory();
    void setConfig(const std::string &fname);
    Result<std::string> getHostname(Positions pos = {}) const;
    // void setHostname(Positions pos = {});

    Result<uint64_t> getStartingFrameNumber(Positions pos = {}) const;
    void setStartingFrameNumber(uint64_t value, Positions pos);

    // Bits and registers
    void clearBit(uint32_t addr, int bit, Positions pos = {});
    void setBit(uint32_t addr, int bit, Positions pos = {});
    Result<uint32_t> getRegister(uint32_t addr, Positions pos = {});

    // File
    Result<std::string> getFname() const;
    void setFname(const std::string &fname);
    Result<std::string> getFpath() const;
    void setFpath(const std::string &fname);
    Result<bool> getFwrite(Positions pos = {}) const;
    void setFwrite(bool value, Positions pos = {});
    Result<bool> getFileOverWrite(Positions pos = {}) const;
    void setFileOverWrite(bool value, Positions pos = {});


    // Time
    Result<ns> getExptime(Positions pos = {}) const;
    void setExptime(ns t, Positions pos = {});
    Result<ns> getSubExptime(Positions pos = {}) const;
    void setSubExptime(ns t, Positions pos = {});
    Result<ns> getPeriod(Positions pos = {}) const;
    void setPeriod(ns t, Positions pos = {});
};

} // namespace sls