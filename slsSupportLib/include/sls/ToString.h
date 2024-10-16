// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

/**
 * \file ToString.h
 *
 * Conversion from various types to std::string
 *
 */

#include "sls/TimeHelper.h"
#include "sls/TypeTraits.h"
#include "sls/sls_detector_defs.h"
#include "sls/sls_detector_exceptions.h"
#include "sls/string_utils.h"
#include <array>
#include <chrono>
#include <iomanip>
#include <map>
#include <sstream>
#include <type_traits>
#include <vector>

namespace sls {

using defs = slsDetectorDefs;
std::string ToString(const defs::runStatus s);
std::string ToString(const defs::detectorType s);
std::string ToString(const defs::detectorSettings s);
std::string ToString(const defs::speedLevel s);
std::string ToString(const defs::timingMode s);
std::string ToString(const defs::frameDiscardPolicy s);
std::string ToString(const defs::fileFormat s);
std::string ToString(const defs::externalSignalFlag s);
std::string ToString(const defs::readoutMode s);
std::string ToString(const defs::dacIndex s);
std::string ToString(const std::vector<defs::dacIndex> &vec);
std::string ToString(const defs::burstMode s);
std::string ToString(const defs::timingSourceType s);
std::string ToString(const defs::M3_GainCaps s);
std::string ToString(const defs::portPosition s);
std::string ToString(const defs::streamingInterface s);
std::string ToString(const defs::vetoAlgorithm s);
std::string ToString(const defs::gainMode s);
std::string ToString(const defs::polarity s);
std::string ToString(const defs::timingInfoDecoder s);
std::string ToString(const defs::collectionMode s);

std::string ToString(const slsDetectorDefs::xy &coord);
std::ostream &operator<<(std::ostream &os, const slsDetectorDefs::xy &coord);
std::string ToString(const slsDetectorDefs::ROI &roi);
std::ostream &operator<<(std::ostream &os, const slsDetectorDefs::ROI &roi);
std::string ToString(const slsDetectorDefs::rxParameters &r);
std::ostream &operator<<(std::ostream &os,
                         const slsDetectorDefs::rxParameters &r);
std::string ToString(const slsDetectorDefs::scanParameters &r);
std::ostream &operator<<(std::ostream &os,
                         const slsDetectorDefs::scanParameters &r);
std::string ToString(const slsDetectorDefs::currentSrcParameters &r);
std::ostream &operator<<(std::ostream &os,
                         const slsDetectorDefs::currentSrcParameters &r);
std::string ToString(const slsDetectorDefs::pedestalParameters &r);
std::ostream &operator<<(std::ostream &os,
                         const slsDetectorDefs::pedestalParameters &r);
const std::string &ToString(const std::string &s);

/** Convert std::chrono::duration with specified output unit */
template <typename T, typename Rep = double>
typename std::enable_if<is_duration<T>::value, std::string>::type
ToString(T t, const std::string &unit) {
    using std::chrono::duration;
    using std::chrono::duration_cast;
    std::ostringstream os;
    if (unit == "ns")
        os << duration_cast<duration<Rep, std::nano>>(t).count() << unit;
    else if (unit == "us")
        os << duration_cast<duration<Rep, std::micro>>(t).count() << unit;
    else if (unit == "ms")
        os << duration_cast<duration<Rep, std::milli>>(t).count() << unit;
    else if (unit == "s")
        os << duration_cast<duration<Rep>>(t).count() << unit;
    else
        throw std::runtime_error("Unknown unit: " + unit);
    return os.str();
}

/** Convert std::chrono::duration automatically selecting the unit */
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

/** Conversion of floating point values, removes trailing zeros*/
template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, std::string>::type
ToString(const T &value) {
    auto s = std::to_string(value);
    s.erase(s.find_last_not_of('0') + 1u, std::string::npos);
    s.erase(s.find_last_not_of('.') + 1u, std::string::npos);
    return s;
}

/** Conversion of integer types, do not remove trailing zeros */
template <typename T>
typename std::enable_if<std::is_integral<T>::value, std::string>::type
ToString(const T &value) {
    return std::to_string(value);
}

/** Conversion of integer types, do not remove trailing zeros */
template <typename T>
typename std::enable_if<std::is_integral<T>::value, std::string>::type
ToStringHex(const T &value) {
    std::ostringstream os;
    os << "0x" << std::hex << value << std::dec;
    return os.str();
}

/** Conversion of integer types, do not remove trailing zeros */
template <typename T>
typename std::enable_if<std::is_integral<T>::value, std::string>::type
ToStringHex(const T &value, int width) {
    std::ostringstream os;
    os << "0x" << std::hex << std::setfill('0') << std::setw(width) << value
       << std::dec;
    return os.str();
}

/**
 * hex
 * For a container loop over all elements and call ToString on the element
 * Container<std::string> is excluded
 */
template <typename T>
typename std::enable_if<
    is_container<T>::value &&
        !std::is_same<typename T::value_type, std::string>::value,
    std::string>::type
ToStringHex(const T &container) {
    std::ostringstream os;
    os << '[';
    if (!container.empty()) {
        auto it = container.cbegin();
        os << ToStringHex(*it++);
        while (it != container.cend())
            os << ", " << ToStringHex(*it++);
    }
    os << ']';
    return os.str();
}

template <typename T>
typename std::enable_if<
    is_container<T>::value &&
        !std::is_same<typename T::value_type, std::string>::value,
    std::string>::type
ToStringHex(const T &container, int width) {
    std::ostringstream os;
    os << '[';
    if (!container.empty()) {
        auto it = container.cbegin();
        os << ToStringHex(*it++, width);
        while (it != container.cend())
            os << ", " << ToStringHex(*it++, width);
    }
    os << ']';
    return os.str();
}

template <typename KeyType, typename ValueType>
std::string ToString(const std::map<KeyType, ValueType> &m) {
    std::ostringstream os;
    os << '{';
    if (!m.empty()) {
        auto it = m.cbegin();
        os << ToString(it->first) << ": " << ToString(it->second);
        it++;
        while (it != m.cend()) {
            os << ", " << ToString(it->first) << ": " << ToString(it->second);
            it++;
        }
    }
    os << '}';
    return os.str();
}

/**
 * Print a c style array
 */
template <typename T, size_t size> std::string ToString(const T (&arr)[size]) {
    std::ostringstream os;
    os << '[';
    if (size) {
        size_t i = 0;
        os << ToString(arr[i++]);
        for (; i < size; ++i)
            os << ", " << ToString(arr[i]);
    }
    os << ']';
    return os.str();
}

/**
 * For a container loop over all elements and call ToString on the element
 * Container<std::string> is excluded
 */
template <typename T>
typename std::enable_if<
    is_container<T>::value &&
        !std::is_same<typename T::value_type, std::string>::value,
    std::string>::type
ToString(const T &container) {
    std::ostringstream os;
    os << '[';
    if (!container.empty()) {
        auto it = container.cbegin();
        os << ToString(*it++);
        while (it != container.cend())
            os << ", " << ToString(*it++);
    }
    os << ']';
    return os.str();
}

/**
 * Special case when container holds a string, don't call ToString again but
 * print directly to stream
 */

template <typename T>
typename std::enable_if<
    is_container<T>::value &&
        std::is_same<typename T::value_type, std::string>::value,
    std::string>::type
ToString(const T &vec) {
    std::ostringstream os;
    os << '[';
    if (!vec.empty()) {
        auto it = vec.begin();
        os << *it++;
        while (it != vec.end())
            os << ", " << *it++;
    }
    os << ']';
    return os.str();
}

/** Container and specified unit, call ToString(value, unit) */
template <typename T>
typename std::enable_if<is_container<T>::value, std::string>::type
ToString(const T &container, const std::string &unit) {
    std::ostringstream os;
    os << '[';
    if (!container.empty()) {
        auto it = container.cbegin();
        os << ToString(*it++, unit);
        while (it != container.cend())
            os << ", " << ToString(*it++, unit);
    }
    os << ']';
    return os.str();
}

template <typename T>
T StringTo(const std::string &t, const std::string &unit) {
    double tval{0};
    try {
        tval = std::stod(t);
    } catch (const std::invalid_argument &e) {
        throw RuntimeError("Could not convert string to time");
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
        throw RuntimeError(
            "Invalid unit in conversion from string to std::chrono::duration");
    }
}

template <typename T> T StringTo(const std::string &t) {
    std::string tmp{t};
    auto unit = RemoveUnit(tmp);
    return StringTo<T>(tmp, unit);
}

template <> defs::detectorType StringTo(const std::string &s);
template <> defs::detectorSettings StringTo(const std::string &s);
template <> defs::speedLevel StringTo(const std::string &s);
template <> defs::timingMode StringTo(const std::string &s);
template <> defs::frameDiscardPolicy StringTo(const std::string &s);
template <> defs::fileFormat StringTo(const std::string &s);
template <> defs::externalSignalFlag StringTo(const std::string &s);
template <> defs::readoutMode StringTo(const std::string &s);
template <> defs::dacIndex StringTo(const std::string &s);
template <> defs::burstMode StringTo(const std::string &s);
template <> defs::timingSourceType StringTo(const std::string &s);
template <> defs::M3_GainCaps StringTo(const std::string &s);
template <> defs::portPosition StringTo(const std::string &s);
template <> defs::streamingInterface StringTo(const std::string &s);
template <> defs::vetoAlgorithm StringTo(const std::string &s);
template <> defs::gainMode StringTo(const std::string &s);
template <> defs::polarity StringTo(const std::string &s);
template <> defs::timingInfoDecoder StringTo(const std::string &s);
template <> defs::collectionMode StringTo(const std::string &s);

template <> uint8_t StringTo(const std::string &s);
template <> uint16_t StringTo(const std::string &s);
template <> uint32_t StringTo(const std::string &s);
template <> uint64_t StringTo(const std::string &s);
template <> int StringTo(const std::string &s);
template <> bool StringTo(const std::string &s);
template <> int64_t StringTo(const std::string &s);

/** For types with a .str() method use this for conversion */
template <typename T>
typename std::enable_if<has_str<T>::value, std::string>::type
ToString(const T &obj) {
    return obj.str();
}

template <typename T>
std::vector<T> StringTo(const std::vector<std::string> &strings) {
    std::vector<T> result;
    result.reserve(strings.size());
    for (const auto &s : strings)
        result.push_back(StringTo<T>(s));
    return result;
}

} // namespace sls
