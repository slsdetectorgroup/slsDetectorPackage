#pragma once

#include "ansi.h"

#include <fcntl.h>		// open
#include <sys/mman.h>	// mmap


/* global variables */
u_int64_t CSP0BASE = 0;
#define CSP0 0x20200000
#define MEM_SIZE 0x100000


/** I2C defines */
#define I2C_CLOCK_MHZ   (131.25)

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
	FILE_LOG(logDEBUG5, (" reg64(%x,%x) %x %x %llx\n", aLSB, aMSB, vLSB, vMSB, (long long unsigned int)v64));
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
 * Read unsigned 64 bit from a 64 bit register
 * @param aLSB LSB offset address
 * @param aMSB MSB offset address
 * @returns unsigned 64 bit data read
 */
uint64_t getU64BitReg(int aLSB, int aMSB){
	uint64_t retval = bus_r(aMSB);
	retval = (retval << 32) | bus_r(aLSB);
	return retval;
}

/**
 * Write unsigned 64 bit into a 64 bit register
 * @param value unsigned 64 bit data
 * @param aLSB LSB offset address
 * @param aMSB MSB offset address
 */
void setU64BitReg(uint64_t value, int aLSB, int aMSB){
	bus_w(aLSB, value & (0xffffffff));
	bus_w(aMSB, (value >> 32) & (0xffffffff));
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
 * Read from a 16 bit register (literal register value provided by client)
 * @param offset address offset
 * @retuns 16 bit data read
 */
u_int32_t readRegister16(u_int32_t offset) {
    return (u_int32_t)bus_r16(offset << MEM_MAP_SHIFT);
}

/**
 * Write into a 16 bit register (literal register value provided by client)
 * @param offset address offset
 * @param data 16 bit data
 */
u_int32_t writeRegister16(u_int32_t offset, u_int32_t data) {
    bus_w16(offset << MEM_MAP_SHIFT, (u_int16_t)data);
    return readRegister16(offset);
}


/**
 * Map FPGA
 */
int mapCSP0(void) {
	// if not mapped
	if (CSP0BASE == 0) {
	    FILE_LOG(logINFO, ("Mapping memory\n"));
#ifdef VIRTUAL
		CSP0BASE = malloc(MEM_SIZE);
		if (CSP0BASE == NULL) {
		    FILE_LOG(logERROR, ("Could not allocate virtual memory.\n"));
		    return FAIL;
		}
		FILE_LOG(logINFO, ("memory allocated\n"));
#else
		int fd;
		fd = open("/dev/mem", O_RDWR | O_SYNC, 0);
		if (fd == -1) {
		    FILE_LOG(logERROR, ("Can't find /dev/mem\n"));
			return FAIL;
		}
		FILE_LOG(logDEBUG1, ("/dev/mem opened\n"));
		CSP0BASE = mmap(0, MEM_SIZE, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, CSP0);
		if (CSP0BASE == MAP_FAILED) {
		    FILE_LOG(logERROR, ("Can't map memmory area\n"));
			return FAIL;
		}
#endif
		FILE_LOG(logINFO, ("CSPOBASE mapped from 0x%llx to 0x%llx\n",
				(long long unsigned int)CSP0BASE,
				(long long unsigned int)(CSP0BASE+MEM_SIZE)));
		FILE_LOG(logINFO, ("Status Register: %08x\n", bus_r(STATUS_REG)));
	}else
	    FILE_LOG(logINFO, ("Memory already mapped before\n"));
	return OK;
}
