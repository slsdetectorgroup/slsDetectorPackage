// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "CommandLineOptions.h"
#include "sls/sls_detector_defs.h"
#include "sls/versionAPI.h"
#include "sls/ToString.h"
#include "sls/logger.h"

ParsedOptions parseCommandLine(AppType app, int argc, char* argv[]) {
    CommonOptions base;
    base.port = DEFAULT_TCP_RX_PORTNO;
    MultiReceiverOptions multi;
    FrameSyncOptions frame;

    int opt;
    int option_index = 0;

    static struct option common_opts[] = {
        {"version", no_argument, nullptr, 'v'},
        {"port", required_argument, nullptr, 'p'},
        {"uid", required_argument, nullptr, 'u'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, 0}
    };

    static struct option single_opts[] = {
        {"rx_tcpport", required_argument, nullptr, 't'},
        {nullptr, 0, nullptr, 0}
    };

    static struct option multi_opts[] = {
        {"callback", no_argument, nullptr, 'c'},
        {"num-receivers", required_argument, nullptr, 'n'},
        {nullptr, 0, nullptr, 0}
    };

    static struct option frame_sync_opts[] = {
        {"print-headers", no_argument, nullptr, 'c'},
        {"num-receivers", required_argument, nullptr, 'n'},
        {nullptr, 0, nullptr, 0}
    };

    std::vector<option> options;
    options.insert(options.end(), std::begin(common_opts), std::end(common_opts) - 1);

     if (app == AppType::SingleReceiver) {
        options.insert(options.end(), std::begin(single_opts), std::end(single_opts) - 1);
    } else if (app == AppType::MultiReceiver) {
        options.insert(options.end(), std::begin(multi_opts), std::end(multi_opts) - 1);
    } else if (app == AppType::FrameSynchronizer) {
        options.insert(options.end(), std::begin(frame_sync_opts), std::end(frame_sync_opts) - 1);
    }

    std::string optstring = "vp:u:h";
    if (app == AppType::SingleReceiver) {
        optstring += "t:";
    }
    if (app == AppType::MultiReceiver || app == AppType::FrameSynchronizer) {
        optstring += "cn:";
    }

    while ((opt = getopt_long(argc, argv, optstring.c_str(), options.data(), &option_index)) != -1) {
        switch (opt) {

            case 'v': 
                base.versionRequested = true; 
                std::cout << getVersion(app) << std::endl;
                break;

            case 'h': 
                base.helpRequested = true; 
                std::cout << getHelpMessage(app) << std::endl;
                break;

            case 't':
                LOG(sls::logWARNING) << "Deprecated option. Please use 'p' or '--port'.";
                [[fallthrough]];

            case 'p':
                try {
                    base.port = sls::StringTo<uint16_t>(optarg);
                } catch (...) {
                    throw sls::RuntimeError("Invalid port number parsed.");
                }
                break;

            case 'u': 
                try {
                    base.userid = sls::StringTo<uint32_t>(optarg); 
                    if (base.userid != static_cast<uid_t>(-1)) {
                        setEffectiveUID(base.userid);
                    }
                } catch (...) {
                    throw sls::RuntimeError("Invalid uid parsed.");
                }
                break;

            case 'n':
                if (app == AppType::MultiReceiver)
                    multi.numReceivers = sls::StringTo<uint16_t>(optarg);
                else if (app == AppType::FrameSynchronizer)
                    frame.numReceivers = sls::StringTo<uint16_t>(optarg);
                break;

            case 'c':
                if (app == AppType::MultiReceiver)
                    multi.callbackEnabled = true;
                else if (app == AppType::FrameSynchronizer)
                    frame.printHeaders = true;
                break;

            default:
                throw sls::RuntimeError("Invalid arguments." + getHelpMessage(app));
        }
    }
    // remaining arguments
    if (optind < argc) {
        throw(sls::RuntimeError(std::string("Invalid arguments\n") + getHelpMessage(app)));
    }

    switch (app) {
        case AppType::SingleReceiver: return base;
        case AppType::MultiReceiver:
            static_cast<CommonOptions&>(multi) = base;
            return multi;
        case AppType::FrameSynchronizer:
            static_cast<CommonOptions&>(frame) = base;
            return frame;
    }

    throw std::logic_error("Unknown AppType");
}

void setEffectiveUID(uid_t uid) {
    if (geteuid() == uid) {
        LOG(sls::logINFO)
            << "Process already has the same Effective UID " << uid;
    } else {
        if (seteuid(uid) != 0 || geteuid() != uid) {
            throw sls::RuntimeError("Could not set Effective UID");
        }
        LOG(sls::logINFO) << "Process Effective UID changed to " << uid;
    }
}

std::string getTypeString(const AppType app) {
    switch (app) {
        case AppType::SingleReceiver: return "SingleReceiver";
        case AppType::MultiReceiver: return "MultiReceiver";
        case AppType::FrameSynchronizer: return "FrameSynchronizer";
        default: return "Unknown";
    }
}

std::string getVersion(AppType app) {
    return getTypeString(app) + " Version: " + APIRECEIVER;
}

std::string getHelpMessage(AppType app) {
    switch (app) {
        case AppType::SingleReceiver: 
            return std::string("\nUsage: ") + getTypeString(app) + " Options:\n" +
                "\t-v, --version       : Version.\n" +
                "\t-p, --port          : TCP port to communicate with client for "
                "configuration. Non-zero and 16 bit.\n" +
                "\t-u, --uid           : Set effective user id if receiver started "
                "with privileges. \n\n";

        case AppType::MultiReceiver:
            return std::string("\nUsage: " + getTypeString(app) + " Options:\n") +
                "\t-v, --version       : Version.\n" +
                "\t-n, --num-receivers : Number of receivers.\n" +
                "\t-p, --port          : TCP port to communicate with client for "
                "configuration. Non-zero and 16 bit.\n" +
                "\t-c, --callback      : Enable dummy callbacks for debugging. "
                "Disabled by default. \n" +
                "\t-u, --uid           : Set effective user id if receiver started "
                "with privileges. \n\n";        
        
        case AppType::FrameSynchronizer:
            return std::string("\nUsage: " + getTypeString(app) +  " Options:\n") +
            "\t-v, --version       : Version.\n" +
            "\t-n, --num-receivers : Number of receivers.\n" +
            "\t-p, --port          : TCP port to communicate with client for "
            "configuration. Non-zero and 16 bit.\n" +
            "\t-c, --print-headers : Print callback headers for debugging. "
            "Disabled by default.\n" +
            "\t-u, --uid           : Set effective user id if receiver started "
            "with privileges. \n\n";
    }
}