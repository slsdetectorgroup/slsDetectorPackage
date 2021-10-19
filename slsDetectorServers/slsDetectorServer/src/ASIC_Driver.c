// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "ASIC_Driver.h"
#include "clogger.h"
#include "common.h"
#include "sls/sls_detector_defs.h"

#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

// defines from the fpga
char ASIC_Driver_DriverFileName[MAX_STR_LENGTH];

void ASIC_Driver_SetDefines(char *driverfname) {
    LOG(logINFOBLUE, ("Configuring ASIC Driver to %s\n", driverfname));
    memset(ASIC_Driver_DriverFileName, 0, MAX_STR_LENGTH);
    strcpy(ASIC_Driver_DriverFileName, driverfname);
}

int ASIC_Driver_Set(int index, int length, char *buffer) {
    char temp[20];
    memset(temp, 0, sizeof(temp));
    sprintf(temp, "%d", index + 1);
    char fname[MAX_STR_LENGTH];
    strcpy(fname, ASIC_Driver_DriverFileName);
    strcat(fname, temp);
    LOG(logDEBUG2,
        ("\t[chip index: %d, length: %d, fname: %s]\n", index, length, fname));
    {
        LOG(logDEBUG2, ("\t[values: \n"));
        for (int i = 0; i < length; ++i) {
            LOG(logDEBUG2, ("\t%d: 0x%02hhx\n", i, buffer[i]));
        }
        LOG(logDEBUG2, ("\t]\n"));
    }

#ifndef VIRTUAL
    int fd = open(fname, O_RDWR);
    if (fd == -1) {
        LOG(logERROR,
            ("Could not open file %s for writing to control ASIC (%d)\n", fname,
             index));
        return FAIL;
    }

    struct spi_ioc_transfer transfer;
    memset(&transfer, 0, sizeof(transfer));
    transfer.tx_buf = (unsigned long)buffer;
    transfer.len = length;
    transfer.cs_change = 0;

    // transfer command
    int status = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer);
    if (status < 0) {
        LOG(logERROR, ("Could not send command to ASIC\n"));
        perror("SPI_IOC_MESSAGE");
        close(fd);
        return FAIL;
    }
    close(fd);
#endif

    return OK;
}
