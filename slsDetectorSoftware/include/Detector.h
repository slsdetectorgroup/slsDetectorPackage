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

    void setConfig(const std::string& fname);
    void acquire();

    //File
    void setFname(const std::string& fname);
    std::string getFname();
    void setFwrite(bool value, Positions pos = {});
    bool getFwrite(Positions pos = {}) const;

    //Time
    std::vector<ns> getExptime(Positions pos = {}) const;
    void setExptime(ns t, Positions pos = {});
    std::vector<ns> getPeriod(Positions pos = {}) const;
    void setPeriod(ns t, Positions pos = {});




};

} // namespace sls