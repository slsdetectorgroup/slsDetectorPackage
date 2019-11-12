#include "ASIC_Driver.h"
#include "clogger.h"
#include "common.h"
#include "sls_detector_defs.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

// defines from the fpga
char ASIC_Driver_DriverFileName[MAX_STR_LENGTH];


void ASIC_Driver_SetDefines(char* driverfname) {
    FILE_LOG(logINFOBLUE, ("Configuring ASIC Driver to %s\n", driverfname));
    memset(ASIC_Driver_DriverFileName, 0, MAX_STR_LENGTH);
    strcpy(ASIC_Driver_DriverFileName, driverfname);
}

int ASIC_Driver_Set (int index, int length, char* buffer) {
    char fname[MAX_STR_LENGTH];
    sprintf(fname, "%s%d", ASIC_Driver_DriverFileName, index + 1);
    FILE_LOG(logDEBUG1, ("\t[chip index: %d, length: %d, fname: %s]\n", index, length, fname)); 
    {
        char printbuffer[2 * length + 15];
        memset(printbuffer, 0, 2 * length);
        strcpy(printbuffer, "\t[values: ");
        int i;
        for (i = 0; i < length; ++i) {
            char bytes[10]="";
            sprintf(bytes, "0x%02hhx", buffer[i]);
            strcat(printbuffer, bytes);
            if (i < length - 1) {
                strcat(printbuffer, "\t");
            }
        }
        int len = strlen(printbuffer);
        printbuffer[len - 1] = ']';
        printbuffer[len] = '\0';
        FILE_LOG(logDEBUG1, ("%s\n", printbuffer));
    }
    
    int fd=open(fname, O_RDWR);
    if (fd == -1) {
        FILE_LOG(logERROR, ("Could not open file %s for writing to control ASIC (%d)\n", fname, index));
        return FAIL;
    }

    struct spi_ioc_transfer transfer;
    memset(&transfer, 0, sizeof(transfer));
    transfer.tx_buf = (unsigned long) buffer;
    transfer.len = length;
    transfer.cs_change = 0; 

    // transfer command
    int status = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer);
    if (status < 0) {
        FILE_LOG(logERROR, ("Could not send command to ASIC\n"));
        perror("SPI_IOC_MESSAGE");
        close(fd);
        return FAIL;
    }
    close(fd);
    
    return OK;
}
