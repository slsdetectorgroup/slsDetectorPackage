#include "sls/ansi.h"

#include <errno.h>
#include <fcntl.h>         // File control definitions
#include <linux/i2c-dev.h> // I2C_SLAVE, __u8 reg
#include <stdio.h>
#include <stdlib.h>    // atoi
#include <string.h>    // memset
#include <sys/ioctl.h> // ioctl
#include <termios.h>   /* POSIX terminal control definitions */
#include <unistd.h>    // read, close

#define PORTNAME           "/dev/ttyBF1"
#define GOODBYE            200
#define BUFFERSIZE         16
#define I2C_DEVICE_FILE    "/dev/i2c-0"
#define I2C_DEVICE_ADDRESS 0x4C
//#define I2C_DEVICE_ADDRESS	0x48
#define I2C_REGISTER_ADDRESS 0x40

int i2c_open(const char *file, unsigned int addr) {

    // device file
    int fd = open(file, O_RDWR);
    if (fd < 0) {
        LOG(logERROR, ("Warning: Unable to open file %s\n", file));
        return -1;
    }

    // device address
    if (ioctl(fd, I2C_SLAVE, addr & 0x7F) < 0) {
        LOG(logERROR, ("Warning: Unable to set slave address:0x%x \n", addr));
        return -2;
    }
    return fd;
}

int i2c_read() {

    int fd = i2c_open(I2C_DEVICE_FILE, I2C_DEVICE_ADDRESS);
    __u8 reg = I2C_REGISTER_ADDRESS & 0xff;

    unsigned char buf = reg;
    if (write(fd, &buf, 1) != 1) {
        LOG(logERROR,
            ("Warning: Unable to write read request to register %d\n", reg));
        return -1;
    }
    // read and update value (but old value read out)
    if (read(fd, &buf, 1) != 1) {
        LOG(logERROR, ("Warning: Unable to read register %d\n", reg));
        return -2;
    }
    // read again to read the updated value
    if (read(fd, &buf, 1) != 1) {
        LOG(logERROR, ("Warning: Unable to read register %d\n", reg));
        return -2;
    }
    close(fd);
    return buf;
}

int i2c_write(unsigned int value) {

    __u8 val = value & 0xff;

    int fd = i2c_open(I2C_DEVICE_FILE, I2C_DEVICE_ADDRESS);
    if (fd < 0)
        return fd;

    __u8 reg = I2C_REGISTER_ADDRESS & 0xff;
    char buf[3];
    buf[0] = reg;
    buf[1] = val;
    if (write(fd, buf, 2) != 2) {
        LOG(logERROR,
            ("Warning: Unable to write %d to register %d\n", val, reg));
        return -1;
    }

    close(fd);
    return 0;
}

int main(int argc, char *argv[]) {

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
    buffer[BUFFERSIZE - 1] = '\n';
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
            buffer[BUFFERSIZE - 1] = '\n';
            if (i2c_write(ival) < 0)
                strcpy(buffer, "fail ");
            else
                strcpy(buffer, "success ");
            LOG(logINFO, ("Sending: '%s'\n", buffer));
            n = write(fd, buffer, BUFFERSIZE);
            LOG(logDEBUG1, ("Sent %d Bytes\n", n));
            break;

        case 'g':
            ival = i2c_read();
            // ok/ fail
            memset(buffer, 0, BUFFERSIZE);
            buffer[BUFFERSIZE - 1] = '\n';
            if (ival < 0)
                strcpy(buffer, "fail ");
            else
                strcpy(buffer, "success ");
            n = write(fd, buffer, BUFFERSIZE);
            LOG(logINFO, ("Sending: '%s'\n", buffer));
            LOG(logDEBUG1, ("Sent %d Bytes\n", n));
            // value
            memset(buffer, 0, BUFFERSIZE);
            buffer[BUFFERSIZE - 1] = '\n';
            if (ival >= 0) {
                LOG(logINFO, ("Sending: '%d'\n", ival));
                sprintf(buffer, "%d ", ival);
                n = write(fd, buffer, BUFFERSIZE);
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
