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

TEST_CASE("Set and get dacs", "[.integration][.multi]") {
    multiSlsDetector d(0, true, true);
    d.setHostname(hostname.c_str());
    auto th = 1000;
    switch (type) {
    case dt::EIGER:
        // set and read back each individual dac of EIGER
        d.setDAC(0, di::E_SvP, 0);
        CHECK(d.setDAC(-1, di::E_SvP, 0) == 0);
        d.setDAC(4000, di::E_SvN, 0);
        CHECK(d.setDAC(-1, di::E_SvN, 0) == 4000);
        d.setDAC(2000, di::E_Vtr, 0);
        CHECK(d.setDAC(-1, di::E_Vtr, 0) == 2000);
        d.setDAC(3500, di::E_Vrf, 0);
        CHECK(d.setDAC(-1, di::E_Vrf, 0) == 3500);
        d.setDAC(1400, di::E_Vrs, 0);
        CHECK(d.setDAC(-1, di::E_Vrs, 0) == 1400);
        d.setDAC(2556, di::E_Vtgstv, 0);
        CHECK(d.setDAC(-1, di::E_Vtgstv, 0) == 2556);
        d.setDAC(1500, di::E_Vcmp_ll, 0);
        CHECK(d.setDAC(-1, di::E_Vcmp_ll, 0) == 1500);
        d.setDAC(1400, di::E_Vcmp_lr, 0);
        CHECK(d.setDAC(-1, di::E_Vcmp_lr, 0) == 1400);
        d.setDAC(4000, di::E_cal, 0);
        CHECK(d.setDAC(-1, di::E_cal, 0) == 4000);
        d.setDAC(1300, di::E_Vcmp_rl, 0);
        CHECK(d.setDAC(-1, di::E_Vcmp_rl, 0) == 1300);
        d.setDAC(1200, di::E_Vcmp_rr, 0);
        CHECK(d.setDAC(-1, di::E_Vcmp_rr, 0) == 1200);
        d.setDAC(1100, di::E_rxb_rb, 0);
        CHECK(d.setDAC(-1, di::E_rxb_rb, 0) == 1100);
        d.setDAC(1100, di::E_rxb_lb, 0);
        CHECK(d.setDAC(-1, di::E_rxb_lb, 0) == 1100);
        d.setDAC(1500, di::E_Vcp, 0);
        CHECK(d.setDAC(-1, di::E_Vcp, 0) == 1500);
        d.setDAC(2000, di::E_Vcn, 0);
        CHECK(d.setDAC(-1, di::E_Vcn, 0) == 2000);
        d.setDAC(1550, di::E_Vis, 0);
        CHECK(d.setDAC(-1, di::E_Vis, 0) == 1550);
        d.setDAC(660, di::IO_DELAY, 0);
        CHECK(d.setDAC(-1, di::IO_DELAY, 0) == 660);

        // setting threshold sets all individual vcmp
        d.setDAC(th, di::THRESHOLD, 0);
        CHECK(d.setDAC(-1, di::THRESHOLD, 0) == th);
        CHECK(d.setDAC(-1, di::E_Vcmp_ll, 0) == th);
        CHECK(d.setDAC(-1, di::E_Vcmp_lr, 0) == th);
        CHECK(d.setDAC(-1, di::E_Vcmp_rl, 0) == th);
        CHECK(d.setDAC(-1, di::E_Vcmp_rr, 0) == th);

        // different values gives -1
        if (d.getNumberOfDetectors() > 1) {
            d.setDAC(1600, di::E_Vcmp_ll, 0, 0);
            d.setDAC(1700, di::E_Vcmp_ll, 0, 1);
            CHECK(d.setDAC(-1, di::E_Vcmp_ll, 0, 0) == 1600);
            CHECK(d.setDAC(-1, di::E_Vcmp_ll, 0, 1) == 1700);
            CHECK(d.setDAC(-1, di::E_Vcmp_ll, 0) == -1);
            CHECK(d.setDAC(-1, di::THRESHOLD, 0) == -1);
            CHECK(d.setDAC(-1, di::THRESHOLD, 0, 0) == -1);
            CHECK(d.setDAC(-1, di::THRESHOLD, 0, 1) == -1);

        }

        break;
    case dt::JUNGFRAU:
        CHECK(false);
        break;
    case dt::GOTTHARD:
        CHECK(false);
        break;
    case dt::CHIPTESTBOARD:
        CHECK(false);
        break;
    case dt::MOENCH:
        CHECK(false);
        break;
    case dt::GENERIC:
        CHECK(false);
        break;
    case dt::GET_DETECTOR_TYPE:
        CHECK(false);
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