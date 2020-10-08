#include "CmdProxy.h"
#include "HelpDacs.h"
#include "TimeHelper.h"
#include "ToString.h"
#include "bit_utils.h"
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

    std::string temp;
    while (temp != cmd) {
        temp = cmd;
        ReplaceIfDepreciated(cmd);
    }

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
        // insert old command into arguments (for dacs)
        if (d_it->second == "dac") {
            args.insert(args.begin(), command);
        }
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
              "all modules concatenated by +.\n\t Virtual servers can already "
              "use the port in hostname separated by ':' and ports incremented "
              "by 2 to accomodate the stop server as well."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getHostname(std::vector<int>{det_id});
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
              "server at local host starting at specific control port. Every "
              "virtual server will have a stop port (control port + 1)"
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
        det->setVirtualDetectorServers(StringTo<int>(args[0]),
                                       StringTo<int>(args[1]));
        os << sls::ToString(args);
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::Acquire(int action) {
    std::ostringstream os;
    if (action == defs::HELP_ACTION) {
        os << cmd
           << "\n\tAcquire the number of frames set up.\n\tBlocking command, "
              "where control server is blocked and cannot accept other "
              "commands until acquisition is done. \n\t- sets acquiring "
              "flag\n\t- starts the receiver listener (if enabled)\n\t- starts "
              "detector acquisition for number of frames set\n\t- monitors "
              "detector status from running to idle\n\t- stops the receiver "
              "listener (if enabled)\n\t- increments file index if file write "
              "enabled\n\t- resets acquiring flag";
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

std::string CmdProxy::Free(int action) {
    // This  function is purely for help, actual functionality is in the caller
    return "free\n\tFree detector shared memory\n";
}

std::string CmdProxy::FirmwareVersion(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "\n\tFimware version of detector in format [0xYYMMDD] or an "
              "increasing 2 digit number for Eiger."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getFirmwareVersion(std::vector<int>{det_id});
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
              "dim. This is used to calculate module coordinates included in "
              "UDP data. \n\tBy default, it adds module in y dimension for 2d "
              "detectors and in x dimension for 1d detectors packet header."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getDetectorSize();
        os << "[" << t.x << ", " << t.y << "]\n";
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

std::string CmdProxy::GapPixels(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[0, 1]\n\t[Eiger][Jungfrau] Include Gap pixels in client data "
              "call back in Detecor api. Will not be in detector streaming, "
              "receiver file or streaming. Default is 0. "
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (det_id != -1) {
            throw sls::RuntimeError("Cannot get gap pixels at module level");
        }
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getGapPixelsinCallback();
        os << t << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (det_id != -1) {
            throw sls::RuntimeError("Cannot add gap pixels at module level");
        }
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        det->setGapPixelsinCallback(StringTo<int>(args[0]));
        os << args.front() << '\n';
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

/* acquisition parameters */

std::string CmdProxy::Exptime(int action) {
    int gateIndex = -1;
    if (cmd == "exptime") {
        gateIndex = -1;
    } else if (cmd == "exptime1") {
        gateIndex = 0;
    } else if (cmd == "exptime2") {
        gateIndex = 1;
    } else if (cmd == "exptime3") {
        gateIndex = 2;
    } else {
        throw sls::RuntimeError(
            "Unknown command, use list to list all commands");
    }

    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        if (cmd == "exptime") {
            os << "[duration] [(optional unit) "
                  "ns|us|ms|s]\n\t[Eiger][Jungfrau][Gotthard][Gotthard2]["
                  "Moench][Ctb] Exposure time"
                  "\n\t[Mythen3] Exposure time of all gate signals in auto and "
                  "trigger mode (internal gating). To specify gate index, use "
                  "exptime1, exptime2, exptime3."
               << '\n';
        } else if (cmd == "exptime1") {
            os << "[n_value]\n\t[Mythen3] Exposure time of gate signal 1 in "
                  "auto and trigger mode (internal gating)."
               << '\n';
        } else if (cmd == "exptime2") {
            os << "[n_value]\n\t[Mythen3] Exposure time of gate signal 2 in "
                  "auto and trigger mode (internal gating)."
               << '\n';
        } else {
            os << "[n_value]\n\t[Mythen3] Exposure time of gate signal 3 in "
                  "auto and trigger mode (internal gating)."
               << '\n';
        }
    } else if (action == defs::GET_ACTION) {
        if (args.size() > 1) {
            WrongNumberOfParameters(1);
        }
        // vector of exptimes
        if (gateIndex == -1 &&
            det->getDetectorType().squash() == defs::MYTHEN3) {
            auto t = det->getExptimeForAllGates(std::vector<int>{det_id});
            if (args.empty()) {
                os << OutString(t) << '\n';
            } else if (args.size() == 1) {
                os << OutString(t, args[0]) << '\n';
            }
        }
        // single exptime
        else {
            Result<ns> t;
            if (gateIndex == -1) {
                t = det->getExptime(std::vector<int>{det_id});
            } else {
                t = det->getExptime(gateIndex, std::vector<int>{det_id});
            }
            if (args.empty()) {
                os << OutString(t) << '\n';
            } else if (args.size() == 1) {
                os << OutString(t, args[0]) << '\n';
            }
        }
    } else if (action == defs::PUT_ACTION) {
        defs::detectorType type = det->getDetectorType().squash();
        if (args.size() == 1) {
            std::string time_str(args[0]);
            std::string unit = RemoveUnit(time_str);
            auto t = StringTo<time::ns>(time_str, unit);
            if (type == defs::MYTHEN3) {
                det->setExptime(gateIndex, t, std::vector<int>{det_id});
            } else {
                det->setExptime(t, std::vector<int>{det_id});
            }
        } else if (args.size() == 2) {
            auto t = StringTo<time::ns>(args[0], args[1]);
            if (type == defs::MYTHEN3) {
                det->setExptime(gateIndex, t, std::vector<int>{det_id});
            } else {
                det->setExptime(t, std::vector<int>{det_id});
            }
        } else {
            WrongNumberOfParameters(2);
        }
        /* TODO: os << args << '\n'; (doesnt work for vectors in .h)*/
        if (args.size() > 1) {
            os << args[0] << args[1] << '\n';
        } else {
            os << args[0] << '\n';
        }
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::DynamicRange(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[value]\n\tDynamic Range or number of bits per "
              "pixel in detector.\n\t"
              "[Eiger] Options: 4, 8, 16, 32. If set to 32, also sets "
              "clkdivider to 2, else to 0.\n\t"
              "[Mythen3] Options: 8, 16, 32\n\t"
              "[Jungfrau][Gotthard][Ctb][Moench][Mythen3][Gotthard2] 16"
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getDynamicRange(std::vector<int>{det_id});
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

std::string CmdProxy::Speed(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[0 or full_speed|1 or half_speed|2 or "
              "quarter_speed]\n\t[Eiger][Jungfrau] Readout speed of "
              "chip.\n\t[Jungfrau] FULL_SPEED option only available from v2.0 "
              "boards and with setting number of interfaces to 2. Also "
              "overwrites adcphase to recommended default. "
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
            auto t = det->getSpeed(std::vector<int>{det_id});
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
            det->setSpeed(t, std::vector<int>{det_id});
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
        os << "[n_value] "
              "[(optional)deg]\n\t[Jungfrau][Ctb][Moench][Gotthard] "
              "Phase shift of ADC clock. \n\t[Jungfrau] Absolute phase shift. "
              "If deg used, then shift in degrees. Changing Speed also resets "
              "adcphase to recommended defaults.\n\t[Ctb][Moench] Absolute "
              "phase shift. If deg used, then shift in degrees. Changing "
              "adcclk also resets adcphase and sets it to previous "
              "values.\n\t[Gotthard] Relative phase shift. Cannot get"
           << '\n';
    } else {
        auto det_type = det->getDetectorType().squash(defs::GENERIC);
        if (det_type == defs::EIGER || det_type == defs::MYTHEN3 ||
            det_type == defs::GOTTHARD2) {
            throw sls::RuntimeError(
                "adcphase not implemented for this detector");
        }
        if (action == defs::GET_ACTION) {
            Result<int> t;
            if (args.empty()) {
                t = det->getADCPhase(std::vector<int>{det_id});
                os << OutString(t) << '\n';
            } else if (args.size() == 1) {
                if (args[0] != "deg") {
                    throw sls::RuntimeError("Unknown adcphase   argument " +
                                            args[0] +
                                            ". Did you mean deg?    ");
                }
                t = det->getADCPhaseInDegrees(std::vector<int>{det_id});
                os << OutString(t) << " deg\n";
            } else {
                WrongNumberOfParameters(0);
            }
        } else if (action == defs::PUT_ACTION) {
            if (args.size() == 1) {
                det->setADCPhase(StringTo<int>(args[0]),
                                 std::vector<int>{det_id});
                os << args.front() << '\n';
            } else if (args.size() == 2) {
                if (args[1] != "deg") {
                    throw sls::RuntimeError("Unknown adcphase   2nd argument " +
                                            args[1] + ". Did you    mean deg?");
                }
                det->setADCPhaseInDegrees(StringTo<int>(args[0]),
                                          std::vector<int>{det_id});
                os << args[0] << " " << args[1] << '\n';
            } else {
                WrongNumberOfParameters(1);
            }
        } else {
            throw sls::RuntimeError("Unknown action");
        }
    }
    return os.str();
}

std::string CmdProxy::Dbitphase(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[n_value] [(optional)deg]\n\t[Ctb][Jungfrau] Phase shift of "
              "clock to latch digital bits. Absolute phase shift. If deg used, "
              "then shift in degrees. \n\t[Ctb]Changing dbitclk also resets "
              "dbitphase and sets to previous values."
           << '\n';
    } else {
        auto det_type = det->getDetectorType().squash(defs::GENERIC);
        if (det_type == defs::EIGER || det_type == defs::MYTHEN3 ||
            det_type == defs::GOTTHARD2) {
            throw sls::RuntimeError(
                "dbitphase not implemented for this detector");
        }
        if (action == defs::GET_ACTION) {
            Result<int> t;
            if (args.empty()) {
                t = det->getDBITPhase(std::vector<int>{det_id});
                os << OutString(t) << '\n';
            } else if (args.size() == 1) {
                if (args[0] != "deg") {
                    throw sls::RuntimeError("Unknown dbitphase argument " +
                                            args[0] + ". Did you mean deg?  ");
                }
                t = det->getDBITPhaseInDegrees(std::vector<int>{det_id});
                os << OutString(t) << " deg\n";
            } else {
                WrongNumberOfParameters(0);
            }
        } else if (action == defs::PUT_ACTION) {
            if (args.size() == 1) {
                det->setDBITPhase(StringTo<int>(args[0]),
                                  std::vector<int>{det_id});
                os << args.front() << '\n';
            } else if (args.size() == 2) {
                if (args[1] != "deg") {
                    throw sls::RuntimeError("Unknown dbitphase 2nd  argument " +
                                            args[1] + ". Did you mean deg?  ");
                }
                det->setDBITPhaseInDegrees(StringTo<int>(args[0]),
                                           std::vector<int>{det_id});
                os << args[0] << " " << args[1] << '\n';
            } else {
                WrongNumberOfParameters(1);
            }
        } else {
            throw sls::RuntimeError("Unknown action");
        }
    }
    return os.str();
}

std::string CmdProxy::ClockFrequency(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[n_clock (0-5)] [freq_in_Hz]\n\t[Gotthard2][Mythen3] Frequency "
              "of clock n_clock in Hz. Use clkdiv to set frequency."
           << '\n';
    } else {
        defs::detectorType type = det->getDetectorType().squash(defs::GENERIC);
        if (type != defs::GOTTHARD2 && type != defs::MYTHEN3) {
            throw sls::RuntimeError(
                "clkfreq not implemented for this detector.");
        }
        if (action == defs::GET_ACTION) {
            if (args.size() != 1) {
                WrongNumberOfParameters(1);
            }
            auto t = det->getClockFrequency(StringTo<int>(args[0]),
                                            std::vector<int>{det_id});
            os << OutString(t) << '\n';
        } else if (action == defs::PUT_ACTION) {
            throw sls::RuntimeError("cannot put");
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
        os << "[n_clock (0-5)] [phase] [deg "
              "(optional)]\n\t[Gotthard2][Mythen3] Phase of clock n_clock. If "
              "deg, then phase shift in degrees, else absolute phase shift "
              "values."
           << '\n';
    } else {
        defs::detectorType type = det->getDetectorType().squash(defs::GENERIC);
        if (type != defs::GOTTHARD2 && type != defs::MYTHEN3) {
            throw sls::RuntimeError(
                "clkphase not implemented for this detector.");
        }
        if (action == defs::GET_ACTION) {
            if (args.size() == 1) {
                auto t = det->getClockPhase(StringTo<int>(args[0]),
                                            std::vector<int>{det_id});
                os << OutString(t) << '\n';
            } else if (args.size() == 2) {
                if (args[1] != "deg") {
                    throw sls::RuntimeError("Cannot scan argument" + args[1] +
                                            ". Did you mean deg?");
                }
                auto t = det->getClockPhaseinDegrees(StringTo<int>(args[0]),
                                                     {det_id});
                os << OutString(t) << " deg\n";
            } else {
                WrongNumberOfParameters(1);
            }
        } else if (action == defs::PUT_ACTION) {
            if (args.size() == 2) {
                det->setClockPhase(StringTo<int>(args[0]),
                                   StringTo<int>(args[1]),
                                   std::vector<int>{det_id});
                os << args[1] << '\n';
            } else if (args.size() == 3) {
                if (args[2] != "deg") {
                    throw sls::RuntimeError("Cannot scan argument" + args[2] +
                                            ". Did you mean deg?");
                }
                det->setClockPhaseinDegrees(StringTo<int>(args[0]),
                                            StringTo<int>(args[1]),
                                            std::vector<int>{det_id});
                os << args[1] << " " << args[2] << '\n';
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
        os << "[n_clock (0-5)]\n\t[Gotthard2][Mythen3] Absolute Maximum Phase "
              "shift of clock n_clock."
           << '\n';
    } else {
        defs::detectorType type = det->getDetectorType().squash(defs::GENERIC);
        if (type != defs::GOTTHARD2 && type != defs::MYTHEN3) {
            throw sls::RuntimeError(
                "maxclkphaseshift not implemented for this detector.");
        }
        if (action == defs::GET_ACTION) {
            if (args.size() != 1) {
                WrongNumberOfParameters(1);
            }
            auto t = det->getMaxClockPhaseShift(StringTo<int>(args[0]),
                                                std::vector<int>{det_id});
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
        os << "[n_clock (0-5)] [n_divider]\n\t[Gotthard2][Mythen3] Clock "
              "Divider of clock n_clock. Must be greater than 1."
           << '\n';
    } else {
        defs::detectorType type = det->getDetectorType().squash(defs::GENERIC);
        if (type != defs::GOTTHARD2 && type != defs::MYTHEN3) {
            throw sls::RuntimeError(
                "clkdiv not implemented for this detector.");
        }
        if (action == defs::GET_ACTION) {
            if (args.size() != 1) {
                WrongNumberOfParameters(1);
            }
            auto t = det->getClockDivider(StringTo<int>(args[0]),
                                          std::vector<int>{det_id});
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

std::string CmdProxy::ExternalSignal(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[n_signal] [signal_type] External signal mode for trigger "
              "timing mode."
              "\n\t[Gotthard] [0] "
              "[trigger_in_rising_edge|trigger_in_falling_edge]"
              "\n\t[Mythen3] [0-7] "
              "[trigger_in_rising_edge|trigger_in_falling_edge|inversion_on|"
              "inversion_off]\n\t where 0 is master input trigger signal, 1-3 "
              "is master input gate signals, 4 is busy out signal and 5-7 is "
              "master output gate signals."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        auto t = det->getExternalSignalFlags(StringTo<int>(args[0]),
                                             std::vector<int>{det_id});
        os << args[0] << " " << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        det->setExternalSignalFlags(
            StringTo<int>(args[0]),
            StringTo<slsDetectorDefs::externalSignalFlag>(args[1]),
            std::vector<int>{det_id});
        os << args[0] << " " << args[1] << '\n';
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

/** temperature */
std::string CmdProxy::TemperatureValues(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "\n\tGets the values for every temperature for this detector."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 0) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getTemperatureList();
        os << '[';
        if (t.size() > 0) {
            auto it = t.cbegin();
            os << ToString(*it) << ' ';
            os << OutString(
                      det->getTemperature(*it++, std::vector<int>{det_id}))
               << " °C";
            while (it != t.cend()) {
                os << ", " << ToString(*it) << ' ';
                os << OutString(
                          det->getTemperature(*it++, std::vector<int>{det_id}))
                   << " °C";
            }
        }
        os << "]\n";
    } else if (action == defs::PUT_ACTION) {
        throw sls::RuntimeError("Cannot put");
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

/* dacs */
std::string CmdProxy::Dac(int action) {
    std::ostringstream os;
    os << cmd << ' ';

    // dac indices only for ctb
    if (args.size() > 0 && action != defs::HELP_ACTION) {
        if (is_int(args[0]) &&
            det->getDetectorType().squash() != defs::CHIPTESTBOARD) {
            throw sls::RuntimeError(
                "Dac indices can only be used for chip test board. Use daclist "
                "to get list of dac names for current detector.");
        }
    }

    if (action == defs::HELP_ACTION) {
        if (args.size() == 0) {
            os << GetHelpDac(std::to_string(0)) << '\n';
        } else {
            os << args[0] << ' ' << GetHelpDac(args[0]) << '\n';
        }
    } else if (action == defs::GET_ACTION) {
        if (args.empty())
            WrongNumberOfParameters(1); // This prints slightly wrong

        defs::dacIndex dacIndex = StringTo<defs::dacIndex>(args[0]);
        bool mV = false;

        if (args.size() == 2) {
            if ((args[1] != "mv") && (args[1] != "mV")) {
                throw sls::RuntimeError("Unknown argument " + args[1] +
                                        ". Did you mean mV?");
            }
            mV = true;
        } else if (args.size() > 2) {
            WrongNumberOfParameters(1);
        }
        auto t = det->getDAC(dacIndex, mV, std::vector<int>{det_id});
        os << args[0] << ' ' << OutString(t) << (mV ? " mV\n" : "\n");
    } else if (action == defs::PUT_ACTION) {
        if (args.empty())
            WrongNumberOfParameters(1); // This prints slightly wrong

        defs::dacIndex dacIndex = StringTo<defs::dacIndex>(args[0]);
        bool mV = false;
        if (args.size() == 3) {
            if ((args[2] != "mv") && (args[2] != "mV")) {
                throw sls::RuntimeError("Unknown argument " + args[2] +
                                        ". Did you mean mV?");
            }
            mV = true;
        } else if (args.size() > 3 || args.size() < 2) {
            WrongNumberOfParameters(2);
        }
        det->setDAC(dacIndex, StringTo<int>(args[1]), mV,
                    std::vector<int>{det_id});
        os << args[0] << ' ' << args[1] << (mV ? " mV\n" : "\n");
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::DacValues(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[(optional unit) mV] \n\tGets the values for every "
              "dac for this detector."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        bool mv = false;
        if (args.size() == 1) {
            if ((args[0] != "mv") && (args[0] != "mV")) {
                throw sls::RuntimeError("Unknown argument " + args[0] +
                                        ". Did you mean mV?");
            }
            mv = true;
        } else if (args.size() > 1) {
            WrongNumberOfParameters(1);
        }
        auto t = det->getDacList();
        os << '[';
        auto it = t.cbegin();
        os << ToString(*it) << ' ';
        os << OutString(det->getDAC(*it++, mv, std::vector<int>{det_id}))
           << (!args.empty() ? " mV" : "");
        while (it != t.cend()) {
            os << ", " << ToString(*it) << ' ';
            os << OutString(det->getDAC(*it++, mv, std::vector<int>{det_id}))
               << (!args.empty() ? " mV" : "");
        }
        os << "]\n";
    } else if (action == defs::PUT_ACTION) {
        throw sls::RuntimeError("Cannot put");
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

/* acquisition */

std::string CmdProxy::ReceiverStatus(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "running, idle, transmitting]\n\tReceiver listener status."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getReceiverStatus(std::vector<int>{det_id});
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        throw sls::RuntimeError(
            "Cannot put. Did you mean to use command 'rx_start' or 'rx_stop'?");
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::DetectorStatus(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[running, error, transmitting, finished, waiting, "
              "idle]\n\tDetector status. Goes to stop server. "
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getDetectorStatus(std::vector<int>{det_id});
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        throw sls::RuntimeError(
            "Cannot put. Did you mean to use command 'start' or 'stop'?");
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::Scan(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[dac_name|0|trimbit_scan] [start_val] [stop_val] "
              "[step_size] [dac settling time ns|us|ms|s]\n\tEnables/ disables "
              "scans for dac and trimbits \n\tEnabling scan sets number of "
              "frames to number of steps in receiver. \n\tTo cancel scan "
              "configuration, set dac to '0', which also sets number of frames "
              "to 1. \n\t[Eiger][Mythen3] Use trimbit_scan as dac name for a "
              "trimbit scan."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 0) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getScan();
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (det_id != -1) {
            throw sls::RuntimeError("Cannot configure scan at module level");
        }
        // disable
        if (args.size() == 1) {
            if (StringTo<int>(args[0]) != 0) {
                throw sls::RuntimeError("Did you mean '0' to disable?");
            }
            det->setScan(defs::scanParameters());
        }
        // enable without settling time
        else if (args.size() == 4) {
            det->setScan(defs::scanParameters(
                StringTo<defs::dacIndex>(args[0]), StringTo<int>(args[1]),
                StringTo<int>(args[2]), StringTo<int>(args[3])));
        }
        // enable with all parameters
        else if (args.size() == 5) {
            std::string time_str(args[4]);
            std::string unit = RemoveUnit(time_str);
            auto t = StringTo<time::ns>(time_str, unit);
            det->setScan(defs::scanParameters(
                StringTo<defs::dacIndex>(args[0]), StringTo<int>(args[1]),
                StringTo<int>(args[2]), StringTo<int>(args[3]), t));
        } else {
            WrongNumberOfParameters(4);
        }
        os << ToString(args) << '\n';
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

/* Network Configuration (Detector<->Receiver) */

std::string CmdProxy::UDPDestinationIP(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[x.x.x.x] or auto\n\tIp address of the receiver (destination) "
              "udp interface. If 'auto' used, then ip is set to ip of "
              "rx_hostname."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        auto t = det->getDestinationUDPIP(std::vector<int>{det_id});
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        if (args[0] == "auto") {
            std::string rxHostname =
                det->getRxHostname(std::vector<int>{det_id}).squash("none");
            // Hostname could be ip try to decode otherwise look up the hostname
            auto val = sls::IpAddr{rxHostname};
            if (val == 0) {
                val = HostnameToIp(rxHostname.c_str());
            }
            LOG(logINFO) << "Setting udp_dstip of detector " << det_id << " to "
                         << val;
            det->setDestinationUDPIP(val, std::vector<int>{det_id});
            os << val << '\n';
        } else {
            auto val = IpAddr(args[0]);
            det->setDestinationUDPIP(val, std::vector<int>{det_id});
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
        os << "[x.x.x.x] or auto\n\t[Jungfrau][Gotthard2] Ip address of the "
              "receiver (destination) udp interface 2. If 'auto' used, then ip "
              "is set to ip of rx_hostname.\n\t[Jungfrau] bottom half "
              "\n\t[Gotthard2] veto debugging. "
           << '\n';
    } else if (action == defs::GET_ACTION) {
        auto t = det->getDestinationUDPIP2(std::vector<int>{det_id});
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        if (args[0] == "auto") {
            std::string rxHostname =
                det->getRxHostname(std::vector<int>{det_id}).squash("none");
            // Hostname could be ip try to decode otherwise look up the hostname
            auto val = sls::IpAddr{rxHostname};
            if (val == 0) {
                val = HostnameToIp(rxHostname.c_str());
            }
            LOG(logINFO) << "Setting udp_dstip2 of detector " << det_id
                         << " to " << val;
            det->setDestinationUDPIP2(val, std::vector<int>{det_id});
            os << val << '\n';
        } else {
            auto val = IpAddr(args[0]);
            det->setDestinationUDPIP2(val, std::vector<int>{det_id});
            os << args.front() << '\n';
        }
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

/* Receiver Config */
std::string CmdProxy::ReceiverHostname(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[hostname or ip address]\n\t"
              "[hostname or ip address]:[tcp port]\n\t"
              "[hostname1]:[tcp_port1]+[hostname2]:[tcp_port2]+\n\t"
              "Receiver hostname or IP. If port included, then the receiver "
              "tcp port.\n\t"
              "Used for TCP control communication between client and receiver "
              "to configure receiver. Also updates receiver with detector "
              "parameters. Also resets any prior receiver property (not on "
              "detector). "
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getRxHostname(std::vector<int>{det_id});
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() < 1) {
            WrongNumberOfParameters(1);
        }
        // multiple arguments
        if (args.size() > 1) {
            // multiple in mulitple
            if (args[0].find('+') != std::string::npos) {
                throw sls::RuntimeError(
                    "Cannot add multiple receivers at module level");
            }
            if (det_id != -1) {
                throw sls::RuntimeError(
                    "Cannot add multiple receivers at module level");
            }
            det->setRxHostname(args);
            os << ToString(args) << '\n';
        }
        // single argument
        else {
            // multiple receivers concatenated with +
            if (args[0].find('+') != std::string::npos) {
                if (det_id != -1) {
                    throw sls::RuntimeError(
                        "Cannot add multiple receivers at module level");
                }
                auto t = sls::split(args[0], '+');
                det->setRxHostname(t);
                os << ToString(t) << '\n';
            }
            // single receiver
            else {
                det->setRxHostname(args[0], std::vector<int>{det_id});
                os << ToString(args) << '\n';
            }
        }
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}
/* File */

/* ZMQ Streaming Parameters (Receiver<->Client) */

std::string CmdProxy::ZMQHWM(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[n_limit] \n\tClient's zmq receive high water mark. Default is "
              "the zmq library's default (1000), can also be set here using "
              "-1. \n This is a high number and can be set to 2 for gui "
              "purposes. \n One must also set the receiver's send high water "
              "mark to similar value. Final effect is sum of them.\n\t Setting "
              "it via command line is useful only before zmq enabled (before "
              "opening gui)."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getClientZmqHwm();
        os << t << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        int t = StringTo<int>(args[0]);
        det->setClientZmqHwm(t);
        os << det->getClientZmqHwm() << '\n';
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

/* Eiger Specific */

std::string CmdProxy::Threshold(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[eV] [(optinal settings) standard, lowgain, veryhighgain, "
              "verylowgain]"
              "\n\t[Eiger] Threshold in eV. It loads trim files from "
              "settingspath."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getThresholdEnergy(std::vector<int>{det_id});
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() == 1) {
            auto t = det->getSettings(std::vector<int>{det_id})
                         .tsquash("Inconsistent settings between detectors");
            det->setThresholdEnergy(StringTo<int>(args[0]), t, true,
                                    std::vector<int>{det_id});
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
        os << "[eV] [(optional settings) standard, lowgain, veryhighgain, "
              "verylowgain]"
              "\n\t[Eiger] Threshold in eV set without setting trimbits"
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() == 1) {
            auto t = det->getSettings(std::vector<int>{det_id})
                         .tsquash("Inconsistent settings between detectors");
            det->setThresholdEnergy(StringTo<int>(args[0]), t, false,
                                    std::vector<int>{det_id});
        } else if (args.size() == 2) {
            det->setThresholdEnergy(
                StringTo<int>(args[0]),
                sls::StringTo<slsDetectorDefs::detectorSettings>(args[1]),
                false, std::vector<int>{det_id});
        } else {
            WrongNumberOfParameters(1);
        }
        os << ToString(args) << '\n';
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
        auto t = det->getTrimEnergies(std::vector<int>{det_id});
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        std::vector<int> t(args.size());
        if (!args.empty()) {
            for (size_t i = 0; i < t.size(); ++i) {
                t[i] = StringTo<int>(args[i]);
            }
        }
        det->setTrimEnergies(t, std::vector<int>{det_id});
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
              "ns. -1 will set to default tau of settings from trimbit file. 0 "
              "will unset rate correction."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getRateCorrection(std::vector<int>{det_id});
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        int tau = StringTo<int>(args[0]);
        if (tau == -1) {
            det->setDefaultRateCorrection(std::vector<int>{det_id});
            auto t = det->getRateCorrection(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        } else {
            auto t = StringTo<time::ns>(args[0], "ns");
            det->setRateCorrection(t, std::vector<int>{det_id});
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
        auto t = det->getActive(std::vector<int>{det_id});
        auto p = det->getRxPadDeactivatedMode(std::vector<int>{det_id});
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
        det->setActive(t, std::vector<int>{det_id});
        os << args[0];
        if (args.size() == 2) {
            bool p = true;
            if (args[1] == "nopadding") {
                p = false;
            } else if (args[1] != "padding") {
                throw sls::RuntimeError(
                    "Unknown argument for deactivated padding.");
            }
            det->setRxPadDeactivatedMode(p, std::vector<int>{det_id});
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
              "coordinates (x, y). Advanced User!"
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
        det->pulsePixel(n, c, std::vector<int>{det_id});
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
              "moves relatively by (x, y). Advanced User!"
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
        det->pulsePixelNMove(n, c, std::vector<int>{det_id});
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
              "where partialreset = 0). Advanced User!"
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        det->pulseChip(StringTo<int>(args[0]), std::vector<int>{det_id});
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
        os << "[0, 1]\n\t[Eiger] Sets detector size to a quad. 0 (disabled) is "
              "default. (Specific hardware required)."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getQuad(std::vector<int>{det_id});
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (det_id != -1) {
            throw sls::RuntimeError("Cannot execute quad at module level");
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
        auto t = det->getTemperatureEvent(std::vector<int>{det_id});
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        if (StringTo<int>(args[0]) != 0) {
            throw sls::RuntimeError("Unknown argument for temp event. Did you "
                                    "mean 0 to reset event?");
        }
        det->resetTemperatureEvent(std::vector<int>{det_id});
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
        os << "[xmin] [xmax] \n\t[Gotthard] Region of interest in detector.\n\t"
              "Options: Only a single ROI per module. \n\tEither all channels "
              "or a single adc or 2 chips (256 channels). Default is all "
              "channels enabled (-1 -1). "
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getROI(std::vector<int>{det_id});
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
        defs::ROI t(StringTo<int>(args[0]), StringTo<int>(args[1]));
        det->setROI(t, det_id);
        os << '[' << t.xmin << ", " << t.xmax << "]\n";
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
              "channels enabled. Default is all channels enabled."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("Cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        det->clearROI(std::vector<int>{det_id});
        os << "[-1, -1]\n";
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
        auto t = det->getInjectChannel(std::vector<int>{det_id});
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        det->setInjectChannel(StringTo<int>(args[0]), StringTo<int>(args[1]),
                              {det_id});
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
        os << "[ichip] [#photons] [energy in keV] [reference "
              "file]\n\t[Gotthard2] Set veto reference for 128 channels for "
              "chip ichip according to reference file and #photons and energy "
              "in keV.\n[ichip] [output file]\n\t Get gain indices and veto "
              "reference for 128 channels for chip ichip, saved to file."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        det->getVetoPhoton(StringTo<int>(args[0]), args[1],
                           std::vector<int>{det_id});
        os << "saved to file " << args[1] << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 4) {
            WrongNumberOfParameters(4);
        }
        det->setVetoPhoton(StringTo<int>(args[0]), StringTo<int>(args[1]),
                           StringTo<int>(args[2]), args[3],
                           std::vector<int>{det_id});
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
        os << "[gain index] [12 bit value] \n\t[Gotthard2] Set veto "
              "reference for all 128 channels for all chips."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("cannot get vetoref. Did you mean vetophoton?");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        det->setVetoReference(StringTo<int>(args[0]), StringTo<int>(args[1]),
                              {det_id});
        os << sls::ToString(args) << '\n';
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::VetoFile(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[chip index 0-10, -1 for all] [file name] \n\t[Gotthard2] Set "
              "veto reference for each 128 channels for specific chip. The "
              "file should have 128 rows of gain index and 12 bit value in dec"
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError(
            "cannot get vetofile. Did you mean vetophoton?");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        det->setVetoFile(StringTo<int>(args[0]), args[1],
                         std::vector<int>{det_id});
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
        os << "[burst_internal or 0, burst_external or 1, cw_internal or 2, "
              "cw_external or 3]\n\t[Gotthard2] Default is burst_internal type"
           << '\n';
    } else {
        if (action == defs::GET_ACTION) {
            if (!args.empty()) {
                WrongNumberOfParameters(0);
            }
            auto t = det->getBurstMode(std::vector<int>{det_id});
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
                    t = defs::BURST_INTERNAL;
                    break;
                case 1:
                    t = defs::BURST_EXTERNAL;
                    break;
                case 2:
                    t = defs::CONTINUOUS_INTERNAL;
                    break;
                case 3:
                    t = defs::CONTINUOUS_EXTERNAL;
                    break;
                default:
                    throw sls::RuntimeError("Unknown burst mode " + args[0]);
                }
            } catch (...) {
                t = sls::StringTo<defs::burstMode>(args[0]);
            }
            det->setBurstMode(t, std::vector<int>{det_id});
            os << sls::ToString(t) << '\n'; // no args to convert 0,1,2 as well
        } else {
            throw sls::RuntimeError("Unknown action");
        }
    }
    return os.str();
}

std::string CmdProxy::ConfigureADC(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[chip index 0-10, -1 for all] [adc index 0-31, -1 for all] [12 "
              "bit configuration value in hex]\n\t[Gotthard2] Sets "
              "configuration for specific chip and adc, but configures 1 chip "
              "(all adcs for that chip) at a time."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        auto t = det->getADCConfiguration(StringTo<int>(args[0]),
                                          StringTo<int>(args[1]),
                                          std::vector<int>{det_id});
        os << OutStringHex(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 3) {
            WrongNumberOfParameters(3);
        }
        int value = StringTo<int>(args[2]);
        det->setADCConfiguration(StringTo<int>(args[0]), StringTo<int>(args[1]),
                                 value, std::vector<int>{det_id});
        os << '[' << args[0] << ", " << args[1] << ", " << ToStringHex(value)
           << "]\n";
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::BadChannels(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[fname]\n\t[Gotthard2] Sets the bad channels (from file of bad "
              "channel numbers) to be masked out."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        det->getBadChannels(args[0], std::vector<int>{det_id});
        os << "successfully retrieved" << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        det->setBadChannels(args[0], std::vector<int>{det_id});
        os << "successfully loaded" << '\n';
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

/* Mythen3 Specific */

std::string CmdProxy::Counters(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[i0] [i1] [i2]... \n\t[Mythen3] List of counters indices "
              "enabled. Each element in list can be 0 - 2 and must be non "
              "repetitive."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto mask = det->getCounterMask(std::vector<int>{det_id}).squash(-1);
        os << sls::ToString(getSetBits(mask)) << '\n';
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
        det->setCounterMask(mask, std::vector<int>{det_id});
        os << sls::ToString(args) << '\n';
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::GateDelay(int action) {
    int gateIndex = -1;
    if (cmd == "gatedelay") {
        gateIndex = -1;
    } else if (cmd == "gatedelay1") {
        gateIndex = 0;
    } else if (cmd == "gatedelay2") {
        gateIndex = 1;
    } else if (cmd == "gatedelay3") {
        gateIndex = 2;
    } else {
        throw sls::RuntimeError(
            "Unknown command, use list to list all commands");
    }

    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        if (cmd == "gatedelay") {
            os << "[duration] [(optional unit) "
                  "ns|us|ms|s]\n\t[Mythen3] Gate Delay of all gate signals in "
                  "auto and trigger mode (internal gating)."
               << '\n';
        } else if (cmd == "gatedelay1") {
            os << "[n_value]\n\t[Mythen3] Gate Delay of gate signal 1 in auto "
                  "and trigger mode (internal gating)."
               << '\n';
        } else if (cmd == "gatedelay2") {
            os << "[n_value]\n\t[Mythen3] Gate Delay of gate signal 2 in auto "
                  "and trigger mode (internal gating)."
               << '\n';
        } else {
            os << "[n_value]\n\t[Mythen3] Gate Delay of gate signal 3 in auto "
                  "and trigger mode (internal gating)."
               << '\n';
        }
    } else if (action == defs::GET_ACTION) {
        if (args.size() > 1) {
            WrongNumberOfParameters(1);
        }
        // vector of gate delays
        if (gateIndex == -1) {
            auto t = det->getGateDelayForAllGates(std::vector<int>{det_id});
            if (args.empty()) {
                os << OutString(t) << '\n';
            } else if (args.size() == 1) {
                os << OutString(t, args[0]) << '\n';
            }
        }
        // single gate delay
        else {
            auto t = det->getGateDelay(gateIndex, std::vector<int>{det_id});
            if (args.empty()) {
                os << OutString(t) << '\n';
            } else if (args.size() == 1) {
                os << OutString(t, args[0]) << '\n';
            }
        }
    } else if (action == defs::PUT_ACTION) {
        if (args.size() == 1) {
            std::string time_str(args[0]);
            std::string unit = RemoveUnit(time_str);
            auto t = StringTo<time::ns>(time_str, unit);
            det->setGateDelay(gateIndex, t, std::vector<int>{det_id});
        } else if (args.size() == 2) {
            auto t = StringTo<time::ns>(args[0], args[1]);
            det->setGateDelay(gateIndex, t, std::vector<int>{det_id});
        } else {
            WrongNumberOfParameters(2);
        }
        /* TODO: os << args << '\n'; (doesnt work for vectors in .h)*/
        if (args.size() > 1) {
            os << args[0] << args[1] << '\n';
        } else {
            os << args[0] << '\n';
        }
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
        auto a = det->getNumberOfAnalogSamples(std::vector<int>{det_id});
        // get also digital samples for ctb and compare with analog
        if (det->getDetectorType().squash() == defs::CHIPTESTBOARD) {
            auto d = det->getNumberOfDigitalSamples(std::vector<int>{det_id});
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
        det->setNumberOfAnalogSamples(StringTo<int>(args[0]),
                                      std::vector<int>{det_id});
        // set also digital samples for ctb
        if (det->getDetectorType().squash() == defs::CHIPTESTBOARD) {
            det->setNumberOfDigitalSamples(StringTo<int>(args[0]),
                                           std::vector<int>{det_id});
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
        os << "[n_channel (0-7 for channel]\n\t[Ctb] Slow "
              "ADC channel in uV"
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(0);
        }
        int nchan = StringTo<int>(args[0]);
        if (nchan < 0 || nchan > 7) {
            throw sls::RuntimeError("Unknown adc argument " + args[0]);
        }
        auto t = det->getSlowADC(
            static_cast<defs::dacIndex>(nchan + defs::SLOW_ADC0),
            std::vector<int>{det_id});
        Result<double> result(t.size());
        for (unsigned int i = 0; i < t.size(); ++i) {
            result[i] = t[i] / 1000.00;
        }
        os << OutString(result) << " mV\n";

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
              "be non repetitive."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getRxDbitList(std::vector<int>{det_id});
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
        det->setRxDbitList(t, std::vector<int>{det_id});
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
        det->setDigitalIODelay(StringTo<uint64_t>(args[0]),
                               StringTo<int>(args[1]),
                               std::vector<int>{det_id});
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
        os << "[fname]\n\t[Mythen3][Moench][Ctb] Loads ASCII pattern file "
              "directly to server (instead of executing line by line)"
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("Cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        det->setPattern(args[0], std::vector<int>{det_id});
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
        os << "[step or address] [64 bit mask]\n\t[Ctb][Moench][Mythen3] 64 "
              "bit pattern at address of pattern memory.\n\t[Ctb][Moench] read "
              "is same as executing pattern"
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        int addr = StringTo<int>(args[0]);
        auto t = det->getPatternWord(addr, std::vector<int>{det_id});
        os << '[' << ToStringHex(addr, 4) << ", " << OutStringHex(t, 16)
           << "]\n";
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        int addr = StringTo<int>(args[0]);
        uint64_t word = StringTo<uint64_t>(args[1]);
        det->setPatternWord(addr, word, std::vector<int>{det_id});
        os << '[' << ToStringHex(addr, 4) << ", " << ToStringHex(word, 16)
           << "]\n";
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
            os << "[start addr] [stop addr] \n\t[Ctb][Moench][Mythen3] Limits "
                  "of complete pattern."
               << '\n';
        } else if (cmd == "patloop0") {
            os << "[start addr] [stop addr] \n\t[Ctb][Moench][Mythen3] Limits "
                  "of loop 0."
               << '\n';
        } else if (cmd == "patloop1") {
            os << "[start addr] [stop addr] \n\t[Ctb][Moench][Mythen3] Limits "
                  "of loop 1."
               << '\n';
        } else if (cmd == "patloop2") {
            os << "[start addr] [stop addr] \n\t[Ctb][Moench][Mythen3] Limits "
                  "of loop 2."
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
            auto t =
                det->getPatternLoopAddresses(level, std::vector<int>{det_id});
            os << OutStringHex(t, 4) << '\n';
        } else if (action == defs::PUT_ACTION) {
            if (args.size() != 2) {
                WrongNumberOfParameters(2);
            }
            int start = StringTo<int>(args[0]);
            int stop = StringTo<int>(args[1]);
            det->setPatternLoopAddresses(level, start, stop,
                                         std::vector<int>{det_id});
            os << '[' << ToStringHex(start, 4) << ", " << ToStringHex(stop, 4)
               << "]\n";
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
            os << "[n_cycles] \n\t[Ctb][Moench][Mythen3] Number of cycles of "
                  "loop 0."
               << '\n';
        } else if (cmd == "patnloop1") {
            os << "[n_cycles] \n\t[Ctb][Moench][Mythen3] Number of cycles of "
                  "loop 1."
               << '\n';
        } else if (cmd == "patnloop2") {
            os << "[n_cycles] \n\t[Ctb][Moench][Mythen3] Number of cycles of "
                  "loop 2."
               << '\n';
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
            auto t = det->getPatternLoopCycles(level, std::vector<int>{det_id});
            os << OutString(t) << '\n';
        } else if (action == defs::PUT_ACTION) {
            if (args.size() != 1) {
                WrongNumberOfParameters(1);
            }
            det->setPatternLoopCycles(level, StringTo<int>(args[0]),
                                      std::vector<int>{det_id});
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
            auto t = det->getPatternWaitAddr(level, std::vector<int>{det_id});
            os << OutStringHex(t, 4) << '\n';
        } else if (action == defs::PUT_ACTION) {
            if (args.size() != 1) {
                WrongNumberOfParameters(1);
            }
            int addr = StringTo<int>(args[0]);
            det->setPatternWaitAddr(level, addr, std::vector<int>{det_id});
            os << ToStringHex(addr, 4) << '\n';
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
            os << "[n_clk] \n\t[Ctb][Moench][Mythen3] Wait 0 time in clock "
                  "cycles."
               << '\n';
        } else if (cmd == "patwaittime1") {
            os << "[n_clk] \n\t[Ctb][Moench][Mythen3] Wait 1 time in clock "
                  "cycles."
               << '\n';
        } else if (cmd == "patwaittime2") {
            os << "[n_clk] \n\t[Ctb][Moench][Mythen3] Wait 2 time in clock "
                  "cycles."
               << '\n';
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
            auto t = det->getPatternWaitTime(level, std::vector<int>{det_id});
            os << OutString(t) << '\n';
        } else if (action == defs::PUT_ACTION) {
            if (args.size() != 1) {
                WrongNumberOfParameters(1);
            }
            det->setPatternWaitTime(level, StringTo<uint64_t>(args[0]),
                                    {det_id});
            os << args.front() << '\n';
        } else {
            throw sls::RuntimeError("Unknown action");
        }
    }
    return os.str();
}

/* Moench */

std::string CmdProxy::AdditionalJsonHeader(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[key1] [value1] [key2] [value2]...[keyn] [valuen]"
              "\n\tAdditional json header to be streamed out from receiver via "
              "zmq. Default is empty. Max 20 characters for each key/value. "
              "Use only if to be processed by an intermediate user process "
              "listening to receiver zmq packets. Empty value deletes header. "
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getAdditionalJsonHeader(std::vector<int>{det_id});
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        // arguments can be empty
        std::map<std::string, std::string> json;
        for (size_t i = 0; i < args.size(); i = i + 2) {
            // last value is empty
            if (i + 1 >= args.size()) {
                json[args[i]] = "";
            } else {
                json[args[i]] = args[i + 1];
            }
        }
        det->setAdditionalJsonHeader(json, std::vector<int>{det_id});
        os << sls::ToString(json) << '\n';
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::JsonParameter(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[key1] [value1]\n\tAdditional json header parameter streamed "
              "out from receiver. If not found in header, the pair is "
              "appended. An empty values deletes parameter. Max 20 characters "
              "for each key/value."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        auto t =
            det->getAdditionalJsonParameter(args[0], std::vector<int>{det_id});
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        switch (args.size()) {
        case 1:
            det->setAdditionalJsonParameter(args[0], "",
                                            std::vector<int>{det_id});
            break;
        case 2:
            det->setAdditionalJsonParameter(args[0], args[1],
                                            std::vector<int>{det_id});
            break;
        default:
            WrongNumberOfParameters(1);
        }
        if (args.size() == 1) {
            os << args[0] << " deleted" << '\n';
        } else {
            os << "{" << args[0] << ": " << args[1] << "}" << '\n';
        }
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

/* Advanced */

std::string CmdProxy::ProgramFpga(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[fname.pof | fname.rbf]\n\t[Jungfrau][Ctb][Moench] Programs "
              "FPGA from pof file. Rebooting controller is recommended. "
              "\n\t[Mythen3][Gotthard2] Programs FPGA from rbf file. Power "
              "cycling the detector is recommended. "
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("Cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        det->programFPGA(args[0], std::vector<int>{det_id});
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
        os << "[server_name] "
              "[pc_host_name]\n\t[Jungfrau][Ctb][Moench][Mythen3][Gotthard2] "
              "Copies detector server via tftp from pc. "
              "\n\t[Jungfrau][Ctb][Moench]Also changes respawn server, which "
              "is effective after a reboot."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("Cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        det->copyDetectorServer(args[0], args[1], std::vector<int>{det_id});
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
        os << "[server_name] [pc_host_name] "
              "[fname.pof]\n\t[Jungfrau][Gotthard][CTB][Moench] Updates the "
              "firmware, detector server and then reboots detector controller "
              "blackfin. \n\t[Mythen3][Gotthard2] Will still have old server "
              "starting up as the new server is not respawned \n\tsname is "
              "name of detector server binary found on tftp folder of host pc "
              "\n\thostname is name of pc to tftp from \n\tfname is "
              "programming file name"
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
        det->updateFirmwareAndServer(args[0], args[1], args[2],
                                     std::vector<int>{det_id});
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
              "in hex. Advanced Function!\n\tGoes to stop server. Hence, can "
              "be called while calling blocking acquire(). \n\t[Eiger] +0x100 "
              "for only left, +0x200 for only right."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        auto t = det->readRegister(StringTo<uint32_t>(args[0]),
                                   std::vector<int>{det_id});
        os << OutStringHex(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        det->writeRegister(StringTo<uint32_t>(args[0]),
                           StringTo<uint32_t>(args[1]),
                           std::vector<int>{det_id});
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
        os << "[address] [value]\n\t[Jungfrau][Ctb][Moench][Gotthard] Writes "
              "to an adc "
              "register in hex. Advanced user Function!"
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("Cannot get.");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        det->writeAdcRegister(StringTo<uint32_t>(args[0]),
                              StringTo<uint32_t>(args[1]),
                              std::vector<int>{det_id});
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
            os << "[reg address in hex] [bit index]\n\tSets bit in address."
               << '\n';
        } else if (cmd == "clearbit") {
            os << "[reg address in hex] [bit index]\n\tClears bit in address."
               << '\n';
        } else if (cmd == "getbit") {
            os << "[reg address in hex] [bit index]\n\tGets bit in address."
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
            auto t = det->getBit(addr, bitnr, std::vector<int>{det_id});
            os << OutString(t) << '\n';
        } else if (action == defs::PUT_ACTION) {
            if (cmd == "getbit") {
                throw sls::RuntimeError("Cannot put");
            }
            if (cmd == "setbit") {
                det->setBit(addr, bitnr, std::vector<int>{det_id});
            } else if (cmd == "clearbit") {
                det->clearBit(addr, bitnr, std::vector<int>{det_id});
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
        os << "[0, 1]\n\tEnable or disable intial compatibility and other "
              "checks at detector start up. It is enabled by default. Must "
              "come before 'hostname' command to take effect. Can be used to "
              "reprogram fpga when current firmware is "
              "incompatible.\n\tAdvanced User function!"
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
        os << "[command]\n\tExecutes command on detector server console."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw sls::RuntimeError("Cannot get.");
    } else if (action == defs::PUT_ACTION) {
        std::string command;
        for (auto &i : args) {
            command += (i + ' ');
        }
        auto t = det->executeCommand(command, std::vector<int>{det_id});
        os << OutString(t) << '\n';
    } else {
        throw sls::RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::UserDetails(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "\n\tUser details from shared memory (hostname, type, PID, User, "
              "Date)."
           << '\n';
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
