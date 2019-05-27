#pragma once

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace sls {

using VectorString = std::vector<std::string>;

template <typename T> class CmdProxy {
  public:
    explicit CmdProxy(T *detector_ptr);

    std::string Call(const std::string &cmd, const VectorString &arg,
                     int detector_id);
    void Help(VectorString commands);
    void PrintMappedFunctions();
    bool CommandExists(std::string command) {
        return functions.count(command);
    };
    bool HelpExists(std::string command) { return help_map.count(command); };
    VectorString getAllCommands();
    T &getMultiDetector() { return det; };
    bool ReplaceIfDepreciated(std::string &command);
    size_t GetFunctionMapSize() { return functions.size(); };

  private:
    using fmap_t = std::map<std::string, std::string (CmdProxy::*)()>;
    using smap_t = std::map<std::string, std::string>;

    T* det;
    fmap_t functions;
    smap_t depreciated_functions;
    smap_t help_map;

    std::string command_;
    VectorString arguments_;
    int detector_id_{-1};

    template <typename U> std::string resultToString(const U &ret);

    // Functions to be mapped
    std::string ExposureTime();


    void WrongNumberOfParameters(size_t n) {
        std::cout << "ERROR: Wrong number of parameters:" << n << "\n";
    };
};

template <typename T> CmdProxy<T>::CmdProxy(T* detectorPtr) : det(detectorPtr) {

    // map between strings and functions of the MultiDetectorCaller
    functions["exptime"] = &CmdProxy::ExposureTime;

    // Functions that will be removed
    depreciated_functions["oldvrf"] = "vrf";
    depreciated_functions["fname"] = "file_name";

    // Help
    help_map["exptime"] = "exptime \t exposure time in [s]\n";
}



template <typename T>
std::string CmdProxy<T>::Call(const std::string &command,
                              const VectorString &arg, int detector_id) {
    // TODO! (Erik) investigate the effects of setReceiverOnline when not using
    // receiver. Seems ok...
    det.setOnline(true);
    det.setReceiverOnline(true);

    // Store command args and detector id for further use
    // Probably also a good place to do sanity check
    command_ = command;
    arguments_ = arg;
    detector_id_ = detector_id;

    ReplaceIfDepreciated(command_);

    auto it = functions.find(command);
    if (it != functions.end()) {
        std::cout << ((*this).*(it->second))();
        return {};
    } else {
        return command_;
    }
}

template <typename T> void CmdProxy<T>::Help(VectorString commands) {
    std::cout << "\nHelp:\n\n";
    for (size_t i = 0; i < commands.size(); ++i) {
        ReplaceIfDepreciated(commands[i]);

        if (CommandExists(commands[i])) {
            auto it = help_map.find(commands[i]);
            if (it != help_map.end()) {
                std::cout << it->second;
            } else {
                std::cout << "help not found\n";
            }
        } else {
            std::cout << "Command not found\n";
            PrintMappedFunctions();
        }
    }
}
template <typename T>
bool CmdProxy<T>::ReplaceIfDepreciated(std::string &command) {
    // if the command is in a map of depreciated functions warn and replace the
    // command
    auto d_it = depreciated_functions.find(command);
    if (d_it != depreciated_functions.end()) {
        std::cout << "WARNING: " << command
                  << " is depreciated and will be removed. Please migrate to: "
                  << d_it->second << "\n";
        command = d_it->second;
        return true;
    }
    return false;
}

template <typename T> void CmdProxy<T>::PrintMappedFunctions() {
    std::cout << "The following commands are available: \n";
    for (auto it = functions.begin(); it != functions.end(); ++it) {
        std::cout << "\t" << it->first << std::endl;
    }
    std::cout << "\nDepreciaded commands: \n";
    for (auto it = depreciated_functions.begin();
         it != depreciated_functions.end(); ++it) {
        std::cout << "\t" << it->first << " -> " << it->second << std::endl;
    }
}

template <typename T> VectorString CmdProxy<T>::getAllCommands() {
    VectorString commands;
    for (auto const &v : functions)
        commands.push_back(v.first);

    return commands;
}

template <typename T>
template <typename U>
std::string CmdProxy<T>::resultToString(const U &ret) {
    std::ostringstream os;
    if (detector_id_ != -1)
        os << detector_id_ << ":";

    os << command_ << " " << ret << "\n";
    return os.str();
};

template <typename T> std::string CmdProxy<T>::ExposureTime() {
    // if (arguments_.size() == 1) {
    //     int64_t t = static_cast<int64_t>(std::stod(arguments_[0]) * 1e9);
    //     det.setExposureTime(t);
    // } else if (arguments_.size() > 1) {
    //     WrongNumberOfParameters(arguments_.size());
    // };

    // return resultToString(det.setExposureTime() / 1e9);
    return "ProxyFired\n";
}

} // namespace sls
