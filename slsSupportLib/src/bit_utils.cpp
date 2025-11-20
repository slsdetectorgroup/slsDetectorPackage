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

BitPosition::BitPosition(RegisterAddress address, int bitPosition)
    : addr_(address), bitPos_(bitPosition) {
    if (bitPosition < 0 || bitPosition > 31) {
        throw RuntimeError("Bit position must be between 0 and 31.");
    }
}

std::string BitPosition::str() const {
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

std::ostream &operator<<(std::ostream &os, const BitPosition &r) {
    os << r.str();
    return os;
}

std::ostream &operator<<(std::ostream &os, const RegisterValue &r) {
    os << r.str();
    return os;
}

} // namespace sls
