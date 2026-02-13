#include "CommandLineOptions.h"
#include "sls/ToString.h"
#include "sls/sls_detector_exceptions.h"
#include "sls/versionAPI.h"

#include <cstdint>
#include <fmt/format.h>
#include <getopt.h>
#include <iostream>

namespace sls {

std::string CommandLineOptions::getVersion() const {
    return fmt::format(
        "MatterhornServer Version: {}",
        APIMATTERHORN); // TODO check that it is updated correctly !!!
}

uint16_t CommandLineOptions::parsePort(const char *optarg) const {
    uint16_t val = 0; // TODO: in c code its unsigned int

    try {
        val = sls::StringTo<uint16_t>(optarg);
    } catch (...) {
        throw("Could not parse port number " + std::string(optarg));
    }

    if (val == std::numeric_limits<uint16_t>::max()) {
        throw sls::RuntimeError("Cannot parse stop server port number. "
                                "Value must be in range 0 - 65535.");
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
        "\t-v, --version            : Software version\n"
        "\t-p, --port <port>        : TCP communication port with client. "
        "\n"
        "\t-d, --devel              : Developer mode. Skips firmware "
        "checks. \n"
        "\t-u, --update             : Update mode. Skips firmware checks "
        "and "
        "initial detector setup. \n",
        executable);
    return helpmessage;
}

DetectorServerOptions CommandLineOptions::parse(int argc, char *argv[]) {

    int opt, option_index = 0;

    DetectorServerOptions serverOptionsValues{};

    while ((opt = getopt_long(argc, argv, optstring.c_str(), options.data(),
                              &option_index)) != -1) {
        switch (opt) {
        case 'h':
            std::cout << getHelpMessage(argv[0]) << std::endl;
            serverOptionsValues.helpRequested = true; // to exit in main
            break;
        case 'v':
            serverOptionsValues.versionRequested = true; // to exit in main
            std::cout << getVersion() << std::endl;
            break;
        case 'p':
            serverOptionsValues.port = parsePort(optarg);
            break;
        case 'd':
            serverOptionsValues.debugflag = true;
            break;
        case 'u':
            serverOptionsValues.updateFlag = true;
            break;
        default:
            std::cout << getHelpMessage(argv[0]) << std::endl;
            throw std::runtime_error("Wrong command line arguments.");
        }
    }

    return serverOptionsValues;
}

} // namespace sls
