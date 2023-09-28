#pragma once
#include <string>
#include <vector>
namespace sls {

class CtbConfig {
    static constexpr size_t name_length = 20;
    static constexpr size_t num_dacs = 18;
    static constexpr size_t num_adcs = 32;
    static constexpr size_t num_signals = 64;
    static constexpr size_t num_powers = 5;
    static constexpr size_t num_slowADCs = 8;
    static constexpr const char *shm_tag_ = "ctbdacs";
    char dacnames[name_length * num_dacs]{};
    char adcnames[name_length * num_adcs]{};
    char signalnames[name_length * num_signals]{};
    char powernames[name_length * num_powers]{};
    char slowADCnames[name_length * num_slowADCs]{};

    void check_dac_index(size_t i) const;
    void check_adc_index(size_t i) const;
    void check_signal_index(size_t i) const;
    void check_power_index(size_t i) const;
    void check_slow_adc_index(size_t i) const;
    void check_size(const std::string &name) const;

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
};

} // namespace sls
