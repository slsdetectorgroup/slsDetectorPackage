#pragma once
#include <string>
#include <vector>
namespace sls {

class CtbConfig {
    static constexpr size_t name_length = 20;
    static constexpr size_t num_dacs = 18;
    static constexpr size_t num_adcs = 32;
    static constexpr const char *shm_tag_ = "ctbdacs";
    char dacnames[name_length * num_dacs]{};
    char adcnames[name_length * num_adcs]{};

    void check_dac_index(size_t i) const;
    void check_adc_index(size_t i) const;
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

    static const char *shm_tag();
};

} // namespace sls
