#include "catch.hpp"

#include "multiSlsDetector.h"
#include "string_utils.h"

#include "tests/globals.h"

#include <iostream>

TEST_CASE("Initialize a multi detector", "[.integration][.multi]") {
    auto hostnames = sls::split(hostname, '+');

    multiSlsDetector d(0, true, true);
    d.setHostname(hostname.c_str());
    REQUIRE(d.setOnline() == true); // get!

    CHECK(d.getHostname() == hostname);
    for (size_t i = 0; i != hostnames.size(); ++i) {
        CHECK(d.getHostname(i) == hostnames[i]);
    }

    CHECK(d.getDetectorTypeAsEnum() == type);
    CHECK(d.getDetectorTypeAsString() == detector_type);

    CHECK(d.getNumberOfDetectors() == hostnames.size());
    d.freeSharedMemory();
}

TEST_CASE("Set and get dacs", "[.integration][.multi]"){
    multiSlsDetector d(0, true, true);
    d.setHostname(hostname.c_str());

    switch(type){
        case dt::EIGER:

            d.setDAC(3500, di::E_Vrf, 0);
            CHECK(d.setDAC(-1, di::E_Vrf, 0) == 3500);

            
            d.setDAC(1500, di::E_Vcmp_ll, 0);
            CHECK(d.setDAC(-1, di::E_Vcmp_ll, 0) == 1500);
            d.setDAC(1400, di::E_Vcmp_lr, 0);
            CHECK(d.setDAC(-1, di::E_Vcmp_lr, 0) == 1400);
            d.setDAC(1300, di::E_Vcmp_rl, 0);
            CHECK(d.setDAC(-1, di::E_Vcmp_rl, 0) == 1300);
            d.setDAC(1200, di::E_Vcmp_rr, 0);
            CHECK(d.setDAC(-1, di::E_Vcmp_rr, 0) == 1200);

            auto th = 1000;
            d.setDAC(th, di::THRESHOLD, 0);
            CHECK(d.setDAC(-1, di::THRESHOLD, 0) == th);
            CHECK(d.setDAC(-1, di::E_Vcmp_ll, 0) == th);
            CHECK(d.setDAC(-1, di::E_Vcmp_lr, 0) == th);
            CHECK(d.setDAC(-1, di::E_Vcmp_rl, 0) == th);
            CHECK(d.setDAC(-1, di::E_Vcmp_rr, 0) == th);
        break;

    }


    d.freeSharedMemory();
}

TEST_CASE("Timers", "[.integration][.multi]") {
    multiSlsDetector d(0, true, true);
    d.setHostname(hostname.c_str());

    int n_frames = 3;
    d.setNumberOfFrames(n_frames);
    CHECK(d.setNumberOfFrames() == n_frames);

    double exptime = 1;
    d.setExposureTime(exptime, true);
    CHECK(d.setExposureTime(-1, true) == exptime);


    d.freeSharedMemory();
}