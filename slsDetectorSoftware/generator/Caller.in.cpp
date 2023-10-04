#include "Caller.h"
#include <iostream>
#include "sls/string_utils.h"
#include "sls/logger.h"

namespace sls {

// enum { GET_ACTION, PUT_ACTION, READOUT_ACTION, HELP_ACTION };

void Caller::call(const CmdParser &parser, int action, std::ostream &os) {

  args = parser.arguments();
  cmd = parser.command();
  det_id = parser.detector_id();
  auto it = functions.find(parser.command());
  if (it != functions.end()) {
    os << ((*this).*(it->second))(action);
  } else {
    throw RuntimeError(parser.command() +
                       " Unknown command, use list to list all commands");
  }
}

std::string Caller::list(int action) {
  std::string ret="free\n";
  for (auto &f : functions) {
    ret += f.first + "\n";
  }

  return ret;
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



// THIS COMMENT IS GOING TO BE REPLACED BY THE ACTUAL CODE


}