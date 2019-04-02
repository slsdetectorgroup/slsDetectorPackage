#pragma once
#include <iostream>
#include <string>

namespace sls {

std::string MacAddrToString(uint64_t mac);
uint64_t MacStringToUint(std::string mac);
uint32_t IpStringToUint(const char *ipstr);
std::string IpToString(uint32_t ip);
std::string IpToHexString(uint32_t ip);
uint32_t HostnameToIp(const char *hostname);

class IpAddr {
  private:
    uint32_t addr_{0};

  public:
    explicit IpAddr(uint32_t address);
    IpAddr(const std::string &address);
    std::string str() const;
    std::string hex() const;
    bool operator==(const IpAddr &other) const { return addr_ == other.addr_; }
    bool operator==(const uint32_t other) const { return addr_ == other; }
};

class MacAddr {
  private:
    uint64_t addr_{0};
    std::string to_hex(const char delimiter = 0);

  public:
    MacAddr(std::string mac);
    std::string str() { return to_hex(':'); }
    std::string hex() { return to_hex(); }
    bool operator==(const MacAddr &other) const { return addr_ == other.addr_; }
    bool operator==(const uint64_t other) const { return addr_ == other; }
};

std::ostream &operator<<(std::ostream &out, const IpAddr &addr){
    return out << addr.str();
}

} // namespace sls
