#include "CommandLineOptions.hpp"
#include "sls/ToString.h"
#include "sls/sls_detector_exceptions.h"

#include <cstdint>
#include <fmt/format.h>
#include <getopt.h>
#include <iostream>

namespace sls {

uint16_t CommandLineOptions::parsePort(const char *optarg) const {
    uint16_t val = 0; // TODO: in c code its unsigned int

    try {
        val = sls::StringTo<uint16_t>(optarg);
    } catch (const std::exception &e) {
        throw sls::RuntimeError(fmt::format(
            "Could not parse port number {}. {}", optarg, e.what()));
    }

    if (val < 1024) {
        throw sls::RuntimeError(
            "Invalid/ privileged port number parsed. Min: 1024.");
    }
    return val;
}

std::string
CommandLineOptions::getHelpMessage(const std::string &executable) const {
    // TODO: update if we keep it Matterhonr specific - refactor a bit better -
    // e.g. if compiled with detector macro
    std::string helpmessage = fmt::format(
        "Usage: {}"
        " [arguments]\n"
        "Possible arguments are:\n"
        "\t-v, --version                 : Software version\n"
        "\t-p, --port <port>             : TCP communication port with client. "
        "\n"
        "\t-s, --safe_startup            : Safe startup mode. Skips initial "
        "detector setup and checks. \n"
        "\t-f, --ignore_fw_compatibility : Ignore firmware compatibility "
        "check. \n",
        executable);
    return helpmessage;
}

void CommandLineOptions::parse_deprecated(const int &opt, char *argv[]) {

    switch (opt) {
    case 'd':
        std::cout << "Warning: -d/--devel option is deprecated. Use "
                     "-l/--safe_startup instead."
                  << std::endl;
        detectorserveroptions.safeStartup = true;
        break;
    case 'u':
        std::cout << "Warning: -u/--update option is deprecated. Use "
                     "-f/--ignore_fw_compatibility instead."
                  << std::endl;
        detectorserveroptions.ignoreFirmwareCompatibility = true;
        break;
    default:
        throw std::runtime_error(fmt::format("Wrong command line arguments. {}",
                                             getHelpMessage(argv[0])));
    }
}

std::string CommandLineOptions::printOptions() const {
    std::string msg = "setting up detector server";

    if (detectorserveroptions.ignoreFirmwareCompatibility) {
        msg += " skipping firmware compatibility checks";
        msg += detectorserveroptions.safeStartup ? " and" : "";
    }
    if (detectorserveroptions.safeStartup) {
        msg += " in safe startup mode e.g. skipping any initial detector setup "
               "and checks";
    }

    return msg;
}

DetectorServerOptions CommandLineOptions::parse(int argc, char *argv[]) {

    int opt, option_index = 0;

    while ((opt = getopt_long(argc, argv, optstring, options.data(),
                              &option_index)) != -1) {
        switch (opt) {
        case 'h':
            std::cout << getHelpMessage(argv[0]) << std::endl;
            detectorserveroptions.helpRequested = true; // to exit in main
            break;
        case 'v':
            detectorserveroptions.versionRequested = true; // to exit in main
            break;
        case 'p':
            detectorserveroptions.port = parsePort(optarg);
            break;
        case 'f':
            detectorserveroptions.ignoreFirmwareCompatibility = true;
            break;
        case 's':
            detectorserveroptions.safeStartup = true;
            break;
        default:
            parse_deprecated(opt, argv); // to handle deprecated options and
                                         // throw error for wrong options
        }
    }

    return detectorserveroptions;
}

} // namespace sls
