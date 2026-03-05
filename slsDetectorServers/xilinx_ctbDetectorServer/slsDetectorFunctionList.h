// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/sls_detector_defs.h"
#include "slsDetectorServer_defs.h" // DAC_INDEX, ADC_INDEX, also include RegisterDefs.h

#include "arm64.h"
#include "programViaArm.h"

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
int testFixedFPGAPattern();

// Ids
void getServerVersion(char *version);
u_int64_t getFirmwareVersion();
u_int64_t getFirmwareAPIVersion();
void getHardwareVersion(char *version);
u_int32_t getDetectorNumber();
u_int64_t getDetectorMAC();
u_int32_t getDetectorIP();

// initialization
void initControlServer();
void initStopServer();

// set up detector
void setupDetector();

// firmware functions (resets)
void cleanFifos();
void resetFlow();
int waitTransceiverReset(char *mess);
#ifdef VIRTUAL
void setTransceiverAlignment(int align);
#endif
int isTransceiverAligned();
int waitTransceiverAligned(char *mess);
int configureTransceiver(char *mess);
int isChipConfigured();
int powerChip(int on, char *mess);
int getPowerChip();
int configureChip(char *mess);
int readConfigFile(char *mess, char *fileName, char *fileType);
int resetChip(char *mess);

// parameters - dr, roi
int setDynamicRange(int dr);
int getDynamicRange(int *retval);
void setADCEnableMask_10G(uint32_t mask);
uint32_t getADCEnableMask_10G();
int setTransceiverEnableMask(uint32_t mask);
uint32_t getTransceiverEnableMask();

// parameters - readout
int setReadoutMode(enum readoutMode mode);
int getReadoutMode();

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
int setNumAnalogSamples(int val);
int getNumAnalogSamples();
int setNumDigitalSamples(int val);
int getNumDigitalSamples();
int setNumTransceiverSamples(int val);
int getNumTransceiverSamples();

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

int getVLimit();
int setVLimit(int val, char *mess);

int validatePower(enum PWRINDEX ind, int val, char *mess);
int getPowerIndexFromDACIndex(enum DACINDEX ind, enum PWRINDEX *pwrIndex,
                              char *mess);
int getPowerRailMask(enum PWRINDEX ind, uint32_t *mask, char *mess);
int EnablePowerRail(enum PWRINDEX ind, char *mess);
int DisablePowerRail(enum PWRINDEX ind, char *mess);
int getPowerRail(enum PWRINDEX ind, int *retval, char *mess);
int getPower(enum DACINDEX ind, int *retval, char *mess);
int setPower(enum DACINDEX ind, int val, char *mess);

int getADC(enum ADCINDEX ind, int *value, char *mess);
int getSlowADC(int ichan, int *retval, char *mess);
int getTemperature(int *retval, char *mess);
int setHighVoltage(int val);

// parameters - timing, extsig
void setTiming(enum timingMode arg);
enum timingMode getTiming();

// configure mac
int getNumberofUDPInterfaces();
void calcChecksum(udp_header *udp);
int configureMAC();
int setDetectorPosition(int pos[]);
int *getDetectorPosition();

// very detector specific

// chip test board specific - configure frequency, phase, pll,
// flashing firmware
int setFrequency(enum CLKINDEX ind, int val);
int getFrequency(enum CLKINDEX ind);

// aquisition
int startStateMachine();
#ifdef VIRTUAL
void *start_timer(void *arg);
#endif
int stopStateMachine();
int softwareTrigger();
enum runStatus getRunStatus();
void waitForAcquisitionEnd();
u_int32_t runBusy();

// common
int calculateDataBytes();
int getTotalNumberOfChannels();
void getNumberOfChannels(int *nchanx, int *nchany);
int getNumberOfChips();
int getNumberOfDACs();
int getNumberOfChannelsPerChip();
