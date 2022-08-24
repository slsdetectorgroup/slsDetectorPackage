#include "DurationWrapper.h"
#include <cmath>

namespace sls{

    DurationWrapper::DurationWrapper(double seconds){
        ns_tick = std::round(seconds*1e9);
    }

    uint64_t DurationWrapper::count() const{
        return ns_tick;
    }

    void DurationWrapper::set_count(uint64_t ns_count){
        ns_tick = ns_count;
    }

    bool DurationWrapper::operator==(const DurationWrapper& other)const{
        return ns_tick == other.ns_tick;
    }

    double DurationWrapper::total_seconds()const{
        return static_cast<double>(ns_tick)/1e9;
    }

}