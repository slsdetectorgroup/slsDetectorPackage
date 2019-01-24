
#include "catch.hpp"

#include "logger.h"
#include <iostream>
#include <vector>
#include "slsDetector.h"
#include "sls_detector_defs.h"
#define VERBOSE

int main(){
    const std::string hostname = "beb083";
    // const std::string hostname = "129.129.202.194";
    auto type = slsDetector::getDetectorTypeAsEnum(hostname);
    auto d = slsDetector(type);
    d.setHostname(hostname.c_str());
    // auto d = slsDetector();

    std::cout << "type: " << d.getDetectorTypeAsString() << '\n';
    std::cout << "hostname: " << d.getHostname() << '\n';
    std::cout << "threshold: " << d.getThresholdEnergy() << '\n';
    std::cout << "exptime: " << d.setTimer(slsDetectorDefs::timerIndex::ACQUISITION_TIME) << '\n';
    
    
    // auto d2 = slsDetector(type, 0, 1);
    // d2.setHostname("beb098");.
    // auto d2 = slsDetector();
    // std::cout << "hn: " << d2.getHostname() << '\n';
    return 0;
}
