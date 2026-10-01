// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/network_utils.h"
#include "sls/sls_detector_exceptions.h"

#include <algorithm>
#include <arpa/inet.h>
#include <cassert>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <ifaddrs.h>
#include <iomanip>
#include <limits>
#include <net/if.h>
#include <netdb.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef __APPLE__
#include <net/if_dl.h> // sockaddr_dl, LLADDR
#else
#include <netpacket/packet.h> // sockaddr_ll
#endif

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
    // Single getifaddrs()-based implementation for both Linux and macOS.
    // The link-layer address is found on:
    //   - Linux:       AF_PACKET entries (sockaddr_ll, sll_addr / sll_halen)
    //   - macOS / BSD: AF_LINK   entries (sockaddr_dl, LLADDR / sdl_alen)
    struct ifaddrs *ifaddr = nullptr;
    if (getifaddrs(&ifaddr) == -1) {
        return MacAddr{};
    }
    MacAddr result{};
    for (struct ifaddrs *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr)
            continue;
        if (inf != ifa->ifa_name)
            continue;

#ifdef __APPLE__
        if (ifa->ifa_addr->sa_family != AF_LINK)
            continue;
        auto *sdl = reinterpret_cast<struct sockaddr_dl *>(ifa->ifa_addr);
        if (sdl->sdl_alen != 6)
            continue;
        const auto *p = reinterpret_cast<const unsigned char *>(LLADDR(sdl));
#else
        if (ifa->ifa_addr->sa_family != AF_PACKET)
            continue;
        auto *sll = reinterpret_cast<struct sockaddr_ll *>(ifa->ifa_addr);
        if (sll->sll_halen != 6)
            continue;
        const auto *p = sll->sll_addr;
#endif

        char mac[18];
        snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X", p[0], p[1],
                 p[2], p[3], p[4], p[5]);
        result = MacAddr(mac);
        break;
    }
    freeifaddrs(ifaddr);
    return result;
}

void validatePortNumber(uint16_t port) {
    if (port < 1024 || port > std::numeric_limits<uint16_t>::max()) {
        throw RuntimeError(std::string("Invalid port number ") +
                           std::to_string(port) +
                           ". Must be between 1024 - 65535.");
    }
}

void validatePortRange(uint16_t startPort, int numPorts) {
    validatePortNumber(startPort);
    validatePortNumber(startPort + numPorts - 1);
}

void setupSignalHandler(int signal, void (*handler)(int)) {
    // Catch signal SIGINT to close files and call destructors properly
    struct sigaction sa {};
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask); // dont block additional signals
    sa.sa_flags = 0;
    if (sigaction(signal, &sa, nullptr) == -1) {
        throw RuntimeError("Could not set handler for " +
                           std::string(strsignal(signal)));
    }
}
} // namespace sls
