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
    IpAddr(uint32_t address);
    IpAddr(const std::string &address);
    IpAddr(const char *address);
    std::string str() const;
    std::string hex() const;
    bool operator==(const IpAddr &other) const { return addr_ == other.addr_; }
    bool operator!=(const IpAddr &other) const { return addr_ != other.addr_; }
    bool operator==(const uint32_t other) const { return addr_ == other; }
    bool operator!=(const uint32_t other) const { return addr_ != other; }
};

class MacAddr {
  private:
    uint64_t addr_{0};
    std::string to_hex(const char delimiter = 0) const;

  public:
    MacAddr(uint64_t mac);
    MacAddr(std::string mac);
    MacAddr(const char *address);
    std::string str() const { return to_hex(':'); }
    std::string hex() const { return to_hex(); }
    bool operator==(const MacAddr &other) const { return addr_ == other.addr_; }
    bool operator!=(const MacAddr &other) const { return addr_ != other.addr_; }
    bool operator==(const uint64_t other) const { return addr_ == other; }
    bool operator!=(const uint64_t other) const { return addr_ != other; }
};

std::ostream &operator<<(std::ostream &out, const IpAddr &addr);
std::ostream &operator<<(std::ostream &out, const MacAddr &addr);

} // namespace sls
