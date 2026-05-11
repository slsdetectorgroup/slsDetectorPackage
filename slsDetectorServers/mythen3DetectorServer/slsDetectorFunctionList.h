// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/sls_detector_defs.h"
#include "slsDetectorServer_defs.h" // DAC_INDEX, ADC_INDEX, also include RegisterDefs.h

#include "mythen3.h"
#include "nios.h"
#include "programViaNios.h"

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
int checkType();
int testFpga();
int testBus();

void setTestImageMode(int ival);
int getTestImageMode();

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
int allocateDetectorStructureMemory();
void setupDetector();
int resetToDefaultDacs(int hardReset, char *mess);
int getDefaultDac(enum DACINDEX index, enum detectorSettings sett, int *retval);
int setDefaultDac(enum DACINDEX index, enum detectorSettings sett, int value);
void setASICDefaults();
void setADIFDefaults();
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
void setNumFrames(int64_t val);
int64_t getNumFrames();
void setNumTriggers(int64_t val);
int64_t getNumTriggers();
int setPeriod(int64_t val);
int64_t getPeriod();
void setNumIntGates(int val);
void setNumGates(int val);
int getNumGates();
void updateGatePeriod();
int64_t getGatePeriod();
int setExpTime(int gateIndex, int64_t val);
int64_t getExpTime(int gateIndex);
int setGateDelay(int gateIndex, int64_t val);
int64_t getGateDelay(int gateIndex);

int updateVthAndCounterMask(char *mess);
int setCounterMask(uint32_t arg, char *mess);
int setCounterMaskAndTimeRegisters(uint32_t arg, char *mess);
uint32_t getCounterMask();
void updatePacketizing();

int64_t getNumFramesLeft();
int64_t getNumTriggersLeft();
int setDelayAfterTrigger(int64_t val);
int64_t getDelayAfterTrigger();
int64_t getDelayAfterTriggerLeft();
int64_t getPeriodLeft();
int64_t getFramesFromStart();
int64_t getActualTime();
int64_t getMeasurementTime();

// parameters - module, settings
void getModule(sls_detector_module *myMod);
int setModule(sls_detector_module myMod, char *mess);
int setTrimbits(int *trimbits);
int setAllTrimbits(int val);
int getAllTrimbits();
int setSettings(enum detectorSettings sett, char *mess);
enum detectorSettings getSettings();

// parameters - threshold
int getThresholdEnergy(int counterIndex);
void setThresholdEnergy(int counterIndex, int eV);

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

int getCounterIndexFromDacIndex(enum DACINDEX ind, int *retval_counterIndex,
                                char *mess);
int setSingleThresholdDAC(enum DACINDEX ind, int val, bool mV, int dacval,
                          bool counterCheck, char *mess);
int setThresholdDACs(int val, bool mV, char *mess);
int getThresholdDACs(bool mV, int *retval, char *mess);

/** If 1 */
int updateValueForVthDac(enum DACINDEX index, int *dacval, char *mess);
int rememberValueIfVthDac(enum DACINDEX index, int val, bool mV, char *mess);
int setVthEnabled(enum DACINDEX index, bool enable, char *mess);

int getADC(enum ADCINDEX ind, int *value);
int setHighVoltage(int val, char *mess);
int getHighVoltage(int *retval, char *mess);

// parameters - timing, extsig
int isMaster(int *retval);
void setTiming(enum timingMode arg);
enum timingMode getTiming();
void setInitialExtSignals();
int setChipStatusRegister(int csr);
int setGainCaps(int caps);
int setInterpolation(bool enable, char *mess);
int setPumpProbe(bool enable, char *mess);
int setDigitalPulsing(int enable);
int setAnalogPulsing(int enable);
int setNegativePolarity(int enable);
void setExtSignal(int signalIndex, enum externalSignalFlag mode);
int getExtSignal(int signalIndex);

// configure mac
int getNumberofUDPInterfaces();
int getNumberofDestinations(int *retval);
int setNumberofDestinations(int value);
int getFirstUDPDestination();
void setFirstUDPDestination(int value);
void calcChecksum(udp_header *udp);
int configureMAC();
int setDetectorPosition(int pos[]);
int *getDetectorPosition();
int enableTenGigabitEthernet(int val);

// very detector specific
int checkDetectorType(char *mess);
int powerChip(int on);
int setPhase(enum CLKINDEX ind, int val, int degrees);
int getPhase(enum CLKINDEX ind, int degrees);
int getMaxPhase(enum CLKINDEX ind);
int validatePhaseinDegrees(enum CLKINDEX ind, int val, int retval);
int getFrequency(enum CLKINDEX ind);
int getVCOFrequency(enum CLKINDEX ind);
int getMaxClockDivider();
int setClockDivider(enum CLKINDEX ind, int val);
int setClockDividerWithTimeUpdateOption(enum CLKINDEX ind, int val,
                                        int timeUpdate);
int getClockDivider(enum CLKINDEX ind);
int setReadoutSpeed(int val);
int getReadoutSpeed(int *retval);
int setBadChannels(int numChannels, int *channelList);
int *getBadChannels(int *numChannels);

int getTransmissionDelayFrame();
int setTransmissionDelayFrame(int value);

// aquisition
int startStateMachine();
#ifdef VIRTUAL
void *start_timer(void *arg);
#endif
int stopStateMachine();
int softwareTrigger();
int startReadOut();
enum runStatus getRunStatus();
void waitForAcquisitionEnd();
u_int32_t runBusy();

// common
int calculateDataBytes();
int getTotalNumberOfChannels();
int getNumberOfChips();
int getNumberOfDACs();
int getNumberOfChannelsPerChip();