
#include "catch.hpp"

#include "ClientSocket.h"
#include "logger.h"
#include "slsDetector.h"
#include "multiSlsDetector.h"
#include "sls_detector_defs.h"

#include "Timer.h"
#include "sls_detector_funcs.h"
#include <iostream>
#include <vector>
#define VERBOSE

int main() {
    // const std::string hostname = "beb083";
    multiSlsDetector d;
    d.setOnline(1);
    auto s = d.checkOnline();
    std::cout << "s: " << s << "\n";
    // auto d = slsDetector(hostname);
    
    // d.setOnline(1);
    // std::cout << "type: " << d.getDetectorTypeAsString() << '\n';
    // std::cout << "hostname: " << d.getHostname() << '\n';
    // std::cout << "receiver: " << d.getReceiverOnline() << '\n';
   
    // std::cout << "control: " << d.getControlPort() << '\n';
    // std::cout << "stop: " << d.getStopPort() << '\n';
    // std::cout << "receiver: " << d.getReceiverPort() << '\n';
    // std::cout << "exptime: " << d.setTimer(slsDetectorDefs::timerIndex::ACQUISITION_TIME) << '\n';



    // auto d2 = slsDetector(type, 0, 1);
    // d2.setHostname("beb098");.
    // auto d2 = slsDetector();
    // std::cout << "hn: " << d2.getHostname() << '\n';

    // sls::Timer t;
    // for (int i = 0; i != 100; ++i) {
    //     int fnum = 1;
    //     int ret = slsDetectorDefs::FAIL;
    //     slsDetectorDefs::detectorType retval = slsDetectorDefs::detectorType::GENERIC;

    //     auto cs = sls::ClientSocket("beb083", 1952);
    //     cs.sendData(reinterpret_cast<char *>(&fnum), sizeof(fnum));
    //     cs.receiveData(reinterpret_cast<char *>(&ret), sizeof(ret));
    //     cs.receiveData(reinterpret_cast<char *>(&retval), sizeof(retval));
    //     std::cout << "retval: " << retval << '\n';
    // }
    // t.print_elapsed();

    return 0;
}
