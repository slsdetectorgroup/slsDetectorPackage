#pragma once

/**
 * \file ToString.h
 *
 * Conversion from various types to std::string
 *
 */

#include "TimeHelper.h"
#include "TypeTraits.h"
#include "sls_detector_exceptions.h"
#include "string_utils.h"
#include "sls_detector_defs.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <type_traits>
#include <vector>


namespace sls {

using defs = slsDetectorDefs;

inline std::string ToString(const defs::runStatus s){
    switch (s) {
    case defs::ERROR:
        return std::string("error");
    case defs::WAITING:
        return std::string("waiting");
    case defs::RUNNING:
        return std::string("running");
    case defs::TRANSMITTING:
        return std::string("data");
    case defs::RUN_FINISHED:
        return std::string("finished");
    case defs::STOPPED:
        return std::string("stopped");
    default:
        return std::string("idle");
    }
}

inline std::string ToString(const defs::detectorType s){
    switch (s) {
    case defs::EIGER:
        return std::string("Eiger");
    case defs::GOTTHARD:
        return std::string("Gotthard");
    case defs::JUNGFRAU:
        return std::string("Jungfrau");
    case defs::CHIPTESTBOARD:
        return std::string("JungfrauCTB");
    case defs::MOENCH:
        return std::string("Moench");
    case defs::MYTHEN3:
        return std::string("Mythen3");
    case defs::GOTTHARD2:
        return std::string("Gotthard2");  
    default:
        return std::string("Unknown"); //TODO: check if generic or get detector type are used anywhere?          
    }
}



// in case we already have a string 
// causes a copy but might be needed in generic code
inline std::string ToString(const std::string& s){
    return s;
}

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

template <typename T> T StringTo(const std::string& t) {
    std::string tmp{t};
    auto unit = RemoveUnit(tmp);
    return StringTo<T>(tmp, unit);
}

//most likely hardly use
template <>
inline defs::detectorType StringTo(const std::string& s){
    if (s == "Eiger")
        return defs::EIGER;
    if (s == "Gotthard")
        return defs::GOTTHARD;
    if (s == "Jungfrau")
        return defs::JUNGFRAU;
    if (s == "JungfrauCTB")
        return defs::CHIPTESTBOARD;
    if (s == "Moench")
        return defs::MOENCH;
    if (s == "Mythen3")
        return defs::MYTHEN3;
    if (s == "Gotthard2")
        return defs::GOTTHARD2;            
    return defs::GENERIC;
}

/** For types with a .str() method use this for conversion */
template <typename T>
typename std::enable_if<has_str<T>::value, std::string>::type
ToString(const T &obj) {
    return obj.str();
}



} // namespace sls
