#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "ToString.h"
#include "logger.h"
#include "slsDetectorCommand.h"
#include "sls_detector_defs.h"
#include "sls_detector_exceptions.h"
#include "string_utils.h"
#include "TimeHelper.h"

namespace sls {

template <typename T> class CmdProxy {
  public:
    explicit CmdProxy(T *detectorPtr) : det(detectorPtr) {}

    std::string Call(const std::string &command,
                     const std::vector<std::string> &arguments, int detector_id,
                     int action = -1) {
        cmd = command;
        args = arguments;
        det_id = detector_id;

        ReplaceIfDepreciated(cmd);

        auto it = functions.find(cmd);
        if (it != functions.end()) {
            std::cout << ((*this).*(it->second))(action);
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

    using FunctionMap = std::map<std::string, std::string (CmdProxy::*)(int)>;
    using StringMap = std::map<std::string, std::string>;

    // Initialize maps for translating name and function
    FunctionMap functions{{"list", &CmdProxy::ListCommands},
                          {"exptime2", &CmdProxy::Exptime}};

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

    void WrongNumberOfParameters(size_t expected) {
        throw RuntimeError(
            "Command " + cmd + " expected <=" + std::to_string(expected) +
            " parameter/s but got " + std::to_string(args.size()) + "\n");
    }

    // Mapped functions

    std::string ListCommands(int action) {
        if (action == slsDetectorDefs::HELP_ACTION)
            return "list - lists all available commands, list deprecated - "
                   "list deprecated commands\n";

        if (args.size() == 0) {
            auto commands = slsDetectorCommand(nullptr).getAllCommands();
            for (const auto &it : functions)
                commands.emplace_back(it.first);
            std::sort(begin(commands), end(commands));

            std::cout << "These " << commands.size()
                      << " commands are available\n";
            for (auto &c : commands)
                std::cout << c << '\n';
            return "";
        } else if (args.size() == 1) {
            if (args[0] == "deprecated") {
                std::cout << "The following " << depreciated_functions.size()
                          << " commands are deprecated\n";
                size_t field_width = 20;
                for (const auto &it : depreciated_functions) {
                    std::cout << std::right << std::setw(field_width)
                              << it.first << " -> " << it.second << '\n';
                }
                return "";
            } else {
                throw RuntimeError(
                    "Could not decode argument. Possible options: deprecated");
            }
        } else {
            WrongNumberOfParameters(1);
            return "";
        }
    }

    std::string Exptime(int action) {
        std::ostringstream os;
        os << cmd << ' ';
        if (action == slsDetectorDefs::HELP_ACTION)
            os << "[duration] [(optional) unit]";
        else if (action == slsDetectorDefs::GET_ACTION) {
            auto t = det->getExptime().squash();
            if (args.size() == 0)
                os << ToString(t) << '\n';
            else if (args.size() == 1) {
                os << ToString(t, args[0]) << '\n';
            } else {
                WrongNumberOfParameters(2);
            }
        } else if (action == slsDetectorDefs::PUT_ACTION) {
            if (args.size() == 1) {
                std::string time_str(args[0]);
                std::string unit = RemoveUnit(time_str);
                auto t = StringTo<time::ns>(time_str, unit);
                det->setExptime(t);
            } else if (args.size() == 2) {
                auto t = StringTo<time::ns>(args[0], args[1]);
                det->setExptime(t);
            } else {
                WrongNumberOfParameters(2);
            }
            os << ToString(args) << '\n';
        } else {
            throw sls::RuntimeError("Unknown action");
        }
        return os.str();
    }
};

} // namespace sls
