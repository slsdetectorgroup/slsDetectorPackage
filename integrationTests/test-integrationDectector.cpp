
#include "catch.hpp"

#include "ClientSocket.h"
#include "logger.h"
#include "multiSlsDetector.h"
#include "slsDetector.h"
#include "sls_detector_defs.h"

#include "Timer.h"
#include "sls_detector_funcs.h"
#include <iostream>
#include <vector>
#define VERBOSE

// Header holding all configurations for different detectors
#include "tests/config.h"
#include "tests/globals.h"

// using dt = slsDetectorDefs::detectorType;
// extern std::string hostname;
// extern std::string detector_type;
// extern dt type;

TEST_CASE("Single detector no receiver", "[.integration][.single]") {
    auto t = slsDetector::getTypeFromDetector(hostname);
    CHECK(t == type);

    slsDetector d(t);
    CHECK(d.getDetectorTypeAsEnum() == t);
    CHECK(d.getDetectorTypeAsString() == detector_type);

    d.setHostname(hostname);
    CHECK(d.getHostname() == hostname);

    d.setOnline(true);
    CHECK(d.getOnlineFlag() == true);

    CHECK(d.setDetectorType() == type);

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
    int old_cport = DEFAULT_PORTNO;
    int old_sport = DEFAULT_PORTNO + 1;
    int new_cport = 1993;
    int new_sport = 2000;
    {
        slsDetector d(type);
        d.setHostname(hostname);
        d.setOnline(true);
        CHECK(d.getControlPort() == old_cport);
        d.setControlPort(new_cport);
        CHECK(d.getStopPort() == old_sport);
        d.setStopPort(new_sport);
        d.freeSharedMemory();
    }
    {
        slsDetector d(type);
        d.setHostname(hostname);
        d.setControlPort(new_cport);
        d.setStopPort(new_sport);
        CHECK(d.getControlPort() == new_cport);
        CHECK(d.getStopPort() == new_sport);

        d.setOnline(true);

        // Reset standard ports
        d.setControlPort(old_cport);
        d.setStopPort(old_sport);
        d.freeSharedMemory();
    }

    slsDetector d(type);
    d.setHostname(hostname);
    d.setOnline(true);
    CHECK(d.getStopPort() == DEFAULT_PORTNO + 1);
    d.freeSharedMemory();
}


TEST_CASE("single EIGER detector no receiver basic set and get",
          "[.integration][eiger]") {
    // TODO! this test should take command line arguments for config
    SingleDetectorConfig c;

    // Read type by connecting to the detector
    auto type = slsDetector::getTypeFromDetector(c.hostname);
    CHECK(type == c.type_enum);

    // Create slsDetector of said type and set hostname and detector online
    slsDetector d(type);
    CHECK(d.getDetectorTypeAsEnum() == type);
    CHECK(d.getDetectorTypeAsString() == c.type_string);

    d.setHostname(c.hostname);
    CHECK(d.getHostname() == c.hostname);

    d.setOnline(true);
    CHECK(d.getOnlineFlag() == true);

    CHECK(d.getReceiverOnlineFlag() == false);
    CHECK(d.checkDetectorVersionCompatibility() == slsDetectorDefs::OK);

    // Setting and reading exposure time
    auto t = 1000000000;
    d.setTimer(slsDetectorDefs::timerIndex::ACQUISITION_TIME, t);
    CHECK(d.setTimer(slsDetectorDefs::timerIndex::ACQUISITION_TIME) == t);

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
    slsDetector d(type);
    d.setHostname(hostname);
    d.setOnline(true);

    // Check that detector server is unlocked then lock
    CHECK(d.lockServer() == 0);
    d.lockServer(1);
    CHECK(d.lockServer() == 1);

    // Can we still access the detector while it's locked
    auto t = 1300000000;
    d.setTimer(slsDetectorDefs::timerIndex::ACQUISITION_TIME, t);
    CHECK(d.setTimer(slsDetectorDefs::timerIndex::ACQUISITION_TIME) == t);

    // unlock again and free
    d.lockServer(0);
    CHECK(d.lockServer() == 0);

    CHECK(d.getLastClientIP() == my_ip);
    d.freeSharedMemory();
}

TEST_CASE("Set settings", "[.integration][.single]"){
    slsDetector d(type);
    d.setHostname(hostname);
    d.setOnline(true);
    CHECK(d.setSettings(defs::STANDARD) == defs::STANDARD);
}


TEST_CASE("Timer functions", "[.integration][cli]") {
    // FRAME_NUMBER, /**< number of real time frames: total number of
    // acquisitions is number or frames*number of cycles */ ACQUISITION_TIME,
    // /**< exposure time */ FRAME_PERIOD, /**< period between exposures */
    // DELAY_AFTER_TRIGGER, /**< delay between trigger and start of exposure or
    // readout (in triggered mode) */ GATES_NUMBER, /**< number of gates per
    // frame (in gated mode) */ CYCLES_NUMBER, /**< number of cycles: total
    // number of acquisitions is number or frames*number of cycles */
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

    slsDetector d(type);
    d.setHostname(hostname);
    d.setOnline(true);

    // Number of frames
    auto frames = 5;
    d.setTimer(slsDetectorDefs::timerIndex::FRAME_NUMBER, frames);
    CHECK(d.setTimer(slsDetectorDefs::timerIndex::FRAME_NUMBER) == frames);

    auto exptime = 2000000000;
    d.setTimer(slsDetectorDefs::timerIndex::ACQUISITION_TIME, exptime);
    CHECK(d.setTimer(slsDetectorDefs::timerIndex::ACQUISITION_TIME) == exptime);

    auto period = 2000000000;
    d.setTimer(slsDetectorDefs::timerIndex::FRAME_PERIOD, period);
    CHECK(d.setTimer(slsDetectorDefs::timerIndex::FRAME_PERIOD) == period);

    if (type != dt::EIGER) {
        auto delay = 10000;
        d.setTimer(slsDetectorDefs::timerIndex::DELAY_AFTER_TRIGGER, delay);
        CHECK(d.setTimer(slsDetectorDefs::timerIndex::DELAY_AFTER_TRIGGER) ==
              delay);
    }

    if (type != dt::EIGER) {
        auto gates = 1;
        d.setTimer(slsDetectorDefs::timerIndex::GATES_NUMBER, gates);
        CHECK(d.setTimer(slsDetectorDefs::timerIndex::GATES_NUMBER) == gates);
    }

    auto cycles = 2;
    d.setTimer(slsDetectorDefs::timerIndex::CYCLES_NUMBER, cycles);
    CHECK(d.setTimer(slsDetectorDefs::timerIndex::CYCLES_NUMBER) == cycles);

    if (type == dt::EIGER) {
        auto subtime = 200;
        d.setTimer(slsDetectorDefs::timerIndex::SUBFRAME_ACQUISITION_TIME,
                   subtime);
        CHECK(d.setTimer(
                  slsDetectorDefs::timerIndex::SUBFRAME_ACQUISITION_TIME) ==
              subtime);
    }
    // for (int i =0; i!=frames; ++i)
        d.startAndReadAll();

    d.freeSharedMemory();

    // If we add a timer we should add tests for the timer
    CHECK(slsDetectorDefs::MAX_TIMERS == 19);
}

// TEST_CASE("Aquire", "[.integration][eiger]"){
//     SingleDetectorConfig c;
//     auto type = slsDetector::getTypeFromDetector(c.hostname);
//     slsDetector d(type);
//     d.setHostname(c.hostname);
//     d.setOnline(true);

//     auto period = 1000000000;
//     auto exptime = 100000000;
//     d.setTimer(slsDetectorDefs::timerIndex::FRAME_NUMBER, 5);
//     d.setTimer(slsDetectorDefs::timerIndex::ACQUISITION_TIME, exptime);
//     d.setTimer(slsDetectorDefs::timerIndex::FRAME_PERIOD, period);
//     d.startAndReadAll();

//     auto rperiod =
//     d.getTimeLeft(slsDetectorDefs::timerIndex::MEASURED_PERIOD);
//     CHECK(rperiod == 0.1);

//     d.freeSharedMemory();
// }

TEST_CASE(
    "Eiger Dynamic Range with effect on rate correction and clock divider",
    "[.eigerintegration]") {
    SingleDetectorConfig c;

    int ratecorr = 125;

    // pick up multi detector from shm id 0
    multiSlsDetector m(0);

    // ensure eiger detector type, hostname and online
    REQUIRE(m.getDetectorTypeAsEnum() == c.type_enum);
    REQUIRE(m.getHostname() == c.hostname);
    REQUIRE(m.setOnline(true) == slsDetectorDefs::ONLINE_FLAG);

    // starting state with rate correction off
    CHECK(m.setRateCorrection(0) == 0);

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
    CHECK_THROWS_AS(m.setDynamicRange(8), sls::NonCriticalError);
    CHECK(m.getRateCorrection() == 0);
    m.setDynamicRange(16);
    m.setDynamicRange(16);
    m.setRateCorrection(ratecorr);
    m.setDynamicRange(16);
    m.setRateCorrection(ratecorr);
    CHECK_THROWS_AS(m.setDynamicRange(4), sls::NonCriticalError);
    CHECK(m.getRateCorrection() == 0);
}

TEST_CASE("Chiptestboard Loading Patterns", "[.ctbintegration]") {
    SingleDetectorConfig c;

    // pick up multi detector from shm id 0
    multiSlsDetector m(0);

    // ensure ctb detector type, hostname and online
    REQUIRE(m.getDetectorTypeAsEnum() == c.type_enum);
    REQUIRE(m.getHostname() == c.hostname);
    REQUIRE(m.setOnline(true) == slsDetectorDefs::ONLINE_FLAG);

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
    CHECK_THROWS_AS(m.setPatternWord(addr, word), sls::NonCriticalError);
    CHECK_THROWS_WITH(m.setPatternWord(addr, word),
                      Catch::Matchers::Contains("be between 0 and"));
    addr = -1;
    CHECK_THROWS_AS(m.setPatternWord(addr, word), sls::NonCriticalError);
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


TEST_CASE("Chiptestboard Dbit offset, list, sampling, advinvert", "[.ctbintegration][dbit]") {
    SingleDetectorConfig c;

    // pick up multi detector from shm id 0
    multiSlsDetector m(0);

    // ensure ctb detector type, hostname and online
    REQUIRE(m.getDetectorTypeAsEnum() == c.type_enum);
    REQUIRE(m.getHostname() == c.hostname);
    REQUIRE(m.setOnline(true) == slsDetectorDefs::ONLINE_FLAG);

    // dbit offset
    m.setReceiverDbitOffset(0);
    CHECK(m.getReceiverDbitOffset() == 0);
    m.setReceiverDbitOffset(-1);
    CHECK(m.getReceiverDbitOffset() == 0);
    m.setReceiverDbitOffset(5);
    CHECK(m.getReceiverDbitOffset() == 5);

    // dbit list

    std::vector <int> list = m.getReceiverDbitList();
    list.clear();
    for (int i = 0; i < 10; ++i)
        list.push_back(i);
    m.setReceiverDbitList(list);
    
    CHECK(m.getReceiverDbitList().size() == 10);

    list.push_back(64);
    CHECK_THROWS_AS(m.setReceiverDbitList(list), sls::RuntimeError);
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