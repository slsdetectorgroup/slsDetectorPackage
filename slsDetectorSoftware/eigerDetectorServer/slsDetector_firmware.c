

#include "sls_detector_defs.h"

#include "slsDetector_firmware.h"
#include "slsDetectorServer_defs.h"
#include "registers.h"


#include <stdio.h>
#include <string.h>



u_int32_t CSP0BASE;


int nModBoard;
int nModY		=	NMAXMOD;
int nModX		=	NMAXMOD;
int dynamicRange=	DYNAMIC_RANGE;
int dataBytes	=	NMAXMOD*NCHIP*NCHAN*2;
int masterMode	=	NO_MASTER;
int syncMode	=	NO_SYNCHRONIZATION;
int timingMode	=	AUTO_TIMING;


#ifdef SLS_DETECTOR_FUNCTION_LIST
extern const int nChans;
extern const int nChips;
extern const int nDacs;
extern const int nAdcs;
#endif
#ifndef SLS_DETECTOR_FUNCTION_LIST
const int nChans	=	NCHAN;
const int nChips	=	NCHIP;
const int nDacs		=	NDAC;
const int nAdcs		=	NADC;
#endif




int mapCSP0(void) {
	printf("Mapping memory\n");
#ifdef VIRTUAL
	CSP0BASE = (u_int32_t)malloc(MEM_SIZE);
	printf("memory allocated\n");
#endif
	printf("CSPOBASE=from %08x to %x\n",CSP0BASE,CSP0BASE+MEM_SIZE);

	return OK;
}



u_int32_t bus_w(u_int32_t offset, u_int32_t data) {
	volatile u_int32_t *ptr1;
	ptr1=(u_int32_t*)(CSP0BASE+offset);
	*ptr1=data;
	return OK;
}



u_int32_t bus_r(u_int32_t offset) {
	volatile u_int32_t *ptr1;
	ptr1=(u_int32_t*)(CSP0BASE+offset);
	return *ptr1;
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

int64_t get64BitReg(int aLSB, int aMSB){
	int64_t v64;
	u_int32_t vLSB,vMSB;
	vLSB=bus_r(aLSB);
	vMSB=bus_r(aMSB);
	v64=vMSB;
	v64=(v64<<32) | vLSB;
	return v64;
}


int64_t setFrames(int64_t value){
	return set64BitReg(value,  SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG);
}
int64_t getFrames(){
	return get64BitReg(GET_FRAMES_LSB_REG, GET_FRAMES_MSB_REG);
}


int64_t setExposureTime(int64_t value){
	/* time is in ns */
	if (value!=-1)
		value*=(1E-9*CLK_FREQ);
	return set64BitReg(value,SET_EXPTIME_LSB_REG, SET_EXPTIME_MSB_REG)/(1E-9*CLK_FREQ);
}
int64_t getExposureTime(){
	return get64BitReg(GET_EXPTIME_LSB_REG, GET_EXPTIME_MSB_REG)/(1E-9*CLK_FREQ);
}


int64_t setGates(int64_t value){
	return set64BitReg(value, SET_GATES_LSB_REG, SET_GATES_MSB_REG);
}
int64_t getGates(){
	return get64BitReg(GET_GATES_LSB_REG, GET_GATES_MSB_REG);
}


int64_t setPeriod(int64_t value){
	/* time is in ns */
	if (value!=-1)
		value*=(1E-9*CLK_FREQ);
	return set64BitReg(value,SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG)/(1E-9*CLK_FREQ);
}
int64_t getPeriod(){
	return get64BitReg(GET_PERIOD_LSB_REG, GET_PERIOD_MSB_REG)/(1E-9*CLK_FREQ);
}


int64_t setDelay(int64_t value){
	/* time is in ns */
	if (value!=-1) {
		value*=(1E-9*CLK_FREQ);
	}
	return set64BitReg(value,SET_DELAY_LSB_REG, SET_DELAY_MSB_REG)/(1E-9*CLK_FREQ);
}
int64_t getDelay(){
	return get64BitReg(GET_DELAY_LSB_REG, GET_DELAY_MSB_REG)/(1E-9*CLK_FREQ);
}


int64_t setTrains(int64_t value){
	return set64BitReg(value,  SET_TRAINS_LSB_REG, SET_TRAINS_MSB_REG);
}
int64_t getTrains(){
	return get64BitReg(GET_TRAINS_LSB_REG, GET_TRAINS_MSB_REG);
}



int64_t setProbes(int64_t value){
	return 0;
}
int64_t getProbes(){
  return 0;
}


