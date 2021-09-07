#pragma once
#include <array>
#include <iostream>
#include <string>

namespace sls {

class IpAddr {
  private:
    uint32_t addr_{0};

  public:
    constexpr IpAddr() noexcept = default;
    explicit constexpr IpAddr(uint32_t address) noexcept : addr_{address} {}
    explicit IpAddr(const std::string &address);
    explicit IpAddr(const char *address);
    std::string str() const;
    std::string hex() const;
    std::array<char, 16u> arr() const;
    constexpr bool operator==(const IpAddr &other) const noexcept {
        return addr_ == other.addr_;
    }
    constexpr bool operator!=(const IpAddr &other) const noexcept {
        return addr_ != other.addr_;
    }
    constexpr bool operator==(const uint32_t other) const noexcept {
        return addr_ == other;
    }
    constexpr bool operator!=(const uint32_t other) const noexcept {
        return addr_ != other;
    }
    constexpr uint32_t uint32() const noexcept { return addr_; }
};

class MacAddr {
  private:
    uint64_t addr_{0};
    std::string to_hex(const char delimiter = 0) const;

  public:
    constexpr MacAddr() noexcept = default;
    explicit constexpr MacAddr(uint64_t mac) noexcept : addr_{mac} {}
    explicit MacAddr(std::string mac);
    explicit MacAddr(const char *address);
    std::string str() const;
    std::string hex() const;
    constexpr bool operator==(const MacAddr &other) const noexcept {
        return addr_ == other.addr_;
    }
    constexpr bool operator!=(const MacAddr &other) const noexcept {
        return addr_ != other.addr_;
    }
    constexpr bool operator==(const uint64_t other) const noexcept {
        return addr_ == other;
    }
    constexpr bool operator!=(const uint64_t other) const noexcept {
        return addr_ != other;
    }
    constexpr uint64_t uint64() const noexcept { return addr_; }
};

class UdpDestination {
  private:
    uint32_t entry_{};
    uint32_t port_{};
    uint32_t port2_{};
    uint32_t ip_{};
    uint32_t ip2_{};
    uint64_t mac_{};
    uint64_t mac2_{};

  public:
    constexpr UdpDestination() noexcept = default;
    explicit constexpr UdpDestination(uint32_t entry, uint32_t port = 0,
                                      IpAddr ip = {}, MacAddr mac = {},
                                      uint32_t port2 = 0, IpAddr ip2 = {},
                                      MacAddr mac2 = {})
        : entry_(entry), port_(port), port2_(port2), ip_(ip.uint32()),
          ip2_(ip2.uint32()), mac_(mac.uint64()), mac2_(mac2.uint64()) {}

    uint32_t getEntry() const noexcept { return entry_; }

    void setEntry(const uint32_t value) { entry_ = value; }

    uint32_t getPort() const noexcept { return port_; }

    void setPort(const uint32_t value) { port_ = value; }

    uint32_t getPort2() const noexcept { return port2_; }

    void setPort2(const uint32_t value) { port2_ = value; }

    IpAddr getIp() const noexcept { return IpAddr(ip_); }

    void setIp(const IpAddr value) { ip_ = value.uint32(); }

    IpAddr getIp2() const noexcept { return IpAddr(ip2_); }

    void setIp2(const IpAddr value) { ip2_ = value.uint32(); }

    MacAddr getMac() const noexcept { return MacAddr(mac_); }

    void setMac(const MacAddr value) { mac_ = value.uint64(); }

    MacAddr getMac2() const noexcept { return MacAddr(mac2_); }

    void setMac2(const MacAddr value) { mac2_ = value.uint64(); }

    std::string str() const;

    constexpr bool operator==(const UdpDestination &other) const {
        return ((entry_ == other.entry_) && (port_ == other.port_) &&
                (port2_ == other.port2_) && (ip_ == other.ip_) &&
                (ip2_ == other.ip2_) && (mac_ == other.mac_) &&
                (mac2_ == other.mac2_));
    }
} __attribute__((packed));

std::ostream &operator<<(std::ostream &out, const IpAddr &addr);
std::ostream &operator<<(std::ostream &out, const MacAddr &addr);
std::ostream &operator<<(std::ostream &out, const UdpDestination &dest);

IpAddr HostnameToIp(const char *hostname);
std::string IpToInterfaceName(const std::string &ip);
MacAddr InterfaceNameToMac(const std::string &inf);
IpAddr InterfaceNameToIp(const std::string &ifn);

} // namespace sls
