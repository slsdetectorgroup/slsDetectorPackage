// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "CommandLineOptions.h"
#include "sls/sls_detector_defs.h"
#include "sls/versionAPI.h"
#include "sls/ToString.h"
#include "sls/logger.h"

#include <csignal>
#include <cstring>
#include <unistd.h>

ParsedOptions CommandLineOptions::parse(int argc, char *argv[]) {
    CommonOptions base;
    base.port = DEFAULT_TCP_RX_PORTNO;

    MultiReceiverOptions multi;
    FrameSyncOptions frame;

    uint16_t numReceivers = 1;
    bool optionalArg = false;

    auto optString = buildOptString();
    auto longOptions = buildOptionList();

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, optString.c_str(), longOptions.data(),
                              &option_index)) != -1) {
        switch (opt) {

        case 'v':
        case 'h':
        case 'p':
        case 'u':
            handleCommonOption(opt, optarg, base);
            break;

        case 'c':
        case 'n':
        case 't':
            handleAppSpecificOption(opt, optarg, base, multi, frame);
            break;

        default:
            throw sls::RuntimeError("Invalid arguments." + getHelpMessage());
        }
    }

    // remaining arguments
    if (optind < argc) {
        // maintain backward compatibility of [start port] [num receivers]
        // [optional arg] ( for multi receiver and frame synchronizer )
        if (appType_ != AppType::SingleReceiver &&
            GetDeprecated(argc, argv, base.port, numReceivers, optionalArg)) {
            if (appType_ == AppType::MultiReceiver) {
                multi.numReceivers = numReceivers;
                multi.callbackEnabled = optionalArg;
            } else if (appType_ == AppType::FrameSynchronizer) {
                frame.numReceivers = numReceivers;
                frame.printHeaders = optionalArg;
            }
        } else {
            throw sls::RuntimeError("Invalid arguments." + getHelpMessage());
        }
    }

    LOG(sls::logINFO) << "Number of receivers: " << numReceivers;
    LOG(sls::logINFO) << "TCP Port: " << base.port;
    switch (appType_) {
    case AppType::SingleReceiver:
        return base;
    case AppType::MultiReceiver:
        LOG(sls::logINFO) << "Call back enable: " << multi.callbackEnabled;
        static_cast<CommonOptions &>(multi) = base;
        return multi;
    case AppType::FrameSynchronizer:
        LOG(sls::logINFO) << "Print headers: " << frame.printHeaders;
        static_cast<CommonOptions &>(frame) = base;
        return frame;
    }

    throw std::logic_error("Unknown AppType");
}

std::vector<option> CommandLineOptions::buildOptionList() const {
    std::vector<option> opts = {
        {"version", no_argument, nullptr, 'v'},
        {"port", required_argument, nullptr, 'p'},
        {"uid", required_argument, nullptr, 'u'},
        {"help", no_argument, nullptr, 'h'},
    };

    switch (appType_) {
    case AppType::SingleReceiver:
        opts.push_back({"rx_tcpport", required_argument, nullptr, 't'});
        break;
    case AppType::MultiReceiver:
        opts.push_back({"callback", no_argument, nullptr, 'c'});
        opts.push_back({"rx_tcpport", required_argument, nullptr, 't'});
        opts.push_back({"num-receivers", required_argument, nullptr, 'n'});
        break;
    case AppType::FrameSynchronizer:
        opts.push_back({"num-receivers", required_argument, nullptr, 'n'});
        opts.push_back({"print-headers", no_argument, nullptr, 'c'});
        break;
    }

    opts.push_back({nullptr, 0, nullptr, 0}); // null-terminator for getopt
    return opts;
}

std::string CommandLineOptions::buildOptString() const {
    std::string optstr = "vp:u:h";
    if (appType_ == AppType::MultiReceiver ||
        appType_ == AppType::FrameSynchronizer)
        optstr += "cn:";
    if (appType_ == AppType::SingleReceiver)
        optstr += "t:";
    return optstr;
}

void CommandLineOptions::handleCommonOption(int opt, const char *optarg,
                                            CommonOptions &base) {
    switch (opt) {
    case 'v':
        base.versionRequested = true;
        std::cout << getVersion() << std::endl;
        break;
    case 'h':
        base.helpRequested = true;
        std::cout << getHelpMessage() << std::endl;
        break;
    case 'p':
        try {
            base.port = sls::StringTo<uint16_t>(optarg);
        } catch (...) {
            throw sls::RuntimeError("Invalid port number parsed.");
        }
        break;
    case 'u':
        try {
            base.userid = sls::StringTo<uid_t>(optarg);
            if (base.userid != static_cast<uid_t>(-1)) {
                setEffectiveUID(base.userid);
            }
        } catch (...) {
            throw sls::RuntimeError("Invalid uid parsed.");
        }
        break;
    }
}

void CommandLineOptions::handleAppSpecificOption(int opt, const char *optarg,
                                                 CommonOptions &base,
                                                 MultiReceiverOptions &multi,
                                                 FrameSyncOptions &frame) {
    switch (opt) {

    case 'c':
        if (appType_ == AppType::MultiReceiver)
            multi.callbackEnabled = true;
        else if (appType_ == AppType::FrameSynchronizer)
            frame.printHeaders = true;
        break;

    case 'n': {
        uint16_t val = 1;
        try {
            val = sls::StringTo<uint16_t>(optarg);
        } catch (...) {
            throw sls::RuntimeError("Invalid number of receivers parsed." +
                                    std::to_string(val));
        }
        if (val == 0 || val > MAX_RECEIVERS) {
            throw sls::RuntimeError(
                "Invalid number of receivers. Options: 1 - " +
                std::to_string(MAX_RECEIVERS));
        }
        if (appType_ == AppType::MultiReceiver)
            multi.numReceivers = val;
        else if (appType_ == AppType::FrameSynchronizer)
            frame.numReceivers = val;
        break;
    }

    case 't':
        LOG(sls::logWARNING) << "Deprecated option 't' and '--rx_tcport'. Use "
                                "'p' or '--port' instead.";
        try {
            base.port = sls::StringTo<uint16_t>(optarg);
        } catch (...) {
            throw sls::RuntimeError("Invalid port number parsed.");
        }
        break;
    }
}

int CommandLineOptions::GetDeprecated(int argc, char *argv[],
                                      uint16_t &startPort,
                                      uint16_t &numReceivers,
                                      bool &optionalArg) {
    std::string deprecatedMessage =
        "Detected deprecated Options. Please update.\n";
    if (argc > 1) {
        try {
            if (argc == 3 || argc == 4) {
                try {
                    startPort = sls::StringTo<uint16_t>(argv[1]);
                    numReceivers = sls::StringTo<uint16_t>(argv[2]);
                } catch (...) {
                    throw sls::RuntimeError(
                        "Invalid port number or number of receivers parsed.");
                }
                if (numReceivers > MAX_RECEIVERS) {
                    LOG(sls::logERROR)
                        << "Did you mix up the order of the arguments? Max "
                           "number of recievers: " << MAX_RECEIVERS;
                    return 0;
                }
                if (numReceivers == 0) {
                    LOG(sls::logERROR)
                        << "Invalid number of receivers. Options:1 - "
                        << MAX_RECEIVERS;
                    return slsDetectorDefs::FAIL;
                }
                if (argc == 4) {
                    try {
                        optionalArg = sls::StringTo<bool>(argv[3]);
                    } catch (...) {
                        throw sls::RuntimeError("Invalid optional argument "
                                                "parsed. Expected 1 (true) or "
                                                "0 (false).");
                    }
                }
            } else
                throw std::runtime_error("Invalid number of arguments");
        } catch (const std::exception &e) {
            LOG(sls::logWARNING) << deprecatedMessage;
            LOG(sls::logERROR) << e.what();
            return 0;
        }
    }
    return 1;
}

std::string CommandLineOptions::getTypeString() {
    switch (appType_) {
    case AppType::SingleReceiver:
        return "SingleReceiver";
    case AppType::MultiReceiver:
        return "MultiReceiver";
    case AppType::FrameSynchronizer:
        return "FrameSynchronizer";
    default:
        return "Unknown";
    }
}

std::string CommandLineOptions::getVersion() {
    return getTypeString() + " Version: " + APIRECEIVER;
}

std::string CommandLineOptions::getHelpMessage() {
    switch (appType_) {
    case AppType::SingleReceiver:
        return std::string("\nUsage: ") + getTypeString() + " Options:\n" +
               "\t-v, --version       : Version.\n" +
               "\t-p, --port          : TCP port to communicate with client "
               "for "
               "configuration. Non-zero and 16 bit.\n" +
               "\t-u, --uid           : Set effective user id if receiver "
               "started "
               "with privileges. \n\n";

    case AppType::MultiReceiver:
        return std::string("\nUsage: " + getTypeString() + " Options:\n") +
               "\t-v, --version       : Version.\n" +
               "\t-n, --num-receivers : Number of receivers.\n" +
               "\t-p, --port          : TCP port to communicate with client "
               "for "
               "configuration. Non-zero and 16 bit.\n" +
               "\t-c, --callback      : Enable dummy callbacks for debugging. "
               "Disabled by default. \n" +
               "\t-u, --uid           : Set effective user id if receiver "
               "started "
               "with privileges. \n\n";

    case AppType::FrameSynchronizer:
        return std::string("\nUsage: " + getTypeString() + " Options:\n") +
               "\t-v, --version       : Version.\n" +
               "\t-n, --num-receivers : Number of receivers.\n" +
               "\t-p, --port          : TCP port to communicate with client "
               "for "
               "configuration. Non-zero and 16 bit.\n" +
               "\t-c, --print-headers : Print callback headers for debugging. "
               "Disabled by default.\n" +
               "\t-u, --uid           : Set effective user id if receiver "
               "started "
               "with privileges. \n\n";
    }
    throw sls::RuntimeError("Unknown AppType for help message");
}

void CommandLineOptions::setupSignalHandler(int signal, void (*handler)(int)) {
    // Catch signal SIGINT to close files and call destructors properly
    struct sigaction sa{};
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask); // dont block additional signals
    sa.sa_flags = 0;
    if (sigaction(signal, &sa, nullptr) == -1) {
        LOG(sls::logERROR) << "Could not set handler for " << strsignal(signal);
    }
}

void CommandLineOptions::setEffectiveUID(uid_t uid) {
    if (geteuid() == uid) {
        LOG(sls::logINFO) << "Process already has the same Effective UID "
                          << uid;
    } else {
        if (seteuid(uid) != 0 || geteuid() != uid) {
            throw sls::RuntimeError("Could not set Effective UID");
        }
        LOG(sls::logINFO) << "Process Effective UID changed to " << uid;
    }
}