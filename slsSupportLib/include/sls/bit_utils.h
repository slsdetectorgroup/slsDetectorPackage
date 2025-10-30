// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <bitset>
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
    int addr_{0};

  public:
    constexpr RegisterAddress() noexcept = default;
    explicit RegisterAddress(int address);
    explicit RegisterAddress(const std::string &address);

    std::string str() const;
    operator int() const noexcept { return addr_; }

    constexpr bool operator==(const RegisterAddress &other) const {
        return (addr_ == other.addr_);
    }
    constexpr bool operator!=(const RegisterAddress &other) const {
        return (addr_ != other.addr_);
    }
} __attribute__((packed));

class BitPosition {
  private:
    RegisterAddress addr_{0};
    int bitPos_{0};

  public:
    constexpr BitPosition() noexcept = default;
    BitPosition(RegisterAddress address, int bitPosition);
    std::string str() const;

    RegisterAddress address() const noexcept { return addr_; }
    int bitPosition() const noexcept { return bitPos_; }
    void setAddress(RegisterAddress address) noexcept { addr_ = address; }
    void setBitPosition(int bitPosition) noexcept { bitPos_ = bitPosition; }

    constexpr bool operator==(const BitPosition &other) const {
        return (addr_ == other.addr_ && bitPos_ == other.bitPos_);
    }
    constexpr bool operator!=(const BitPosition &other) const {
        return !(*this == other && bitPos_ == other.bitPos_);
    }
} __attribute__((packed));

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
std::ostream &operator<<(std::ostream &os, const BitPosition &r);
std::ostream &operator<<(std::ostream &os, const RegisterValue &r);

} // namespace sls
