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
//#include <asm/page.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>


int mapCSP0(void);

u_int16_t bus_r16(u_int32_t offset);
u_int16_t bus_w16(u_int32_t offset, u_int16_t data);//aldos function
u_int32_t bus_w(u_int32_t offset, u_int32_t data);
u_int32_t bus_r(u_int32_t offset);

int setPhaseShiftOnce();
int phaseStep(int st, int ic);
int cleanFifo();
int setDAQRegister();

u_int32_t putout(char *s, int modnum);
u_int32_t readin(int modnum);
u_int32_t setClockDivider(int d, int ic);
u_int32_t getClockDivider(int ic);

void resetPLL();
u_int32_t setPllReconfigReg(u_int32_t reg, u_int32_t val, int trig);
u_int32_t getPllReconfigReg(u_int32_t reg, int trig);

u_int32_t setSetLength(int d);
u_int32_t getSetLength();
u_int32_t setWaitStates(int d);
u_int32_t getWaitStates();
u_int32_t setTotClockDivider(int d);
u_int32_t getTotClockDivider();
u_int32_t setTotDutyCycle(int d);
u_int32_t getTotDutyCycle();
u_int32_t setOversampling(int d);
u_int32_t adcPipeline(int d);

u_int32_t setExtSignal(int d, enum externalSignalFlag  mode);
int  getExtSignal(int d);

u_int32_t setFPGASignal(int d, enum externalSignalFlag  mode);
int  getFPGASignal(int d);

int setTiming(int t);


int setConfigurationRegister(int d);
int setToT(int d);
int setContinousReadOut(int d);
int startReceiver(int d);

int setDACRegister(int idac, int val, int imod);

int getTemperature(int tempSensor,int imod);
int initHighVoltage(int val,int imod);
int initConfGain(int isettings,int val,int imod);

int setADC(int adc);
int configureMAC(int ipad, long long int macad, long long int detectormacadd, int detipad, int ival, int udpport);
int getAdcConfigured();


u_int64_t getDetectorNumber();
u_int32_t getFirmwareVersion();
int testFifos(void);
u_int32_t testFpga(void);
u_int32_t testRAM(void);
int testBus(void);
int setDigitalTestBit(int ival);

int64_t set64BitReg(int64_t value, int aLSB, int aMSB);
int64_t get64BitReg(int aLSB, int aMSB);

int64_t setFrames(int64_t value);
int64_t getFrames();

int64_t setExposureTime(int64_t value);
int64_t getExposureTime();

int64_t setGates(int64_t value);
int64_t getGates();

int64_t setDelay(int64_t value);
int64_t getDelay();

int64_t setPeriod(int64_t value);
int64_t getPeriod();

int64_t setTrains(int64_t value);
int64_t getTrains();

int64_t setProbes(int64_t value);
int64_t getProbes();

int64_t getProgress();
int64_t setProgress();

int64_t getActualTime();
int64_t getMeasurementTime();
int64_t getFramesFromStart();

u_int32_t runBusy(void); 
u_int32_t runState(void); 
u_int32_t dataPresent(void); 


int startStateMachine();
int stopStateMachine();
int startReadOut();
u_int32_t fifoReset(void);
u_int32_t fifoReadCounter(int fifonum);
u_int32_t fifoReadStatus();


u_int32_t fifo_full(void);



u_int16_t* fifo_read_event();
u_int16_t* fifo_read_frame();
u_int32_t* decode_data(int* datain);
//u_int32_t move_data(u_int64_t* datain, u_int64_t* dataout);
int setDynamicRange(int dr);
int getDynamicRange();
int getNModBoard();
int setNMod(int n);
int setStoreInRAM(int b);
int allocateRAM();
int clearRAM();


int setMaster(int f);
int setSynchronization(int s);

int loadImage(int index, short int ImageVals[]);
int readCounterBlock(int startACQ, short int CounterVals[]);
int resetCounterBlock(int startACQ);

int calibratePedestal(int frames);

uint64_t writePatternWord(int addr, uint64_t word);
uint64_t writePatternIOControl(uint64_t word);
uint64_t writePatternClkControl(uint64_t word);
int setPatternLoop(int level, int *start, int *stop, int *n);
int setPatternWaitAddress(int level, int addr);
uint64_t setPatternWaitTime(int level, uint64_t t);


void initDac(int dacnum);
int setDac(int dacnum,int dacvalue);

/*

u_int32_t setNBits(u_int32_t);
u_int32_t getNBits();
*/

/*
//move to mcb_funcs?

int readOutChan(int *val);
u_int32_t getModuleNumber(int modnum);
int testShiftIn(int imod);
int testShiftOut(int imod);
int testShiftStSel(int imod);
int testDataInOut(int num, int imod);
int testExtPulse(int imod);
int testExtPulseMux(int imod, int ow);
int testDataInOutMux(int imod, int ow, int num);
int testOutMux(int imod);
int testFpgaMux(int imod);
int calibration_sensor(int num, int *values, int *dacs) ;
int calibration_chip(int num, int *values, int *dacs);
*/


#endif
