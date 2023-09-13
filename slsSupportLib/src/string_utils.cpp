// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "sls/string_utils.h"
#include "sls/container_utils.h"
#include "sls/network_utils.h"

#include <algorithm>
#include <iomanip>
#include <sls/ToString.h>
#include <sstream>

namespace sls {

std::vector<std::string> split(const std::string &strToSplit, char delimeter) {
    std::stringstream ss(strToSplit);
    std::string item;
    std::vector<std::string> splittedStrings;
    while (std::getline(ss, item, delimeter)) {
        if (item.length() > 0) {
            splittedStrings.push_back(item);
        }
    }
    return splittedStrings;
}

std::string RemoveUnit(std::string &str) {
    auto it = str.begin();
    while (it != str.end()) {
        if (std::isalpha(*it))
            break;
        ++it;
    }
    auto pos = it - str.begin();
    auto unit = str.substr(pos);
    str.erase(it, end(str));
    return unit;
}

bool is_int(const std::string &s) {
    return !s.empty() && std::find_if(s.begin(), s.end(), [](unsigned char c) {
                             return !std::isdigit(c);
                         }) == s.end();
}

bool replace_first(std::string *s, const std::string &substr,
                   const std::string &repl) {
    auto pos = s->find(substr);
    if (pos != std::string::npos) {
        s->replace(pos, substr.size(), repl);
        return true;
    }
    return false;
}

std::pair<std::string, uint16_t> ParseHostPort(const std::string &s) {
    // TODO deal with to many :, port not there?
    // no port return hostname as is and port as 0
    std::string host;
    uint16_t port{0};
    auto res = split(s, ':');
    host = res[0];
    if (res.size() > 1) {
        port = StringTo<uint16_t>(res[1]);
    }
    return std::make_pair(host, port);
}

}; // namespace sls