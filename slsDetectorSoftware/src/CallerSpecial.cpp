#include "Caller.h"
#include "sls/ZmqSocket.h"
#include "sls/bit_utils.h"
#include "sls/file_utils.h"
#include "sls/logger.h"
#include "sls/string_utils.h"
#include <iostream>
#include <thread>
namespace sls {
// some helper functions to print

std::vector<std::string> Caller::getAllCommands() {
    std::vector<std::string> ret;
    for (auto it : functions)
        ret.push_back(it.first);
    return ret;
}

std::map<std::string, std::string> Caller::GetDeprecatedCommands() {
    return deprecated_functions;
}

void Caller::call(const std::string &command,
                  const std::vector<std::string> &arguments, int detector_id,
                  int action, std::ostream &os, int receiver_id) {
    cmd = command;
    args = arguments; // copy args before replacing
    std::string temp;
    while (temp != cmd) {
        temp = cmd;
        ReplaceIfDeprecated(cmd);
    }

    det_id = detector_id;
    rx_id = receiver_id;
    auto it = functions.find(cmd);
    if (it != functions.end()) {
        auto ret = ((*this).*(it->second))(action);
        os << cmd << ' ' << ret;
    } else {
        throw RuntimeError(cmd +
                           " Unknown command, use list to list all commands");
    }
}

bool Caller::ReplaceIfDeprecated(std::string &command) {
    auto d_it = deprecated_functions.find(command);
    if (d_it != deprecated_functions.end()) {

        // insert old command into arguments (for dacs)
        if (d_it->second == "dac") {
            args.insert(args.begin(), command);
            LOG(logWARNING)
                << command
                << " is deprecated and will be removed. Please migrate to: "
                << d_it->second << " " << command;
        } else {
            LOG(logWARNING)
                << command
                << " is deprecated and will be removed. Please migrate to: "
                << d_it->second;
        }
        command = d_it->second;
        return true;
    }
    return false;
}

std::string Caller::list(int action) {
    if (action == defs::HELP_ACTION) {
        return "[deprecated(optional)]\n\tlists all available commands, list "
               "deprecated - list deprecated commands\n";
    }
    if (args.empty()) {
        std::string ret = "free\n";
        for (auto &f : functions) {
            ret += f.first + "\n";
        }
        return ret;
    } else if (args.size() == 1) {
        if (args[0] == "deprecated") {
            std::ostringstream os;
            os << "The following " << deprecated_functions.size()
               << " commands are deprecated\n";
            const size_t field_width = 20;
            for (const auto &it : deprecated_functions) {
                os << std::right << std::setw(field_width) << it.first << " -> "
                   << it.second << '\n';
            }
            return os.str();
        } else {
            throw RuntimeError(
                "Could not decode argument. Possible options: deprecated");
        }
    } else {
        WrongNumberOfParameters(0);
        return "";
    }
}

/* Network Configuration (Detector<->Receiver) */

IpAddr Caller::getDstIpFromAuto() {
    std::string rxHostname =
        det->getRxHostname(std::vector<int>{det_id}).squash("none");
    // Hostname could be ip try to decode otherwise look up the hostname
    auto val = IpAddr{rxHostname};
    if (val == 0) {
        val = HostnameToIp(rxHostname.c_str());
    }
    return val;
}

IpAddr Caller::getSrcIpFromAuto() {
    if (det->getDetectorType().squash() == defs::GOTTHARD) {
        throw RuntimeError(
            "Cannot use 'auto' for udp_srcip for GotthardI Detector.");
    }
    std::string hostname =
        det->getHostname(std::vector<int>{det_id}).squash("none");
    // Hostname could be ip try to decode otherwise look up the hostname
    auto val = IpAddr{hostname};
    if (val == 0) {
        val = HostnameToIp(hostname.c_str());
    }
    return val;
}

UdpDestination Caller::getUdpEntry() {
    UdpDestination udpDestination{};
    udpDestination.entry = rx_id;

    for (auto it : args) {
        size_t pos = it.find('=');
        std::string key = it.substr(0, pos);
        std::string value = it.substr(pos + 1);
        if (key == "ip") {
            if (value == "auto") {
                auto val = getDstIpFromAuto();
                LOG(logINFO) << "Setting udp_dstip of detector " << det_id
                             << " to " << val;
                udpDestination.ip = val;
            } else {
                udpDestination.ip = IpAddr(value);
            }
        } else if (key == "ip2") {
            if (value == "auto") {
                auto val = getDstIpFromAuto();
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
void Caller::WrongNumberOfParameters(size_t expected) {
    if (expected == 0) {
        throw RuntimeError("Command " + cmd +
                           " expected no parameter/s but got " +
                           std::to_string(args.size()) + "\n");
    }
    throw RuntimeError("Command " + cmd + " expected (or >=) " +
                       std::to_string(expected) + " parameter/s but got " +
                       std::to_string(args.size()) + "\n");
}

void Caller::GetLevelAndUpdateArgIndex(int action,
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

std::string Caller::free(int action) {
    // This  function is purely for help, actual functionality is in the caller
    return "free\n\tFree detector shared memory\n";
}

std::string Caller::hostname(int action) {
    std::ostringstream os;
    if (action == defs::HELP_ACTION) {
        os << "\n\tFrees shared memory and sets hostname (or IP address) of "
              "all modules concatenated by +.\n\t Virtual servers can already "
              "use the port in hostname separated by ':' and ports incremented "
              "by 2 to accomodate the stop server as well. The row and column "
              "values in the udp/zmq header are affected by the order in this "
              "command and the detsize command. The modules are stacked row by "
              "row until they reach the y-axis limit set by detsize (if "
              "specified). Then, stacking continues in the next column and so "
              "on. This only affects row and column in udp/zmq header."
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

void Caller::EmptyDataCallBack(detectorData *data, uint64_t frameIndex,
                               uint32_t subFrameIndex, void *this_pointer) {
    LOG(logDEBUG) << "EmptyDataCallBack to start up zmq sockets";
}

std::string Caller::acquire(int action) {
    std::ostringstream os;
    if (action == defs::HELP_ACTION) {
        os << "\n\tAcquire the number of frames set up.\n\tBlocking command, "
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
        if (action == defs::READOUT_ZMQ_ACTION) {
            det->registerDataCallback(&(EmptyDataCallBack), this);
        }
        det->acquire();

        if (det->getUseReceiverFlag().squash(false)) {
            os << "\nAcquired ";
            os << det->getFramesCaught() << '\n';
        }
    }
    return os.str();
}
std::string Caller::versions(int action) {
    std::ostringstream os;
    if (action == defs::HELP_ACTION) {
        os << "\n\tPrint all versions and detector type" << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }

        std::string vType = "Unknown";
        std::string vFirmware = "Unknown";
        std::string vServer = "Unknown";
        std::string vKernel = "Unknown";
        std::string vHardware = "Unknown";
        bool eiger = false;
        std::string vBebFirmware = "Unknown";
        std::string vFeblFirmware = "Unknown";
        std::string vFebrFirmware = "Unknown";
        bool receiver = false;
        std::string vReceiver = "Unknown";

        std::string vRelease = det->getPackageVersion();
        std::string vClient = det->getClientVersion();

        if (det->size() != 0) {
            // shared memory has detectors
            vType = OutString(det->getDetectorType());
            eiger = (det->getDetectorType().squash() == defs::EIGER);
            receiver = det->getUseReceiverFlag().squash(false);
            if (receiver) {
                // cannot connect to receiver
                try {
                    vReceiver = OutString(
                        det->getReceiverVersion(std::vector<int>{det_id}));
                } catch (const std::exception &e) {
                }
            }
            // cannot connect to Detector
            try {
                auto firmwareVersion =
                    det->getFirmwareVersion(std::vector<int>{det_id});
                vFirmware = OutStringHex(firmwareVersion);
                vServer = OutString(
                    det->getDetectorServerVersion(std::vector<int>{det_id}));
                vKernel = OutString(
                    det->getKernelVersion({std::vector<int>{det_id}}));
                vHardware = OutString(
                    det->getHardwareVersion(std::vector<int>{det_id}));
                if (eiger) {
                    vBebFirmware = OutString(firmwareVersion);
                    vFeblFirmware = OutString(det->getFrontEndFirmwareVersion(
                        defs::FRONT_LEFT, std::vector<int>{det_id}));
                    vFebrFirmware = OutString(det->getFrontEndFirmwareVersion(
                        defs::FRONT_RIGHT, std::vector<int>{det_id}));
                }
            } catch (const std::exception &e) {
            }
        }

        os << "\nType            : " << vType
           << "\nRelease         : " << vRelease
           << "\nClient          : " << vClient;
        if (eiger) {
            os << "\nFirmware (Beb)  : " << vBebFirmware
               << "\nFirmware (Febl) : " << vFeblFirmware
               << "\nFirmware (Febr) : " << vFebrFirmware;
        } else {
            os << "\nFirmware        : " << vFirmware;
        }
        os << "\nServer          : " << vServer
           << "\nKernel          : " << vKernel
           << "\nHardware        : " << vHardware;
        if (receiver)
            os << "\nReceiver        : " << vReceiver;
        os << std::dec << '\n';

    } else if (action == defs::PUT_ACTION) {
        throw RuntimeError("cannot put");
    } else {
        throw RuntimeError("Unknown action");
    }
    return os.str();
}
std::string Caller::threshold(int action) {
    std::ostringstream os;
    if (action == defs::HELP_ACTION) {
        os << "[eV] [(optinal settings)"
              "\n\t[Eiger][Mythen3] Threshold in eV. It loads trim files from "
              "settingspath.";
        if (cmd == "thresholdnotb") {
            os << "Trimbits are not loaded.";
        }
        os << "\n\t" << cmd
           << " [eV1] [eV2] [eV3] [(optional settings)]"
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

std::string Caller::trimen(int action) {
    std::ostringstream os;
    if (action == defs::HELP_ACTION) {
        os << "[trim_ev1] [trim_Ev2 (optional)] [trim_ev3 (optional)] "
              "...\n\t[Eiger][Mythen3] list of trim energies, where "
              "corresponding default trim files exist in corresponding trim "
              "folders."
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
std::string Caller::badchannels(int action) {
    std::ostringstream os;
    if (action == defs::HELP_ACTION) {
        os << "[fname|none|0]\n\t[Gotthard2][Mythen3] Sets the bad channels "
              "(from file of bad channel numbers) to be masked out. None or 0 "
              "unsets all the badchannels.\n\t[Mythen3] Also does trimming"
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (args.size() != 1) {
            WrongNumberOfParameters(1);
        }
        det->getBadChannels(args[0], std::vector<int>{det_id});
        os << "successfully retrieved" << '\n';
    } else if (action == defs::PUT_ACTION) {
        bool parse = false;
        if (args.size() == 0) {
            WrongNumberOfParameters(1);
        } else if (args.size() == 1) {
            if (args[0] == "none" || args[0] == "0") {
                det->setBadChannels(std::vector<int>{},
                                    std::vector<int>{det_id});
            } else if (args[0].find(".") != std::string::npos) {
                det->setBadChannels(args[0], std::vector<int>{det_id});
            } else {
                parse = true;
            }
        }
        // parse multi args or single one with range or single value
        if (parse || args.size() > 1) {
            // get channels
            auto list = getChannelsFromStringList(args);
            det->setBadChannels(list, std::vector<int>{det_id});
        }
        os << "successfully loaded" << '\n';
    } else {
        throw RuntimeError("Unknown action");
    }
    return os.str();
}
std::string Caller::udp_srcip(int action) {
    std::ostringstream os;
    if (action == defs::HELP_ACTION) {
        os << "[x.x.x.x] or auto\n\tIp address of the detector (source) udp "
              "interface. Must be same subnet as destination udp "
              "ip.\n\t[Eiger] Set only for 10G. For 1G, detector will replace "
              "with its own DHCP IP address. \n\tOne can also set this to "
              "'auto' for 1 GbE data and virtual detectors. It will set to IP "
              "of detector. Not available for GotthardI"
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
            val = getSrcIpFromAuto();
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
std::string Caller::udp_srcip2(int action) {
    std::ostringstream os;
    if (action == defs::HELP_ACTION) {
        os << "[x.x.x.x] or auto\n\t[Jungfrau][Moench][Gotthard2] Ip address "
              "of the "
              "detector (source) udp interface 2. Must be same subnet as "
              "destination udp ip2.\n\t [Jungfrau][Moench] top half or inner "
              "interface\n\t [Gotthard2] veto debugging. \n\tOne can also set "
              "this to 'auto' for 1 GbE data and virtual detectors. It will "
              "set to IP of detector."
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
            val = getSrcIpFromAuto();
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
std::string Caller::udp_dstip(int action) {
    std::ostringstream os;
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
            auto val = getDstIpFromAuto();
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
std::string Caller::udp_dstip2(int action) {
    std::ostringstream os;
    if (action == defs::HELP_ACTION) {
        os << "[x.x.x.x] or auto\n\t[Jungfrau][Moench][Gotthard2] Ip address "
              "of the "
              "receiver (destination) udp interface 2. If 'auto' used, then ip "
              "is set to ip of rx_hostname.\n\t[Jungfrau][Moench] bottom half "
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
            auto val = getDstIpFromAuto();
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
std::string Caller::rx_hostname(int action) {
    std::ostringstream os;
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
std::string Caller::rx_zmqip(int action) {
    std::string helpMessage =
        "\n\t[deprecated] The receiver zmq socket (publisher) will "
        "listen to all interfaces ('tcp://0.0.0.0:[port]'to all interfaces "
        "(from v9.0.0). This command does nothing and will be removed "
        "(from v10.0.0). This change makes no difference to the user.\n";
    std::ostringstream os;
    if (action == defs::HELP_ACTION) {
        os << helpMessage << '\n';
    } else if (action == defs::GET_ACTION) {
        os << ZMQ_PUBLISHER_IP << '\n';
    } else if (action == defs::PUT_ACTION) {
        LOG(logWARNING) << helpMessage << '\n';
        os << ZMQ_PUBLISHER_IP << '\n';
    } else {
        throw RuntimeError("Unknown action");
    }
    return os.str();
}
std::string Caller::rx_roi(int action) {
    std::ostringstream os;
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
std::string Caller::ratecorr(int action) {
    std::ostringstream os;
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
std::string Caller::burstmode(int action) {
    std::ostringstream os;
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
std::string Caller::vetostream(int action) {
    std::ostringstream os;
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
std::string Caller::counters(int action) {
    std::ostringstream os;
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
std::string Caller::samples(int action) {
    std::ostringstream os;
    if (action == defs::HELP_ACTION) {
        os << "[n_samples]\n\t[Ctb][Xilinx_Ctb] Number of samples (analog, "
              "digitial and "
              "transceiver) expected.\n"
           << '\n';
    } else if (action == defs::GET_ACTION) {
        if (!args.empty()) {
            WrongNumberOfParameters(0);
        }
        auto a = det->getNumberOfAnalogSamples(std::vector<int>{det_id});
        // get also digital samples for ctb and compare with analog
        auto det_type = det->getDetectorType().squash(defs::GENERIC);
        if (det_type == defs::CHIPTESTBOARD ||
            det_type == defs::XILINX_CHIPTESTBOARD) {
            auto d = det->getNumberOfDigitalSamples(std::vector<int>{det_id});
            auto t =
                det->getNumberOfTransceiverSamples(std::vector<int>{det_id});
            int as = a.squash(-1);
            int ds = d.squash(-1);
            int ts = t.squash(-1);
            if (as == -1 || ds == -1 || ts == -1 || as != ds ||
                as != ts) { // check if a == d?
                throw RuntimeError(
                    "Different samples. Use asamples, dsamples or tsamples.");
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
        auto det_type = det->getDetectorType().squash(defs::GENERIC);
        if (det_type == defs::CHIPTESTBOARD ||
            det_type == defs::XILINX_CHIPTESTBOARD) {
            det->setNumberOfDigitalSamples(StringTo<int>(args[0]),
                                           std::vector<int>{det_id});
            det->setNumberOfTransceiverSamples(StringTo<int>(args[0]),
                                               std::vector<int>{det_id});
        }
        os << args.front() << '\n';
    } else {
        throw RuntimeError("Unknown action");
    }
    return os.str();
}
std::string Caller::slowadc(int action) {
    std::ostringstream os;
    if (action == defs::HELP_ACTION) {
        os << "[n_channel (0-7 for channel]\n\t[Ctb] Slow "
              "ADC channel in mV"
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
std::string Caller::rx_dbitlist(int action) {
    std::ostringstream os;
    if (action == defs::HELP_ACTION) {
        os << "[all] or [none] or [i0] [i1] [i2]... \n\t[Ctb] List of digital "
              "signal bits enabled and rearranged according to the signals "
              "(all samples of each signal is put together). If 'all' is used "
              "instead of a list, all digital bits (64) enabled. Each element "
              "in list can be 0 - 63 and must be non repetitive. The option "
              "'none' will still spit out all data as is from the detector, "
              "but without rearranging it. Please note that when using the "
              "receiver list, the data size will be bigger if the number of "
              "samples is not divisible by 8 as every signal bit is padded to "
              "the next byte when combining all the samples in the receiver."
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
        }
        // 'none' option already covered as t is empty by default
        else if (args[0] != "none") {
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
std::string Caller::rx_jsonaddheader(int action) {
    std::ostringstream os;
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
std::string Caller::execcommand(int action) {
    std::ostringstream os;
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
std::string Caller::dacvalues(int action) {
    std::ostringstream os;
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
std::string Caller::currentsource(int action) {
    std::ostringstream os;
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
std::string Caller::gaincaps(int action) {
    std::ostringstream os;
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
std::string Caller::sleep(int action) {
    std::ostringstream os;
    if (action == defs::HELP_ACTION) {
        os << "[duration] [(optional unit) ns|us|ms|s]\n\tSleep for duration. "
              "Mainly for config files for firmware developers."
              "Default unit is s."
           << '\n';
    } else if (action == defs::GET_ACTION) {
        throw RuntimeError("Cannot get.");
    } else if (action == defs::PUT_ACTION) {
        if (args.size() != 1 && args.size() != 2) {
            WrongNumberOfParameters(1);
        }
        time::ns converted_time{0};
        try {
            if (args.size() == 1) {
                std::string tmp_time(args[0]);
                std::string unit = RemoveUnit(tmp_time);
                converted_time = StringTo<time::ns>(tmp_time, unit);
            } else {
                converted_time = StringTo<time::ns>(args[0], args[1]);
            }
        } catch (...) {
            throw RuntimeError("Could not convert argument to time::ns");
        }
        std::this_thread::sleep_for(converted_time);
        os << "for " << ToString(converted_time) << " completed" << '\n';
    } else {
        throw RuntimeError("Unknown action");
    }
    return os.str();
}

} // namespace sls