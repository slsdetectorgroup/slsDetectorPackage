#pragma once

/**
 * \file ToString.h
 *
 * Conversion from various types to std::string
 *
 */

#include "TimeHelper.h"
#include "TypeTraits.h"
#include "sls_detector_defs.h"
#include "sls_detector_exceptions.h"
#include "string_utils.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <type_traits>
#include <vector>
#include <map>

namespace sls {

using defs = slsDetectorDefs;

inline std::string ToString(const defs::runStatus s) {
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

inline std::string ToString(const defs::detectorType s) {
    switch (s) {
    case defs::EIGER:
        return std::string("Eiger");
    case defs::GOTTHARD:
        return std::string("Gotthard");
    case defs::JUNGFRAU:
        return std::string("Jungfrau");
    case defs::CHIPTESTBOARD:
        return std::string("ChipTestBoard");
    case defs::MOENCH:
        return std::string("Moench");
    case defs::MYTHEN3:
        return std::string("Mythen3");
    case defs::GOTTHARD2:
        return std::string("Gotthard2");
    default:
        return std::string("Unknown");
    }
}

inline std::string ToString(const defs::detectorSettings s) {
    switch (s) {
    case defs::STANDARD:
        return std::string("standard");
    case defs::FAST:
        return std::string("fast");
    case defs::HIGHGAIN:
        return std::string("highgain");
    case defs::DYNAMICGAIN:
        return std::string("dynamicgain");
    case defs::LOWGAIN:
        return std::string("lowgain");
    case defs::MEDIUMGAIN:
        return std::string("mediumgain");
    case defs::VERYHIGHGAIN:
        return std::string("veryhighgain");
    case defs::DYNAMICHG0:
        return std::string("dynamichg0");
    case defs::FIXGAIN1:
        return std::string("fixgain1");
    case defs::FIXGAIN2:
        return std::string("fixgain2");
    case defs::FORCESWITCHG1:
        return std::string("forceswitchg1");
    case defs::FORCESWITCHG2:
        return std::string("forceswitchg2");
    case defs::VERYLOWGAIN:
        return std::string("verylowgain");
    case defs::G1_HIGHGAIN:
        return std::string("g1_hg");
    case defs::G1_LOWGAIN:
        return std::string("g1_lg");
    case defs::G2_HIGHCAP_HIGHGAIN:
        return std::string("g2_hc_hg");
    case defs::G2_HIGHCAP_LOWGAIN:
        return std::string("g2_hc_lg");
    case defs::G2_LOWCAP_HIGHGAIN:
        return std::string("g2_lc_hg");
    case defs::G2_LOWCAP_LOWGAIN:
        return std::string("g2_lc_lg");
    case defs::G4_HIGHGAIN:
        return std::string("g4_hg");
    case defs::G4_LOWGAIN:
        return std::string("g4_lg");
    case defs::UNDEFINED:
        return std::string("undefined");
    case defs::UNINITIALIZED:
        return std::string("uninitialized");
    default:
        return std::string("Unknown");
    }
}

inline std::string ToString(const defs::speedLevel s) {
    switch (s) {
    case defs::FULL_SPEED:
        return std::string("full_speed");
    case defs::HALF_SPEED:
        return std::string("half_speed");
    case defs::QUARTER_SPEED:
        return std::string("quarter_speed");
    default:
        return std::string("Unknown");
    }
}

inline std::string ToString(const defs::timingMode s) {
    switch (s) {
    case defs::AUTO_TIMING:
        return std::string("auto");
    case defs::TRIGGER_EXPOSURE:
        return std::string("trigger");
    case defs::GATED:
        return std::string("gating");
    case defs::BURST_TRIGGER:
        return std::string("burst_trigger");
    default:
        return std::string("Unknown");
    }
}

inline std::string ToString(const defs::frameDiscardPolicy s) {
    switch (s) {
    case defs::NO_DISCARD:
        return std::string("nodiscard");
    case defs::DISCARD_EMPTY_FRAMES:
        return std::string("discardempty");
    case defs::DISCARD_PARTIAL_FRAMES:
        return std::string("discardpartial");
    default:
        return std::string("Unknown");
    }
}

inline std::string ToString(const defs::fileFormat s) {
    switch (s) {
    case defs::HDF5:
        return std::string("hdf5");
    case defs::BINARY:
        return std::string("binary");
    default:
        return std::string("Unknown");
    }
}

inline std::string ToString(const defs::externalSignalFlag s) {
    switch (s) {
    case defs::TRIGGER_IN_RISING_EDGE:
        return std::string("trigger_in_rising_edge");
    case defs::TRIGGER_IN_FALLING_EDGE:
        return std::string("trigger_in_falling_edge");
    default:
        return std::string("Unknown");
    }
}

inline std::string ToString(const defs::readoutMode s) {
    switch (s) {
    case defs::ANALOG_ONLY:
        return std::string("analog");
    case defs::DIGITAL_ONLY:
        return std::string("digital");
    case defs::ANALOG_AND_DIGITAL:
        return std::string("analog_digital");
    default:
        return std::string("Unknown");
    }
}

inline std::string ToString(const defs::frameModeType s) {
    switch (s) {
    case defs::PEDESTAL:
        return std::string("pedestal");
    case defs::NEW_PEDESTAL:
        return std::string("newpedestal");
    case defs::FLATFIELD:
        return std::string("flatfield");
    case defs::NEW_FLATFIELD:
        return std::string("newflatfield");
    default:
        return std::string("Unknown");
    }
}

inline std::string ToString(const defs::detectorModeType s) {
    switch (s) {
    case defs::COUNTING:
        return std::string("counting");
    case defs::INTERPOLATING:
        return std::string("interpolating");
    case defs::ANALOG:
        return std::string("analog");
    default:
        return std::string("Unknown");
    }
}

inline std::string ToString(const defs::burstMode s) {
    switch (s) {
    case defs::BURST_OFF:
        return std::string("off");
    case defs::BURST_INTERNAL:
        return std::string("internal");
    case defs::BURST_EXTERNAL:
        return std::string("external");
    default:
        return std::string("Unknown");
    }
}

inline std::string ToString(const defs::timingSourceType s) {
    switch (s) {
    case defs::TIMING_INTERNAL:
        return std::string("internal");
    case defs::TIMING_EXTERNAL:
        return std::string("external");
    default:
        return std::string("Unknown");
    }
}

// in case we already have a string
// causes a copy but might be needed in generic code
inline std::string ToString(const std::string &s) { return s; }

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

template <typename KeyType, typename ValueType>
std::string ToString(std::map<KeyType, ValueType> m) {
    std::ostringstream os;
    os << '{';
    if (!m.empty()) {
        auto it = m.cbegin();
        os << ToString(it->first) << ": " << ToString(it->second);
        it++;
        while (it != m.cend()) {
            os << ", "<< ToString(it->first) << ": " << ToString(it->second);
            it++;
        }
    }
    os << '}';
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

template <typename T> T StringTo(const std::string &t) {
    std::string tmp{t};
    auto unit = RemoveUnit(tmp);
    return StringTo<T>(tmp, unit);
}

template <> inline defs::detectorType StringTo(const std::string &s) {
    if (s == "Eiger")
        return defs::EIGER;
    if (s == "Gotthard")
        return defs::GOTTHARD;
    if (s == "Jungfrau")
        return defs::JUNGFRAU;
    if (s == "ChipTestBoard")
        return defs::CHIPTESTBOARD;
    if (s == "Moench")
        return defs::MOENCH;
    if (s == "Mythen3")
        return defs::MYTHEN3;
    if (s == "Gotthard2")
        return defs::GOTTHARD2;
    throw sls::RuntimeError("Unknown detector type " + s);
}

template <> inline defs::detectorSettings StringTo(const std::string &s) {
    if (s == "standard")
        return defs::STANDARD;
    if (s == "fast")
        return defs::FAST;
    if (s == "highgain")
        return defs::HIGHGAIN;
    if (s == "dynamicgain")
        return defs::DYNAMICGAIN;
    if (s == "lowgain")
        return defs::LOWGAIN;
    if (s == "mediumgain")
        return defs::MEDIUMGAIN;
    if (s == "veryhighgain")
        return defs::VERYHIGHGAIN;
    if (s == "dynamichg0")
        return defs::DYNAMICHG0;
    if (s == "fixgain1")
        return defs::FIXGAIN1;
    if (s == "fixgain2")
        return defs::FIXGAIN2;
    if (s == "forceswitchg1")
        return defs::FORCESWITCHG1;
    if (s == "forceswitchg2")
        return defs::FORCESWITCHG2;
    if (s == "verylowgain")
        return defs::VERYLOWGAIN;
    if (s == "g1_hg")
        return defs::G1_HIGHGAIN;
    if (s == "g1_lg")
        return defs::G1_LOWGAIN;
    if (s == "g2_hc_hg")
        return defs::G2_HIGHCAP_HIGHGAIN;
    if (s == "g2_hc_lg")
        return defs::G2_HIGHCAP_LOWGAIN;
    if (s == "g2_lc_hg")
        return defs::G2_LOWCAP_HIGHGAIN;
    if (s == "g2_lc_lg")
        return defs::G2_LOWCAP_LOWGAIN;
    if (s == "g4_hg")
        return defs::G4_HIGHGAIN;
    if (s == "g4_lg")
        return defs::G4_LOWGAIN;
    throw sls::RuntimeError("Unknown setting " + s);
}

template <> inline defs::speedLevel StringTo(const std::string &s) {
    if (s == "full_speed")
        return defs::FULL_SPEED;
    if (s == "half_speed")
        return defs::HALF_SPEED;
    if (s == "quarter_speed")
        return defs::QUARTER_SPEED;
    throw sls::RuntimeError("Unknown speed " + s);
}

template <> inline defs::timingMode StringTo(const std::string &s) {
    if (s == "auto")
        return defs::AUTO_TIMING;
    if (s == "trigger")
        return defs::TRIGGER_EXPOSURE;
    if (s == "gating")
        return defs::GATED;
    if (s == "burst_trigger")
        return defs::BURST_TRIGGER;
    throw sls::RuntimeError("Unknown timing mode " + s);
}

template <> inline defs::frameDiscardPolicy StringTo(const std::string &s) {
    if (s == "nodiscard")
        return defs::NO_DISCARD;
    if (s == "discardempty")
        return defs::DISCARD_EMPTY_FRAMES;
    if (s == "discardpartial")
        return defs::DISCARD_PARTIAL_FRAMES;
    throw sls::RuntimeError("Unknown frame discard policy " + s);
}

template <> inline defs::fileFormat StringTo(const std::string &s) {
    if (s == "hdf5")
        return defs::HDF5;
    if (s == "binary")
        return defs::BINARY;
    throw sls::RuntimeError("Unknown file format " + s);
}

template <> inline defs::externalSignalFlag StringTo(const std::string &s) {
    if (s == "trigger_in_rising_edge")
        return defs::TRIGGER_IN_RISING_EDGE;
    if (s == "trigger_in_falling_edge")
        return defs::TRIGGER_IN_FALLING_EDGE;
    throw sls::RuntimeError("Unknown external signal flag " + s);
}

template <> inline defs::readoutMode StringTo(const std::string &s) {
    if (s == "analog")
        return defs::ANALOG_ONLY;
    if (s == "digital")
        return defs::DIGITAL_ONLY;
    if (s == "analog_digital")
        return defs::ANALOG_AND_DIGITAL;
    throw sls::RuntimeError("Unknown readout mode " + s);
}

template <> inline defs::frameModeType StringTo(const std::string &s) {
    if (s == "pedestal")
        return defs::PEDESTAL;
    if (s == "newpedestal")
        return defs::NEW_PEDESTAL;
    if (s == "flatfield")
        return defs::FLATFIELD;
    if (s == "newflatfield")
        return defs::NEW_FLATFIELD;
    throw sls::RuntimeError("Unknown frame mode " + s);
}

template <> inline defs::detectorModeType StringTo(const std::string &s) {
    if (s == "counting")
        return defs::COUNTING;
    if (s == "interpolating")
        return defs::INTERPOLATING;
    if (s == "analog")
        return defs::ANALOG;
    throw sls::RuntimeError("Unknown detector mode " + s);
}

template <> inline defs::dacIndex StringTo(const std::string &s) {
    if (s == "vcmp_ll")
        return defs::VCMP_LL;
    if (s == "vcmp_lr")
        return defs::VCMP_LR;
    if (s == "vcmp_rl")
        return defs::VCMP_RL;
    if (s == "vcmp_rr")
        return defs::VCMP_RR;
    if (s == "vthreshold")
        return defs::THRESHOLD;
    if (s == "vrf")
        return defs::VRF;
    if (s == "vrs")
        return defs::VRS;
    if (s == "vtr")
        return defs::VTR;
    if (s == "vcall")
        return defs::CAL;
    if (s == "vcp")
        return defs::VCP;
    throw sls::RuntimeError("Unknown dac Index " + s);
}

template <> inline defs::burstMode StringTo(const std::string &s) {
    if (s == "off")
        return defs::BURST_OFF;
    if (s == "internal")
        return defs::BURST_INTERNAL;
    if (s == "external")
        return defs::BURST_EXTERNAL;
    throw sls::RuntimeError("Unknown burst mode " + s);
}

template <> inline defs::timingSourceType StringTo(const std::string &s) {
    if (s == "internal")
        return defs::TIMING_INTERNAL;
    if (s == "external")
        return defs::TIMING_EXTERNAL;
    throw sls::RuntimeError("Unknown timing source type " + s);
}

template <> inline uint32_t StringTo(const std::string &s) {
    int base = s.find("0x") != std::string::npos ? 16 : 10;
    return std::stoul(s, nullptr, base);
}

template <> inline uint64_t StringTo(const std::string &s) {
    int base = s.find("0x") != std::string::npos ? 16 : 10;
    return std::stoull(s, nullptr, base);
}

template <> inline int StringTo(const std::string &s) {
    int base = s.find("0x") != std::string::npos ? 16 : 10;
    return std::stoi(s, nullptr, base);
}

template <> inline int64_t StringTo(const std::string &s) {
    int base = s.find("0x") != std::string::npos ? 16 : 10;
    return std::stol(s, nullptr, base);
}

/** For types with a .str() method use this for conversion */
template <typename T>
typename std::enable_if<has_str<T>::value, std::string>::type
ToString(const T &obj) {
    return obj.str();
}

} // namespace sls
