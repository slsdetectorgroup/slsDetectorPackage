
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
        setDacName(i, "dac" + ToString(i));
    }
    for (size_t i = 0; i != num_adcs; ++i) {
        setAdcName(i, "adc" + ToString(i));
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

void CtbConfig::check_size(const std::string &name) const {

    if (name.empty())
        throw RuntimeError("Name needs to be at least one character");

    // dac/adc name_length -1 to account for \0 termination
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

const char *CtbConfig::shm_tag() { return shm_tag_; }

} // namespace sls