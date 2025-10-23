// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2025 Contributors to the SLS Detector Package
#include "Caller.h"
#include "catch.hpp"
#include "sls/Detector.h"
#include "tests/globals.h"

#include <sstream>

namespace sls {

using test::PUT;

TEST_CASE("Ctb and xilinx - cant put if receiver is not idle",
          "[.cmdcall][.rx]") {

    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD ||
        det_type == defs::XILINX_CHIPTESTBOARD) {
        auto prev_romode = det.getReadoutMode();
        auto prev_asamples = det.getNumberOfAnalogSamples();
        auto prev_dsamples = det.getNumberOfDigitalSamples();
        auto prev_tsamples = det.getNumberOfTransceiverSamples();
        auto prev_adcenable10g = det.getTenGigaADCEnableMask();
        auto prev_trasnsceiverenable = det.getTransceiverEnableMask();
        auto prev_rxdbitlist = det.getRxDbitList();
        auto prev_rxdbitoffset = det.getRxDbitOffset();
        auto prev_rxdbitreorder = det.getRxDbitReorder();

        // start receiver
        REQUIRE_NOTHROW(caller.call("rx_start", {}, -1, PUT));

        REQUIRE_THROWS(caller.call("romode", {"digital"}, -1, PUT));
        REQUIRE_THROWS(caller.call("asamples", {"5"}, -1, PUT));
        REQUIRE_THROWS(caller.call("dsamples", {"100"}, -1, PUT));
        REQUIRE_THROWS(caller.call("tsamples", {"2"}, -1, PUT));
        REQUIRE_THROWS(caller.call("adcenable10g", {"0xFF00FFFF"}, -1, PUT));
        REQUIRE_THROWS(caller.call("transceiverenable", {"0x3"}, -1, PUT));
        REQUIRE_THROWS(caller.call("rx_dbitlist", {"{1,2,10}"}, -1, PUT));
        REQUIRE_THROWS(caller.call("rx_dbitoffset", {"5"}, -1, PUT));
        REQUIRE_THROWS(caller.call("rx_dbitreorder", {"0"}, -1, PUT));

        // stop receiver
        REQUIRE_NOTHROW(caller.call("rx_stop", {}, -1, PUT));

        for (int i = 0; i != det.size(); ++i) {
            det.setReadoutMode(prev_romode[i], {i});
            det.setNumberOfAnalogSamples(prev_asamples[i], {i});
            det.setNumberOfDigitalSamples(prev_dsamples[i], {i});
            det.setNumberOfTransceiverSamples(prev_tsamples[i], {i});
            det.setTenGigaADCEnableMask(prev_adcenable10g[i], {i});
            det.setTransceiverEnableMask(prev_trasnsceiverenable[i], {i});
            det.setRxDbitList(prev_rxdbitlist[i], {i});
            det.setRxDbitOffset(prev_rxdbitoffset[i], {i});
            det.setRxDbitReorder(prev_rxdbitreorder[i], {i});
        }
    }
}

TEST_CASE("adcenable - cant put if receiver is not idle", "[.cmdcall][.rx]") {

    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::CHIPTESTBOARD) {
        auto prev_adcenable = det.getADCEnableMask();

        // start receiver
        REQUIRE_NOTHROW(caller.call("rx_start", {}, -1, PUT));

        REQUIRE_THROWS(caller.call("adcenable", {"0xFFFFFF00"}, -1, PUT));

        // stop receiver
        REQUIRE_NOTHROW(caller.call("rx_stop", {}, -1, PUT));

        for (int i = 0; i != det.size(); ++i) {
            det.setADCEnableMask(prev_adcenable[i], {i});
        }
    }
}

TEST_CASE("bursts - cant put if receiver is not idle", "[.cmdcall][.rx]") {

    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::GOTTHARD2) {
        auto prev_burst =
            det.getNumberOfBursts().tsquash("#bursts should be same to test");

        // start receiver
        REQUIRE_NOTHROW(caller.call("rx_start", {}, -1, PUT));

        REQUIRE_THROWS(caller.call("bursts", {"20"}, -1, PUT));

        // stop receiver
        REQUIRE_NOTHROW(caller.call("rx_stop", {}, -1, PUT));

        det.setNumberOfBursts(prev_burst);
    }
}

TEST_CASE("counters - cant put if receiver is not idle", "[.cmdcall][.rx]") {

    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::MYTHEN3) {
        auto prev_counters = det.getCounterMask();

        // start receiver
        REQUIRE_NOTHROW(caller.call("rx_start", {}, -1, PUT));

        REQUIRE_THROWS(caller.call("counters", {"0 1 2"}, -1, PUT));

        // stop receiver
        REQUIRE_NOTHROW(caller.call("rx_stop", {}, -1, PUT));

        for (int i = 0; i != det.size(); ++i) {
            det.setCounterMask(prev_counters[i], {i});
        }
    }
}

TEST_CASE("numinterfaces - cant put if receiver is not idle",
          "[.cmdcall][.rx]") {

    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::JUNGFRAU || det_type == defs::MOENCH) {
        auto prev_numinterfaces = det.getNumberofUDPInterfaces();

        // start receiver
        REQUIRE_NOTHROW(caller.call("rx_start", {}, -1, PUT));

        REQUIRE_THROWS(caller.call("numinterafaces", {"2"}, -1, PUT));

        // stop receiver
        REQUIRE_NOTHROW(caller.call("rx_stop", {}, -1, PUT));

        for (int i = 0; i != det.size(); ++i) {
            det.setNumberofUDPInterfaces(prev_numinterfaces[i], {i});
        }
    }
}

TEST_CASE("dr - cant put if receiver is not idle", "[.cmdcall][.rx]") {

    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::EIGER || det_type == defs::MYTHEN3) {
        auto prev_dr =
            det.getDynamicRange().tsquash("dr should be same to test");

        // start receiver
        REQUIRE_NOTHROW(caller.call("rx_start", {}, -1, PUT));

        REQUIRE_THROWS(caller.call("dr", {"16"}, -1, PUT));

        // stop receiver
        REQUIRE_NOTHROW(caller.call("rx_stop", {}, -1, PUT));

        det.setDynamicRange(prev_dr);
    }
}

TEST_CASE("tengiga - cant put if receiver is not idle", "[.cmdcall][.rx]") {

    Detector det;
    Caller caller(&det);
    auto det_type = det.getDetectorType().squash();

    if (det_type == defs::EIGER || det_type == defs::MYTHEN3 ||
        det_type == defs::CHIPTESTBOARD) {
        auto prev_tengiga = det.getTenGiga();

        // start receiver
        REQUIRE_NOTHROW(caller.call("rx_start", {}, -1, PUT));

        REQUIRE_THROWS(caller.call("tengiga", {"1"}, -1, PUT));

        // stop receiver
        REQUIRE_NOTHROW(caller.call("rx_stop", {}, -1, PUT));

        for (int i = 0; i != det.size(); ++i) {
            det.setTenGiga(prev_tengiga[i], {i});
        }
    }
}

TEST_CASE("general - cant put if receiver is not idle", "[.cmdcall][.rx]") {

    Detector det;
    Caller caller(&det);

    {
        auto prev_frames =
            det.getNumberOfFrames().tsquash("#frames should be same to test");
        auto prev_triggers =
            det.getNumberOfTriggers().tsquash("#triggers must be same to test");
        auto prev_findex = det.getAcquisitionIndex();
        auto prev_fwrite = det.getFileWrite();
        auto prev_fifodepth = det.getRxFifoDepth();
        auto rx_hostname = det.getRxHostname();

        // start receiver
        REQUIRE_NOTHROW(caller.call("rx_start", {}, -1, PUT));

        REQUIRE_THROWS(caller.call("frames", {"10"}, -1, PUT));
        REQUIRE_THROWS(caller.call("triggers", {"5"}, -1, PUT));
        REQUIRE_THROWS(caller.call("findex", {"2"}, -1, PUT));
        REQUIRE_THROWS(caller.call("fwrite", {"0"}, -1, PUT));
        REQUIRE_THROWS(caller.call("rx_fifodepth", {"1000"}, -1, PUT));
        REQUIRE_THROWS(caller.call("rx_hostname", {rx_hostname[0]}, 0, PUT));

        // stop receiver
        REQUIRE_NOTHROW(caller.call("rx_stop", {}, -1, PUT));

        det.setNumberOfFrames(prev_frames);
        det.setNumberOfTriggers(prev_triggers);
        for (int i = 0; i != det.size(); ++i) {
            det.setAcquisitionIndex(prev_findex[i], {i});
            det.setFileWrite(prev_fwrite[i], {i});
            det.setRxFifoDepth(prev_fifodepth[i], {i});
            det.setRxHostname(rx_hostname[i], {i});
        }
    }
}
} // namespace sls