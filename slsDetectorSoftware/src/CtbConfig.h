#pragma once
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace sls {

#define CTB_SHMAPIVERSION 0x250919
#define CTB_SHMVERSION    0x250919

#define CTB_NAME_LENGTH 32

struct Entry {
    char key[CTB_NAME_LENGTH]{};
    int value{0};
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
    static constexpr size_t max_regs = 64;
    static constexpr size_t max_bits = 64;
    size_t num_regs{0};
    size_t num_bits{0};

    char dacnames[name_length * num_dacs]{};
    char adcnames[name_length * num_adcs]{};
    char signalnames[name_length * num_signals]{};
    char powernames[name_length * num_powers]{};
    char slowADCnames[name_length * num_slowADCs]{};

    Entry registernames[max_regs]{};
    Entry bitnames[max_bits]{};

    void check_dac_index(size_t i) const;
    void check_adc_index(size_t i) const;
    void check_signal_index(size_t i) const;
    void check_power_index(size_t i) const;
    void check_slow_adc_index(size_t i) const;
    void check_size(const std::string &name) const;

    std::optional<Entry *> findEntryByName(const std::string &name,
                                           const bool is_register);
    std::optional<const Entry *> findEntryByName(const std::string &name,
                                                 const bool is_register) const;
    std::optional<const Entry *> findEntryByValue(const int value,
                                                  const bool is_register) const;
    std::optional<int> lookupEntryByName(const char *name,
                                         const bool is_register) const;
    std::optional<std::string> lookupEntryByValue(const int value,
                                                  const bool is_register) const;
    void addEntry(const char *name, const int value, const bool is_register);

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
    void setRegisterName(const std::string &name, const int value);
    std::optional<int> getRegisterAddress(const std::string &name) const;
    std::optional<std::string> getRegisterName(const int value) const;
    void clearRegisterNames();
    void setRegisterNames(const std::map<std::string, int> &list);
    std::map<std::string, int> getRegisterNames() const;

    int getBitNamesCount() const;
    void setBitName(const std::string &name, const int value);
    std::optional<int> getBitPosition(const std::string &name) const;
    void clearBitNames();
    void setBitNames(const std::map<std::string, int> &list);
    std::map<std::string, int> getBitNames() const;
};

} // namespace sls
