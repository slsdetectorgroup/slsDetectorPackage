// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include <cstdint>

namespace sls{
/*
Wrapper for nanoseconds stored in uint64_t, used for conversion between
std::chrono::nanoseconds and python (float or sls::DurationWrapper)
*/

class DurationWrapper{
    uint64_t ns_tick{0};

    public:
        DurationWrapper() = default;
        explicit DurationWrapper(double seconds);
        ~DurationWrapper() = default;

        bool operator==(const DurationWrapper& other) const;
        uint64_t count() const;
        void set_count(uint64_t count);
        double total_seconds() const; 
};

}