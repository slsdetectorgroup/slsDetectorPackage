// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#define UART_DEVICE "/dev/ttyAL0"

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...)                                              \
    fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__,   \
            ##args)
#else
#define DEBUG_PRINT(fmt, args...) /* Don't do anything in release builds */
#endif

struct termios oldtio;
int uart;
int update_files = 1;

void intHandler(int signo) {
    DEBUG_PRINT("signo %d received\n", signo);
    if (signo == SIGINT) {
        DEBUG_PRINT("SIGINT handling\n");
        tcsetattr(uart, TCSANOW, &oldtio);
        close(uart);
        exit(0);
    }
    if (signo == SIGUSR1) {
        DEBUG_PRINT("SIGUSR1 handling\n");
        update_files = 1;
    }
}

void kill_servers() {
    system("killall gotthard2DetectorServer mythen3DetectorServer "
           "gotthard2DetectorServer_developer mythen3DetectorServer_developer");
}

void set_hv_zero() {
    FILE *hv;
    hv = fopen("/etc/devlinks/hvdac", "w");
    if (hv == NULL)
        fprintf(stderr, "Could not open hvdac file.\n");
    else {
        fputc('0', hv);
        fclose(hv);
    }
}

void disable_chip_power() {
    unsigned *mem_ptr;
    unsigned mem_val;
    unsigned mem_address = 0x18040000;
    int mem_desc;
    mem_desc = open("/dev/mem", O_RDWR | O_SYNC, 0);
    if (mem_desc == -1)
        fprintf(stderr, "Cannot open /dev/mem\n");
    else {
        mem_ptr =
            (unsigned *)mmap(0, 0x1000, PROT_READ | PROT_WRITE,
                             MAP_FILE | MAP_SHARED, mem_desc, mem_address);
        if (mem_ptr == MAP_FAILED)
            fprintf(stderr, "Cannot map memory area\n");
        else {
            mem_val = mem_ptr[0x84 / 4];
            mem_val = mem_val & 0x7FFFFFFF; // Removes top bit
            mem_ptr[0x84 / 4] = mem_val;
            munmap((unsigned *)mem_address, 0x1000);
        }
        close(mem_desc);
    }
}

void prepare_for_shutdown() {
    kill_servers();
    set_hv_zero();
    disable_chip_power();
    system("sync");
    return;
}

int uart_setup(struct termios *old_settings) {
    int desc;
    struct termios newtio;
    desc = open(UART_DEVICE, O_RDWR | O_NOCTTY);
    if (desc < 0) {
        perror(UART_DEVICE);
        exit(-1);
    }
    tcgetattr(desc, old_settings);
    newtio.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR | IGNCR | IXANY;
    newtio.c_oflag = 0;
    newtio.c_lflag = ICANON;
    tcflush(desc, TCIFLUSH);
    tcsetattr(desc, TCSANOW, &newtio);
    return desc;
}

int get_boot_condition() {
    FILE *ru_trig_cond;
    char ru_trig_cond_content[10];
    int bytes_read;

    ru_trig_cond = fopen("/sys/devices/platform/sopc@0/18000000.bridge/"
                         "180000c0.remoteupdate/remote_update/trig_cond",
                         "r");
    if (ru_trig_cond == NULL)
        fprintf(stderr, "Cannot read remote update status\n");
    bytes_read = fread(ru_trig_cond_content, 1, 10, ru_trig_cond);
    if (bytes_read < 4)
        fprintf(stderr, "Invalid file content\n");
    return strtoul(ru_trig_cond_content, NULL, 0);
}

void write_cmd_in_file(char *cmd, char *filename, int lines) {
    int res, i;
    char buf[255];
    FILE *cmd_file;
    cmd_file = fopen(filename, "w");
    write(uart, cmd, 1);
    for (i = 0; i < lines; i++) {
        res = read(uart, buf, 255);
        fwrite(buf, 1, res, cmd_file);
    }
    fclose(cmd_file);
}

int main(int argc, char *argv[]) {
    char buf[255];
    fd_set readfs;
    int maxfd;
    int res;
    char factory_image_detected_cmd = 'f';
    char get_voltage_cmd = 'u';
    char get_versions_cmd = 'v';
    char get_eventlog_cmd = 'e';

    uart = uart_setup(&oldtio);
    signal(SIGINT, intHandler);
    signal(SIGUSR1, intHandler);

    maxfd = uart + 1;

    if (get_boot_condition() != 4)
        write(uart, &factory_image_detected_cmd, 1);

    while (1) {
        if (update_files == 1) {
            DEBUG_PRINT("Updating files...\n");
            write_cmd_in_file(&get_voltage_cmd, "/tmp/uc_voltage", 1);
            write_cmd_in_file(&get_versions_cmd, "/tmp/uc_versions", 2);
            write_cmd_in_file(&get_eventlog_cmd, "/tmp/uc_eventlog", 16);
            update_files = 0;
        }
        FD_SET(uart, &readfs);
        res = select(maxfd, &readfs, NULL, NULL, NULL);
        if ((res == -1) && (errno == EINTR)) // If SIGUSR1 occured
            continue;

        if (FD_ISSET(uart, &readfs)) {
            res = read(uart, buf, 255);
            buf[res] = 0;
            DEBUG_PRINT("-->%s:%d<--", buf, res);
            if ((buf[0] == 'S') && (res < 5)) {
                prepare_for_shutdown();
                exit(0);
            }
        }
        usleep(1000);
    }

    // Should never get here
    tcsetattr(uart, TCSANOW, &oldtio);

    return 0;
}
