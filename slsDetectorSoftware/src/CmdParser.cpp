// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "CmdParser.h"
#include "sls/string_utils.h"

#include <algorithm>
#include <iterator>
#include <sstream>

namespace sls {

void CmdParser::Parse(int argc, const char *const argv[]) {
    Reset();
    executable_ = argv[0]; // first arg is calling binary
    if (argc > 1) {
        std::string s = argv[1];
        for (int i = 2; i < argc; ++i) {
            s += " ";
            s += argv[i];
        }
        Parse(s);
    }
}

void CmdParser::Parse(std::string s) {
    // taking s by value we can modify it.
    Reset();

    // Are we looking at -h --help? avoid removing h from command starting
    // with h when combined with detector id (ex, 1-hostname)
    bool h = replace_first(&s, "--help", " ");
    h = h || replace_first(&s, " -h", " ");
    h = h || replace_first(&s, "-h ", " ");
    help_ = h;

    // Extract the position indicies
    auto pos = s.find_first_not_of("0123456789:- ");
    if (pos != 0) {
        auto pre = s.substr(0, pos);
        pre.erase(std::remove(pre.begin(), pre.end(), ' '), pre.end());
        s.erase(0, pos);
        DecodeIdAndPosition(pre.c_str());
    }

    // Command and args should now be all that's left in the string
    std::istringstream iss(s);
    auto it = std::istream_iterator<std::string>(iss);
    command_ = *it++; // First arg is the comand to run

    arguments_ =
        std::vector<std::string>(it, std::istream_iterator<std::string>());

    // allow comma sep
    for (auto &arg : arguments_) {
        if (arg.back() == ',')
            arg.pop_back();
    }
}

void CmdParser::DecodeIdAndPosition(std::string pre) {
    if (pre.empty())
        return;

    // Get the detector id
    auto pos = pre.find_first_of("-");
    if (pos != std::string::npos) {
        multi_id_ = std::stoi(pre);
        pre.erase(0, pos + 1);
    }

    // if there is nothing left to parse we need to return
    if (pre.empty()) {
        return;
    }

    // now lets see if there is a :
    pos = pre.find_first_of(":");
    if (pos == std::string::npos) {
        // no : we only have the multi id
        detector_id_ = std::stoi(pre);

    } else if (pos == 0) {
        // do nothing, there is no multi id specified
        pre.erase(0, 1);
    } else {
        // the : is somewhere in the middle
        detector_id_ = std::stoi(pre);
        pre.erase(0, pos + 1);
    }

    // now if the string is not empty we also have a receiver id
    if (!pre.empty()) {
        receiver_id_ = std::stoi(pre);
    }
}

void CmdParser::Reset() {
    multi_id_ = 0;
    detector_id_ = -1;
    receiver_id_ = -1;
    help_ = false;
    command_.clear();
    executable_.clear();
    arguments_.clear();
}

} // namespace sls