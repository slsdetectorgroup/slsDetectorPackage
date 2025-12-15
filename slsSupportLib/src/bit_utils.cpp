// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/bit_utils.h"
#include "sls/ToString.h"
#include "sls/sls_detector_exceptions.h"

namespace sls {

RegisterAddress::RegisterAddress(const std::string &address) {
    if (!is_hex_or_dec_uint(address)) {
        throw RuntimeError("Address must be an integer value.");
    }
    uint32_t addr = StringTo<uint32_t>(address);
    addr_ = addr;
}

std::string RegisterAddress::str() const { return ToStringHex(addr_); }

BitAddress::BitAddress(RegisterAddress address, uint32_t bitPosition)
    : addr_(address) {
    if (bitPosition > 31) {
        throw RuntimeError("Bit position must be between 0 and 31.");
    }
    bitPos_ = bitPosition;
}

BitAddress::BitAddress(const std::string &address,
                       const std::string &bitPosition) {
    addr_ = RegisterAddress(address);
    if (!is_hex_or_dec_uint(bitPosition)) {
        throw RuntimeError("Bit position must be an integer value.");
    }
    uint32_t bitPos = StringTo<uint32_t>(bitPosition);
    if (bitPos > 31) {
        throw RuntimeError("Bit position must be between 0 and 31.");
    }
    bitPos_ = bitPos;
}

std::string BitAddress::str() const {
    std::ostringstream os;
    os << '[' << addr_.str() << ", " << ToString(bitPos_) << ']';
    return os.str();
}

RegisterValue::RegisterValue(const std::string &value) {
    if (!is_hex_or_dec_uint(value)) {
        throw RuntimeError("Value must be an integer value.");
    }
    uint32_t val = StringTo<uint32_t>(value);
    value_ = val;
}

std::string RegisterValue::str() const { return ToStringHex(value_); }

std::ostream &operator<<(std::ostream &os, const RegisterAddress &r) {
    os << r.str();
    return os;
}

std::ostream &operator<<(std::ostream &os, const BitAddress &r) {
    os << r.str();
    return os;
}

std::ostream &operator<<(std::ostream &os, const RegisterValue &r) {
    os << r.str();
    return os;
}

} // namespace sls
