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
u_int16_t bus_w16(u_int32_t offset, u_int16_t data);
u_int16_t bus_r16(u_int32_t offset);
u_int16_t ram_w16(u_int32_t ramType, int adc, int adcCh, int Ch, u_int16_t data);
u_int16_t ram_r16(u_int32_t ramType, int adc, int adcCh, int Ch);
u_int32_t bus_w(u_int32_t offset, u_int32_t data);
u_int32_t bus_r(u_int32_t offset);

void initializeDetector();
int checkType();
void printVersions();

int testFifos(void);
u_int32_t testFpga(void);
u_int32_t testRAM(void);
int testBus(void);

u_int64_t getDetectorNumber();
u_int64_t getFirmwareVersion();
int64_t getId(enum idMode arg);

void defineGPIOpins();
void resetFPGA();
void FPGAdontTouchFlash();
void FPGATouchFlash();
void eraseFlash();
int startWritingFPGAprogram(FILE** filefp);
int stopWritingFPGAprogram(FILE* filefp);
int writeFPGAProgram(char* fpgasrc, size_t fsize, FILE* filefp);

long int calcChecksum(int sourceip, int destip);
void configureMAC(uint32_t destip,uint64_t destmac,uint64_t  sourcemac,int detipad,int ival,uint32_t destport);

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

int setDynamicRange(int dr);
int getDynamicRange();
int getNModBoard();
int setNMod(int n);
int getNMod();

u_int32_t runBusy(void); 
u_int32_t runState(void); 

int startStateMachine();
int stopStateMachine();
int startReadOut();
enum runStatus getStatus();
void waitForAcquisitionEnd();

void serializeToSPI(int bitsize, u_int32_t val, u_int16_t csmask, int numbitstosend, u_int16_t clkmask, u_int16_t digoutmask, int digofset);
void initDac(int dacnum);
int setDac(int dacnum, int dacvalue);
int setHighVoltage(int val, int imod);
void setAdc(int addr, int val);
void configureAdc();
void prepareADC();

int powerChip (int on);
int setPhaseShiftOnce();
int adcPhase(int st);
int getPhase();

u_int32_t putout(char *s, int modnum);
u_int32_t readin(int modnum);
u_int32_t setClockDivider(int d);
u_int32_t getClockDivider();

void resetPLL();
u_int32_t setPllReconfigReg(u_int32_t reg, u_int32_t val, int trig);
u_int32_t getPllReconfigReg(u_int32_t reg, int trig);
void configurePll(int i);

int getTemperature(int tempSensor,int imod);
int initConfGain(int isettings,int val,int imod);
int initSpeedConfGain(int val);

ROI *setROI(int nroi,ROI* arg,int *retvalsize, int *ret);
int getChannels();

int loadImage(int index, short int ImageVals[]);
int readCounterBlock(int startACQ, short int CounterVals[]);
int resetCounterBlock(int startACQ);
int calibratePedestal(int frames);
uint64_t readPatternWord(int addr);
uint64_t writePatternWord(int addr, uint64_t word);
uint64_t writePatternIOControl(uint64_t word);
uint64_t writePatternClkControl(uint64_t word);
int setPatternLoop(int level, int *start, int *stop, int *n);
int setPatternWaitAddress(int level, int addr);
uint64_t setPatternWaitTime(int level, uint64_t t);

u_int32_t setExtSignal(int d, enum externalSignalFlag  mode);
int  getExtSignal(int d);
u_int32_t setFPGASignal(int d, enum externalSignalFlag  mode);
int  getFPGASignal(int d);
int setTiming(int t);
int setMaster(int f);
int setSynchronization(int s);

#endif
