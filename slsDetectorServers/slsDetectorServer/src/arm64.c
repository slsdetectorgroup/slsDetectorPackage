// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "arm64.h"
#include "RegisterDefs.h"
#include "clogger.h"
#include "common.h"
#include "sls/ansi.h"
#include "sls/sls_detector_defs.h"

#include <fcntl.h>    // open
#include <string.h>   // memcpy
#include <sys/mman.h> // mmap

/* global variables */
#define CSP0          (0xB0080000)
#define CSP1          (0xB0050000) // udp
#define CSP2          (0xA0000000)
#define MEM_SIZE_CSP0 (0x20000)
#define MEM_SIZE_CSP1 (0x2000) // smaller size for udp
#define MEM_SIZE_CSP2 (0x4000) // smaller size for udp

u_int32_t *csp0base = 0;
u_int32_t *csp1base = 0;
u_int32_t *csp2base = 0;

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

void bus_w_csp1(u_int32_t offset, u_int32_t data) {
    volatile u_int32_t *ptr1;
    ptr1 = (u_int32_t *)(csp1base + offset / (sizeof(u_int32_t)));
    *ptr1 = data;
}

u_int32_t bus_r_csp1(u_int32_t offset) {
    volatile u_int32_t *ptr1;
    ptr1 = (u_int32_t *)(csp1base + offset / (sizeof(u_int32_t)));
    return *ptr1;
}

void bus_w_csp2(u_int32_t offset, u_int32_t data) {
    volatile u_int32_t *ptr1;
    ptr1 = (u_int32_t *)(csp2base + offset / (sizeof(u_int32_t)));
    *ptr1 = data;
}

u_int32_t bus_r_csp2(u_int32_t offset) {
    volatile u_int32_t *ptr1;
    ptr1 = (u_int32_t *)(csp2base + offset / (sizeof(u_int32_t)));
    return *ptr1;
}

int bus_w_block(u_int32_t offset, const void *src, size_t size,
                bool fixedAddress) {
    if (size == 0 || size % sizeof(uint32_t) != 0) {
        LOG(logERROR, ("Could not write block: size %zu is not a non-zero "
                       "multiple of %zu\n",
                       size, sizeof(uint32_t)));
        return FAIL;
    }
    const uint32_t step = fixedAddress ? 0 : sizeof(uint32_t);
    uint32_t words[size / sizeof(uint32_t)];
    memcpy(words, src, size);
    for (size_t i = 0; i != sizeof(words) / sizeof(words[0]); ++i) {
        bus_w(offset + i * step, words[i]);
    }
    return OK;
}

int bus_r_block(u_int32_t offset, void *dst, size_t size, bool fixedAddress) {
    if (size == 0 || size % sizeof(uint32_t) != 0) {
        LOG(logERROR, ("Could not read block: size %zu is not a non-zero "
                       "multiple of %zu\n",
                       size, sizeof(uint32_t)));
        return FAIL;
    }
    const uint32_t step = fixedAddress ? 0 : sizeof(uint32_t);
    uint32_t words[size / sizeof(uint32_t)];
    for (size_t i = 0; i != sizeof(words) / sizeof(words[0]); ++i) {
        words[i] = bus_r(offset + i * step);
    }
    memcpy(dst, words, size);
    return OK;
}

int bus_w_csp1_block(const void *src, size_t size) {
    uint32_t step = sizeof(uint32_t);
    if (size == 0 || size % sizeof(uint32_t) != 0) {
        LOG(logERROR, ("Could not write block: size %zu is not a non-zero "
                       "multiple of %zu\n",
                       size, sizeof(uint32_t)));
        return FAIL;
    }
    uint32_t words[size / sizeof(uint32_t)];
    memcpy(words, src, size);
    for (size_t i = 0; i != sizeof(words) / sizeof(words[0]); ++i) {
        bus_w_csp1(i * step, words[i]);
    }
    return OK;
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

u_int32_t readRegister(u_int32_t offset) { return bus_r(offset); }

u_int32_t writeRegister(u_int32_t offset, u_int32_t data) {
    bus_w(offset, data);
    return readRegister(offset);
}

int mapCSP0(void) {
    LOG(logINFO, ("Mapping memory\n"));
    u_int32_t csps[3] = {CSP0, CSP1, CSP2};
    u_int32_t **cspbases[3] = {&csp0base, &csp1base, &csp2base};
    u_int32_t memsize[3] = {MEM_SIZE_CSP0, MEM_SIZE_CSP1, MEM_SIZE_CSP2};
    char names[3][10] = {"csp0base", "csp1base", "csp2base"};

    for (int i = 0; i < 3; ++i) {
        // if not mapped
        if (*cspbases[i] == 0) {
            LOG(logINFO, ("\tMapping memory for %s\n", names[i]));
#ifdef VIRTUAL
            *cspbases[i] = malloc(memsize[i]);
            if (*cspbases[i] == NULL) {
                LOG(logERROR,
                    ("Could not allocate virtual memory of size %d for %s.\n",
                     memsize[i], names[i]));
                return FAIL;
            }
            LOG(logINFO, ("\tmemory allocated for %s\n", names[i]));
#else
            int fd = open("/dev/mem", O_RDWR | O_SYNC, 0);
            if (fd == -1) {
                LOG(logERROR, ("Can't find /dev/mem for %s\n", names[i]));
                return FAIL;
            }
            LOG(logDEBUG1,
                ("\t/dev/mem opened for %s, (CSP:0x%x)\n", names[i], csps[i]));
            *cspbases[i] =
                (u_int32_t *)mmap(0, memsize[i], PROT_READ | PROT_WRITE,
                                  MAP_FILE | MAP_SHARED, fd, csps[i]);
            if (*cspbases[i] == MAP_FAILED) {
                LOG(logERROR, ("Can't map memmory area for %s\n", names[i]));
                return FAIL;
            }
#endif
            LOG(logINFO,
                ("\t%s mapped of size %d from %p to %p,(CSP:0x%x) \n", names[i],
                 memsize[i], *cspbases[i], *cspbases[i] + memsize[i], csps[i]));
            // LOG(logINFO, ("Status Register: %08x\n", bus_r(STATUS_REG)));
        } else
            LOG(logINFO, ("\tMemory %s already mapped before\n", names[i]));
    }
    return OK;
}
