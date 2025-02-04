// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
/**
 * Utility program to generate input files for the command line
 * documentation. Uses the string returned from sls_detector_help cmd
 *
 */
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "Caller.h"
#include "sls/Detector.h"
#include "sls/sls_detector_defs.h"

std::string replace_all(const std::string &src, const std::string &from,
                        const std::string &to) {

    std::string results;
    std::string::const_iterator end = src.end();
    std::string::const_iterator current = src.begin();
    std::string::const_iterator next =
        std::search(current, end, from.begin(), from.end());
    while (next != end) {
        results.append(current, next);
        results.append(to);
        current = next + from.size();
        next = std::search(current, end, from.begin(), from.end());
    }
    results.append(current, next);
    return results;
}

int main() {

    std::cout << "Generating command line documentation!\n";

    sls::Caller caller(nullptr);
    auto commands = caller.getAllCommands();

    std::ofstream fs("commands.rst");
    fs << ".. glossary::\n";

    for (const auto &cmd : commands) {
        std::ostringstream os;
        std::cout << cmd << '\n';
        caller.call(cmd, {}, -1, slsDetectorDefs::HELP_ACTION, os);

        auto tmp = os.str().erase(0, cmd.size());
        auto usage = tmp.substr(0, tmp.find_first_of('\n'));
        tmp.erase(0, usage.size());
        auto help = replace_all(tmp, "\n\t", "\n\t\t| ");
        fs << '\t' << cmd << usage << help << "\n";
    }

    std::ofstream fs2("deprecated.csv");
    fs2 << "Old, New\n";
    auto cmds = caller.GetDeprecatedCommands();
    for (auto it : cmds) {
        fs2 << it.first << ", " << it.second << '\n';
    }
}
