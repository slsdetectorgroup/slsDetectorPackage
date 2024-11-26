// This file is used as input to generate the caller class

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
    void GetLevelAndUpdateArgIndex(int action,
                                   std::string levelSeparatedCommand,
                                   int &level, int &iArg, size_t nGetArgs,
                                   size_t nPutArgs);
    void WrongNumberOfParameters(size_t expected);

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
    using FunctionMap = std::map<std::string, std::string (Caller::*)(int)>;
    using StringMap = std::map<std::string, std::string>;
    Detector *ptr; // pointer to the detector that executes the command

    static void EmptyDataCallBack(detectorData *data, uint64_t frameIndex,
                                  uint32_t subFrameIndex, void *this_pointer);

    FunctionMap functions{
        {"list", &Caller::list},

        // THIS COMMENT TO BE REPLACED BY THE ACTUAL CODE (2)

    };

    StringMap deprecated_functions{

        // THIS COMMENT TO BE REPLACED BY THE ACTUAL CODE (3)

    };
};

} // namespace sls