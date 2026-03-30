#include "CommandLineOptions.h"
#include "StopServer.h"
#include "VirtualMatterhornServer.h"
#include "sls/logger.h"
#include "sls/sls_detector_exceptions.h"
#include <semaphore.h>

#include <csignal>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

// gettid added in glibc 2.30
#if __GLIBC__ == 2 && __GLIBC_MINOR__ < 30
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif

using namespace sls;

pid_t child_pid = -1;

/**
 * Control+C Interrupt Handler
 * to let all the other process know to exit properly
 */
void sigInterruptHandler(int signal) {
    (void)signal; // suppress unused warning if needed
    if (child_pid > 0) {
        kill(child_pid, SIGTERM); // tell child to exit
    }
    std::exit(EXIT_SUCCESS);
}

// TODO: should be a generic ServerApp for all detectors
int main(int argc, char *argv[]) {

    CommandLineOptions cli;
    DetectorServerOptions opts{};
    try {
        opts = cli.parse(argc, argv);
    } catch (sls::RuntimeError &e) {
        return EXIT_FAILURE;
    }
    if (opts.versionRequested || opts.helpRequested) {
        return EXIT_SUCCESS;
    }

    // Register Ctrl+C handler
    std::signal(SIGINT, sigInterruptHandler);

    // handle locally on socket crash
    // sls::setupSignalHandler(SIGPIPE, SIG_IGN); / what is this?

    child_pid = fork(); // fork process for control and stop server

    if (child_pid == 0) {
        // Stop server Process
        signal(SIGPIPE, SIG_IGN);

        // std::signal(SIGTERM, childSigTermHandler);
        LOG(TLogLevel::logINFOBLUE) << "Stop Server [" << opts.port + 1 << "]";
        try {
            VirtualMatterhornServer stopServer(opts.port + 1);
        } catch (...) {
            LOG(TLogLevel::logINFOBLUE)
                << "Exiting Stop Server [ Tid: " << gettid() << " ]";
            // TODO: maybe also terminate the control server !!!!
            std::exit(EXIT_FAILURE);
        }
        LOG(TLogLevel::logINFOBLUE)
            << "Exiting Stop Server [ Tid: " << gettid() << " ]";
        LOG(sls::logINFO) << "Exiting Stop Server";
        exit(EXIT_SUCCESS);
    } else if (child_pid > 0) {
        // Control Server Process
        signal(SIGPIPE, SIG_IGN);

        LOG(TLogLevel::logINFOBLUE) << "Control Server [" << opts.port << "]\n";

        try {
            VirtualMatterhornServer server(
                opts.port); // TODO use virtual if compiled with virtual
                            // simulators on
        } catch (...) {
            kill(child_pid, SIGTERM); // tell child to exit
            LOG(sls::logINFOBLUE) << "Exiting [ Tid: " << gettid() << " ]";
            std::exit(EXIT_FAILURE);
        }
        waitpid(child_pid, nullptr, 0); // wait for child to exit
        LOG(sls::logINFOBLUE) << "Exiting [ Tid: " << gettid() << " ]";
        LOG(sls::logINFO) << "Exiting Detector Server";
        exit(EXIT_SUCCESS);
    } else {
        LOG(sls::logERROR)
            << "Failed to fork process for control and stop server";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
