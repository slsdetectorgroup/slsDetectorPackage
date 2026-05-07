// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/sls_detector_defs.h"
#include "slsDetectorServer_defs.h" // DAC_INDEX, ADC_INDEX, also include RegisterDefs.h

#include <stdbool.h>
#include <stdio.h> // FILE
#include <stdlib.h>
#include <sys/types.h>

/****************************************************
This functions are used by the slsDetectroServer_funcs interface.
Here are the definitions, but the actual implementation should be done for each
single detector.

****************************************************/

enum interfaceType { OUTER, INNER };
typedef struct udpStruct_s {
    uint16_t srcport;
    uint16_t srcport2;
    uint16_t dstport;
    uint16_t dstport2;
    uint64_t srcmac;
    uint64_t srcmac2;
    uint64_t dstmac;
    uint64_t dstmac2;
    uint32_t srcip;
    uint32_t srcip2;
    uint32_t dstip;
    uint32_t dstip2;
} udpStruct;
#define MAC_ADDRESS_SIZE 18

// basic tests
int isInitCheckDone();
int getInitResult(char **mess);
void basictests();

#if defined(VIRTUAL)
void setTestImageMode(int ival);
int getTestImageMode();
#endif

// Ids
void getServerVersion(char *version);
u_int64_t getFirmwareVersion();
uint64_t getFrontEndFirmwareVersion(enum fpgaPosition fpgaPosition);
u_int64_t getFirmwareAPIVersion();
void getHardwareVersion(char *version);
int getHardwareVersionNumber();
int getModuleId(int *ret, char *mess);
int updateModuleId();
u_int64_t getDetectorMAC();
u_int32_t getDetectorIP();

// initialization
void initControlServer();
void initStopServer();
int updateModuleConfiguration();
int getModuleConfiguration(int *m, int *t, int *n);
#ifdef VIRTUAL
void checkVirtual9MFlag();
#endif

// set up detector
#if !defined(VIRTUAL)
void setupFebBeb();
#endif
int allocateDetectorStructureMemory();
void setupDetector();

int resetToDefaultDacs(int hardReset, char *mess);
int getDefaultDac(enum DACINDEX index, enum detectorSettings sett, int *retval);
int setDefaultDac(enum DACINDEX index, enum detectorSettings sett, int value);
int readConfigFile();
int checkCommandLineConfiguration();
void resetToHardwareSettings();

// advanced read/write reg
int writeRegister(uint32_t offset, uint32_t data, int validate);
int readRegister(uint32_t offset, uint32_t *retval);
int setBit(const uint32_t addr, const int nBit, int validate);
int clearBit(const uint32_t addr, const int nBit, int validate);
int getBit(const uint32_t addr, const int nBit, int *retval);

// parameters - dr, roi
int setDynamicRange(int dr);
int getDynamicRange(int *retval);

// parameters - readout
int setParallelMode(int mode);
int getParallelMode();
int setOverFlowMode(int mode);
int getOverFlowMode();

// parameters - timer
int setNextFrameNumber(uint64_t value);
int getNextFrameNumber(uint64_t *value);
void setNumFrames(int64_t val);
int64_t getNumFrames();
void setNumTriggers(int64_t val);
int64_t getNumTriggers();
int setExpTime(int64_t val);
int64_t getExpTime();
int setPeriod(int64_t val);
int64_t getPeriod();
int setSubExpTime(int64_t val);
int64_t getSubExpTime();
int setSubDeadTime(int64_t val);
int64_t getSubDeadTime();
int64_t getMeasuredPeriod();
int64_t getMeasuredSubPeriod();

// parameters - module, settings
void getModule(sls_detector_module *myMod);
int setModule(sls_detector_module myMod, char *mess);
int setTrimbits(int *chanregs, char *mess);
int setSettings(enum detectorSettings sett, char *mess);
enum detectorSettings getSettings();

// parameters - threshold
int getThresholdEnergy();
int setThresholdEnergy(int ev);

// parameters - dac, adc, hv
int validateDACIndex(enum DACINDEX ind, char *mess);
int validateDACVoltage(enum DACINDEX ind, int voltage, char *mess);
int convertVoltageToDACValue(enum DACINDEX ind, int voltage, int *retval_dacval,
                             char *mess);
int convertDACValueToVoltage(enum DACINDEX ind, int dacval, int *retval_voltage,
                             char *mess);
int getDAC(enum DACINDEX ind, bool mV, int *retval, char *mess);
/** @param val value can be in mV or dac units */
int setDAC(enum DACINDEX ind, int val, bool mV, char *mess);

int setThresholdDACs(int val, bool mV, char *mess);
int getThresholdDACs(bool mV, int *retval, char *mess);

int getADC(enum ADCINDEX ind);
int setHighVoltage(int val, char *mess);
int getHighVoltage(int *retval, char *mess);

// parameters - timing, extsig
int setMaster(enum MASTERINDEX m);
int setTop(enum TOPINDEX t);
int isTop(int *retval);
int isMaster(int *retval);

void setTiming(enum timingMode arg);
enum timingMode getTiming();

// configure mac
int getNumberofUDPInterfaces();
int getNumberofDestinations(int *retval);
int setNumberofDestinations(int value);
int configureMAC();
int setDetectorPosition(int pos[]);
int *getDetectorPosition();

int setQuad(int value);
int getQuad();
int setInterruptSubframe(int value);
int getInterruptSubframe();
int setReadNRows(int value);
int getReadNRows();
int enableTenGigabitEthernet(int val);

// very detector specific

// eiger specific - iodelay, pulse, rate, temp, activate, delay nw parameter
int setReadoutSpeed(int val);
int getReadoutSpeed(int *retval);
int setIODelay(int val);
int setCounterBit(int val);
int pulsePixel(int n, int x, int y);
int pulsePixelNMove(int n, int x, int y);
int pulseChip(int n);
int updateRateCorrection(char *mess);
int validateAndSetRateCorrection(int64_t tau_ns, char *mess);
int setRateCorrection(int64_t custom_tau_in_nsec);
int getRateCorrectionEnable();
int getDefaultSettingsTau_in_nsec();
void setDefaultSettingsTau_in_nsec(int t);
int64_t getCurrentTau();
void setExternalGating(int enable[]);
int setAllTrimbits(int val);
int getAllTrimbits();
int getBebFPGATemp();
int setActivate(int enable);
int getActivate(int *retval);
int getDataStream(enum portPosition port, int *retval);
int setDataStream(enum portPosition port, int enable);

int getTenGigaFlowControl();
int setTenGigaFlowControl(int value);
int getTransmissionDelayFrame();
int setTransmissionDelayFrame(int value);
int getTransmissionDelayLeft();
int setTransmissionDelayLeft(int value);
int getTransmissionDelayRight();
int setTransmissionDelayRight(int value);

// aquisition
int startStateMachine();
#ifdef VIRTUAL
void *start_timer(void *arg);
#endif
int stopStateMachine();
int softwareTrigger(int block);
int startReadOut();
enum runStatus getRunStatus();
void waitForAcquisitionEnd(int *ret, char *mess);

// common
int calculateDataBytes();
int getTotalNumberOfChannels();
int getNumberOfChips();
int getNumberOfDACs();
int getNumberOfChannelsPerChip();