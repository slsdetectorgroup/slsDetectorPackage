// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/sls_detector_defs.h"
#include "slsDetectorServer_defs.h" // DAC_INDEX, ADC_INDEX, also include RegisterDefs.h

#include "nios.h"
#include "programViaNios.h"

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
int checkType();
int testFpga();
int testBus();

// Ids
void getServerVersion(char *version);
u_int64_t getFirmwareVersion();
u_int64_t getFirmwareAPIVersion();
void getHardwareVersion(char *version);
u_int16_t getHardwareVersionNumber();
int isHardwareVersion_1_0();
u_int32_t getDetectorNumber();
int getModuleId(int *ret, char *mess);
int updateModuleId();
void setModuleId(int modid);
u_int64_t getDetectorMAC();
u_int32_t getDetectorIP();

// initialization
void initControlServer();
void initStopServer();

// set up detector
void setupDetector();
int resetToDefaultDacs(int hardReset, char* mess);
int getDefaultDac(enum DACINDEX index, enum detectorSettings sett, int *retval);
int setDefaultDac(enum DACINDEX index, enum detectorSettings sett, int value);
void setASICDefaults();
int readConfigFile();
int checkCommandLineConfiguration();

// firmware functions (resets)
void cleanFifos();
void resetCore();
void resetPeripheral();

// parameters - dr, roi
int setDynamicRange(int dr);
int getDynamicRange(int *retval);

// parameters - readout
int setParallelMode(int mode);
int getParallelMode();

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
void setNumBursts(int64_t val);
int64_t getNumBursts();
int setBurstPeriod(int64_t val);
int64_t getBurstPeriod();
int64_t getNumFramesLeft();
int64_t getNumTriggersLeft();
int setDelayAfterTrigger(int64_t val);
int64_t getDelayAfterTrigger();
int64_t getDelayAfterTriggerLeft();
int64_t getPeriodLeft();
int64_t getNumBurstsLeft();
int64_t getFramesFromStart();
int64_t getActualTime();
int64_t getMeasurementTime();

// parameters - module, settings
int setSettings(enum detectorSettings sett, char* mess);
enum detectorSettings getSettings();

// parameters - dac, adc, hv
int setOnChipDAC(enum ONCHIP_DACINDEX ind, int chipIndex, int val);
int getOnChipDAC(enum ONCHIP_DACINDEX ind, int chipIndex);

int validateDAC(enum DACINDEX ind, int val, int mV, char* mess);
int setDAC(enum DACINDEX ind, int val, int mV, char* mess);
int getDAC(enum DACINDEX ind, int mV, int* retval, char* mess);


int getADC(enum ADCINDEX ind, int *value);
int setHighVoltage(int val, char* mess);
int getHighVoltage(int *retval, char* mess);

// parameters - timing, extsig
int setMaster(enum MASTERINDEX m);
int isMaster(int *retval);

void updatingRegisters();
int updateClockDivs();
void setTiming(enum timingMode arg);
enum timingMode getTiming();

// configure mac
void setNumberofUDPInterfaces(int val);
int getNumberofUDPInterfaces();
int getNumberofDestinations(int *retval);
int setNumberofDestinations(int value);
int getFirstUDPDestination();
void setFirstUDPDestination(int value);
void calcChecksum(udp_header *udp);
int configureMAC();
int setDetectorPosition(int pos[]);
int *getDetectorPosition();

// very detector specific
int checkDetectorType(char *mess);
int powerChip(int on, char *mess);
int getPowerChip();
int isChipConfigured();
int configureChip(char *mess);
void setDBITPipeline(int val);
int getDBITPipeline();
int setPhase(enum CLKINDEX ind, int val, int degrees);
int getPhase(enum CLKINDEX ind, int degrees);
int getMaxPhase(enum CLKINDEX ind);
int validatePhaseinDegrees(enum CLKINDEX ind, int val, int retval);
// void       	setFrequency(enum CLKINDEX ind, int val);
int getFrequency(enum CLKINDEX ind);
int getVCOFrequency(enum CLKINDEX ind);
int setReadoutSpeed(int val);
int getReadoutSpeed(int *retval);
int getMaxClockDivider();
int setClockDivider(enum CLKINDEX ind, int val);
int getClockDivider(enum CLKINDEX ind);
int setInjectChannel(int offset, int increment);
void getInjectedChannels(int *offset, int *increment);
int setVetoReference(int gainIndex, int value);
int setVetoPhoton(int chipIndex, int *gainIndices, int *values);
int configureASICVetoReference(int chipIndex, int *gainIndices, int *values);
int getVetoPhoton(int chipIndex, int *retvals, int *gainRetvals);
int setADCConfiguration(int chipIndex, int adcIndex, int value);
int getADCConfiguration(int chipIndex, int adcIndex);
int setBurstModeinFPGA(enum burstMode value);
int setBurstMode(enum burstMode burst);
int configureASICGlobalSettings();
enum burstMode getBurstMode();
int setCDSGain(int enable);
int getCDSGain();
int setFilterResistor(int value);
int getFilterResistor();
void setCurrentSource(int value);
int getCurrentSource();
void setTimingSource(enum timingSourceType value);
enum timingSourceType getTimingSource();
void setVeto(int enable);
int getVeto();
void setVetoStream(int value);
int getVetoStream();
enum vetoAlgorithm getVetoAlgorithm(enum streamingInterface interface);
void setVetoAlgorithm(enum vetoAlgorithm alg,
                      enum streamingInterface interface);

int setBadChannels(int numChannels, int *channelList);
int *getBadChannels(int *numChannels);

// aquisition
int startStateMachine();
#ifdef VIRTUAL
void *start_timer(void *arg);
#endif
int stopStateMachine();
enum runStatus getRunStatus();
void waitForAcquisitionEnd();
u_int32_t runBusy();

// common
int calculateDataBytes();
int getTotalNumberOfChannels();
int getNumberOfChips();
int getNumberOfDACs();
int getNumberOfChannelsPerChip();