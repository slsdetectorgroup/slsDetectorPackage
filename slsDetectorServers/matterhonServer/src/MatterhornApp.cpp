#include "CommandLineOptions.h"
#include "VirtualMatterhornServer.h"
#include "sls/logger.h"
#include "sls/sls_detector_exceptions.h"
#include "sls/versionAPI.h"
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

pid_t pid = -1;

volatile bool interruption = false;

/**
 * Control+C Interrupt Handler
 * to let all the other process know to exit properly
 */
void sigInterruptHandler(int signal) {
    (void)signal; // suppress unused warning if needed
    /*
    if (pid > 0) {
        kill(pid, SIGTERM); // tell child to exit
    }
    */
    interruption = true; // tell parent to exit
}

void sigterm_handler(int) { interruption = true; }

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
                  << std::endl; // might go back to costum CommandLIneOptions
                                // getVersion
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

    // handle locally on socket crash
    // sls::setupSignalHandler(SIGPIPE, SIG_IGN); / what is this?

    pid = fork(); // fork process for control and stop server

    if (pid == 0) {
        // Stop server Process
        signal(SIGTERM, sigterm_handler);

        LOG(TLogLevel::logINFOBLUE) << "Stop Server [" << opts.port + 1 << "]";
        try {
            VirtualMatterhornServer stopServer(opts.port + 1);
            while (!interruption) {
                sleep(1);
            }
        } catch (...) {
            LOG(TLogLevel::logINFOBLUE)
                << "Exiting Stop Server [ Tid: " << gettid() << " ]";
            // TODO: maybe also terminate the control server !!!!
            return EXIT_FAILURE;
        }
        LOG(TLogLevel::logINFOBLUE)
            << "Exiting Stop Server [ Tid: " << gettid() << " ]";
        LOG(sls::logINFO) << "Exiting Stop Server";
        return EXIT_SUCCESS;
    } else if (pid > 0) {
        // parent
        // Control Server Process

        LOG(TLogLevel::logINFOBLUE) << "Control Server [" << opts.port << "]\n";

        try {
            VirtualMatterhornServer server(
                opts.port); // TODO use virtual if compiled with virtual
                            // simulators on
            while (!interruption) {
                sleep(1);
            }
        } catch (...) {
            kill(0, SIGTERM); // tell child to exit
            LOG(sls::logINFOBLUE) << "Exiting [ Tid: " << gettid() << " ]";
            return EXIT_FAILURE;
        }
        waitpid(0, nullptr, 0); // wait for child to exit
        LOG(sls::logINFOBLUE) << "Exiting [ Tid: " << gettid() << " ]";
        LOG(sls::logINFO) << "Exiting Detector Server";
    } else {
        LOG(sls::logERROR)
            << "Failed to fork process for control and stop server";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
