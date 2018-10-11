#include "blackfin.h"

 #include <sys/ipc.h> 
 #include <sys/shm.h>

 #include <sys/time.h> 
 #include <string.h> 
 #include <sys/utsname.h>  
 #include <sys/types.h>
 #include <sys/socket.h>  
 #include <netinet/in.h>
 #include <netinet/tcp.h>
 #include <arpa/inet.h> 
 #include <netdb.h> 
 #include <time.h>  
 #include <sys/time.h> 
 #include <sys/mman.h>
 #include <sys/socket.h>
 #include <sys/stat.h> 
 #include <errno.h> 
 #include <fcntl.h> 
 #include <stdarg.h>
 #include <stdio.h> 
 #include <stdlib.h> 
 #include <string.h> 
 #include <unistd.h> 
#include "server_defs.h"
#include "registers_m.h"

//for memory mapping
u_int32_t CSP0BASE;

u_int16_t volatile  *values;

int mapCSP0(void) {
  printf("Mapping memory\n");
#ifndef VIRTUAL
  int fd;
  fd = open("/dev/mem", O_RDWR | O_SYNC, 0);
  if (fd == -1) {
    printf("\nCan't find /dev/mem!\n");
       return FAIL;
  }
  printf("/dev/mem opened\n");

  CSP0BASE = (u_int32_t)mmap(0, MEM_SIZE, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, CSP0);
  if (CSP0BASE == (u_int32_t)MAP_FAILED) {
    printf("\nCan't map memmory area!!\n");
    return FAIL;
  }
  printf("CSP0 mapped\n");

#endif
#ifdef VIRTUAL
  CSP0BASE = malloc(MEM_SIZE);
  printf("memory allocated\n");
#endif
#ifdef SHAREDMEMORY 
  if ( (res=inism(SMSV))<0) {
    printf("error attaching shared memory! %i",res);
    return FAIL;
  }
#endif
  printf("CSPObase is 0x%08x \n",CSP0BASE);
  printf("CSPOBASE=from %08x to %08x\n",CSP0BASE,CSP0BASE+MEM_SIZE);

  u_int32_t address;
  address = FIFO_DATA_REG;//_OFF;
  //values=(u_int32_t*)(CSP0BASE+address*2);
  values=(u_int16_t*)(CSP0BASE+address*2);
  printf("statusreg=%08x\n",bus_r(STATUS_REG));
  printf("\n\n");
  return OK;
}

u_int16_t bus_r16(u_int32_t offset){
  volatile u_int16_t *ptr1;
  ptr1=(u_int16_t*)(CSP0BASE+offset*2);
  return *ptr1;
}

u_int16_t bus_w16(u_int32_t offset, u_int16_t data) {
  volatile u_int16_t  *ptr1;
  ptr1=(u_int16_t*)(CSP0BASE+offset*2);
  *ptr1=data;
  return OK;
}


u_int32_t bus_w(u_int32_t offset, u_int32_t data) {
 volatile  u_int32_t  *ptr1;

  ptr1=(u_int32_t*)(CSP0BASE+offset*2);
  *ptr1=data;

  return OK;
}


u_int32_t bus_r(u_int32_t offset) {
  volatile u_int32_t  *ptr1;
  ptr1=(u_int32_t*)(CSP0BASE+offset*2);
  return *ptr1;
}

// program dacq settings 

int64_t set64BitReg(int64_t value, int aLSB, int aMSB){
  int64_t v64;
  u_int32_t vLSB,vMSB;
  if (value!=-1) {
    vLSB=value&(0xffffffff);
    bus_w(aLSB,vLSB);
    v64=value>> 32;
    vMSB=v64&(0xffffffff);
    bus_w(aMSB,vMSB);
    //   printf("Wreg64(%x,%x) %08x %08x %016llx\n", aLSB>>11, aMSB>>11, vLSB, vMSB, value);
  }
  return get64BitReg(aLSB, aMSB);

}

int64_t get64BitReg(int aLSB, int aMSB){
  int64_t v64;
  u_int32_t vLSB,vMSB;
  vLSB=bus_r(aLSB);
  vMSB=bus_r(aMSB);
  v64=vMSB;
  v64=(v64<<32) | vLSB;

  // printf("reg64(%x,%x) %x %x %llx\n", aLSB, aMSB, vLSB, vMSB, v64);

  return v64;
}

/* /\** */
/* /\** ramType is DARK_IMAGE_REG or GAIN_IMAGE_REG *\/ */
/* u_int16_t ram_w16(u_int32_t ramType, int adc, int adcCh, int Ch, u_int16_t data) {  */
/*   unsigned int adr = (ramType | adc << 8 | adcCh << 5 | Ch ); */
/*   // printf("Writing to addr:%x\n",adr); */
/*   return bus_w16(adr,data); */
/* } */

/* /\** ramType is DARK_IMAGE_REG or GAIN_IMAGE_REG *\/ */
/* u_int16_t ram_r16(u_int32_t ramType, int adc, int adcCh, int Ch){ */
/*   unsigned int adr = (ramType | adc << 8 | adcCh << 5 | Ch ); */
/*   //  printf("Reading from addr:%x\n",adr); */
/*   return bus_r16(adr); */
/* } */
/* **\/ */
