#include "CmdProxy.h"
#include "logger.h"
#include "slsDetectorCommand.h"
#include "sls_detector_defs.h"
#include "ToString.h"
#include "TimeHelper.h"


#include <iostream>
#include <sstream>
#include <vector>
#include <iomanip>


namespace sls {

using defs = slsDetectorDefs;

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

std::string CmdProxy::ListCommands(int action) {
    if (action == defs::HELP_ACTION)
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

/* configuration */

std::string CmdProxy::Hostname(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "\n\tFrees shared memory and sets hostname (or IP address) of all modules concatenated by +." << '\n';   
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 0) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getHostname({det_id});
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() < 1) {
            WrongNumberOfParameters(1);
        } 
        if (det_id != -1) { 
            throw sls::RuntimeError("Cannot execute this at module level");
        }
        // only args[0], but many hostames concatenated with +
        if (args[0].find('+') != std::string::npos) {
            auto t = sls::split(args[0], '+');
            det->setHostname(t);
            os << ToString(t) << '\n';           
        }
        // either hostnames separated by space, or single hostname
        else {
            det->setHostname(args);
            os << ToString(args) << '\n';
        }
        auto t = det->getHostname({det_id});
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::FirmwareVersion(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "\n\tFimware version of detector in format [0xYYMMDD] or integer for Eiger." << '\n';   
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 0) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getFirmwareVersion({det_id});
        if (det->getDetectorType().squash() == defs::EIGER) {
            os << OutString(t) << '\n';
        } else {
            os << OutStringHex(t) << '\n';
        }
    } else if (action == defs::PUT_ACTION) {
        throw sls::RuntimeError("cannot put");
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::Versions(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "\n\tPrint all versions and detector type" << '\n';   
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 0) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getFirmwareVersion();
        os << "\nDetector Type: " << OutString(det->getDetectorType())
            << "\nPackage Version: " << det->getPackageVersion()
            << std::hex  
            << "\nClient Version: 0x" << det->getClientVersion();
            if (det->getDetectorType().squash() == defs::EIGER) {
                os << "\nFirmware Version: " << OutString(t);
            } else {
                os << "\nFirmware Version: " << OutStringHex(t);
            }
            os << "\nDetector Server Version: " << OutStringHex(det->getDetectorServerVersion());
        if (det->getUseReceiverFlag().squash(true)) {
            os << "\nReceiver Version: " << OutStringHex(det->getReceiverVersion());       
        }
        os << std::dec << '\n';
    } else if (action == defs::PUT_ACTION) {
        throw sls::RuntimeError("cannot put");
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::PackageVersion(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "\n\tPackage version (git branch)." << '\n';   
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 0) {
            WrongNumberOfParameters(0);
        }
         os << det->getPackageVersion() << '\n';
    } else if (action == defs::PUT_ACTION) {
        throw sls::RuntimeError("cannot put");
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::ClientVersion(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "\n\tClient software version in format [0xYYMMDD]." << '\n';   
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 0) {
            WrongNumberOfParameters(0);
        }
        os << ToStringHex(det->getClientVersion()) << '\n';
    } else if (action == defs::PUT_ACTION) {
        throw sls::RuntimeError("cannot put");
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::DetectorSize(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[nx] [ny]\n\tDetector size, ie. Number of channels in x and y dim. If 0, then hostname adds all modules in y dim. This is used to calculate module coordinates included in UDP data packet header." << '\n';   
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 0) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getDetectorSize();
        os << "[" << t.x << "," << t.y << "]\n";
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }        
        defs::xy t; 
        t.x = std::stoi(args[0]);
        t.y = std::stoi(args[1]);
        det->setDetectorSize(t);
        os << ToString(args) << '\n';
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}


/* acquisition parameters */

std::string CmdProxy::DelayLeft(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[(optional unit) ns|us|ms|s]\n\t[Gotthard][Jungfrau][CTB] DelayLeft Delay Left in Acquisition." << '\n';   
    } else if (action == defs::GET_ACTION) {
        auto t = det->getDelayAfterTriggerLeft({det_id});       
        if (args.size() == 0) {  
            os << OutString(t) << '\n';  
        } else if (args.size() == 1) { 
            os << OutString(t, args[0]) << '\n';
        } else {                                
            WrongNumberOfParameters(0);         
        }             
    } else if (action == defs::PUT_ACTION) {
        throw sls::RuntimeError("cannot put");
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::Speed(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[0 or full_speed|1 or half_speed|2 or quarter_speed]\n\t[Eiger][Jungfrau] Readout speed of chip.\n\tJungfrau also overwrites adcphase to recommended default. " << '\n';   
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 0) {                                
            WrongNumberOfParameters(0);         
        } 
        auto t = det->getSpeed({det_id});       
        os << OutString(t) << '\n';  
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {                                
            WrongNumberOfParameters(1);         
        }        
        defs::speedLevel t;
        try{
            int ival = std::stoi(args[0]);
            switch (ival) {
            case 0:
                t = defs::FULL_SPEED;
                break;
            case 1:
                t = defs::HALF_SPEED;
                break; 
            case 2:
                t = defs::QUARTER_SPEED;
                break;  
            default:
                throw sls::RuntimeError("Unknown speed " + args[0]);  
            }
        } catch (...) {
            t = sls::StringTo<defs::speedLevel>(args[0]);                                   
        }
        det->setSpeed(t, {det_id});    
        os << sls::ToString(t) << '\n'; // no args to convert 0,1,2 as well
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::Adcphase(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[n_value] [(optional)deg]\n\t[Jungfrau][Ctb][Gotthard] Phase shift of ADC clock. \n\t[Jungfrau] Absolute phase shift. If deg used, then shift in degrees. Changing Speed also resets adcphase to recommended defaults.\n\t[Ctb] Absolute phase shift. If deg used, then shift in degrees. Changing adcclk also resets adcphase.\n\t[Gotthard] Relative phase shift" << '\n';   
    } else if (action == defs::GET_ACTION) {
        Result<int> t;
        if (args.size() == 0) {   
            t = det->getADCPhase({det_id});  
            os << OutString(t) << '\n';  
        } else if (args.size() == 1) {                                
            if (args[0] != "deg") {
                throw sls::RuntimeError("Unknown adcphase argument " + args[0] + ". Did you mean deg?"); 
            } 
            t = det->getADCPhaseInDegrees({det_id});   
            os << OutString(t) << " deg\n";  
        } else {
            WrongNumberOfParameters(0);         
        }        
    } else if (action == defs::PUT_ACTION) {
        if (args.size() == 1) {                                
            det->setADCPhase(std::stoi(args[0]), {det_id});  
            os << args.front() << '\n';
        } else if (args.size() == 2) {        
            if (args[1] != "deg") {
                throw sls::RuntimeError("Unknown adcphase 2nd argument " + args[1] + ". Did you mean deg?"); 
            } 
            det->setADCPhaseInDegrees(std::stoi(args[0]), {det_id});  
            os << args[0] << args[1] << '\n';
        } else {
           WrongNumberOfParameters(1);          
        }
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}



/* acquisition */




















std::string CmdProxy::SlowAdc(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[n_channel (0-7 for channel|8 for temperature)]\n\t[Ctb] Slow ADC channel." << '\n';   
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 1) { 
            WrongNumberOfParameters(0); 
        }       
        int nchan = std::stoi(args[0]);
        if (nchan < 0 || nchan > defs::SLOW_ADC_TEMP - defs::SLOW_ADC0) {
            throw sls::RuntimeError("Unknown adc argument " + args[0]); 
        }
        if (nchan == 8) {
            auto t = det->getTemperature(defs::SLOW_ADC_TEMP, {det_id});  
            os << OutString(t) << " °C\n";
        } else {
            auto t = det->getSlowADC(static_cast<defs::dacIndex>(nchan + defs::SLOW_ADC0));
            os << OutString(t) << '\n';
        }      
    } else if (action == defs::PUT_ACTION) {
        throw sls::RuntimeError("cannot put");
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::Threshold(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[eV] [(optinal settings) standard, fast, highgain, dynamicgain, lowgain, mediumgain, veryhighgain, dynamichg0, fixgain1, fixgain2, forceswitchg1, forceswitchg2]\n\t[Eiger] Threshold in eV" << '\n';   
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 0) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getThresholdEnergy();
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {    
        if (args.size() == 1) {
            det->setThresholdEnergy(std::stoi(args[0]), slsDetectorDefs::GET_SETTINGS, true, {det_id});  
        } else if (args.size() == 2) {
            det->setThresholdEnergy(std::stoi(args[0]), sls::StringTo<slsDetectorDefs::detectorSettings>(args[1]), true, {det_id});
        } else {
            WrongNumberOfParameters(1);
        }  
        os << ToString(args) << '\n';
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::ThresholdNoTb(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[eV] [(optional settings) standard, fast, highgain, dynamicgain, lowgain, mediumgain, veryhighgain, dynamichg0, fixgain1, fixgain2, forceswitchg1, forceswitchg2]\n\t[Eiger] Threshold in eV set without setting trimbits" << '\n';   
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() == 1) {
            det->setThresholdEnergy(std::stoi(args[0]), slsDetectorDefs::GET_SETTINGS, false, {det_id});  
        } else if (args.size() == 2) {
            det->setThresholdEnergy(std::stoi(args[0]), sls::StringTo<slsDetectorDefs::detectorSettings>(args[1]), false, {det_id});
        } else {
            WrongNumberOfParameters(1);
        }
        os << ToString(args) << '\n';
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}




} // namespace sls