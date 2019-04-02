#include "sls_detector_exceptions.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iomanip>
#include <sstream>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "network_utils.h"

namespace sls {

IpAddr::IpAddr(uint32_t address) : addr_{address} {}
IpAddr::IpAddr(const std::string &address) { inet_pton(AF_INET, address.c_str(), &addr_); }
std::string IpAddr::str() const {
    char ipstring[INET_ADDRSTRLEN]{};
    inet_ntop(AF_INET, &addr_, ipstring, INET_ADDRSTRLEN);
    return ipstring;
}
std::string IpAddr::hex() const {
    std::ostringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(2);
    for (int i = 0; i != 4; ++i) {
        ss << ((addr_ >> i * 8) & 0xFF);
    }
    return ss.str();
}

MacAddr::MacAddr(std::string mac) {
    if ((mac.length() != 17) || (mac[2] != ':') || (mac[5] != ':') || (mac[8] != ':') ||
        (mac[11] != ':') || (mac[14] != ':')) {
        addr_ = 0;
    }
    mac.erase(std::remove(mac.begin(), mac.end(), ':'), mac.end());
    addr_ = std::stol(mac, nullptr, 16);
}

std::string MacAddr::to_hex(const char delimiter) {
    std::ostringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(2);
    ss << ((addr_ >> 40) & 0xFF);
    for (int i = 32; i >= 0; i -= 8) {
        if (delimiter)
            ss << delimiter;
        ss << ((addr_ >> i) & 0xFF);
    }
    return ss.str();
}

std::string MacAddrToString(uint64_t mac) {
    std::ostringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(2);
    ss << ((mac >> 40) & 0xFF);
    for (int i = 32; i >= 0; i -= 8) {
        ss << ':' << ((mac >> i) & 0xFF);
    }
    return ss.str();
}

uint64_t MacStringToUint(std::string mac) {
    if ((mac.length() != 17) || (mac[2] != ':') || (mac[5] != ':') || (mac[8] != ':') ||
        (mac[11] != ':') || (mac[14] != ':')) {
        return 0;
    }
    mac.erase(std::remove(mac.begin(), mac.end(), ':'), mac.end());
    return std::stol(mac, nullptr, 16);
}

uint32_t IpStringToUint(const char *ipstr) {
    uint32_t ip{0};
    inet_pton(AF_INET, ipstr, &ip);
    return ip;
}

std::string IpToString(uint32_t ip) {
    char ipstring[INET_ADDRSTRLEN]{};
    inet_ntop(AF_INET, &ip, ipstring, INET_ADDRSTRLEN);
    return ipstring;
}

std::string IpToHexString(uint32_t ip) {
    std::ostringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(2);
    for (int i = 0; i != 4; ++i) {
        ss << ((ip >> i * 8) & 0xFF);
    }
    return ss.str();
}

uint32_t HostnameToIp(const char *hostname) {
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(hostname, NULL, &hints, &result)) {
        freeaddrinfo(result);
        throw RuntimeError("Could not convert hostname to ip");
    }
    uint32_t ip = ((struct sockaddr_in *)result->ai_addr)->sin_addr.s_addr;
    freeaddrinfo(result);
    return ip;
}

} // namespace sls
