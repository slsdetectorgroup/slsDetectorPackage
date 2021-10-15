// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/md5_helper.h"

#include "sls/md5.h"

#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace sls {

std::string md5_calculate_checksum(char *buffer, ssize_t bytes) {
    MD5_CTX c;
    if (!MD5_Init_SLS(&c)) {
        throw std::runtime_error(
            "Could not calculate md5 checksum.[initializing]");
    }
    if (!MD5_Update_SLS(&c, buffer, bytes)) {
        throw std::runtime_error("Could not calculate md5 checksum.[Updating]");
    }
    unsigned char out[MD5_DIGEST_LENGTH];
    if (!MD5_Final_SLS(out, &c)) {
        throw std::runtime_error("Could not calculate md5 checksum.[Final]");
    }
    std::ostringstream oss;
    for (int i = 0; i != MD5_DIGEST_LENGTH; ++i)
        oss << std::hex << std::setw(2) << std::setfill('0') << +out[i];
    return oss.str();
}

} // namespace sls
