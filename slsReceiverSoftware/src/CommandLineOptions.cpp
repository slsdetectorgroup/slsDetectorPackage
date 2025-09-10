// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "CommandLineOptions.h"
#include "sls/ToString.h"
#include "sls/logger.h"
#include "sls/sls_detector_defs.h"
#include "sls/versionAPI.h"

#include <cstring>
#include <unistd.h>

CommandLineOptions::CommandLineOptions(AppType app)
    : appType_(app), optString_(buildOptString()),
      longOptions_(buildOptionList()) {}

/** for testing */
ParsedOptions CommandLineOptions::parse(const std::vector<std::string> &args) {
    std::vector<char *> argv;
    argv.reserve(args.size());
    for (const auto &arg : args) {
        argv.push_back(const_cast<char *>(arg.c_str()));
    }
    int argc = static_cast<int>(argv.size());
    return parse(argc, argv.data());
}

ParsedOptions CommandLineOptions::parse(int argc, char *argv[]) {
    CommonOptions base;
    MultiReceiverOptions multi;
    FrameSyncOptions frame;
    base.port = DEFAULT_TCP_RX_PORTNO;

    optind = 0; // reset getopt
    int opt, option_index = 0;

    bool help_or_version_requested = false;

    while ((opt = getopt_long(argc, argv, optString_.c_str(),
                              longOptions_.data(), &option_index)) != -1) {
        switch (opt) {
        case 'v':
        case 'h':
            handleCommonOption(opt, optarg, base);
            help_or_version_requested = true;
            break;
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
    if (!help_or_version_requested && optind < argc) {

        // deprecated and current options => invalid
        if (base.port != DEFAULT_TCP_RX_PORTNO || multi.numReceivers != 1 ||
            frame.numReceivers != 1 || multi.callbackEnabled != false ||
            frame.printHeaders != false) {
            LOG(sls::logWARNING) << "Cannot use both deprecated options and "
                                    "the valid options simultaneously. Please "
                                    "move away from the deprecated options.\n";
        }

        // unsupported deprecated arguments
        if (appType_ == AppType::SingleReceiver) {
            throw sls::RuntimeError("Invalid arguments." + getHelpMessage());
        }

        // parse deprecated arguments
        std::vector<std::string> args(argv, argv + argc);
        auto [p, n, o] = ParseDeprecated(args);
        // set options
        base.port = p;
        if (appType_ == AppType::MultiReceiver) {
            multi.numReceivers = n;
            multi.callbackEnabled = o;
        } else if (appType_ == AppType::FrameSynchronizer) {
            frame.numReceivers = n;
            frame.printHeaders = o;
        }
    }

    // Logging
    if (!help_or_version_requested) {
        LOG(sls::logINFO) << "TCP Port: " << base.port;
        if (appType_ == AppType::MultiReceiver) {
            LOG(sls::logINFO) << "Number of receivers: " << multi.numReceivers;
            LOG(sls::logINFO) << "Callback enabled: " << multi.callbackEnabled;
        } else if (appType_ == AppType::FrameSynchronizer) {
            LOG(sls::logINFO) << "Number of receivers: " << frame.numReceivers;
            LOG(sls::logINFO) << "Print headers: " << frame.printHeaders;
        }
    }

    switch (appType_) {
    case AppType::SingleReceiver:
        return base;
    case AppType::MultiReceiver:
        static_cast<CommonOptions &>(multi) = base;
        return multi;
    case AppType::FrameSynchronizer:
        static_cast<CommonOptions &>(frame) = base;
        return frame;
    default:
        throw sls::RuntimeError("Unknown AppType in CommandLineOptions::parse");
    }
}

std::vector<option> CommandLineOptions::buildOptionList() const {
    std::vector<option> opts = {
        {"version", no_argument, nullptr, 'v'},
        {"help", no_argument, nullptr, 'h'},
        {"port", required_argument, nullptr, 'p'},
        {"uid", required_argument, nullptr, 'u'},
    };
    switch (appType_) {
    case AppType::SingleReceiver:
        opts.push_back({"rx_tcpport", required_argument, nullptr, 't'});
        break;
    case AppType::MultiReceiver:
        opts.push_back({"num-receivers", required_argument, nullptr, 'n'});
        opts.push_back({"callback", no_argument, nullptr, 'c'});
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
    std::string optstr = "vhp:u:";
    if (appType_ == AppType::MultiReceiver ||
        appType_ == AppType::FrameSynchronizer)
        optstr += "cn:";
    if (appType_ == AppType::SingleReceiver)
        optstr += "t:";
    return optstr;
}

uint16_t CommandLineOptions::parsePort(const char *optarg) {
    uint16_t val = 0;
    try {
        val = sls::StringTo<uint16_t>(optarg);
    } catch (...) {
        throw sls::RuntimeError("Could not parse port number " +
                                std::string(optarg));
    }
    if (val < 1024) {
        throw sls::RuntimeError(
            "Invalid/ privileged port number parsed. Min: 1024.");
    }
    return val;
}

uint16_t CommandLineOptions::parseNumReceivers(const char *optarg) {
    uint16_t val = 0;
    try {
        val = sls::StringTo<uint16_t>(optarg);
    } catch (...) {
        throw sls::RuntimeError("Could not parse number of receivers " +
                                std::string(optarg));
    }
    if (val == 0 || val > MAX_RECEIVERS) {
        throw sls::RuntimeError(
            "Invalid number of receivers parsed. Options: 1 - " +
            std::to_string(MAX_RECEIVERS));
    }
    return val;
}

uid_t CommandLineOptions::parseUID(const char *optarg) {
    uid_t val = -1;
    try {
        val = sls::StringTo<uid_t>(optarg);
    } catch (...) {
        throw sls::RuntimeError("Could not parse UID " + std::string(optarg));
    }
    if (val == static_cast<uid_t>(-1)) {
        throw sls::RuntimeError(
            "Could not parse UID. Expected a valid user ID." +
            std::string(optarg));
    }
    return val;
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
        base.port = parsePort(optarg);
        break;
    case 'u':
        base.userid = parseUID(optarg);
        setEffectiveUID(base.userid);
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
        auto val = parseNumReceivers(optarg);
        if (appType_ == AppType::MultiReceiver)
            multi.numReceivers = val;
        else if (appType_ == AppType::FrameSynchronizer)
            frame.numReceivers = val;
        break;
    }

    case 't':
        LOG(sls::logWARNING) << "Deprecated option '-t' and '--rx_tcport'. Use "
                                "'--p' or '--port' instead.";
        base.port = parsePort(optarg);
        break;
    }
}

/* maintain backward compatibility of [start port] [num receivers] [optional
 * arg] */
std::tuple<uint16_t, uint16_t, bool>
CommandLineOptions::ParseDeprecated(const std::vector<std::string> &args) {

    size_t nargs = args.size();
    if (nargs != 1 && nargs != 3 && nargs != 4) {
        throw sls::RuntimeError("Invalid number of arguments.");
    }

    LOG(sls::logWARNING)
        << "Deprecated options will be removed in future versions. "
           "Please use the new options.\n";

    // default deprecated values
    if (nargs == 1) {
        return std::make_tuple(DEFAULT_TCP_RX_PORTNO, 1, false);
    }

    // parse deprecated arguments
    uint16_t p = parsePort(args[1].c_str());
    uint16_t n = parseNumReceivers(args[2].c_str());
    bool o = false;
    if (nargs == 4) {
        try {
            o = sls::StringTo<bool>(args[3].c_str());
        } catch (...) {
            throw sls::RuntimeError("Invalid optional argument "
                                    "parsed. Expected 1 (true) or "
                                    "0 (false).");
        }
    }
    return std::make_tuple(p, n, o);
}

std::string CommandLineOptions::getTypeString() const {
    switch (appType_) {
    case AppType::SingleReceiver:
        return "slsReceiver";
    case AppType::MultiReceiver:
        return "slsMultiReceiver";
    case AppType::FrameSynchronizer:
        return "slsFrameSynchronizer";
    default:
        return "Unknown";
    }
}

std::string CommandLineOptions::getVersion() const {
    return getTypeString() + " Version: " + APIRECEIVER;
}

std::string CommandLineOptions::getHelpMessage() const {
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