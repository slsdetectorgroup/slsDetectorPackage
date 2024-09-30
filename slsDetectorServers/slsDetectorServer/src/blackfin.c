// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "blackfin.h"
#include "RegisterDefs.h"
#include "clogger.h"
#include "common.h"
#include "sls/ansi.h"
#include "sls/sls_detector_defs.h"

#include <fcntl.h>    // open
#include <sys/mman.h> // mmap

/* global variables */
u_int32_t *csp0base = 0;
#define CSP0     0x20200000
#define MEM_SIZE 0x100000

#ifdef JUNGFRAUD

extern void configureChip();
#endif

void bus_w16(u_int32_t offset, u_int16_t data) {
    volatile u_int16_t *ptr1;
    ptr1 = (u_int16_t *)(csp0base + offset / 2);
    *ptr1 = data;
}

u_int16_t bus_r16(u_int32_t offset) {
    volatile u_int16_t *ptr1;
    ptr1 = (u_int16_t *)(csp0base + offset / 2);
    return *ptr1;
}

void bus_w(u_int32_t offset, u_int32_t data) {
    volatile u_int32_t *ptr1;
    ptr1 = (u_int32_t *)(csp0base + offset / 2);
    *ptr1 = data;
}

u_int32_t bus_r(u_int32_t offset) {
    volatile u_int32_t *ptr1;
    ptr1 = (u_int32_t *)(csp0base + offset / 2);
    return *ptr1;
}

int64_t get64BitReg(int aLSB, int aMSB) {
    int64_t v64;
    u_int32_t vLSB, vMSB;
    vLSB = bus_r(aLSB);
    vMSB = bus_r(aMSB);
    v64 = vMSB;
    v64 = (v64 << 32) | vLSB;
    LOG(logDEBUG5, (" reg64(%x,%x) %x %x %llx\n", aLSB, aMSB, vLSB, vMSB,
                    (long long unsigned int)v64));
    return v64;
}

int64_t set64BitReg(int64_t value, int aLSB, int aMSB) {
    int64_t v64;
    u_int32_t vLSB, vMSB;
    if (value != -1) {
        vLSB = value & (0xffffffff);
        bus_w(aLSB, vLSB);
        v64 = value >> 32;
        vMSB = v64 & (0xffffffff);
        bus_w(aMSB, vMSB);
    }
    return get64BitReg(aLSB, aMSB);
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

u_int32_t readRegister(u_int32_t offset) {
    return bus_r(offset << MEM_MAP_SHIFT);
}

void writeRegister(u_int32_t offset, u_int32_t data) {
    bus_w(offset << MEM_MAP_SHIFT, data);
}

u_int32_t readRegister16(u_int32_t offset) {
    return (u_int32_t)bus_r16(offset << MEM_MAP_SHIFT);
}

void writeRegister16(u_int32_t offset, u_int32_t data) {
    bus_w16(offset << MEM_MAP_SHIFT, (u_int16_t)data);
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
        csp0base = mmap(0, MEM_SIZE, PROT_READ | PROT_WRITE,
                        MAP_FILE | MAP_SHARED, fd, CSP0);
        if (csp0base == MAP_FAILED) {
            LOG(logERROR, ("Can't map memmory area\n"));
            return FAIL;
        }
#endif
        LOG(logINFO, ("csp0base mapped from %p to %p\n", csp0base,
                      (csp0base + MEM_SIZE)));
        LOG(logINFO, ("Status Register: %08x\n", bus_r(STATUS_REG)));
    } else
        LOG(logINFO, ("Memory already mapped before\n"));
    return OK;
}

uint32_t *Blackfin_getBaseAddress() { return csp0base; }
