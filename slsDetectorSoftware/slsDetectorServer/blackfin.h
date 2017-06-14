#ifndef BLACKFIN_H
#define BLACKFIN_H

#include <stdio.h>
#include <fcntl.h>		// open
#include <sys/mman.h>	// mmap


/* global variables */
u_int32_t CSP0BASE = 0;
#define CSP0 0x20200000
#define MEM_SIZE 0x100000




void bus_w16(u_int32_t offset, u_int16_t data) {
	volatile u_int16_t  *ptr1;
	ptr1=(u_int16_t*)(CSP0BASE+offset*2);
	*ptr1=data;
}

u_int16_t bus_r16(u_int32_t offset){
	volatile u_int16_t *ptr1;
	ptr1=(u_int16_t*)(CSP0BASE+offset*2);
	return *ptr1;
}

void bus_w(u_int32_t offset, u_int32_t data) {
	volatile  u_int32_t  *ptr1;
	ptr1=(u_int32_t*)(CSP0BASE+offset*2);
	*ptr1=data;
}

u_int32_t bus_r(u_int32_t offset) {
	volatile u_int32_t  *ptr1;
	ptr1=(u_int32_t*)(CSP0BASE+offset*2);
	return *ptr1;
}

int64_t get64BitReg(int aLSB, int aMSB){
	int64_t v64;
	u_int32_t vLSB,vMSB;
	vLSB=bus_r(aLSB);
	vMSB=bus_r(aMSB);
	v64=vMSB;
	v64=(v64<<32) | vLSB;
	printf(" reg64(%x,%x) %x %x %llx\n", aLSB, aMSB, vLSB, vMSB, v64);
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

u_int32_t readRegister(u_int32_t offset) {
	return bus_r(offset << 11);
}

u_int32_t writeRegister(u_int32_t offset, u_int32_t data) {
	bus_w(offset << 11, data);
	return readRegister(offset);
}



int mapCSP0(void) {
	// if not mapped
	if (!CSP0BASE) {
		printf("Mapping memory\n");
#ifdef VIRTUAL
		CSP0BASE = malloc(MEM_SIZE);
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
		CSP0BASE = (u_int32_t)mmap(0, MEM_SIZE, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, CSP0);
		if (CSP0BASE == (u_int32_t)MAP_FAILED) {
			cprintf(BG_RED, "Error: Can't map memmory area\n");
			return FAIL;
		}
		printf("CSPOBASE mapped from %08x to %08x\n",CSP0BASE,CSP0BASE+MEM_SIZE);
#endif
		printf("Status Register: %08x\n",bus_r(STATUS_REG));
	}else
		printf("Memory already mapped before\n");
	return OK;
}


#endif	//BLACKFIN_H
