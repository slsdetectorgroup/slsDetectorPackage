#include "sls/Receiver.h"
#include "ClientInterface.h"
#include "sls/container_utils.h"
#include "sls/logger.h"
#include "sls/sls_detector_exceptions.h"
#include "sls/versionAPI.h"

#include <cstdlib>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <sys/syscall.h>
#include <unistd.h>

namespace sls{

Receiver::~Receiver() = default;

Receiver::Receiver(int argc, char *argv[]) : tcpipInterface(nullptr) {

    // options
    int tcpip_port_no = 1954;
    uid_t userid = -1;

    // parse command line for config
    static struct option long_options[] = {
        // These options set a flag.
        //{"verbose", no_argument,       &verbose_flag, 1},
        // These options don’t set a flag. We distinguish them by their indices.
        {"rx_tcpport", required_argument, nullptr,
         't'}, // TODO change or backward compatible to "port, p"?
        {"uid", required_argument, nullptr, 'u'},
        {"version", no_argument, nullptr, 'v'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, 0}};

    // initialize global optind variable (required when instantiating multiple
    // receivers in the same process)
    optind = 1;
    // getopt_long stores the option index here.
    int option_index = 0;
    int c = 0;

    while (c != -1) {
        c = getopt_long(argc, argv, "hvf:t:u:", long_options, &option_index);

        // Detect the end of the options.
        if (c == -1)
            break;

        switch (c) {

        case 't':
            sscanf(optarg, "%d", &tcpip_port_no);
            break;

        case 'u':
            if (sscanf(optarg, "%u", &userid) != 1) {
                throw sls::RuntimeError("Could not scan uid");
            }
            break;

        case 'v':
            std::cout << "SLS Receiver Version: " << GITBRANCH << " (0x"
                      << std::hex << APIRECEIVER << ")" << std::endl;
            LOG(logINFOBLUE)
                << "Exiting [ Tid: " << syscall(SYS_gettid) << " ]";
            exit(EXIT_SUCCESS);

        case 'h':
        default:
            std::cout << std::endl;

            std::string help_message =
                "Usage: " + std::string(argv[0]) + " [arguments]\n" +
                "Possible arguments are:\n" +
                "\t-t, --rx_tcpport <port> : TCP Communication Port with "
                "client. \n" +
                "\t-u, --uid <user id>     : Set effective user id if receiver "
                "\n" +
                "\t                          started with privileges. \n\n";

            // std::cout << help_message << std::endl;
            throw sls::RuntimeError(help_message);
        }
    }

    // set effective id if provided
    if (userid != static_cast<uid_t>(-1)) {
        if (geteuid() == userid) {
            LOG(logINFO) << "Process already has the same Effective UID "
                         << userid;
        } else {
            if (seteuid(userid) != 0) {
                std::ostringstream oss;
                oss << "Could not set Effective UID to " << userid;
                throw sls::RuntimeError(oss.str());
            }
            if (geteuid() != userid) {
                std::ostringstream oss;
                oss << "Could not set Effective UID to " << userid << ". Got "
                    << geteuid();
                throw sls::RuntimeError(oss.str());
            }
            LOG(logINFO) << "Process Effective UID changed to " << userid;
        }
    }

    // might throw an exception
    tcpipInterface = sls::make_unique<ClientInterface>(tcpip_port_no);
}

Receiver::Receiver(int tcpip_port_no) {
    // might throw an exception
    tcpipInterface = sls::make_unique<ClientInterface>(tcpip_port_no);
}

int64_t Receiver::getReceiverVersion() {
    return tcpipInterface->getReceiverVersion();
}

void Receiver::registerCallBackStartAcquisition(
    int (*func)(std::string, std::string, uint64_t, uint32_t, void *),
    void *arg) {
    tcpipInterface->registerCallBackStartAcquisition(func, arg);
}

void Receiver::registerCallBackAcquisitionFinished(void (*func)(uint64_t,
                                                                void *),
                                                   void *arg) {
    tcpipInterface->registerCallBackAcquisitionFinished(func, arg);
}

void Receiver::registerCallBackRawDataReady(void (*func)(char *, char *,
                                                         uint32_t, void *),
                                            void *arg) {
    tcpipInterface->registerCallBackRawDataReady(func, arg);
}

void Receiver::registerCallBackRawDataModifyReady(
    void (*func)(char *, char *, uint32_t &, void *), void *arg) {
    tcpipInterface->registerCallBackRawDataModifyReady(func, arg);
}

}