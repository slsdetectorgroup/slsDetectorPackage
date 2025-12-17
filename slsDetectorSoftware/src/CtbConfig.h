#pragma once

#include "MapOnStack.h"
#include "sls/bit_utils.h"
#include "sls/sls_detector_defs.h"
#include "sls/string_utils.h"

#include <vector>

namespace sls {

class CtbConfig {
  public:
    /** fixed pattern */
    static constexpr int SHM_VERSION = 0x251215;
    int shmversion{SHM_VERSION};
    bool isValid{true}; // false if freed to block access from python or c++ api
                        /** end of fixed pattern */

  private:
    static constexpr const char *shm_tag_ = "ctbdacs";
    static constexpr size_t name_length = 32;

    static constexpr size_t num_dacs = 18;
    static constexpr size_t num_adcs = 32;
    static constexpr size_t num_signals = 64;
    static constexpr size_t num_powers = 5;
    static constexpr size_t num_slowADCs = 8;
    char dacnames[num_dacs][name_length]{};
    char adcnames[num_adcs][name_length]{};
    char signalnames[num_signals][name_length]{};
    char powernames[num_powers][name_length]{};
    char slowADCnames[num_slowADCs][name_length]{};

    void check_index(size_t index, size_t max, const std::string &name,
                     const std::string &suffix = "") const;
    void set_name(const std::string &name, char dst[][name_length],
                  size_t index);

    void setNames(const std::vector<std::string> &names, size_t expected_size,
                  void (CtbConfig::*setNameFunc)(size_t, const std::string &));
    std::vector<std::string>
    getNames(size_t expected_size,
             std::string (CtbConfig::*getNameFunc)(size_t) const) const;

    static constexpr size_t Max_Named_Regs = 1024;
    static constexpr size_t Max_Named_Bits = 32 * 1024;
    MapOnStack<FixedString<name_length>, RegisterAddress, Max_Named_Regs, true>
        registers;
    MapOnStack<FixedString<name_length>, BitAddress, Max_Named_Bits, true> bits;

  public:
    CtbConfig();
    static const char *shm_tag();

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

    int getRegisterNamesCount() const;
    bool hasRegisterName(const std::string &name) const;
    bool hasRegisterAddress(RegisterAddress addr) const;
    void clearRegisterNames();
    void setRegisterName(const std::string &name, RegisterAddress addr);
    RegisterAddress getRegisterAddress(const std::string &name) const;
    std::string getRegisterName(RegisterAddress addr) const;
    void setRegisterNames(const std::map<std::string, RegisterAddress> &list);
    std::map<std::string, RegisterAddress> getRegisterNames() const;

    int getBitNamesCount() const;
    void clearBitNames();
    bool hasBitName(const std::string &name) const;
    bool hasBitAddress(BitAddress bitPos) const;
    void setBitName(const std::string &name, BitAddress bitPos);
    BitAddress getBitAddress(const std::string &name) const;
    std::string getBitName(BitAddress bitPos) const;
    void setBitNames(const std::map<std::string, BitAddress> &list);
    std::map<std::string, BitAddress> getBitNames() const;
};

} // namespace sls
