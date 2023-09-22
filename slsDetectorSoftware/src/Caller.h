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

std::string adcenable(int action);
std::string adcenable10g(int action);
std::string adcinvert(int action);
std::string burstperiod(int action);
std::string compdisabletime(int action);
std::string delay(int action);
std::string delayl(int action);
std::string exptime(int action);
std::string exptime1(int action);
std::string exptime2(int action);
std::string exptime3(int action);
std::string exptimel(int action);
std::string fname(int action);
std::string fpath(int action);
std::string frames(int action);
std::string frametime(int action);
std::string measuredperiod(int action);
std::string measuredsubperiod(int action);
std::string patioctrl(int action);
std::string patmask(int action);
std::string patsetbit(int action);
std::string period(int action);
std::string periodl(int action);
std::string runtime(int action);
std::string settingspath(int action);
std::string storagecell_delay(int action);
std::string subdeadtime(int action);
std::string subexptime(int action);
std::string transceiverenable(int action);

    std::vector<std::string> args;
    std::string cmd;
    Detector* det;
    int det_id{};

  private:
    using FunctionMap = std::map<std::string, std::string (Caller::*)(int)>;
    Detector *ptr; //pointer to the detector that executes the command

FunctionMap functions{{"adcenable", &Caller::adcenable},{"adcenable10g", &Caller::adcenable10g},{"adcinvert", &Caller::adcinvert},{"burstperiod", &Caller::burstperiod},{"compdisabletime", &Caller::compdisabletime},{"delay", &Caller::delay},{"delayl", &Caller::delayl},{"exptime", &Caller::exptime},{"exptime1", &Caller::exptime1},{"exptime2", &Caller::exptime2},{"exptime3", &Caller::exptime3},{"exptimel", &Caller::exptimel},{"fname", &Caller::fname},{"fpath", &Caller::fpath},{"frames", &Caller::frames},{"frametime", &Caller::frametime},{"measuredperiod", &Caller::measuredperiod},{"measuredsubperiod", &Caller::measuredsubperiod},{"patioctrl", &Caller::patioctrl},{"patmask", &Caller::patmask},{"patsetbit", &Caller::patsetbit},{"period", &Caller::period},{"periodl", &Caller::periodl},{"runtime", &Caller::runtime},{"settingspath", &Caller::settingspath},{"storagecell_delay", &Caller::storagecell_delay},{"subdeadtime", &Caller::subdeadtime},{"subexptime", &Caller::subexptime},{"transceiverenable", &Caller::transceiverenable}};
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