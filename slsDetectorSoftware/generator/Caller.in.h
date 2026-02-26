// This file is used as input to generate the caller class
#pragma once
#include "CmdParser.h"
#include "HelpDacs.h"
#include "sls/Detector.h"

#include <iostream>
#include <string>
#include <vector>
namespace sls {

class Caller {
  public:
    Caller(Detector *ptr) : det(ptr) {}
    void call(const std::string &command,
              const std::vector<std::string> &arguments, int detector_id,
              int action, std::ostream &os = std::cout, int receiver_id = -1);

    IpAddr getDstIpFromAuto();
    IpAddr getSrcIpFromAuto();
    UdpDestination getUdpEntry();
    int GetLevelAndInsertIntoArgs(std::string levelSeparatedCommand);
    void WrongNumberOfParameters(size_t expected);
    std::vector<defs::ROI> parseRoiVector(const std::string &input);
    defs::ROI parseRoi(const std::vector<std::string> &args);

    template <typename V> std::string OutStringHex(const V &value) {
        if (value.equal())
            return ToStringHex(value.front());
        return ToStringHex(value);
    }

    template <typename V> std::string OutStringHex(const V &value, int width) {
        if (value.equal())
            return ToStringHex(value.front(), width);
        return ToStringHex(value, width);
    }

    template <typename V> std::string OutString(const Result<V> &value) {
        if (value.equal())
            return ToString(value.front());
        return ToString(value);
    }

    template <typename V> std::string OutString(const V &value) {
        return ToString(value);
    }

    template <typename V>
    std::string OutString(const V &value, const std::string &unit) {
        if (value.equal())
            return ToString(value.front(), unit);
        return ToString(value, unit);
    }

    std::vector<std::string> getAllCommands();
    std::map<std::string, std::string> GetDeprecatedCommands();
    std::string list(int action);

    // THIS COMMENT TO BE REPLACED BY THE ACTUAL CODE (1)

    std::vector<std::string> args;
    std::string cmd;
    Detector *det;
    int det_id{-1};
    int rx_id{-1};

  private:
    bool ReplaceIfDeprecated(std::string &command);
    void SuggestIfRemoved(const std::string &command);
    using FunctionMap = std::map<std::string, std::string (Caller::*)(int)>;
    using StringMap = std::map<std::string, std::string>;
    Detector *ptr; // pointer to the detector that executes the command

    static void EmptyDataCallBack(detectorData *data, uint64_t frameIndex,
                                  uint32_t subFrameIndex, void *this_pointer);

    std::string bitoperations(int action);

    // parsing from args
    // parse from string to RegisterAddress
    RegisterAddress parseRegisterAddress(const std::string &addr) const;
    // parse from 2 strings to BitAddress
    BitAddress parseBitAddress(const std::string &addr,
                               const std::string &bitPos) const;
    // parse from string to RegisterValue
    RegisterValue parseRegisterValue(const std::string &addr) const;
    // parse validate flag from args and remove it from args
    bool parseValidate();

    // parses from args, but also gets addresses from shared memory if
    // applicable
    RegisterAddress getRegisterAddress(const std::string &saddr) const;
    BitAddress getBitAddress() const;

    FunctionMap functions{
        {"list", &Caller::list},

        // THIS COMMENT TO BE REPLACED BY THE ACTUAL CODE (2)

    };

    StringMap deprecated_functions{

        // THIS COMMENT TO BE REPLACED BY THE ACTUAL CODE (3)

    };

    StringMap removed_functions{

        // THIS COMMENT TO BE REPLACED BY THE ACTUAL CODE (4)

    };
};

} // namespace sls