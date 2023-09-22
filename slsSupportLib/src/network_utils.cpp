// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/sls_detector_exceptions.h"

#include "sls/network_utils.h"
#include <algorithm>
#include <arpa/inet.h>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <ifaddrs.h>
#include <iomanip>
#include <limits>
#include <net/if.h>
#include <netdb.h>
#include <sstream>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace sls {

IpAddr::IpAddr(const std::string &address) {
    inet_pton(AF_INET, address.c_str(), &addr_);
}

IpAddr::IpAddr(const char *address) { inet_pton(AF_INET, address, &addr_); }

std::string IpAddr::str() const { return arr().data(); }

std::array<char, INET_ADDRSTRLEN> IpAddr::arr() const {
    std::array<char, INET_ADDRSTRLEN> ipstring{};
    inet_ntop(AF_INET, &addr_, ipstring.data(), INET_ADDRSTRLEN);
    return ipstring;
}

std::string IpAddr::hex() const {
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i != 4; ++i) {
        ss << std::setw(2) << ((addr_ >> i * 8) & 0xFF);
    }
    return ss.str();
}

MacAddr::MacAddr(std::string mac) {
    if ((mac.length() != 17) || (mac[2] != ':') || (mac[5] != ':') ||
        (mac[8] != ':') || (mac[11] != ':') || (mac[14] != ':')) {
        addr_ = 0;
    } else {
        mac.erase(std::remove(mac.begin(), mac.end(), ':'), mac.end());
        addr_ = std::strtoul(mac.c_str(), nullptr, 16);
    }
}
MacAddr::MacAddr(const char *address) : MacAddr(std::string(address)) {}

std::string MacAddr::to_hex(const char delimiter) const {
    std::ostringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(2);
    ss << ((addr_ >> 40) & 0xFF);
    for (int i = 32; i >= 0; i -= 8) {
        if (delimiter)
            ss << delimiter;
        ss << std::setw(2) << ((addr_ >> i) & 0xFF);
    }
    return ss.str();
}

std::string MacAddr::str() const { return to_hex(':'); }

std::string MacAddr::hex() const { return to_hex(); }

std::string UdpDestination::str() const {
    std::ostringstream oss;
    oss << '[' << std::endl
        << "entry " << entry << std::endl
        << "ip " << ip << std::endl
        << "mac " << mac << std::endl
        << "port " << port << std::endl;
    if (port2 != 0) {
        oss << "port2 " << port2 << std::endl;
    }
    if (ip2 != 0) {
        oss << "ip2 " << ip2 << std::endl;
    }
    if (mac2 != 0) {
        oss << "mac2 " << mac2 << std::endl;
    }
    oss << ']';
    return oss.str();
}

std::ostream &operator<<(std::ostream &out, const IpAddr &addr) {
    return out << addr.str();
}

std::ostream &operator<<(std::ostream &out, const MacAddr &addr) {
    return out << addr.str();
}

std::ostream &operator<<(std::ostream &out, const UdpDestination &dest) {
    return out << dest.str();
}

IpAddr HostnameToIp(const char *hostname) {
    addrinfo hints;
    addrinfo *result = nullptr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(hostname, nullptr, &hints, &result)) {
        freeaddrinfo(result);
        throw RuntimeError("Could not convert hostname (" +
                           std::string(hostname) + ") to ip");
    }
    uint32_t ip = ((sockaddr_in *)result->ai_addr)->sin_addr.s_addr;
    freeaddrinfo(result);
    return IpAddr(ip);
}

std::string IpToInterfaceName(const std::string &ip) {
    // TODO! Copied from genericSocket needs to be refactored!
    struct ifaddrs *addrs, *iap;
    struct sockaddr_in *sa;

    char buf[32];
    const int buf_len = sizeof(buf);
    memset(buf, 0, buf_len);
    strcpy(buf, "none");

    getifaddrs(&addrs);
    for (iap = addrs; iap != nullptr; iap = iap->ifa_next) {
        if (iap->ifa_addr && (iap->ifa_flags & IFF_UP) &&
            iap->ifa_addr->sa_family == AF_INET) {
            sa = (struct sockaddr_in *)(iap->ifa_addr);
            inet_ntop(iap->ifa_addr->sa_family, (void *)&(sa->sin_addr), buf,
                      buf_len);
            if (ip == std::string(buf)) {
                strcpy(buf, iap->ifa_name);
                break;
            }
        }
    }
    freeifaddrs(addrs);
    return std::string(buf);
}

IpAddr InterfaceNameToIp(const std::string &ifn) {
    struct ifaddrs *ifaddr, *ifa;
    // int family, s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        return {};
    }

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr)
            continue;

        auto s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host,
                             NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);

        if ((strcmp(ifa->ifa_name, ifn.c_str()) == 0) &&
            (ifa->ifa_addr->sa_family == AF_INET)) {
            if (s != 0) {
                return {};
            }
            break;
        }
    }

    freeifaddrs(ifaddr);
    return IpAddr{host};
}

MacAddr InterfaceNameToMac(const std::string &inf) {
    // TODO! Copied from genericSocket needs to be refactored!
    struct ifreq ifr;
    char mac[32];
    const int mac_len = sizeof(mac);
    memset(mac, 0, mac_len);

    int sock = socket(PF_INET, SOCK_STREAM, 0);
    strncpy(ifr.ifr_name, inf.c_str(), sizeof(ifr.ifr_name) - 1);
    ifr.ifr_name[sizeof(ifr.ifr_name) - 1] = '\0';

    if (-1 == ioctl(sock, SIOCGIFHWADDR, &ifr)) {
        perror("ioctl(SIOCGIFHWADDR) ");
        return MacAddr{};
    }
    for (int j = 0, k = 0; j < 6; j++) {
        k += snprintf(
            mac + k, mac_len - k - 1, j ? ":%02X" : "%02X",
            (int)(unsigned int)(unsigned char)ifr.ifr_hwaddr.sa_data[j]);
    }
    mac[mac_len - 1] = '\0';

    if (sock != 1) {
        close(sock);
    }
    return MacAddr(mac);
}

void validatePortNumber(uint16_t port) {
    // random local port. might work if internal = bad practise
    if (port == 0) {
        throw RuntimeError("Invalid port number. Must be between 1 - 65535.");
    }
}

void validatePortRange(uint16_t startPort, int numPorts) {
    validatePortNumber(startPort);
    if ((startPort + numPorts) > std::numeric_limits<uint16_t>::max()) {
        throw RuntimeError("Invalid port range. Must be between 1 - 65535.");
    }
}

} // namespace sls
