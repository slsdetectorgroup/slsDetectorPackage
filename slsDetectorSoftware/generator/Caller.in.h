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
    /**
     * very special functions
    */
    std::string Acquire(int action);
    std::string Versions(int action);
    std::string Threshold(int action);
    std::string TrimEnergies(int action);
    std::string BadChannels(int action);
    std::string CurrentSource(int action);
    std::string DacValues(int action);
    std::string UDPSourceIP(int action);
    std::string UDPSourceIP2(int action);
    std::string UDPDestinationIP(int action);
    std::string UDPDestinationIP2(int action);
    std::string ReceiverHostname(int action);
    std::string Rx_ROI(int action);
    std::string RateCorrection(int action);
    std::string BurstMode(int action);
    std::string VetoStreaming(int action);
    std::string Counters(int action);
    std::string GainCaps(int action);
    std::string Samples(int action);
    std::string SlowADC(int action);
    std::string ReceiverDbitList(int action);
    std::string AdditionalJsonHeader(int action);
    std::string ExecuteCommand(int action);
    std::string Hostname(int action);


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
    /**
     * very special functions
    */
            {"hostname", &Caller::Hostname},

    {"acquire", &Caller::Acquire},
        {"versions", &Caller::Versions},
        {"threshold", &Caller::Threshold},
        {"trimen", &Caller::TrimEnergies},
        {"badchannels", &Caller::BadChannels},
        {"dacvalues", &Caller::DacValues},
        {"burstmode", &Caller::BurstMode},
        {"currentsource", &Caller::CurrentSource},
        {"udp_srcip", &Caller::UDPSourceIP},
        {"udp_srcip2", &Caller::UDPSourceIP2},
        {"udp_dstip", &Caller::UDPDestinationIP},
        {"udp_dstip2", &Caller::UDPDestinationIP2},
        {"rx_hostname", &Caller::ReceiverHostname},
        {"rx_roi", &Caller::Rx_ROI},
        {"ratecorr", &Caller::RateCorrection},
        {"burstmode", &Caller::BurstMode},
        {"vetostream", &Caller::VetoStreaming},
        {"counters", &Caller::Counters},
        {"gaincaps", &Caller::GainCaps},
        {"samples", &Caller::Samples},
        {"slowadc", &Caller::SlowADC},
        {"rx_dbitlist", &Caller::ReceiverDbitList},
        {"rx_jsonaddheader", &Caller::AdditionalJsonHeader},
        {"execcommand", &Caller::ExecuteCommand},
        {"thresholdnotb", &Caller::Threshold},


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