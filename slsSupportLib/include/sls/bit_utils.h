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
    uint32_t value_{0};

  public:
    constexpr RegisterAddress() noexcept = default;
    constexpr explicit RegisterAddress(uint32_t value) : value_(value) {}
    std::string str() const;
    constexpr uint32_t value() const noexcept { return value_; }

    constexpr bool operator==(const RegisterAddress &other) const {
        return (value_ == other.value_);
    }
    constexpr bool operator!=(const RegisterAddress &other) const {
        return (value_ != other.value_);
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
    constexpr RegisterAddress address() const noexcept { return addr_; }
    constexpr uint32_t bitPosition() const noexcept { return bitPos_; }

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
    std::string str() const;
    constexpr uint32_t value() const noexcept { return value_; }

    constexpr RegisterValue &operator|=(const RegisterValue &rhs) noexcept {
        value_ |= rhs.value();
        return *this;
    }

    constexpr RegisterValue operator|(const RegisterValue &rhs) const noexcept {
        RegisterValue tmp(*this);
        tmp |= rhs;
        return tmp;
    }

    constexpr RegisterValue &operator|=(uint32_t rhs) noexcept {
        value_ |= rhs;
        return *this;
    }

    constexpr RegisterValue operator|(uint32_t rhs) const noexcept {
        RegisterValue tmp(*this);
        tmp |= rhs;
        return tmp;
    }

    constexpr bool operator==(const RegisterValue &other) const noexcept {
        return value_ == other.value_;
    }
    constexpr bool operator!=(const RegisterValue &other) const noexcept {
        return value_ != other.value_;
    }
};

std::ostream &operator<<(std::ostream &os, const RegisterAddress &r);
std::ostream &operator<<(std::ostream &os, const BitAddress &r);
std::ostream &operator<<(std::ostream &os, const RegisterValue &r);

} // namespace sls
