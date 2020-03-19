#include "CmdProxy.h"
#include "TimeHelper.h"
#include "ToString.h"
#include "container_utils.h"
#include "logger.h"
#include "sls_detector_defs.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

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

void CmdProxy::Call(const std::string &command,
                    const std::vector<std::string> &arguments, int detector_id,
                    int action, std::ostream &os) {
    cmd = command;
    args = arguments;
    det_id = detector_id;

    ReplaceIfDepreciated(cmd);

    auto it = functions.find(cmd);
    if (it != functions.end()) {
        os << ((*this).*(it->second))(action);
    } else {
        throw sls::RuntimeError(
            cmd + " Unknown command, use list to list all commands");
    }
}

bool CmdProxy::ReplaceIfDepreciated(std::string &command) {
    auto d_it = depreciated_functions.find(command);
    if (d_it != depreciated_functions.end()) {
        LOG(logWARNING)
            << command
            << " is depreciated and will be removed. Please migrate to: "
            << d_it->second;
        command = d_it->second;
        return true;
    }
    return false;
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

    if (args.empty()) {
        auto commands = GetProxyCommands();
        std::cout << "These " << commands.size() << " commands are available\n";
        for (auto &c : commands)
            std::cout << c << '\n';
        return "";
    } else if (args.size() == 1) {
        if (args[0] == "deprecated") {
            std::cout << "The following " << depreciated_functions.size()
                      << " commands are deprecated\n";
            const size_t field_width = 20;
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
        os << "\n\tFrees shared memory and sets hostname (or IP address) of "
              "all modules concatenated by +."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getHostname({det_id});
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.empty()) {
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

std::string CmdProxy::VirtualServer(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[n_servers] [starting_port_number]\n\tConnecs to n virtual "
              "server at local host starting at specific control port."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        if (det_id != -1) {
            throw sls::RuntimeError("Cannot execute this at module level");
        }
        det->setVirtualDetectorServers(StringTo<int>(args[0]), StringTo<int>(args[1]));
        os << sls::ToString(args);
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::acquire(int action) {
    std::ostringstream os;
    if (action == defs::HELP_ACTION) {
        os << cmd << " - Acquire the number of frames set up.\n";
    } else {
        if (det->empty()) {
            throw sls::RuntimeError(
                "This shared memory has no detectors added.");
        }
        if (det_id >= 0) {
            throw sls::RuntimeError(
                "Individual detectors not allowed for readout.");
        }

        det->acquire();

        if (det->getUseReceiverFlag().squash(false)) {
            os << "\nAcquired ";
            os << det->getFramesCaught() << '\n';
        }
    }
    return os.str();
}

std::string CmdProxy::free(int action) {
    // This  function is purely for help, actual functionality is in the caller
    return "\n\tFree detector shared memory\n";
}

std::string CmdProxy::FirmwareVersion(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "\n\tFimware version of detector in format [0xYYMMDD] or integer "
              "for Eiger."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
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
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getFirmwareVersion();
        os << "\nDetector Type: " << OutString(det->getDetectorType())
           << "\nPackage Version: " << det->getPackageVersion() << std::hex
           << "\nClient Version: 0x" << det->getClientVersion();
        if (det->getDetectorType().squash() == defs::EIGER) {
            os << "\nFirmware Version: " << OutString(t);
        } else {
            os << "\nFirmware Version: " << OutStringHex(t);
        }
        os << "\nDetector Server Version: "
           << OutStringHex(det->getDetectorServerVersion());
        if (det->getUseReceiverFlag().squash(true)) {
            os << "\nReceiver Version: "
               << OutStringHex(det->getReceiverVersion());
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
        if (!args.empty()) {
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
        if (!args.empty()) {
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
        os << "[nx] [ny]\n\tDetector size, ie. Number of channels in x and y "
              "dim. If 0, then hostname adds all modules in y dim. This is "
              "used to calculate module coordinates included in UDP data "
              "packet header."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getDetectorSize();
        os << "[" << t.x << "," << t.y << "]\n";
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        defs::xy t;
        t.x = StringTo<int>(args[0]);
        t.y = StringTo<int>(args[1]);
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
        os << "[0 or full_speed|1 or half_speed|2 or "
              "quarter_speed]\n\t[Eiger][Jungfrau] Readout speed of "
              "chip.\n\tJungfrau also overwrites adcphase to recommended "
              "default. "
           << '\n';
    } else {
        defs::detectorType type = det->getDetectorType().squash();
        if (type == defs::CHIPTESTBOARD || type == defs::MOENCH) {
            throw sls::RuntimeError(
                "Speed not implemented. Did you mean runclk?");
        }
        if (type != defs::EIGER && type != defs::JUNGFRAU) {
            throw sls::RuntimeError(
                "Speed not implemented."); // setspped one function problem. tbr
                                           // after change
        }
        if (action == defs::GET_ACTION) {
            if (!args.empty()) {
                WrongNumberOfParameters(0);
            }
            auto t = det->getSpeed({det_id});
            os << OutString(t) << '\n';
        } else if (action == defs::PUT_ACTION) {
            if (args.size() != 1) {
                WrongNumberOfParameters(1);
            }
            defs::speedLevel t;
            try {
                int ival = StringTo<int>(args[0]);
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
    }
    return os.str();
}

std::string CmdProxy::Adcphase(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[n_value] [(optional)deg]\n\t[Jungfrau][Ctb][Moench][Moench][Gotthard] Phase "
              "shift of ADC clock. \n\t[Jungfrau] Absolute phase shift. If deg "
              "used, then shift in degrees. Changing Speed also resets "
              "adcphase to recommended defaults.\n\t[Ctb][Moench] Absolute phase "
              "shift. If deg used, then shift in degrees. Changing adcclk also "
              "resets adcphase and sets it to previous values.\n\t[Gotthard] "
              "Relative phase shift"
           << '\n';
    } else if (action == defs::GET_ACTION) {
        Result<int> t;
        if (args.empty()) {
            t = det->getADCPhase({det_id});
            os << OutString(t) << '\n';
        } else if (args.size() == 1) {
            if (args[0] != "deg") {
                throw sls::RuntimeError("Unknown adcphase argument " + args[0] +
                                        ". Did you mean deg?");
            }
            t = det->getADCPhaseInDegrees({det_id});
            os << OutString(t) << " deg\n";
        } else {
            WrongNumberOfParameters(0);
        }
    } else if (action == defs::PUT_ACTION) {
        if (args.size() == 1) {
            det->setADCPhase(StringTo<int>(args[0]), {det_id});
            os << args.front() << '\n';
        } else if (args.size() == 2) {
            if (args[1] != "deg") {
                throw sls::RuntimeError("Unknown adcphase 2nd argument " +
                                        args[1] + ". Did you mean deg?");
            }
            det->setADCPhaseInDegrees(StringTo<int>(args[0]), {det_id});
            os << args[0] << args[1] << '\n';
        } else {
            WrongNumberOfParameters(1);
        }
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::Dbitphase(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[n_value] [(optional)deg]\n\t[Ctb][Jungfrau] Phase shift of clock to "
              "latch digital bits. Absolute phase shift. If deg used, then "
              "shift in degrees. \n\t[Ctb]Changing dbitclk also resets dbitphase and "
              "sets to previous values."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        Result<int> t;
        if (args.empty()) {
            t = det->getDBITPhase({det_id});
            os << OutString(t) << '\n';
        } else if (args.size() == 1) {
            if (args[0] != "deg") {
                throw sls::RuntimeError("Unknown dbitphase argument " +
                                        args[0] + ". Did you mean deg?");
            }
            t = det->getDBITPhaseInDegrees({det_id});
            os << OutString(t) << " deg\n";
        } else {
            WrongNumberOfParameters(0);
        }
    } else if (action == defs::PUT_ACTION) {
        if (args.size() == 1) {
            det->setDBITPhase(StringTo<int>(args[0]), {det_id});
            os << args.front() << '\n';
        } else if (args.size() == 2) {
            if (args[1] != "deg") {
                throw sls::RuntimeError("Unknown dbitphase 2nd argument " +
                                        args[1] + ". Did you mean deg?");
            }
            det->setDBITPhaseInDegrees(StringTo<int>(args[0]), {det_id});
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
        os << "[n_clock (0-8)] [freq_in_Hz]\n\t[Gotthard2][Mythen3] Frequency "
              "of clock n_clock in Hz. Use clkdiv to set frequency."
           << '\n';
    } else {
        defs::detectorType type = det->getDetectorType().squash(defs::GENERIC);
        if (type != defs::GOTTHARD2 && type != defs::MYTHEN3) {
            throw sls::RuntimeError("clkfreq not implemented for this detector.");
        }
        if (action == defs::GET_ACTION) {
            if (args.size() != 1) {
                WrongNumberOfParameters(1);
            }
            auto t = det->getClockFrequency(StringTo<int>(args[0]), {det_id});
            os << OutString(t) << '\n';
        } else if (action == defs::PUT_ACTION) {
            if (args.size() != 2) {
                WrongNumberOfParameters(2);
            }
            det->setClockFrequency(StringTo<int>(args[0]), StringTo<int>(args[1]),
                                   {det_id});
            os << StringTo<int>(args[1]) << '\n';
        } else {
            throw sls::RuntimeError("Unknown action");
        }
    }
    return os.str();
}

std::string CmdProxy::ClockPhase(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[n_clock (0-8)] [phase] [deg "
              "(optional)]\n\t[Gotthard2][Mythen3] Phase of clock n_clock. If "
              "deg, then phase shift in degrees, else absolute phase shift "
              "values."
           << '\n';
    } else {
        defs::detectorType type = det->getDetectorType().squash(defs::GENERIC);
        if (type != defs::GOTTHARD2 && type != defs::MYTHEN3) {
            throw sls::RuntimeError("clkphase not implemented for this detector.");
        }
        if (action == defs::GET_ACTION) {
            if (args.size() == 1) {
                auto t = det->getClockPhase(StringTo<int>(args[0]), {det_id});
                os << OutString(t) << '\n';
            } else if (args.size() == 2) {
                if (args[1] != "deg") {
                    throw sls::RuntimeError("Cannot scan argument" + args[1] +
                                            ". Did you mean deg?");
                }
                auto t =
                    det->getClockPhaseinDegrees(StringTo<int>(args[0]), {det_id});
                os << OutString(t) << '\n';
            } else {
                WrongNumberOfParameters(1);
            }
        } else if (action == defs::PUT_ACTION) {
            if (args.size() == 2) {
                det->setClockPhase(StringTo<int>(args[0]), StringTo<int>(args[1]),
                                   {det_id});
                os << args[1] << '\n';
            } else if (args.size() == 3) {
                if (args[2] != "deg") {
                    throw sls::RuntimeError("Cannot scan argument" + args[2] +
                                            ". Did you mean deg?");
                }
                det->setClockPhaseinDegrees(StringTo<int>(args[0]),
                                            StringTo<int>(args[1]), {det_id});
                os << args[1] << '\n';
            } else {
                WrongNumberOfParameters(1);
            }
        } else {
            throw sls::RuntimeError("Unknown action");
        }
    }
    return os.str();
}

std::string CmdProxy::MaxClockPhaseShift(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[n_clock (0-8)]\n\t[Gotthard2][Mythen3] Absolute Maximum Phase "
              "shift of clock n_clock."
           << '\n';
    } else {
        defs::detectorType type = det->getDetectorType().squash(defs::GENERIC);
        if (type != defs::GOTTHARD2 && type != defs::MYTHEN3) {
            throw sls::RuntimeError("maxclkphaseshift not implemented for this detector.");
        }
        if (action == defs::GET_ACTION) {
            if (args.size() != 1) {
                WrongNumberOfParameters(1);
            }
            auto t = det->getMaxClockPhaseShift(StringTo<int>(args[0]), {det_id});
            os << OutString(t) << '\n';
        } else if (action == defs::PUT_ACTION) {
            throw sls::RuntimeError("Cannot put");
        } else {
            throw sls::RuntimeError("Unknown action");
        }
    }
    return os.str();
}

std::string CmdProxy::ClockDivider(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[n_clock (0-8)] [n_divider]\n\t[Gotthard2][Mythen3] Clock "
              "Divider of clock n_clock. Must be greater than 1."
           << '\n';
    } else {
        defs::detectorType type = det->getDetectorType().squash(defs::GENERIC);
        if (type != defs::GOTTHARD2 && type != defs::MYTHEN3) {
            throw sls::RuntimeError("clkdiv not implemented for this detector.");
        }
        if (action == defs::GET_ACTION) {
            if (args.size() != 1) {
                WrongNumberOfParameters(1);
            }
            auto t = det->getClockDivider(StringTo<int>(args[0]), {det_id});
            os << OutString(t) << '\n';
        } else if (action == defs::PUT_ACTION) {
            if (args.size() != 2) {
                WrongNumberOfParameters(2);
            }
            det->setClockDivider(StringTo<int>(args[0]), StringTo<int>(args[1]),
                                 {det_id});
            os << args[1] << '\n';
        } else {
            throw sls::RuntimeError("Unknown action");
        }
    }
    return os.str();
}

/** temperature */
/* dacs */
std::string CmdProxy::Dac(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[dac index] [dac or mv value] [(optional unit) mv] "
              "\n\t[Ctb] Dac."
           << '\n';
    } else if (det->getDetectorType().squash(defs::GENERIC) != defs::CHIPTESTBOARD) {
        throw sls::RuntimeError("Dac command can only be used for chip test board. Use daclist to get list of dac commands for current detector.");
    } else if (action == defs::GET_ACTION) {
        bool mv = false;
        if (args.size() == 2) {
            if (args[1] != "mv") {
                throw sls::RuntimeError("Unknown argument " + args[1] +
                                        ". Did you mean mv?");
            }
            mv = true;
        } else if (args.size() > 2) {
            WrongNumberOfParameters(1);
        }
        auto t = det->getDAC(static_cast<defs::dacIndex>(StringTo<int>(args[0])),
                             mv, {det_id});
        os << args[0] << ' ' << OutString(t)
           << (args.size() > 1 ? " mv\n" : "\n");
    } else if (action == defs::PUT_ACTION) {
        bool mv = false;
        if (args.size() == 3) {
            if (args[2] != "mv") {
                throw sls::RuntimeError("Unknown argument " + args[2] +
                                        ". Did you mean mv?");
            }
            mv = true;
        } else if (args.size() > 3 || args.size() < 2) {
            WrongNumberOfParameters(2);
        }
        det->setDAC(static_cast<defs::dacIndex>(StringTo<int>(args[0])),
                    StringTo<int>(args[1]), mv, {det_id});
        os << args[0] << ' ' << args[1] << (args.size() > 2 ? " mv\n" : "\n");
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::DacList(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "\n\tGets the list of commands for every dac for this detector."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        os << sls::ToString(DacCommands()) << '\n';
    } else if (action == defs::PUT_ACTION) {
        throw sls::RuntimeError("Cannot put");
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::DacValues(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "\n\tGets the list of commands for every dac for this detector."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        std::vector<std::string> names = DacCommands();
        std::vector<std::string> res(names.size());
        std::vector<std::string> args;
        for (size_t i = 0; i < names.size(); ++i) {
            // for multiple values for each command (to use ToString on vector)
            std::ostringstream each;
            size_t spacepos = names[i].find(' ');
            // chip test board (dac)
            if (spacepos != std::string::npos) {
                if (args.empty()) {
                    args.resize(1);
                }
                args[0] = names[i].substr(spacepos + 1 - 1);
                names[i] = names[i].substr(0, spacepos);
            }
            Call(names[i], args, det_id, action, each);
            res[i] = each.str();
            res[i].pop_back(); // remove last \n character
        }
        os << sls::ToString(res) << '\n';
    } else if (action == defs::PUT_ACTION) {
        throw sls::RuntimeError("Cannot put");
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::vector<std::string> CmdProxy::DacCommands() {
    switch (det->getDetectorType().squash(defs::GENERIC)) {
    case defs::EIGER:
        return std::vector<std::string>{
            "vsvp",    "vtr",     "vrf",  "vrs",     "vsvn",      "vtgstv",
            "vcmp_ll", "vcmp_lr", "vcal", "vcmp_rl", "rxb_rb",    "rxb_lb",
            "vcmp_rr", "vcp",     "vcn",  "vis",     "vthreshold"};
        break;
    case defs::JUNGFRAU:
        return std::vector<std::string>{"vb_comp",    "vdd_prot",  "vin_com",
                                        "vref_prech", "vb_pixbuf", "vb_ds",
                                        "vref_ds",    "vref_comp"};
        break;
    case defs::GOTTHARD:
        return std::vector<std::string>{"vref_ds",   "vcascn_pb", "vcascp_pb",
                                        "vout_cm",   "vcasc_out", "vin_cm",
                                        "vref_comp", "ib_test_c"};
        break;
    case defs::GOTTHARD2:
        return std::vector<std::string>{
            "vref_h_adc",   "vb_comp_fe", "vb_comp_adc",  "vcom_cds",
            "vref_rstore", "vb_opa_1st", "vref_comp_fe", "vcom_adc1",
            "vref_prech",   "vref_l_adc", "vref_cds",     "vb_cs",
            "vb_opa_fd",    "vcom_adc2"};
        break;
    case defs::MYTHEN3:
        return std::vector<std::string>{
            "vcassh", "vth2",  "vshaper", "vshaperneg", "vipre_out", "vth3",
            "vth1",   "vicin", "vcas",    "vpreamp",    "vpl",       "vipre",
            "viinsh", "vph",   "vtrim",   "vdcsh"};
        break;
    case defs::MOENCH:
        return std::vector<std::string>{"vbp_colbuf", "vipre",  "vin_cm",
                                        "vb_sda", "vcasc_sfp", "vout_cm",
                                        "vipre_cds",    "ibias_sfp"};
        break;
    case defs::CHIPTESTBOARD:
        return std::vector<std::string>{
            "dac 0",  "dac 1",  "dac 2",  "dac 3",  "dac 4",  "dac 5",
            "dac 6",  "dac 7",  "dac 8",  "dac 9",  "dac 10", "dac 11",
            "dac 12", "dac 13", "dac 14", "dac 15", "dac 16", "dac 17"};
        break;
    default:
        throw sls::RuntimeError("Unknown detector type.");
    }
}

/* acquisition */
/* Network Configuration (Detector<->Receiver) */

std::string CmdProxy::UDPDestinationIP(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[x.x.x.x] or auto\n\tIp address of the receiver (destination) udp interface. If 'auto' used, then ip is set to ip of rx_hostname."
               << '\n';
    } else if (action == defs::GET_ACTION) {
        auto t = det->getDestinationUDPIP({det_id});
        if (args.size() != 0) {
            WrongNumberOfParameters(0);
        }
        os << OutString(t) << '\n';  
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        if (args[0] == "auto") {
            std::string rxHostname = det->getRxHostname({det_id}).squash("none");
            // Hostname could be ip try to decode otherwise look up the hostname
            auto val = sls::IpAddr{rxHostname};
            if (val == 0) {
                val = HostnameToIp(rxHostname.c_str());
            }
            LOG(logINFO) << "Setting udp_dstip of detector " << 
                det_id << " to " << val;
            det->setDestinationUDPIP(val, {det_id});
            os << val << '\n'; 
        } else {
            auto val = IpAddr(args[0]);
            det->setDestinationUDPIP(val, {det_id});
            os << args.front() << '\n'; 
        }
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::UDPDestinationIP2(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[x.x.x.x] or auto\n\t[Jungfrau] Ip address of the receiver (destination) udp interface where the second half of detector data is sent to. If 'auto' used, then ip is set to ip of rx_hostname."
               << '\n';
    } else if (action == defs::GET_ACTION) {
        auto t = det->getDestinationUDPIP2({det_id});
        if (args.size() != 0) {
            WrongNumberOfParameters(0);
        }
        os << OutString(t) << '\n';  
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        if (args[0] == "auto") {
            std::string rxHostname = det->getRxHostname({det_id}).squash("none");
            // Hostname could be ip try to decode otherwise look up the hostname
            auto val = sls::IpAddr{rxHostname};
            if (val == 0) {
                val = HostnameToIp(rxHostname.c_str());
            }
            LOG(logINFO) << "Setting udp_dstip2 of detector " << 
                det_id << " to " << val;
            det->setDestinationUDPIP2(val, {det_id});
            os << val << '\n'; 
        } else {
            auto val = IpAddr(args[0]);
            det->setDestinationUDPIP2(val, {det_id});
            os << args.front() << '\n'; 
        }
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

/* Receiver Config */
/* File */
/* ZMQ Streaming Parameters (Receiver<->Client) */
/* Eiger Specific */

std::string CmdProxy::DynamicRange(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[4|8|16|32]\n\t[Eiger] Dynamic Range or number of bits per "
              "pixel in detector."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getDynamicRange({det_id});
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (det_id != -1) {
            throw sls::RuntimeError(
                "Cannot execute dynamic range at module level");
        }
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        det->setDynamicRange(StringTo<int>(args[0]));
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
        os << "[eV] [(optinal settings) standard, lowgain, veryhighgain, verylowgain]"
        "\n\t[Eiger] Threshold in eV" << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getThresholdEnergy({det_id});
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() == 1) {
            det->setThresholdEnergy(StringTo<int>(args[0]),
                                    slsDetectorDefs::GET_SETTINGS, true,
                                    {det_id});
        } else if (args.size() == 2) {
            det->setThresholdEnergy(
                StringTo<int>(args[0]),
                sls::StringTo<slsDetectorDefs::detectorSettings>(args[1]), true,
                {det_id});
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
        os << "[eV] [(optional settings) standard, lowgain, veryhighgain, verylowgain]"
               "\n\t[Eiger] Threshold in eV set without setting trimbits"
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() == 1) {
            det->setThresholdEnergy(StringTo<int>(args[0]),
                                    slsDetectorDefs::GET_SETTINGS, false,
                                    {det_id});
        } else if (args.size() == 2) {
            det->setThresholdEnergy(
                StringTo<int>(args[0]),
                sls::StringTo<slsDetectorDefs::detectorSettings>(args[1]),
                false, {det_id});
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
        os << "[0, 1]\n\t[Eiger] Include Gap pixels in data file or data call "
              "back. 4 bit mode gap pixels only ind ata call back."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getRxAddGapPixels({det_id});
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (det_id != -1) {
            throw sls::RuntimeError(
                "Cannot execute dynamic range at module level");
        }
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        det->setRxAddGapPixels(StringTo<int>(args[0]));
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
        os << "[trim_ev1] [trim_Ev2 (optional)] [trim_ev3 (optional)] "
              "...\n\t[Eiger] Number of trim energies and list of trim "
              "energies, where corresponding default trim files exist in "
              "corresponding trim folders."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getTrimEnergies({det_id});
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.empty()) {
            WrongNumberOfParameters(1);
        }

        std::vector<int> t(args.size());
        for (size_t i = 0; i < t.size(); ++i) {
            t[i] = StringTo<int>(args[i]);
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
        os << "[n_rate (in ns)]\n\t[Eiger] Dead time correction constant in "
              "ns. -1 will set to default tau of settings. 0 will unset rate "
              "correction."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getRateCorrection({det_id});
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        int tau = StringTo<int>(args[0]);
        if (tau == -1) {
            det->setDefaultRateCorrection({det_id});
            auto t = det->getRateCorrection({det_id});
            os << OutString(t) << '\n';
        } else {
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
        os << "[0, 1] [(optional) padding|nopadding]\n\t[Eiger] 1 is default. "
              "0 deactivates readout and does not send data. \n\tPadding will "
              "pad data files for deactivates readouts."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
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
        if (args.empty() || args.size() > 2) {
            WrongNumberOfParameters(2);
        }
        int t = StringTo<int>(args[0]);
        det->setActive(t, {det_id});
        os << args[0];
        if (args.size() == 2) {
            bool p = true;
            if (args[1] == "nopadding") {
                p = false;
            } else if (args[1] != "padding") {
                throw sls::RuntimeError(
                    "Unknown argument for deactivated padding.");
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
        os << "[n_times] [x] [y]\n\t[Eiger] Pulse pixel n number of times at "
              "coordinates (x, y)."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 3) {
            WrongNumberOfParameters(3);
        }
        int n = StringTo<int>(args[0]);
        defs::xy c;
        c.x = StringTo<int>(args[1]);
        c.y = StringTo<int>(args[2]);
        det->pulsePixel(n, c, {det_id});
        os << sls::ToString(args) << '\n';
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::PulsePixelAndMove(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[n_times] [x] [y]\n\t[Eiger] Pulse pixel n number of times and "
              "moves relatively by (x, y)."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 3) {
            WrongNumberOfParameters(3);
        }
        int n = StringTo<int>(args[0]);
        defs::xy c;
        c.x = StringTo<int>(args[1]);
        c.y = StringTo<int>(args[2]);
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
        os << "[n_times] \n\t[Eiger] Pulse chip n times. If n is -1, resets to "
              "normal mode (reset chip completely at start of acquisition, "
              "where partialreset = 0)."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        det->pulseChip(StringTo<int>(args[0]), {det_id});
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
        os << "[0, 1]\n\t[Eiger] 0 is default. 1 sets detector size to a quad "
              "(Specific hardware required)."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getQuad({det_id});
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (det_id != -1) {
            throw sls::RuntimeError(
                "Cannot execute dynamic range at module level");
        }
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        det->setQuad(StringTo<int>(args[0]));
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
        os << "[0]\n\t[Jungfrau] 1, if a temperature event occured. To clear "
              "this event, set it to 0.\n\tIf temperature crosses threshold "
              "temperature and temperature control is enabled, power to chip "
              "will be switched off and temperature event occurs. To power on "
              "chip again, temperature has to be less than threshold "
              "temperature and temperature event has to be cleared."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getTemperatureEvent({det_id});
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        if (StringTo<int>(args[0]) != 0) {
            throw sls::RuntimeError("Unknown argument for temp event. Did you "
                                    "mean 0 to reset event?");
        }
        det->resetTemperatureEvent({det_id});
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
        os << "[xmin] [xmax] \n\t[Gotthard] Region of interest in detector. "
              "Either all channels or a single adc or 2 chips (256 channels). "
              "Default is all channels enabled (-1 -1). "
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
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
        t.xmin = StringTo<int>(args[0]);
        t.xmax = StringTo<int>(args[1]);
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
        os << "\n\t[Gotthard] Resets Region of interest in detector. All "
              "channels enabled. Default is all channels."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("Cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (!args.empty()) {
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

std::string CmdProxy::InjectChannel(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[offset] [increment]\n\t[Gotthard2] Inject channels with "
              "current source for calibration. Offset is starting channel that "
              "is injected, increment determines succeeding channels to be "
              "injected."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getInjectChannel({det_id});
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        det->setInjectChannel(StringTo<int>(args[0]), StringTo<int>(args[1]), {det_id});
        os << sls::ToString(args) << '\n';
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::VetoPhoton(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[n_chip] [#photons] [energy in keV] [reference "
              "file]\n\t[Gotthard2] Set veto reference for 128 channels for "
              "chip n_chip according to referenc file and #photons and energy "
              "in keV."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        auto t = det->getVetoPhoton(StringTo<int>(args[0]), {det_id});
        os << args[0] << ' ' << OutStringHex(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 4) {
            WrongNumberOfParameters(4);
        }
        det->setVetoPhoton(StringTo<int>(args[0]), StringTo<int>(args[1]),
                           StringTo<int>(args[2]), args[3], {det_id});
        os << sls::ToString(args) << '\n';
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::VetoReference(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[gain index] [12 bit value in hex] \n\t[Gotthard2] Set veto "
              "reference for all 128 channels for all chips."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("cannot get vetoref. Did you mean vetophoton?");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        det->setVetoReference(StringTo<int>(args[0]), StringTo<int>(args[1]), {det_id});
        os << sls::ToString(args) << '\n';
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::BurstMode(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[off or 0, internal or 1, external or 2]\n\t[Gotthard2] Default is burst internal type"
           << '\n';
    } else {
        if (action == defs::GET_ACTION) {
            if (!args.empty()) {
                WrongNumberOfParameters(0);
            }
            auto t = det->getBurstMode({det_id});
            os << OutString(t) << '\n';
        } else if (action == defs::PUT_ACTION) {
            if (args.size() != 1) {
                WrongNumberOfParameters(1);
            }
            defs::burstMode t;
            try {
                int ival = StringTo<int>(args[0]);
                switch (ival) {
                case 0:
                    t = defs::BURST_OFF;
                    break;
                case 1:
                    t = defs::BURST_INTERNAL;
                    break;
                case 2:
                    t = defs::BURST_EXTERNAL;
                    break;
                default:
                    throw sls::RuntimeError("Unknown burst mode " + args[0]);
                }
            } catch (...) {
                t = sls::StringTo<defs::burstMode>(args[0]);
            }
            det->setBurstMode(t, {det_id});
            os << sls::ToString(t) << '\n'; // no args to convert 0,1,2 as well
        } else {
            throw sls::RuntimeError("Unknown action");
        }
    }
    return os.str();
}

/* Mythen3 Specific */

std::string CmdProxy::Counters(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[i0] [i1] [i2]... \n\t[Mythen3] List of counters enabled. Each "
              "element in list can be 0 - 2 and must be non repetitive."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto mask = det->getCounterMask({det_id}).squash(-1);
        // scan counter enable mask to get vector
        std::vector <int> result;
        for (size_t i = 0; i < 32; ++i) {
            if (mask & (1 << i)) {
                result.push_back(static_cast<int>(i));
            }
        }
        os << sls::ToString(result) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.empty()) {
            WrongNumberOfParameters(1);
        }
        // convert vector to counter enable mask
        uint32_t mask = 0;
        for (size_t i = 0; i < args.size(); ++i) {
            int val = StringTo<int>(args[i]);
            // already enabled earlier
            if (mask & (1 << val)) {
                std::ostringstream oss;
                oss << "Duplicate counter values (" << val << ") in arguments";
                throw sls::RuntimeError(oss.str());
            }
            mask |= (1 << val);
        }
        det->setCounterMask(mask, {det_id});
        os << sls::ToString(args) << '\n';
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

/* CTB / Moench Specific */

std::string CmdProxy::Samples(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[n_samples]\n\t[CTB] Number of samples (both analog and "
              "digitial) expected.\n\t[Moench] Number of samples (analog only)"
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto a = det->getNumberOfAnalogSamples({det_id});
        // get also digital samples for ctb and compare with analog
        if (det->getDetectorType().squash() == defs::CHIPTESTBOARD) {
            auto d = det->getNumberOfDigitalSamples({det_id});
            int as = a.squash(-1);
            int ds = d.squash(-1);
            if (as == -1 || ds == -1 || as != ds) { // check if a == d?
                throw sls::RuntimeError(
                "Different samples. Use asamples or dsamples.");
            }            
        }
        os << OutString(a) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        det->setNumberOfAnalogSamples(StringTo<int>(args[0]), {det_id});
        // set also digital samples for ctb
        if (det->getDetectorType().squash() == defs::CHIPTESTBOARD) {
            det->setNumberOfDigitalSamples(StringTo<int>(args[0]), {det_id});
        }
        os << args.front() << '\n';
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

/* CTB Specific */

std::string CmdProxy::SlowAdc(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[n_channel (0-7 for channel|8 for temperature)]\n\t[Ctb] Slow "
              "ADC channel in mV or °C."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(0);
        }
        int nchan = StringTo<int>(args[0]);
        if (nchan < 0 || nchan > defs::SLOW_ADC_TEMP - defs::SLOW_ADC0) {
            throw sls::RuntimeError("Unknown adc argument " + args[0]);
        }
        if (nchan == 8) {
            auto t = det->getTemperature(defs::SLOW_ADC_TEMP, {det_id});
            os << OutString(t) << " °C\n";
        } else {
            auto t = det->getSlowADC(
                static_cast<defs::dacIndex>(nchan + defs::SLOW_ADC0), {det_id});
            Result<double> result(t.size());
            for (unsigned int i = 0; i < t.size(); ++i) {
                result[i] = t[i] / 1000.00;
            }    
            os << OutString(result) << " mV\n";
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
        os << "[all] or [i0] [i1] [i2]... \n\t[Ctb] List of digital signal "
              "bits read out. If all is used instead of a list, all digital "
              "bits (64) enabled. Each element in list can be 0 - 63 and must "
              "be non "
              "repetitive."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getRxDbitList({det_id});
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.empty()) {
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
                t[i] = StringTo<int>(args[i]);
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
        os << "[64 bit bitmask] [0-775]\n\t[Ctb] Delay for digital IO pins "
              "selected by the bitmask. Delay is in ps and max of 775 ps. "
              "Resolution is 25 ps."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("Cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        det->setDigitalIODelay(StringTo<uint64_t>(args[0]), StringTo<int>(args[1]), {det_id});
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
        os << "[fname]\n\t[Mythen3][Moench][Ctb][Moench] Loads binary pattern file with only pattern "
              "words"
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("Cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        det->setPattern(args[0], {det_id});
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
        os << "[step or address] [64 bit mask]\n\t[Ctb][Moench][Mythen3] 64 bit pattern at "
              "address of pattern memory."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        auto t = det->getPatternWord(StringTo<uint64_t>(args[0]), {det_id});
        os << OutStringHex(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        det->setPatternWord(StringTo<int>(args[0]), StringTo<uint64_t>(args[1]), {det_id});
        os << sls::ToString(args) << '\n';
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::PatternLoopAddresses(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        if (cmd == "patlimits") {
            os << "[start addr] [stop addr] \n\t[Ctb][Moench][Mythen3] Limits of complete "
                  "pattern."
               << '\n';
        } else if (cmd == "patloop0") {
            os << "[start addr] [stop addr] \n\t[Ctb][Moench][Mythen3] Limits of loop 0."
               << '\n';
        } else if (cmd == "patloop1") {
            os << "[start addr] [stop addr] \n\t[Ctb][Moench][Mythen3] Limits of loop 1."
               << '\n';
        } else if (cmd == "patloop2") {
            os << "[start addr] [stop addr] \n\t[Ctb][Moench][Mythen3] Limits of loop 2."
               << '\n';
        } else {
            throw sls::RuntimeError(
                "Unknown command, use list to list all commands");
        }
    } else {
        int level = -1;
        if (cmd == "patlimits") {
            level = -1;
        } else if (cmd == "patloop0") {
            level = 0;
        } else if (cmd == "patloop1") {
            level = 1;
        } else if (cmd == "patloop2") {
            level = 2;
        } else {
            throw sls::RuntimeError(
                "Unknown command, use list to list all commands");
        }
        if (action == defs::GET_ACTION) {
            if (!args.empty()) {
                WrongNumberOfParameters(0);
            }
            auto t = det->getPatternLoopAddresses(level, {det_id});
            os << OutStringHex(t) << '\n';
        } else if (action == defs::PUT_ACTION) {
            if (args.size() != 2) {
                WrongNumberOfParameters(2);
            }
            det->setPatternLoopAddresses(level, StringTo<int>(args[0]),
                                         StringTo<int>(args[1]), {det_id});
            os << sls::ToString(args) << '\n';
        } else {
            throw sls::RuntimeError("Unknown action");
        }
    }
    return os.str();
}

std::string CmdProxy::PatternLoopCycles(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        if (cmd == "patnloop0") {
            os << "[n_cycles] \n\t[Ctb][Moench][Mythen3] Number of cycles of loop 0." << '\n';
        } else if (cmd == "patnloop1") {
            os << "[n_cycles] \n\t[Ctb][Moench][Mythen3] Number of cycles of loop 1." << '\n';
        } else if (cmd == "patnloop2") {
            os << "[n_cycles] \n\t[Ctb][Moench][Mythen3] Number of cycles of loop 2." << '\n';
        } else {
            throw sls::RuntimeError(
                "Unknown command, use list to list all commands");
        }
    } else {
        int level = -1;
        if (cmd == "patnloop0") {
            level = 0;
        } else if (cmd == "patnloop1") {
            level = 1;
        } else if (cmd == "patnloop2") {
            level = 2;
        } else {
            throw sls::RuntimeError(
                "Unknown command, use list to list all commands");
        }
        if (action == defs::GET_ACTION) {
            if (!args.empty()) {
                WrongNumberOfParameters(0);
            }
            auto t = det->getPatternLoopCycles(level, {det_id});
            os << OutString(t) << '\n';
        } else if (action == defs::PUT_ACTION) {
            if (args.size() != 1) {
                WrongNumberOfParameters(1);
            }
            det->setPatternLoopCycles(level, StringTo<int>(args[0]), {det_id});
            os << args.front() << '\n';
        } else {
            throw sls::RuntimeError("Unknown action");
        }
    }
    return os.str();
}

std::string CmdProxy::PatternWaitAddress(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        if (cmd == "patwait0") {
            os << "[addr] \n\t[Ctb][Moench][Mythen3] Wait 0 address." << '\n';
        } else if (cmd == "patwait1") {
            os << "[addr] \n\t[Ctb][Moench][Mythen3] Wait 1 address." << '\n';
        } else if (cmd == "patwait2") {
            os << "[addr] \n\t[Ctb][Moench][Mythen3] Wait 2 address." << '\n';
        } else {
            throw sls::RuntimeError(
                "Unknown command, use list to list all commands");
        }
    } else {
        int level = -1;
        if (cmd == "patwait0") {
            level = 0;
        } else if (cmd == "patwait1") {
            level = 1;
        } else if (cmd == "patwait2") {
            level = 2;
        } else {
            throw sls::RuntimeError(
                "Unknown command, use list to list all commands");
        }
        if (action == defs::GET_ACTION) {
            if (!args.empty()) {
                WrongNumberOfParameters(0);
            }
            auto t = det->getPatternWaitAddr(level, {det_id});
            os << OutStringHex(t) << '\n';
        } else if (action == defs::PUT_ACTION) {
            if (args.size() != 1) {
                WrongNumberOfParameters(1);
            }
            det->setPatternWaitAddr(level, StringTo<int>(args[0]), {det_id});
            os << args.front() << '\n';
        } else {
            throw sls::RuntimeError("Unknown action");
        }
    }
    return os.str();
}

std::string CmdProxy::PatternWaitTime(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        if (cmd == "patwaittime0") {
            os << "[n_clk] \n\t[Ctb][Moench][Mythen3] Wait 0 time in clock cycles." << '\n';
        } else if (cmd == "patwaittime1") {
            os << "[n_clk] \n\t[Ctb][Moench][Mythen3] Wait 1 time in clock cycles." << '\n';
        } else if (cmd == "patwaittime2") {
            os << "[n_clk] \n\t[Ctb][Moench][Mythen3] Wait 2 time in clock cycles." << '\n';
        } else {
            throw sls::RuntimeError(
                "Unknown command, use list to list all commands");
        }
    } else {
        int level = -1;
        if (cmd == "patwaittime0") {
            level = 0;
        } else if (cmd == "patwaittime1") {
            level = 1;
        } else if (cmd == "patwaittime2") {
            level = 2;
        } else {
            throw sls::RuntimeError(
                "Unknown command, use list to list all commands");
        }
        if (action == defs::GET_ACTION) {
            if (!args.empty()) {
                WrongNumberOfParameters(0);
            }
            auto t = det->getPatternWaitTime(level, {det_id});
            os << OutString(t) << '\n';
        } else if (action == defs::PUT_ACTION) {
            if (args.size() != 1) {
                WrongNumberOfParameters(1);
            }
            det->setPatternWaitTime(level, std::stoul(args[0]), {det_id});
            os << args.front() << '\n';
        } else {
            throw sls::RuntimeError("Unknown action");
        }
    }
    return os.str();
}

/* Moench */

std::string CmdProxy::JsonParameter(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[key1] [value1]\n\tAdditional json header parameter streamed "
              "out from receiver. If empty in a get, then no parameter found. "
              "This is same as calling rx_jsonaddheader \"key\":\"value1\"."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        auto t = det->getAdditionalJsonParameter(args[0], {det_id});
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        det->setAdditionalJsonParameter(args[0], args[1], {det_id});
        os << sls::ToString(args) << '\n';
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::MinMaxEnergyThreshold(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        if (cmd == "emin") {
            os << "[n_value]\n\t[Moench] Minimum energy threshold (soft "
                  "setting) for processor."
               << '\n';
        } else if (cmd == "emax") {
            os << "[n_value]\n\t[Moench] Maximum energy threshold (soft "
                  "setting) for processor."
               << '\n';
        } else {
            throw sls::RuntimeError(
                "Unknown command, use list to list all commands");
        }
    } else {
        bool emax = false;
        if (cmd == "emin") {
            emax = false;
        } else if (cmd == "emax") {
            emax = true;
        } else {
            throw sls::RuntimeError(
                "Unknown command, use list to list all commands");
        }
        if (action == defs::GET_ACTION) {
            if (!args.empty()) {
                WrongNumberOfParameters(0);
            }
            auto t = det->getDetectorMinMaxEnergyThreshold(emax, {det_id});
            os << OutString(t) << '\n';
        } else if (action == defs::PUT_ACTION) {
            if (args.size() != 1) {
                WrongNumberOfParameters(1);
            }
            det->setDetectorMinMaxEnergyThreshold(emax, StringTo<int>(args[0]),
                                                  {det_id});
            os << args.front() << '\n';
        } else {
            throw sls::RuntimeError("Unknown action");
        }
    }
    return os.str();
}

/* Advanced */

std::string CmdProxy::ProgramFpga(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[fname.pof | fname.rbf]\n\t[Jungfrau][Ctb][Moench] Programs FPGA from pof file."
        << "\n\t[Mythen3][Gotthard2] Programs FPGA from rbf file."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("Cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        det->programFPGA(args[0], {det_id});
        os << "successful\n";
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::CopyDetectorServer(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[server_name] [pc_host_name]\n\t[Jungfrau][Ctb][Moench] Copies detector "
              "server via tftp from pc and changes respawn server name in "
              "/etc/inittab of detector."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("Cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        det->copyDetectorServer(args[0], args[1], {det_id});
        os << "successful\n";
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::UpdateFirmwareAndDetectorServer(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[server_name] [pc_host_name] [fname.pof]\n\t[Jungfrau][Ctb][Moench] "
              "Updates detector server via tftp from pc, updates firmware to "
              "pof file and then reboots controller (blackfin)."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("Cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 3) {
            WrongNumberOfParameters(3);
        }
        if (args[2].find(".pof") == std::string::npos) {
            throw sls::RuntimeError("Programming file must be a pof file.");
        }
        det->updateFirmwareAndServer(args[0], args[1], args[2], {det_id});
        os << "successful\n";
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::Register(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[address] [32 bit value]\n\tReads/writes to a 32 bit register "
              "in hex.\n\t[Eiger] +0x100 for only left, +0x200 for only right"
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        auto t = det->readRegister(StringTo<uint32_t>(args[0]), {det_id});
        os << OutStringHex(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        det->writeRegister(StringTo<uint32_t>(args[0]), StringTo<uint32_t>(args[1]), {det_id});
        os << sls::ToString(args) << '\n';
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::AdcRegister(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[address] [value]\n\t[Jungfrau][Ctb][Moench][Gotthard] Writes to an adc "
              "register in hex."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("Cannot get.");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        det->writeAdcRegister(StringTo<uint32_t>(args[0]), StringTo<uint32_t>(args[1]), {det_id});
        os << sls::ToString(args) << '\n';
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::BitOperations(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        if (cmd == "setbit") {
            os << "[address] [value\n\t[Moench] Minimum energy threshold (soft "
                  "setting) for processor."
               << '\n';
        } else if (cmd == "clearbit") {
            os << "[n_value]\n\t[Moench] Maximum energy threshold (soft "
                  "setting) for processor."
               << '\n';
        } else if (cmd == "getbit") {
            os << "[n_value]\n\t[Moench] Maximum energy threshold (soft "
                  "setting) for processor."
               << '\n';
        } else {
            throw sls::RuntimeError(
                "Unknown command, use list to list all commands");
        }
    } else {
        if (cmd != "setbit" && cmd != "clearbit" && cmd != "getbit") {
            throw sls::RuntimeError(
                "Unknown command, use list to list all commands");
        }
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        auto addr = StringTo<uint32_t>(args[0]);
        auto bitnr = StringTo<int>(args[1]);
        if (bitnr < 0 || bitnr > 31) {
            return std::string("Bit number out of range") +
                   std::to_string(bitnr);
        }
        if (action == defs::GET_ACTION) {
            if (cmd == "setbit" || cmd == "clearbit") {
                throw sls::RuntimeError("Cannot get");
            }
            auto t = det->readRegister(addr, {det_id});
            Result<int> result(t.size());
            for (unsigned int i = 0; i < t.size(); ++i) {
                result[i] = ((t[i] >> bitnr) & 0x1);
            }
            os << OutString(result) << '\n';
        } else if (action == defs::PUT_ACTION) {
            if (cmd == "getbit") {
                throw sls::RuntimeError("Cannot put");
            }
            if (cmd == "setbit") {
                det->setBit(addr, bitnr, {det_id});
            } else if (cmd == "clearbit") {
                det->clearBit(addr, bitnr, {det_id});
            }
            os << sls::ToString(args) << '\n';
        } else {
            throw sls::RuntimeError("Unknown action");
        }
    }
    return os.str();
}

std::string CmdProxy::InitialChecks(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[0, 1]\n\tEnable or disable intial compatibility and other checks at detector start up. It is enabled by default. Must come before 'hostname' command to take effect. Can be used to reprogram fpga when current firmware is incompatible."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (det_id != -1) {
            throw sls::RuntimeError(
                "Cannot enable/disable initial checks at module level");
        }
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getInitialChecks();
        os << t << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (det_id != -1) {
            throw sls::RuntimeError(
                "Cannot get initial checks enable at module level");
        }
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        det->setInitialChecks(StringTo<int>(args[0]));
        os << args.front() << '\n';
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}


/* Insignificant */

std::string CmdProxy::ExecuteCommand(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[command]\n\tExecutes command on detector server." << '\n';
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("Cannot get.");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        det->executeCommand(args[0], {det_id});
        os << "successful" << '\n';
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::UserDetails(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "\n\tUser details from shared memory." << '\n';
    } else if (action == defs::GET_ACTION) {
        if (det_id != -1) {
            throw sls::RuntimeError("Cannot execute this at module level");
        }
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getUserDetails();
        os << t << '\n';
    } else if (action == defs::PUT_ACTION) {
        throw sls::RuntimeError("Cannot put.");
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

} // namespace sls