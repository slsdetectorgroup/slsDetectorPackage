#pragma once
#include <chrono>
#include <memory>
#include <vector>

class multiSlsDetector;
namespace sls {
using ns = std::chrono::nanoseconds;
using Positions = const std::vector<int> &;

class Detector {
    std::unique_ptr<multiSlsDetector> pimpl;

  public:
    Detector(int multi_id = 0);
    ~Detector();

    void setConfig(const std::string &fname);
    void freeSharedMemory();
    void acquire();

    // File
    std::vector<std::string> getFname() const;
    void setFname(const std::string &fname);
    void setFwrite(bool value, Positions pos = {});
    std::vector<bool> getFwrite(Positions pos = {}) const;

    // Time
    std::vector<ns> getExptime(Positions pos = {}) const;
    void setExptime(ns t, Positions pos = {});
    std::vector<ns> getSubExptime(Positions pos = {}) const;
    void setSubExptime(ns t, Positions pos = {});
    std::vector<ns> getPeriod(Positions pos = {}) const;
    void setPeriod(ns t, Positions pos = {});
};

} // namespace sls