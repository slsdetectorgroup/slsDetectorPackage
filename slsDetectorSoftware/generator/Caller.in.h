// This file is used as input to generate the caller class

#include "CmdParser.h"
#include "sls/Detector.h"

#include <string>
#include <vector>
#include <iostream>
namespace sls {

class Caller {
  public:
    Caller(Detector *ptr):det(ptr){}
    void call(const CmdParser &parser, int action, std::ostream &os=std::cout);

    IpAddr getDstIpFromAuto();
    IpAddr getSrcIpFromAuto();
    UdpDestination getUdpEntry();
    void GetLevelAndUpdateArgIndex(int action,
                                   std::string levelSeparatedCommand,
                                   int &level, int &iArg, size_t nGetArgs,
                                   size_t nPutArgs);
    void WrongNumberOfParameters(size_t expected);


    std::string list(int action);


    // THIS COMMENT IS GOING TO BE REPLACED BY THE ACTUAL CODE (1)


    std::vector<std::string> args;
    std::string cmd;
    Detector* det;
    int det_id{-1};
    int rx_id{-1};

  private:
    using FunctionMap = std::map<std::string, std::string (Caller::*)(int)>;
    Detector *ptr; //pointer to the detector that executes the command

    FunctionMap functions{{"list", &Caller::list},

    // THIS COMMENT IS GOING TO BE REPLACED BY THE ACTUAL CODE (2)

    };
    //some helper functions to print
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

};

} // namespace sls