#include "catch.hpp"
#include "multiSlsDetector.h"
#include "string_utils.h"
#include "tests/globals.h"
#include <iostream>

class MultiDetectorFixture {
  protected:
    multiSlsDetector d;

  public:
    MultiDetectorFixture() : d(0, true, true) {
        d.setHostname(hostname.c_str());
    }
    ~MultiDetectorFixture() { d.freeSharedMemory(); }
};

TEST_CASE_METHOD(MultiDetectorFixture, "Set and get dacs",
                 "[.eigerintegration][cli]") {
    auto th = 1000;

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
    d.setTimer(ti::MEASUREMENTS_NUMBER, 1);
    d.setNumberOfFrames(1);
    d.setExposureTime(10000);
    d.acquire();
    CHECK(to_time(d.readRegister(0x4, 0)) == 10000);

    d.writeRegister(0x4, 500);
    CHECK(d.readRegister(0x4) == 500);
}

TEST_CASE_METHOD(MultiDetectorFixture, "Set dynamic range",
                 "[.eigerintegration][cli]") {
    std::vector<int> dynamic_range{4, 8, 16, 32};
    for (auto dr : dynamic_range) {
        d.setDynamicRange(dr);
        CHECK(d.setDynamicRange() == dr);
    }
}

TEST_CASE_METHOD(MultiDetectorFixture, "Set clock divider",
                 "[.eigerintegration][cli]") {
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
    std::string hn = hostname;
    hn.erase(std::remove(begin(hn), end(hn), 'b'), end(hn));
    hn.erase(std::remove(begin(hn), end(hn), 'e'), end(hn));
    auto hostnames = sls::split(hn, '+');
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

TEST_CASE_METHOD(MultiDetectorFixture, "Flow control 10gbe",
                 "[.eigerintegration][cli]") {
    d.setFlowControl10G(1);
    CHECK(d.setFlowControl10G() == 1);
    d.setFlowControl10G(0);
    CHECK(d.setFlowControl10G() == 0);
}

TEST_CASE_METHOD(MultiDetectorFixture, "activate", "[.eigerintegration][cli]") {
    d.activate(0);
    CHECK(d.activate() == 0);
    d.activate(1);
    CHECK(d.activate() == 1);
}


TEST_CASE_METHOD(MultiDetectorFixture, "all trimbits", "[.eigerintegration][cli]") {
    d.setAllTrimbits(32);
    CHECK(d.setAllTrimbits(-1) == 32);
}

