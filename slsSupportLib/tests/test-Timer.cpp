// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "catch.hpp"
#include "sls/Timer.h"

#include <chrono>
#include <thread>

namespace sls {

TEST_CASE("Time 1s restart then time 2s", "[.timer]") {
    auto sleep_duration = std::chrono::seconds(1);
    auto t = Timer();
    std::this_thread::sleep_for(sleep_duration);
    REQUIRE(t.elapsed_s() == Approx(1).epsilon(0.01));

    t.restart();
    std::this_thread::sleep_for(sleep_duration * 2);
    REQUIRE(t.elapsed_s() == Approx(2).epsilon(0.01));
}

TEST_CASE("Return ms", "[.timer]") {
    auto sleep_duration = std::chrono::milliseconds(1300);
    auto t = Timer();
    std::this_thread::sleep_for(sleep_duration);
    REQUIRE(t.elapsed_ms() == Approx(1300).epsilon(0.5));
}

} // namespace sls
