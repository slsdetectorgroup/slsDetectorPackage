#pragma once
#include <string>

namespace sls {

std::string MacAddrToString(uint64_t mac);
uint64_t MacStringToUint(std::string mac);

uint32_t IpStringToUint(const char *ipstr);

std::string IpToString(uint32_t ip);
std::string IpToHexString(uint32_t ip);

uint32_t HostnameToIp(const char *hostname);

} // namespace sls
