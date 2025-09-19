
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
}

void CtbConfig::check_dac_index(size_t i) const {
    if (i >= num_dacs) {
        std::ostringstream oss;
        oss << "Invalid DAC index. Options: 0 - " << num_dacs;
        throw RuntimeError(oss.str());
    }
}

void CtbConfig::check_adc_index(size_t i) const {
    if (i >= num_adcs) {
        std::ostringstream oss;
        oss << "Invalid ADC index. Options: 0 - " << num_adcs;
        throw RuntimeError(oss.str());
    }
}

void CtbConfig::check_signal_index(size_t i) const {
    if (i >= num_signals) {
        std::ostringstream oss;
        oss << "Invalid Signal index. Options: 0 - " << num_signals;
        throw RuntimeError(oss.str());
    }
}

void CtbConfig::check_power_index(size_t i) const {
    if (i >= num_powers) {
        std::ostringstream oss;
        oss << "Invalid Power index. Options: 0 - " << num_powers
            << " or V_POWER_A - V_POWER_IO";
        throw RuntimeError(oss.str());
    }
}

void CtbConfig::check_slow_adc_index(size_t i) const {
    if (i >= num_slowADCs) {
        std::ostringstream oss;
        oss << "Invalid Slow ADC index. Options: 0 - " << num_slowADCs
            << " or SLOW_ADC0 - SLOW_ADC7";
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

void CtbConfig::setSlowADCName(size_t index, const std::string &name) {
    check_slow_adc_index(index);
    check_size(name);
    char *dst = &slowADCnames[index * name_length];
    memset(dst, '\0', name_length);
    memcpy(dst, &name[0], name.size());
}

void CtbConfig::setSlowADCNames(const std::vector<std::string> &names) {
    if (names.size() != num_slowADCs) {
        throw RuntimeError("Slow ADC names need to be of size " +
                           std::to_string(num_slowADCs));
    }
    for (size_t i = 0; i != num_slowADCs; ++i) {
        setSlowADCName(i, names[i]);
    }
}

std::string CtbConfig::getSlowADCName(size_t index) const {
    check_slow_adc_index(index);
    return slowADCnames + index * name_length;
}

std::vector<std::string> CtbConfig::getSlowADCNames() const {
    std::vector<std::string> names;
    for (size_t i = 0; i != num_slowADCs; ++i)
        names.push_back(getSlowADCName(i));
    return names;
}

const char *CtbConfig::shm_tag() { return shm_tag_; }


std::optional<Entry*> CtbConfig::findEntryByName(const std::string &name, const bool is_register) {
    Entry* begin = is_register ? registernames : bitnames;
    Entry* end = begin + (is_register ? num_regs : num_bits);
    auto it = std::find_if(begin, end, [&name](const Entry &e) {
        return std::strncmp(e.key, name.c_str(), CTB_NAME_LENGTH) == 0;
    });

    if (it != end) return it;
    return std::nullopt;
}

// const overload
std::optional<const Entry*> CtbConfig::findEntryByName(const std::string &name, const bool is_register) const {
    const Entry* begin = is_register ? registernames : bitnames;
    const Entry* end = begin + (is_register ? num_regs : num_bits);
    auto it = std::find_if(begin, end, [&name](const Entry &e) {
        return std::strncmp(e.key, name.c_str(), CTB_NAME_LENGTH) == 0;
    });

    if (it != end) return it;
    return std::nullopt;
}

std::optional<const Entry*> CtbConfig::findEntryByValue(const int value, const bool is_register) const {
    const Entry* begin = is_register ? registernames : bitnames;
    const Entry* end = begin + (is_register ? num_regs : num_bits);
    auto it = std::find_if(begin, end, [&value](const Entry &e) {
        return e.value == value;
    });

    if (it != end) return it;
    return std::nullopt;
}

std::optional<int> CtbConfig::lookupEntryByName(const char* name, const bool is_register) const {
    auto entry = findEntryByName(name, is_register);
    return (entry ? std::optional<int>((*entry)->value) : std::nullopt);
}

std::optional<std::string> CtbConfig::lookupEntryByValue(const int value, const bool is_register) const {
    if (!is_register) {
        throw RuntimeError("Lookup by value only valid for registers");
    }
    auto entry = findEntryByValue(value, is_register);
    return (entry ? std::optional<std::string>((*entry)->key) : std::nullopt);
}

void CtbConfig::addEntry(const char* name, const int value, const bool is_register) {
    check_size(name);

    Entry* begin = is_register ? registernames : bitnames;
    size_t* size_ptr = is_register ? &num_regs : &num_bits;
    size_t max_size = is_register ? max_regs : max_bits;

    // exists: overwrite value
    if (auto entry = findEntryByName(name, is_register)) {
        (*entry)->value = value; 
        return;
    } 

    // check size
    if (*size_ptr >= max_size) {
        throw RuntimeError("Maximum number of " + std::string(is_register ? "registers" : "bits") + " reached. Clear shared memory and try again.");
    }

    // check value exists
    if (is_register) {
        if (auto addr_entry = findEntryByValue(value, is_register)) {
            throw RuntimeError("Address " + std::to_string(value) + " already assigned to " + std::string(is_register ? "register" : "bit") + " '" + std::string((*addr_entry)->key) + "'. Cannot assign to '" + name + "'");
        }
    }

    // add new entry
    std::strncpy(begin[*size_ptr].key, name, CTB_NAME_LENGTH - 1);
    begin[*size_ptr].key[CTB_NAME_LENGTH - 1] = '\0';
    begin[*size_ptr].value = value;
    ++(*size_ptr);
}


void CtbConfig::setRegisterName(const std::string &name, const int value) {
    addEntry(name.c_str(), value, true);
}

std::optional<int> CtbConfig::getRegisterAddress(const std::string &name) const {
    return lookupEntryByName(name.c_str(), true);
}

std::optional<std::string> CtbConfig::getRegisterName(const int value) const {
    return lookupEntryByValue(value, true);
}

void CtbConfig::clearRegisterNames() {
    memset(registernames, 0, sizeof(registernames));
    num_regs = 0;
}

void CtbConfig::setRegisterNames(const std::vector<std::pair<std::string, int>> &list) {
    if (list.size() > max_regs) {
        throw RuntimeError("Register names need to be of size less than " +
                           std::to_string(max_regs));
    }
    clearRegisterNames();
    for (const auto& [name, value] : list) {
        setRegisterName(name, value);
    }
}

std::vector<std::pair<std::string, int>> CtbConfig::getRegisterNames() const {
    std::vector<std::pair<std::string, int>> names;
    for (size_t i = 0; i != num_regs; ++i)
        names.push_back({std::string(registernames[i].key), registernames[i].value});
    return names;
}


void CtbConfig::setBitName(const std::string &name, const int value) {
    addEntry(name.c_str(), value, false);
}

std::optional<int> CtbConfig::getBitAddress(const std::string &name) const {
    return lookupEntryByName(name.c_str(), false);
}

void CtbConfig::clearBitNames() {
    memset(bitnames, 0, sizeof(bitnames));
    num_bits = 0;
}

void CtbConfig::setBitNames(const std::vector<std::pair<std::string, int>> &list) {
    if (list.size() > max_bits) {
        throw RuntimeError("Bit names need to be of size less than " +
                           std::to_string(max_bits));
    }
    clearBitNames();
    for (const auto& [name, value] : list) {
        setBitName(name, value);
    }
}

std::vector<std::pair<std::string, int>> CtbConfig::getBitNames() const {
    std::vector<std::pair<std::string, int>> names;
    for (size_t i = 0; i != num_bits; ++i)
        names.push_back({std::string(bitnames[i].key), bitnames[i].value});
    return names;
}


} // namespace sls