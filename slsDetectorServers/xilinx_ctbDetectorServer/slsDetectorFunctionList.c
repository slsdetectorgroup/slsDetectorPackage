// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "slsDetectorFunctionList.h"
#include "clogger.h"
#include "sharedMemory.h"
#include "sls/versionAPI.h"
#include "common.h"
#include "arm.h"

#include <string.h>
#include <unistd.h> // usleep

// Global variable from slsDetectorServer_funcs
extern int updateFlag;
extern const enum detectorType myDetectorType;

// Global variable from communication_funcs.c
extern int isControlServer;

int initError = OK;
int initCheckDone = 0;
char initErrorMessage[MAX_STR_LENGTH];


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
}

/* Ids */

void getServerVersion(char *version) { strcpy(version, APIXILINXCTB); }

uint64_t getFirmwareVersion() {
#ifdef VIRTUAL
    return REQRD_FRMWR_VRSN;
#endif
    return 0;
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
    LOG(logINFO, ("This Server is for 1 Xilinx Chip Test Board\n"));
#ifdef VIRTUAL
    sharedMemory_setStatus(IDLE);
#endif  
    LOG(logINFO, ("Goodbye...\n"));
}

