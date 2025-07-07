// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#include "CommandLineOptions.h"
#include "sls/sls_detector_defs.h"
#include "sls/versionAPI.h"
#include "sls/ToString.h"
#include "sls/logger.h"

#include <csignal>
#include <cstring>
#include <getopt.h>
#include <unistd.h>

#define MAX_RECEIVERS 1024

ParsedOptions parseCommandLine(AppType app, int argc, char* argv[]) {
    CommonOptions base;
    base.port = DEFAULT_TCP_RX_PORTNO;
    MultiReceiverOptions multi;
    FrameSyncOptions frame;
    uint16_t numReceivers = 1;
    bool optionalArg = false;

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
        {"rx_tcpport", required_argument, nullptr, 't'},
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
    } else if (app == AppType::MultiReceiver) {
        optstring += "cn:t:";
    } else if (app == AppType::FrameSynchronizer) {
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
                try {
                    if (app == AppType::MultiReceiver)
                        multi.numReceivers = sls::StringTo<uint16_t>(optarg);
                    else if (app == AppType::FrameSynchronizer)
                        frame.numReceivers = sls::StringTo<uint16_t>(optarg);
                    if (numReceivers == 0 || numReceivers > MAX_RECEIVERS) {
                        throw sls::RuntimeError("Invalid number of receivers. Max: "   + std::to_string(MAX_RECEIVERS));
                    }
                    multi.numReceivers = numReceivers;
                    frame.numReceivers = numReceivers;
                } catch (...) {
                    throw sls::RuntimeError("Invalid number of receivers parsed." + std::to_string(numReceivers));
                }
                break;

            case 'c':
                optionalArg = true;
                if (app == AppType::MultiReceiver) {
                    multi.callbackEnabled = true;
                } else if (app == AppType::FrameSynchronizer) {
                    frame.printHeaders = true;
                }
                break;

            default:
                throw sls::RuntimeError("Invalid arguments." + getHelpMessage(app));
        }
    }
    // remaining arguments
    if (optind < argc) {
        // maintain backward compatibility of [start port] [num receivers] [optional arg] ( for multi receiver and frame synchronizer )
        if (app != AppType::SingleReceiver && slsDetectorDefs::OK == GetDeprecatedCommandLineOptions(argc, argv, base.port, numReceivers, optionalArg)) {
            if (app == AppType::MultiReceiver) {
                multi.numReceivers = numReceivers;
                multi.callbackEnabled = optionalArg;
            } else if (app == AppType::FrameSynchronizer) {
                frame.numReceivers = numReceivers;
                frame.printHeaders = optionalArg;
            }
        } else {
            throw sls::RuntimeError("Invalid arguments." + getHelpMessage(app));
        }
    }

    LOG(sls::logINFO) << "Number of receivers: " << numReceivers;
    LOG(sls::logINFO) << "TCP Port: " << base.port;
    switch (app) {
        case AppType::SingleReceiver: 
            return base;
        case AppType::MultiReceiver:
            LOG(sls::logINFO) << "Call back enable: " << multi.callbackEnabled;
            static_cast<CommonOptions&>(multi) = base;
            return multi;
        case AppType::FrameSynchronizer:
            LOG(sls::logINFO) << "Print headers: " << frame.printHeaders;
            static_cast<CommonOptions&>(frame) = base;
            return frame;
    }

    throw std::logic_error("Unknown AppType");
}


int GetDeprecatedCommandLineOptions(int argc, char *argv[], uint16_t &startPort, uint16_t &numReceivers, bool &optionalArg) {
    std::string deprecatedMessage =
        "Detected deprecated Options. Please update.\n";
    if (argc > 1) {
        try {
            if (argc == 3 || argc == 4) {
                startPort = sls::StringTo<uint16_t>(argv[1]);
                numReceivers = sls::StringTo<uint16_t>(argv[2]);
                if (numReceivers > MAX_RECEIVERS) {
                    LOG(sls::logWARNING) << deprecatedMessage;
                    LOG(sls::logERROR)
                        << "Did you mix up the order of the arguments? Max "
                           "number of recievers: " << MAX_RECEIVERS;
                    return slsDetectorDefs::FAIL;
                }
                if (numReceivers == 0) {
                    LOG(sls::logWARNING) << deprecatedMessage;
                    LOG(sls::logERROR) << "Invalid number of receivers. Options: 1 - " << MAX_RECEIVERS;
                    return slsDetectorDefs::FAIL;
                }
                if (argc == 4) {
                    optionalArg = sls::StringTo<bool>(argv[3]);
                }
            } else
                throw std::runtime_error("Invalid number of arguments");
        } catch (const std::exception &e) {
            LOG(sls::logWARNING) << deprecatedMessage;
            LOG(sls::logERROR) << e.what();
            return slsDetectorDefs::FAIL;
        }
    }
    return slsDetectorDefs::OK;
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
    throw sls::RuntimeError("Unknown AppType for help message");
}

void setupSignalHandler(int signal, void (*handler)(int)) {
    // Catch signal SIGINT to close files and call destructors properly
    struct sigaction sa{};
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask); // dont block additional signals
    sa.sa_flags = 0;
    if (sigaction(signal, &sa, nullptr) == -1) {
        LOG(sls::logERROR) << "Could not set handler for " << strsignal(signal);
    }
}