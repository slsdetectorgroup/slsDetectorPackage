#pragma once

#include "sls/sls_detector_defs.h"
#include "sls/string_utils.h"
#include "sls/bit_utils.h"

#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace sls {

#define CTB_SHMAPIVERSION 0x251031
#define CTB_SHMVERSION    0x251031

#define CTB_NAME_LENGTH 32


class RegisterDefinition {
    private:
        char name_[CTB_NAME_LENGTH]{};
        RegisterAddress addr_;
    
    public:
        RegisterDefinition(const std::string& name, RegisterAddress address)
        : addr_(address) {
            if (name.empty()) {
                throw sls::RuntimeError("Register name cannot be empty.");
            }
            strcpy_checked(name_, name);
        }

        std::string name() const noexcept { return name_; }
        RegisterAddress value() const noexcept { return addr_; }
};

class BitDefinition {
    private:
        char name_[CTB_NAME_LENGTH]{};
        BitPosition bitPos_;

    public:
        BitDefinition(const std::string& name, BitPosition bitPos)
        : bitPos_(bitPos) {
            if (name.empty()) {
                throw sls::RuntimeError("Bit name cannot be empty.");
            }
            strcpy_checked(name_, name);  
        }

        std::string name() const noexcept { return name_; }
        BitPosition value() const noexcept { return bitPos_; }
};

class CtbConfig {
  public:
    /** fixed pattern */
    int shmversion{CTB_SHMVERSION};
    bool isValid{true}; // false if freed to block access from python or c++ api
                        /** end of fixed pattern */
  private:
    static constexpr const char *shm_tag_ = "ctbdacs";
    static constexpr size_t name_length = CTB_NAME_LENGTH;
    static constexpr size_t num_dacs = 18;
    static constexpr size_t num_adcs = 32;
    static constexpr size_t num_signals = 64;
    static constexpr size_t num_powers = 5;
    static constexpr size_t num_slowADCs = 8;
    char dacnames[name_length * num_dacs]{};
    char adcnames[name_length * num_adcs]{};
    char signalnames[name_length * num_signals]{};
    char powernames[name_length * num_powers]{};
    char slowADCnames[name_length * num_slowADCs]{};

    static constexpr size_t max_regs = 64;
    static constexpr size_t max_bits = 64;
    size_t num_regs{0};
    size_t num_bits{0};
    RegisterDefinition registers[max_regs];
    BitDefinition bits[max_bits];

    void check_dac_index(size_t i) const;
    void check_adc_index(size_t i) const;
    void check_signal_index(size_t i) const;
    void check_power_index(size_t i) const;
    void check_slow_adc_index(size_t i) const;
    void check_size(const std::string &name) const;


    /*template <typename T>
    std::optional<T *> findEntryByName(const std::string &name, T* array, size_t count) {
        T *begin = is_register ? registers : bits;
        Entry *end = begin + (is_register ? num_regs : num_bits);

        auto it = std::find_if(array, array + count, [&name](const T& e) {
            return std::strncmp(e.key, name.c_str(), CTB_NAME_LENGTH) == 0;
        });

        if (it != array + count)
            return it;
        return std::nullopt;
    }*/

    template <typename T>
    std::optional<const T *> findEntryByName(const std::string &name, T* array, size_t count) const {
        auto it = std::find_if(array, array + count, [&name](const T& e) {
            return std::strncmp(e.name(), name.c_str(), CTB_NAME_LENGTH) == 0;
        });

        if (it != array + count)
            return it;
        return std::nullopt;
    }


    template <typename T, typename Tval>
    std::optional<const T *> findEntryByValue(Tval value, T* array, size_t count) const {
        auto it = std::find_if(array, array + count, [&value](const T& e) {
            return e.value() == value;
        });

        if (it != array + count)
            return it;
        return std::nullopt;
    }
         
    template <typename T, typename Tval>
    std::optional<Tval> lookupEntryByName(const std::string &name, const T* array, size_t count) const {
        auto entry = findEntryByName<T>(name, array, count);
        return (entry ? std::optional<Tval>((*entry)->value()) : std::nullopt);
    }

    template <typename T, typename Tval>
    std::optional<std::string> lookupEntryByValue(Tval value, const T* array, size_t count) const {
        auto entry = findEntryByValue<T>(value, array, count);
        return (entry ? std::optional<std::string>((*entry)->name()) : std::nullopt);
    }

    template <typename T, typename Tval>   
    void addEntry(const std::string& name, Tval value, T* array, size_t& count, size_t max_count, const std::string& type_name) {
        check_size(name);

        // exists: overwrite value
        if (auto entry = findEntryByName<T>(name, array, count)) {
            (*entry)->value = value;
            return;
        }

        // check size
        if (count >= max_count) {
            throw RuntimeError("Maximum number of " + type_name + " reached. Clear shared memory and try again.");
        }

        // check value exists
        if (auto entry = findEntryByValue<T>(value, array, count)) {
            throw RuntimeError(value.str() + " already assigned to " + typename + " '" + std::string((*entry)->name()) + "'. Cannot assign to '" + name + "'");
        }

        // add new entry
        array[count] = T(name, value);
        ++(count);
    }

  public:
    CtbConfig();
    CtbConfig(const CtbConfig &) = default;
    CtbConfig(CtbConfig &&) = default;
    CtbConfig &operator=(const CtbConfig &) = default;
    ~CtbConfig() = default;

    void setDacNames(const std::vector<std::string> &names);
    void setDacName(size_t index, const std::string &name);
    std::string getDacName(size_t index) const;
    std::vector<std::string> getDacNames() const;

    void setAdcNames(const std::vector<std::string> &names);
    void setAdcName(size_t index, const std::string &name);
    std::string getAdcName(size_t index) const;
    std::vector<std::string> getAdcNames() const;

    void setSignalNames(const std::vector<std::string> &names);
    void setSignalName(size_t index, const std::string &name);
    std::string getSignalName(size_t index) const;
    std::vector<std::string> getSignalNames() const;

    void setPowerNames(const std::vector<std::string> &names);
    void setPowerName(size_t index, const std::string &name);
    std::string getPowerName(size_t index) const;
    std::vector<std::string> getPowerNames() const;

    void setSlowADCNames(const std::vector<std::string> &names);
    void setSlowADCName(size_t index, const std::string &name);
    std::string getSlowADCName(size_t index) const;
    std::vector<std::string> getSlowADCNames() const;
    static const char *shm_tag();

    int getRegisterNamesCount() const;
    void setRegisterName(const std::string &name, RegisterAddress addr);
    std::optional<RegisterAddress> getRegisterAddress(const std::string &name) const;
    std::optional<std::string> getRegisterName(RegisterAddress addr) const;
    void clearRegisterNames();
    void setRegisterNames(const std::map<std::string, RegisterAddress> &list);
    std::map<std::string, RegisterAddress> getRegisterNames() const;

    int getBitNamesCount() const;
    void setBitName(const std::string &name, BitPosition bitPos);
    std::optional<BitPosition> getBitPosition(const std::string &name) const;
    std::optional<std::string> getBitPosition(BitPosition bitPos) const;
    void clearBitNames();
    void setBitNames(const std::map<std::string, BitPosition> &list);
    std::map<std::string, BitPosition> getBitNames() const;
};

} // namespace sls
