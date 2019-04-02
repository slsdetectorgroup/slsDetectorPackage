#pragma once
#include <iostream>
#include <string>

namespace sls {

uint32_t HostnameToIp(const char *hostname);

class IpAddr {
  private:
    uint32_t addr_{0};

  public:
    constexpr IpAddr(uint32_t address) noexcept: addr_{address} {}
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
    constexpr MacAddr(uint64_t mac) noexcept : addr_{mac} {}
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
