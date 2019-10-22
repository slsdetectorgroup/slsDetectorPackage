#include "CmdProxy.h"
#include "logger.h"
#include "slsDetectorCommand.h"
#include "sls_detector_defs.h"
#include "ToString.h"
#include "TimeHelper.h"
#include "container_utils.h"


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
        os << "[n_value] [(optional)deg]\n\t[Jungfrau][Ctb][Gotthard] Phase shift of ADC clock. \n\t[Jungfrau] Absolute phase shift. If deg used, then shift in degrees. Changing Speed also resets adcphase to recommended defaults.\n\t[Ctb] Absolute phase shift. If deg used, then shift in degrees. Changing adcclk also resets adcphase and sets it to previous values.\n\t[Gotthard] Relative phase shift" << '\n';   
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
        os << std::stoi(args[1]) << '\n';
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}



/* acquisition */
/* Network Configuration (Detector<->Receiver) */
/* Receiver Config */
/* File */
/* ZMQ Streaming Parameters (Receiver<->Client) */
/* Eiger Specific */

std::string CmdProxy::DynamicRange(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[4|8|16|32]\n\t[Eiger] Dynamic Range or number of bits per pixel in detector." << '\n';   
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 0) {                                
            WrongNumberOfParameters(0);         
        } 
        auto t = det->getDynamicRange({det_id});       
        os << OutString(t) << '\n';     
    } else if (action == defs::PUT_ACTION) {
        if (det_id != -1) { 
            throw sls::RuntimeError("Cannot execute dynamic range at module level");
        }        
        if (args.size() != 1) {
            WrongNumberOfParameters(1);  
        }                                
        det->setDynamicRange(std::stoi(args[0]));  
        os << args.front() << '\n';
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

std::string CmdProxy::GapPixels(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[0, 1]\n\t[Eiger] Include Gap pixels in data file or data call back. 4 bit mode gap pixels only ind ata call back." << '\n';   
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 0) {                                
            WrongNumberOfParameters(0);         
        } 
        auto t = det->getRxAddGapPixels({det_id});       
        os << OutString(t) << '\n';     
    } else if (action == defs::PUT_ACTION) {
        if (det_id != -1) { 
            throw sls::RuntimeError("Cannot execute dynamic range at module level");
        }        
        if (args.size() != 1) {
            WrongNumberOfParameters(1);  
        }                                
        det->setRxAddGapPixels(std::stoi(args[0]));  
        os << args.front() << '\n';
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::TrimEnergies(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[trim_ev1] [trim_Ev2 (optional)] [trim_ev3 (optional)] ...\n\t[Eiger] Number of trim energies and list of trim energies, where corresponding default trim files exist in corresponding trim folders." << '\n';   
    } else if (action == defs::GET_ACTION) {
         if (args.size() != 0) {                                
            WrongNumberOfParameters(0);         
        }       
        auto t = det->getTrimEnergies({det_id});
        os << OutString(t) << '\n';     
    } else if (action == defs::PUT_ACTION) {
        if (args.size() < 1) {                                
            WrongNumberOfParameters(1);         
        } 
        unsigned int ntrim = args.size();        
        std::vector<int> t(ntrim);
        for (unsigned int i = 0; i < ntrim; ++i) {
            t[i] = std::stoi(args[i]);
        }                  
        det->setTrimEnergies(t, {det_id}); 
        os << sls::ToString(args) << '\n';
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::RateCorrection(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[n_rate (in ns)]\n\t[Eiger] Dead time correction constant in ns. -1 will set to default tau of settings. 0 will unset rate correction." << '\n';   
    } else if (action == defs::GET_ACTION) {
         if (args.size() != 0) {                                
            WrongNumberOfParameters(0);         
        }       
        auto t = det->getRateCorrection({det_id});
        os << OutString(t) << '\n';     
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {                                
            WrongNumberOfParameters(1);         
        } 
        int tau = std::stoi(args[0]);
        if (tau == -1) {
            det->setDefaultRateCorrection({det_id});
            auto t = det->getRateCorrection({det_id});
            os << OutString(t) << '\n';        
        } else  {
            auto t = StringTo<time::ns>(args[0], "ns");
            det->setRateCorrection(t, {det_id});
            os << args.front() << "ns\n";
        }
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::Activate(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[0, 1] [(optional) padding|nopadding]\n\t[Eiger] 1 is default. 0 deactivates readout and does not send data. \n\tPadding will pad data files for deactivates readouts." << '\n';   
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 0) {
            WrongNumberOfParameters(0); 
        }
        auto t = det->getActive({det_id});
        auto p = det->getRxPadDeactivatedMode({det_id});  
        Result<std::string> pResult(p.size());
        for (unsigned int i = 0; i < p.size(); ++i) {
            pResult[i] = p[i] ? "padding" : "nopadding";
        }
        os << OutString(t) << ' ' << OutString(pResult) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() < 1 || args.size() > 2 ) {
            WrongNumberOfParameters(2); 
        }
        int t = std::stoi(args[0]);
        det->setActive(t, {det_id});
        os << args[0];
        if (args.size() == 2) {  
            bool p = true;
            if (args[1] == "nopadding") {
                p = false;
            } else if (args[1] != "padding") {
                throw sls::RuntimeError("Unknown argument for deactivated padding.");   
            }
            det->setRxPadDeactivatedMode(p, {det_id});
            os << ' ' << args[1];
        } 
        os << '\n';
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::PulsePixel(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[n_times] [x] [y]\n\t[Eiger] Pulse pixel n number of times at coordinates (x, y)." << '\n';   
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 3) {
            WrongNumberOfParameters(3); 
        }
        int n = std::stoi(args[0]);
        defs::xy c;
        c.x = std::stoi(args[1]);
        c.y = std::stoi(args[2]);       
        det->pulsePixel(n, c, {det_id});
        os << sls::ToString(args)  << '\n';
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::PulsePixelAndMove(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[n_times] [x] [y]\n\t[Eiger] Pulse pixel n number of times and moves relatively by (x, y)." << '\n';   
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 3) {
            WrongNumberOfParameters(3); 
        }
        int n = std::stoi(args[0]);
        defs::xy c;
        c.x = std::stoi(args[1]);
        c.y = std::stoi(args[2]);       
        det->pulsePixelNMove(n, c, {det_id});
        os << sls::ToString(args) << '\n';
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::PulseChip(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[n_times] \n\t[Eiger] Pulse chip n times. If n is -1, resets to normal mode (reset chip completely at start of acquisition, where partialreset = 0)." << '\n';   
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1); 
        }
        det->pulseChip(std::stoi(args[0]), {det_id});
        os << args.front() << '\n';
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::Quad(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[0, 1]\n\t[Eiger] 0 is default. 1 sets detector size to a quad (Specific hardware required)." << '\n';   
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 0) {                                
            WrongNumberOfParameters(0);         
        } 
        auto t = det->getQuad({det_id});       
        os << OutString(t) << '\n';     
    } else if (action == defs::PUT_ACTION) {
        if (det_id != -1) { 
            throw sls::RuntimeError("Cannot execute dynamic range at module level");
        }        
        if (args.size() != 1) {
            WrongNumberOfParameters(1);  
        }                                
        det->setQuad(std::stoi(args[0]));  
        os << args.front() << '\n';
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}


/* Jungfrau Specific */

std::string CmdProxy::TemperatureEvent(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[0]\n\t[Jungfrau] 1, if a temperature event occured. To clear this event, set it to 0.\n\tIf temperature crosses threshold temperature and temperature control is enabled, power to chip will be switched off and temperature event occurs. To power on chip again, temperature has to be less than threshold temperature and temperature event has to be cleared." << '\n';   
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 0) {                                
            WrongNumberOfParameters(0);         
        } 
        auto t = det->getTemperatureEvent({det_id});       
        os << OutString(t) << '\n';     
    } else if (action == defs::PUT_ACTION) {  
        if (args.size() != 1) {
            WrongNumberOfParameters(1);  
        }        
        if (std::stoi(args[0]) != 0) {
            throw sls::RuntimeError("Unknown argument for temp event. Did you mean 0 to reset event?");
        }                        
        det->resetTemperatureEvent();  
        os << "cleared" << '\n';
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}


/* Gotthard Specific */

std::string CmdProxy::ROI(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[xmin] [xmax] \n\t[Gotthard] Region of interest in detector. Either all channels or a single adc or 2 chips (256 channels). Default is all channels enabled (-1 -1). " << '\n';   
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 0) {                                
            WrongNumberOfParameters(0);         
        } 
        auto t = det->getROI({det_id});   
        for (auto &it : t) {    
            os << '[' << it.xmin << ", " << it.xmax << "] \n";     
        }
    } else if (action == defs::PUT_ACTION) {
        if (det_id == -1 && det->size() > 1) { 
            throw sls::RuntimeError("Cannot execute ROI at multi module level");
        }        
        if (args.size() != 2) {
            WrongNumberOfParameters(2);  
        }            
        defs::ROI t;
        t.xmin = std::stoi(args[0]);
        t.xmax = std::stoi(args[1]);
        det->setROI(t, det_id);  
        os << '[' << t.xmin << ", " << t.xmax << "] \n";  
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::ClearROI(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "\n\t[Gotthard] Resets Region of interest in detector. All channels enabled. Default is all channels." << '\n';   
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("Cannot get");   
    } else if (action == defs::PUT_ACTION) {    
        if (args.size() != 0) {
            WrongNumberOfParameters(0);  
        }                                 
        det->clearROI({det_id});  
        os << "[-1, -1] \n";
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

/* Gotthard2 Specific */
/* CTB Specific */

std::string CmdProxy::Samples(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[n_samples]\n\t[CTB] Number of samples (both analog and digitial) expected." << '\n';   
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 0) {                                
            WrongNumberOfParameters(0);         
        } 
        auto a = det->getNumberOfAnalogSamples({det_id});  
        auto d = det->getNumberOfDigitalSamples({det_id});   
        int as = a.squash(-1);
        int ds = d.squash(-1);
        if (as == -1 || ds == -1 || as != ds) { // check if a == d?
            throw sls::RuntimeError("Different samples. Use asamples or dsamples.");
        }  
        os << OutString(a) << '\n';     
    } else if (action == defs::PUT_ACTION) {     
        if (args.size() != 1) {
            WrongNumberOfParameters(1);  
        }                                
        det->setNumberOfAnalogSamples(std::stoi(args[0]));  
        det->setNumberOfDigitalSamples(std::stoi(args[0]));  
        os << args.front() << '\n';
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::Dbitphase(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[n_value] [(optional)deg]\n\t[Ctb] Phase shift of clock to latch digital bits. Absolute phase shift. If deg used, then shift in degrees. Changing dbitclk also resets dbitphase and sets to previous values." << '\n';   
    } else if (action == defs::GET_ACTION) {
        Result<int> t;
        if (args.size() == 0) {   
            t = det->getDBITPhase({det_id});  
            os << OutString(t) << '\n';  
        } else if (args.size() == 1) {                                
            if (args[0] != "deg") {
                throw sls::RuntimeError("Unknown dbitphase argument " + args[0] + ". Did you mean deg?"); 
            } 
            t = det->getDBITPhaseInDegrees({det_id});   
            os << OutString(t) << " deg\n";  
        } else {
            WrongNumberOfParameters(0);         
        }        
    } else if (action == defs::PUT_ACTION) {
        if (args.size() == 1) {                                
            det->setDBITPhase(std::stoi(args[0]), {det_id});  
            os << args.front() << '\n';
        } else if (args.size() == 2) {        
            if (args[1] != "deg") {
                throw sls::RuntimeError("Unknown dbitphase 2nd argument " + args[1] + ". Did you mean deg?"); 
            } 
            det->setDBITPhaseInDegrees(std::stoi(args[0]), {det_id});  
            os << args[0] << args[1] << '\n';
        } else {
           WrongNumberOfParameters(1);          
        }
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

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


std::string CmdProxy::ReceiverDbitList(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[all] or [i0] [i1] [i2]... \n\t[Ctb] List of digital signal bits read out. If all is used instead of a list, all digital bits (64) enabled. Each element in list can be 0 - 63 and non repetitive." << '\n';   
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 0) {                                
            WrongNumberOfParameters(0);         
        } 
        auto t = det->getRxDbitList({det_id});      
        os << OutString(t) << '\n';      
    } else if (action == defs::PUT_ACTION) {
        if (args.size() < 1) {                                
            WrongNumberOfParameters(1);         
        } 
        std::vector<int> t;
        if (args[0] == "all") {
           t.resize(64);
            for (unsigned int i = 0; i < 64; ++i) {
                t[i] = i;
            } 
        } else {
            unsigned int ntrim = args.size();        
            t.resize(ntrim);
            for (unsigned int i = 0; i < ntrim; ++i) {
                t[i] = std::stoi(args[i]);
            }    
        }              
        det->setRxDbitList(t, {det_id}); 
        os << sls::ToString(args) << '\n';
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::DigitalIODelay(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[64 bit bitmask] [0-775]\n\t[Ctb] Delay for digital IO pins selected by the bitmask. Delay is in ps and max of 775 ps. Resolution is 25 ps." << '\n';   
    } else if (action == defs::GET_ACTION) {
         throw sls::RuntimeError("Cannot get");     
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);  
        }                                
        det->setDigitalIODelay(std::stoul(args[0]), std::stoi(args[2]));  
        os << sls::ToString(args) << '\n';
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}


/* Pattern */

std::string CmdProxy::Pattern(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[fname]\n\t[Ctb] Loads binary pattern file with only pattern words" << '\n';   
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("Cannot get");     
    } else if (action == defs::PUT_ACTION) {     
        if (args.size() != 1) {
            WrongNumberOfParameters(1);  
        }                                
        det->setPattern(args[0]);  
        os << args.front() << '\n';
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::PatternWord(int action) {
    std::ostringstream os; 
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[step or address] [64 bit mask]\n\t[Ctb] 64 bit pattern at address of pattern memory." << '\n';   
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 1) {                                
            WrongNumberOfParameters(1);         
        } 
        auto t = det->getPatternWord(std::stoi(args[0]), {det_id});       
        os << OutStringHex(t) << '\n';     
    } else if (action == defs::PUT_ACTION) {      
        if (args.size() != 2) {
            WrongNumberOfParameters(2);  
        }                                
        det->setPatternWord(std::stoi(args[0]), std::stoul(args[1]));  
        os << sls::ToString(args) << '\n';
    } else { 
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}


} // namespace sls