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

// Ids
void getServerVersion(char *version);
u_int64_t getFirmwareVersion();
u_int64_t getFirmwareAPIVersion();
void getHardwareVersion(char *version);
u_int16_t getHardwareVersionNumber();
u_int16_t getHardwareSerialNumber();
int isHardwareVersion_1_0();
u_int32_t getDetectorNumber();
u_int64_t getDetectorMAC();
u_int32_t getDetectorIP();
int enableBlackfinAMCExternalAccessExtension(char *mess);

// initialization
void initControlServer();
void initStopServer();

// set up detector
void setupDetector();
int updateDatabytesandAllocateRAM();
void updateDataBytes();

// firmware functions (resets)
void cleanFifos();
void resetCore();
void resetPeripheral();

// parameters - dr, roi
int setDynamicRange(int dr);
int getDynamicRange(int *retval);

int setADCEnableMask(uint32_t mask);
uint32_t getADCEnableMask();
void setADCEnableMask_10G(uint32_t mask);
uint32_t getADCEnableMask_10G();
int setTransceiverEnableMask(uint32_t mask);
uint32_t getTransceiverEnableMask();
void setADCInvertRegister(uint32_t val);
uint32_t getADCInvertRegister();

int setExternalSamplingSource(int val);
int setExternalSampling(int val);

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
enum detectorSettings getSettings();

// parameters - threshold
// parameters - dac, adc, hv
int getVLimit();
int setVLimit(int val, char *mess);

int validateDACIndex(enum DACINDEX ind, char *mess);
int validateDACVoltage(enum DACINDEX ind, int voltage, char *mess);
int convertVoltageToDAC(enum DACINDEX ind, int voltage, int *retval_dacval,
                        char *mess);
int convertDACToVoltage(enum DACINDEX ind, int dacval, int *retval_voltage,
                        char *mess);
int getDAC(enum DACINDEX ind, bool mV, int *retval, char *mess);
/** @param val value can be in mV or dac units */
int setDAC(enum DACINDEX ind, int val, bool mV, char *mess);

int setADCVpp(int val, int mV, char *mess);
int getADCVpp(int mV, int *retval, char *mess);

int validatePowerDACIndex(enum powerIndex ind, char *mess);
int validatePower(enum powerIndex ind, int val, char *mess);
int convertVoltageToPowerDAC(enum powerIndex ind, int voltage,
                             int *retval_dacval, char *mess);
int convertPowerDACToVoltage(enum powerIndex ind, int dacval,
                             int *retval_voltage, char *mess);
int getPowerDAC(enum powerIndex ind, int *retval, char *mess);
int setPowerDAC(enum powerIndex ind, int voltage, char *mess);
int getDACIndexForPower(enum powerIndex pind, enum DACINDEX *dacIndex,
                        char *mess);

int getPowerMask(enum powerIndex ind, uint32_t *mask, char *mess);
void powerOff();
int setPowerEnabled(enum powerIndex indices[], int count, bool enable,
                    char *mess);
int isPowerEnabled(enum powerIndex ind, bool *retval, char *mess);

int validateVchip(int val, char *mess);
int getVchip(int *retval, char *mess);
int setVchip(int val, char *mess);
int getAllPowerValues(bool *pwrEnable, int *pwrValues, char *mess);
/** pwrEnable and pwrValues are current values
 * updated with the current command
 * (current cmd: eg. power enable all or only set one power dac) */
int computeVchip(int *retval_vchip, bool *pwrEnable, int *pwrValues,
                 char *mess);

int getPowerADC(enum powerIndex index, int *retval, char *mess);

int getADC(enum ADCINDEX ind);
int getSlowADC(int ichan);
int getSlowADCTemperature();
int setHighVoltage(int val, char *mess);
int getHighVoltage(int *retval, char *mess);

// parameters - timing, extsig

void setTiming(enum timingMode arg);
enum timingMode getTiming();

// configure mac
int getNumberofUDPInterfaces();
void calcChecksum(udp_header *udp);

int configureMAC();
int setDetectorPosition(int pos[]);
int *getDetectorPosition();

int enableTenGigabitEthernet(int val);

// very detector specific

// chip test board specific - configure frequency, phase, pll,
// flashing firmware
int setPhase(enum CLKINDEX ind, int val, int degrees);
int getPhase(enum CLKINDEX ind, int degrees);
int getMaxPhase(enum CLKINDEX ind);
int validatePhaseinDegrees(enum CLKINDEX ind, int val, int retval);
void configureSyncFrequency(enum CLKINDEX ind);
void setADCPipeline(int val);
int getADCPipeline();
void setDBITPipeline(int val);
int getDBITPipeline();
int setLEDEnable(int enable);
void setDigitalIODelay(uint64_t pinMask, int delay);

int setFrequency(enum CLKINDEX ind, int val);
int getFrequency(enum CLKINDEX ind);

// aquisition
int startStateMachine();
#ifdef VIRTUAL
void *start_timer(void *arg);
#endif
int stopStateMachine();
int startReadOut();
enum runStatus getRunStatus();
void waitForAcquisitionEnd();
int validateUDPSocket();
void readandSendUDPFrames();
void unsetFifoReadStrobes();
int readSample(int ns);
uint32_t checkDataInFifo();
int checkFifoForEndOfAcquisition();
int readFrameFromFifo();
u_int32_t runBusy();

// common
int calculateDataBytes();
int getTotalNumberOfChannels();
void getNumberOfChannels(int *nchanx, int *nchany);
int getNumberOfChips();
int getNumberOfDACs();
int getNumberOfChannelsPerChip();