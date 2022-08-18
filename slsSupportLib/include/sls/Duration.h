// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include <cstdint>
#include <chrono>
#include <string>
namespace sls{

/*
Wrapper for nanoseconds stored in uint64_t

*/


class Duration{
    uint64_t ns_tick{0};

    public:
        Duration();
        explicit Duration(double seconds);
        Duration(int64_t ns);

        //Allows for seemless converion from sls::Duration to 
        //std::chrono::nanoseconds. Avoids a bigger API break.
        //is it a good idea? 
        Duration(std::chrono::nanoseconds ns);
        operator std::chrono::nanoseconds();
        bool operator==(const Duration& other) const;


        uint64_t count() const;
        double total_seconds() const;

        std::string str() const;

        
};

}