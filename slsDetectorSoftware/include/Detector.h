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

    std::vector<ns> setExptime(ns t, int det_id = -1);
    std::vector<ns> getExptime(int det_id = -1) const;



    //one value or vector
    void setExptime(ns t, int det_id=-1) const;
    std::vector<ns> getExptime(int det_id = -1) const; 


    //vector of size
    std::vector<ns> getExptime(const std::vector<int>& det_id = {}) const;
    void setExptime(ns t, const std::vector<int>& det_id = {});




};

} // namespace sls