// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "programViaArm.h"
#include "clogger.h"
#include "common.h"
#include "sls/sls_detector_defs.h"
#include "slsDetectorServer_defs.h"

#include <string.h> //memset
#include <unistd.h> // access

#define CMD_ARM_LOAD_BIT_FILE "~/fpgautil/fpgautil -b " FIRMWARE_FILE " -f Full"
#define CMD_ARM_LOAD_DEVICE_TREE                                               \
    "cat " DEVICE_TREE_OVERLAY_FILE " > " DEVICE_TREE_API_FOLDER "/dtbo"
#define CMD_ARM_SYM_LINK_FORMAT                                                \
    "ln -sf " DEVICE_TREE_DST "%d " IIO_DEVICE_FOLDER "/%s"
#define TIME_LOAD_DEVICE_TREE_MS (500)

extern int executeCommand(char *command, char *result, enum TLogLevel level);

int resetFPGA(char *mess) {
    LOG(logINFOBLUE, ("Reseting FPGA...\n"));
#ifndef VIRTUAL
    char retvals[MAX_STR_LENGTH] = {0};
    if (executeCommand(CMD_ARM_LOAD_BIT_FILE, retvals, logDEBUG1) == FAIL) {
        snprintf(mess, MAX_STR_LENGTH,
                 "Could not reset fpga. Command to load bit file failed (%s)\n",
                 retvals);
        LOG(logERROR, (mess));
        return FAIL;
    }
#endif
    LOG(logINFOBLUE, ("FPGA reset successfully\n"))
    return OK;
}

int loadDeviceTree(char *mess) {
    if (verifyDeviceTree(mess) == OK)
        return OK;

    if (checksBeforeCreatingDeviceTree(mess) == FAIL)
        return FAIL;

    if (createDeviceTree(mess) == FAIL)
        return FAIL;

    if (verifyDeviceTree(mess) == FAIL) {
        LOG(logERROR, ("Device tree loading failed at verification\n"));
        return FAIL;
    }

    LOG(logINFOBLUE, ("Device tree loaded successfully\n"))
    return OK;
}

int checksBeforeCreatingDeviceTree(char *mess) {
    // check if device tree overlay file exists
    if (access(DEVICE_TREE_OVERLAY_FILE, F_OK) != 0) {
        snprintf(mess, MAX_STR_LENGTH,
                 "Device tree overlay file (%s) does not exist\n",
                 DEVICE_TREE_OVERLAY_FILE);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO,
        ("\tDevice tree overlay file exists (%s)\n", DEVICE_TREE_OVERLAY_FILE));

    // check if device tree folder exists. If it does, remove it
    if (access(DEVICE_TREE_API_FOLDER, F_OK) == 0) {
        // remove it
        char cmd[MAX_STR_LENGTH] = {0};
        memset(cmd, 0, MAX_STR_LENGTH);
        sprintf(cmd, "rmdir %s", DEVICE_TREE_API_FOLDER);
        char retvals[MAX_STR_LENGTH] = {0};
        memset(retvals, 0, MAX_STR_LENGTH);
        if (executeCommand(cmd, retvals, logDEBUG1) == FAIL) {
            snprintf(mess, MAX_STR_LENGTH,
                     "Could not unload device tree overlay api with %s (%s)\n",
                     cmd, retvals);
            LOG(logERROR, (mess));
            return FAIL;
        }
        LOG(logINFO, ("\tUnloaded existing device tree overlay api (%s)\n",
                      DEVICE_TREE_API_FOLDER));
    } else {
        LOG(logINFO, ("\tNo existing device tree overlay api found(%s)\n",
                      DEVICE_TREE_API_FOLDER));
    }

    // create device tree overlay folder
    {
        char cmd[MAX_STR_LENGTH] = {0};
        memset(cmd, 0, MAX_STR_LENGTH);
        sprintf(cmd, "mkdir %s", DEVICE_TREE_API_FOLDER);
        char retvals[MAX_STR_LENGTH] = {0};
        memset(retvals, 0, MAX_STR_LENGTH);
        if (executeCommand(cmd, retvals, logDEBUG1) == FAIL) {
            snprintf(mess, MAX_STR_LENGTH,
                     "Could not create device tree overlay api with %s (%s)\n",
                     cmd, retvals);
            LOG(logERROR, (mess));
            return FAIL;
        }
        LOG(logINFO, ("\tDevice tree overlay api created (%s)\n",
                      DEVICE_TREE_API_FOLDER));
    }

    return OK;
}

int createDeviceTree(char *mess) {
    char cmd[MAX_STR_LENGTH] = {0};
    memset(cmd, 0, MAX_STR_LENGTH);
    strcpy(cmd, CMD_ARM_LOAD_DEVICE_TREE);
    char retvals[MAX_STR_LENGTH] = {0};
    memset(retvals, 0, MAX_STR_LENGTH);
    if (executeCommand(cmd, retvals, logDEBUG1) == FAIL) {
        snprintf(mess, MAX_STR_LENGTH,
                 "Could not load device tree overlay with %s (%s)\n", cmd,
                 retvals);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tDevice tree overlay created (cmd: %s)\n", cmd));

    usleep(TIME_LOAD_DEVICE_TREE_MS * 1000);
    return OK;
}

int verifyDeviceTree(char *mess) {
    LOG(logINFOBLUE, ("Verifying Device Tree...\n"));

#ifdef VIRTUAL
    LOG(logINFOBLUE, ("Device tree verified successfully\n"));
    return OK;
#else

    // check if iio:device0-4 exists in device tree destination
    int hardcodedDeviceIndex = 0;
    int adcDeviceIndex = 1;
    int dacDeviceIndex = 2;

    for (int i = 0; i != 5; ++i) {
        char deviceName[MAX_STR_LENGTH] = {0};
        memset(deviceName, 0, MAX_STR_LENGTH);
        sprintf(deviceName, "%s%d/name", DEVICE_TREE_DST, i);
        // check if device exist
        if (access(deviceName, F_OK) != 0) {
            snprintf(mess, MAX_STR_LENGTH,
                     "Could not verify device tree. Device %s does not exist\n",
                     deviceName);
            LOG(logERROR, (mess));
            return FAIL;
        }
        // find name
        char cmd[MAX_STR_LENGTH] = {0};
        memset(cmd, 0, MAX_STR_LENGTH);
        sprintf(cmd, "cat %s", deviceName);
        char retvals[MAX_STR_LENGTH] = {0};
        memset(retvals, 0, MAX_STR_LENGTH);
        if (executeCommand(cmd, retvals, logDEBUG1) == FAIL) {
            snprintf(mess, MAX_STR_LENGTH,
                     "Could not retrieve device name from device %s (%s)\n",
                     deviceName, retvals);
            LOG(logERROR, (mess));
            return FAIL;
        }
        // verify name
        char *deviceNames[] = {DEVICE_NAME_LIST};
        if (strstr(retvals, deviceNames[hardcodedDeviceIndex]) == NULL) {
            // dacs got loaded first
            if (i == 1 &&
                strstr(retvals, deviceNames[hardcodedDeviceIndex + 1]) !=
                    NULL) {
                ++hardcodedDeviceIndex;
                adcDeviceIndex = 4;
                dacDeviceIndex = 1;
            } else {
                snprintf(
                    mess, MAX_STR_LENGTH,
                    "Could not verify device tree. Device %s expected %s but "
                    "got %s\n",
                    deviceName, deviceNames[i], retvals);
                LOG(logERROR, (mess));
                return FAIL;
            }
        }
        ++hardcodedDeviceIndex;
        // in case dacs were loaded first
        if (hardcodedDeviceIndex == 5)
            hardcodedDeviceIndex = 1;
    }
    LOG(logINFOBLUE, ("Device tree verified successfully [temp: 0, adc:%d, "
                      "dac:%d, %d, %d]\n",
                      adcDeviceIndex, dacDeviceIndex, dacDeviceIndex + 1,
                      dacDeviceIndex + 2));

    return createSymbolicLinksForDevices(adcDeviceIndex, dacDeviceIndex, mess);
#endif
}

#ifndef VIRTUAL
int createSymbolicLinksForDevices(int adcDeviceIndex, int dacDeviceIndex,
                                  char *mess) {

    // delete andcreate iio device links folder (deleting because cannot
    // overwrite symbolic links with -sf)
    if (deleteAbsoluteDirectory(mess, IIO_DEVICE_FOLDER,
                                "create sym links for device trees") == FAIL) {
        return FAIL;
    }
    if (createAbsoluteDirectory(mess, IIO_DEVICE_FOLDER,
                                "create sym links for device trees") == FAIL) {
        return FAIL;
    }

    char *deviceNames[] = {DEVICE_NAME_LIST};
    // create symbolic links for adc
    {
        char cmd[MAX_STR_LENGTH] = {0};
        memset(cmd, 0, MAX_STR_LENGTH);
        sprintf(cmd, CMD_ARM_SYM_LINK_FORMAT, adcDeviceIndex, deviceNames[1]);
        char retvals[MAX_STR_LENGTH] = {0};
        memset(retvals, 0, MAX_STR_LENGTH);
        if (executeCommand(cmd, retvals, logDEBUG1) == FAIL) {
            snprintf(mess, MAX_STR_LENGTH,
                     "Could not create sym link [adc] (%s)\n", retvals);
            LOG(logERROR, (mess));
            return FAIL;
        }
        LOG(logINFO, ("\tSym link [adc] created (%s)\n", cmd));
    }

    // create symbolic links for dacs
    for (int idac = 0; idac != 3; ++idac) {
        char cmd[MAX_STR_LENGTH] = {0};
        memset(cmd, 0, MAX_STR_LENGTH);
        sprintf(cmd, CMD_ARM_SYM_LINK_FORMAT, dacDeviceIndex + idac,
                deviceNames[idac + 2]);
        char retvals[MAX_STR_LENGTH] = {0};
        memset(retvals, 0, MAX_STR_LENGTH);
        if (executeCommand(cmd, retvals, logDEBUG1) == FAIL) {
            snprintf(mess, MAX_STR_LENGTH,
                     "Could not create sym link [dac%d] (%s)\n", idac, retvals);
            LOG(logERROR, (mess));
            return FAIL;
        }
        LOG(logINFO, ("\tSym link [dac%d] created (%s)\n", idac, cmd));
    }
    return OK;
}
#endif