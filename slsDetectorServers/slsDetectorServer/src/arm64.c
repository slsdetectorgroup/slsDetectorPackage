// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "arm64.h"
#include "RegisterDefs.h"
#include "clogger.h"
#include "common.h"
#include "sls/ansi.h"
#include "sls/sls_detector_defs.h"

#include <fcntl.h>    // open
#include <sys/mman.h> // mmap


/* global variables */
#define CSP0     (0xB0010000)/// 0xB008_0000
#define MEM_SIZE 0x100000

u_int32_t *csp0base = 0;

void bus_w(u_int32_t offset, u_int32_t data) {
    volatile u_int32_t *ptr1;
    ptr1 = (u_int32_t *)(csp0base + offset / (sizeof(u_int32_t)));
    *ptr1 = data;
}

u_int32_t bus_r(u_int32_t offset) {
    volatile u_int32_t *ptr1;
    ptr1 = (u_int32_t *)(csp0base + offset / (sizeof(u_int32_t)));
    return *ptr1;
}

uint64_t getU64BitReg(int aLSB, int aMSB) {
    uint64_t retval = bus_r(aMSB);
    retval = (retval << 32) | bus_r(aLSB);
    return retval;
}

void setU64BitReg(uint64_t value, int aLSB, int aMSB) {
    bus_w(aLSB, value & (0xffffffff));
    bus_w(aMSB, (value >> 32) & (0xffffffff));
}

int mapCSP0(void) {
    // if not mapped
    if (csp0base == 0) {
        LOG(logINFO, ("Mapping memory\n"));
#ifdef VIRTUAL
        csp0base = malloc(MEM_SIZE);
        if (csp0base == NULL) {
            LOG(logERROR, ("Could not allocate virtual memory.\n"));
            return FAIL;
        }
        LOG(logINFO, ("memory allocated\n"));
#else
        int fd;
        fd = open("/dev/mem", O_RDWR | O_SYNC, 0);
        if (fd == -1) {
            LOG(logERROR, ("Can't find /dev/mem\n"));
            return FAIL;
        }
        LOG(logDEBUG1, ("/dev/mem opened\n"));
        csp0base = (u_int32_t*)mmap(0, MEM_SIZE, PROT_READ | PROT_WRITE,
                        MAP_FILE | MAP_SHARED, fd, CSP0);
        if (csp0base == MAP_FAILED) {
            LOG(logERROR, ("Can't map memmory area\n"));
            return FAIL;
        }
#endif
        LOG(logINFO, ("csp0base mapped from %p to %p\n", csp0base,
                      (csp0base + MEM_SIZE)));
    } else
        LOG(logINFO, ("Memory already mapped before\n"));
    return OK;
}

