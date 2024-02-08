// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include "clogger.h"
#include "common.h"
#include "communication_funcs.h"
#include "sharedMemory.h"
#include "sls/sls_detector_defs.h"
#include "sls/versionAPI.h"
#include "slsDetectorServer_defs.h"
#include "slsDetectorServer_funcs.h"

#include <getopt.h>
#include <limits.h>
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
extern int ignoreConfigFileFlag;

// Global variables from slsDetectorFunctionList
#ifdef GOTTHARDD
extern int phaseShift;
#endif
#if defined(GOTTHARDD) || defined(GOTTHARD2D) || defined(EIGERD) ||            \
    defined(MYTHEN3D)
extern int masterCommandLine;
#endif
#ifdef EIGERD
extern int topCommandLine;
#endif
int portno = DEFAULT_TCP_CNTRL_PORTNO;

void error(char *msg) { perror(msg); }

void sigInterruptHandler(int p) {
    sharedMemory_remove();
    exit(-1);
}

int main(int argc, char *argv[]) {

    // options
    isControlServer = 1;
    debugflag = 0;
    updateFlag = 0;
    checkModuleFlag = 1;
    char version[MAX_STR_LENGTH] = {0};
    memset(version, 0, MAX_STR_LENGTH);
    ignoreConfigFileFlag = 0;
#if defined(GOTTHARDD) || defined(GOTTHARD2D) || defined(EIGERD) ||            \
    defined(MYTHEN3D)
    masterCommandLine = -1;
#endif
#ifdef EIGERD
    topCommandLine = -1;
#endif

    // help message
    const size_t helpMessageSize = 1200;
    char helpMessage[helpMessageSize];
    {
        memset(helpMessage, 0, helpMessageSize);
        int len = snprintf(
            helpMessage, helpMessageSize,
            "Usage: %s [arguments]\n"
            "Possible arguments are:\n"
            "\t-v, --version            : Software version\n"
            "\t-p, --port <port>        : TCP communication port with client. "
            "\n"
            "\t-g, --nomodule           : [Mythen3][Gotthard2][Xilinx Ctb] \n"
            "\t                           Generic or No Module mode. Skips "
            "detector type checks. \n"
            "\t-f, --phaseshift <value> : [Gotthard] only. Sets phase shift. \n"
            "\t-d, --devel              : Developer mode. Skips firmware "
            "checks. \n"
            "\t-u, --update             : Update mode. Skips firmware checks "
            "and "
            "initial detector setup. \n"
            "\t-i, --ignore-config      : "
            "[Eiger][Jungfrau][Gotthard][Gotthard2] \n"
            "\t                           Ignore config file. \n"
            "\t-m, --master <master>    : "
            "[Eiger][Mythen3][Gotthard][Gotthard2] \n"
            "\t                           Set Master to 0 or 1. Precedence "
            "over "
            "config file. Only for virtual servers except Eiger. \n"
            "\t-t, --top <top>          : [Eiger] Set Top to 0 or 1. "
            "Precedence "
            "over config file. \n"
            "\t-s, --stopserver         : Stop server. Do not use as it is "
            "created "
            "by control server \n\n",
            argv[0]);
        if (len >= (int)helpMessageSize) {
            LOG(logERROR, ("Help for Server command line arguments size %d "
                           "exceed capacity of %d characters\n",
                           len, helpMessageSize));
        }
    }

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
        {"ignore-config", no_argument, NULL, 'i'},
        {"master", required_argument, NULL, 'm'},
        {"top", required_argument, NULL, 't'},
        {"stopserver", no_argument, NULL, 's'},
        {NULL, 0, NULL, 0}};

    optind = 1;
    // getopt_long stores the option index here
    int option_index = 0;
    int c = 0;

    while (c != -1) {
        c = getopt_long(argc, argv, "hvp:f:gduim:t:s", long_options,
                        &option_index);

        // Detect the end of the options
        if (c == -1)
            break;

        switch (c) {

        case 'v':
#ifdef GOTTHARDD
            strcpy(version, APIGOTTHARD);
#elif EIGERD
            strcpy(version, APIEIGER);
#elif JUNGFRAUD
            strcpy(version, APIJUNGFRAU);
#elif CHIPTESTBOARDD
            strcpy(version, APICTB);
#elif MOENCHD
            strcpy(version, APIMOENCH);
#elif MYTHEN3D
            strcpy(version, APIMYTHEN3);
#elif GOTTHARD2D
            strcpy(version, APIGOTTHARD2);
#endif
            LOG(logINFO, ("SLS Detector Server Version: %s\n", version));
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
            LOG(logINFO, ("Detected update mode from command line\n"));
            updateFlag = 1;
            break;

        case 's':
            LOG(logINFO, ("Detected stop server\n"));
            isControlServer = 0;
            break;

        case 'i':
#if defined(EIGERD) || defined(GOTTHARDD) || defined(GOTTHARD2D) ||            \
    defined(JUNGFRAUD)
            LOG(logINFO, ("Ignoring config file\n"));
            ignoreConfigFileFlag = 1;
#else
            LOG(logERROR, ("No server config files for this detector\n"));
            exit(EXIT_FAILURE);
#endif
            break;

        case 'm':
#if !defined(VIRTUAL) && !defined(EIGERD)
            LOG(logERROR, ("Cannot set master via the detector server for this "
                           "detector\n"));
            exit(EXIT_FAILURE);
#elif defined(GOTTHARDD) || defined(GOTTHARD2D) || defined(EIGERD) ||          \
    defined(MYTHEN3D)
            if (sscanf(optarg, "%d", &masterCommandLine) != 1) {
                LOG(logERROR, ("Cannot scan master argument\n%s", helpMessage));
                exit(EXIT_FAILURE);
            }
            if (masterCommandLine == 1) {
                LOG(logINFO, ("Detector Master mode\n"));
            } else {
                LOG(logINFO, ("Detector Slave mode\n"));
            }
#else
            LOG(logERROR, ("No master implemented for this detector server\n"));
            exit(EXIT_FAILURE);
#endif
            break;

        case 't':
#ifdef EIGERD
            if (sscanf(optarg, "%d", &topCommandLine) != 1) {
                LOG(logERROR, ("Cannot scan top argument\n%s", helpMessage));
                exit(EXIT_FAILURE);
            }
            if (topCommandLine == 1) {
                LOG(logINFO, ("Detector Top mode\n"));
            } else {
                LOG(logINFO, ("Detector Bottom mode\n"));
            }
#else
            LOG(logERROR, ("No top implemented for this detector server\n"));
            exit(EXIT_FAILURE);
#endif
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

        // validate control and stop port number
        if (0 >= portno || portno > USHRT_MAX || 0 >= (portno + 1) ||
            (portno + 1) > USHRT_MAX) {
            LOG(logERROR, ("Invalid control server or stop server port "
                           "numbers (%d, %d). It must be in range 1 - %d",
                           portno, portno + 1, USHRT_MAX));
            return -1;
        }

        if (sharedMemory_create(portno) == FAIL) {
            return -1;
        }

        if (updateFlag == 0) {
            // update flag if update file exists (command line arg overwrites)
            const int fileNameSize = 128;
            char fname[fileNameSize];
            if (getAbsPath(fname, fileNameSize, UPDATE_FILE) == FAIL) {
                LOG(logERROR,
                    ("Could not get abs path to check if update file exists. "
                     "Will try current folder instead.\n"));
                strcpy(fname, UPDATE_FILE);
            }
            if (access(fname, F_OK) == 0) {
                updateFlag = 1;
                LOG(logINFOBLUE, ("File Found: Update Mode enabled\n"));
            } else {
                LOG(logINFOBLUE, ("File not Found: Update Mode diabled\n"));
            }
        }
#ifdef STOP_SERVER
        // start stop server process
        char cmd[MAX_STR_LENGTH];
        memset(cmd, 0, MAX_STR_LENGTH);
        char portCmd[256];
        memset(portCmd, 0, 256);
        sprintf(portCmd, "-p%d", portno);
        for (int i = 0; i < argc; ++i) {
            LOG(logDEBUG, ("i:%d argv[i]:%s\n", i, argv[i]));
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
