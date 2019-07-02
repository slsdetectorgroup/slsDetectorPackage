#pragma once

#include "TimeAlias.h"
#include "TypeTraits.h"
#include "sls_detector_exceptions.h"
#include "string_utils.h"
#include <chrono>
#include <sstream>
#include <type_traits>
#include <vector>

namespace sls {

std::string ToString(const std::vector<std::string> &vec,
                     const char delimiter = ' ');

template <typename T, typename Rep = double>
typename std::enable_if<is_duration<T>::value, std::string>::type
ToString(T t, const std::string &unit) {
    using std::chrono::duration;
    using std::chrono::duration_cast;
    std::ostringstream os;
    if (unit == "ns")
        os << duration_cast<duration<Rep, std::nano>>(t).count() << " " << unit;
    else if (unit == "us")
        os << duration_cast<duration<Rep, std::micro>>(t).count() << " "
           << unit;
    else if (unit == "ms")
        os << duration_cast<duration<Rep, std::milli>>(t).count() << " "
           << unit;
    else if (unit == "s")
        os << duration_cast<duration<Rep>>(t).count() << " " << unit;
    else
        throw std::runtime_error("Unknown unit: " + unit);
    return os.str();
}

template <typename From>
typename std::enable_if<is_duration<From>::value, std::string>::type
ToString(From t) {
    auto tns = std::chrono::duration_cast<std::chrono::nanoseconds>(t);
    if (time::abs(tns) < std::chrono::microseconds(1)) {
        return ToString(tns, "ns");
    } else if (time::abs(tns) < std::chrono::milliseconds(1)) {
        return ToString(tns, "us");
    } else if (time::abs(tns) < std::chrono::milliseconds(99)) {
        return ToString(tns, "ms");
    } else {
        return ToString(tns, "s");
    }
}

template <typename T>
T StringTo(const std::string &t, const std::string &unit) {
    double tval{0};
    try {
        tval = std::stod(t);
    } catch (const std::invalid_argument &e) {
        throw sls::RuntimeError("Could not convert string to time");
    }

    using std::chrono::duration;
    using std::chrono::duration_cast;
    if (unit == "ns") {
        return duration_cast<T>(duration<double, std::nano>(tval));
    } else if (unit == "us") {
        return duration_cast<T>(duration<double, std::micro>(tval));
    } else if (unit == "ms") {
        return duration_cast<T>(duration<double, std::milli>(tval));
    } else if (unit == "s" || unit.empty()) {
        return duration_cast<T>(std::chrono::duration<double>(tval));
    } else {
        throw sls::RuntimeError(
            "Invalid unit in conversion from string to std::chrono::duration");
    }
}

template <typename T> T StringTo(std::string t) {
    auto unit = RemoveUnit(t);
    return StringTo<T>(t, unit);
}

} // namespace sls
