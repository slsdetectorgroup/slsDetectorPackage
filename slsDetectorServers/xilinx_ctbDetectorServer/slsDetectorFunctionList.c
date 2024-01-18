// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "slsDetectorFunctionList.h"
#include "arm64.h"
#include "clogger.h"
#include "common.h"
#include "sharedMemory.h"
#include "sls/versionAPI.h"

#include "loadPattern.h"

#include <arpa/inet.h> // INET_ADDRSTRLEN
#include <string.h>
#include <unistd.h> // usleep

// Global variable from slsDetectorServer_funcs
extern int debugflag;
extern int updateFlag;
extern const enum detectorType myDetectorType;

// Global variable from communication_funcs.c
extern int isControlServer;

int initError = OK;
int initCheckDone = 0;
char initErrorMessage[MAX_STR_LENGTH];

int detPos[2] = {0, 0};

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
    LOG(logINFOBLUE, ("****** Xilinx Chip Test Board Virtual Server ******\n"));
#else
    LOG(logINFOBLUE, ("********** Xilinx Chip Test Board Server **********\n"));
#endif
    if (mapCSP0() == FAIL) {
        strcpy(initErrorMessage,
               "Could not map to memory. Cannot proceed. Check Firmware.\n");
        LOG(logERROR, (initErrorMessage));
        initError = FAIL;
        return;
    }

#ifndef VIRTUAL
    if ((!debugflag) && (!updateFlag) &&
        ((validateKernelVersion(KERNEL_DATE_VRSN) == FAIL) ||
         (checkType() == FAIL) || (testFpga() == FAIL) ||
         (testBus() == FAIL))) {
        sprintf(initErrorMessage,
                "Could not pass basic tests of FPGA and bus. Cannot proceed. "
                "Check Firmware. (Firmware version:0x%lx) \n",
                getFirmwareVersion());
        LOG(logERROR, ("%s\n\n", initErrorMessage));
        initError = FAIL;
        return;
    }
#endif
    char hversion[MAX_STR_LENGTH] = {0};
    memset(hversion, 0, MAX_STR_LENGTH);
    getHardwareVersion(hversion);
    uint32_t ipadd = getDetectorIP();
    uint64_t macadd = getDetectorMAC();
    int64_t fwversion = getFirmwareVersion();
    char swversion[MAX_STR_LENGTH] = {0};
    memset(swversion, 0, MAX_STR_LENGTH);
    getServerVersion(swversion);
    int64_t sw_fw_apiversion = getFirmwareAPIVersion();
    uint64_t requiredfwversion = REQRD_FRMWRE_VRSN;

    LOG(logINFOBLUE,
        ("**************************************************\n"
         "Hardware Version:\t\t %s\n"

         "Detector IP Addr:\t\t 0x%x\n"
         "Detector MAC Addr:\t\t 0x%lx\n\n"

         "Firmware Version:\t\t 0x%lx\n"
         "Software Version:\t\t %s\n"
         "F/w-S/w API Version:\t\t 0x%lx\n"
         "Required Firmware Version:\t 0x%lx\n"
         "********************************************************\n",
         hversion, ipadd, macadd, fwversion, swversion, sw_fw_apiversion,
         requiredfwversion));

#ifndef VIRTUAL
    // return if flag is not zero, debug mode
    if (debugflag || updateFlag) {
        return;
    }

    // cant read versions
    LOG(logINFO, ("Testing Firmware-software compatibility:\n"));
    if (!fwversion || !sw_fw_apiversion) {
        strcpy(initErrorMessage,
               "Cant read versions from FPGA. Please update firmware.\n");
        LOG(logERROR, (initErrorMessage));
        initError = FAIL;
        return;
    }

    // check for API compatibility - old server
    if (sw_fw_apiversion > requiredfwversion) {
        sprintf(initErrorMessage,
                "This firmware-software api version (0x%lx) is incompatible "
                "with the software's minimum required firmware version "
                "(0x%lx).\nPlease update detector software to be compatible "
                "with this firmware.\n",
                sw_fw_apiversion, requiredfwversion);
        LOG(logERROR, (initErrorMessage));
        initError = FAIL;
        return;
    }

    // check for firmware compatibility - old firmware
    if (requiredfwversion > fwversion) {
        sprintf(initErrorMessage,
                "This firmware version (0x%lx) is incompatible.\n"
                "Please update firmware (min. 0x%lx) to be compatible with "
                "this server.\n",
                fwversion, requiredfwversion);
        LOG(logERROR, (initErrorMessage));
        initError = FAIL;
        return;
    }
    LOG(logINFO, ("\tCompatibility - success\n"));
#endif
}

int checkType() {
#ifdef VIRTUAL
    return OK;
#endif
    u_int32_t type =
        ((bus_r(FPGAVERSIONREG) & FPGADETTYPE_MSK) >> FPGADETTYPE_OFST);
    if (type != XILINX_CHIPTESTBOARD) {
        LOG(logERROR,
            ("This is not a Xilinx CTB firmware (read %d, expected %d)\n", type,
             XILINX_CHIPTESTBOARD));
        return FAIL;
    }
    return OK;
}

int testFpga() {
#ifdef VIRTUAL
    return OK;
#endif
    LOG(logINFO, ("Testing FPGA:\n"));

    // fixed pattern
    int ret = OK;

    uint32_t val = bus_r(FIXEDPATTERNREG);
    if (val == FIXEDPATTERNVAL) {
        LOG(logINFO, ("\tFixed pattern: successful match (0x%08x)\n", val));
    } else {
        LOG(logERROR,
            ("Fixed pattern does not match! Read 0x%08x, expected 0x%08x\n",
             val, FIXEDPATTERNVAL));
        ret = FAIL;
    }

    if (ret == OK) {
        // Delay LSB reg
        LOG(logINFO, ("\tTesting Delay LSB Register:\n"));
        uint32_t addr = DELAYINREG1;

        // store previous delay value
        uint32_t previousValue = bus_r(addr);

        volatile uint32_t val = 0, readval = 0;
        int times = 1000 * 1000;
        for (int i = 0; i < times; ++i) {
            val = 0x5A5A5A5A - i;
            bus_w(addr, val);
            readval = bus_r(addr);
            if (readval != val) {
                LOG(logERROR, ("1:Mismatch! Loop(%d): Wrote 0x%x, read 0x%x\n",
                               i, val, readval));
                ret = FAIL;
                break;
            }
            val = (i + (i << 10) + (i << 20));
            bus_w(addr, val);
            readval = bus_r(addr);
            if (readval != val) {
                LOG(logERROR, ("2:Mismatch! Loop(%d): Wrote 0x%x, read 0x%x\n",
                               i, val, readval));
                ret = FAIL;
                break;
            }
            val = 0x0F0F0F0F;
            bus_w(addr, val);
            readval = bus_r(addr);
            if (readval != val) {
                LOG(logERROR, ("3:Mismatch! Loop(%d): Wrote 0x%x, read 0x%x\n",
                               i, val, readval));
                ret = FAIL;
                break;
            }
            val = 0xF0F0F0F0;
            bus_w(addr, val);
            readval = bus_r(addr);
            if (readval != val) {
                LOG(logERROR, ("4:Mismatch! Loop(%d): Wrote 0x%x, read 0x%x\n",
                               i, val, readval));
                ret = FAIL;
                break;
            }
        }
        // write back previous value
        bus_w(addr, previousValue);
        if (ret == OK) {
            LOG(logINFO,
                ("\tSuccessfully tested FPGA Delay LSB Register %d times\n",
                 times));
        }
    }

    return ret;
}

int testBus() {
#ifdef VIRTUAL
    return OK;
#endif
    LOG(logINFO, ("Testing Bus:\n"));

    int ret = OK;
    uint32_t addr = DELAYINREG1;

    // store previous delay value
    uint32_t previousValue = bus_r(addr);

    volatile uint32_t val = 0, readval = 0;
    int times = 1000 * 1000;

    for (int i = 0; i < times; ++i) {
        val += 0xbbbbb;
        bus_w(addr, val);
        readval = bus_r(addr);
        if (readval != val) {
            LOG(logERROR, ("Mismatch! Loop(%d): Wrote 0x%x, read 0x%x\n", i,
                           val, readval));
            ret = FAIL;
        }
    }

    // write back previous value
    bus_w(addr, previousValue);

    if (ret == OK) {
        LOG(logINFO, ("\tSuccessfully tested bus %d times\n", times));
    }
    return ret;
}

/* Ids */

void getServerVersion(char *version) { strcpy(version, APIXILINXCTB); }

uint64_t getFirmwareVersion() {
#ifdef VIRTUAL
    return REQRD_FRMWRE_VRSN;
#endif
    return ((bus_r(FPGAVERSIONREG) & FPGACOMPDATE_MSK) >> FPGACOMPDATE_OFST);
}

uint64_t getFirmwareAPIVersion() {
#ifdef VIRTUAL
    return 0;
#endif
    return ((bus_r(APIVERSIONREG) & APICOMPDATE_MSK) >> APICOMPDATE_OFST);
}

void getHardwareVersion(char *version) { strcpy(version, "Not applicable"); }

u_int64_t getDetectorMAC() {
#ifdef VIRTUAL
    return 0;
#else
    char output[255], mac[255] = "";
    u_int64_t res = 0;
    FILE *sysFile =
        popen("ifconfig eth0 | grep ether |  awk '{ print $2 }'", "r");
    fgets(output, sizeof(output), sysFile);
    pclose(sysFile);
    // getting rid of ":"
    char *pch;
    pch = strtok(output, ":");
    while (pch != NULL) {
        strcat(mac, pch);
        pch = strtok(NULL, ":");
    }
    sscanf(mac, "%lx", &res);
    return res;
#endif
}

u_int32_t getDetectorIP() {
#ifdef VIRTUAL
    return 0;
#endif
    char temp[INET_ADDRSTRLEN] = "";
    u_int32_t res = 0;
    // execute and get address
    char output[255];
    FILE *sysFile = popen(
        "ifconfig  | grep 'inet '| grep -v '127.0.0.1' | awk '{ print $2 }'",
        "r");
    fgets(output, sizeof(output), sysFile);
    pclose(sysFile);

    // converting IPaddress to hex.
    char *pcword = strtok(output, ".");
    while (pcword != NULL) {
        sprintf(output, "%02x", atoi(pcword));
        strcat(temp, output);
        pcword = strtok(NULL, ".");
    }
    strcpy(output, temp);
    sscanf(output, "%x", &res);
    // LOG(logINFO, ("ip:%x\n",res);

    return res;
}

/* initialization */

void initControlServer() {
    if (!updateFlag && initError == OK) {
        setupDetector();
    }
    initCheckDone = 1;
}

void initStopServer() {
    if (!updateFlag && initError == OK) {
        usleep(CTRL_SRVR_INIT_TIME_US);
        if (mapCSP0() == FAIL) {
            initError = FAIL;
            strcpy(initErrorMessage,
                   "Stop Server: Map Fail. Cannot proceed. Check Firmware.\n");
            LOG(logERROR, (initErrorMessage));
            initCheckDone = 1;
            return;
        }
#ifdef VIRTUAL
        sharedMemory_setStop(0);
#endif
    }
    initCheckDone = 1;
}

/* set up detector */

void setupDetector() {
    LOG(logINFO, ("Setting up Server for 1 Xilinx Chip Test Board\n"));
#ifdef VIRTUAL
    sharedMemory_setStatus(IDLE);
    initializePatternWord();
#endif

    LOG(logINFOBLUE, ("Setting Default parameters\n"));
    initializePatternAddresses();

    setNumFrames(DEFAULT_NUM_FRAMES);
    setNumTriggers(DEFAULT_NUM_CYCLES);
    setTiming(DEFAULT_TIMING_MODE);
    setExpTime(DEFAULT_EXPTIME);
    setPeriod(DEFAULT_PERIOD);
}

/* set parameters -  dr */

int setDynamicRange(int dr) {
    if (dr == 16)
        return OK;
    return FAIL;
}

int getDynamicRange(int *retval) {
    *retval = DYNAMIC_RANGE;
    return OK;
}

/* parameters - timer */

void setNumFrames(int64_t val) {
    if (val > 0) {
        LOG(logINFO, ("Setting number of frames %ld\n", val));
        setU64BitReg(val, FRAMESINREG1, FRAMESINREG2);
    }
}

int64_t getNumFrames() { return getU64BitReg(FRAMESINREG1, FRAMESINREG2); }

void setNumTriggers(int64_t val) {
    if (val > 0) {
        LOG(logINFO, ("Setting number of triggers %ld\n", val));
        setU64BitReg(val, CYCLESINREG1, CYCLESINREG2);
    }
}

int64_t getNumTriggers() { return getU64BitReg(CYCLESINREG1, CYCLESINREG2); }

int setExpTime(int64_t val) {
    if (val < 0) {
        LOG(logERROR, ("Invalid exptime: %lld ns\n", (long long int)val));
        return FAIL;
    }
    LOG(logINFO, ("Setting exptime %lld ns\n", (long long int)val));
    val *= (1E-3 * RUN_CLK);
    setPatternWaitTime(0, val);

    // validate for tolerance
    int64_t retval = getExpTime();
    val /= (1E-3 * RUN_CLK);
    if (val != retval) {
        return FAIL;
    }
    return OK;
}

int64_t getExpTime() { return getPatternWaitTime(0) / (1E-3 * RUN_CLK); }

int setPeriod(int64_t val) {
    if (val < 0) {
        LOG(logERROR, ("Invalid period: %lld ns\n", (long long int)val));
        return FAIL;
    }
    LOG(logINFO, ("Setting period %lld ns\n", (long long int)val));
    val *= (1E-3 * RUN_CLK);
    setU64BitReg(val, PERIODINREG1, PERIODINREG2);

    // validate for tolerance
    int64_t retval = getPeriod();
    val /= (1E-3 * RUN_CLK);
    if (val != retval) {
        return FAIL;
    }
    return OK;
}

int64_t getPeriod() {
    return getU64BitReg(PERIODINREG1, PERIODINREG2) / (1E-3 * RUN_CLK);
}

int64_t getNumFramesLeft() {
    return getU64BitReg(FRAMESOUTREG1, FRAMESOUTREG2);
}

int64_t getNumTriggersLeft() {
    return getU64BitReg(CYCLESOUTREG1, CYCLESOUTREG2);
}

/* parameters - timing, extsig */

void setTiming(enum timingMode arg) {
    switch (arg) {
    case AUTO_TIMING:
        LOG(logINFO, ("Set Timing: Auto\n"));
        bus_w(FLOWCONTROLREG, bus_r(FLOWCONTROLREG) & ~TRIGGERENABLE_MSK);
        break;
    case TRIGGER_EXPOSURE:
        LOG(logINFO, ("Set Timing: Trigger\n"));
        bus_w(FLOWCONTROLREG, bus_r(FLOWCONTROLREG) | TRIGGERENABLE_MSK);
        break;
    default:
        LOG(logERROR, ("Unknown timing mode %d\n", arg));
    }
}

enum timingMode getTiming() {
    if (bus_r(FLOWCONTROLREG) == TRIGGERENABLE_MSK)
        return TRIGGER_EXPOSURE;
    return AUTO_TIMING;
}

int setDetectorPosition(int pos[]) {
    memcpy(detPos, pos, sizeof(detPos));
    // TODO
    return OK;
}

int configureMAC() {
    // TODO
    LOG(logINFO, ("Configuring MAC\n"));
    return OK;
}

int *getDetectorPosition() { return detPos; }

int getNumberofUDPInterfaces() { return 1; }

/* aquisition */

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
#endif
    // TODO: get status
    LOG(logINFOBLUE, ("Status: IDLE\n"));
    return IDLE;
}

void getNumberOfChannels(int *nchanx, int *nchany) {
    // TODO
    *nchanx = NCHAN;
    *nchany = 1;
}