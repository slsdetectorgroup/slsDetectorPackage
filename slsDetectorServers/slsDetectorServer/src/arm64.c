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
u_int32_t *csp0base = 0;
#define CSP0     0x20200000
#define MEM_SIZE 0x100000


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
    } else
        LOG(logINFO, ("Memory already mapped before\n"));
    return OK;
}

