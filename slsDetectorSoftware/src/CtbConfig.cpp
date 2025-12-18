
#include "CtbConfig.h"
#include "SharedMemory.h"
#include "sls/ToString.h"
#include "sls/string_utils.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace sls {

CtbConfig::CtbConfig() {
    for (size_t i = 0; i != num_dacs; ++i) {
        setDacName(i, "DAC" + ToString(i));
    }
    for (size_t i = 0; i != num_adcs; ++i) {
        setAdcName(i, "ADC" + ToString(i));
    }
    for (size_t i = 0; i != num_signals; ++i) {
        setSignalName(i, "BIT" + ToString(i));
    }
    setPowerName(0, "VA");
    setPowerName(1, "VB");
    setPowerName(2, "VC");
    setPowerName(3, "VD");
    setPowerName(4, "VIO");
    for (size_t i = 0; i != num_slowADCs; ++i) {
        setSlowADCName(i, "SLOWADC" + ToString(i));
    }
    registers.clear();
    bits.clear();
}

const char *CtbConfig::shm_tag() { return shm_tag_; }

void CtbConfig::check_index(size_t index, size_t max, const std::string &name,
                            const std::string &suffix) const {
    if (index >= max) {
        std::ostringstream oss;
        oss << "Invalid " << name << " index. Options: 0 - " << max;
        if (!suffix.empty()) {
            oss << " " << suffix;
        }
        throw RuntimeError(oss.str());
    }
}

void CtbConfig::set_name(const std::string &name, char dst[][name_length],
                         size_t index) {
    if (name.empty())
        throw RuntimeError("Name needs to be at least one character");

    strcpy_checked<name_length>(dst[index], name);
}

void CtbConfig::setNames(const std::vector<std::string> &names,
                         size_t expected_size,
                         void (CtbConfig::*setNameFunc)(size_t,
                                                        const std::string &)) {
    if (names.size() != expected_size) {
        throw RuntimeError("Name list need to be of size " +
                           std::to_string(expected_size));
    }
    for (size_t i = 0; i != expected_size; ++i) {
        (this->*setNameFunc)(i, names[i]);
    }
}

std::vector<std::string>
CtbConfig::getNames(size_t expected_size,
                    std::string (CtbConfig::*getNameFunc)(size_t) const) const {
    std::vector<std::string> result;
    result.reserve(expected_size);
    for (size_t i = 0; i != expected_size; ++i) {
        result.push_back((this->*getNameFunc)(i));
    }
    return result;
}

void CtbConfig::setDacName(size_t index, const std::string &name) {
    check_index(index, num_dacs, "DAC");
    set_name(name, dacnames, index);
}

void CtbConfig::setDacNames(const std::vector<std::string> &names) {
    setNames(names, num_dacs, &CtbConfig::setDacName);
}

std::string CtbConfig::getDacName(size_t index) const {
    check_index(index, num_dacs, "DAC");
    return dacnames[index];
}

std::vector<std::string> CtbConfig::getDacNames() const {
    return getNames(num_dacs, &CtbConfig::getDacName);
}

void CtbConfig::setAdcName(size_t index, const std::string &name) {
    check_index(index, num_adcs, "ADC");
    set_name(name, adcnames, index);
}

void CtbConfig::setAdcNames(const std::vector<std::string> &names) {
    setNames(names, num_adcs, &CtbConfig::setAdcName);
}

std::string CtbConfig::getAdcName(size_t index) const {
    check_index(index, num_adcs, "ADC");
    return adcnames[index];
}

std::vector<std::string> CtbConfig::getAdcNames() const {
    return getNames(num_adcs, &CtbConfig::getAdcName);
}

void CtbConfig::setSignalName(size_t index, const std::string &name) {
    check_index(index, num_signals, "Signal");
    set_name(name, signalnames, index);
}

void CtbConfig::setSignalNames(const std::vector<std::string> &names) {
    setNames(names, num_signals, &CtbConfig::setSignalName);
}

std::string CtbConfig::getSignalName(size_t index) const {
    check_index(index, num_signals, "Signal");
    return signalnames[index];
}

std::vector<std::string> CtbConfig::getSignalNames() const {
    return getNames(num_signals, &CtbConfig::getSignalName);
}

void CtbConfig::setPowerName(size_t index, const std::string &name) {
    check_index(index, num_powers, "Power");
    set_name(name, powernames, index);
}

void CtbConfig::setPowerNames(const std::vector<std::string> &names) {
    setNames(names, num_powers, &CtbConfig::setPowerName);
}

std::string CtbConfig::getPowerName(size_t index) const {
    check_index(index, num_powers, "Power");
    return powernames[index];
}

std::vector<std::string> CtbConfig::getPowerNames() const {
    return getNames(num_powers, &CtbConfig::getPowerName);
}

void CtbConfig::setSlowADCName(size_t index, const std::string &name) {
    check_index(index, num_slowADCs, "Slow ADC", "or SLOW_ADC0 - SLOW_ADC7");
    set_name(name, slowADCnames, index);
}

void CtbConfig::setSlowADCNames(const std::vector<std::string> &names) {
    setNames(names, num_slowADCs, &CtbConfig::setSlowADCName);
}

std::string CtbConfig::getSlowADCName(size_t index) const {
    check_index(index, num_slowADCs, "Slow ADC", "or SLOW_ADC0 - SLOW_ADC7");
    return slowADCnames[index];
}

std::vector<std::string> CtbConfig::getSlowADCNames() const {
    return getNames(num_slowADCs, &CtbConfig::getSlowADCName);
}

int CtbConfig::getRegisterNamesCount() const { return registers.size(); }

bool CtbConfig::hasRegisterName(const std::string &name) const {
    auto fixed_name = FixedString<name_length>(name);
    return registers.containsKey(fixed_name);
}

bool CtbConfig::hasRegisterAddress(RegisterAddress addr) const {
    return registers.hasValue(addr);
}

void CtbConfig::clearRegisterNames() { registers.clear(); }

void CtbConfig::setRegisterName(const std::string &name, RegisterAddress addr) {
    try {
        auto fixed_name = FixedString<name_length>(name);
        registers.addKeyOrSetValue(fixed_name, addr);
    } catch (const std::runtime_error &e) {
        std::ostringstream oss;
        oss << e.what();
        if (strstr(e.what(), "Maximum capacity reached")) {
            oss << ". Clear shared memory and try again.";
        }
        throw RuntimeError("Could not set register name '" + name +
                           "': " + oss.str());
    }
}

RegisterAddress CtbConfig::getRegisterAddress(const std::string &name) const {
    try {
        auto fixed_name = FixedString<name_length>(name);
        return registers.getValue(fixed_name);
    } catch (const std::runtime_error &e) {
        throw RuntimeError("Could not get register address for name '" + name +
                           "': " + std::string(e.what()));
    }
}

std::string CtbConfig::getRegisterName(RegisterAddress addr) const {
    try {
        return registers.getKey(addr).str();
    } catch (const std::runtime_error &e) {
        throw RuntimeError("Could not get register name for address '" +
                           addr.str() + "': " + std::string(e.what()));
    }
}

void CtbConfig::setRegisterNames(
    const std::map<std::string, RegisterAddress> &list) {
    try {
        std::map<FixedString<name_length>, RegisterAddress> fixed_list;
        for (const auto &[name, value] : list) {
            auto fixed_name = FixedString<name_length>(name);
            fixed_list[fixed_name] = value;
        }
        registers.setMap(fixed_list);
    } catch (const std::runtime_error &e) {
        throw RuntimeError("Could not set register names: " +
                           std::string(e.what()));
    }
}

std::map<std::string, RegisterAddress> CtbConfig::getRegisterNames() const {
    auto fixed_result = registers.getMap();
    std::map<std::string, RegisterAddress> result;
    for (const auto &[fixed_name, value] : fixed_result) {
        result[fixed_name.str()] = value;
    }
    return result;
}

int CtbConfig::getBitNamesCount() const { return bits.size(); }

bool CtbConfig::hasBitName(const std::string &name) const {
    auto fixed_name = FixedString<name_length>(name);
    return bits.containsKey(fixed_name);
}

bool CtbConfig::hasBitAddress(BitAddress addr) const {
    return bits.hasValue(addr);
}

void CtbConfig::clearBitNames() { bits.clear(); }

void CtbConfig::setBitName(const std::string &name, BitAddress addr) {
    try {
        auto fixed_name = FixedString<name_length>(name);
        bits.addKeyOrSetValue(fixed_name, addr);
    } catch (const std::runtime_error &e) {
        std::ostringstream oss;
        oss << e.what();
        if (strstr(e.what(), "Maximum capacity reached")) {
            oss << ". Clear shared memory and try again.";
        }
        throw RuntimeError("Could not set bit name '" + name +
                           "': " + oss.str());
    }
}

BitAddress CtbConfig::getBitAddress(const std::string &name) const {
    try {
        auto fixed_name = FixedString<name_length>(name);
        return bits.getValue(fixed_name);
    } catch (const std::runtime_error &e) {
        throw RuntimeError("Could not get bit address for name '" + name +
                           "': " + std::string(e.what()));
    }
}

std::string CtbConfig::toRegisterNameBitString(BitAddress addr) const {
    std::ostringstream oss;
    if (registers.hasValue(addr.address())) {
        oss << "[" << registers.getKey(addr.address()).str() << ", "
            << std::to_string(addr.bitPosition()) << "]";
    } else {
        oss << addr.str();
    }
    return oss.str();
}

std::string CtbConfig::getBitName(BitAddress addr) const {
    try {
        return bits.getKey(addr).str();
    } catch (const std::runtime_error &e) {
        std::ostringstream oss;
        oss << "Could not get bit name for bit address ";
        oss << "'" << toRegisterNameBitString(addr) << "'";
        oss << ":" << e.what();
        throw RuntimeError(oss.str());
    }
}

void CtbConfig::setBitNames(const std::map<std::string, BitAddress> &list) {
    try {
        std::map<FixedString<name_length>, BitAddress> fixed_list;
        for (const auto &[name, value] : list) {
            auto fixed_name = FixedString<name_length>(name);
            fixed_list[fixed_name] = value;
        }
        bits.setMap(fixed_list);
    } catch (const std::runtime_error &e) {
        throw RuntimeError("Could not set bit names: " + std::string(e.what()));
    }
}

std::map<std::string, BitAddress> CtbConfig::getBitNames() const {
    auto fixed_result = bits.getMap();
    std::map<std::string, BitAddress> result;
    for (const auto &[fixed_name, value] : fixed_result) {
        result[fixed_name.str()] = value;
    }
    return result;
}

} // namespace sls