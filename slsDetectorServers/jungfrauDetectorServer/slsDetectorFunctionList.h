// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/sls_detector_defs.h"
#include "slsDetectorServer_defs.h" // DAC_INDEX, ADC_INDEX, also include RegisterDefs.h

#include "AD9257.h" // commonServerFunctions.h, blackfin.h, ansi.h
#include "blackfin.h"
#include "programViaBlackfin.h"

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

#if defined(VIRTUAL)
void setTestImageMode(int ival);
int getTestImageMode();
#endif

// Ids
void getServerVersion(char *version);
u_int64_t getFirmwareVersion();
u_int64_t getFirmwareAPIVersion();
void getHardwareVersion(char *version);
u_int16_t getHardwareVersionNumber();
u_int16_t getHardwareSerialNumber();
int isHardwareVersion_1_0();
int getChipVersionInFPGA();
int findChipIndex(enum CHIPINDEX *ind, int val, char *mess);
int setChipVersionFromConfigFile(int val, char* mess);
int setChipIndexFromConfigFile(int val, char* mess);
int validateChipIndex(enum CHIPINDEX ind, char* mess);
int setChipVersionInFPGA(char* mess);
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
int resetToDefaultDacs(int hardReset, char *mess);
int getDefaultDac(enum DACINDEX index, enum detectorSettings sett, int *retval);
int setDefaultDac(enum DACINDEX index, enum detectorSettings sett, int value);
int readConfigFile();

// firmware functions (resets)
void cleanFifos();
void resetCore();
void resetPeripheral();

// parameters - dr, roi
int setDynamicRange(int dr);
int getDynamicRange(int *retval);
void setADCInvertRegister(uint32_t val);
uint32_t getADCInvertRegister();

// parameters - timer
int selectStoragecellStart(int pos);
int getMaxStoragecellStart();
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
void setNumAdditionalStorageCells(int val);
int getNumAdditionalStorageCells();
int setStorageCellDelay(int64_t val);
int64_t getStorageCellDelay();
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
int setModule(sls_detector_module myMod, char *mess);
int setSettings(enum detectorSettings sett, char *mess);
enum detectorSettings getSettings();
enum gainMode getGainMode();
void setGainMode(enum gainMode mode);

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

int getADC(enum ADCINDEX ind);
int setHighVoltage(int val, char *mess);
int getHighVoltage(int *retval, char *mess);

// parameters - timing, extsig
int setMaster(enum MASTERINDEX m);
int isMaster(int *retval);
int getSynchronization();
void setSynchronization(int enable);
void setTiming(enum timingMode arg);
enum timingMode getTiming();

// configure mac
void setNumberofUDPInterfaces(int val);
int getNumberofUDPInterfaces();
int getNumberofDestinations(int *retval);
int setNumberofDestinations(int value);
int getFirstUDPDestination();
void setFirstUDPDestination(int value);
void selectPrimaryInterface(int val);
int getPrimaryInterface();
void setupHeader(int iRxEntry, enum interfaceType type, uint32_t destip,
                 uint64_t destmac, uint16_t destport, uint64_t sourcemac,
                 uint32_t sourceip, uint16_t sourceport);
void calcChecksum(udp_header *udp);

int configureMAC();
int setDetectorPosition(int pos[]);
int *getDetectorPosition();

// very detector specific

int setReadNRows(int value);
int getReadNRows();
void initReadoutConfiguration();
int powerChip(int on);
int isChipConfigured();
void configureChip();
int autoCompDisable(int on);
int setComparatorDisableTime(int64_t val);
int64_t getComparatorDisableTime();
void configureASICTimer();
int setReadoutSpeed(int val);
int getReadoutSpeed(int *retval);
int setPhase(enum CLKINDEX ind, int val, int degrees);
int getPhase(enum CLKINDEX ind, int degrees);
int getMaxPhase(enum CLKINDEX ind);
int validatePhaseinDegrees(enum CLKINDEX ind, int val, int retval);
int setThresholdTemperature(int val);
int setTemperatureControl(int val);
int setTemperatureEvent(int val);
void alignDeserializer();
int getFlipRows();
void setFlipRows(int arg);
int setFilterResistor(int value);
int getFilterResistor();
int getNumberOfFilterCells();
void setNumberOfFilterCells(int iCell);
void disableCurrentSource();
void enableCurrentSource(int fix, uint64_t select, int normal);
int getCurrentSource();
int getFixCurrentSource();
int getNormalCurrentSource();
uint64_t getSelectCurrentSource();
int getPedestalMode();
void getPedestalParameters(uint8_t *frames, uint16_t *loops);
void setPedestalMode(int enable, uint8_t frames, uint16_t loops);
int setTimingInfoDecoder(enum timingInfoDecoder val);
int getTimingInfoDecoder(enum timingInfoDecoder *retval);
int getElectronCollectionMode();
void setElectronCollectionMode(int enable);

int getTenGigaFlowControl();
int setTenGigaFlowControl(int value);
int getTransmissionDelayFrame();
int setTransmissionDelayFrame(int value);

// aquisition
int startStateMachine();
#ifdef VIRTUAL
void *start_timer(void *arg);
#endif
int stopStateMachine();
int softwareTrigger(int block);
enum runStatus getRunStatus();
void waitForAcquisitionEnd();
u_int32_t runBusy();

// common
int calculateDataBytes();
int getTotalNumberOfChannels();
int getNumberOfChips();
int getNumberOfDACs();
int getNumberOfChannelsPerChip();