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

namespace sls {

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
    char ipstring[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &ip, ipstring, INET_ADDRSTRLEN) == nullptr) {
        // handle error
    }
    // TODO! Check return
    return ipstring;
}

uint32_t HostnameToIp(const char *const hostname) {
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

// char ipstring[INET_ADDRSTRLEN];
//     inet_ntop(AF_INET, &detector_shm()->detectorIP, ipstring, INET_ADDRSTRLEN);

// inet_pton(AF_INET, DEFAULT_DET_IP, &(detector_shm()->detectorIP));