// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "DetectorImpl.h"
#include "catch.hpp"
#include "sls/string_utils.h"
#include "tests/globals.h"
#include <iostream>

namespace sls {

class MultiDetectorFixture {
  protected:
    DetectorImpl d;

  public:
    MultiDetectorFixture() : d(0, true, true) {
        d.setHostname(test::hostname.c_str());
        if (test::my_ip != "undefined")
            d.setReceiverHostname(test::my_ip);
    }
    ~MultiDetectorFixture() { d.freeSharedMemory(); }
};

TEST_CASE_METHOD(MultiDetectorFixture, "Set and get dacs",
                 "[.eigerintegration][cli]") {
    auto th = 1000;

    // set and read back each individual dac of EIGER
    d.setDAC(0, di::SVP, 0);
    CHECK(d.setDAC(-1, di::SVP, 0) == 0);
    d.setDAC(4000, di::SVN, 0);
    CHECK(d.setDAC(-1, di::SVN, 0) == 4000);
    d.setDAC(2000, di::VTR, 0);
    CHECK(d.setDAC(-1, di::VTR, 0) == 2000);
    d.setDAC(3500, di::VRF, 0);
    CHECK(d.setDAC(-1, di::VRF, 0) == 3500);
    d.setDAC(1400, di::VRS, 0);
    CHECK(d.setDAC(-1, di::VRS, 0) == 1400);
    d.setDAC(2556, di::VTGSTV, 0);
    CHECK(d.setDAC(-1, di::VTGSTV, 0) == 2556);
    d.setDAC(1500, di::VCMP_LL, 0);
    CHECK(d.setDAC(-1, di::VCMP_LL, 0) == 1500);
    d.setDAC(1400, di::VCMP_LR, 0);
    CHECK(d.setDAC(-1, di::VCMP_LR, 0) == 1400);
    d.setDAC(4000, di::CAL, 0);
    CHECK(d.setDAC(-1, di::CAL, 0) == 4000);
    d.setDAC(1300, di::VCMP_RL, 0);
    CHECK(d.setDAC(-1, di::VCMP_RL, 0) == 1300);
    d.setDAC(1200, di::VCMP_RR, 0);
    CHECK(d.setDAC(-1, di::VCMP_RR, 0) == 1200);
    d.setDAC(1100, di::RXB_RB, 0);
    CHECK(d.setDAC(-1, di::RXB_RB, 0) == 1100);
    d.setDAC(1100, di::RXB_LB, 0);
    CHECK(d.setDAC(-1, di::RXB_LB, 0) == 1100);
    d.setDAC(1500, di::VCP, 0);
    CHECK(d.setDAC(-1, di::VCP, 0) == 1500);
    d.setDAC(2000, di::VCN, 0);
    CHECK(d.setDAC(-1, di::VCN, 0) == 2000);
    d.setDAC(1550, di::VIS, 0);
    CHECK(d.setDAC(-1, di::VIS, 0) == 1550);
    d.setDAC(660, di::IO_DELAY, 0);
    CHECK(d.setDAC(-1, di::IO_DELAY, 0) == 660);

    // setting threshold sets all individual vcmp
    d.setDAC(th, di::THRESHOLD, 0);
    CHECK(d.setDAC(-1, di::THRESHOLD, 0) == th);
    CHECK(d.setDAC(-1, di::VCMP_LL, 0) == th);
    CHECK(d.setDAC(-1, di::VCMP_LR, 0) == th);
    CHECK(d.setDAC(-1, di::VCMP_RL, 0) == th);
    CHECK(d.setDAC(-1, di::VCMP_RR, 0) == th);

    // different values gives -1
    if (d.getNumberOfDetectors() > 1) {
        d.setDAC(1600, di::VCMP_LL, 0, 0);
        d.setDAC(1700, di::VCMP_LL, 0, 1);
        CHECK(d.setDAC(-1, di::VCMP_LL, 0, 0) == 1600);
        CHECK(d.setDAC(-1, di::VCMP_LL, 0, 1) == 1700);
        CHECK(d.setDAC(-1, di::VCMP_LL, 0) == -1);
        CHECK(d.setDAC(-1, di::THRESHOLD, 0) == -1);
        CHECK(d.setDAC(-1, di::THRESHOLD, 0, 0) == -1);
        CHECK(d.setDAC(-1, di::THRESHOLD, 0, 1) == -1);
    }
}

TEST_CASE_METHOD(MultiDetectorFixture, "Read temperatures",
                 "[.eigerintegration][cli]") {
    std::vector<di> tempindex{di::TEMPERATURE_FPGA, di::TEMPERATURE_FPGA2,
                              di::TEMPERATURE_FPGA3};
    for (auto index : tempindex) {
        for (int i = 0; i != d.getNumberOfDetectors(); ++i) {
            double temp = static_cast<double>(d.getADC(index, 0)) / 1000;
            CHECK(temp > 20);
            CHECK(temp < 60);
        }
    }
}

int to_time(uint32_t reg) {
    uint32_t clocks = reg >> 3;
    uint32_t exponent = (reg & 0b111) + 1;
    return clocks * pow(10, exponent);
}

TEST_CASE_METHOD(MultiDetectorFixture, "Read/write register",
                 "[.eigerintegration][cli]") {
    d.setNumberOfFrames(1);
    d.setExposureTime(10000);
    d.acquire();
    CHECK(to_time(d.readRegister(0x4, 0)) == 10000);

    d.writeRegister(0x4, 500);
    CHECK(d.readRegister(0x4) == 500);
}

TEST_CASE_METHOD(MultiDetectorFixture, "Set dynamic range",
                 "[.eigerintegration][cli][dr]") {
    std::vector<int> dynamic_range{4, 8, 16, 32};
    for (auto dr : dynamic_range) {
        d.setDynamicRange(dr);
        CHECK(d.setDynamicRange() == dr);
    }
}

TEST_CASE_METHOD(MultiDetectorFixture, "Set clock divider",
                 "[.eigerintegration][cli][this]") {
    for (int i = 0; i != 3; ++i) {
        d.setSpeed(sv::CLOCK_DIVIDER, i);
        CHECK(d.setSpeed(sv::CLOCK_DIVIDER) == i);
    }
}

TEST_CASE_METHOD(MultiDetectorFixture, "Get time left",
                 "[.eigerintegration][cli]") {
    CHECK_THROWS(d.getTimeLeft(ti::PROGRESS));
}

TEST_CASE_METHOD(MultiDetectorFixture, "Get ID", "[.eigerintegration][cli]") {
    std::string hn = test::hostname;
    hn.erase(std::remove(begin(hn), end(hn), 'b'), end(hn));
    hn.erase(std::remove(begin(hn), end(hn), 'e'), end(hn));
    auto hostnames = split(hn, '+');
    CHECK(hostnames.size() == d.getNumberOfDetectors());
    for (int i = 0; i != d.getNumberOfDetectors(); ++i) {
        CHECK(d.getId(defs::DETECTOR_SERIAL_NUMBER, 0) ==
              std::stoi(hostnames[0]));
    }
}

TEST_CASE_METHOD(MultiDetectorFixture, "Lock server",
                 "[.eigerintegration][cli]") {

    d.lockServer(1);
    CHECK(d.lockServer() == 1);
    d.lockServer(0);
    CHECK(d.lockServer() == 0);
}

TEST_CASE_METHOD(MultiDetectorFixture, "Settings", "[.eigerintegration][cli]") {
    CHECK(d.setSettings(defs::STANDARD) == defs::STANDARD);
}

TEST_CASE_METHOD(MultiDetectorFixture, "Set readout flags",
                 "[.eigerintegration][cli]") {
    d.setReadOutFlags(defs::PARALLEL);
    CHECK((d.setReadOutFlags() & defs::PARALLEL));

    d.setReadOutFlags(defs::NONPARALLEL);
    CHECK_FALSE((d.setReadOutFlags() & defs::PARALLEL));
    CHECK((d.setReadOutFlags() & defs::NONPARALLEL));
}

// TEST_CASE_METHOD(MultiDetectorFixture, "Flow control and tengiga",
//                  "[.eigerintegration][cli]") {
//     d.setFlowControl10G(1);
//     CHECK(d.setFlowControl10G() == 1);
//     d.setFlowControl10G(0);
//     CHECK(d.setFlowControl10G() == 0);

//     d.enableTenGigabitEthernet(1);
//     CHECK(d.enableTenGigabitEthernet() == 1);
//     d.enableTenGigabitEthernet(0);
//     CHECK(d.enableTenGigabitEthernet() == 0);
// }

TEST_CASE_METHOD(MultiDetectorFixture, "activate", "[.eigerintegration][cli]") {
    d.activate(0);
    CHECK(d.activate() == 0);
    d.activate(1);
    CHECK(d.activate() == 1);
}

TEST_CASE_METHOD(MultiDetectorFixture, "all trimbits",
                 "[.eigerintegration][cli]") {
    d.setAllTrimbits(32);
    CHECK(d.setAllTrimbits(-1) == 32);
}

TEST_CASE_METHOD(MultiDetectorFixture, "rate correction",
                 "[.eigerintegration][cli]") {
    d.setRateCorrection(200);
    CHECK(d.getRateCorrection() == 200);
}

} // namespace sls
