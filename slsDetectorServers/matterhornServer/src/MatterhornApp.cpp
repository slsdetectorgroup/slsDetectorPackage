#include "CommandLineOptions.h"
#include "VirtualMatterhornServer.h"
#include "sls/thread_utils.h"
#include "sls/logger.h"
#include "sls/sls_detector_exceptions.h"
#include "sls/versionAPI.h"
#include <semaphore.h>

#include <csignal>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace sls;

pid_t pid = -1;

static volatile sig_atomic_t interruption = 0;

/**
 * Control+C Interrupt Handler
 * to let all the other process know to exit properly
 */
void sigInterruptHandler(int signal) {
    (void)signal; // suppress unused warning if needed
    interruption = 1;
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
    if (opts.versionRequested) {
        std::cout << fmt::format("MatterhornServer Version: {}", APIMATTERHORN)
                  << std::endl;

        return EXIT_SUCCESS;
    }

    if (opts.helpRequested) {
        return EXIT_SUCCESS;
    }

    LOG(TLogLevel::logINFOMAGENTA) << cli.printOptions();

    // Register Ctrl+C handler
    std::signal(SIGINT, sigInterruptHandler);

    // handle locally on socket crash
    signal(SIGPIPE, SIG_IGN);

    pid = fork(); // fork process for control and stop server

    if (pid == 0) {
        // Stop server Process

        LOG(TLogLevel::logINFOBLUE) << "Stop Server [" << opts.port + 1 << "]";
        try {
            VirtualMatterhornServer stopServer(opts.port + 1);
            while (!interruption) {
                pause(); // wait for signal to exit
            }
        } catch (...) {
            kill(getppid(), SIGINT); // tell parent to exit // TODO: should then
                                     // also return EXIT_FAILURE
        }
        LOG(TLogLevel::logINFOBLUE)
            << "Exiting Stop Server [ Tid: " << getThreadId() << " ]";
        LOG(sls::logINFO) << "Exiting Stop Server";
    } else if (pid > 0) {
        // parent
        // Control Server Process

        LOG(TLogLevel::logINFOBLUE) << "Control Server [" << opts.port << "]\n";

        try {
            VirtualMatterhornServer server(
                opts.port); // TODO use virtual if compiled with virtual
                            // simulators on
            while (!interruption) {
                pause(); // wait for signal to exit
            }
        } catch (...) {
            LOG(sls::logINFOBLUE)
                << "Exiting Control Server [ Tid: " << getThreadId() << " ]";
            LOG(sls::logINFO) << "Exiting Detector Server";
            kill(pid, SIGINT);        // tell child to exit
            waitpid(pid, nullptr, 0); // wait for child to exit
            return EXIT_FAILURE;
        }
        waitpid(pid, nullptr, 0); // wait for child to exit
        LOG(sls::logINFOBLUE)
            << "Exiting Detector Control Server [ Tid: " << getThreadId()
            << " ]";
        LOG(sls::logINFO) << "Exiting Detector Server";
    } else {
        LOG(sls::logERROR)
            << "Failed to fork process for control and stop server";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
