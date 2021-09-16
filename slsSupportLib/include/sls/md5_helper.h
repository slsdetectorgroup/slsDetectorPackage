#pragma once

#include "sls/md5.h"

#include <string>

namespace sls {
std::string md5_calculate_checksum(char *buffer, ssize_t bytes);
} // namespace sls