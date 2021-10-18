// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include <chrono>
#include <iostream>
#include <string>
#include <memory>

#include "sls/Result.h"
#include "sls/Detector.h"
#include "sls/container_utils.h"
#include "sls/ToString.h"
#include <algorithm>

std::chrono::nanoseconds to_nano(double val, const std::string &unit) {
    if (unit == "us")
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::duration<double, std::micro>(val));
    else if (unit == "ms")
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::duration<double, std::milli>(val));
    else if (unit == "s")
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::duration<double>(val));
}

int main() {

    // std::cout << "5.8 us is " << to_nano(5.8, "us").count() << " ns\n";
    // std::cout << "7 ms is " << to_nano(7, "ms").count() << " ns\n";
    // std::cout << "3.123 s is " << to_nano(3.123, "s").count() << " ns\n";

    // sls::Result<int> res{1,0,6,4,0,4};

    
    // std::unique_ptr<sls::Detector> 
    auto d = sls::make_unique<sls::Detector>(0);

    std::cout << d->getHostname() << "\n";

    auto s= sls::ToString(d->getHostname());
    std::cout << s << "\n";

    std::string str;
    for (const auto& s : d->getHostname())
        str += s;
    std::cout << str << "\n";

}