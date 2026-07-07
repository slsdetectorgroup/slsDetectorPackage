// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "clogger.h"
#include "sls/ansi.h"

#include <errno.h>
#include <fcntl.h> // File control definitions
#include <stdio.h>
#include <stdlib.h>    // atoi
#include <string.h>    // memset
#include <sys/ioctl.h> // ioctl
#include <termios.h>   /* POSIX terminal control definitions */
#include <unistd.h>    // read, close

#define PORTNAME   "/dev/ttyBF1"
#define GOODBYE    200
#define BUFFERSIZE 16
#define INFILE     "/sys/devices/platform/i2c-bfin-twi.0/i2c-0/0-0048/in0_input"
#define OUTFILE    "/sys/devices/platform/i2c-bfin-twi.0/i2c-0/0-0048/out0_output"
#define OUTENABLE                                                              \
    "/sys/devices/platform/i2c-bfin-twi.0/i2c-0/0-0048/out0_enable"

int set_hv(int dac_value) {
    if ((dac_value > 255) || (dac_value < 0)) {
        LOG(logERROR, ("Invalid dac value %d\n", dac_value));
        return -1;
    }

    dac_value = dac_value * 10;

    FILE *file;
    file = fopen(OUTFILE, "w");
    if (file == NULL) {
        perror("set_hv:");
        LOG(logERROR, ("Cannot open out0_output file\n"));
        return -1;
    }
    if (setvbuf(file, NULL, _IONBF, 0) != 0) {
        perror("set_hv:");
        LOG(logERROR, ("Cannot disable buffering\n"));
        return -1;
    }
    if (fprintf(file, "%d", dac_value) < 1) {
        ferror(file);
        LOG(logERROR, ("Couldn't write to out0_output file\n"));
        return -1;
    }
    if (fclose(file) != 0) {
        perror("set_hv:");
        LOG(logERROR, ("Troubles closing out0_output file\n"));
        return -1;
    }
    return 0;
}

int enable_hv(int val) {
    if ((val > 1) || (val < 0))
        return -1;
    FILE *file;
    file = fopen(OUTENABLE, "w");
    if (file == NULL) {
        perror("enable_hv:");
        LOG(logERROR, ("Cannot open out0_enable file\n"));
        return -1;
    }
    if (setvbuf(file, NULL, _IONBF, 0) != 0) {
        perror("enable_hv:");
        LOG(logERROR, ("Cannot disable buffering\n"));
        return -1;
    }
    if (fprintf(file, "%d", val) < 1) {
        ferror(file);
        LOG(logERROR, ("Couldn't write to out0_enable file\n"));
        return -1;
    }
    if (fclose(file) != 0) {
        perror("enable_hv:");
        LOG(logERROR, ("Troubles closing out0_enable file\n"));
        return -1;
    }
    return 0;
}

int get_hv() {
    int value;
    FILE *file;
    file = fopen(INFILE, "r");
    if (file == NULL) {
        perror("get_hv:");
        LOG(logERROR, ("Cannot open in0_input file\n"));
        return -1;
    }
    if (fscanf(file, "%d", &value) < 1) {
        ferror(file);
        LOG(logERROR, ("Couldn't read from in0_input file\n"));
        return -1;
    }
    if (fclose(file) != 0) {
        perror("get_hv:");
        LOG(logERROR, ("Troubles closing out0_enable file\n"));
        return -1;
    }
    return value / 10;
}

int main(int argc, char *argv[]) {

    enable_hv(0);
    set_hv(0);

    int fd = open(PORTNAME, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        LOG(logERROR, ("Warning: Unable to open port %s\n", PORTNAME));
        return -1;
    }
    LOG(logINFO, ("opened port at %s\n", PORTNAME));

    struct termios serial_conf;
    // reset structure
    memset(&serial_conf, 0, sizeof(serial_conf));
    // control options
    serial_conf.c_cflag = B2400 | CS8 | CREAD | CLOCAL;
    // input options
    serial_conf.c_iflag = IGNPAR;
    // output options
    serial_conf.c_oflag = 0;
    // line options
    serial_conf.c_lflag = ICANON;
    // flush input
    if (tcflush(fd, TCIOFLUSH) < 0) {
        LOG(logERROR, ("Warning: error form tcflush %d\n", errno));
        return 0;
    }
    // set new options for the port, TCSANOW:changes occur immediately without
    // waiting for data to complete
    if (tcsetattr(fd, TCSANOW, &serial_conf) < 0) {
        LOG(logERROR, ("Warning: error form tcsetattr %d\n", errno));
        return 0;
    }

    if (tcsetattr(fd, TCSAFLUSH, &serial_conf) < 0) {
        LOG(logERROR, ("Warning: error form tcsetattr %d\n", errno));
        return 0;
    }

    int ret = 0;
    int n = 0;
    int ival = 0;
    char buffer[BUFFERSIZE];
    memset(buffer, 0, BUFFERSIZE);
    //    buffer[BUFFERSIZE - 1] = '\n';
    LOG(logINFO, ("Ready...\n"));

    while (ret != GOODBYE) {
        memset(buffer, 0, BUFFERSIZE);
        n = read(fd, buffer, BUFFERSIZE);
        LOG(logDEBUG1, ("Received %d Bytes\n", n));
        LOG(logINFO, ("Got message: '%s'\n", buffer));

        switch (buffer[0]) {
        case '\0':
            LOG(logINFO, ("Got Start (Detector restart)\n"));
            break;
        case 's':
            LOG(logINFO, ("Got Start \n"));
            break;
        case 'p':
            if (!sscanf(&buffer[1], "%d", &ival)) {
                LOG(logERROR, ("Warning: cannot scan voltage value\n"));
                break;
            }
            // ok/ fail
            memset(buffer, 0, BUFFERSIZE);
            //            buffer[BUFFERSIZE - 1] = '\n';

            if (set_hv(ival) < 0)
                strcpy(buffer, "fail\n");
            else if (enable_hv(ival > 0 ? 1 : 0) < 0)
                strcpy(buffer, "fail\n");
            else
                strcpy(buffer, "success\n");
            /*
                        if (i2c_write(ival) < 0)
                            strcpy(buffer, "fail ");
                        else
                            strcpy(buffer, "success ");
            */
            LOG(logINFO, ("Sending: '%s'\n", buffer));
            n = write(fd, buffer, strlen(buffer)); // BUFFERSIZE);
            LOG(logDEBUG1, ("Sent %d Bytes\n", n));
            break;

        case 'g':
            ival = get_hv();
            if (ival < 0)
                strcpy(buffer, "fail\n");
            else
                strcpy(buffer, "success\n");
            /*
                        ival = i2c_read();
                        // ok/ fail
                        memset(buffer, 0, BUFFERSIZE);
                        buffer[BUFFERSIZE - 1] = '\n';
                        if (ival < 0)
                            strcpy(buffer, "fail ");
                        else
                            strcpy(buffer, "success ");
            */
            n = write(fd, buffer, strlen(buffer)); // BUFFERSIZE);
            LOG(logINFO, ("Sending: '%s'\n", buffer));
            LOG(logDEBUG1, ("Sent %d Bytes\n", n));
            // value
            memset(buffer, 0, BUFFERSIZE);
            //            buffer[BUFFERSIZE - 1] = '\n';
            if (ival >= 0) {
                LOG(logINFO, ("Sending: '%d'\n", ival));
                sprintf(buffer, "%d\n", ival);
                n = write(fd, buffer, strlen(buffer)); // BUFFERSIZE);
                LOG(logINFO, ("Sent %d Bytes\n", n));
            } else
                LOG(logERROR, ("%s\n", buffer));
            break;

        case 'e':
            printf("Exiting Program\n");
            ret = GOODBYE;
            break;
        default:
            LOG(logERROR, ("Unknown Command. buffer:'%s'\n", buffer));
            break;
        }
    }

    close(fd);
    printf("Goodbye Serial Communication for HV(9M)\n");
    return 0;
}
