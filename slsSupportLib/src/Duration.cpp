#include "sls/Duration.h"
#include <cmath>
namespace sls{
    Duration::Duration(){}
    Duration::Duration(double seconds){
        ns_tick = std::round(seconds*1e9);
    }

    Duration::Duration(int64_t ns){
        ns_tick = ns;
    }

    Duration::Duration(std::chrono::nanoseconds ns){
        ns_tick = ns.count();
    }

    uint64_t Duration::count() const{
        return ns_tick;
    }


    Duration::operator std::chrono::nanoseconds(){
        return std::chrono::nanoseconds(ns_tick);
    }

    std::string Duration::str() const{
        return "HEJ";
    }

    bool Duration::operator==(const Duration& other)const{
        return false;
    }

    double Duration::total_seconds()const{
        return static_cast<double>(ns_tick)/1e9;
    }
}