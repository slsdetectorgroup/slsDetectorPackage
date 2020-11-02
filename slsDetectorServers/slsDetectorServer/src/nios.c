#include "nios.h"
#include "RegisterDefs.h"
#include "sls/ansi.h"
#include "clogger.h"
#include "common.h"
#include "sls/sls_detector_defs.h"

#include <fcntl.h> // open
#include <string.h>
#include <sys/mman.h>    // mmap
#include <sys/utsname.h> // uname

/* global variables */
u_int32_t *csp0base = 0;
#define CSP0 0x18060000
#define MEM_SIZE                                                               \
    0x100000 // TODO  (1804 0000 - 1804 07FF = 800 * 4 = 2000), (1806 0000 =
             // 10000* 4 = 40000)

u_int32_t *csp1base = 0;
#define CSP1 0x18040000

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

u_int32_t readRegister(u_int32_t offset) { return bus_r(offset); }

u_int32_t writeRegister(u_int32_t offset, u_int32_t data) {
    bus_w(offset, data);
    return readRegister(offset);
}

int mapCSP0(void) {
    u_int32_t csps[2] = {CSP0, CSP1};
    u_int32_t **cspbases[2] = {&csp0base, &csp1base};
    char names[2][10] = {"csp0base", "csp1base"};

    for (int i = 0; i < 2; ++i) {
        // if not mapped
        if (*cspbases[i] == 0) {
            LOG(logINFO, ("Mapping memory for %s\n", names[i]));
#ifdef VIRTUAL
            *cspbases[i] = malloc(MEM_SIZE);
            if (*cspbases[i] == NULL) {
                LOG(logERROR,
                    ("Could not allocate virtual memory for %s.\n", names[i]));
                return FAIL;
            }
            LOG(logINFO, ("memory allocated for %s\n", names[i]));
#else
            int fd = open("/dev/mem", O_RDWR | O_SYNC, 0);
            if (fd == -1) {
                LOG(logERROR, ("Can't find /dev/mem for %s\n", names[i]));
                return FAIL;
            }
            LOG(logDEBUG1,
                ("/dev/mem opened for %s, (CSP:0x%x)\n", names[i], csps[i]));
            *cspbases[i] =
                (u_int32_t *)mmap(0, MEM_SIZE, PROT_READ | PROT_WRITE,
                                  MAP_FILE | MAP_SHARED, fd, csps[i]);
            if (*cspbases[i] == MAP_FAILED) {
                LOG(logERROR, ("Can't map memmory area for %s\n", names[i]));
                return FAIL;
            }
#endif
            LOG(logINFO, ("%s mapped from %p to %p,(CSP:0x%x) \n", names[i],
                          *cspbases[i], *cspbases[i] + MEM_SIZE, csps[i]));
            // LOG(logINFO, ("Status Register: %08x\n", bus_r(STATUS_REG)));
        } else
            LOG(logINFO, ("Memory %s already mapped before\n", names[i]));
    }
    return OK;
}

u_int32_t *Nios_getBaseAddress() { return csp0base; }

int Nios_checkKernelVersion(char *expectedVersion) {
    // extract kernel date string
    struct utsname buf;
    if (uname(&buf) == -1) {
        LOG(logERROR, ("Could not get kernel version\n"));
        return FAIL;
    }

    // remove first word (#version number)
    const char *ptr = strchr(buf.version, ' ');
    if (ptr == NULL) {
        LOG(logERROR, ("Could not parse kernel version\n"));
        return FAIL;
    }
    char output[256];
    memset(output, 0, 256);
    strcpy(output, buf.version + (ptr - buf.version + 1));

    // convert kernel date string into time
    time_t kernelDate;
    if (GetTimeFromString(output, &kernelDate) == FAIL) {
        LOG(logERROR, ("Could not parse retrieved kernel date, %s\n", output));
        return FAIL;
    }

    // convert expected date into time
    time_t expDate;
    if (GetTimeFromString(expectedVersion, &expDate) == FAIL) {
        LOG(logERROR,
            ("Could not parse expected kernel date, %s\n", expectedVersion));
        return FAIL;
    }

    // compare if kernel time is older than expected time
    if (kernelDate < expDate) {
        LOG(logERROR, ("Kernel Version Incompatible (too old)! Expected: [%s], "
                       "Got [%s]\n",
                       expectedVersion, output));
        return FAIL;
    }

    LOG(logINFOBLUE, ("Kernel Version Compatible: %s [min.: %s]\n", output,
                      expectedVersion));
    return OK;
}