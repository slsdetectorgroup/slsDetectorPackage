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


    void acquire();


    //vector of size
    std::vector<ns> getExptime(Positions pos = {});
    void setExptime(ns t, Positions pos = {});




};

} // namespace sls