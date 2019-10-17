#include "CmdProxy.h"


#include "TimeHelper.h"
#include "ToString.h"
#include "logger.h"
#include "slsDetectorCommand.h"
#include "sls_detector_defs.h"


#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#define TIME_COMMAND(GETFCN, SETFCN, HLPSTR)                                   \
    std::ostringstream os;                                                     \
    os << cmd << ' ';                                                          \
    if (action == slsDetectorDefs::HELP_ACTION)                                \
        os << HLPSTR << '\n';                                                  \
    else if (action == slsDetectorDefs::GET_ACTION) {                          \
        auto t = det->GETFCN({det_id});                                        \
        if (args.size() == 0) {                                                \
            os << OutString(t) << '\n';                                        \
        } else if (args.size() == 1) {                                         \
            os << OutString(t, args[0]) << '\n';                               \
        } else {                                                               \
            WrongNumberOfParameters(2);                                        \
        }                                                                      \
    } else if (action == slsDetectorDefs::PUT_ACTION) {                        \
        if (args.size() == 1) {                                                \
            std::string time_str(args[0]);                                     \
            std::string unit = RemoveUnit(time_str);                           \
            auto t = StringTo<time::ns>(time_str, unit);                       \
            det->SETFCN(t, {det_id});                                          \
        } else if (args.size() == 2) {                                         \
            auto t = StringTo<time::ns>(args[0], args[1]);                     \
            det->SETFCN(t, {det_id});                                          \
        } else {                                                               \
            WrongNumberOfParameters(2);                                        \
        }                                                                      \
        os << args << '\n';                                                    \
    } else {                                                                   \
        throw sls::RuntimeError("Unknown action");                             \
    }                                                                          \
    return os.str();



namespace sls {

std::ostream &operator<<(std::ostream &os,
                         const std::vector<std::string> &vec) {
    if (!vec.empty()) {
        auto it = vec.begin();
        os << *it++;
        while (it != vec.end())
            os << ' ' << *it++;
    }
    return os;
}

std::string CmdProxy::Call(const std::string &command,
                           const std::vector<std::string> &arguments,
                           int detector_id, int action, std::ostream &os) {
    cmd = command;
    args = arguments;
    det_id = detector_id;

    ReplaceIfDepreciated(cmd);

    auto it = functions.find(cmd);
    if (it != functions.end()) {
        os << ((*this).*(it->second))(action);
        return {};
    } else {
        return cmd;
    }
}

bool CmdProxy::ReplaceIfDepreciated(std::string &command) {
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

std::vector<std::string> CmdProxy::GetAllCommands() {
    auto commands = slsDetectorCommand(nullptr).getAllCommands();
    for (const auto &it : functions)
        commands.emplace_back(it.first);
    std::sort(begin(commands), end(commands));
    return commands;
}

std::vector<std::string> CmdProxy::GetProxyCommands() {
    std::vector<std::string> commands;
    for (const auto &it : functions)
        commands.emplace_back(it.first);
    std::sort(begin(commands), end(commands));
    return commands;
}

void CmdProxy::WrongNumberOfParameters(size_t expected) {
    throw RuntimeError(
        "Command " + cmd + " expected <=" + std::to_string(expected) +
        " parameter/s but got " + std::to_string(args.size()) + "\n");
}



/************************************************
 *                                              *
 *            COMMANDS                          *
 *                                              *
 ************************************************/

std::string CmdProxy::Period(int action) {
    TIME_COMMAND(getPeriod, setPeriod,
                 "[duration] [(optional unit) ns|us|ms|s]\n\tSet the period");
}
std::string CmdProxy::Exptime(int action) {
    TIME_COMMAND(
        getExptime, setExptime,
        "[duration] [(optional unit) ns|us|ms|s]\n\tSet the exposure time");
}
std::string CmdProxy::SubExptime(int action) {
    TIME_COMMAND(getSubExptime, setSubExptime,
                 "[duration] [(optional unit) ns|us|ms|s]\n\tSet the "
                 "exposure time of EIGER subframes");
}




std::string CmdProxy::ListCommands(int action) {
    if (action == slsDetectorDefs::HELP_ACTION)
        return "list\n\tlists all available commands, list deprecated - "
               "list deprecated commands\n";

    if (args.size() == 0) {
        auto commands = slsDetectorCommand(nullptr).getAllCommands();
        for (const auto &it : functions)
            commands.emplace_back(it.first);
        std::sort(begin(commands), end(commands));

        std::cout << "These " << commands.size() << " commands are available\n";
        for (auto &c : commands)
            std::cout << c << '\n';
        return "";
    } else if (args.size() == 1) {
        if (args[0] == "deprecated") {
            std::cout << "The following " << depreciated_functions.size()
                      << " commands are deprecated\n";
            size_t field_width = 20;
            for (const auto &it : depreciated_functions) {
                std::cout << std::right << std::setw(field_width) << it.first
                          << " -> " << it.second << '\n';
            }
            return "";
        } else if (args[0] == "migrated") {
            std::cout << "The following " << functions.size()
                      << " commands have been migrated to the new API\n";
            for (const auto &it : functions) {
                std::cout << it.first << '\n';
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



std::string CmdProxy::ClockFrequency(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[n_clock (0-8)] [freq_in_Hz]\n\t[Gotthard2] Frequency of clock n_clock in Hz. Use clkdiv to set frequency." << '\n';   
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 1) {                                
            WrongNumberOfParameters(1);         
        } 
        auto t = det->getClockFrequency(std::stoi(args[0]), {det_id});       
        os << OutString(t) << '\n';     
    } else if (action == defs::PUT_ACTION) {     
        if (args.size() != 2) {
            WrongNumberOfParameters(2);  
        }                                
        det->setClockFrequency(std::stoi(args[0]), std::stoi(args[1]));  
        //TODO print args
        os << std::stoi(args[1]) << '\n';
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}


std::string CmdProxy::ClockPhase(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[n_clock (0-8)] [phase] [deg (optional)]\n\t[Gotthard2] Phase of clock n_clock. If deg, then phase shift in degrees, else absolute phase shift values." << '\n';   
    } else if (action == defs::GET_ACTION) {
        if (args.size() == 1) {                                
            auto t = det->getClockPhase(std::stoi(args[0]), {det_id});
            os << OutString(t) << '\n';       
        } else if (args.size() == 2) {    
            if (args[1] != "deg") {
                throw sls::RuntimeError("Cannot scan argument" + args[1] + ". Did you mean deg?");   
            }                            
            auto t = det->getClockPhaseinDegrees(std::stoi(args[0]), {det_id});
            os << OutString(t) << '\n';       
        } else {
            WrongNumberOfParameters(1);  
        }
    } else if (action == defs::PUT_ACTION) {
        if (args.size() == 2) {                                
            det->setClockPhase(std::stoi(args[0]), std::stoi(args[1]), {det_id});
            os << args[1] << '\n';       
        } else if (args.size() == 3) {    
            if (args[2] != "deg") {
                throw sls::RuntimeError("Cannot scan argument" + args[2] + ". Did you mean deg?");     
            }                            
            det->setClockPhaseinDegrees(std::stoi(args[0]), std::stoi(args[1]), {det_id});
            os << std::stoi(args[1]) << '\n';       
        } else {
            WrongNumberOfParameters(1);  
        }     
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::MaxClockPhaseShift(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[n_clock (0-8)]\n\t[Gotthard2] Absolute Maximum Phase shift of clock n_clock." << '\n';   
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);  
        }                    
        auto t = det->getMaxClockPhaseShift(std::stoi(args[0]), {det_id});
        os << OutString(t) << '\n';       
    } else if (action == defs::PUT_ACTION) {
        throw sls::RuntimeError("Cannot put");   
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::ClockDivider(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[n_clock (0-8)] [n_divider]\n\t[Gotthard2] Clock Divider of clock n_clock. Must be greater than 1." << '\n';   
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 1) {                                
            WrongNumberOfParameters(1);         
        } 
        auto t = det->getClockDivider(std::stoi(args[0]), {det_id});       
        os << OutString(t) << '\n';     
    } else if (action == defs::PUT_ACTION) {     
        if (args.size() != 2) {
            WrongNumberOfParameters(2);  
        }                         
        det->setClockDivider(std::stoi(args[0]), std::stoi(args[1]));  
        //TODO print args
        os << std::stoi(args[1]) << '\n';
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}
} // namespace sls