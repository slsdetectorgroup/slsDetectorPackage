// WARNING this file is auto generated
// changes might be overwritten at any time

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
    std::string frames(int action);
    std::vector<std::string> args;
    std::string cmd;
    Detector* det;
    int det_id{};

  private:
    using FunctionMap = std::map<std::string, std::string (Caller::*)(int)>;
    FunctionMap functions{{"frames", &Caller::frames}};
    Detector *ptr; //pointer to the detector that executes the command

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