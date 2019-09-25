#include "nios.h"
#include "RegisterDefs.h"
#include "sls_detector_defs.h"
#include "ansi.h"
#include "clogger.h"

#include <fcntl.h>		// open
#include <sys/mman.h>	// mmap

/* global variables */
u_int32_t* csp0base = 0;
#define CSP0 0x18060000
#define MEM_SIZE 0x100000


void bus_w(u_int32_t offset, u_int32_t data) {
	volatile  u_int32_t  *ptr1;
	ptr1=(u_int32_t*)(csp0base + offset/(sizeof(u_int32_t)));
	*ptr1=data;
}

u_int32_t bus_r(u_int32_t offset) {
	volatile u_int32_t  *ptr1;
	ptr1=(u_int32_t*)(csp0base + offset/(sizeof(u_int32_t))); 
	return *ptr1;
}

int64_t get64BitReg(int aLSB, int aMSB){
	int64_t v64;
	u_int32_t vLSB,vMSB;
	vLSB=bus_r(aLSB);
	vMSB=bus_r(aMSB);
	v64=vMSB;
	v64=(v64<<32) | vLSB;
	FILE_LOG(logDEBUG5, (" reg64(%x,%x) %x %x %llx\n", aLSB, aMSB, vLSB, vMSB, (long long unsigned int)v64));
	return v64;
}

int64_t set64BitReg(int64_t value, int aLSB, int aMSB){
	int64_t v64;
	u_int32_t vLSB,vMSB;
	if (value!=-1) {
		vLSB=value&(0xffffffff);
		bus_w(aLSB,vLSB);
		v64=value>> 32;
		vMSB=v64&(0xffffffff);
		bus_w(aMSB,vMSB);
	}
	return get64BitReg(aLSB, aMSB);

}

uint64_t getU64BitReg(int aLSB, int aMSB){
	uint64_t retval = bus_r(aMSB);
	retval = (retval << 32) | bus_r(aLSB);
	return retval;
}

void setU64BitReg(uint64_t value, int aLSB, int aMSB){
	bus_w(aLSB, value & (0xffffffff));
	bus_w(aMSB, (value >> 32) & (0xffffffff));
}

u_int32_t readRegister(u_int32_t offset) {
	return bus_r(offset);
}

u_int32_t writeRegister(u_int32_t offset, u_int32_t data) {
	bus_w(offset, data);
	return readRegister(offset);
}


int mapCSP0(void) {
	// if not mapped
	if (csp0base == 0) {
	    FILE_LOG(logINFO, ("Mapping memory\n"));
#ifdef VIRTUAL
		csp0base = malloc(MEM_SIZE);
		if (csp0base == NULL) {
		    FILE_LOG(logERROR, ("Could not allocate virtual memory.\n"));
		    return FAIL;
		}
		FILE_LOG(logINFO, ("memory allocated\n"));
#else
		int fd = open("/dev/mem", O_RDWR | O_SYNC, 0);
		if (fd == -1) {
		    FILE_LOG(logERROR, ("Can't find /dev/mem\n"));
			return FAIL;
		}
		FILE_LOG(logDEBUG1, ("/dev/mem opened\n"));
		csp0base = (u_int32_t*)mmap(0, MEM_SIZE, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, CSP0);
		if (csp0base == MAP_FAILED) {
		    FILE_LOG(logERROR, ("Can't map memmory area\n"));
			return FAIL;
		}
#endif
		FILE_LOG(logINFO, ("CSPOBASE mapped from 0x%p to 0x%p\n",
				csp0base, csp0base+MEM_SIZE));
		//FILE_LOG(logINFO, ("Status Register: %08x\n", bus_r(STATUS_REG)));
	}else
	    FILE_LOG(logINFO, ("Memory already mapped before\n"));
	return OK;
}


