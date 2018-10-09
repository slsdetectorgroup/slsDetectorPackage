#ifndef FIRMWARE_FUNCS_H
#define FIRMWARE_FUNCS_H


#include "sls_detector_defs.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


int mapCSP0(void);
u_int16_t bus_r16(u_int32_t offset);
u_int16_t bus_w16(u_int32_t offset, u_int16_t data);//aldos function
u_int32_t bus_w(u_int32_t offset, u_int32_t data);
u_int32_t bus_r(u_int32_t offset);

int initDetector();
int setDefaultDacs();
void setMasterSlaveConfiguration();
int configureADC();
int setPhaseShiftOnce();
int setPhaseShift(int numphaseshift);
int cleanFifo();
int setDAQRegister();
u_int32_t putout(char *s);
int setConfigurationRegister(int d);
int sendviaUDP(int d);
int setDACRegister(int idac, int val);


u_int32_t setExtSignal(enum externalSignalFlag  mode);
int  getExtSignal();
u_int32_t setFPGASignal(enum externalSignalFlag  mode);
int  getFPGASignal();

int setTiming(int t);

u_int64_t getDetectorNumber();
u_int32_t getFirmwareVersion();
u_int32_t  getFirmwareSVNVersion();

u_int32_t testFpga(void);
int testBus(void);

int initHighVoltage(int val);
int getTemperature(int tempSensor);
int setSettings(int i);
int initConfGain(int isettings,int val);
ROI* setROI(int n, ROI arg[], int *retvalsize, int *ret);
int setADC(int adc);
int configureMAC(int ipad, long long int macad, long long int detectormacadd, int detipad, int ival, int udpport);
int getAdcConfigured();


int64_t set64BitReg(int64_t value, int aLSB, int aMSB);
int64_t get64BitReg(int aLSB, int aMSB);

int64_t setFrames(int64_t value);
int64_t getFrames();

int64_t setExposureTime(int64_t value);
int64_t getExposureTime();

int64_t setGates(int64_t value);
int64_t getGates();

int64_t setPeriod(int64_t value);
int64_t getPeriod();

int64_t setDelay(int64_t value);
int64_t getDelay();

int64_t setTrains(int64_t value);
int64_t getTrains();

int64_t getActualTime();
int64_t getMeasurementTime();

u_int32_t fifoReadStatus();
u_int32_t fifo_full(void);
u_int32_t runBusy(void);
u_int32_t runState(void);
int startStateMachine();
int stopStateMachine();
int startReadOut();
void waitForAcquisitionFinish();
int getStatus();

int loadImage(int index, short int ImageVals[]);
int readCounterBlock(int startACQ, short int CounterVals[]);
int resetCounterBlock(int startACQ);

int copyModule(sls_detector_module *destMod, sls_detector_module *srcMod);

int setDAC(int ind,int val,int mV, int retval[]);
int getDAC(int ind);

int setModule(sls_detector_module);
void getModule(sls_detector_module*);

void initDACs(int* v);
void initDAC(int dac_addr, int value);
void clearDACSregister();
void nextDAC();
void program_one_dac(int addr, int value);






#endif
