// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "sls/sls_detector_defs.h"

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

namespace sls {


std::vector<char> readBinaryFile(const std::string &fname,
                                 const std::string &errorPrefix);

// mkdir -p path implemented by recursive calls
void mkdir_p(const std::string &path, std::string dir = "");

int getFileSize(std::ifstream &ifs);
ssize_t getFileSize(FILE *fd, const std::string &prependErrorString);

std::string getFileNameFromFilePath(const std::string &fpath);

std::vector<int> getChannelsFromStringList(const std::vector<std::string> list);

/** File can have # for comments.
 * Channels can be separated by spaces, commas
 * and ranges provided using ':', eg. 23:29
 * */
std::vector<int> getChannelsFromFile(const std::string &fname);

std::string getAbsolutePathFromCurrentProcess(const std::string &fname);

} // namespace sls
