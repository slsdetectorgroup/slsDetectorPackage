#ifndef BLACKFIN_H
#define BLACKFIN_H

#include "ansi.h"

#include <stdio.h>
#include <fcntl.h>		// open
#include <sys/mman.h>	// mmap


/* global variables */
u_int64_t CSP0BASE = 0;
#define CSP0 0x20200000
#define MEM_SIZE 0x100000
#define MEM_MAP_SHIFT 1



/**
 * Write into a 16 bit register
 * @param offset address offset
 * @param data 16 bit data
 */
void bus_w16(u_int32_t offset, u_int16_t data) {
	volatile u_int16_t  *ptr1;
	ptr1=(u_int16_t*)(CSP0BASE+offset*2);
	*ptr1=data;
}

/**
 * Read from a 16 bit register
 * @param offset address offset
 * @retuns 16 bit data read
 */
u_int16_t bus_r16(u_int32_t offset){
	volatile u_int16_t *ptr1;
	ptr1=(u_int16_t*)(CSP0BASE+offset*2);
	return *ptr1;
}

/**
 * Write into a 32 bit register
 * @param offset address offset
 * @param data 32 bit data
 */
void bus_w(u_int32_t offset, u_int32_t data) {
	volatile  u_int32_t  *ptr1;
	ptr1=(u_int32_t*)(CSP0BASE+offset*2);
	*ptr1=data;
}

/**
 * Read from a 32 bit register
 * @param offset address offset
 * @retuns 32 bit data read
 */
u_int32_t bus_r(u_int32_t offset) {
	volatile u_int32_t  *ptr1;
	ptr1=(u_int32_t*)(CSP0BASE+offset*2);
	return *ptr1;
}

/**
 * Read from a 64 bit register
 * @param aLSB LSB offset address
 * @param aMSB MSB offset address
 * @returns 64 bit data read
 */
int64_t get64BitReg(int aLSB, int aMSB){
	int64_t v64;
	u_int32_t vLSB,vMSB;
	vLSB=bus_r(aLSB);
	vMSB=bus_r(aMSB);
	v64=vMSB;
	v64=(v64<<32) | vLSB;
	printf(" reg64(%x,%x) %x %x %llx\n", aLSB, aMSB, vLSB, vMSB, (long long unsigned int)v64);
	return v64;
}

/**
 * Write into a 64 bit register
 * @param value 64 bit data
 * @param aLSB LSB offset address
 * @param aMSB MSB offset address
 * @returns 64 bit data read
 */
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

/**
 * Read from a 32 bit register (literal register value provided by client)
 * @param offset address offset
 * @retuns 32 bit data read
 */
u_int32_t readRegister(u_int32_t offset) {
	return bus_r(offset << MEM_MAP_SHIFT);
}

/**
 * Write into a 32 bit register (literal register value provided by client)
 * @param offset address offset
 * @param data 32 bit data
 */
u_int32_t writeRegister(u_int32_t offset, u_int32_t data) {
	bus_w(offset << MEM_MAP_SHIFT, data);
	return readRegister(offset);
}


/**
 * Map FPGA
 */
int mapCSP0(void) {
	// if not mapped
	if (CSP0BASE == 0) {
		printf("Mapping memory\n");
#ifdef VIRTUAL
		CSP0BASE = malloc(MEM_SIZE);
		if (CSP0BASE == NULL) {
		    cprintf(BG_RED, "Error: Could not allocate virtual memory.\n");
		    return FAIL;
		}
		printf("memory allocated\n");
#else
		int fd;
		fd = open("/dev/mem", O_RDWR | O_SYNC, 0);
		if (fd == -1) {
			cprintf(BG_RED, "Error: Can't find /dev/mem\n");
			return FAIL;
		}
#ifdef VERBOSE
		printf("/dev/mem opened\n");
#endif
		CSP0BASE = mmap(0, MEM_SIZE, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, CSP0);
		if (CSP0BASE == MAP_FAILED) {
			cprintf(BG_RED, "Error: Can't map memmory area\n");
			return FAIL;
		}
#endif
		printf("CSPOBASE mapped from 0x%llx to 0x%llx\n",
				(long long unsigned int)CSP0BASE,
				(long long unsigned int)(CSP0BASE+MEM_SIZE));
		printf("Status Register: %08x\n",bus_r(STATUS_REG));

	}else
		printf("Memory already mapped before\n");
	return OK;
}


#endif	//BLACKFIN_H
