#pragma once

#include <string>

namespace sls {
std::string md5_calculate_checksum(char *buffer, ssize_t bytes);
} // namespace sls