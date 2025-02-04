// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "slsDetectorFunctionList.h"
#include "clogger.h"
#include "common.h"
#include "sharedMemory.h"
#include "sls/versionAPI.h"

#ifndef VIRTUAL
#include "Beb.h"
#include "FebControl.h"
#endif

#include <string.h>
#include <unistd.h> //to gethostname
#ifdef VIRTUAL
#include "communication_funcs_UDP.h"
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>
#endif
extern int portno;
// Global variable from slsDetectorServer_funcs
extern int debugflag;
extern int updateFlag;
extern udpStruct udpDetails[MAX_UDP_DESTINATION];
extern int numUdpDestinations;
extern const enum detectorType myDetectorType;
extern int ignoreConfigFileFlag;

// Global variable from communication_funcs.c
extern int isControlServer;
extern void getMacAddressinString(char *cmac, int size, uint64_t mac);
extern void getIpAddressinString(char *cip, uint32_t ip);

// Variables that will be exported
int masterCommandLine = -1;
int topCommandLine = -1;

int initError = OK;
int initCheckDone = 0;
char initErrorMessage[MAX_STR_LENGTH];

int default_tau_from_file = -1;
enum detectorSettings thisSettings;
sls_detector_module *detectorModules = NULL;
int *detectorChans = NULL;
int *detectorDacs = NULL;

int send_to_ten_gig = 0;
unsigned int nimages_per_request = 1;

int top = 0;
int master = 0;
int normal = 0;

int eiger_highvoltage = 0;
int eiger_theo_highvoltage = 0;
int eiger_iodelay = 0;
int eiger_dynamicrange = 0;
int eiger_parallelmode = 0;
int eiger_overflow32 = 0;
int eiger_readoutspeed = 0;
int eiger_triggermode = 0;
int eiger_extgating = 0;
int eiger_extgatingpolarity = 0;
int eiger_nexposures = 1;
int eiger_ntriggers = 1;
int eiger_tau_ns = 0;

#ifdef VIRTUAL
pthread_t virtual_tid;
// values for virtual server
int64_t eiger_virtual_exptime = 0;
int64_t eiger_virtual_subexptime = 0;
int64_t eiger_virtual_subperiod = 0;
int64_t eiger_virtual_period = 0;
int eiger_virtual_counter_bit = 1;
int eiger_virtual_ratecorrection_variable = 0;
int64_t eiger_virtual_ratetable_tau_in_ns = -1;
int64_t eiger_virtual_ratetable_period_in_ns = -1;
int eiger_virtual_transmission_delay_left = 0;
int eiger_virtual_transmission_delay_right = 0;
int eiger_virtual_transmission_delay_frame = 0;
int eiger_virtual_transmission_flowcontrol_10g = 0;
int eiger_virtual_activate = 1;
uint64_t eiger_virtual_nextframenumber = 1;
int eiger_virtual_detPos[2] = {0, 0};
int eiger_virtual_test_mode = 0;
int eiger_virtual_quad_mode = 0;
int eiger_virtual_read_n_rows = 256;
int eiger_virtual_interrupt_subframe = 0;
int eiger_virtual_left_datastream = 1;
int eiger_virtual_right_datastream = 1;
int eiger_virtual_module_id = 0;
#endif
int defaultDacValues[NDAC] = DEFAULT_DAC_VALS;

int isInitCheckDone() { return initCheckDone; }

int getInitResult(char **mess) {
    *mess = initErrorMessage;
    return initError;
}

void basictests() {
    initError = OK;
    initCheckDone = 0;
    memset(initErrorMessage, 0, MAX_STR_LENGTH);
#ifdef VIRTUAL
    LOG(logINFOBLUE,
        ("************ EIGER Virtual Server ****************\n\n"));
#else
    LOG(logINFOBLUE,
        ("**************** EIGER Server ********************\n\n"));
#endif
    uint32_t ipadd = getDetectorIP();
    uint64_t macadd = getDetectorMAC();
    int64_t fwversion = getFirmwareVersion();
    char swversion[MAX_STR_LENGTH] = {0};
    memset(swversion, 0, MAX_STR_LENGTH);
    getServerVersion(swversion);
    int64_t sw_fw_apiversion = getFirmwareAPIVersion();

    LOG(logINFOBLUE,
        ("\n********************************************************\n"
         "Detector IP Addr         : 0x%x\n"
         "Detector MAC Addr        : 0x%llx\n"

         "Firmware (Beb) Version   : %lld\n"
         "F/w-S/w API Version      : %lld\n"
         "Required Firmware Version: %d\n"
         "Software Version         : %s\n"
         "********************************************************\n",
         (unsigned int)ipadd, (long long unsigned int)macadd,
         (long long int)fwversion, (long long int)sw_fw_apiversion,
         REQUIRED_FIRMWARE_VERSION, swversion));

    // update default udpdstip and udpdstmac (1g is hardware ip and hardware
    // mac)
    for (int iRxEntry = 0; iRxEntry != MAX_UDP_DESTINATION; ++iRxEntry) {
        udpDetails[iRxEntry].srcip = ipadd;
        udpDetails[iRxEntry].srcmac = macadd;
    }

#ifndef VIRTUAL
    // return if debugflag is not zero, debug mode
    if (debugflag || updateFlag) {
        return;
    }

    // cant read versions
    if (!fwversion || !sw_fw_apiversion) {
        strcpy(initErrorMessage,
               "Cant read versions from FPGA. Please update firmware.\n");
        LOG(logERROR, (initErrorMessage));
        initError = FAIL;
        return;
    }

    // check for API compatibility - old server
    if (sw_fw_apiversion > REQUIRED_FIRMWARE_VERSION) {
        sprintf(initErrorMessage,
                "This firmware-software api version (0x%lld) is incompatible "
                "with the software's minimum required firmware version "
                "(0x%lld).\nPlease update detector software to be compatible "
                "with this firmware.\n",
                (long long int)sw_fw_apiversion,
                (long long int)REQUIRED_FIRMWARE_VERSION);
        LOG(logERROR, (initErrorMessage));
        initError = FAIL;
        return;
    }

    // check for firmware compatibility - old firmware
    if (REQUIRED_FIRMWARE_VERSION > fwversion) {
        sprintf(initErrorMessage,
                "This firmware version (%lld) is incompatible.\n"
                "Please update firmware (min. %lld) to be compatible with this "
                "server.\n",
                (long long int)fwversion,
                (long long int)REQUIRED_FIRMWARE_VERSION);
        LOG(logERROR, (initErrorMessage));
        initError = FAIL;
        return;
    }
    LOG(logINFO, ("Compatibility - success\n"));
#endif
}

#ifdef VIRTUAL
void setTestImageMode(int ival) {
    if (ival >= 0) {
        if (ival == 0) {
            LOG(logINFO, ("Switching off Image Test Mode\n"));
            eiger_virtual_test_mode = 0;
        } else {
            LOG(logINFO, ("Switching on Image Test Mode\n"));
            eiger_virtual_test_mode = 1;
        }
    }
}

int getTestImageMode() { return eiger_virtual_test_mode; }
#endif

/* Ids */

void getServerVersion(char *version) { strcpy(version, APIEIGER); }

u_int64_t getFirmwareVersion() {
#ifdef VIRTUAL
    return REQUIRED_FIRMWARE_VERSION;
#else
    return Beb_GetFirmwareRevision();
#endif
}

uint64_t getFrontEndFirmwareVersion(enum fpgaPosition fpgaPosition) {
    uint64_t retval = 0;
#ifdef VIRTUAL
    return REQUIRED_FIRMWARE_VERSION;
#else
    sharedMemory_lockLocalLink();
    switch (fpgaPosition) {
    case FRONT_LEFT:
        retval = Feb_Control_GetFrontLeftFirmwareVersion(fpgaPosition);
        break;
    case FRONT_RIGHT:
        retval = Feb_Control_GetFrontRightFirmwareVersion(fpgaPosition);
        break;
    default:
        LOG(logERROR,
            ("unknown index for fpga position to read firmware version\n"));
        retval = 0;
    }
    sharedMemory_unlockLocalLink();
#endif
    return retval;
}

u_int64_t getFirmwareAPIVersion() {
#ifdef VIRTUAL
    return REQUIRED_FIRMWARE_VERSION;
#else
    return Beb_GetFirmwareSoftwareAPIVersion();
#endif
}

void getHardwareVersion(char *version) {
    strcpy(version, "unknown");
    int hwversion = getHardwareVersionNumber();
    const int hwNumberList[] = HARDWARE_VERSION_NUMBERS;
    const char *hwNamesList[] = HARDWARE_VERSION_NAMES;
    for (int i = 0; i != NUM_HARDWARE_VERSIONS; ++i) {
        LOG(logDEBUG, ("0x%x %d 0x%x %s\n", hwversion, i, hwNumberList[i],
                       hwNamesList[i]));
        if (hwNumberList[i] == hwversion) {
            strcpy(version, hwNamesList[i]);
            return;
        }
    }
}

int getHardwareVersionNumber() {
    int retval = 0;
#ifndef VIRTUAL
    sharedMemory_lockLocalLink();
    if (!Feb_Control_GetFPGAHardwareVersion(&retval)) {
        retval = -1;
    }
    sharedMemory_unlockLocalLink();
#endif
    return retval;
}

int getModuleId(int *ret, char *mess) {
    return getModuleIdInFile(ret, mess, ID_FILE);
}

int updateModuleId() {
    int modid = getModuleIdInFile(&initError, initErrorMessage, ID_FILE);
    if (initError == FAIL) {
        return FAIL;
    }
#ifdef VIRTUAL
    eiger_virtual_module_id = modid;
#else
    if (Beb_SetModuleId(modid) == FAIL) {
        initError = FAIL;
        strcpy(initErrorMessage, ("Could not get module id from the file"));
        return FAIL;
    }
#endif
    return OK;
}

u_int64_t getDetectorMAC() {
    char mac[255] = "";
    u_int64_t res = 0;

    // execute and get address
    char output[255];
#ifdef VIRTUAL
    FILE *sysFile =
        popen("cat /sys/class/net/$(ip route show default | grep -v vpn  | awk "
              "'/default/ {print $5}')/address",
              "r");
#else
    FILE *sysFile = popen("more /sys/class/net/eth0/address", "r");
#endif
    // FILE* sysFile = popen("ifconfig eth0 | grep HWaddr | cut -d \" \" -f 11",
    // "r");
    fgets(output, sizeof(output), sysFile);
    pclose(sysFile);

    // getting rid of ":"
    char *pch;
    pch = strtok(output, ":");
    while (pch != NULL) {
        strcat(mac, pch);
        pch = strtok(NULL, ":");
    }
#ifdef VIRTUAL
    sscanf(mac, "%lx", &res);
#else
    sscanf(mac, "%llx", &res);
#endif
    // increment by 1 for 10g
    if (send_to_ten_gig)
        res++;
    // LOG(logINFO, ("mac:%llx\n",res));

    return res;
}

u_int32_t getDetectorIP() {
    char temp[INET_ADDRSTRLEN] = "";
    u_int32_t res = 0;
    // execute and get address
    char output[255];
#ifdef VIRTUAL
    FILE *sysFile =
        popen("ifconfig $(ip route show default | grep -v vpn | awk '/default/ "
              "{print $5}') | grep 'inet ' | cut -d ' ' -f10",
              "r");
#else
    FILE *sysFile = popen(
        "ifconfig  | grep 'inet addr:'| grep -v '127.0.0.1' | cut -d: -f2",
        "r");
#endif
    fgets(output, sizeof(output), sysFile);
    pclose(sysFile);
    if (strlen(output) <= 1) {
        return 0;
    }

    // converting IPaddress to hex.
    char *pcword = strtok(output, ".");
    while (pcword != NULL) {
        sprintf(output, "%02x", atoi(pcword));
        strcat(temp, output);
        pcword = strtok(NULL, ".");
    }
    strcpy(output, temp);
    sscanf(output, "%x", &res);
    // LOG(logINFO, ("ip:%x\n",res));

    return res;
}

/* initialization */

void initControlServer() {
    LOG(logINFOBLUE, ("Configuring Control server\n"));
    if (!updateFlag && initError == OK) {
        if (updateModuleConfiguration() == FAIL) {
            initCheckDone = 1;
            return;
        }
#ifndef VIRTUAL
        setupFebBeb();
#endif
        // also reads config file and deactivates
        setupDetector();
    }
    initCheckDone = 1;
}

void initStopServer() {
    if (!updateFlag && initError == OK) {
        // wait a few s (control server is setting top/master from config file/
        // command line)
        usleep(WAIT_STOP_SERVER_START);
        LOG(logINFOBLUE, ("Configuring Stop server\n"));
        // ismaster from variable in stop server
        if (updateModuleConfiguration() == FAIL) {
            initCheckDone = 1;
            return;
        }
#ifdef VIRTUAL
        setupDetector();
#else
        setupFebBeb();
        // client first connect (from shm) will activate
        if (setActivate(0) == FAIL) {
            initError = FAIL;
            strcpy(initErrorMessage, "Could not deactivate\n");
            LOG(logERROR, (initErrorMessage));
        }
#endif
    }
    initCheckDone = 1;
}

void checkVirtual9MFlag() {
#ifdef VIRTUAL
#ifdef VIRTUAL_9M
    normal = 0;
#else
    normal = 1;
#endif
#endif
}

int updateModuleConfiguration() {
    if (getModuleConfiguration(&master, &top, &normal) == FAIL) {
        return FAIL;
    }
#ifdef VIRTUAL
    checkVirtual9MFlag();
#endif
    if (isControlServer) {
        LOG(logINFOBLUE,
            ("Module: %s %s %s\n", (top ? "TOP" : "BOTTOM"),
             (master ? "MASTER" : "SLAVE"), (normal ? "NORMAL" : "SPECIAL")));
    }
    return OK;
}

int getModuleConfiguration(int *m, int *t, int *n) {
    if (initError == FAIL) {
        return FAIL;
    }
#ifdef VIRTUAL
    *m = master;
    *t = top;
    *n = normal;
#else
    if (Beb_GetModuleConfiguration(m, t, n) == FAIL) {
        initError = FAIL;
        strcpy(initErrorMessage, ("Could not get module configuration\n"));
        LOG(logERROR, (initErrorMessage));
        return FAIL;
    }
#endif
    LOG(logDEBUG,
        ("module config read: master:%d top:%d normal:%d\n", *m, *t, *n));
    return OK;
}

int readConfigFile() {
    if (initError == FAIL) {
        return initError;
    }

    if (ignoreConfigFileFlag) {
        LOG(logWARNING, ("Ignoring Config file\n"));
        return OK;
    }

#ifndef VIRTUAL
    // if not found in config file, they will be reset to hardware settings
    top = -1;
    master = -1;
#endif

    const int fileNameSize = 128;
    char fname[fileNameSize];
    if (getAbsPath(fname, fileNameSize, CONFIG_FILE) == FAIL) {
        return FAIL;
    }

    // open config file
    FILE *fd = fopen(fname, "r");
    if (fd == NULL) {
        LOG(logINFO, ("No config file found. Resetting to hardware settings "
                      "(Top/Master)\n"));
        // reset to hardware settings if not in config file (if overwritten)
        resetToHardwareSettings();
        return initError;
    }
    LOG(logINFO, ("Reading config file %s\n", CONFIG_FILE));

    // Initialization
    const size_t LZ = 256;
    char line[LZ];
    memset(line, 0, LZ);
    char command[LZ];

    // keep reading a line
    while (fgets(line, LZ, fd)) {
        // ignore comments
        if (line[0] == '#') {
            LOG(logDEBUG1, ("Ignoring Comment\n"));
            continue;
        }

        // ignore empty lines
        if (strlen(line) <= 1) {
            LOG(logDEBUG1, ("Ignoring Empty line\n"));
            continue;
        }

        // ignoring lines beginning with space or tab
        if (line[0] == ' ' || line[0] == '\t') {
            LOG(logDEBUG1, ("Ignoring Lines starting with space or tabs\n"));
            continue;
        }

        LOG(logDEBUG1, ("Command to process: (size:%d) %.*s\n", strlen(line),
                        strlen(line) - 1, line));
        memset(command, 0, LZ);

        // top command
        if (!strncmp(line, "top", strlen("top"))) {
            int t = -1;
            // cannot scan values
            if (sscanf(line, "%s %d", command, &t) != 2) {
                sprintf(initErrorMessage,
                        "Could not scan top commands from on-board server "
                        "config file. Line:[%s].\n",
                        line);
                break;
            }
            if (t != 0 && t != 1) {
                sprintf(initErrorMessage,
                        "Invalid top argument from on-board server "
                        "config file. Line:[%s].\n",
                        line);
                break;
            }
            if (setTop(t == 1 ? OW_TOP : OW_BOTTOM) == FAIL) {
                sprintf(initErrorMessage,
                        "Could not set top from config file. Line:[%s].\n",
                        line);
                break;
            }
        }

        // master command
        else if (!strncmp(line, "master", strlen("master"))) {
            int m = -1;
            // cannot scan values
            if (sscanf(line, "%s %d", command, &m) != 2) {
                sprintf(initErrorMessage,
                        "Could not scan master commands from on-board server "
                        "config file. Line:[%s].\n",
                        line);
                break;
            }
            if (m != 0 && m != 1) {
                sprintf(initErrorMessage,
                        "Invalid master argument from on-board server "
                        "config file. Line:[%s].\n",
                        line);
                break;
            }
            if (setMaster(m == 1 ? OW_MASTER : OW_SLAVE) == FAIL) {
                sprintf(initErrorMessage,
                        "Could not set master from config file. Line:[%s].\n",
                        line);
                break;
            }
        }

        // other commands
        else {
            sprintf(initErrorMessage,
                    "Could not scan command from on-board server "
                    "config file. Line:[%s].\n",
                    line);
            break;
        }
    }
    fclose(fd);

    if (strlen(initErrorMessage)) {
        initError = FAIL;
        LOG(logERROR, ("%s\n\n", initErrorMessage));
    } else {
        LOG(logINFO, ("Successfully read config file\n"));
    }

#ifndef VIRTUAL
    // reset to hardware settings if not in config file (if overwritten)
    resetToHardwareSettings();
#endif

    return initError;
}

void resetToHardwareSettings() {
#ifndef VIRTUAL
    if (initError == FAIL) {
        return;
    }
    // top not set in config file
    if (top == -1) {
        LOG(logINFO, ("Resetting Top to hardware settings\n"));
        if (setTop(TOP_HARDWARE) == FAIL) {
            initError = FAIL;
            strcpy(initErrorMessage,
                   "Could not reset Top flag to hardware settings.\n");
            LOG(logERROR, ("%s\n\n", initErrorMessage));
            return;
        }
    }
    // master not set in config file
    if (master == -1) {
        LOG(logINFO, ("Resetting Master to hardware settings\n"));
        if (setMaster(MASTER_HARDWARE) == FAIL) {
            initError = FAIL;
            strcpy(initErrorMessage,
                   "Could not reset Master flag to hardware settings.\n");
            LOG(logERROR, ("%s\n\n", initErrorMessage));
            return;
        }
    }
#endif
}

int checkCommandLineConfiguration() {
    if (masterCommandLine != -1) {
        LOG(logINFO, ("Setting %s from Command Line\n",
                      (masterCommandLine == 1 ? "Master" : "Slave")));
        if (setMaster(masterCommandLine == 1 ? OW_MASTER : OW_SLAVE) == FAIL) {
            initError = FAIL;
            sprintf(initErrorMessage, "Could not set %s from command line.\n",
                    (masterCommandLine == 1 ? "Master" : "Slave"));
            LOG(logERROR, (initErrorMessage));
            return FAIL;
        }
    }

    if (topCommandLine != -1) {
        LOG(logINFO, ("Setting %s from Command Line\n",
                      (topCommandLine == 1 ? "Top" : "Bottom")));
        if (setTop(topCommandLine == 1 ? OW_TOP : OW_BOTTOM) == FAIL) {
            initError = FAIL;
            sprintf(initErrorMessage, "Could not set %s from command line.\n",
                    (topCommandLine == 1 ? "Top" : "Bottom"));
            LOG(logERROR, (initErrorMessage));
            return FAIL;
        }
    }
    return OK;
}

/* set up detector */

#ifndef VIRTUAL
void setupFebBeb() {
    sharedMemory_lockLocalLink();
    if (!Feb_Interface_FebInterface()) {
        initError = FAIL;
        sprintf(initErrorMessage,
                "Could not intitalize eiger detector sever: feb interface\n");
        LOG(logERROR, (initErrorMessage));
        initCheckDone = 1;
        sharedMemory_unlockLocalLink();
        return;
    }
    if (!Feb_Control_FebControl(normal)) {
        initError = FAIL;
        sprintf(initErrorMessage,
                "Could not intitalize eiger detector sever: feb control\n");
        LOG(logERROR, (initErrorMessage));
        initCheckDone = 1;
        sharedMemory_unlockLocalLink();
        return;
    }
    if (Feb_Control_SetMasterEffects(master, isControlServer) == FAIL) {
        initError = FAIL;
        sprintf(initErrorMessage, "Could not intitalize HV for eiger "
                                  "detector server: feb control serial "
                                  "communication\n");
        LOG(logERROR, (initErrorMessage));
        initCheckDone = 1;
        sharedMemory_unlockLocalLink();
        return;
    }
    sharedMemory_unlockLocalLink();
    LOG(logDEBUG1, ("%s server: FEB Initialization done\n",
                    isControlServer ? "Control" : "Stop"));
    Beb_SetTopVariable(top);
    if (!Beb_Beb()) {
        initError = FAIL;
        sprintf(initErrorMessage,
                "Could not intitalize eiger detector sever: beb\n");
        LOG(logERROR, (initErrorMessage));
        initCheckDone = 1;
        return;
    }
    LOG(logDEBUG1, ("%s server: BEB Initialization done\n",
                    isControlServer ? "Control" : "Stop"));

    if (isControlServer) {
        // Getting the feb versions after initialization
        char hversion[MAX_STR_LENGTH] = {0};
        memset(hversion, 0, MAX_STR_LENGTH);
        getHardwareVersion(hversion);
        int64_t fwversion = getFirmwareVersion();
        int64_t feblfwversion = getFrontEndFirmwareVersion(FRONT_LEFT);
        int64_t febrfwversion = getFrontEndFirmwareVersion(FRONT_RIGHT);
        LOG(logINFOBLUE,
            ("\n********************************************************\n"
             "Feb Versions\n"
             "Hardware Version         : %s\n"
             "Firmware (Febl) Version  : %lld\n"
             "Firmware (Febr) Version  : %lld\n"
             "********************************************************\n",
             hversion, (long long int)feblfwversion,
             (long long int)febrfwversion));

        // ensure febl, febr and beb fw versions are the same
        if (fwversion != feblfwversion || fwversion != febrfwversion) {
            sprintf(
                initErrorMessage,
                "Inconsistent firmware versions in feb and beb. [Beb: %lld, "
                "Febl: %lld Febr: %lld]\n",
                (long long int)fwversion, (long long int)feblfwversion,
                (long long int)febrfwversion);
            LOG(logERROR, (initErrorMessage));
            initError = FAIL;
            return;
        }
    }
}
#endif

int allocateDetectorStructureMemory() {
    detectorModules = malloc(sizeof(sls_detector_module));
    detectorChans = malloc(NCHIP * NCHAN * sizeof(int));
    detectorDacs = malloc(NDAC * sizeof(int));
    if (detectorModules == NULL || detectorChans == NULL ||
        detectorDacs == NULL) {
        initError = FAIL;
        strcpy(initErrorMessage,
               "Could not allocate memory for dacs or channels in detector\n");
        LOG(logERROR, (initErrorMessage));
        return FAIL;
    }
    LOG(logDEBUG1,
        ("modules from 0x%x to 0x%x\n", detectorModules, detectorModules));
    LOG(logDEBUG1, ("chans from 0x%x to 0x%x\n", detectorChans, detectorChans));
    LOG(logDEBUG1, ("dacs from 0x%x to 0x%x\n", detectorDacs, detectorDacs));

    detectorModules->dacs = detectorDacs;
    detectorModules->chanregs = detectorChans;
    detectorModules->ndac = NDAC;
    detectorModules->nchip = NCHIP;
    detectorModules->nchan = NCHIP * NCHAN;
    detectorModules->reg = 0;
    detectorModules->iodelay = 0;
    detectorModules->tau = 0;
    detectorModules->eV[0] = -1;
    detectorModules->eV[1] = -1;
    detectorModules->eV[2] = -1;
    thisSettings = UNINITIALIZED;

    // initialize (trimbits at -1 for unknown)
    for (int idac = 0; idac < (detectorModules)->ndac; ++idac) {
        detectorDacs[idac] = 0;
    }
    for (int ichan = 0; ichan < (detectorModules->nchan); ichan++) {
        *((detectorModules->chanregs) + ichan) = -1;
    }
    return OK;
}

void setupDetector() {
    LOG(logINFO, ("This Server is for 1 Eiger half module (250k)\n\n"));

    if (allocateDetectorStructureMemory() == FAIL)
        return;

    // force top or master if in config file
    if (readConfigFile() == FAIL)
        return;
    // force top or master if in command line
    if (checkCommandLineConfiguration() == FAIL)
        return;

    LOG(logINFOBLUE,
        ("Module: %s %s %s\n", (top ? "TOP" : "BOTTOM"),
         (master ? "MASTER" : "SLAVE"), (normal ? "NORMAL" : "SPECIAL")));

    if (updateModuleId() == FAIL)
        return;

    LOG(logINFOBLUE, ("Setting Default Parameters\n"));
    resetToDefaultDacs(0);
#ifdef VIRTUAL
    if (isControlServer) {
        sharedMemory_setStatus(IDLE);
        setupUDPCommParameters();
    } else {
        sharedMemory_setStop(0);
    }
#endif

    // setting default measurement parameters
    setNumFrames(DEFAULT_NUM_FRAMES);
    setExpTime(DEFAULT_EXPTIME);
    setSubExpTime(DEFAULT_SUBFRAME_EXPOSURE);
    getSubExpTime(DEFAULT_SUBFRAME_DEADTIME);
    setPeriod(DEFAULT_PERIOD);
    setNumTriggers(DEFAULT_NUM_CYCLES);
    setDynamicRange(DEFAULT_DYNAMIC_RANGE);
    detectorModules->eV[0] = DEFAULT_PHOTON_ENERGY;
    setParallelMode(DEFAULT_PARALLEL_MODE);
    setOverFlowMode(DEFAULT_READOUT_OVERFLOW32_MODE);
    setReadoutSpeed(DEFAULT_CLK_SPEED);
    setIODelay(DEFAULT_IO_DELAY);
    setTiming(DEFAULT_TIMING_MODE);
    setNextFrameNumber(DEFAULT_STARTING_FRAME_NUMBER);
    setReadNRows(MAX_ROWS_PER_READOUT);
    // SetPhotonEnergyCalibrationParameters(-5.8381e-5,1.838515,5.09948e-7,-4.32390e-11,1.32527e-15);
    eiger_tau_ns = DEFAULT_RATE_CORRECTION;
    setRateCorrection(DEFAULT_RATE_CORRECTION);
    int enable[2] = {DEFAULT_EXT_GATING_ENABLE, DEFAULT_EXT_GATING_POLARITY};
    setExternalGating(enable); // disable external gating
#ifndef VIRTUAL
    sharedMemory_lockLocalLink();
    Feb_Control_SetInTestModeVariable(DEFAULT_TEST_MODE);
    sharedMemory_unlockLocalLink();
#endif
    setHighVoltage(DEFAULT_HIGH_VOLTAGE);
#ifndef VIRTUAL
    sharedMemory_lockLocalLink();
    if (!Feb_Control_CheckSetup()) {
        initError = FAIL;
        sprintf(initErrorMessage, "Could not pass feb control setup checks\n");
        LOG(logERROR, (initErrorMessage));
        sharedMemory_unlockLocalLink();
        return;
    }
    sharedMemory_unlockLocalLink();
#endif
    if (setNumberofDestinations(numUdpDestinations) == FAIL) {
        initError = FAIL;
        strcpy(initErrorMessage, "Could not set number of udp destinations\n");
        LOG(logERROR, (initErrorMessage));
    }

    // client first connect (from shm) will activate
    if (setActivate(0) == FAIL) {
        initError = FAIL;
        strcpy(initErrorMessage, "Could not deactivate\n");
        LOG(logERROR, (initErrorMessage));
    }
    LOG(logDEBUG1, ("Setup detector done\n\n"));
}

int resetToDefaultDacs(int hardReset) {
    // reset defaults to hardcoded defaults
    if (hardReset) {
        const int vals[] = DEFAULT_DAC_VALS;
        for (int i = 0; i < NDAC; ++i) {
            defaultDacValues[i] = vals[i];
        }
    }
    // reset dacs to defaults
    int ret = OK;
    LOG(logINFOBLUE, ("Setting Default Dac values\n"));
    for (int i = 0; i < NDAC; ++i) {
        setDAC((enum DACINDEX)i, defaultDacValues[i], 0);
        if ((detectorModules)->dacs[i] != defaultDacValues[i]) {
            ret = FAIL;
            LOG(logERROR, ("Setting dac %d failed, wrote %d, read %d\n", i,
                           defaultDacValues[i], (detectorModules)->dacs[i]));
        }
    }
    return ret;
}

int getDefaultDac(enum DACINDEX index, enum detectorSettings sett,
                  int *retval) {
    if (sett != UNDEFINED) {
        return FAIL;
    }
    if (index < 0 || index >= NDAC)
        return FAIL;
    *retval = defaultDacValues[index];
    return OK;
}

int setDefaultDac(enum DACINDEX index, enum detectorSettings sett, int value) {
    if (sett != UNDEFINED) {
        return FAIL;
    }
    if (index < 0 || index >= NDAC)
        return FAIL;

    char *dac_names[] = {DAC_NAMES};
    LOG(logINFO, ("Setting Default Dac [%d - %s]: %d\n", (int)index,
                  dac_names[index], value));
    defaultDacValues[index] = value;
    return OK;
}

/* advanced read/write reg */
int writeRegister(uint32_t offset, uint32_t data, int validate) {
#ifdef VIRTUAL
    return OK;
#else
    sharedMemory_lockLocalLink();
    if (!Feb_Control_WriteRegister(offset, data, validate)) {
        sharedMemory_unlockLocalLink();
        return FAIL;
    }
    sharedMemory_unlockLocalLink();
    return OK;
#endif
}

int readRegister(uint32_t offset, uint32_t *retval) {
#ifdef VIRTUAL
    return OK;
#else
    sharedMemory_lockLocalLink();
    if (!Feb_Control_ReadRegister(offset, retval)) {
        sharedMemory_unlockLocalLink();
        return FAIL;
    }
    sharedMemory_unlockLocalLink();
    return OK;
#endif
}

int setBit(const uint32_t addr, const int nBit, int validate) {
#ifndef VIRTUAL
    uint32_t regval = 0;
    if (readRegister(addr, &regval) == FAIL) {
        return FAIL;
    }
    uint32_t bitmask = (1 << nBit);
    uint32_t val = regval | bitmask;

    sharedMemory_lockLocalLink();
    if (!Feb_Control_WriteRegister_BitMask(addr, val, bitmask, validate)) {
        sharedMemory_unlockLocalLink();
        return FAIL;
    }
    sharedMemory_unlockLocalLink();
#endif
    return OK;
}

int clearBit(const uint32_t addr, const int nBit, int validate) {
#ifndef VIRTUAL
    uint32_t regval = 0;
    if (readRegister(addr, &regval) == FAIL) {
        return FAIL;
    }
    uint32_t bitmask = (1 << nBit);
    uint32_t val = regval & ~bitmask;

    sharedMemory_lockLocalLink();
    if (!Feb_Control_WriteRegister_BitMask(addr, val, bitmask, validate)) {
        sharedMemory_unlockLocalLink();
        return FAIL;
    }
    sharedMemory_unlockLocalLink();
#endif
    return OK;
}

int getBit(const uint32_t addr, const int nBit, int *retval) {
#ifndef VIRTUAL
    uint32_t regval = 0;
    uint32_t bitmask = (1 << nBit);
    sharedMemory_lockLocalLink();
    if (!Feb_Control_ReadRegister_BitMask(addr, &regval, bitmask)) {
        sharedMemory_unlockLocalLink();
        return FAIL;
    }
    sharedMemory_unlockLocalLink();
    *retval = (regval >> nBit);
#endif
    return OK;
}

/* set parameters -  dr, roi */

int setDynamicRange(int dr) {
    if (dr <= 0) {
        return FAIL;
    }
#ifdef VIRTUAL
    LOG(logINFO, ("Setting dynamic range: %d\n", dr));
#else
    sharedMemory_lockLocalLink();
    if (!Feb_Control_SetDynamicRange(dr)) {
        LOG(logERROR, ("Could not set dynamic range in feb\n"));
        sharedMemory_unlockLocalLink();
        return FAIL;
    }
    if (!Beb_SetUpTransferParameters(dr)) {
        LOG(logERROR, ("Could not set bit mode in the back end\n"));
        sharedMemory_unlockLocalLink();
        return eiger_dynamicrange;
    }
    sharedMemory_unlockLocalLink();
#endif
    eiger_dynamicrange = dr;
    return OK;
}

int getDynamicRange(int *retval) {
#ifdef VIRTUAL
    *retval = eiger_dynamicrange;
#else
    sharedMemory_lockLocalLink();
    if (!Feb_Control_GetDynamicRange(retval)) {
        sharedMemory_unlockLocalLink();
        return FAIL;
    }
    sharedMemory_unlockLocalLink();
#endif
    return OK;
}

/* parameters - readout */

int setParallelMode(int mode) {
    mode = (mode == 0 ? E_NON_PARALLEL : E_PARALLEL);
#ifndef VIRTUAL
    sharedMemory_lockLocalLink();
    if (!Feb_Control_SetReadoutMode(mode)) {
        sharedMemory_unlockLocalLink();
        return FAIL;
    }
    sharedMemory_unlockLocalLink();
#endif
    eiger_parallelmode = mode;
    return OK;
}

int getParallelMode() { return (eiger_parallelmode == E_PARALLEL ? 1 : 0); }

int setOverFlowMode(int mode) {
    mode = (mode == 0 ? 0 : 1);
#ifndef VIRTUAL
    if (Beb_Set32bitOverflow(mode == 0 ? 0 : 1) == -1) {
        return FAIL;
    }
#endif
    eiger_overflow32 = mode;
    return OK;
}

int getOverFlowMode() { return eiger_overflow32; }

/* parameters - timer */

int setNextFrameNumber(uint64_t value) {
#ifdef VIRTUAL
    eiger_virtual_nextframenumber = value;
    return OK;
#else
    return Beb_SetNextFrameNumber(value);
#endif
}

int getNextFrameNumber(uint64_t *retval) {
#ifdef VIRTUAL
    *retval = eiger_virtual_nextframenumber;
    return OK;
#else
    return Beb_GetNextFrameNumber(retval, send_to_ten_gig);
#endif
}

void setNumFrames(int64_t val) {
    if (val > 0) {
        LOG(logINFO, ("Setting number of frames %lld\n", (long long int)val));
#ifndef VIRTUAL
        sharedMemory_lockLocalLink();
        if (Feb_Control_SetNExposures((unsigned int)val * eiger_ntriggers)) {
            eiger_nexposures = val;
            nimages_per_request = eiger_nexposures * eiger_ntriggers;
        }
        sharedMemory_unlockLocalLink();
#else
        eiger_nexposures = val;
        nimages_per_request = eiger_nexposures * eiger_ntriggers;
#endif
    }
}

int64_t getNumFrames() { return eiger_nexposures; }

void setNumTriggers(int64_t val) {
    if (val > 0) {
        LOG(logINFO, ("Setting number of triggers %lld\n", (long long int)val));
#ifndef VIRTUAL
        sharedMemory_lockLocalLink();
        if (Feb_Control_SetNExposures((unsigned int)val * eiger_nexposures)) {
            eiger_ntriggers = val;
            nimages_per_request = eiger_nexposures * eiger_ntriggers;
        }
        sharedMemory_unlockLocalLink();
#else
        eiger_ntriggers = val;
        nimages_per_request = eiger_nexposures * eiger_ntriggers;
#endif
    }
}

int64_t getNumTriggers() { return eiger_ntriggers; }

int setExpTime(int64_t val) {
    LOG(logINFO, ("Setting exptime %lld ns\n", (long long int)val));
#ifndef VIRTUAL
    sharedMemory_lockLocalLink();
    Feb_Control_SetExposureTime(val / (1E9));
    sharedMemory_unlockLocalLink();
#else
    eiger_virtual_exptime = val;
#endif
    return OK;
}

int64_t getExpTime() {
#ifndef VIRTUAL
    sharedMemory_lockLocalLink();
    int64_t retval = (Feb_Control_GetExposureTime() * (1E9));
    sharedMemory_unlockLocalLink();
    return retval;
#else
    return eiger_virtual_exptime;
#endif
}

int setPeriod(int64_t val) {
    LOG(logINFO, ("Setting period %lld ns\n", (long long int)val));
#ifndef VIRTUAL
    sharedMemory_lockLocalLink();
    Feb_Control_SetExposurePeriod(val / (1E9));
    sharedMemory_unlockLocalLink();
#else
    eiger_virtual_period = val;
#endif
    return OK;
}

int64_t getPeriod() {
#ifndef VIRTUAL
    sharedMemory_lockLocalLink();
    int64_t retval = (Feb_Control_GetExposurePeriod() * (1E9));
    sharedMemory_unlockLocalLink();
    return retval;
#else
    return eiger_virtual_period;
#endif
}

int setSubExpTime(int64_t val) {
    LOG(logINFO, ("Setting subexptime %lld ns\n", (long long int)val));
#ifndef VIRTUAL
    sharedMemory_lockLocalLink();
    // calculate subdeadtime before settings subexptime
    int64_t subdeadtime =
        Feb_Control_GetSubFramePeriod() - Feb_Control_GetSubFrameExposureTime();
    Feb_Control_SetSubFrameExposureTime(val / 10);
    // set subperiod
    Feb_Control_SetSubFramePeriod((val + subdeadtime) / 10);
    sharedMemory_unlockLocalLink();
#else
    int64_t subdeadtime =
        eiger_virtual_subperiod * 10 - eiger_virtual_subexptime * 10;
    eiger_virtual_subexptime = (val / (10));
    eiger_virtual_subperiod = (val + subdeadtime) / 10;
#endif
    return OK;
}

int64_t getSubExpTime() {
#ifndef VIRTUAL
    sharedMemory_lockLocalLink();
    int64_t retval = (Feb_Control_GetSubFrameExposureTime());
    sharedMemory_unlockLocalLink();
    return retval;
#else
    return eiger_virtual_subexptime * 10;
#endif
}

int setSubDeadTime(int64_t val) {
    LOG(logINFO, ("Setting subdeadtime %lld ns\n", (long long int)val));
#ifndef VIRTUAL
    sharedMemory_lockLocalLink();
    // get subexptime
    int64_t subexptime = Feb_Control_GetSubFrameExposureTime();
    sharedMemory_unlockLocalLink();
#else
    int64_t subexptime = eiger_virtual_subexptime * 10;
#endif
    LOG(logINFO,
        ("Setting sub period (subdeadtime(%lld)): %lldns\n",
         (long long int)subexptime, (long long int)val),
        (long long int)(val + subexptime));
    // calculate subperiod
    val += subexptime;
#ifndef VIRTUAL
    sharedMemory_lockLocalLink();
    Feb_Control_SetSubFramePeriod(val / 10);
    sharedMemory_unlockLocalLink();
#else
    eiger_virtual_subperiod = (val / 10);
#endif
    return OK;
}

int64_t getSubDeadTime() {
#ifndef VIRTUAL
    sharedMemory_lockLocalLink();
    // get subexptime
    int64_t subexptime = Feb_Control_GetSubFrameExposureTime();
    sharedMemory_unlockLocalLink();
#else
    int64_t subexptime = eiger_virtual_subexptime * 10;
#endif
#ifndef VIRTUAL
    sharedMemory_lockLocalLink();
    int64_t retval = (Feb_Control_GetSubFramePeriod() - subexptime);
    sharedMemory_unlockLocalLink();
    return retval;
#else
    return (eiger_virtual_subperiod * 10 - subexptime);
#endif
}

int64_t getMeasuredPeriod() {
#ifdef VIRTUAL
    return 0;
#else
    sharedMemory_lockLocalLink();
    int64_t retval = Feb_Control_GetMeasuredPeriod();
    sharedMemory_unlockLocalLink();
    return retval;
#endif
}

int64_t getMeasuredSubPeriod() {
#ifdef VIRTUAL
    return 0;
#else
    sharedMemory_lockLocalLink();
    int64_t retval = Feb_Control_GetSubMeasuredPeriod();
    sharedMemory_unlockLocalLink();
    return retval;
#endif
}

/* parameters - channel, module, settings */

void getModule(sls_detector_module *myMod) {
    // serial number
    myMod->serialnumber = detectorModules->serialnumber;
    // reg (settings)
    myMod->reg = detectorModules->reg;
    // iodelay
    myMod->iodelay = setIODelay(-1);
    // tau
    myMod->tau = (int)getCurrentTau();
    // eV
    myMod->eV[0] = detectorModules->eV[0];
    // dacs
    for (int idac = 0; idac < (detectorModules->ndac); idac++) {
        *((myMod->dacs) + idac) = *((detectorModules->dacs) + idac);
    }
    // trimbits
    for (int ichan = 0; ichan < (detectorModules->nchan); ichan++) {
        *((myMod->chanregs) + ichan) = *((detectorModules->chanregs) + ichan);
    }
}

int setModule(sls_detector_module myMod, char *mess) {

    LOG(logINFO, ("Setting module with settings %d\n", myMod.reg));

    if (((myMod.nchan) > (detectorModules->nchan)) ||
        ((myMod.ndac) > (detectorModules->ndac))) {
        strcpy(mess, "Could not set module as the number of channels or dacs "
                     "do not match to the one in the detector server\n");
        LOG(logERROR, (mess));
        return FAIL;
    }

    // serial number (pointless)
    detectorModules->serialnumber = myMod.serialnumber;

    // settings
    setSettings((enum detectorSettings)myMod.reg);

    // iodelay
    if (setIODelay(myMod.iodelay) != myMod.iodelay) {
        sprintf(mess, "Could not set module. Could not set iodelay %d\n",
                myMod.iodelay);
        LOG(logERROR, (mess));
        setSettings(UNDEFINED);
        LOG(logERROR, ("Settings has been changed to undefined\n"));
        return FAIL;
    }

    // threshold
    if (myMod.eV[0] >= 0)
        setThresholdEnergy(myMod.eV[0]);
    else {
        // (loading a random trim file) (dont return fail)
        setSettings(UNDEFINED);
        LOG(logERROR,
            ("Settings has been changed to undefined (random trim file)\n"));
    }

    // dacs
    for (int i = 0; i < NDAC; ++i) {
        setDAC((enum DACINDEX)i, myMod.dacs[i], 0);
        if (myMod.dacs[i] != (detectorModules)->dacs[i]) {
            sprintf(mess, "Could not set module. Could not set dac %d\n", i);
            LOG(logERROR, (mess));
            setSettings(UNDEFINED);
            LOG(logERROR, ("Settings has been changed to undefined\n"));
            return FAIL;
        }
    }

    // trimbits
#ifndef VIRTUAL
    if (myMod.nchan == 0) {
        LOG(logINFO, ("Setting module without trimbits\n"));
    } else {
        LOG(logINFO, ("Setting module with trimbits\n"));
        if (setTrimbits(myMod.chanregs, mess) == FAIL) {
            return FAIL;
        }
    }
#endif

    // rate correction
    // switch off rate correction: no value read from load settings)
    if (myMod.tau == -1) {
        if (getRateCorrectionEnable()) {
            setRateCorrection(0);
            sprintf(mess,
                    "Cannot set module. Cannot set Rate correction. "
                    "No default tau provided. Deactivating Rate Correction\n");
            LOG(logERROR, (mess));
            setSettings(UNDEFINED);
            LOG(logERROR, ("Settings has been changed to undefined (random "
                           "trim file)\n"));
            return FAIL;
        }
    }
    // normal tau value (only if enabled)
    else {
        setDefaultSettingsTau_in_nsec(myMod.tau);
        if (getRateCorrectionEnable()) {
            if (setRateCorrection(myMod.tau) == FAIL) {
                sprintf(mess, "Cannot set module. Rate correction failed.\n");
                LOG(logERROR, (mess));
                setSettings(UNDEFINED);
                LOG(logERROR, ("Settings has been changed to undefined (random "
                               "trim file)\n"));
                return FAIL;
            } else {
                int64_t retvalTau = getCurrentTau();
                if (myMod.tau != retvalTau) {
                    sprintf(
                        mess,
                        "Cannot set module. Could not set rate correction\n");
                    LOG(logERROR, (mess));
                    setSettings(UNDEFINED);
                    LOG(logERROR, ("Settings has been changed to undefined "
                                   "(random trim file)\n"));
                    return FAIL;
                }
            }
        }
    }
    return OK;
}

enum detectorSettings setSettings(enum detectorSettings sett) {
    if (sett == UNINITIALIZED) {
        return thisSettings;
    }
    thisSettings = sett;
    detectorModules->reg = sett;
    LOG(logINFO, ("Settings: %d\n", thisSettings));
    return thisSettings;
}

enum detectorSettings getSettings() { return thisSettings; }

/* parameters - threshold */

int getThresholdEnergy() {
    LOG(logDEBUG1, ("Getting Threshold energy\n"));
    return detectorModules->eV[0];
}

int setThresholdEnergy(int ev) {
    LOG(logINFO, ("Setting threshold energy:%d\n", ev));
    if (ev >= 0) {
        detectorModules->eV[0] = ev;
    }
    return getThresholdEnergy();
}

/* parameters - dac, adc, hv */

// uses LTC2620 with 2.048V (implementation different to others not bit banging)
void setDAC(enum DACINDEX ind, int val, int mV) {
    if (val < 0)
        return;

    LOG(logDEBUG1, ("Setting dac[%d]: %d %s \n", (int)ind, val,
                    (mV ? "mV" : "dac units")));

    if (ind == E_VTHRESHOLD) {
        setDAC(E_VCMP_LL, val, mV);
        setDAC(E_VCMP_LR, val, mV);
        setDAC(E_VCMP_RL, val, mV);
        setDAC(E_VCMP_RR, val, mV);
        setDAC(E_VCP, val, mV);
        return;
    }

    // validate index
    if (ind < 0 || ind >= NDAC) {
        LOG(logERROR,
            ("\tDac index %d is out of bounds (0 to %d)\n", ind, NDAC - 1));
        return;
    }

    char *dac_names[] = {DAC_NAMES};
    LOG(logINFO, ("Setting dac[%d - %s]: %d %s \n", (int)ind, dac_names[ind],
                  val, (mV ? "mV" : "dac units")));

#ifdef VIRTUAL
    int dacval = 0;
    if (!mV) {
        (detectorModules)->dacs[ind] = val;
    }
    // convert to dac units
    else if (ConvertToDifferentRange(DAC_MIN_MV, DAC_MAX_MV, LTC2620_MIN_VAL,
                                     LTC2620_MAX_VAL, val, &dacval) == OK) {
        (detectorModules)->dacs[ind] = dacval;
    }
#else
    int dacval = val;
    if (mV) {
        // convert to dac units
        if (ConvertToDifferentRange(DAC_MIN_MV, DAC_MAX_MV, LTC2620_MIN_VAL,
                                    LTC2620_MAX_VAL, val, &dacval) == FAIL) {
            LOG(logERROR,
                ("Could not convert %d mV for dac to dac units\n", val));
            return;
        }
    }
    sharedMemory_lockLocalLink();
    if (Feb_Control_SetDAC(ind, dacval)) {
        (detectorModules)->dacs[ind] = dacval;
    }
    sharedMemory_unlockLocalLink();
#endif
}

int getDAC(enum DACINDEX ind, int mV) {
    if (ind == E_VTHRESHOLD) {
        int ret[5] = {0};
        ret[0] = getDAC(E_VCMP_LL, mV);
        ret[1] = getDAC(E_VCMP_LR, mV);
        ret[2] = getDAC(E_VCMP_RL, mV);
        ret[3] = getDAC(E_VCMP_RR, mV);
        ret[4] = getDAC(E_VCP, mV);

        if ((ret[0] == ret[1]) && (ret[1] == ret[2]) && (ret[2] == ret[3]) &&
            (ret[3] == ret[4])) {
            LOG(logINFO, ("\tvthreshold match\n"));
            return ret[0];
        } else {
            LOG(logERROR, ("\tvthreshold mismatch vcmp_ll:%d vcmp_lr:%d "
                           "vcmp_rl:%d vcmp_rr:%d vcp:%d\n",
                           ret[0], ret[1], ret[2], ret[3], ret[4]));
            return -1;
        }
    }

    if (!mV) {
        LOG(logDEBUG1,
            ("Getting DAC %d : %d dac\n", ind, (detectorModules)->dacs[ind]));
        return (detectorModules)->dacs[ind];
    }
    int voltage = -1;
    // dac units to voltage
    ConvertToDifferentRange(LTC2620_MIN_VAL, LTC2620_MAX_VAL, DAC_MIN_MV,
                            DAC_MAX_MV, (detectorModules)->dacs[ind], &voltage);
    LOG(logDEBUG1, ("Getting DAC %d : %d dac (%d mV)\n", ind,
                    (detectorModules)->dacs[ind], voltage));
    return voltage;
}

int getMaxDacSteps() { return DAC_MAX_STEPS; }

int getADC(enum ADCINDEX ind) {
#ifdef VIRTUAL
    return 0;
#else
    int retval = -1;
    char *adc_names[] = {ADC_NAMES};
    char cstore[255];
    memset(cstore, 0, 255);

    switch (ind) {
    case TEMP_FPGA:
        retval = getBebFPGATemp();
        break;
    case TEMP_FPGAFEBL:
        sharedMemory_lockLocalLink();
        retval = Feb_Control_GetLeftFPGATemp();
        sharedMemory_unlockLocalLink();
        break;
    case TEMP_FPGAFEBR:
        sharedMemory_lockLocalLink();
        retval = Feb_Control_GetRightFPGATemp();
        sharedMemory_unlockLocalLink();
        break;
    case TEMP_FPGAEXT:
    case TEMP_10GE:
    case TEMP_DCDC:
    case TEMP_SODL:
    case TEMP_SODR:
        sprintf(cstore, "more  /sys/class/hwmon/hwmon%d/device/temp1_input",
                ind);
        FILE *sysFile = popen(cstore, "r");
        fgets(cstore, sizeof(cstore), sysFile);
        pclose(sysFile);
        sscanf(cstore, "%d", &retval);
        break;
    default:
        return -1;
    }

    LOG(logINFO,
        ("Temperature %s: %f°C\n", adc_names[ind], (double)retval / 1000.00));

    return retval;
#endif
}

int setHighVoltage(int val) {
#ifdef VIRTUAL
    if (master) {
        // set
        if (val != -1) {
            LOG(logINFO, ("Setting High voltage: %d V\n", val));
            eiger_theo_highvoltage = val;
        }
        return eiger_theo_highvoltage;
    }

    return SLAVE_HIGH_VOLTAGE_READ_VAL;
#else

    if (master) {

        // set
        if (val != -1) {
            eiger_theo_highvoltage = val;
            sharedMemory_lockLocalLink();
            int ret = Feb_Control_SetHighVoltage(val);
            sharedMemory_unlockLocalLink();
            if (!ret) // could not set
                return -2;
            else if (ret == -1) // outside range
                return -1;
        }

        // get
        sharedMemory_lockLocalLink();
        if (!Feb_Control_GetHighVoltage(&eiger_highvoltage)) {
            LOG(logERROR, ("Could not read high voltage\n"));
            sharedMemory_unlockLocalLink();
            return -3;
        }
        // need to read the file twice to get the proper value
        if (!Feb_Control_GetHighVoltage(&eiger_highvoltage)) {
            LOG(logERROR, ("Could not read high voltage\n"));
            sharedMemory_unlockLocalLink();
            return -3;
        }
        sharedMemory_unlockLocalLink();

        // tolerance of 5
        if (abs(eiger_theo_highvoltage - eiger_highvoltage) >
            HIGH_VOLTAGE_TOLERANCE) {
            LOG(logINFO,
                ("High voltage still ramping: %d\n", eiger_highvoltage));
            return eiger_highvoltage;
        }
        return eiger_theo_highvoltage;
    }

    return SLAVE_HIGH_VOLTAGE_READ_VAL;
#endif
}

/* parameters - timing, extsig */

int setMaster(enum MASTERINDEX m) {
    char *master_names[] = {MASTER_NAMES};
    LOG(logINFOBLUE, ("Setting up Master flag as %s\n", master_names[m]));
#ifdef VIRTUAL
    switch (m) {
    case OW_MASTER:
        master = 1;
        break;
    case OW_SLAVE:
        master = 0;
        break;
    default:
        // hardware settings (do nothing)
        break;
    }
#else
    // need to set it only once via the control server
    if (isControlServer) {
        if (!Beb_SetMaster(m)) {
            return FAIL;
        }

        sharedMemory_lockLocalLink();
        if (!Feb_Control_SetMaster(m)) {
            sharedMemory_unlockLocalLink();
            return FAIL;
        }
        sharedMemory_unlockLocalLink();
    }

    // get and update master variable (cannot get from m, could be hardware)
    if (isMaster(&master) == FAIL) {
        return FAIL;
    }
    // verify for master and slave (not hardware)
    if ((m == OW_MASTER && master == 0) || (m == OW_SLAVE && master == 1)) {
        LOG(logERROR,
            ("could not set master/slave. Master value retrieved %d\n",
             master));
        return FAIL;
    }

    // feb variable and hv comms (9m)
    sharedMemory_lockLocalLink();
    if (Feb_Control_SetMasterEffects(master, isControlServer) == FAIL) {
        sharedMemory_unlockLocalLink();
        return FAIL;
    }
    sharedMemory_unlockLocalLink();
#endif
    return OK;
}

int isMaster(int *retval) {
    int m = -1, t = -1, n = -1;
    if (getModuleConfiguration(&m, &t, &n) == FAIL) {
        return FAIL;
    }
    *retval = m;
    return OK;
}

int setTop(enum TOPINDEX t) {
    char *top_names[] = {TOP_NAMES};
    LOG(logINFOBLUE, ("Setting up Top flag as %s\n", top_names[t]));
#ifdef VIRTUAL
    switch (t) {
    case OW_TOP:
        top = 1;
        break;
    case OW_BOTTOM:
        top = 0;
        break;
    default:
        // hardware settings (do nothing)
        break;
    }
#else
    if (!Beb_SetTop(t)) {
        return FAIL;
    }

    sharedMemory_lockLocalLink();
    if (!Feb_Control_SetTop(t, 1, 1)) {
        sharedMemory_unlockLocalLink();
        return FAIL;
    }
    sharedMemory_unlockLocalLink();

    // get and update top variable(cannot get from t, could be hardware)
    if (isTop(&top) == FAIL) {
        return FAIL;
    }
    // verify for master and slave (not hardware)
    if ((t == OW_TOP && top == 0) || (t == OW_BOTTOM && top == 1)) {
        LOG(logERROR,
            ("could not set top/bottom. Top value retrieved %d\n", top));
        return FAIL;
    }

    Beb_SetTopVariable(top);
#endif
    return OK;
}

int isTop(int *retval) {
    int m = -1, t = -1, n = -1;
    if (getModuleConfiguration(&m, &t, &n) == FAIL) {
        return FAIL;
    }

    *retval = t;
    return OK;
}

void setTiming(enum timingMode arg) {
    int ret = 0;
    switch (arg) {
    case AUTO_TIMING:
        ret = 0;
        break;
    case TRIGGER_EXPOSURE:
        ret = 2;
        break;
    case BURST_TRIGGER:
        ret = 1;
        break;
    case GATED:
        ret = 3;
        break;
    default:
        LOG(logERROR, ("Unknown timing mode %d\n", arg));
        return;
    }
    LOG(logDEBUG1, ("Setting Triggering Mode: %d\n", (int)ret));
#ifndef VIRTUAL
    sharedMemory_lockLocalLink();
    if (Feb_Control_SetTriggerMode(ret)) {
        eiger_triggermode = ret;
    }
    sharedMemory_unlockLocalLink();
#else
    eiger_triggermode = ret;
#endif
}

enum timingMode getTiming() {
    switch (eiger_triggermode) {
    case 0:
        return AUTO_TIMING;
    case 2:
        return TRIGGER_EXPOSURE;
    case 1:
        return BURST_TRIGGER;
    case 3:
        return GATED;
    default:
        LOG(logERROR, ("Unknown trigger mode found %d\n", eiger_triggermode));
        return GET_FLAG;
    }
}

/* configure mac */
int getNumberofUDPInterfaces() { return 2; }

int getNumberofDestinations(int *retval) {
#ifdef VIRTUAL
    *retval = numUdpDestinations;
    return OK;
#else
    return Beb_GetNumberofDestinations(retval);
#endif
}

int setNumberofDestinations(int value) {
#ifdef VIRTUAL
    // already set in funcs.c
    return OK;
#else
    return Beb_SetNumberofDestinations(value);
#endif
}

int configureMAC() {

    LOG(logINFOBLUE, ("Configuring MAC\n"));

    LOG(logINFO, ("Number of entries: %d\n", numUdpDestinations));
    for (int iRxEntry = 0; iRxEntry != MAX_UDP_DESTINATION; ++iRxEntry) {
        uint32_t srcip = udpDetails[iRxEntry].srcip;
        uint32_t dstip = udpDetails[iRxEntry].dstip;
        uint64_t srcmac = udpDetails[iRxEntry].srcmac;
        uint64_t dstmac = udpDetails[iRxEntry].dstmac;
        uint16_t srcport = udpDetails[iRxEntry].srcport;
        uint16_t dstport = udpDetails[iRxEntry].dstport;
        uint16_t dstport2 = udpDetails[iRxEntry].dstport2;

        char src_mac[MAC_ADDRESS_SIZE], src_ip[INET_ADDRSTRLEN],
            dst_mac[MAC_ADDRESS_SIZE], dst_ip[INET_ADDRSTRLEN];
        getMacAddressinString(src_mac, MAC_ADDRESS_SIZE, srcmac);
        getMacAddressinString(dst_mac, MAC_ADDRESS_SIZE, dstmac);
        getIpAddressinString(src_ip, srcip);
        getIpAddressinString(dst_ip, dstip);

        if (iRxEntry < numUdpDestinations) {
            LOG(logINFOBLUE, ("\tEntry %d\n", iRxEntry));
            LOG(logINFO,
                ("\tSource IP   : %s\n"
                 "\tSource MAC  : %s\n"
                 "\tSource Port : %hu\n"
                 "\tDest IP     : %s\n"
                 "\tDest MAC    : %s\n"
                 "\tDest Port   : %hu\n"
                 "\tDest Port2  : %hu\n",
                 src_ip, src_mac, srcport, dst_ip, dst_mac, dstport, dstport2));
        }

#ifdef VIRTUAL
        if (setUDPDestinationDetails(iRxEntry, 0, dst_ip, dstport) == FAIL) {
            LOG(logERROR,
                ("could not set udp destination IP and port [entry:%d]\n",
                 iRxEntry));
            return FAIL;
        }
        if (setUDPDestinationDetails(iRxEntry, 1, dst_ip, dstport2) == FAIL) {
            LOG(logERROR,
                ("could not set udp destination IP and port2 [entry:%d]\n",
                 iRxEntry));
            return FAIL;
        }
#else
        uint16_t dst_port = dstport;
        if (!top)
            dst_port = dstport2;

        if (Beb_SetUpUDPHeader(iRxEntry, send_to_ten_gig, srcmac, srcip,
                               srcport, dstmac, dstip, dst_port)) {
            LOG(logDEBUG1, ("\tset up left ok\n"));
        } else {
            return FAIL;
        }

        dst_port = dstport2;
        if (!top)
            dst_port = dstport;

        if (Beb_SetUpUDPHeader(iRxEntry + MAX_UDP_DESTINATION, send_to_ten_gig,
                               srcmac, srcip, srcport, dstmac, dstip,
                               dst_port)) {
            LOG(logDEBUG1, ("\tset up right ok\n"));
        } else {
            return FAIL;
        }
#endif
    }
    return OK;
}

int setDetectorPosition(int pos[]) {
#ifdef VIRTUAL
    memcpy(eiger_virtual_detPos, pos, sizeof(eiger_virtual_detPos));
    return OK;
#else
    return Beb_SetDetectorPosition(pos);
#endif
}

int *getDetectorPosition() {
#ifdef VIRTUAL
    return eiger_virtual_detPos;
#else
    return Beb_GetDetectorPosition();
#endif
}

int setQuad(int value) {
    if (value < 0) {
        return OK;
    }
    // only top can be set to quad
    if (!top && value > 0) {
        LOG(logERROR, ("Only a top can be set to quad\n"));
        return FAIL;
    }
#ifndef VIRTUAL
    if (Beb_SetQuad(value) == FAIL) {
        return FAIL;
    }
    sharedMemory_lockLocalLink();
    if (!Feb_Control_SetQuad(value)) {
        sharedMemory_unlockLocalLink();
        return FAIL;
    }
    sharedMemory_unlockLocalLink();
#else
    eiger_virtual_quad_mode = value;
#endif
    return OK;
}

int getQuad() {
#ifdef VIRTUAL
    return eiger_virtual_quad_mode;
#else
    return Beb_GetQuad();
#endif
}

int setInterruptSubframe(int value) {
    if (value < 0)
        return FAIL;
#ifndef VIRTUAL
    sharedMemory_lockLocalLink();
    if (!Feb_Control_SetInterruptSubframe(value)) {
        sharedMemory_unlockLocalLink();
        return FAIL;
    }
    sharedMemory_unlockLocalLink();
#else
    eiger_virtual_interrupt_subframe = value;
#endif
    return OK;
}

int getInterruptSubframe() {
#ifdef VIRTUAL
    return eiger_virtual_interrupt_subframe;
#else
    sharedMemory_lockLocalLink();
    int retval = Feb_Control_GetInterruptSubframe();
    sharedMemory_unlockLocalLink();
    return retval;
#endif
}

int setReadNRows(int value) {
    if (value < 0)
        return FAIL;
#ifndef VIRTUAL
    sharedMemory_lockLocalLink();
    if (!Feb_Control_SetReadNRows(value)) {
        sharedMemory_unlockLocalLink();
        return FAIL;
    }
    sharedMemory_unlockLocalLink();
    Beb_SetReadNRows(value);
#else
    eiger_virtual_read_n_rows = value;
#endif
    return OK;
}

int getReadNRows() {
#ifdef VIRTUAL
    return eiger_virtual_read_n_rows;
#else
    sharedMemory_lockLocalLink();
    int retval = Feb_Control_GetReadNRows();
    sharedMemory_unlockLocalLink();
    return retval;
#endif
}

int enableTenGigabitEthernet(int val) {
    if (val != -1) {
        LOG(logINFO, ("Setting 10Gbe: %d\n", (val > 0) ? 1 : 0));
        if (val > 0)
            send_to_ten_gig = 1;
        else
            send_to_ten_gig = 0;
#ifndef VIRTUAL
        Beb_ClearHeaderData(send_to_ten_gig == 0 ? 1 : 0);
#endif
    }
    return send_to_ten_gig;
}

/* eiger specific - iodelay, pulse, rate, temp, activate, delay nw parameter */
int setReadoutSpeed(int val) {
    if (val >= 0) {
        LOG(logINFO, ("Setting Read out Speed: %d\n", val));
#ifndef VIRTUAL
        sharedMemory_lockLocalLink();
        if (Feb_Control_SetReadoutSpeed(val)) {
            eiger_readoutspeed = val;
        }
        sharedMemory_unlockLocalLink();
#else
        eiger_readoutspeed = val;
#endif
    }
    return OK;
}

int getReadoutSpeed(int *retval) {
    *retval = eiger_readoutspeed;
    return OK;
}

int setIODelay(int val) {
    if (val != -1) {
        LOG(logDEBUG1, ("Setting IO Delay: %d\n", val));
#ifndef VIRTUAL
        sharedMemory_lockLocalLink();
        if (Feb_Control_SetIDelays(val)) {
            eiger_iodelay = val;
        }
        sharedMemory_unlockLocalLink();
#else
        eiger_iodelay = val;
#endif
        detectorModules->iodelay = val;
    }
    return eiger_iodelay;
}

int setCounterBit(int val) {
    if (val != -1) {
        LOG(logINFO, ("Setting Counter Bit: %d\n", val));
#ifdef VIRTUAL
        eiger_virtual_counter_bit = val;
#else
        sharedMemory_lockLocalLink();
        Feb_Control_Set_Counter_Bit(val);
        sharedMemory_unlockLocalLink();
#endif
    }
#ifdef VIRTUAL
    return eiger_virtual_counter_bit;
#else
    sharedMemory_lockLocalLink();
    int retval = Feb_Control_Get_Counter_Bit();
    sharedMemory_unlockLocalLink();
    return retval;
#endif
}

int pulsePixel(int n, int x, int y) {
#ifndef VIRTUAL
    sharedMemory_lockLocalLink();
    if (!Feb_Control_Pulse_Pixel(n, x, y)) {
        sharedMemory_unlockLocalLink();
        return FAIL;
    }
    sharedMemory_unlockLocalLink();
#endif
    return OK;
}

int pulsePixelNMove(int n, int x, int y) {
#ifndef VIRTUAL
    sharedMemory_lockLocalLink();
    if (!Feb_Control_PulsePixelNMove(n, x, y)) {
        sharedMemory_unlockLocalLink();
        return FAIL;
    }
    sharedMemory_unlockLocalLink();
#endif
    return OK;
}

int pulseChip(int n) {
#ifndef VIRTUAL
    sharedMemory_lockLocalLink();
    if (!Feb_Control_PulseChip(n)) {
        sharedMemory_unlockLocalLink();
        return FAIL;
    }
    sharedMemory_unlockLocalLink();
#endif
    return OK;
}

int updateRateCorrection(char *mess) {
    int ret = OK;
    // recalculates rate correction table, or switches off in wrong bit mode
    if (eiger_tau_ns != 0) {
        switch (eiger_dynamicrange) {
        case 16:
        case 32:
            ret = setRateCorrection(eiger_tau_ns);
            break;
        default:
            setRateCorrection(0);
            strcpy(mess,
                   "Rate correction Deactivated, must be in 32 or 16 bit mode");
            ret = FAIL;
            break;
        }
    }
    getCurrentTau(); // update eiger_tau_ns
    return ret;
}

int validateAndSetRateCorrection(int64_t tau_ns, char *mess) {
    // switching on in wrong bit mode
    if ((tau_ns != 0) && (eiger_dynamicrange != 32) &&
        (eiger_dynamicrange != 16)) {
        strcpy(mess,
               "Rate correction Deactivated, must be in 32 or 16 bit mode\n");
        LOG(logERROR, (mess));
        return FAIL;
    }
    // default tau (-1, get proper value)
    if (tau_ns < 0) {
        tau_ns = getDefaultSettingsTau_in_nsec();
        if (tau_ns < 0) {
            strcpy(mess,
                   "Default settings file not loaded. No default tau yet\n");
            LOG(logERROR, (mess));
            return FAIL;
        }
        eiger_tau_ns = -1;
    }
    // user defined value (settings become undefined)
    else if (tau_ns > 0) {
        setSettings(UNDEFINED);
        LOG(logERROR,
            ("Settings has been changed to undefined (tau changed)\n"));
        eiger_tau_ns = tau_ns;
    }
    return setRateCorrection(tau_ns);
}

int setRateCorrection(
    int64_t custom_tau_in_nsec) { // in nanosec (will never be -1)
#ifdef VIRTUAL
    // deactivating rate correction
    if (custom_tau_in_nsec == 0) {
        eiger_virtual_ratecorrection_variable = 0;
        return OK;
    }

    // when dynamic range changes, use old tau
    else if (custom_tau_in_nsec == -1)
        custom_tau_in_nsec = eiger_virtual_ratetable_tau_in_ns;

    // get period = subexptime if 32bit , else period = exptime if 16 bit
    int64_t actual_period = eiger_virtual_subexptime * 10; // already in nsec
    if (eiger_dynamicrange == 16)
        actual_period = eiger_virtual_exptime;

    int64_t ratetable_period_in_nsec = eiger_virtual_ratetable_period_in_ns;
    int64_t tau_in_nsec = eiger_virtual_ratetable_tau_in_ns;

    // same setting
    if ((tau_in_nsec == custom_tau_in_nsec) &&
        (ratetable_period_in_nsec == actual_period)) {
        if (eiger_dynamicrange == 32) {
            LOG(logINFO, ("Rate Table already created before: Same Tau %lldns, "
                          "Same subexptime %lldns\n",
                          (long long int)tau_in_nsec,
                          (long long int)ratetable_period_in_nsec));
        } else {
            LOG(logINFO, ("Rate Table already created before: Same Tau %lldns, "
                          "Same exptime %lldns\n",
                          (long long int)tau_in_nsec,
                          (long long int)ratetable_period_in_nsec));
        }
    }
    // different setting, calculate table
    else {
        eiger_virtual_ratetable_tau_in_ns = custom_tau_in_nsec;
        eiger_virtual_ratetable_period_in_ns = eiger_virtual_subexptime * 10;
        if (eiger_dynamicrange == 16)
            eiger_virtual_ratetable_period_in_ns = eiger_virtual_exptime;
    }
    // activating rate correction
    eiger_virtual_ratecorrection_variable = 1;
    LOG(logINFO, ("Rate Correction Value set to %lld ns\n",
                  (long long int)eiger_virtual_ratetable_tau_in_ns));

    return OK;
#else
    sharedMemory_lockLocalLink();

    // deactivating rate correction
    if (custom_tau_in_nsec == 0) {
        Feb_Control_SetRateCorrectionVariable(0);
        sharedMemory_unlockLocalLink();
        return OK;
    }

    // when dynamic range changes, use old tau
    else if (custom_tau_in_nsec == -1)
        custom_tau_in_nsec = Feb_Control_Get_RateTable_Tau_in_nsec();

    int dr = eiger_dynamicrange;

    // get period = subexptime if 32bit , else period = exptime if 16 bit
    int64_t actual_period =
        Feb_Control_GetSubFrameExposureTime(); // already in nsec
    if (dr == 16)
        actual_period = Feb_Control_GetExposureTime_in_nsec();

    int64_t ratetable_period_in_nsec =
        Feb_Control_Get_RateTable_Period_in_nsec();
    int64_t tau_in_nsec = Feb_Control_Get_RateTable_Tau_in_nsec();

    // same setting
    if ((tau_in_nsec == custom_tau_in_nsec) &&
        (ratetable_period_in_nsec == actual_period)) {
        if (dr == 32) {
            LOG(logINFO, ("Rate Table already created before: Same Tau %lldns, "
                          "Same subexptime %lldns\n",
                          tau_in_nsec, ratetable_period_in_nsec));
        } else {
            LOG(logINFO, ("Rate Table already created before: Same Tau %lldns, "
                          "Same exptime %lldns\n",
                          tau_in_nsec, ratetable_period_in_nsec));
        }
    }
    // different setting, calculate table
    else {
        int ret = Feb_Control_SetRateCorrectionTau(custom_tau_in_nsec);
        if (ret <= 0) {
            LOG(logERROR,
                ("Rate correction failed. Deactivating rate correction\n"));
            Feb_Control_SetRateCorrectionVariable(0);
            sharedMemory_unlockLocalLink();
            return FAIL;
        }
    }
    // activating rate correction
    Feb_Control_SetRateCorrectionVariable(1);
    LOG(logINFO, ("Rate Correction Value set to %lld ns\n",
                  (long long int)Feb_Control_Get_RateTable_Tau_in_nsec()));
    Feb_Control_PrintCorrectedValues();
    sharedMemory_unlockLocalLink();

    return OK;
#endif
}

int getRateCorrectionEnable() {
#ifdef VIRTUAL
    return eiger_virtual_ratecorrection_variable;
#else
    sharedMemory_lockLocalLink();
    int retval = Feb_Control_GetRateCorrectionVariable();
    sharedMemory_unlockLocalLink();
    return retval;
#endif
}

int getDefaultSettingsTau_in_nsec() { return default_tau_from_file; }

void setDefaultSettingsTau_in_nsec(int t) {
    default_tau_from_file = t;
    LOG(logINFOBLUE, ("Default tau set to %d\n", default_tau_from_file));
}

int64_t getCurrentTau() {
    if (!getRateCorrectionEnable()) {
        eiger_tau_ns = 0;
    } else {
#ifndef VIRTUAL
        sharedMemory_lockLocalLink();
        eiger_tau_ns = Feb_Control_Get_RateTable_Tau_in_nsec();
        sharedMemory_unlockLocalLink();
#else
        eiger_tau_ns = eiger_virtual_ratetable_tau_in_ns;
#endif
    }
    detectorModules->tau = eiger_tau_ns;
    return eiger_tau_ns;
}

void setExternalGating(int enable[]) {
    // not configured from client
    // default: disable gating with positive polarity
    if (enable[0] >= 0 && enable[1] >= 0) {
#ifndef VIRTUAL
        sharedMemory_lockLocalLink();
        Feb_Control_SetExternalEnableMode(
            enable[0], enable[1]); // enable = 0 or 1, polarity = 0 or 1 , where
                                   // 1 is positive
        sharedMemory_unlockLocalLink();
#endif
        eiger_extgating = enable[0];
        eiger_extgatingpolarity = enable[1];
    }
    enable[0] = eiger_extgating;
    enable[1] = eiger_extgatingpolarity;
}

int setTrimbits(int *chanregs, char *mess) {
    LOG(logINFO, ("Setting module with trimbits\n"));
#ifndef VIRTUAL
    // include gap pixels
    unsigned int tt[263680];
    int ip = 0, ich = 0;
    for (int iy = 0; iy < 256; ++iy) {
        for (int ichip = 0; ichip < 4; ++ichip) {
            for (int ix = 0; ix < 256; ++ix) {
                tt[ip++] = chanregs[ich++];
            }
            if (ichip < 3) {
                tt[ip++] = 0;
                tt[ip++] = 0;
            }
        }
    }

    // set trimbits
    sharedMemory_lockLocalLink();

    // if quad, set M8 and PROGRAM manually
    if (!Feb_Control_SetChipSignalsToTrimQuad(1)) {
        sprintf(mess, "Could not set module. Could not enable chip signals to "
                      "set trimbits\n");
        LOG(logERROR, (mess));
        sharedMemory_unlockLocalLink();
        return FAIL;
    }

    if (!Feb_Control_SetTrimbits(tt, top)) {
        sprintf(mess, "Could not set module. Could not set trimbits\n");
        LOG(logERROR, (mess));
        setSettings(UNDEFINED);
        LOG(logERROR, ("Settings has been changed to undefined (random "
                       "trim file)\n"));

        // if quad, reset M8 and PROGRAM manually
        if (!Feb_Control_SetChipSignalsToTrimQuad(0)) {
            sprintf(mess, "Could not set module. Could not disable chip "
                          "signals to set trimbits\n");
            LOG(logERROR, (mess));
            sharedMemory_unlockLocalLink();
            return FAIL;
        }

        sharedMemory_unlockLocalLink();
        return FAIL;
    }

    // if quad, reset M8 and PROGRAM manually
    if (!Feb_Control_SetChipSignalsToTrimQuad(0)) {
        sprintf(mess, "Could not set module. Could not disable chip signals to "
                      "set trimbits\n");
        LOG(logERROR, (mess));
        sharedMemory_unlockLocalLink();
        return FAIL;
    }

    sharedMemory_unlockLocalLink();

    // copying trimbits locally (if tirmbit value > -1)
    for (int ichan = 0; ichan < (detectorModules->nchan); ichan++) {
        if (*(chanregs + ichan) >= 0)
            *((detectorModules->chanregs) + ichan) = *(chanregs + ichan);
    }
#endif
    return OK;
}

int setAllTrimbits(int val) {
    LOG(logINFO, ("Setting all trimbits to %d\n", val));
#ifndef VIRTUAL
    sharedMemory_lockLocalLink();
    if (!Feb_Control_SaveAllTrimbitsTo(val, top)) {
        LOG(logERROR, ("Could not set all trimbits\n"));
        sharedMemory_unlockLocalLink();
        return FAIL;
    }
    sharedMemory_unlockLocalLink();
#endif

    // copying trimbits locally
    if (detectorModules) {
        for (int ichan = 0; ichan < (detectorModules->nchan); ichan++) {
            *((detectorModules->chanregs) + ichan) = val;
        }
    }

    LOG(logINFO, ("All trimbits have been set to %d\n", val));
    return OK;
}

int getAllTrimbits() {
    if (detectorModules) {
        int value = (*((detectorModules->chanregs)));
        for (int ichan = 0; ichan < (detectorModules->nchan); ichan++) {
            if (*((detectorModules->chanregs) + ichan) != value) {
                return -1;
            }
        }
        LOG(logINFO, ("Value of all Trimbits: %d\n", value));
        return value;
    }
    return -1;
}

int getBebFPGATemp() {
#ifdef VIRTUAL
    return 0;
#else
    return Beb_GetBebFPGATemp();
#endif
}

int setActivate(int enable) {
    if (enable < 0) {
        LOG(logERROR, ("Invalid activate argument: %d\n", enable));
        return FAIL;
    }
#ifdef VIRTUAL
    eiger_virtual_activate = enable;
#else
    if (!Beb_SetActivate(enable)) {
        return FAIL;
    }
    sharedMemory_lockLocalLink();
    Feb_Control_activate(enable);
    sharedMemory_unlockLocalLink();
#endif
    if (enable) {
        LOG(logINFOGREEN, ("Activated in %s Server!\n",
                           isControlServer ? " Control" : "Stop"));
    } else {
        LOG(logINFORED, ("Deactivated in %s Server!\n",
                         isControlServer ? " Control" : "Stop"));
    }
    return OK;
}

int getActivate(int *retval) {
#ifdef VIRTUAL
    *retval = eiger_virtual_activate;
#else
    if (!Beb_GetActivate(retval)) {
        return FAIL;
    }
#endif
    return OK;
}

int setDataStream(enum portPosition port, int enable) {
    if (enable < 0) {
        LOG(logERROR, ("Invalid setDataStream enable argument: %d\n", enable));
        return FAIL;
    }
    LOG(logINFO,
        ("%s 10GbE %s datastream\n", (enable ? "Enabling" : "Disabling"),
         (port == LEFT ? "left" : "right")));
#ifdef VIRTUAL
    if (port == LEFT) {
        eiger_virtual_left_datastream = enable;
    } else {
        eiger_virtual_right_datastream = enable;
    }
#else
    if (!Beb_SetDataStream(port, enable)) {
        return FAIL;
    }
#endif
    return OK;
}

int getDataStream(enum portPosition port, int *retval) {
#ifdef VIRTUAL
    if (port == LEFT) {
        *retval = eiger_virtual_left_datastream;
    } else {
        *retval = eiger_virtual_right_datastream;
    }
#else
    if (!Beb_GetDataStream(port, retval)) {
        return FAIL;
    }
#endif
    return OK;
}

int getTenGigaFlowControl() {
#ifdef VIRTUAL
    return eiger_virtual_transmission_flowcontrol_10g;
#else
    return Beb_GetTenGigaFlowControl();
#endif
}

int setTenGigaFlowControl(int value) {
#ifdef VIRTUAL
    eiger_virtual_transmission_flowcontrol_10g = (value == 0 ? 0 : 1);
#else
    if (!Beb_SetTenGigaFlowControl(value)) {
        return FAIL;
    }
#endif
    return OK;
}

int getTransmissionDelayFrame() {
#ifdef VIRTUAL
    return eiger_virtual_transmission_delay_frame;
#else
    return Beb_GetTransmissionDelayFrame();
#endif
}

int setTransmissionDelayFrame(int value) {
#ifdef VIRTUAL
    eiger_virtual_transmission_delay_frame = value;
#else
    if (!Beb_SetTransmissionDelayFrame(value)) {
        return FAIL;
    }
#endif
    return OK;
}

int getTransmissionDelayLeft() {
#ifdef VIRTUAL
    return eiger_virtual_transmission_delay_left;
#else
    return Beb_GetTransmissionDelayLeft();
#endif
}

int setTransmissionDelayLeft(int value) {
#ifdef VIRTUAL
    eiger_virtual_transmission_delay_left = value;
#else
    if (!Beb_SetTransmissionDelayLeft(value)) {
        return FAIL;
    }
#endif
    return OK;
}

int getTransmissionDelayRight() {
#ifdef VIRTUAL
    return eiger_virtual_transmission_delay_right;
#else
    return Beb_GetTransmissionDelayRight();
#endif
}

int setTransmissionDelayRight(int value) {
#ifdef VIRTUAL
    eiger_virtual_transmission_delay_right = value;
#else
    if (!Beb_SetTransmissionDelayRight(value)) {
        return FAIL;
    }
#endif
    return OK;
}

/* aquisition */

int startStateMachine() {
    sharedMemory_lockAcqFlag();

#ifdef VIRTUAL
    // create udp socket
    if (createUDPSocket(0) != OK) {
        sharedMemory_unlockAcqFlag();
        return FAIL;
    }
    if (createUDPSocket(1) != OK) {
        sharedMemory_unlockAcqFlag();
        return FAIL;
    }
    LOG(logINFOBLUE, ("Starting State Machine\n"));
    if (sharedMemory_getStop() != 0) {
        LOG(logERROR, ("Cant start acquisition. "
                       "Stop server has not updated stop status to 0\n"));
        sharedMemory_unlockAcqFlag();
        return FAIL;
    }
    sharedMemory_setStatus(RUNNING);
    if (pthread_create(&virtual_tid, NULL, &start_timer, NULL)) {
        LOG(logERROR, ("Could not start Virtual acquisition thread\n"));
        sharedMemory_setStatus(IDLE);
        sharedMemory_unlockAcqFlag();
        return FAIL;
    } else
        pthread_detach(virtual_tid);
    LOG(logINFO, ("Virtual Acquisition started\n"));
    sharedMemory_unlockAcqFlag();
    return OK;
#else
    sharedMemory_lockLocalLink();

    LOG(logINFO, ("Going to prepare for acquisition with counter_bit:%d\n",
                  Feb_Control_Get_Counter_Bit()));
    Feb_Control_PrepareForAcquisition();

    LOG(logINFO, ("Acquisition started bit toggled\n"));
    int ret = OK, prev_flag;
    // get the DAQ toggle bit
    prev_flag = Feb_Control_AcquisitionStartedBit();

    LOG(logINFOBLUE, ("Starting State Machine\n"));
    Feb_Control_StartAcquisition();

    LOG(logINFO, ("requesting images right after start\n"));
    ret = startReadOut();

    // wait for acquisition start
    if (ret == OK) {
        if (!Feb_Control_WaitForStartedFlag(5000, prev_flag)) {
            LOG(logERROR,
                ("Acquisition did not LOG(logERROR ouble reading register\n"));
            sharedMemory_unlockLocalLink();
            sharedMemory_unlockAcqFlag();
            return FAIL;
        }
        LOG(logINFOGREEN, ("Acquisition started\n"));
    }
    sharedMemory_unlockLocalLink();
    sharedMemory_unlockAcqFlag();
    return ret;
#endif
}

#ifdef VIRTUAL
void *start_timer(void *arg) {
    if (!isControlServer) {
        return NULL;
    }

    int skipData = 0;
    int tgEnable = send_to_ten_gig;
    if (!eiger_virtual_activate ||
        (tgEnable &&
         (!eiger_virtual_left_datastream && !eiger_virtual_right_datastream))) {
        skipData = 1;
        LOG(logWARNING, ("Not sending Left and Right datastream\n"));
    }
    if (tgEnable && !eiger_virtual_left_datastream) {
        LOG(logWARNING, ("Not sending Left datastream\n"));
    }
    if (tgEnable && !eiger_virtual_right_datastream) {
        LOG(logWARNING, ("Not sending Right datastream\n"));
    }

    int64_t periodNs = eiger_virtual_period;
    int numFrames = nimages_per_request;
    int64_t expUs = eiger_virtual_exptime / 1000;

    int dr = eiger_dynamicrange;
    double bytesPerPixel = (double)dr / 8.00;
    int datasize = (tgEnable ? 4096 : 1024);
    int packetsize = datasize + sizeof(sls_detector_header);
    int maxPacketsPerFrame = (tgEnable ? 4 : 16) * dr;
    int npixelsx = 256 * 2 * bytesPerPixel;
    int databytes = 256 * 256 * 2 * bytesPerPixel;
    int row = eiger_virtual_detPos[Y];
    int colLeft = top ? eiger_virtual_detPos[X] : eiger_virtual_detPos[X] + 1;
    int colRight = top ? eiger_virtual_detPos[X] + 1 : eiger_virtual_detPos[X];

    int readNRows = getReadNRows();
    if (readNRows == -1) {
        LOG(logERROR, ("readNRows is -1. Assuming no readNRows.\n"));
        readNRows = MAX_ROWS_PER_READOUT;
    }
    const int maxRows = MAX_ROWS_PER_READOUT;
    const int packetsPerFrame = (maxPacketsPerFrame * readNRows) / maxRows;

    LOG(logDEBUG,
        (" dr:%d\n bytesperpixel:%f\n tgenable:%d\n datasize:%d\n "
         "packetsize:%d\n maxnumpackes:%d\n npixelsx:%d\n databytes:%d\n",
         dr, bytesPerPixel, tgEnable, datasize, packetsize, maxPacketsPerFrame,
         npixelsx, databytes));

    // Generate data
    char imageData[databytes * 2];
    memset(imageData, 0, databytes * 2);
    {
        int npixels = NCHAN * NCHIP;
        const int pixelsPerPacket = (double)datasize / bytesPerPixel;
        int pixelVal = 0;
        if (dr == 4) {
            npixels /= 2;
        }
        LOG(logDEBUG1,
            ("npixels:%d pixelsperpacket:%d\n", npixels, pixelsPerPacket));
        uint8_t *src = (uint8_t *)imageData;
        for (int i = 0; i < npixels; ++i) {
            if (i > 0 && i % pixelsPerPacket == 0) {
                ++pixelVal;
            }

            switch (dr) {
            case 4:
                *((uint8_t *)(imageData + i)) =
                    eiger_virtual_test_mode
                        ? 0xEE
                        : (uint8_t)(((2 * pixelVal & 0xF) << 4) |
                                    ((2 * pixelVal) & 0xF));
                //: (uint8_t)(((2 * pixelVal & 0xF) << 4) | ((2 * pixelVal + 1)
                //& 0xF));
                break;
            case 8:
                *((uint8_t *)(imageData + i)) =
                    eiger_virtual_test_mode ? 0xFE : (uint8_t)pixelVal;
                break;
            case 12:
                if (eiger_virtual_test_mode) {
                    // first 12 bit pixel
                    // first 8 byte
                    *src++ = 0xFE;
                    // second 12bit pixel
                    ++i;
                    // second 8 byte
                    *src++ = 0xEF;
                    // third byte
                    *src++ = 0xFF;
                } else {
                    // first 12 bit pixel
                    // first 8 byte
                    *src++ = (uint8_t)(i & 0xFF);
                    // second 8 byte (first nibble)
                    *src = (uint8_t)((i++ >> 8u) & 0xF);
                    // second 12bit pixel
                    // second 8 byte (second nibble)
                    *src++ |= ((uint8_t)(i & 0xF) << 4u);
                    // third byte
                    *src++ = (uint8_t)((i >> 4u) & 0xFF);
                }
                break;
            case 16:
// to debug multi module geometry (row, column) in virtual servers (all pixels
// in a module set to particular value)
#ifdef TEST_MOD_GEOMETRY
                if ((i % 1024) < 512) {
                    *((uint16_t *)(imageData + i * sizeof(uint16_t))) =
                        top ? (portno % 1900) : ((portno % 1900) + 1);
                } else {
                    *((uint16_t *)(imageData + i * sizeof(uint16_t))) =
                        top ? ((portno % 1900) + 1) : (portno % 1900);
                }
#else
                *((uint16_t *)(imageData + i * sizeof(uint16_t))) =
                    eiger_virtual_test_mode ? 0xFFE : (uint16_t)pixelVal;
#endif
                break;
            case 32:
                *((uint32_t *)(imageData + i * sizeof(uint32_t))) =
                    eiger_virtual_test_mode ? 0xFFFFFE : (uint32_t)pixelVal;
                break;
            default:
                break;
            }
        }
    }

    // Send data
    if (!skipData) {
        uint64_t frameNr = 0;
        getNextFrameNumber(&frameNr);
        int iRxEntry = 0;
        // loop over number of frames
        for (int iframes = 0; iframes != numFrames; ++iframes) {

            usleep(eiger_virtual_transmission_delay_frame);

            // check if manual stop
            if (sharedMemory_getStop() == 1) {
                setNextFrameNumber(frameNr + iframes + 1);
                break;
            }

            // sleep for exposure time
            struct timespec begin, end;
            clock_gettime(CLOCK_REALTIME, &begin);
            usleep(expUs);

            int srcOffset = 0;
            int srcOffset2 = npixelsx;

            // loop packet
            for (int i = 0; i != maxPacketsPerFrame; ++i) {

                // calculate for readNRows
                const int startval = 0;
                const int endval = startval + packetsPerFrame - 1;

                // set header
                char packetData[packetsize];
                memset(packetData, 0, packetsize);
                sls_detector_header *header =
                    (sls_detector_header *)(packetData);
                header->detType = (uint16_t)myDetectorType;
                header->version = SLS_DETECTOR_HEADER_VERSION;
                header->frameNumber = frameNr + iframes;
                header->packetNumber = i;
                header->modId = eiger_virtual_module_id;
                header->row = row;
                header->column = colLeft;
                if (eiger_virtual_quad_mode) {
                    header->row = 1;    // left is next row
                    header->column = 0; // left same first column
                }

                char packetData2[packetsize];
                memset(packetData2, 0, packetsize);
                header = (sls_detector_header *)(packetData2);
                header->detType = (uint16_t)myDetectorType;
                header->version = SLS_DETECTOR_HEADER_VERSION;
                header->frameNumber = frameNr + iframes;
                header->packetNumber = i;
                header->modId = eiger_virtual_module_id + 1;
                header->row = row;
                header->column = colRight;
                if (eiger_virtual_quad_mode) {
                    header->row = 0;    // right is next row
                    header->column = 0; // right same first column
                }

                // fill data
                int dstOffset = sizeof(sls_detector_header);
                int dstOffset2 = sizeof(sls_detector_header);
                if (dr == 12) {
                    // multiple of 768,1024,4096
                    int copysize = 256;
                    for (int psize = 0; psize < datasize; psize += copysize) {
                        memcpy(packetData + dstOffset, imageData + srcOffset,
                               copysize);
                        memcpy(packetData2 + dstOffset2, imageData + srcOffset2,
                               copysize);
                        srcOffset += copysize;
                        srcOffset2 += copysize;
                        dstOffset += copysize;
                        dstOffset2 += copysize;

                        // reached 1 row (quarter module)
                        if ((srcOffset % npixelsx) == 0) {
                            srcOffset += npixelsx;
                            srcOffset2 += npixelsx;
                        }
                    }
                } else {
                    for (int psize = 0; psize < datasize; psize += npixelsx) {
                        if (dr == 32 && tgEnable == 0) {
                            memcpy(packetData + dstOffset,
                                   imageData + srcOffset, npixelsx / 2);
                            memcpy(packetData2 + dstOffset2,
                                   imageData + srcOffset2, npixelsx / 2);
                            if (srcOffset % npixelsx == 0) {
                                srcOffset += npixelsx / 2;
                                srcOffset2 += npixelsx / 2;
                            }
                            // skip the other half (2 packets in 1 line for
                            // 32 bit)
                            else {
                                srcOffset += npixelsx;
                                srcOffset2 += npixelsx;
                            }
                            dstOffset += npixelsx / 2;
                            dstOffset2 += npixelsx / 2;
                        } else {
                            memcpy(packetData + dstOffset,
                                   imageData + srcOffset, npixelsx);
                            memcpy(packetData2 + dstOffset2,
                                   imageData + srcOffset2, npixelsx);
                            srcOffset += 2 * npixelsx;
                            srcOffset2 += 2 * npixelsx;
                            dstOffset += npixelsx;
                            dstOffset2 += npixelsx;
                        }
                    }
                }
                if ((!tgEnable ||
                     (tgEnable && eiger_virtual_left_datastream)) &&
                    i >= startval && i <= endval) {
                    usleep(eiger_virtual_transmission_delay_left);
                    sendUDPPacket(iRxEntry, 0, packetData, packetsize);
                    LOG(logDEBUG1, ("Sent left packet: %d\n", i));
                }
                if ((!tgEnable ||
                     (tgEnable && eiger_virtual_right_datastream)) &&
                    i >= startval && i <= endval) {
                    usleep(eiger_virtual_transmission_delay_right);
                    sendUDPPacket(iRxEntry, 1, packetData2, packetsize);
                    LOG(logDEBUG1, ("Sent right packet: %d\n", i));
                }
            }
            LOG(logINFO, ("Sent frame %d [#%ld] to E%d\n", iframes,
                          frameNr + iframes, iRxEntry));
            clock_gettime(CLOCK_REALTIME, &end);
            int64_t timeNs = ((end.tv_sec - begin.tv_sec) * 1E9 +
                              (end.tv_nsec - begin.tv_nsec));

            // sleep for (period - exptime)
            if (iframes < numFrames) { // if there is a next frame
                if (periodNs > timeNs) {
                    usleep((periodNs - timeNs) / 1000);
                }
            }
            ++iRxEntry;
            if (iRxEntry == numUdpDestinations) {
                iRxEntry = 0;
            }
        }
        setNextFrameNumber(frameNr + numFrames);
    }

    closeUDPSocket(0);
    closeUDPSocket(1);

    sharedMemory_setStatus(IDLE);
    LOG(logINFOBLUE, ("Transmitting frames done\n"));
    return NULL;
}
#endif

int stopStateMachine() {

    // acq lock for seamless stop
    sharedMemory_lockAcqFlag();

    LOG(logINFORED, ("Stopping state machine\n"));
    // if scan active, stop scan
    if (sharedMemory_getScanStatus() == RUNNING) {
        sharedMemory_setScanStop(1);
    }
#ifdef VIRTUAL
    sharedMemory_setStop(1);
    // read till status is idle
    while (sharedMemory_getStatus() == RUNNING)
        usleep(500);
    sharedMemory_setStop(0);
    LOG(logINFO, ("Stopped State Machine\n"));
    sharedMemory_unlockAcqFlag();
    return OK;
#else
    sharedMemory_lockLocalLink();
    // sends last frames from fifo and wait for feb processing done
    if (!Feb_Control_StopAcquisition()) {
        LOG(logERROR, ("failed to stop acquisition\n"));
        sharedMemory_unlockLocalLink();
        sharedMemory_unlockAcqFlag();
        return FAIL;
    }
    sharedMemory_unlockLocalLink();

    // wait for beb to finish sending packets
    int isTransmitting = 1;
    while (isTransmitting) {
        // wait for beb to send out all packets
        if (Beb_IsTransmitting(&isTransmitting, send_to_ten_gig, 1) == FAIL) {
            LOG(logERROR, ("failed to stop beb acquisition\n"));
            sharedMemory_unlockAcqFlag();
            return FAIL;
        }
        if (isTransmitting) {
            printf("Transmitting...\n");
        }
    }
    LOG(logINFO, ("Beb: Detector has sent all data (stop)\n"));

    // reset feb and beb
    sharedMemory_lockLocalLink();
    Feb_Control_Reset();
    sharedMemory_unlockLocalLink();
    if (!Beb_StopAcquisition()) {
        LOG(logERROR, ("failed to stop acquisition\n"));
        sharedMemory_unlockAcqFlag();
        return FAIL;
    }

    // ensure all have same starting frame numbers
    uint64_t retval = 0;
    if (Beb_GetNextFrameNumber(&retval, send_to_ten_gig) == -2) {
        Beb_SetNextFrameNumber(retval + 1);
    }
    LOG(logINFOBLUE, ("Stopping state machine complete\n\n"));
    sharedMemory_unlockAcqFlag();
    return OK;
#endif
}

int softwareTrigger(int block) {
#ifdef VIRTUAL
    return OK;
#else
    sharedMemory_lockLocalLink();
    if (!Feb_Control_SoftwareTrigger(block)) {
        sharedMemory_unlockLocalLink();
        return FAIL;
    }
    sharedMemory_unlockLocalLink();
    return OK;
#endif
}

int startReadOut() {
    LOG(logINFO, ("Requesting images...\n"));
#ifndef VIRTUAL
    if (!Beb_RequestNImages(send_to_ten_gig, nimages_per_request, 0)) {
        return FAIL;
    }
#endif
    return OK;
}

enum runStatus getRunStatus() {
    LOG(logDEBUG1, ("Getting status\n"));
    // scan error or running
    if (sharedMemory_getScanStatus() == ERROR) {
        LOG(logINFOBLUE, ("Status: scan ERROR\n"));
        return ERROR;
    }
    if (sharedMemory_getScanStatus() == RUNNING) {
        LOG(logINFOBLUE, ("Status: scan RUNNING\n"));
        return RUNNING;
    }
#ifdef VIRTUAL
    if (sharedMemory_getStatus() == RUNNING) {
        LOG(logINFOBLUE, ("Status: RUNNING\n"));
        return RUNNING;
    }
    LOG(logINFOBLUE, ("Status: IDLE\n"));
    return IDLE;
#else
    sharedMemory_lockLocalLink();
    int i = Feb_Control_AcquisitionInProgress();
    sharedMemory_unlockLocalLink();
    if (i == STATUS_ERROR) {
        LOG(logERROR, ("Status: ERROR reading status register\n"));
        return ERROR;
    } else if (i == STATUS_IDLE) {
        int isTransmitting = 0;
        if (Beb_IsTransmitting(&isTransmitting, send_to_ten_gig, 0) == FAIL) {
            return ERROR;
        }
        if (isTransmitting) {
            LOG(logINFOBLUE, ("Status: TRANSMITTING\n"));
            return TRANSMITTING;
        }
        LOG(logINFOBLUE, ("Status: IDLE\n"));
        return IDLE;
    }
    LOG(logINFOBLUE, ("Status: RUNNING...\n"));
    return RUNNING;
#endif
}

void waitForAcquisitionEnd(int *ret, char *mess) {
#ifdef VIRTUAL
    // wait for status to be done
    while (sharedMemory_getStatus() == RUNNING) {
        usleep(500);
    }
#else
    sharedMemory_lockLocalLink();
    if (Feb_Control_WaitForFinishedFlag(5000, 1) == STATUS_ERROR) {
        sharedMemory_unlockLocalLink();
        strcpy(mess, "Could not wait for finished flag\n");
        LOG(logERROR, (mess));
        *ret = FAIL;
        return;
    }
    sharedMemory_unlockLocalLink();
    LOG(logINFO, ("Acquisition  done\n"));

    // wait for detector to send
    int isTransmitting = 1;
    while (isTransmitting) {
        // wait for feb processing to be done
        sharedMemory_lockLocalLink();
        int i = Feb_Control_ProcessingInProgress();
        sharedMemory_unlockLocalLink();
        if (i == STATUS_ERROR) {
            strcpy(mess, "Could not read feb processing done register\n");
            LOG(logERROR, (mess));
            *ret = (int)FAIL;
            return;
        }
        if (i == RUNNING) {
            LOG(logINFOBLUE, ("Status: TRANSMITTING (feb processing)\n"));
            isTransmitting = 1;
        }

        // wait for beb to send out all packets
        if (Beb_IsTransmitting(&isTransmitting, send_to_ten_gig, 1) == FAIL) {
            strcpy(mess, "Could not read delay counters\n");
            LOG(logERROR, (mess));
            *ret = (int)FAIL;
            return;
        }
        if (isTransmitting) {
            printf("Transmitting...\n");
        }
    }
    LOG(logINFOBLUE, ("Transmitting frames done\n"));
#endif
    LOG(logINFOGREEN, ("Blocking Acquisition done\n"));
}

/* common */

int calculateDataBytes() {
    if (send_to_ten_gig)
        return eiger_dynamicrange * ONE_GIGA_CONSTANT * TEN_GIGA_BUFFER_SIZE;
    else
        return eiger_dynamicrange * TEN_GIGA_CONSTANT * ONE_GIGA_BUFFER_SIZE;
}

int getTotalNumberOfChannels() {
    return (getNumberOfChannelsPerChip() * getNumberOfChips());
}
int getNumberOfChips() { return NCHIP; }
int getNumberOfDACs() { return NDAC; }
int getNumberOfChannelsPerChip() { return NCHAN; }
