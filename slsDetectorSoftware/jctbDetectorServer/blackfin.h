#ifndef BLACKFIN_H
#define BLACKFIN_H

#define CSP0 0x20200000
#define MEM_SIZE 0x100000
#ifndef OLDVERSION 
#define MEM_MAP_SHIFT 1
#endif
#ifdef OLDVERSION 
#define MEM_MAP_SHIFT 11
#endif
#include <sys/types.h>

int mapCSP0(void);

u_int16_t bus_r16(u_int32_t offset);
u_int16_t bus_w16(u_int32_t offset, u_int16_t data);//aldos function
u_int32_t bus_w(u_int32_t offset, u_int32_t data);
u_int32_t bus_r(u_int32_t offset);

int64_t set64BitReg(int64_t value, int aLSB, int aMSB);
int64_t get64BitReg(int aLSB, int aMSB);


#endif
