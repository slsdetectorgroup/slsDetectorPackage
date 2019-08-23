#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace sls {
class Detector;

class CmdProxy {
  public:
    explicit CmdProxy(Detector *ptr) : det(ptr) {}

    std::string Call(const std::string &command,
                     const std::vector<std::string> &arguments, int detector_id,
                     int action = -1, std::ostream &os = std::cout);

    bool ReplaceIfDepreciated(std::string &command);
    size_t GetFunctionMapSize() const noexcept { return functions.size(); };
    std::vector<std::string> GetAllCommands();
    std::vector<std::string> GetProxyCommands();

  private:
    Detector *det;
    std::string cmd;
    std::vector<std::string> args;
    int det_id{-1};

    template <typename V> std::string OutString(const V &value);
    template <typename V>
    std::string OutString(const V &value, const std::string &unit);

    using FunctionMap = std::map<std::string, std::string (CmdProxy::*)(int)>;
    using StringMap = std::map<std::string, std::string>;

    // Initialize maps for translating name and function
    FunctionMap functions{{"list", &CmdProxy::ListCommands},
                          {"exptime2", &CmdProxy::Exptime},
                          {"period2", &CmdProxy::Period},
                          {"subexptime2", &CmdProxy::SubExptime},
                          {"rx_fifodepth", &CmdProxy::RxFifoDepth},
                          {"rx_silent", &CmdProxy::RxSilent}};

    StringMap depreciated_functions{{"r_readfreq", "rx_readfreq"},
                                    {"r_padding", "rx_padding"},
                                    {"r_silent", "rx_silent"},
                                    {"r_lastclient", "rx_lastclient"},
                                    {"r_lock", "rx_lock"},
                                    {"r_online", "rx_online"},
                                    {"r_checkonline", "rx_checkonline"},
                                    {"r_framesperfile", "rx_framesperfile"},
                                    {"r_discardpolicy", "rx_discardpolicy"},
                                    {"receiverversion", "rx_version"},
                                    {"receiver", "rx_status"},
                                    {"index", "findex"},
                                    {"exitreceiver", "rx_exit"},
                                    {"enablefwrite", "fwrite"},
                                    {"checkrecversion", "rx_checkversion"},
                                    {"masterfile", "fmaster"},
                                    {"outdir", "fpath"},
                                    {"fileformat", "fformat"},
                                    {"overwrite", "foverwrite"}};

    void WrongNumberOfParameters(size_t expected);

    /* Commands */
    std::string ListCommands(int action);
    std::string Period(int action);
    std::string Exptime(int action);
    std::string SubExptime(int action);
    std::string RxFifoDepth(const int action);
    std::string RxSilent(const int action);
};

} // namespace sls
