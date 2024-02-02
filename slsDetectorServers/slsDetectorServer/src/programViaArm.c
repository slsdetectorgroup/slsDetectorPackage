// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "programViaArm.h"
#include "clogger.h"
#include "sls/sls_detector_defs.h"

#include <string.h> //memset
#include <unistd.h> // access

#define CMD_ARM_LOAD_BIT_FILE                                                  \
    "~/fpgautil/fpgautil -b /root/apps/xilinx-ctb/XilinxCTB.bit -f Full"
#define CMD_ARM_DEVICE_TREE_API_FOLDER                                         \
    "/sys/kernel/config/device-tree/overlays/spidr"
#define CMD_ARM_DEVICE_TREE_OVERLAY_FILE "/root/apps/xilinx-ctb/pl.dtbo"
#define CMD_ARM_LOAD_DEVICE_TREE_FORMAT  "cat %s > %s/dtbo"
#define CMD_ARM_DEVICE_TREE_DST          "/sys/bus/iio/devices/iio:device"
#define CMD_ARM_DEVICE_NAME              "xilinx-ams", "ad7689", "dac@0", "dac@1", "dac@2"
#define TIME_LOAD_DEVICE_TREE_MS         (500)

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

int loadDeviceTree(char *mess, int *adcDeviceIndex, int *dacDeviceIndex) {
    if (verifyDeviceTree(mess, adcDeviceIndex, dacDeviceIndex) == OK)
        return OK;

    if (checksBeforeCreatingDeviceTree(mess) == FAIL)
        return FAIL;

    if (createDeviceTree(mess) == FAIL)
        return FAIL;

    if (verifyDeviceTree(mess, adcDeviceIndex, dacDeviceIndex) == FAIL) {
        LOG(logERROR, ("Device tree loading failed at verification\n"));
        return FAIL;
    }

    LOG(logINFOBLUE, ("Device tree loaded successfully\n"))
    return OK;
}

int checksBeforeCreatingDeviceTree(char *mess) {
    // check if device tree overlay file exists
    if (access(CMD_ARM_DEVICE_TREE_OVERLAY_FILE, F_OK) != 0) {
        snprintf(mess, MAX_STR_LENGTH,
                 "Device tree overlay file (%s) does not exist\n",
                 CMD_ARM_DEVICE_TREE_OVERLAY_FILE);
        LOG(logERROR, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tDevice tree overlay file exists (%s)\n",
                  CMD_ARM_DEVICE_TREE_OVERLAY_FILE));

    // check if device tree folder exists. If it does, remove it
    if (access(CMD_ARM_DEVICE_TREE_API_FOLDER, F_OK) == 0) {
        // remove it
        char cmd[MAX_STR_LENGTH] = {0};
        memset(cmd, 0, MAX_STR_LENGTH);
        sprintf(cmd, "rmdir %s", CMD_ARM_DEVICE_TREE_API_FOLDER);
        char retvals[MAX_STR_LENGTH] = {0};
        memset(retvals, 0, MAX_STR_LENGTH);
        if (executeCommand(cmd, retvals, logDEBUG1) == FAIL) {
            snprintf(mess, MAX_STR_LENGTH,
                     "Could not unload device tree overlay api with %s (%s)\n",
                     cmd, retvals);
            LOG(logWARNING, (mess));
            return FAIL;
        }
        LOG(logINFO, ("\tUnloaded existing device tree overlay api (%s)\n",
                      CMD_ARM_DEVICE_TREE_API_FOLDER));
    } else {
        LOG(logINFO, ("\tNo existing device tree overlay api found(%s)\n",
                      CMD_ARM_DEVICE_TREE_API_FOLDER));
    }

    // create device tree overlay folder
    {
        char cmd[MAX_STR_LENGTH] = {0};
        memset(cmd, 0, MAX_STR_LENGTH);
        sprintf(cmd, "mkdir %s", CMD_ARM_DEVICE_TREE_API_FOLDER);
        char retvals[MAX_STR_LENGTH] = {0};
        memset(retvals, 0, MAX_STR_LENGTH);
        if (executeCommand(cmd, retvals, logDEBUG1) == FAIL) {
            snprintf(mess, MAX_STR_LENGTH,
                     "Could not create device tree overlay api with %s (%s)\n",
                     cmd, retvals);
            LOG(logWARNING, (mess));
            return FAIL;
        }
        LOG(logINFO, ("\tDevice tree overlay api created (%s)\n",
                      CMD_ARM_DEVICE_TREE_API_FOLDER));
    }

    return OK;
}

int createDeviceTree(char *mess) {
    char cmd[MAX_STR_LENGTH] = {0};
    memset(cmd, 0, MAX_STR_LENGTH);
    sprintf(cmd, CMD_ARM_LOAD_DEVICE_TREE_FORMAT,
            CMD_ARM_DEVICE_TREE_OVERLAY_FILE, CMD_ARM_DEVICE_TREE_API_FOLDER);
    char retvals[MAX_STR_LENGTH] = {0};
    memset(retvals, 0, MAX_STR_LENGTH);
    if (executeCommand(cmd, retvals, logDEBUG1) == FAIL) {
        snprintf(mess, MAX_STR_LENGTH,
                 "Could not load device tree overlay with %s (%s)\n", cmd,
                 retvals);
        LOG(logWARNING, (mess));
        return FAIL;
    }
    LOG(logINFO, ("\tDevice tree overlay created (cmd: %s)\n", cmd));

    usleep(TIME_LOAD_DEVICE_TREE_MS * 1000);
    return OK;
}

int verifyDeviceTree(char *mess, int *adcDeviceIndex, int *dacDeviceIndex) {
    LOG(logINFOBLUE, ("Verifying Device Tree...\n"));
    *adcDeviceIndex = 1;
    *dacDeviceIndex = 2;
#ifndef VIRTUAL

    // check if iio:device0-4 exists in device tree destination
    int hardcodedDeviceIndex = 0;
    for (int i = 0; i != 5; ++i) {
        char deviceName[MAX_STR_LENGTH] = {0};
        memset(deviceName, 0, MAX_STR_LENGTH);
        sprintf(deviceName, "%s%d/name", CMD_ARM_DEVICE_TREE_DST, i);
        // check if device exist
        if (access(deviceName, F_OK) != 0) {
            snprintf(mess, MAX_STR_LENGTH,
                     "Could not verify device tree. Device %s does not exist\n",
                     deviceName);
            LOG(logWARNING, (mess));
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
            LOG(logWARNING, (mess));
            return FAIL;
        }
        // verify name
        char *deviceNames[] = {CMD_ARM_DEVICE_NAME};
        if (strstr(retvals, deviceNames[hardcodedDeviceIndex]) == NULL) {
            // dacs got loaded first
            if (i == 1 &&
                strstr(retvals, deviceNames[hardcodedDeviceIndex + 1]) !=
                    NULL) {
                ++hardcodedDeviceIndex;
                *adcDeviceIndex = 4;
                *dacDeviceIndex = 1;
            } else {
                snprintf(
                    mess, MAX_STR_LENGTH,
                    "Could not verify device tree. Device %s expected %s but "
                    "got %s\n",
                    deviceName, deviceNames[i], retvals);
                LOG(logWARNING, (mess));
                return FAIL;
            }
        }
        ++hardcodedDeviceIndex;
        // in case dacs were loaded first
        if (hardcodedDeviceIndex == 5)
            hardcodedDeviceIndex = 1;
    }
#endif
    LOG(logINFOBLUE, ("Device tree verified successfully [temp: 0, adc:%d, "
                      "dac:%d, %d, %d]\n",
                      *adcDeviceIndex, *dacDeviceIndex, *dacDeviceIndex + 1,
                      *dacDeviceIndex + 2));
    return OK;
}