#pragma once

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "logger.h"
#include "sls_detector_exceptions.h"

namespace sls {

template <typename T> class CmdProxy {
  public:
    explicit CmdProxy(T *detectorPtr) : det(detectorPtr) {}

    std::string Call(const std::string &command,
                     const std::vector<std::string> &arguments,
                     int detector_id) {
        cmd = command;
        args = arguments;
        det_id = detector_id;

        ReplaceIfDepreciated(cmd);

        auto it = functions.find(cmd);
        if (it != functions.end()) {
            std::cout << ((*this).*(it->second))();
            return {};
        } else {
            return cmd;
        }
    }

    bool ReplaceIfDepreciated(std::string &command) {
        auto d_it = depreciated_functions.find(command);
        if (d_it != depreciated_functions.end()) {
            FILE_LOG(logWARNING)
                << command
                << " is depreciated and will be removed. Please migrate to: "
                << d_it->second;
            command = d_it->second;
            return true;
        }
        return false;
    }

    size_t GetFunctionMapSize() const noexcept { return functions.size(); };

  private:
    T *det;
    std::string cmd;
    std::vector<std::string> args;
    int det_id{-1};

    using FunctionMap = std::map<std::string, std::string (CmdProxy::*)()>;
    using StringMap = std::map<std::string, std::string>;

    // Initialize maps for translating name and function
    FunctionMap functions{{"newfunc", &CmdProxy::NewFunction}};

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
                                    {"overwrite", "rx_overwrite"}};

    template <typename U> std::string ResultToString(const U &ret) {
        std::ostringstream os;
        if (det_id != -1)
            os << det_id << ":";
        os << cmd << " " << ret << "\n";
        return os.str();
    }

    void WrongNumberOfParameters(size_t expected) {
        throw RuntimeError("ERROR: Expected " + std::to_string(expected) +
                           " parameters but got " +
                           std::to_string(args.size()) + "\n");
    }

    // Mapped functions

    // example
    std::string NewFunction() {
        if (args.size() == 0) {
            std::cout << "This is the new function function\n";
            return ResultToString(det->setExposureTime(-1, true));
        } else if (args.size() == 1) {
            std::cout << "Setting exposure time to " << args[0] << "s\n";
            return ResultToString(
                det->setExposureTime(std::stod(args[0]), true, det_id));
        } else {
            WrongNumberOfParameters(1);
            return {};
        }
    }
};

} // namespace sls
