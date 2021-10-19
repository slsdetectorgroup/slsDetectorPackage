// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef TIMER_H
#define TIMER_H
#include <chrono>
#include <iostream>
#include <string>

namespace sls {

class Timer {
    using clock = std::chrono::high_resolution_clock;
    using time_point = std::chrono::time_point<clock>;

  public:
    Timer(std::string name = "0") : t0(clock::now()), name_(name) {}

    double elapsed_ms() {
        return std::chrono::duration<double, std::milli>(clock::now() - t0)
            .count();
    }
    double elapsed_s() {
        return std::chrono::duration<double>(clock::now() - t0).count();
    }
    void print_elapsed() {
        std::cout << "Timer \"" << name_ << "\": Elapsed time " << elapsed_ms()
                  << " ms\n";
    }
    void restart() { t0 = clock::now(); }

  private:
    time_point t0;
    std::string name_;
};

};     // namespace sls
#endif // TIMER_H
