
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
    setPowerName(0, "VB");
    setPowerName(0, "VC");
    setPowerName(0, "VD");
    setPowerName(0, "VIO");
    for (size_t i = 0; i != num_senses; ++i) {
        setSenseName(i, "SENSE" + ToString(i));
    }
}

void CtbConfig::check_dac_index(size_t i) const {
    if (!(i < num_dacs)) {
        std::ostringstream oss;
        oss << "DAC index is too large. Needs to be below " << num_dacs;
        throw RuntimeError(oss.str());
    }
}

void CtbConfig::check_adc_index(size_t i) const {
    if (!(i < num_adcs)) {
        std::ostringstream oss;
        oss << "ADC index is too large.Needs to be below " << num_adcs;
        throw RuntimeError(oss.str());
    }
}

void CtbConfig::check_signal_index(size_t i) const {
    if (!(i < num_signals)) {
        std::ostringstream oss;
        oss << "Signal index is too large.Needs to be below " << num_signals;
        throw RuntimeError(oss.str());
    }
}

void CtbConfig::check_power_index(size_t i) const {
    if (!(i < num_powers)) {
        std::ostringstream oss;
        oss << "Power index is too large.Needs to be below " << num_powers;
        throw RuntimeError(oss.str());
    }
}

void CtbConfig::check_sense_index(size_t i) const {
    if (!(i < num_senses)) {
        std::ostringstream oss;
        oss << "Sense index is too large.Needs to be below " << num_senses;
        throw RuntimeError(oss.str());
    }
}

void CtbConfig::check_size(const std::string &name) const {

    if (name.empty())
        throw RuntimeError("Name needs to be at least one character");

    // name_length -1 to account for \0 termination
    if (!(name.size() < (name_length - 1))) {
        std::ostringstream oss;
        oss << "Length of name needs to be less than " << name_length - 1
            << " chars";
        throw RuntimeError(oss.str());
    }
}

void CtbConfig::setDacName(size_t index, const std::string &name) {
    check_dac_index(index);
    check_size(name);
    char *dst = &dacnames[index * name_length];
    memset(dst, '\0', name_length);
    memcpy(dst, &name[0], name.size());
}

void CtbConfig::setDacNames(const std::vector<std::string> &names) {
    if (names.size() != num_dacs) {
        throw RuntimeError("Dac names need to be of size " +
                           std::to_string(num_dacs));
    }
    for (size_t i = 0; i != num_dacs; ++i) {
        setDacName(i, names[i]);
    }
}

std::string CtbConfig::getDacName(size_t index) const {
    check_dac_index(index);
    return dacnames + index * name_length;
}

std::vector<std::string> CtbConfig::getDacNames() const {
    std::vector<std::string> names;
    for (size_t i = 0; i != num_dacs; ++i)
        names.push_back(getDacName(i));
    return names;
}

void CtbConfig::setAdcName(size_t index, const std::string &name) {
    check_adc_index(index);
    check_size(name);
    char *dst = &adcnames[index * name_length];
    memset(dst, '\0', name_length);
    memcpy(dst, &name[0], name.size());
}

void CtbConfig::setAdcNames(const std::vector<std::string> &names) {
    if (names.size() != num_adcs) {
        throw RuntimeError("Adc names need to be of size " +
                           std::to_string(num_adcs));
    }
    for (size_t i = 0; i != num_adcs; ++i) {
        setAdcName(i, names[i]);
    }
}

std::string CtbConfig::getAdcName(size_t index) const {
    check_adc_index(index);
    return adcnames + index * name_length;
}

std::vector<std::string> CtbConfig::getAdcNames() const {
    std::vector<std::string> names;
    for (size_t i = 0; i != num_adcs; ++i)
        names.push_back(getAdcName(i));
    return names;
}

void CtbConfig::setSignalName(size_t index, const std::string &name) {
    check_signal_index(index);
    check_size(name);
    char *dst = &signalnames[index * name_length];
    memset(dst, '\0', name_length);
    memcpy(dst, &name[0], name.size());
}

void CtbConfig::setSignalNames(const std::vector<std::string> &names) {
    if (names.size() != num_signals) {
        throw RuntimeError("Signal names need to be of size " +
                           std::to_string(num_signals));
    }
    for (size_t i = 0; i != num_signals; ++i) {
        setSignalName(i, names[i]);
    }
}

std::string CtbConfig::getSignalName(size_t index) const {
    check_signal_index(index);
    return signalnames + index * name_length;
}

std::vector<std::string> CtbConfig::getSignalNames() const {
    std::vector<std::string> names;
    for (size_t i = 0; i != num_signals; ++i)
        names.push_back(getSignalName(i));
    return names;
}

void CtbConfig::setPowerName(size_t index, const std::string &name) {
    check_power_index(index);
    check_size(name);
    char *dst = &powernames[index * name_length];
    memset(dst, '\0', name_length);
    memcpy(dst, &name[0], name.size());
}

void CtbConfig::setPowerNames(const std::vector<std::string> &names) {
    if (names.size() != num_powers) {
        throw RuntimeError("Power names need to be of size " +
                           std::to_string(num_powers));
    }
    for (size_t i = 0; i != num_powers; ++i) {
        setPowerName(i, names[i]);
    }
}

std::string CtbConfig::getPowerName(size_t index) const {
    check_power_index(index);
    return powernames + index * name_length;
}

std::vector<std::string> CtbConfig::getPowerNames() const {
    std::vector<std::string> names;
    for (size_t i = 0; i != num_powers; ++i)
        names.push_back(getPowerName(i));
    return names;
}

void CtbConfig::setSenseName(size_t index, const std::string &name) {
    check_sense_index(index);
    check_size(name);
    char *dst = &sensenames[index * name_length];
    memset(dst, '\0', name_length);
    memcpy(dst, &name[0], name.size());
}

void CtbConfig::setSenseNames(const std::vector<std::string> &names) {
    if (names.size() != num_senses) {
        throw RuntimeError("Sense names need to be of size " +
                           std::to_string(num_senses));
    }
    for (size_t i = 0; i != num_senses; ++i) {
        setSenseName(i, names[i]);
    }
}

std::string CtbConfig::getSenseName(size_t index) const {
    check_sense_index(index);
    return sensenames + index * name_length;
}

std::vector<std::string> CtbConfig::getSenseNames() const {
    std::vector<std::string> names;
    for (size_t i = 0; i != num_senses; ++i)
        names.push_back(getSenseName(i));
    return names;
}

const char *CtbConfig::shm_tag() { return shm_tag_; }

} // namespace sls