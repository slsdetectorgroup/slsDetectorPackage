#ifndef MULTI_DETECTOR_CALLER_H
#define MULTI_DETECTOR_CALLER_H

#include <exception>
#include <iostream>
#include <map>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <vector>

using VectorString = std::vector<std::string>;

template <typename T> class CmdProxy {
  public:
    explicit CmdProxy(int multi_id);

    std::string Call(std::string cmd, VectorString arg, int detector_id);
    void Help(VectorString commands);
    void PrintMappedFunctions();
    bool CommandExists(std::string command) {
        return function_map.count(command);
    };
    bool HelpExists(std::string command) { return help_map.count(command); };
    VectorString getAllCommands();
    T &getMultiDetector() { return det; };
    bool ReplaceIfDepreciated(std::string &command);
    size_t GetFunctionMapSize() { return function_map.size(); };

  private:
    // Aliases to shorten code
    using Fmap = std::map<std::string, std::string (CmdProxy::*)()>;
    using Smap = std::map<std::string, std::string>;

    T det;
    Fmap function_map;
    Smap depreciated_function_map;
    Smap help_map;

    std::string command_;
    VectorString arguments_;
    int detector_id_{-1};

    template <typename U> std::string resultToString(const U &ret);

    // Functions to be mapped
    // std::string FileWrite();
    // std::string FileName();
    // std::string Hostname();
    std::string ExposureTime();
    // std::string OverWrite();
    // std::string SettingsDir();
    // std::string TenGiga();

    void WrongNumberOfParameters(size_t n) {
        std::cout << "ERROR: Wrong number of parameters:" << n << "\n";
    };
};

template <typename T> CmdProxy<T>::CmdProxy(int multi_id) : det(multi_id) {

    // map between strings and functions of the MultiDetectorCaller
    function_map["exptime"] = &CmdProxy::ExposureTime;
    // function_map["file_name"]   = &CmdProxy::FileName;
    // function_map["file_write"]  = &CmdProxy::FileWrite;
    // function_map["hostname"]    = &CmdProxy::Hostname;
    // function_map["overwrite"]   = &CmdProxy::OverWrite;
    // function_map["settingsdir"] = &CmdProxy::SettingsDir;
    // function_map["tengiga"]     = &CmdProxy::TenGiga;

    // Functions that will be removed
    depreciated_function_map["oldvrf"] = "vrf";
    depreciated_function_map["fname"] = "file_name";

    // Help
    help_map["exptime"] = "exptime \t exposure time in [s]\n";
    help_map["file_name"] =
        "file_name \t filename for the data without index and extension\n";
    help_map["file_write"] =
        "file_write \t Write data files 0 disabled 1 enabled\n";
    help_map["hostname"] = "hostname \t hostname of det\n";
    help_map["overwrite"] = "overwrite \t overwite files 0 for no 1 for yes\n";
    help_map["settingsdir"] = "settingsdir \t directory for settings files\n";
    help_map["tengiga"] = "tengiga \t sets system to be configure for 10Gbe if "
                          "set to 1, else 1Gbe if set to 0\n";
}

template <typename T>
std::string CmdProxy<T>::Call(std::string command, VectorString arg, int detector_id) {
    // TODO! (Erik) investigate the effects of setReceiverOnline when not using
    // receiver. Seems ok...
    det.setOnline(true);
    det.setReceiverOnline(true);
    ReplaceIfDepreciated(command);

    // Store command args and detector id for further use
    // Probably also a good place to do sanity check
    command_ = command;
    arguments_ = arg;
    detector_id_ = detector_id;

    auto it = function_map.find(command);
    if (it != function_map.end()) {
        std::cout << ((*this).*(it->second))();
        return std::string();

    } else {
        std::cout << "Command \"" << command << "\" not found in proxy\n";
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
    auto d_it = depreciated_function_map.find(command);
    if (d_it != depreciated_function_map.end()) {
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
    for (auto it = function_map.begin(); it != function_map.end(); ++it) {
        std::cout << "\t" << it->first << std::endl;
    }
    std::cout << "\nDepreciaded commands: \n";
    for (auto it = depreciated_function_map.begin();
         it != depreciated_function_map.end(); ++it) {
        std::cout << "\t" << it->first << " -> " << it->second << std::endl;
    }
}

template <typename T> VectorString CmdProxy<T>::getAllCommands() {
    VectorString commands;
    for (auto const &v : function_map)
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

// template <typename T>
// std::string CmdProxy<T>::FileName()
// {
//     if (arguments_.size() == 1) {
//         det.setFileName(arguments_[0]);
//     } else if (arguments_.size() > 1) {
//         WrongNumberOfParameters(arguments_.size());
//     }
//     return resultToString(det.getFileName());
// }
// template <typename T>
// std::string CmdProxy<T>::FileWrite()
// {
//     if (arguments_.size() == 1) {
//         det.enableWriteToFile(std::stoi(arguments_[0]));
//     } else if (arguments_.size() > 1) {
//         WrongNumberOfParameters(arguments_.size());
//     }

//     return resultToString(det.enableWriteToFile());
// }
// template <typename T>
// std::string CmdProxy<T>::OverWrite()
// {
//     if (arguments_.size() == 1) {
//         det.overwriteFile(std::stoi(arguments_[0]));
//     } else if (arguments_.size() > 1) {
//         WrongNumberOfParameters(arguments_.size());
//     }
//     return resultToString(det.overwriteFile());
// }
// template <typename T>
// std::string CmdProxy<T>::Hostname()
// {
//     if (arguments_.size() == 1) {
//         //TODO! (Erik) adding logic
//         //currently only readds hostname
//     } else if (arguments_.size() > 1) {
//         WrongNumberOfParameters(arguments_.size());
//     }
//     return resultToString(det.getHostname(detector_id_));
// }
// template <typename T>
// std::string CmdProxy<T>::SettingsDir()
// {
//     if (arguments_.size() == 1) {
//         det.setSettingsDir(arguments_[0]);
//     } else if (arguments_.size() > 1) {
//         WrongNumberOfParameters(arguments_.size());
//     }
//     return resultToString(det.getSettingsDir());
// }

// template <typename T>
// std::string CmdProxy<T>::TenGiga()
// {
//     if (arguments_.size() == 1) {
//         det.tenGigabitEthernet(std::stoi(arguments_[0]), detector_id_);
//     } else if (arguments_.size() > 1) {
//         WrongNumberOfParameters(arguments_.size());
//     }
//     return resultToString(det.tenGigabitEthernet(-1, detector_id_));
// }
#endif // MULTI_DETECTOR_CALLER_H
