// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "catch.hpp"

#include "DetectorImpl.h"
#include "Module.h"
#include "sls/ClientSocket.h"
#include "sls/logger.h"
#include "sls/sls_detector_defs.h"

#include "sls/Timer.h"
#include "sls/sls_detector_funcs.h"
#include <iostream>
#include <vector>
#define VERBOSE

// Header holding all configurations for different detectors
#include "tests/config.h"
#include "tests/globals.h"
// using namespace test;
// using dt = slsDetectorDefs::detectorType;
// extern std::string hostname;
// extern std::string detector_type;
// extern dt type;

namespace sls {

TEST_CASE("Single detector no receiver", "[.integration][.single]") {
    auto t = Module::getTypeFromDetector(test::hostname);
    CHECK(t == test::type);

    Module d(t);
    CHECK(d.getDetectorTypeAsEnum() == t);
    CHECK(d.getDetectorTypeAsString() == test::detector_type);

    d.setHostname(test::hostname);
    CHECK(d.getHostname() == test::hostname);

    CHECK(d.setDetectorType() == test::type);

    d.freeSharedMemory();
}

TEST_CASE("Set control port then create a new object with this control port",
          "[.integration][.single]") {
    /*
    TODO!
    Standard port but should not be hardcoded
    Is this the best way to initialize the detectors
    Using braces to make the object go out of scope
    */
    int old_cport = DEFAULT_TCP_CNTRL_PORTNO;
    int old_sport = DEFAULT_TCP_STOP_PORTNO;
    int new_cport = 1993;
    int new_sport = 2000;
    {
        Module d(test::type);
        d.setHostname(test::hostname);
        CHECK(d.getControlPort() == old_cport);
        d.setControlPort(new_cport);
        CHECK(d.getStopPort() == old_sport);
        d.setStopPort(new_sport);
        d.freeSharedMemory();
    }
    {
        Module d(test::type);
        d.setHostname(test::hostname);
        d.setControlPort(new_cport);
        d.setStopPort(new_sport);
        CHECK(d.getControlPort() == new_cport);
        CHECK(d.getStopPort() == new_sport);

        // Reset standard ports
        d.setControlPort(old_cport);
        d.setStopPort(old_sport);
        d.freeSharedMemory();
    }

    Module d(test::type);
    d.setHostname(test::hostname);
    CHECK(d.getStopPort() == DEFAULT_TCP_STOP_PORTNO);
    d.freeSharedMemory();
}

TEST_CASE("single EIGER detector no receiver basic set and get",
          "[.integration][eiger]") {
    // TODO! this test should take command line arguments for config
    SingleDetectorConfig c;

    // Read type by connecting to the detector
    auto type = Module::getTypeFromDetector(c.hostname);
    CHECK(type == c.type_enum);

    // Create Module of said type and set hostname and detector online
    Module d(type);
    CHECK(d.getDetectorTypeAsEnum() == type);
    CHECK(d.getDetectorTypeAsString() == c.type_string);

    d.setHostname(c.hostname);
    CHECK(d.getHostname() == c.hostname);

    CHECK(d.getUseReceiverFlag() == false);
    CHECK_NOTHROW(d.checkDetectorVersionCompatibility());

    // Setting and reading exposure time
    auto t = 1000000000;
    d.setExptime(t);
    CHECK(d.getExptime() == t);

    // size of an eiger half module with and without gap pixels
    CHECK(d.getTotalNumberOfChannels() == 256 * 256 * 4);
    CHECK(d.getTotalNumberOfChannels(slsDetectorDefs::dimension::X) == 1024);
    CHECK(d.getTotalNumberOfChannels(slsDetectorDefs::dimension::Y) == 256);
    // CHECK(d.getTotalNumberOfChannels(slsDetectorDefs::dimension::Z) == 1);
    CHECK(d.getTotalNumberOfChannelsInclGapPixels(
              slsDetectorDefs::dimension::X) == 1024);
    CHECK(d.getTotalNumberOfChannelsInclGapPixels(
              slsDetectorDefs::dimension::Y) == 256);
    // CHECK(d.getTotalNumberOfChannelsInclGapPixels(slsDetectorDefs::dimension::Z)
    // == 1);

    CHECK(d.getNChans() == 256 * 256);
    CHECK(d.getNChans(slsDetectorDefs::dimension::X) == 256);
    CHECK(d.getNChans(slsDetectorDefs::dimension::Y) == 256);
    // CHECK(d.getNChans(slsDetectorDefs::dimension::Z) == 1);

    CHECK(d.getNChips() == 4);
    CHECK(d.getNChips(slsDetectorDefs::dimension::X) == 4);
    CHECK(d.getNChips(slsDetectorDefs::dimension::Y) == 1);
    // CHECK(d.getNChips(slsDetectorDefs::dimension::Z) == 1);

    d.freeSharedMemory();
}

TEST_CASE("Locking mechanism and last ip", "[.integration][.single]") {
    Module d(test::type);
    d.setHostname(test::hostname);

    // Check that detector server is unlocked then lock
    CHECK(d.lockServer() == 0);
    d.lockServer(1);
    CHECK(d.lockServer() == 1);

    // Can we still access the detector while it's locked
    auto t = 1300000000;
    d.setExptime(t);
    CHECK(d.getExptime() == t);

    // unlock again and free
    d.lockServer(0);
    CHECK(d.lockServer() == 0);

    CHECK(d.getLastClientIP() == test::my_ip);
    d.freeSharedMemory();
}

TEST_CASE("Set settings", "[.integration][.single]") {
    Module d(test::type);
    d.setHostname(test::hostname);
    CHECK(d.setSettings(defs::STANDARD) == defs::STANDARD);
}

TEST_CASE("Timer functions", "[.integration][cli]") {
    // FRAME_NUMBER, /**< number of real time frames: total number of
    // acquisitions is number or frames*number of triggers */ ACQUISITION_TIME,
    // /**< exposure time */ FRAME_PERIOD, /**< period between exposures */
    // DELAY_AFTER_TRIGGER, /**< delay between trigger and start of exposure or
    // readout (in triggered mode) */ GATES_NUMBER, /**< number of gates per
    // frame (in gated mode) */ TRIGGER_NUMBER, /**< number of triggers: total
    // number of acquisitions is number or frames*number of triggers */
    // ACTUAL_TIME, /**< Actual time of the detector's internal timer */
    // MEASUREMENT_TIME,  /**< Time of the measurement from the detector (fifo)
    // */

    // PROGRESS, /**< fraction of measurement elapsed - only get! */
    // MEASUREMENTS_NUMBER,
    // FRAMES_FROM_START,
    // FRAMES_FROM_START_PG,
    // SAMPLES,
    // SUBFRAME_ACQUISITION_TIME, /**< subframe exposure time */
    // STORAGE_CELL_NUMBER, /**<number of storage cells */
    // SUBFRAME_DEADTIME, /**< subframe deadtime */
    // MEASURED_PERIOD,	/**< measured period */
    // MEASURED_SUBPERIOD,	/**< measured subperiod */
    // MAX_TIMERS

    Module d(test::type);
    d.setHostname(test::hostname);

    // Number of frames
    auto frames = 5;
    d.setNumberOfFrames(frames);
    CHECK(d.getNumberOfFrames() == frames);

    auto exptime = 2000000000;
    d.setExptime(exptime);
    CHECK(d.getExptime() == exptime);

    auto period = 2000000000;
    d.setPeriod(period);
    CHECK(d.getPeriod() == period);

    if (test::type != dt::EIGER) {
        auto delay = 10000;
        d.setDelayAfterTrigger(delay);
        CHECK(d.getDelayAfterTrigger() == delay);
    }

    auto triggers = 2;
    d.setNumberOfTriggers(triggers);
    CHECK(d.getNumberOfTriggers() == triggers);

    if (test::type == dt::EIGER) {
        auto subtime = 200;
        d.setSubExptime(subtime);
        CHECK(d.getSubExptime() == subtime);
    }
    // for (int i =0; i!=frames; ++i)
    d.startAndReadAll();

    d.freeSharedMemory();
}

// TEST_CASE("Aquire", "[.integration][eiger]"){
//     SingleDetectorConfig c;
//     auto type = Module::getTypeFromDetector(c.hostname);
//     Module d(type);
//     d.setHostname(c.hostname);

//     auto period = 1000000000;
//     auto exptime = 100000000;
//     d.setNumberOfFrames(5);
//     d.setExptime(exptime);
//     d.setPeriod(period);
//     d.startAndReadAll();

//     auto rperiod =
//     d.getMeasuredPeriod();
//     CHECK(rperiod == 0.1);

//     d.freeSharedMemory();
// }

TEST_CASE(
    "Eiger Dynamic Range with effect on rate correction and clock divider",
    "[.eigerintegration]") {
    SingleDetectorConfig c;

    int ratecorr = 125;

    // pick up multi detector from shm id 0
    DetectorImpl m(0);

    // ensure eiger detector type, hostname and online
    REQUIRE(m.getDetectorTypeAsEnum() == c.type_enum);
    REQUIRE(m.getHostname() == c.hostname);

    // starting state with rate correction off
    m.setRateCorrection(0);

    // dr 16: clk divider, no change for ratecorr
    CHECK(m.setDynamicRange(16) == 16);
    CHECK(m.setSpeed(slsDetectorDefs::CLOCK_DIVIDER) == 1);
    CHECK(m.getRateCorrection() == 0);

    // dr 32: clk divider, no change for ratecorr
    CHECK(m.setDynamicRange(32) == 32);
    CHECK(m.setSpeed(slsDetectorDefs::CLOCK_DIVIDER) == 2);
    CHECK(m.getRateCorrection() == 0);

    // other drs: no change for clk divider, no change for ratecorr
    CHECK(m.setDynamicRange(8) == 8);
    CHECK(m.setSpeed(slsDetectorDefs::CLOCK_DIVIDER) == 2);
    CHECK(m.getRateCorrection() == 0);
    CHECK(m.setDynamicRange(4) == 4);
    CHECK(m.setSpeed(slsDetectorDefs::CLOCK_DIVIDER) == 2);
    CHECK(m.getRateCorrection() == 0);

    // switching on rate correction with dr 16, 32
    m.setDynamicRange(16);
    m.setRateCorrection(ratecorr);
    CHECK(m.getRateCorrection() == ratecorr);
    m.setDynamicRange(32);
    CHECK(m.getRateCorrection() == ratecorr);

    // ratecorr fail with dr 4 or 8
    CHECK_THROWS_AS(m.setDynamicRange(8), RuntimeError);
    CHECK(m.getRateCorrection() == 0);
    m.setDynamicRange(16);
    m.setDynamicRange(16);
    m.setRateCorrection(ratecorr);
    m.setDynamicRange(16);
    m.setRateCorrection(ratecorr);
    CHECK_THROWS_AS(m.setDynamicRange(4), RuntimeError);
    CHECK(m.getRateCorrection() == 0);
}

TEST_CASE("Chiptestboard Loading Patterns", "[.ctbintegration]") {
    SingleDetectorConfig c;

    // pick up multi detector from shm id 0
    DetectorImpl m(0);

    // ensure ctb detector type, hostname and online
    REQUIRE(m.getDetectorTypeAsEnum() == c.type_enum);
    REQUIRE(m.getHostname() == c.hostname);

    uint64_t word = 0;
    int addr = 0;
    int level = 0;
    const int MAX_ADDR = 0x7fff;

    word = 0xc000000000f47ff;
    CHECK(m.setPatternIOControl(word) == word);
    CHECK(m.setPatternIOControl(-1) == word);
    CHECK(m.setPatternIOControl(0) == 0);

    CHECK(m.setPatternClockControl(word) == word);
    CHECK(m.setPatternClockControl(-1) == word);
    CHECK(m.setPatternClockControl(0) == 0);

    // testing pattern word will execute the pattern as well
    addr = 0;
    m.setPatternWord(addr, word);
    CHECK(m.setPatternWord(addr, -1) == word);
    addr = MAX_ADDR - 1;
    m.setPatternWord(addr, word);
    CHECK(m.setPatternWord(addr, -1) == word);
    addr = 0x2FF;
    m.setPatternWord(addr, word);
    CHECK(m.setPatternWord(addr, -1) == word);
    addr = MAX_ADDR;
    CHECK_THROWS_AS(m.setPatternWord(addr, word), RuntimeError);
    CHECK_THROWS_WITH(m.setPatternWord(addr, word),
                      Catch::Matchers::Contains("be between 0 and"));
    addr = -1;
    CHECK_THROWS_AS(m.setPatternWord(addr, word), RuntimeError);
    CHECK_THROWS_WITH(m.setPatternWord(addr, word),
                      Catch::Matchers::Contains("be between 0 and"));

    addr = 0x2FF;
    for (level = 0; level < 3; ++level) {
        CHECK(m.setPatternWaitAddr(level, addr) == addr);
        CHECK(m.setPatternWaitAddr(level, -1) == addr);
    }
    CHECK_THROWS_WITH(m.setPatternWaitAddr(-1, addr),
                      Catch::Matchers::Contains("be between 0 and"));
    CHECK_THROWS_WITH(m.setPatternWaitAddr(0, MAX_ADDR),
                      Catch::Matchers::Contains("be between 0 and"));

    for (level = 0; level < 3; ++level) {
        CHECK(m.setPatternWaitTime(level, word) == word);
        CHECK(m.setPatternWaitTime(level, -1) == word);
    }
    CHECK_THROWS_WITH(m.setPatternWaitTime(-1, word),
                      Catch::Matchers::Contains("be between 0 and"));

    {
        int startaddr = addr;
        int stopaddr = addr + 5;
        int nloops = 2;
        for (level = 0; level < 3; ++level) {
            m.setPatternLoops(level, startaddr, stopaddr, nloops);
            auto r = m.getPatternLoops(level);
            CHECK(r[0] == startaddr);
            CHECK(r[1] == stopaddr);
            CHECK(r[2] == nloops);
        }
        m.setPatternLoops(-1, startaddr, stopaddr, nloops);
        auto r = m.getPatternLoops(-1);
        CHECK(r[0] == startaddr);
        CHECK(r[1] == stopaddr);
        CHECK(r[2] == -1);

        CHECK_THROWS_WITH(m.setPatternLoops(-1, startaddr, MAX_ADDR, nloops),
                          Catch::Matchers::Contains("be less than"));
        CHECK_THROWS_WITH(m.setPatternLoops(-1, MAX_ADDR, stopaddr, nloops),
                          Catch::Matchers::Contains("be less than"));
    }
}

TEST_CASE("Chiptestboard Dbit offset, list, sampling, advinvert",
          "[.ctbintegration][dbit]") {
    SingleDetectorConfig c;

    // pick up multi detector from shm id 0
    DetectorImpl m(0);

    // ensure ctb detector type, hostname and online
    REQUIRE(m.getDetectorTypeAsEnum() == c.type_enum);
    REQUIRE(m.getHostname() == c.hostname);

    // dbit offset
    m.setReceiverDbitOffset(0);
    CHECK(m.getReceiverDbitOffset() == 0);
    m.setReceiverDbitOffset(-1);
    CHECK(m.getReceiverDbitOffset() == 0);
    m.setReceiverDbitOffset(5);
    CHECK(m.getReceiverDbitOffset() == 5);

    // dbit list

    std::vector<int> list = m.getReceiverDbitList();
    list.clear();
    for (int i = 0; i < 10; ++i)
        list.push_back(i);
    m.setReceiverDbitList(list);

    CHECK(m.getReceiverDbitList().size() == 10);

    list.push_back(64);
    CHECK_THROWS_AS(m.setReceiverDbitList(list), RuntimeError);
    CHECK_THROWS_WITH(m.setReceiverDbitList(list),
                      Catch::Matchers::Contains("be between 0 and 63"));

    list.clear();
    for (int i = 0; i < 65; ++i)
        list.push_back(i);
    CHECK(list.size() == 65);
    CHECK_THROWS_WITH(m.setReceiverDbitList(list),
                      Catch::Matchers::Contains("be greater than 64"));

    list.clear();
    m.setReceiverDbitList(list);
    CHECK(m.getReceiverDbitList().empty());

    // adcinvert
    m.setADCInvert(0);
    CHECK(m.getADCInvert() == 0);
    m.setADCInvert(5);
    CHECK(m.getADCInvert() == 5);
    m.setADCInvert(-1);
    CHECK(m.getADCInvert() == -1);

    // ext sampling reg
    m.setExternalSamplingSource(0);
    CHECK(m.getExternalSamplingSource() == 0);
    m.setExternalSamplingSource(62);
    CHECK(m.getExternalSamplingSource() == 62);
    CHECK_THROWS_WITH(m.setExternalSamplingSource(64),
                      Catch::Matchers::Contains("be 0-63"));
    CHECK(m.getExternalSamplingSource() == 62);
    m.setExternalSampling(1);
    CHECK(m.getExternalSampling() == 1);
    m.setExternalSampling(0);
    CHECK(m.getExternalSampling() == 0);
    m.setExternalSampling(1);
    CHECK(m.getExternalSampling() == 1);
    CHECK(m.readRegister(0x7b) == 0x1003E);
}

TEST_CASE("Eiger or Jungfrau nextframenumber",
          "[.eigerintegration][.jungfrauintegration][nextframenumber]") {
    SingleDetectorConfig c;

    // pick up multi detector from shm id 0
    DetectorImpl m(0);

    // ensure ctb detector type, hostname and online
    REQUIRE(
        ((m.getDetectorTypeAsEnum() == slsDetectorDefs::detectorType::EIGER) ||
         (m.getDetectorTypeAsEnum() ==
          slsDetectorDefs::detectorType::JUNGFRAU)));
    REQUIRE(m.getHostname() == c.hostname);

    CHECK(m.setNumberOfFrames(1) == 1);

    // starting fnum
    uint64_t val = 8;

    m.setNextFrameNumber(val);
    CHECK(m.getNextFrameNumber() == val);
    CHECK(m.acquire() == slsDetectorDefs::OK);
    CHECK(m.getReceiverCurrentFrameIndex() == val);

    ++val;
    CHECK(m.acquire() == slsDetectorDefs::OK);
    CHECK(m.getReceiverCurrentFrameIndex() == val);

    CHECK_THROWS_AS(m.setNextFrameNumber(0), RuntimeError);

    if (m.getDetectorTypeAsString() == "Eiger") {
        val = 281474976710655;
    } else if (m.getDetectorTypeAsString() == "Jungfrau") {
        val = 18446744073709551615;
    }
    m.setNextFrameNumber(val);
    CHECK(m.getNextFrameNumber() == val);
    CHECK(m.acquire() == slsDetectorDefs::OK);
    CHECK(m.getReceiverCurrentFrameIndex() == val);
    CHECK(m.getNextFrameNumber() == (val + 1));
}

TEST_CASE("Eiger partialread", "[.eigerintegration][partialread]") {
    SingleDetectorConfig c;

    // pick up multi detector from shm id 0
    DetectorImpl m(0);

    // ensure detector type, hostname
    REQUIRE(
        (m.getDetectorTypeAsEnum() == slsDetectorDefs::detectorType::EIGER));
    REQUIRE(m.getHostname() == c.hostname);

    m.setDynamicRange(16);
    m.enableTenGigabitEthernet(0);
    m.setPartialReadout(256);
    CHECK(m.getPartialReadout() == 256);
    m.setPartialReadout(1);
    CHECK(m.getPartialReadout() == 1);

    m.setDynamicRange(8);
    m.setPartialReadout(256);
    CHECK(m.getPartialReadout() == 256);
    CHECK_THROWS_AS(m.setPartialReadout(1), RuntimeError);
    CHECK(m.getPartialReadout() == 256);
    CHECK_THROWS_AS(m.setPartialReadout(0), RuntimeError);
    m.setPartialReadout(256);
}

} // namespace sls
