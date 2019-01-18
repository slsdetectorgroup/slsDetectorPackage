#include "Timer.h"
#include "catch.hpp"

#include <chrono>
#include <thread>

TEST_CASE("Time 1s restart then time 2s") {
    auto sleep_duration = std::chrono::seconds(1);
    auto t = sls::Timer();
    std::this_thread::sleep_for(sleep_duration);
    REQUIRE(t.elapsed_s() == Approx(1).epsilon(0.01));

    t.restart();
    std::this_thread::sleep_for(sleep_duration * 2);
    REQUIRE(t.elapsed_s() == Approx(2).epsilon(0.01));
}

TEST_CASE("Return ms") {
    auto sleep_duration = std::chrono::milliseconds(1300);
    auto t = sls::Timer();
    std::this_thread::sleep_for(sleep_duration);
    REQUIRE(t.elapsed_ms() == Approx(1300).epsilon(0.5));
}