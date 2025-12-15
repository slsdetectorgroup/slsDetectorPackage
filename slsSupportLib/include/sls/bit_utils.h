// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <bitset>
#include <cstdint>
#include <vector>
namespace sls {
template <typename T> std::vector<int> getSetBits(T val) {
    constexpr size_t bitsPerByte = 8;
    constexpr size_t numBits = sizeof(T) * bitsPerByte;
    std::bitset<numBits> bs(val);
    std::vector<int> set_bits;
    set_bits.reserve(bs.count());
    for (size_t i = 0; i < bs.size(); ++i) {
        if (bs[i]) {
            set_bits.push_back(static_cast<int>(i));
        }
    }
    return set_bits;
}

class RegisterAddress {
  private:
    uint32_t addr_{0};

  public:
    constexpr RegisterAddress() noexcept = default;
    constexpr explicit RegisterAddress(uint32_t address) : addr_(address) {}
    explicit RegisterAddress(const std::string &address);

    std::string str() const;
    operator uint32_t() const noexcept { return addr_; }

    constexpr bool operator==(const RegisterAddress &other) const {
        return (addr_ == other.addr_);
    }
    constexpr bool operator!=(const RegisterAddress &other) const {
        return (addr_ != other.addr_);
    }
};

class BitAddress {
  private:
    RegisterAddress addr_{0};
    uint32_t bitPos_{0};

  public:
    constexpr BitAddress() noexcept = default;
    BitAddress(RegisterAddress address, uint32_t bitPosition);
    std::string str() const;

    RegisterAddress address() const noexcept { return addr_; }
    uint32_t bitPosition() const noexcept { return bitPos_; }

    void setBitPosition(uint32_t bitPos);
    void setBitPosition(const std::string &bitPos);

    void setAddress(RegisterAddress address) noexcept { addr_ = address; }
    void setAddress(const std::string &address);

    constexpr bool operator==(const BitAddress &other) const {
        return (addr_ == other.addr_ && bitPos_ == other.bitPos_);
    }
    constexpr bool operator!=(const BitAddress &other) const {
        return !(*this == other);
    }
};

class RegisterValue {
  private:
    uint32_t value_{0};

  public:
    constexpr RegisterValue() noexcept = default;
    explicit constexpr RegisterValue(uint32_t value) noexcept : value_(value) {}
    explicit RegisterValue(const std::string &value);

    std::string str() const;
    operator uint32_t() const noexcept { return value_; }

    RegisterValue &operator|=(uint32_t rhs) noexcept {
        value_ |= rhs;
        return *this;
    }
    constexpr bool operator==(const RegisterValue &other) const noexcept {
        return value_ == other.value_;
    }
    constexpr bool operator!=(const RegisterValue &other) const noexcept {
        return value_ != other.value_;
    }
} __attribute__((packed));

std::ostream &operator<<(std::ostream &os, const RegisterAddress &r);
std::ostream &operator<<(std::ostream &os, const BitAddress &r);
std::ostream &operator<<(std::ostream &os, const RegisterValue &r);

} // namespace sls
