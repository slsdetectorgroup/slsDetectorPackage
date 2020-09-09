/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include "clogger.h"
#include "communication_funcs.h"
#include "sharedMemory.h"
#include "slsDetectorServer_defs.h"
#include "slsDetectorServer_funcs.h"
#include "sls_detector_defs.h"
#include "versionAPI.h"

#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

// Global variables from  communication_funcs
extern int isControlServer;
extern int ret;

// Global variables from slsDetectorServer_funcs
extern int sockfd;
extern int debugflag;
extern int updateFlag;
extern int checkModuleFlag;

// Global variables from slsDetectorFunctionList
#ifdef GOTTHARDD
extern int phaseShift;
#endif

void error(char *msg) { perror(msg); }

void sigInterruptHandler(int p) {
    sharedMemory_remove();
    exit(-1);
}

int main(int argc, char *argv[]) {

    // options
    int portno = DEFAULT_PORTNO;
    isControlServer = 1;
    debugflag = 0;
    updateFlag = 0;
    checkModuleFlag = 1;
    int version = 0;

    // help message
    char helpMessage[MAX_STR_LENGTH];
    memset(helpMessage, 0, MAX_STR_LENGTH);
    sprintf(
        helpMessage,
        "Usage: %s [arguments]\n"
        "Possible arguments are:\n"
        "\t-v, --version            : Software version\n"
        "\t-p, --port <port>        : TCP communication port with client. \n"
        "\t-g, --nomodule           : [Mythen3][Gotthard2] Generic or No "
        "Module mode. Skips detector type checks. \n"
        "\t-f, --phaseshift <value> : [Gotthard] only. Sets phase shift. \n"
        "\t-d, --devel              : Developer mode. Skips firmware checks. \n"
        "\t-u, --update             : Update mode. Skips firmware checks and "
        "initial detector setup. \n"
        "\t-s, --stopserver         : Stop server. Do not use as created by "
        "control server \n\n",
        argv[0]);

    // parse command line for config
    static struct option long_options[] = {
        // These options set a flag.
        // These options don’t set a flag. We distinguish them by their indices.
        {"help", no_argument, NULL, 'h'},
        {"version", no_argument, NULL, 'v'},
        {"port", required_argument, NULL, 'p'},
        {"phaseshift", required_argument, NULL, 'f'},
        {"nomodule", no_argument, NULL, 'g'}, // generic
        {"devel", no_argument, NULL, 'd'},
        {"update", no_argument, NULL, 'u'},
        {"stopserver", no_argument, NULL, 's'},
        {NULL, 0, NULL, 0}};

    optind = 1;
    // getopt_long stores the option index here
    int option_index = 0;
    int c = 0;

    while (c != -1) {
        c = getopt_long(argc, argv, "hvp:f:gdus", long_options, &option_index);

        // Detect the end of the options
        if (c == -1)
            break;

        switch (c) {

        case 'v':
#ifdef GOTTHARDD
            version = APIGOTTHARD;
#elif EIGERD
            version = APIEIGER;
#elif JUNGFRAUD
            version = APIJUNGFRAU;
#elif CHIPTESTBOARDD
            version = APICTB;
#elif MOENCHD
            version = APIMOENCH;
#elif MYTHEN3D
            version = APIMYTHEN3;
#elif GOTTHARD2D
            version = APIGOTTHARD2;
#endif
            LOG(logINFOBLUE, ("SLS Detector Server Version: %s (0x%x)\n",
                              GITBRANCH, version));
            exit(EXIT_SUCCESS);

        case 'p':
            if (sscanf(optarg, "%d", &portno) != 1) {
                LOG(logERROR, ("Cannot scan port argument\n%s", helpMessage));
                exit(EXIT_FAILURE);
            }
            LOG(logINFO, ("Detected port: %d\n", portno));
            break;

        case 'f':
#ifndef GOTTHARDD
            LOG(logERROR,
                ("Phase shift argument not implemented for this detector\n"));
            exit(EXIT_FAILURE);
#else
            if (sscanf(optarg, "%d", &phaseShift) != 1) {
                LOG(logERROR,
                    ("Cannot scan phase shift argument\n%s", helpMessage));
                exit(EXIT_FAILURE);
            }
            LOG(logINFO, ("Detected phase shift: %d\n", phaseShift));
#endif
            break;

        case 'g':
            LOG(logINFO, ("Detected generic mode (no module)\n"));
            checkModuleFlag = 0;
            break;

        case 'd':
            LOG(logINFO, ("Detected developer mode\n"));
            debugflag = 1;
            break;

        case 'u':
            LOG(logINFO, ("Detected update mode\n"));
            updateFlag = 1;
            break;

        case 's':
            LOG(logINFO, ("Detected stop server\n"));
            isControlServer = 0;
            break;

        case 'h':
            printf("%s", helpMessage);
            exit(EXIT_SUCCESS);
        default:
            printf("\n%s", helpMessage);
            exit(EXIT_FAILURE);
        }
    }

    // control server
    if (isControlServer) {
        LOG(logINFOBLUE, ("Control Server [%d]\n", portno));

        // Catch signal SIGINT (Ctrl + c) to destroy shm properly
        struct sigaction sa;
        sa.sa_flags = 0;                     // no flags
        sa.sa_handler = sigInterruptHandler; // handler function
        sigemptyset(&sa.sa_mask); // dont block additional signals during
                                  // invocation of handler
        if (sigaction(SIGINT, &sa, NULL) == -1) {
            LOG(logERROR, ("Could not set handler function for SIGINT"));
        }

        if (sharedMemory_create(portno) == FAIL) {
            return -1;
        }
#ifdef STOP_SERVER
        // start stop server process
        char cmd[MAX_STR_LENGTH];
        memset(cmd, 0, MAX_STR_LENGTH);
        char portCmd[256];
        memset(portCmd, 0, 256);
        sprintf(portCmd, "-p%d", portno);
        for (int i = 0; i < argc; ++i) {
            LOG(logINFOBLUE, ("i:%d argv[i]:%s\n", i, argv[i]));
            // remove port argument (--port) and [value]
            if (!strcasecmp(argv[i], "--port")) {
                ++i;
                continue;
            }
            // remove port argument (-p[value])
            if (!strcasecmp(argv[i], portCmd)) {
                continue;
            }
            if (i > 0) {
                strcat(cmd, " ");
            }
            strcat(cmd, argv[i]);
        }
        char temp[50];
        memset(temp, 0, sizeof(temp));
        sprintf(temp, " --stopserver --port %d &", portno + 1);
        strcat(cmd, temp);

        LOG(logDEBUG1, ("Command to start stop server:%s\n", cmd));
        system(cmd);
#endif
    }
    // stop server
    else {
        LOG(logINFOBLUE, ("Stop Server [%d]\n", portno));
        if (sharedMemory_open(portno - 1) == FAIL) {
            return -1;
        }
    }

    // if socket crash, ignores SISPIPE, prevents global signal handler
    // subsequent read/write to socket gives error - must handle locally
    signal(SIGPIPE, SIG_IGN);

    init_detector();
    // bind socket
    sockfd = bindSocket(portno);
    if (ret == FAIL)
        return -1;
    // assign function table
    function_table();

    if (isControlServer) {
        LOG(logINFOBLUE, ("Control Server Ready...\n\n"));
    } else {
        LOG(logINFOBLUE, ("Stop Server Ready...\n\n"));
    }

    // waits for connection
    int retval = OK;
    while (retval != GOODBYE && retval != REBOOT) {
        int fd = acceptConnection(sockfd);
        if (fd > 0) {
            retval = decode_function(fd);
            closeConnection(fd);
        }
    }

    exitServer(sockfd);

    // detach shared memory
    if (sharedMemory_detach() == FAIL) {
        return -1;
    }
    // remove shared memory (control server)
    if (isControlServer) {
        if (sharedMemory_remove() == FAIL) {
            return -1;
        }
    }

    if (retval == REBOOT) {
        LOG(logINFORED, ("Rebooting!\n"));
        fflush(stdout);
#if defined(MYTHEN3D) || defined(GOTTHARD2D)
        rebootNiosControllerAndFPGA();
#else
#ifndef VIRTUAL
        system("reboot");
#endif
#endif
    }
    LOG(logINFO, ("Goodbye!\n"));
    return 0;
}
