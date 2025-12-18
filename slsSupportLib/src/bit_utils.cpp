// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/bit_utils.h"
#include "sls/ToString.h"
#include "sls/sls_detector_exceptions.h"

namespace sls {

std::string RegisterAddress::str() const { return ToStringHex(value_); }

BitAddress::BitAddress(RegisterAddress address, uint32_t bitPosition)
    : addr_(address) {
    if (bitPosition > 31) {
        throw RuntimeError("Bit position must be between 0 and 31.");
    }
    bitPos_ = bitPosition;
}

std::string BitAddress::str() const {
    std::ostringstream os;
    os << '[' << addr_.str() << ", " << ToString(bitPos_) << ']';
    return os.str();
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
