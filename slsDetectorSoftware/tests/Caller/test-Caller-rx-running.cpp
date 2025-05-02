// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2025 Contributors to the SLS Detector Package
#include "Caller.h"
#include "catch.hpp"
#include "sls/Detector.h"
#include "tests/globals.h"

#include <sstream>

namespace sls {

using test::PUT;

/*
detfuncs::F_RECEIVER_SET_NUM_FRAMES
detfuncs::F_RECEIVER_SET_NUM_TRIGGERS
detfuncs::F_RECEIVER_SET_NUM_BURSTS
detfuncs::F_RECEIVER_SET_TIMING_MODE
detfuncs::F_RECEIVER_SET_BURST_MODE
detfuncs::F_RECEIVER_SET_NUM_ANALOG_SAMPLES
detfuncs::F_RECEIVER_SET_NUM_DIGITAL_SAMPLES
detfuncs::F_RECEIVER_SET_NUM_TRANSCEIVER_SAMPLES
detfuncs::F_RECEIVER_SET_DYNAMIC_RANGE
detfuncs::F_RECEIVER_SET_STREAMING_FREQUENCY
detfuncs::F_RECEIVER_SET_FILE_INDEX
detfuncs::F_RECEIVER_SET_FILE_WRITE
detfuncs::F_RECEIVER_SET_MASTER_FILE_WRITE
detfuncs::F_RECEIVER_SET_OVERWRITE
detfuncs::F_RECEIVER_ENABLE_TENGIGA
detfuncs::F_RECEIVER_SET_FIFO_DEPTH
detfuncs::F_RECEIVER_SET_ACTIVATE
detfuncs::F_RECEIVER_SET_STREAMING
detfuncs::F_RECEIVER_SET_STREAMING_TIMER
detfuncs::F_RECEIVER_SET_FLIP_ROWS
detfuncs::F_RECEIVER_SET_DBIT_LIST
detfuncs::F_RECEIVER_SET_DBIT_OFFSET
detfuncs::F_RECEIVER_SET_DBIT_REORDE
*/

auto get_test_parameters() {
    return GENERATE(
        std::make_tuple("asamples", std::vector<std::string>{"5"}),
        std::make_tuple("tsamples", std::vector<std::string>{"2"}),
        std::make_tuple("dsamples", std::vector<std::string>{"100"}),
        std::make_tuple("frames", std::vector<std::string>{"10"}),
        std::make_tuple("triggers", std::vector<std::string>{"5"}),
        std::make_tuple("rx_dbitlist", std::vector<std::string>{"{1,2,10}"}),
        std::make_tuple("rx_dbitreorder", std::vector<std::string>{"0"}),
        std::make_tuple("rx_dbitoffset", std::vector<std::string>{"5"}),
        std::make_tuple("findex", std::vector<std::string>{"2"}),
        std::make_tuple("fwrite", std::vector<std::string>{"0"}),
        std::make_tuple("bursts", std::vector<std::string>{"20"}));
}

TEST_CASE("cant put if receiver is not idle", "[.cmdcall][.rx]") {
    auto [command, function_arguments] = get_test_parameters();

    Detector det;
    Caller caller(&det);

    // start receiver
    std::ostringstream oss;
    caller.call("rx_start", {}, -1, PUT, oss);
    REQUIRE(oss.str() == "rx_start successful\n");

    REQUIRE_THROWS(caller.call(command, function_arguments, -1, PUT, oss, -1));

    std::ostringstream oss_stop;
    caller.call("rx_stop", {}, -1, PUT, oss_stop);
    REQUIRE(oss_stop.str() == "rx_stop successful\n");
}
} // namespace sls