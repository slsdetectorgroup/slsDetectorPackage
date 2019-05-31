#include "sls_detector_exceptions.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <sys/prctl.h> 
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>
#include <net/if.h>

#include "network_utils.h"

namespace sls {


IpAddr::IpAddr(const std::string &address) {
    inet_pton(AF_INET, address.c_str(), &addr_);
}

IpAddr::IpAddr(const char *address) { inet_pton(AF_INET, address, &addr_); }

std::string IpAddr::str() const {
    char ipstring[INET_ADDRSTRLEN]{};
    inet_ntop(AF_INET, &addr_, ipstring, INET_ADDRSTRLEN);
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

std::ostream &operator<<(std::ostream &out, const IpAddr &addr) {
    return out << addr.str();
}

std::ostream &operator<<(std::ostream &out, const MacAddr &addr) {
    return out << addr.str();
}

uint32_t HostnameToIp(const char *hostname) {
    addrinfo hints;
    addrinfo *result = nullptr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(hostname, nullptr, &hints, &result)) {
        freeaddrinfo(result);
        throw RuntimeError("Could not convert hostname to ip");
    }
    uint32_t ip = ((sockaddr_in *)result->ai_addr)->sin_addr.s_addr;
    freeaddrinfo(result);
    return ip;
}

std::string IpToInterfaceName(const std::string &ip) {
    //TODO! Copied from genericSocket needs to be refactored!
    struct ifaddrs *addrs, *iap;
    struct sockaddr_in *sa;

    char buf[32];
    const int buf_len = sizeof(buf);
    memset(buf, 0, buf_len);
    strcpy(buf, "none");

    getifaddrs(&addrs);
    for (iap = addrs; iap != NULL; iap = iap->ifa_next) {
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

MacAddr InterfaceNameToMac(std::string inf) {
        //TODO! Copied from genericSocket needs to be refactored!
		struct ifreq ifr;
		char mac[32];
		const int mac_len = sizeof(mac);
		memset(mac,0,mac_len);

		int sock=socket(PF_INET, SOCK_STREAM, 0);
		strncpy(ifr.ifr_name,inf.c_str(),sizeof(ifr.ifr_name)-1);
		ifr.ifr_name[sizeof(ifr.ifr_name)-1]='\0';


		if (-1==ioctl(sock, SIOCGIFHWADDR, &ifr)) {
			perror("ioctl(SIOCGIFHWADDR) ");
			return std::string("00:00:00:00:00:00");
		}
		for (int j=0, k=0; j<6; j++) {
			k+=snprintf(mac+k, mac_len-k-1, j ? ":%02X" : "%02X",
					(int)(unsigned int)(unsigned char)ifr.ifr_hwaddr.sa_data[j]);
		}
		mac[mac_len-1]='\0';

		if(sock!=1){
			close(sock);
		}
		return MacAddr(mac);

	}

} // namespace sls
