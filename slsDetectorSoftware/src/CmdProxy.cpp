
// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "CmdProxy.h"
#include "HelpDacs.h"
#include "sls/TimeHelper.h"
#include "sls/ToString.h"
#include "sls/bit_utils.h"
#include "sls/container_utils.h"
#include "sls/logger.h"
#include "sls/sls_detector_defs.h"

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
                    int action, std::ostream &os, int receiver_id) {
    cmd = command;
    args = arguments;
    det_id = detector_id;
    rx_id = receiver_id;

    std::string temp;
    while (temp != cmd) {
        temp = cmd;
        ReplaceIfDepreciated(cmd);
    }

    auto it = functions.find(cmd);
    if (it != functions.end()) {
        os << ((*this).*(it->second))(action);
    } else {
        throw RuntimeError(cmd +
                           " Unknown command, use list to list all commands");
    }
}

bool CmdProxy::ReplaceIfDepreciated(std::string &command) {
    auto d_it = depreciated_functions.find(command);
    if (d_it != depreciated_functions.end()) {
        LOG(logWARNING)
            << command
            << " is deprecated and will be removed. Please migrate to: "
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

std::map<std::string, std::string> CmdProxy::GetDepreciatedCommands() {
    return depreciated_functions;
}

void CmdProxy::WrongNumberOfParameters(size_t expected) {
    if (expected == 0) {
        throw RuntimeError("Command " + cmd +
                           " expected no parameter/s but got " +
                           std::to_string(args.size()) + "\n");
    }
    throw RuntimeError("Command " + cmd + " expected (or >=) " +
                       std::to_string(expected) + " parameter/s but got " +
                       std::to_string(args.size()) + "\n");
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
            throw RuntimeError("Cannot execute this at module level");
        }
        // only args[0], but many hostames concatenated with +
        if (args[0].find('+') != std::string::npos) {
            auto t = split(args[0], '+');
            det->setHostname(t);
            os << ToString(t) << '\n';
        }
        // either hostnames separated by space, or single hostname
        else {
            det->setHostname(args);
            os << ToString(args) << '\n';
        }
    } else {
        throw RuntimeError("Unknown action");
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
        throw RuntimeError("cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        if (det_id != -1) {
            throw RuntimeError("Cannot execute this at module level");
        }
        det->setVirtualDetectorServers(StringTo<int>(args[0]),
                                       StringTo<int>(args[1]));
        os << ToString(args);
    } else {
        throw RuntimeError("Unknown action");
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
              "enabled\n\t- resets acquiring flag"
           << '\n';
    } else {
        if (det->empty()) {
            throw RuntimeError("This shared memory has no detectors added.");
        }
        if (det_id >= 0) {
            throw RuntimeError("Individual detectors not allowed for readout.");
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
        throw RuntimeError("cannot put");
    } else {
        throw RuntimeError("Unknown action");
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
        auto t = det->getFirmwareVersion(std::vector<int>{det_id});
        os << "\nDetector Type: " << OutString(det->getDetectorType())
           << "\nPackage Version: " << det->getPackageVersion() << std::hex
           << "\nClient Version: 0x" << det->getClientVersion();
        if (det->getDetectorType().squash() == defs::EIGER) {
            os << "\nFirmware Version: " << OutString(t);
        } else {
            os << "\nFirmware Version: " << OutStringHex(t);
        }
        os << "\nDetector Server Version: "
           << OutStringHex(
                  det->getDetectorServerVersion(std::vector<int>{det_id}));
        os << "\nDetector Server Version: "
           << OutString(det->getKernelVersion({std::vector<int>{det_id}}));
        if (det->getUseReceiverFlag().squash(true)) {
            os << "\nReceiver Version: "
               << OutStringHex(
                      det->getReceiverVersion(std::vector<int>{det_id}));
        }
        os << std::dec << '\n';
    } else if (action == defs::PUT_ACTION) {
        throw RuntimeError("cannot put");
    } else {
        throw RuntimeError("Unknown action");
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
        throw RuntimeError("cannot put");
    } else {
        throw RuntimeError("Unknown action");
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
        throw RuntimeError("cannot put");
    } else {
        throw RuntimeError("Unknown action");
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
        os << t << '\n';
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
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::Threshold(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[eV] [(optinal settings)"
              "\n\t[Eiger][Mythen3] Threshold in eV. It loads trim files from "
              "settingspath.";
        if (cmd == "thresholdnotb") {
            os << "Trimbits are not loaded.";
        }
        os << "\n\nthreshold [eV1] [eV2] [eV3] [(optional settings)]"
              "\n\t[Mythen3] Threshold in eV for each counter. It loads trim "
              "files from settingspath. An energy of -1 will pick up values "
              " from detector.";
        if (cmd == "thresholdnotb") {
            os << "Trimbits are not loaded.";
        }
        os << '\n';
    } else if (action == defs::GET_ACTION) {
        if (cmd == "thresholdnotb") {
            throw RuntimeError("cannot get");
        }
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        defs::detectorType type = det->getDetectorType().squash();
        if (type == defs::EIGER) {
            auto t = det->getThresholdEnergy(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        } else if (type == defs::MYTHEN3) {
            auto t = det->getAllThresholdEnergy(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        } else {
            throw RuntimeError("Not implemented for this detector\n");
        }
    } else if (action == defs::PUT_ACTION) {
        defs::detectorType type = det->getDetectorType().squash();
        if (type == defs::EIGER && args.size() != 1 && args.size() != 2) {
            WrongNumberOfParameters(1);
        }
        if (type == defs::MYTHEN3 && (args.size() < 1 || args.size() > 4)) {
            WrongNumberOfParameters(1);
        }

        bool trimbits = (cmd == "thresholdnotb") ? false : true;
        std::array<int, 3> energy = {StringTo<int>(args[0]), 0, 0};
        energy[1] = energy[0];
        energy[2] = energy[0];
        defs::detectorSettings sett = defs::STANDARD;

        // check if argument has settings or get it
        if (args.size() == 2 || args.size() == 4) {
            sett = StringTo<defs::detectorSettings>(args[args.size() - 1]);
        } else {
            sett = det->getSettings(std::vector<int>{det_id})
                       .tsquash("Inconsistent settings between detectors");
        }

        // get other threshold values
        if (args.size() > 2) {
            energy[1] = StringTo<int>(args[1]);
            energy[2] = StringTo<int>(args[2]);
        }
        switch (type) {
        case defs::EIGER:
            det->setThresholdEnergy(energy[0], sett, trimbits,
                                    std::vector<int>{det_id});
            break;
        case defs::MYTHEN3:
            det->setThresholdEnergy(energy, sett, trimbits,
                                    std::vector<int>{det_id});
            break;
        default:
            throw RuntimeError("Not implemented for this detector\n");
        }
        os << ToString(args) << '\n';
    } else {
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::Trimbits(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[fname]\n\t[Eiger][Mythen3] Put will load the trimbit file to "
              "detector. If no extension specified, serial number of each "
              "module is attached. Get will save the trimbits from the "
              "detector to file with serial number added to file name."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        det->saveTrimbits(args[0], std::vector<int>{det_id});
        os << args << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        det->loadTrimbits(args[0], std::vector<int>{det_id});
        os << args << '\n';
    } else {
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::TrimEnergies(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[trim_ev1] [trim_Ev2 (optional)] [trim_ev3 (optional)] "
              "...\n\t[Eiger][Mythen3] Number of trim energies and list of "
              "trim "
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
        os << ToString(args) << '\n';
    } else {
        throw RuntimeError("Unknown action");
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
            throw RuntimeError("Cannot get gap pixels at module level");
        }
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getGapPixelsinCallback();
        os << t << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (det_id != -1) {
            throw RuntimeError("Cannot add gap pixels at module level");
        }
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        det->setGapPixelsinCallback(StringTo<int>(args[0]));
        os << args.front() << '\n';
    } else {
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::BadChannels(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[fname]\n\t[Gotthard2][Mythen3] Sets the bad channels (from "
              "file of bad channel numbers) to be masked out."
              "\n\t[Mythen3] Also does trimming"
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
        throw RuntimeError("Unknown action");
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
        throw RuntimeError("Unknown command, use list to list all commands");
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
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::ReadoutSpeed(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "\n\t[0 or full_speed|1 or half_speed|2 or "
              "quarter_speed]\n\t\t[Eiger][Jungfrau] Readout "
              "speed of chip.\n\t\t[Eiger] Default speed is full_speed."
              "\n\t\t[Jungfrau] Default speed is half_speed. full_speed "
              "option only available from v2.0 boards and is recommended to "
              "set "
              "number of interfaces to 2. Also overwrites "
              "adcphase to recommended default.\n\t [144|108]\n\t\t[Gotthard2] "
              "Readout speed of chip in MHz. Default is 108."
           << '\n';
    } else {
        defs::detectorType type = det->getDetectorType().squash();
        if (type == defs::CHIPTESTBOARD || type == defs::MOENCH) {
            throw RuntimeError(
                "ReadoutSpeed not implemented. Did you mean runclk?");
        }
        if (action == defs::GET_ACTION) {
            if (!args.empty()) {
                WrongNumberOfParameters(0);
            }
            auto t = det->getReadoutSpeed(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        } else if (action == defs::PUT_ACTION) {
            if (args.size() != 1) {
                WrongNumberOfParameters(1);
            }
            defs::speedLevel t = StringTo<defs::speedLevel>(args[0]);
            det->setReadoutSpeed(t, std::vector<int>{det_id});
            os << ToString(t) << '\n'; // no args to convert 0,1,2 as well
        } else {
            throw RuntimeError("Unknown action");
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
            throw RuntimeError("adcphase not implemented for this detector");
        }
        if (action == defs::GET_ACTION) {
            Result<int> t;
            if (args.empty()) {
                t = det->getADCPhase(std::vector<int>{det_id});
                os << OutString(t) << '\n';
            } else if (args.size() == 1) {
                if (args[0] != "deg") {
                    throw RuntimeError("Unknown adcphase   argument " +
                                       args[0] + ". Did you mean deg?    ");
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
                    throw RuntimeError("Unknown adcphase   2nd argument " +
                                       args[1] + ". Did you    mean deg?");
                }
                det->setADCPhaseInDegrees(StringTo<int>(args[0]),
                                          std::vector<int>{det_id});
                os << args[0] << " " << args[1] << '\n';
            } else {
                WrongNumberOfParameters(1);
            }
        } else {
            throw RuntimeError("Unknown action");
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
            throw RuntimeError("dbitphase not implemented for this detector");
        }
        if (action == defs::GET_ACTION) {
            Result<int> t;
            if (args.empty()) {
                t = det->getDBITPhase(std::vector<int>{det_id});
                os << OutString(t) << '\n';
            } else if (args.size() == 1) {
                if (args[0] != "deg") {
                    throw RuntimeError("Unknown dbitphase argument " + args[0] +
                                       ". Did you mean deg?  ");
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
                    throw RuntimeError("Unknown dbitphase 2nd  argument " +
                                       args[1] + ". Did you mean deg?  ");
                }
                det->setDBITPhaseInDegrees(StringTo<int>(args[0]),
                                           std::vector<int>{det_id});
                os << args[0] << " " << args[1] << '\n';
            } else {
                WrongNumberOfParameters(1);
            }
        } else {
            throw RuntimeError("Unknown action");
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
            throw RuntimeError("clkfreq not implemented for this detector.");
        }
        if (action == defs::GET_ACTION) {
            if (args.size() != 1) {
                WrongNumberOfParameters(1);
            }
            auto t = det->getClockFrequency(StringTo<int>(args[0]),
                                            std::vector<int>{det_id});
            os << OutString(t) << '\n';
        } else if (action == defs::PUT_ACTION) {
            throw RuntimeError("cannot put");
        } else {
            throw RuntimeError("Unknown action");
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
            throw RuntimeError("clkphase not implemented for this detector.");
        }
        if (action == defs::GET_ACTION) {
            if (args.size() == 1) {
                auto t = det->getClockPhase(StringTo<int>(args[0]),
                                            std::vector<int>{det_id});
                os << OutString(t) << '\n';
            } else if (args.size() == 2) {
                if (args[1] != "deg") {
                    throw RuntimeError("Cannot scan argument" + args[1] +
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
                    throw RuntimeError("Cannot scan argument" + args[2] +
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
            throw RuntimeError("Unknown action");
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
            throw RuntimeError(
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
            throw RuntimeError("Cannot put");
        } else {
            throw RuntimeError("Unknown action");
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
            throw RuntimeError("clkdiv not implemented for this detector.");
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
            throw RuntimeError("Unknown action");
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
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::CurrentSource(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "\n\t[0|1]\n\t\t[Gotthard2] Enable or disable current source. "
              "Default "
              "is disabled.\n\t[0|1] [fix|nofix] [select source] [(only for "
              "chipv1.1)normal|low]\n\t\t[Jungfrau] Disable or enable current "
              "source with some parameters. The select source is 0-63 for "
              "chipv1.0 and a 64 bit mask for chipv1.1. To disable, one needs "
              "only one argument '0'."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 0) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getCurrentSource(std::vector<int>{det_id});
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() == 1) {
            det->setCurrentSource(
                defs::currentSrcParameters(StringTo<bool>(args[0])));
        } else if (args.size() >= 3) {
            // scan fix
            bool fix = false;
            if (args[1] == "fix") {
                fix = true;
            } else if (args[1] == "nofix") {
                fix = false;
            } else {
                throw RuntimeError("Invalid argument: " + args[1] +
                                   ". Did you mean fix or nofix?");
            }
            if (args.size() == 3) {
                det->setCurrentSource(defs::currentSrcParameters(
                    fix, StringTo<uint64_t>(args[2])));
            } else if (args.size() == 4) {
                bool normalCurrent = false;
                if (args[3] == "normal") {
                    normalCurrent = true;
                } else if (args[3] == "low") {
                    normalCurrent = false;
                } else {
                    throw RuntimeError("Invalid argument: " + args[3] +
                                       ". Did you mean normal or low?");
                }
                det->setCurrentSource(defs::currentSrcParameters(
                    fix, StringTo<uint64_t>(args[2]), normalCurrent));
            } else {
                throw RuntimeError(
                    "Invalid number of parareters for this command.");
            }
        } else {
            throw RuntimeError(
                "Invalid number of parareters for this command.");
        }
        os << ToString(args) << '\n';
    } else {
        throw RuntimeError("Unknown action");
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
        throw RuntimeError("Cannot put");
    } else {
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

/* dacs */
std::string CmdProxy::Dac(int action) {
    std::ostringstream os;
    os << cmd << ' ';

    if (action == defs::HELP_ACTION) {
        if (args.size() == 0) {
            os << GetHelpDac(std::to_string(0)) << '\n';
        } else {
            os << args[0] << ' ' << GetHelpDac(args[0]) << '\n';
        }
        return os.str();
    }

    auto type = det->getDetectorType().squash();

    // dac indices only for ctb
    if (args.size() > 0 && action != defs::HELP_ACTION) {
        if (is_int(args[0]) && type != defs::CHIPTESTBOARD) {
            throw RuntimeError(
                "Dac indices can only be used for chip test board. Use daclist "
                "to get list of dac names for current detector.");
        }
    }

    if (action == defs::GET_ACTION) {
        if (args.empty())
            WrongNumberOfParameters(1); // This prints slightly wrong

        defs::dacIndex dacIndex{};
        // TODO! Remove if
        if (type == defs::CHIPTESTBOARD && !is_int(args[0])) {
            dacIndex = det->getDacIndex(args[0]);
        } else {
            dacIndex = StringTo<defs::dacIndex>(args[0]);
        }

        bool mV = false;

        if (args.size() == 2) {
            if ((args[1] != "mv") && (args[1] != "mV")) {
                throw RuntimeError("Unknown argument " + args[1] +
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

        defs::dacIndex dacIndex{};
        if (type == defs::CHIPTESTBOARD && !is_int(args[0]))
            dacIndex = det->getDacIndex(args[0]);
        else
            dacIndex = StringTo<defs::dacIndex>(args[0]);
        bool mV = false;
        if (args.size() == 3) {
            if ((args[2] != "mv") && (args[2] != "mV")) {
                throw RuntimeError("Unknown argument " + args[2] +
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
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::DacList(const int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "\n\t[dacname1 dacname2 .. dacname18] \n\t\t[ChipTestBoard] Set "
              "the list of dac names for this detector.\n\t\t[All] Gets the "
              "list "
              "of "
              "dac names for every dac for this detector."
           << '\n';
    } else if (action == slsDetectorDefs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getDacNames();
        os << ToString(t) << '\n';
    } else if (action == slsDetectorDefs::PUT_ACTION) {
        if (det->getDetectorType().squash() != defs::CHIPTESTBOARD) {
            throw RuntimeError("This detector already has fixed dac "
                               "names. Cannot change them.");
        }
        if (det_id != -1) {
            throw RuntimeError("Cannot configure dacnames at module level");
        }
        if (args.size() != 18) {
            WrongNumberOfParameters(18);
        }
        det->setDacNames(args);
        os << ToString(args) << '\n';
    } else {
        throw RuntimeError("Unknown action");
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
                throw RuntimeError("Unknown argument " + args[0] +
                                   ". Did you mean mV?");
            }
            mv = true;
        } else if (args.size() > 1) {
            WrongNumberOfParameters(1);
        }
        auto t = det->getDacList();
        auto names = det->getDacNames();
        auto name_it = names.begin();
        os << '[';
        auto it = t.cbegin();
        os << ToString(*name_it++) << ' ';
        os << OutString(det->getDAC(*it++, mv, std::vector<int>{det_id}))
           << (!args.empty() ? " mV" : "");
        while (it != t.cend()) {
            os << ", " << ToString(*name_it++) << ' ';
            os << OutString(det->getDAC(*it++, mv, std::vector<int>{det_id}))
               << (!args.empty() ? " mV" : "");
        }
        os << "]\n";
    } else if (action == defs::PUT_ACTION) {
        throw RuntimeError("Cannot put");
    } else {
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::ResetDacs(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[(optional) hard] "
              "\n\t[Eiger][Jungfrau][Gotthard][Moench][Gotthard2]["
              "Mythen3]Reset dac values to the defaults. A 'hard' optional "
              "reset will reset the dacs to the hardcoded defaults in on-board "
              "detector server."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw RuntimeError("Cannot get");
    } else if (action == defs::PUT_ACTION) {
        bool hardReset = false;
        if (args.size() == 1) {
            if (args[0] != "hard") {
                throw RuntimeError("Unknown argument " + args[0] +
                                   ". Did you mean hard?");
            }
            hardReset = true;
        } else if (args.size() > 1) {
            WrongNumberOfParameters(1);
        }
        det->resetToDefaultDacs(hardReset, std::vector<int>{det_id});
        os << "successful\n";
    } else {
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::DefaultDac(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[dac name][value][(optional)setting]\n\tSets the default for "
              "that dac to this value.\n\t[Jungfrau][Mythen3] When settings is "
              "provided, it sets the default value only for that setting"
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (args.size() < 1) {
            WrongNumberOfParameters(1);
        }
        // optional settings
        if (args.size() == 2) {
            auto t = det->getDefaultDac(
                StringTo<defs::dacIndex>(args[0]),
                StringTo<slsDetectorDefs::detectorSettings>(args[1]),
                std::vector<int>{det_id});
            os << args[0] << ' ' << args[1] << ' ' << OutString(t) << '\n';
        } else {
            auto t = det->getDefaultDac(StringTo<defs::dacIndex>(args[0]),
                                        std::vector<int>{det_id});
            os << args[0] << ' ' << OutString(t) << '\n';
        }
    } else if (action == defs::PUT_ACTION) {
        if (args.size() < 2) {
            WrongNumberOfParameters(2);
        }
        // optional settings
        if (args.size() == 3) {
            det->setDefaultDac(
                StringTo<defs::dacIndex>(args[0]), StringTo<int>(args[1]),
                StringTo<slsDetectorDefs::detectorSettings>(args[2]),
                std::vector<int>{det_id});
            os << args[0] << ' ' << args[2] << ' ' << args[1] << '\n';
        } else {
            det->setDefaultDac(StringTo<defs::dacIndex>(args[0]),
                               StringTo<int>(args[1]));
            os << args[0] << ' ' << args[1] << '\n';
        }
    } else {
        throw RuntimeError("Unknown action");
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
        throw RuntimeError(
            "Cannot put. Did you mean to use command 'rx_start' or 'rx_stop'?");
    } else {
        throw RuntimeError("Unknown action");
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
        throw RuntimeError(
            "Cannot put. Did you mean to use command 'start' or 'stop'?");
    } else {
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::Scan(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[dac_name|0|trimbits] [start_val] [stop_val] "
              "[step_size] [dac settling time ns|us|ms|s]\n\tEnables/ disables "
              "scans for dac and trimbits \n\tEnabling scan sets number of "
              "frames to number of steps in receiver. \n\tTo cancel scan "
              "configuration, set dac to '0', which also sets number of frames "
              "to 1. \n\t[Eiger][Mythen3] Use trimbits as dac name for a "
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
            throw RuntimeError("Cannot configure scan at module level");
        }
        // disable
        if (args.size() == 1) {
            if (StringTo<int>(args[0]) != 0) {
                throw RuntimeError("Did you mean '0' to disable?");
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
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::Trigger(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        if (cmd == "trigger") {
            os << "\n\t[Eiger][Mythen3][Jungfrau] Sends software trigger "
                  "signal to detector";
        } else if (cmd == "blockingtrigger") {
            os << "\n\t[Eiger][Jungfrau] Sends software trigger signal to "
                  "detector and blocks till the frames are sent out for that "
                  "trigger.";
        } else {
            throw RuntimeError("unknown command " + cmd);
        }
        os << '\n';
    } else if (action == slsDetectorDefs::GET_ACTION) {
        throw RuntimeError("Cannot get");
    } else if (action == slsDetectorDefs::PUT_ACTION) {
        if (det_id != -1) {
            throw RuntimeError("Cannot execute this at module level");
        }
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        bool block = false;
        if (cmd == "blockingtrigger") {
            block = true;
        }
        det->sendSoftwareTrigger(block);
        os << "successful\n";
    } else {
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

/* Network Configuration (Detector<->Receiver) */

IpAddr CmdProxy::getIpFromAuto() {
    std::string rxHostname =
        det->getRxHostname(std::vector<int>{det_id}).squash("none");
    // Hostname could be ip try to decode otherwise look up the hostname
    auto val = IpAddr{rxHostname};
    if (val == 0) {
        val = HostnameToIp(rxHostname.c_str());
    }
    return val;
}

UdpDestination CmdProxy::getUdpEntry() {
    UdpDestination udpDestination{};
    udpDestination.entry = rx_id;

    for (auto it : args) {
        size_t pos = it.find('=');
        std::string key = it.substr(0, pos);
        std::string value = it.substr(pos + 1);
        if (key == "ip") {
            if (value == "auto") {
                auto val = getIpFromAuto();
                LOG(logINFO) << "Setting udp_dstip of detector " << det_id
                             << " to " << val;
                udpDestination.ip = val;
            } else {
                udpDestination.ip = IpAddr(value);
            }
        } else if (key == "ip2") {
            if (value == "auto") {
                auto val = getIpFromAuto();
                LOG(logINFO) << "Setting udp_dstip2 of detector " << det_id
                             << " to " << val;
                udpDestination.ip2 = val;
            } else {
                udpDestination.ip2 = IpAddr(value);
            }
        } else if (key == "mac") {
            udpDestination.mac = MacAddr(value);
        } else if (key == "mac2") {
            udpDestination.mac2 = MacAddr(value);
        } else if (key == "port") {
            udpDestination.port = StringTo<uint32_t>(value);
        } else if (key == "port2") {
            udpDestination.port2 = StringTo<uint32_t>(value);
        }
    }
    return udpDestination;
}

std::string CmdProxy::UDPDestinationList(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[ip=x.x.x.x] [(optional)ip2=x.x.x.x] "
              "\n\t[mac=xx:xx:xx:xx:xx:xx] "
              "[(optional)mac2=xx:xx:xx:xx:xx:xx]\n\t[port=value] "
              "[(optional)port2=value\n\tThe order of ip, mac and port does "
              "not matter. entry_value can be >0 only for "
              "[Eiger][Jungfrau][Mythen3][Gotthard2] where round robin is "
              "implemented. If 'auto' used, then ip is set to ip of "
              "rx_hostname."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        if (det_id == -1) {
            throw RuntimeError("udp_dstlist must be at module level.");
        }
        if (rx_id < 0 || rx_id >= MAX_UDP_DESTINATION) {
            throw RuntimeError(std::string("Invalid receiver index ") +
                               std::to_string(rx_id) +
                               std::string(" to set round robin entry."));
        }
        auto t = det->getDestinationUDPList(rx_id, std::vector<int>{det_id});
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.empty()) {
            WrongNumberOfParameters(1);
        }
        if (det_id == -1) {
            throw RuntimeError("udp_dstlist must be at module level.");
        }
        if (rx_id < 0 || rx_id >= MAX_UDP_DESTINATION) {
            throw RuntimeError(
                "Invalid receiver index to set round robin entry.");
        }
        auto t = getUdpEntry();
        det->setDestinationUDPList(t, det_id);
        os << ToString(args) << std::endl;
    } else {
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::UDPSourceIP(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[x.x.x.x] or auto\n\tIp address of the detector (source) udp "
              "interface. Must be same subnet as destination udp "
              "ip.\n\t[Eiger] Set only for 10G. For 1G, detector will replace "
              "with its own DHCP IP address. If 'auto' used, then ip is set to "
              "ip of rx_hostname."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        auto t = det->getSourceUDPIP(std::vector<int>{det_id});
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        IpAddr val;
        if (args[0] == "auto") {
            val = getIpFromAuto();
            LOG(logINFO) << "Setting udp_srcip of detector " << det_id << " to "
                         << val;
        } else {
            val = IpAddr(args[0]);
        }
        det->setSourceUDPIP(val, std::vector<int>{det_id});
        os << val << '\n';

    } else {
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::UDPSourceIP2(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[x.x.x.x] or auto\n\t[Jungfrau][Gotthard2] Ip address of the "
              "detector (source) udp interface 2. Must be same subnet as "
              "destination udp ip2.\n\t [Jungfrau] top half or inner "
              "interface\n\t [Gotthard2] veto debugging. If 'auto' used, then "
              "ip is set to ip of rx_hostname."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        auto t = det->getSourceUDPIP2(std::vector<int>{det_id});
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        IpAddr val;
        if (args[0] == "auto") {
            val = getIpFromAuto();
            LOG(logINFO) << "Setting udp_srcip2 of detector " << det_id
                         << " to " << val;
        } else {
            val = IpAddr(args[0]);
        }
        det->setSourceUDPIP2(val, std::vector<int>{det_id});
        os << val << '\n';

    } else {
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

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
            auto val = getIpFromAuto();
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
        throw RuntimeError("Unknown action");
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
            auto val = getIpFromAuto();
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
        throw RuntimeError("Unknown action");
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
                throw RuntimeError(
                    "Cannot add multiple receivers at module level");
            }
            if (det_id != -1) {
                throw RuntimeError(
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
                    throw RuntimeError(
                        "Cannot add multiple receivers at module level");
                }
                auto t = split(args[0], '+');
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
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::Rx_ROI(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[xmin] [xmax] [ymin] [ymax]\n\tRegion of interest in "
              "receiver.\n\tOnly allowed at multi module level and without gap "
              "pixels."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        if (det_id == -1) {
            auto t = det->getRxROI();
            os << t << '\n';
        } else {
            auto t = det->getIndividualRxROIs(std::vector<int>{det_id});
            os << t << '\n';
        }
    } else if (action == defs::PUT_ACTION) {
        defs::ROI t;
        // 2 or 4 arguments
        if (args.size() != 2 && args.size() != 4) {
            WrongNumberOfParameters(2);
        }
        if (args.size() == 2 || args.size() == 4) {
            t.xmin = StringTo<int>(args[0]);
            t.xmax = StringTo<int>(args[1]);
        }
        if (args.size() == 4) {
            t.ymin = StringTo<int>(args[2]);
            t.ymax = StringTo<int>(args[3]);
        }
        // only multi level
        if (det_id != -1) {
            throw RuntimeError("Cannot execute receiver ROI at module level");
        }
        det->setRxROI(t);
        os << t << '\n';
    } else {
        throw RuntimeError("Unknown action");
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
              "-1. \n\tThis is a high number and can be set to 2 for gui "
              "purposes. \n\tOne must also set the receiver's send high water "
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
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

/* Eiger Specific */

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
        throw RuntimeError("Unknown action");
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
        throw RuntimeError("cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 3) {
            WrongNumberOfParameters(3);
        }
        int n = StringTo<int>(args[0]);
        defs::xy c;
        c.x = StringTo<int>(args[1]);
        c.y = StringTo<int>(args[2]);
        det->pulsePixel(n, c, std::vector<int>{det_id});
        os << ToString(args) << '\n';
    } else {
        throw RuntimeError("Unknown action");
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
        throw RuntimeError("cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 3) {
            WrongNumberOfParameters(3);
        }
        int n = StringTo<int>(args[0]);
        defs::xy c;
        c.x = StringTo<int>(args[1]);
        c.y = StringTo<int>(args[2]);
        det->pulsePixelNMove(n, c, std::vector<int>{det_id});
        os << ToString(args) << '\n';
    } else {
        throw RuntimeError("Unknown action");
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
        throw RuntimeError("cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        det->pulseChip(StringTo<int>(args[0]), std::vector<int>{det_id});
        os << args.front() << '\n';
    } else {
        throw RuntimeError("Unknown action");
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
            throw RuntimeError("Cannot execute quad at module level");
        }
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        det->setQuad(StringTo<int>(args[0]));
        os << args.front() << '\n';
    } else {
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::DataStream(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[left|right] [0, 1]\n\t[Eiger] Enables or disables data "
              "streaming from left or/and right side of detector for 10 GbE "
              "mode. "
              "1 (enabled) "
              "by default."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        auto t = det->getDataStream(StringTo<defs::portPosition>(args[0]),
                                    std::vector<int>{det_id});
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        det->setDataStream(StringTo<defs::portPosition>(args[0]),
                           StringTo<bool>(args[1]), std::vector<int>{det_id});
        os << args << '\n';
    } else {
        throw RuntimeError("Unknown action");
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
            throw RuntimeError("Unknown argument for temp event. Did you "
                               "mean 0 to reset event?");
        }
        det->resetTemperatureEvent(std::vector<int>{det_id});
        os << "cleared" << '\n';
    } else {
        throw RuntimeError("Unknown action");
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
        os << t << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (det_id == -1 && det->size() > 1) {
            throw RuntimeError("Cannot execute ROI at multi module level");
        }
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        defs::ROI t(StringTo<int>(args[0]), StringTo<int>(args[1]));
        det->setROI(t, det_id);
        os << t << '\n';
    } else {
        throw RuntimeError("Unknown action");
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
        os << ToString(args) << '\n';
    } else {
        throw RuntimeError("Unknown action");
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
        os << ToString(args) << '\n';
    } else {
        throw RuntimeError("Unknown action");
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
        throw RuntimeError("cannot get vetoref. Did you mean vetophoton?");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        det->setVetoReference(StringTo<int>(args[0]), StringTo<int>(args[1]),
                              {det_id});
        os << ToString(args) << '\n';
    } else {
        throw RuntimeError("Unknown action");
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
        throw RuntimeError("cannot get vetofile. Did you mean vetophoton?");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        det->setVetoFile(StringTo<int>(args[0]), args[1],
                         std::vector<int>{det_id});
        os << ToString(args) << '\n';
    } else {
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::BurstMode(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[burst_internal or 0, burst_external or 1, cw_internal or 2, "
              "cw_external or 3]\n\t[Gotthard2] Default is burst_internal "
              "type. Also changes clkdiv 2, 3, 4"
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
                    throw RuntimeError("Unknown burst mode " + args[0]);
                }
            } catch (...) {
                t = StringTo<defs::burstMode>(args[0]);
            }
            det->setBurstMode(t, std::vector<int>{det_id});
            os << ToString(t) << '\n'; // no args to convert 0,1,2 as well
        } else {
            throw RuntimeError("Unknown action");
        }
    }
    return os.str();
}

std::string CmdProxy::VetoStreaming(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[none|lll|10gbe|...]\n\t[Gotthard2] Enable or disable the 2 "
              "veto streaming interfaces available. Can include more than one "
              "interface. \n\tDefault: none. lll (low latency link) is the "
              "default interface to work with. \n\t10GbE is for debugging and "
              "also enables second interface in receiver for listening to veto "
              "packets (writes a separate file if writing enabled). Also "
              "restarts client and receiver zmq sockets if zmq streaming "
              "enabled."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getVetoStream(std::vector<int>{det_id});
        os << OutString(t) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.empty()) {
            WrongNumberOfParameters(1);
        }
        defs::streamingInterface interface = defs::streamingInterface::NONE;
        for (const auto &arg : args) {
            if (arg == "none") {
                if (args.size() > 1) {
                    throw RuntimeError(
                        std::string(
                            "cannot have other arguments with 'none'. args: ") +
                        ToString(args));
                }
                break;
            }
            interface = interface | (StringTo<defs::streamingInterface>(arg));
        }
        det->setVetoStream(interface, std::vector<int>{det_id});
        os << ToString(interface) << '\n';
    } else {
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::VetoAlgorithm(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[hits|raw] [lll|10gbe]\n\t[Gotthard2] Set the veto "
              "algorithm. Default is hits."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        defs::streamingInterface interface =
            StringTo<defs::streamingInterface>(args[0]);
        if (interface == defs::streamingInterface::NONE) {
            throw RuntimeError("Must specify an interface to set algorithm");
        }
        auto t = det->getVetoAlgorithm(interface, std::vector<int>{det_id});
        os << OutString(t) << ' ' << ToString(interface) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        defs::vetoAlgorithm alg = StringTo<defs::vetoAlgorithm>(args[0]);
        defs::streamingInterface interface =
            StringTo<defs::streamingInterface>(args[1]);
        if (interface == defs::streamingInterface::NONE) {
            throw RuntimeError("Must specify an interface to set algorithm");
        }
        det->setVetoAlgorithm(alg, interface, std::vector<int>{det_id});
        os << ToString(alg) << ' ' << ToString(interface) << '\n';
    } else {
        throw RuntimeError("Unknown action");
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
        throw RuntimeError("Unknown action");
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
              "repetitive. Enabling counters sets vth dacs to remembered "
              "values and disabling sets them to disabled values."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto mask = det->getCounterMask(std::vector<int>{det_id}).squash(-1);
        os << ToString(getSetBits(mask)) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.empty()) {
            WrongNumberOfParameters(1);
        }
        if (std::any_of(args.cbegin(), args.cend(), [](std::string s) {
                return (StringTo<int>(s) < 0 || StringTo<int>(s) > 2);
            })) {
            throw RuntimeError("Invalid counter indices list. Example: 0 1 2");
        }
        // convert vector to counter enable mask
        uint32_t mask = 0;
        for (size_t i = 0; i < args.size(); ++i) {
            int val = StringTo<int>(args[i]);
            // already enabled earlier
            if (mask & (1 << val)) {
                std::ostringstream oss;
                oss << "Duplicate counter values (" << val << ") in arguments";
                throw RuntimeError(oss.str());
            }
            mask |= (1 << val);
        }
        det->setCounterMask(mask, std::vector<int>{det_id});
        os << ToString(args) << '\n';
    } else {
        throw RuntimeError("Unknown action");
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
        throw RuntimeError("Unknown command, use list to list all commands");
    }

    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        if (cmd == "gatedelay") {
            os << "[duration] [(optional unit) ns|us|ms|s]\n\t[Mythen3] Gate "
                  "Delay of all gate signals in auto and trigger mode "
                  "(internal gating)."
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
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::GainCaps(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[cap1, cap2, ...]\n\t[Mythen3] gain, options: C10pre, C15sh, "
              "C30sh, C50sh, C225ACsh, C15pre"
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty())
            WrongNumberOfParameters(0);

        auto tmp = det->getGainCaps();
        Result<defs::M3_GainCaps> csr;
        for (auto val : tmp) {
            if (val)
                csr.push_back(static_cast<defs::M3_GainCaps>(val));
        }

        os << OutString(csr) << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (args.size() < 1) {
            WrongNumberOfParameters(1);
        }
        int caps = 0;
        for (const auto &arg : args) {
            if (arg != "0")
                caps |= StringTo<defs::M3_GainCaps>(arg);
        }

        det->setGainCaps(caps);
        os << OutString(args) << '\n';
    } else {
        throw RuntimeError("Unknown action");
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
                throw RuntimeError(
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
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

/* CTB Specific */
std::string CmdProxy::AdcVpp(int action) {
    std::ostringstream os;
    os << cmd << ' ';

    if (action == defs::HELP_ACTION) {
        os << "[dac or mV value][(optional unit) mV] \n\t[Ctb][Moench] Vpp of "
              "ADC.\n\t 0 -> 1V ; 1 -> 1.14V ; 2 -> 1.33V ; 3 -> 1.6V ; 4 -> "
              "2V. "
              "\n\tAdvanced User function!\n"
           << '\n';
        return os.str();
    }

    if (action == defs::GET_ACTION) {
        bool mV = false;

        if (args.size() == 1) {
            if ((args[0] != "mv") && (args[0] != "mV")) {
                throw RuntimeError("Unknown argument " + args[0] +
                                   ". Did you mean mV?");
            }
            mV = true;
        } else if (args.size() > 1) {
            WrongNumberOfParameters(1);
        }
        auto t = det->getADCVpp(mV, std::vector<int>{det_id});
        os << OutString(t) << (mV ? " mV\n" : "\n");
    } else if (action == defs::PUT_ACTION) {
        bool mV = false;
        if (args.size() == 2) {
            if ((args[1] != "mv") && (args[1] != "mV")) {
                throw RuntimeError("Unknown argument " + args[1] +
                                   ". Did you mean mV?");
            }
            mV = true;
        } else if (args.size() > 2 || args.size() < 1) {
            WrongNumberOfParameters(1);
        }
        det->setADCVpp(StringTo<int>(args[0]), mV, std::vector<int>{det_id});
        os << args[0] << (mV ? " mV\n" : "\n");
    } else {
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

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
            throw RuntimeError("Unknown adc argument " + args[0]);
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
        throw RuntimeError("cannot put");
    } else {
        throw RuntimeError("Unknown action");
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
        os << ToString(args) << '\n';
    } else {
        throw RuntimeError("Unknown action");
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
        throw RuntimeError("Cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        det->setDigitalIODelay(StringTo<uint64_t>(args[0]),
                               StringTo<int>(args[1]),
                               std::vector<int>{det_id});
        os << ToString(args) << '\n';
    } else {
        throw RuntimeError("Unknown action");
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
        throw RuntimeError("Cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        det->setPattern(args[0], std::vector<int>{det_id});
        os << args.front() << '\n';
    } else {
        throw RuntimeError("Unknown action");
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
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

void CmdProxy::GetLevelAndUpdateArgIndex(int action,
                                         std::string levelSeparatedCommand,
                                         int &level, int &iArg, size_t nGetArgs,
                                         size_t nPutArgs) {
    if (cmd == levelSeparatedCommand) {
        ++nGetArgs;
        ++nPutArgs;
    } else {
        LOG(logWARNING) << "This command is deprecated and will be removed. "
                           "Please migrate to "
                        << levelSeparatedCommand;
    }
    if (action == defs::GET_ACTION && args.size() != nGetArgs) {
        WrongNumberOfParameters(nGetArgs);
    } else if (action == defs::PUT_ACTION && args.size() != nPutArgs) {
        WrongNumberOfParameters(nPutArgs);
    }
    if (cmd == levelSeparatedCommand) {
        level = StringTo<int>(args[iArg++]);
    } else {
        level = cmd[cmd.find_first_of("012")] - '0';
    }
}

std::string CmdProxy::PatternLoopAddresses(int action) {
    if (cmd != "patlimits" && cmd != "patloop0" && cmd != "patloop1" &&
        cmd != "patloop2" && cmd != "patloop") {
        throw RuntimeError("Unknown command, use list to list all commands");
    }
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        if (cmd == "patlimits") {
            os << "[start addr] [stop addr] \n\t[Ctb][Moench][Mythen3] Limits "
                  "of complete pattern."
               << '\n';
        } else if (cmd == "patloop") {
            os << "[0-6] [start addr] [stop addr] \n\t[Ctb][Moench][Mythen3] "
                  "Limits of the loop level provided."
               << "\n\t[Mythen3] Level options: 0-3 only." << '\n';
        } else {
            os << "Depreciated command. Use patloop." << '\n';
        }
    } else {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 2;
        if (cmd != "patlimits") {
            GetLevelAndUpdateArgIndex(action, "patloop", level, iArg, nGetArgs,
                                      nPutArgs);
        }
        if (action == defs::GET_ACTION) {
            auto t =
                det->getPatternLoopAddresses(level, std::vector<int>{det_id});
            os << OutStringHex(t, 4) << '\n';
        } else if (action == defs::PUT_ACTION) {
            int start = StringTo<int>(args[iArg++]);
            int stop = StringTo<int>(args[iArg++]);
            det->setPatternLoopAddresses(level, start, stop,
                                         std::vector<int>{det_id});
            os << '[' << ToStringHex(start, 4) << ", " << ToStringHex(stop, 4)
               << "]\n";
        } else {
            throw RuntimeError("Unknown action");
        }
    }
    return os.str();
}

std::string CmdProxy::PatternLoopCycles(int action) {
    if (cmd != "patnloop0" && cmd != "patnloop1" && cmd != "patnloop2" &&
        cmd != "patnloop") {
        throw RuntimeError("Unknown command, use list to list all commands");
    }
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        if (cmd == "patnloop") {
            os << "[0-6] [n_cycles] \n\t[Ctb][Moench][Mythen3] Number of "
                  "cycles of "
                  "the loop level provided."
               << "\n\t[Mythen3] Level options: 0-3 only." << '\n';
        } else {
            os << "Depreciated command. Use patnloop." << '\n';
        }
    } else {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 1;
        GetLevelAndUpdateArgIndex(action, "patnloop", level, iArg, nGetArgs,
                                  nPutArgs);
        if (action == defs::GET_ACTION) {
            auto t = det->getPatternLoopCycles(level, std::vector<int>{det_id});
            os << OutString(t) << '\n';
        } else if (action == defs::PUT_ACTION) {
            int nloops = StringTo<int>(args[iArg++]);
            det->setPatternLoopCycles(level, nloops, std::vector<int>{det_id});
            os << nloops << '\n';
        } else {
            throw RuntimeError("Unknown action");
        }
    }
    return os.str();
}

std::string CmdProxy::PatternWaitAddress(int action) {
    if (cmd != "patwait0" && cmd != "patwait1" && cmd != "patwait2" &&
        cmd != "patwait") {
        throw RuntimeError("Unknown command, use list to list all commands");
    }
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        if (cmd == "patwait") {
            os << "[0-6] [addr] \n\t[Ctb][Moench][Mythen3] Wait address for "
                  "loop level provided."
               << "\n\t[Mythen3] Level options: 0-3 only.";
        } else {
            os << "Depreciated command. Use patwait.";
        }
        os << '\n';
    } else {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 1;
        GetLevelAndUpdateArgIndex(action, "patwait", level, iArg, nGetArgs,
                                  nPutArgs);
        if (action == defs::GET_ACTION) {
            auto t = det->getPatternWaitAddr(level, std::vector<int>{det_id});
            os << OutStringHex(t, 4) << '\n';
        } else if (action == defs::PUT_ACTION) {
            int addr = StringTo<int>(args[iArg++]);
            det->setPatternWaitAddr(level, addr, std::vector<int>{det_id});
            os << ToStringHex(addr, 4) << '\n';
        } else {
            throw RuntimeError("Unknown action");
        }
    }
    return os.str();
}

std::string CmdProxy::PatternWaitTime(int action) {
    if (cmd != "patwaittime0" && cmd != "patwaittime1" &&
        cmd != "patwaittime2" && cmd != "patwaittime") {
        throw RuntimeError("Unknown command, use list to list all commands");
    }
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        if (cmd == "patwaittime") {
            os << "[0-6] [n_clk] \n\t[Ctb][Moench][Mythen3] Wait time in clock "
                  "cycles for the loop provided."
               << "\n\t[Mythen3] Level options: 0-3 only." << '\n';
        } else {
            os << "Depreciated command. Use patwaittime." << '\n';
        }
    } else {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 1;
        GetLevelAndUpdateArgIndex(action, "patwaittime", level, iArg, nGetArgs,
                                  nPutArgs);
        if (action == defs::GET_ACTION) {
            auto t = det->getPatternWaitTime(level, std::vector<int>{det_id});
            os << OutString(t) << '\n';
        } else if (action == defs::PUT_ACTION) {
            uint64_t waittime = StringTo<uint64_t>(args[iArg++]);
            det->setPatternWaitTime(level, waittime, {det_id});
            os << waittime << '\n';
        } else {
            throw RuntimeError("Unknown action");
        }
    }
    return os.str();
}

/* Moench */

std::string CmdProxy::AdditionalJsonHeader(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[key1] [value1] [key2] [value2]...[keyn] [valuen]\n\tAdditional "
              "json header to be streamed out from receiver via zmq. Default "
              "is empty. Max 20 characters for each key/value. Use only if to "
              "be processed by an intermediate user process listening to "
              "receiver zmq packets. Empty value deletes header. "
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
        os << ToString(json) << '\n';
    } else {
        throw RuntimeError("Unknown action");
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
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

/* Advanced */

std::string CmdProxy::ProgramFpga(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[fname.pof | fname.rbf (full "
              "path)][(opitonal)--force-delete-normal-file]\n\t[Jungfrau][Ctb]["
              "Moench] Programs FPGA from pof file (full path). Then, detector "
              "controller is rebooted. \n\t\tUse --force-delete-normal-file "
              "argument, if normal file found in device tree, it must be "
              "deleted, a new device drive created and programming "
              "continued.\n\t[Mythen3][Gotthard2] Programs FPGA from rbf file "
              "(full path). Then, detector controller is rebooted."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw RuntimeError("Cannot get");
    } else if (action == defs::PUT_ACTION) {
        bool forceDeteleNormalFile = false;
        if (args.size() == 2) {
            if (args[1] != "--force-delete-normal-file") {
                throw RuntimeError("Could not scan second argument. Did you "
                                   "mean --force-delete-normal-file?");
            }
            forceDeteleNormalFile = true;
        } else if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        det->programFPGA(args[0], forceDeteleNormalFile,
                         std::vector<int>{det_id});
        os << "successful\n";
    } else {
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::UpdateDetectorServer(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[server_name  with full "
              "path]\n\t[Jungfrau][Eiger][Ctb][Moench][Mythen3][Gotthard2] "
              "Copies detector server via TCP (without tftp). Makes a symbolic "
              "link with a shorter name (without vx.x.x). Then, detector "
              "controller reboots (except "
              "Eiger).\n\t[Jungfrau][Ctb][Moench]Also changes respawn server "
              "to the link, which is effective after a reboot."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw RuntimeError("Cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        det->updateDetectorServer(args[0], std::vector<int>{det_id});
        os << "successful\n";
    } else {
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::UpdateKernel(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[kernel_name with full "
              "path]\n\t[Jungfrau][Ctb][Moench][Mythen3][Gotthard2] Advanced "
              "Command!! You could damage the detector. Please use with "
              "caution.\n\tUpdates the kernel image. Then, detector controller "
              "reboots with new kernel."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw RuntimeError("Cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        det->updateKernel(args[0], std::vector<int>{det_id});
        os << "successful\n";
    } else {
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::UpdateFirmwareAndDetectorServer(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "\n\tWithout tftp: [server_name (incl fullpath)] [fname.pof "
              "(incl full path)] This does not use "
              "tftp.\n\t\t[Jungfrau][Gotthard][CTB][Moench] Updates the "
              "firmware, detector server, deletes old server, creates the "
              "symbolic link and then reboots detector controller. "
              "\n\t\t[Mythen3][Gotthard2] will require a script to start up "
              "the shorter named server link at start up. \n\t\tserver_name is "
              "full path name of detector server binary\n\t\tfname is full "
              "path of programming file"
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw RuntimeError("Cannot get");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        int fpos = args.size() - 1;
        if (args[fpos].find(".pof") == std::string::npos &&
            args[fpos].find(".rbf") == std::string::npos) {
            throw RuntimeError("Programming file must be a pof/rbf file.");
        }
        det->updateFirmwareAndServer(args[0], args[1],
                                     std::vector<int>{det_id});
        os << "successful\n";
    } else {
        throw RuntimeError("Unknown action");
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
        os << ToString(args) << '\n';
    } else {
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

std::string CmdProxy::AdcRegister(int action) {
    std::ostringstream os;
    os << cmd << ' ';
    if (action == defs::HELP_ACTION) {
        os << "[address] [value]\n\t[Jungfrau][Ctb][Moench][Gotthard] Writes "
              "to an adc register in hex. Advanced user Function!"
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw RuntimeError("Cannot get.");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 2) {
            WrongNumberOfParameters(2);
        }
        det->writeAdcRegister(StringTo<uint32_t>(args[0]),
                              StringTo<uint32_t>(args[1]),
                              std::vector<int>{det_id});
        os << ToString(args) << '\n';
    } else {
        throw RuntimeError("Unknown action");
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
            throw RuntimeError(
                "Unknown command, use list to list all commands");
        }
    } else {
        if (cmd != "setbit" && cmd != "clearbit" && cmd != "getbit") {
            throw RuntimeError(
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
                throw RuntimeError("Cannot get");
            }
            auto t = det->getBit(addr, bitnr, std::vector<int>{det_id});
            os << OutString(t) << '\n';
        } else if (action == defs::PUT_ACTION) {
            if (cmd == "getbit") {
                throw RuntimeError("Cannot put");
            }
            if (cmd == "setbit") {
                det->setBit(addr, bitnr, std::vector<int>{det_id});
            } else if (cmd == "clearbit") {
                det->clearBit(addr, bitnr, std::vector<int>{det_id});
            }
            os << ToString(args) << '\n';
        } else {
            throw RuntimeError("Unknown action");
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
            throw RuntimeError(
                "Cannot enable/disable initial checks at module level");
        }
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getInitialChecks();
        os << t << '\n';
    } else if (action == defs::PUT_ACTION) {
        if (det_id != -1) {
            throw RuntimeError(
                "Cannot get initial checks enable at module level");
        }
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        det->setInitialChecks(StringTo<int>(args[0]));
        os << args.front() << '\n';
    } else {
        throw RuntimeError("Unknown action");
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
        throw RuntimeError("Cannot get.");
    } else if (action == defs::PUT_ACTION) {
        std::string command;
        for (auto &i : args) {
            command += (i + ' ');
        }
        auto t = det->executeCommand(command, std::vector<int>{det_id});
        os << OutString(t) << '\n';
    } else {
        throw RuntimeError("Unknown action");
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
            throw RuntimeError("Cannot execute this at module level");
        }
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto t = det->getUserDetails();
        os << t << '\n';
    } else if (action == defs::PUT_ACTION) {
        throw RuntimeError("Cannot put.");
    } else {
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

} // namespace sls
