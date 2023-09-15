// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include <array>
#include <cstdint>
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
} __attribute__((packed));

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
} __attribute__((packed));

struct UdpDestination {
    uint32_t entry{};
    uint16_t port{};
    uint16_t port2{};
    IpAddr ip;
    IpAddr ip2;
    MacAddr mac;
    MacAddr mac2;
    std::string str() const;

    constexpr bool operator==(const UdpDestination &other) const {
        return ((entry == other.entry) && (port == other.port) &&
                (port2 == other.port2) && (ip == other.ip) &&
                (ip2 == other.ip2) && (mac == other.mac) &&
                (mac2 == other.mac2));
    }
} __attribute__((packed));

std::ostream &operator<<(std::ostream &out, const IpAddr &addr);
std::ostream &operator<<(std::ostream &out, const MacAddr &addr);
std::ostream &operator<<(std::ostream &out, const UdpDestination &dest);

IpAddr HostnameToIp(const char *hostname);
std::string IpToInterfaceName(const std::string &ip);
MacAddr InterfaceNameToMac(const std::string &inf);
IpAddr InterfaceNameToIp(const std::string &ifn);
void validatePortNumber(uint16_t port);
void validatePortRange(uint16_t startPort, int numPorts);
} // namespace sls
