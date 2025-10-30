// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/bit_utils.h"
#include "sls/sls_detector_exceptions.h"
#include "sls/ToString.h"

namespace sls {

RegisterAddress::RegisterAddress(int address)  : addr_(address) {
    if (address < 0) {
        throw RuntimeError("Register address cannot be negative.");
    }
}

RegisterAddress::RegisterAddress(const std::string &address) {
    int addr = StringTo<int>(address);
    if (addr < 0) {
        throw RuntimeError("Register address cannot be negative.");
    }
    addr_ = addr;
}

std::string RegisterAddress::str() const {
    return ToStringHex(addr_);
}

BitPosition::BitPosition(RegisterAddress address, int bitPosition) : addr_(address), bitPos_(bitPosition) {
    if (bitPosition < 0 || bitPosition > 31) {
        throw RuntimeError("Bit position must be between 0 and 31.");
    }
}

std::string BitPosition::str() const {
    std::ostringstream os;
    os << '[' << addr_.str() << ',' << ToString(bitPos_) << ']';
    return os.str();
}

RegisterValue::RegisterValue(const std::string &value) {
    uint32_t val = StringTo<uint32_t>(value);
    value_ = val;
}

std::string RegisterValue::str() const {
    return ToStringHex(value_);
}

} // namespace sls
