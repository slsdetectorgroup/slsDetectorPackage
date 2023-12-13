// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include <string>
#include <vector>

namespace sls {

std::string GetHelpDac(std::string dac);

std::string GetHelpDacWrapper(const std::string &cmd,
                              const std::vector<std::string> &args);

} // namespace sls
