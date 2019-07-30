#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
// #include <algorithm>
#include "CmdProxy.h"
#include "Detector.h"
#include "sls_detector_defs.h"

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

    std::cout << "Generating command line documentation!!!!!!!!!!!!!!!!\n";

    sls::CmdProxy<sls::Detector> proxy(nullptr);
    auto commands = proxy.GetProxyCommands();

    std::ofstream fs("commands.rst");
    fs << ".. glossary::\n";

    // std::vector<std::string> extra{"exptime2", "period2", "subexptime2"};
    for (const auto &cmd : commands) {
        std::ostringstream os;
        proxy.Call(cmd, {}, -1, slsDetectorDefs::HELP_ACTION, os);

        auto tmp = os.str().erase(0, cmd.size());
        auto usage = tmp.substr(0, tmp.find_first_of('\n'));
        tmp.erase(0, usage.size());
        auto help = replace_all(tmp, "\n\t", "\n\t\t");
        fs << '\t' << cmd << usage << help << "\n";
    }

    // for (const auto& cmd : commands)
    //     fs << '\t' << cmd << '\n';
}