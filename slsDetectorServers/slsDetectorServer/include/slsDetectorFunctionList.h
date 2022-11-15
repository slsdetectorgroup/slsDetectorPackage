// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/sls_detector_defs.h"
#include "slsDetectorServer_defs.h" // DAC_INDEX, ADC_INDEX, also include RegisterDefs.h
#ifdef GOTTHARDD
#include "AD9252.h"  // old board compatibility
#include "clogger.h" // runState(enum TLogLevel)
#endif
#if defined(GOTTHARDD) || defined(JUNGFRAUD) || defined(CHIPTESTBOARDD) ||     \
    defined(MOENCHD)
#include "AD9257.h" // commonServerFunctions.h, blackfin.h, ansi.h
#endif

#if defined(MYTHEN3D) || defined(GOTTHARD2D)
#include "programViaNios.h"
#elif defined(CHIPTESTBOARDD) || defined(JUNGFRAUD) || defined(MOENCHD) ||     \
    defined(GOTTHARDD)
#include "programViaBlackfin.h"
#endif

#if defined(MYTHEN3D) || defined(GOTTHARD2D)
#include "nios.h"
#elif defined(GOTTHARDD) || defined(JUNGFRAUD) || defined(CHIPTESTBOARDD) ||   \
    defined(MOENCHD)
#include "blackfin.h"
#endif

#ifdef MYTHEN3D
#include "mythen3.h"
#endif

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
    int srcport;
    int srcport2;
    int dstport;
    int dstport2;
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
#if defined(GOTTHARDD) || defined(JUNGFRAUD) || defined(CHIPTESTBOARDD) ||     \
    defined(MOENCHD) || defined(MYTHEN3D) || defined(GOTTHARD2D)
int checkType();
int testFpga();
int testBus();
#endif

#if defined(GOTTHARDD) ||                                                      \
    ((defined(EIGERD) || defined(JUNGFRAUD)) && defined(VIRTUAL))
void setTestImageMode(int ival);
int getTestImageMode();
#endif

// Ids
u_int64_t getServerVersion();
u_int64_t getClientServerAPIVersion();
u_int64_t getFirmwareVersion();
u_int64_t getFirmwareAPIVersion();
#if defined(JUNGFRAUD) || defined(CHIPTESTBOARDD) || defined(MOENCHD) ||       \
    defined(MYTHEN3D) || defined(GOTTHARD2D)
u_int16_t getHardwareVersionNumber();
#endif
#if defined(JUNGFRAUD) || defined(CHIPTESTBOARDD) || defined(MOENCHD)
u_int16_t getHardwareSerialNumber();
#endif
#ifdef JUNGFRAUD
int isHardwareVersion2();
int getChipVersion();
void setChipVersion(int version);
#endif
#ifndef EIGERD
u_int32_t getDetectorNumber();
#endif
#if defined(GOTTHARD2D) || defined(EIGERD) || defined(MYTHEN3D)
int getModuleId(int *ret, char *mess);
#endif
#if defined(GOTTHARD2D) || defined(MYTHEN3D)
void setModuleId(int modid);
#endif
u_int64_t getDetectorMAC();
u_int32_t getDetectorIP();
#ifdef GOTTHARDD
u_int32_t getBoardRevision();
#endif

// initialization
void initControlServer();
void initStopServer();
#ifdef EIGERD
void getModuleConfiguration();
#endif

// set up detector
#if defined(EIGERD) || defined(MYTHEN3D)
void allocateDetectorStructureMemory();
#endif
void setupDetector();
#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
int updateDatabytesandAllocateRAM();
void updateDataBytes();
#endif

#ifndef CHIPTESTBOARDD
int resetToDefaultDacs(int hardReset);
int getDefaultDac(enum DACINDEX index, enum detectorSettings sett, int *retval);
int setDefaultDac(enum DACINDEX index, enum detectorSettings sett, int value);
#endif
#if defined(MYTHEN3D) || defined(GOTTHARD2D)
void setASICDefaults();
#endif
#ifdef MYTHEN3D
void setADIFDefaults();
#endif
#if defined(GOTTHARD2D) || defined(EIGERD) || defined(JUNGFRAUD)
int readConfigFile();
#endif
#ifdef EIGERD
void resetToHardwareSettings();
#endif

// advanced read/write reg
#ifdef EIGERD
int writeRegister(uint32_t offset, uint32_t data);
int readRegister(uint32_t offset, uint32_t *retval);
#elif GOTTHARDD
uint32_t
writeRegister16And32(uint32_t offset,
                     uint32_t data); // FIXME its not there in ctb or moench?
uint32_t readRegister16And32(uint32_t offset);
#endif

// firmware functions (resets)
#if defined(JUNGFRAUD) || defined(CHIPTESTBOARDD) || defined(MOENCHD) ||       \
    defined(MYTHEN3D) || defined(GOTTHARD2D)
void cleanFifos();
void resetCore();
void resetPeripheral();
#elif GOTTHARDD
void setPhaseShiftOnce();
void setPhaseShift(int numphaseshift);
void cleanFifos();
void setADCSyncRegister();
void setDAQRegister();
void setChipOfInterestRegister(int adc);
void setROIADC(int adc);
void setGbitReadout();
int readConfigFile();
void setMasterSlaveConfiguration();
#endif

// parameters - dr, roi
int setDynamicRange(int dr);
#ifdef GOTTHARDD
int setROI(ROI arg);
ROI getROI();
#endif
#ifdef JUNGFRAUD
void setADCInvertRegister(uint32_t val);
uint32_t getADCInvertRegister();
#endif
#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
int setADCEnableMask(uint32_t mask);
uint32_t getADCEnableMask();
void setADCEnableMask_10G(uint32_t mask);
uint32_t getADCEnableMask_10G();
void setADCInvertRegister(uint32_t val);
uint32_t getADCInvertRegister();
#endif
#if defined(CHIPTESTBOARDD)
int setExternalSamplingSource(int val);
int setExternalSampling(int val);
#endif

// parameters - readout
#if defined(EIGERD) || defined(MYTHEN3D)
int setParallelMode(int mode);
int getParallelMode();
#endif
#ifdef EIGERD
int setOverFlowMode(int mode);
int getOverFlowMode();
#endif
#ifdef CHIPTESTBOARDD
int setReadoutMode(enum readoutMode mode);
int getReadoutMode();
#endif

// parameters - timer
#ifdef JUNGFRAUD
int selectStoragecellStart(int pos);
int getMaxStoragecellStart();
#endif
#if defined(JUNGFRAUD) || defined(EIGERD)
int setNextFrameNumber(uint64_t value);
int getNextFrameNumber(uint64_t *value);
#endif
void setNumFrames(int64_t val);
int64_t getNumFrames();
void setNumTriggers(int64_t val);
int64_t getNumTriggers();
#ifndef MYTHEN3D
int setExpTime(int64_t val);
int64_t getExpTime();
#endif
int setPeriod(int64_t val);
int64_t getPeriod();
#ifdef MYTHEN3D
void setNumIntGates(int val);
void setNumGates(int val);
int getNumGates();
void updateGatePeriod();
int64_t getGatePeriod();
int setExpTime(int gateIndex, int64_t val);
int64_t getExpTime(int gateIndex);
int setGateDelay(int gateIndex, int64_t val);
int64_t getGateDelay(int gateIndex);
#endif
#ifdef GOTTHARD2D
void setNumBursts(int64_t val);
int64_t getNumBursts();
int setBurstPeriod(int64_t val);
int64_t getBurstPeriod();
#endif
#ifdef EIGERD
int setSubExpTime(int64_t val);
int64_t getSubExpTime();
int setSubDeadTime(int64_t val);
int64_t getSubDeadTime();
int64_t getMeasuredPeriod();
int64_t getMeasuredSubPeriod();
#endif
#ifdef JUNGFRAUD
void setNumAdditionalStorageCells(int val);
int getNumAdditionalStorageCells();
int setStorageCellDelay(int64_t val);
int64_t getStorageCellDelay();
#endif
#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
int setNumAnalogSamples(int val);
int getNumAnalogSamples();
#endif
#ifdef CHIPTESTBOARDD
int setNumDigitalSamples(int val);
int getNumDigitalSamples();
#endif
#ifdef MYTHEN3D
void setCounterMask(uint32_t arg);
uint32_t getCounterMask();
void updatePacketizing();
#endif

#if defined(JUNGFRAUD) || defined(GOTTHARDD) || defined(CHIPTESTBOARDD) ||     \
    defined(MOENCHD) || defined(MYTHEN3D) || defined(GOTTHARD2D)
int setDelayAfterTrigger(int64_t val);
int64_t getDelayAfterTrigger();
int64_t getNumFramesLeft();
int64_t getNumTriggersLeft();
int64_t getDelayAfterTriggerLeft();
int64_t getPeriodLeft();
#endif
#ifdef GOTTHARD2D
int64_t getNumBurstsLeft();
#endif
#ifdef GOTTHARDD
int64_t getExpTimeLeft();
#endif
#if defined(JUNGFRAUD) || defined(CHIPTESTBOARDD) || defined(MOENCHD) ||       \
    defined(MYTHEN3D) || defined(GOTTHARD2D)
int64_t getFramesFromStart();
int64_t getActualTime();
int64_t getMeasurementTime();
#endif

// parameters - module, settings
#if (!defined(CHIPTESTBOARDD)) && (!defined(MOENCHD)) && (!defined(GOTTHARD2D))
int setModule(sls_detector_module myMod, char *mess);
#endif
#ifdef MYTHEN3D
int setTrimbits(int *trimbits);
int setAllTrimbits(int val);
int getAllTrimbits();
#endif
#ifndef CHIPTESTBOARDD
enum detectorSettings setSettings(enum detectorSettings sett);
#endif
enum detectorSettings getSettings();
#ifdef JUNGFRAUD
enum gainMode getGainMode();
void setGainMode(enum gainMode mode);
#endif

// parameters - threshold
#ifdef EIGERD
int getThresholdEnergy();
int setThresholdEnergy(int ev);
#endif
#ifdef MYTHEN3D
int getThresholdEnergy(int counterIndex);
void setThresholdEnergy(int counterIndex, int eV);
#endif
// parameters - dac, adc, hv

#ifdef GOTTHARD2D
int setOnChipDAC(enum ONCHIP_DACINDEX ind, int chipIndex, int val);
int getOnChipDAC(enum ONCHIP_DACINDEX ind, int chipIndex);
#endif
void setDAC(enum DACINDEX ind, int val, int mV);
#ifdef MYTHEN3D
void setGeneralDAC(enum DACINDEX ind, int val, int mV);
#endif
int getDAC(enum DACINDEX ind, int mV);
int getMaxDacSteps();
#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
int dacToVoltage(int dac);
int checkVLimitCompliant(int mV);
int checkVLimitDacCompliant(int dac);
int getVLimit();
void setVLimit(int l);
#endif

#ifdef CHIPTESTBOARDD
int isVchipValid(int val);
int getVchip();
void setVchip(int val);
int getVChipToSet(enum DACINDEX ind, int val);
int getDACIndexFromADCIndex(enum ADCINDEX ind);
int getADCIndexFromDACIndex(enum DACINDEX ind);
int isPowerValid(enum DACINDEX ind, int val);
int getPower();
void setPower(enum DACINDEX ind, int val);
void powerOff();
#endif

#if !defined(MOENCHD) && !defined(MYTHEN3D) && !defined(GOTTHARD2D)
int getADC(enum ADCINDEX ind);
#endif

int setHighVoltage(int val);

// parameters - timing, extsig
#if defined(MYTHEN3D) || defined(EIGERD) || defined(GOTTHARDD)
int isMaster();
#endif
#ifdef GOTTHARD2D
void updatingRegisters();
#endif
void setTiming(enum timingMode arg);
enum timingMode getTiming();
#ifdef MYTHEN3D
void setInitialExtSignals();
int setGainCaps(int caps);
int getGainCaps();
int setChipStatusRegister(int csr);
int setDACS(int *dacs);
#endif
#if defined(GOTTHARDD) || defined(MYTHEN3D)
void setExtSignal(int signalIndex, enum externalSignalFlag mode);
int getExtSignal(int signalIndex);
#endif

// configure mac
#ifdef GOTTHARDD
void calcChecksum(mac_conf *mac, int sourceip, int destip);
#endif
#if defined(JUNGFRAUD) || defined(GOTTHARD2D)
void setNumberofUDPInterfaces(int val);
int getNumberofUDPInterfaces();
#endif

#if defined(JUNGFRAUD) || defined(EIGERD)
int getNumberofDestinations(int *retval);
int setNumberofDestinations(int value);
#endif
#ifdef JUNGFRAUD
int getFirstUDPDestination();
void setFirstUDPDestination(int value);
void selectPrimaryInterface(int val);
int getPrimaryInterface();
void setupHeader(int iRxEntry, enum interfaceType type, uint32_t destip,
                 uint64_t destmac, uint32_t destport, uint64_t sourcemac,
                 uint32_t sourceip, uint32_t sourceport);
#endif
#if defined(JUNGFRAUD) || defined(GOTTHARD2D) || defined(MYTHEN3D) ||          \
    defined(CHIPTESTBOARDD) || defined(MOENCHD)
void calcChecksum(udp_header *udp);
#endif
#ifdef GOTTHARDD
int getAdcConfigured();
#endif

int configureMAC();
int setDetectorPosition(int pos[]);
int *getDetectorPosition();

#ifdef EIGERD
int setQuad(int value);
int getQuad();
int setInterruptSubframe(int value);
int getInterruptSubframe();
int setReadNRows(int value);
int getReadNRows();
#endif
#if defined(CHIPTESTBOARDD) || defined(MOENCHD) || defined(EIGERD) ||          \
    defined(MYTHEN3D)
int enableTenGigabitEthernet(int val);
#endif

// very detector specific

// moench specific - powerchip
#ifdef MOENCHD
int powerChip(int on);
int setAnalogOnlyReadout();
#endif

// chip test board or moench specific - configure frequency, phase, pll,
// flashing firmware
#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
int setPhase(enum CLKINDEX ind, int val, int degrees);
int getPhase(enum CLKINDEX ind, int degrees);
int getMaxPhase(enum CLKINDEX ind);
int validatePhaseinDegrees(enum CLKINDEX ind, int val, int retval);
int setFrequency(enum CLKINDEX ind, int val);
int getFrequency(enum CLKINDEX ind);
void configureSyncFrequency(enum CLKINDEX ind);
void setADCPipeline(int val);
int getADCPipeline();
#endif

#ifdef CHIPTESTBOARDD
void setDBITPipeline(int val);
int getDBITPipeline();
int setLEDEnable(int enable);
void setDigitalIODelay(uint64_t pinMask, int delay);
#endif

// jungfrau specific - powerchip, autocompdisable, clockdiv, asictimer, clock,
// pll, flashing firmware
#ifdef JUNGFRAUD
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

// eiger specific - iodelay, pulse, rate, temp, activate, delay nw parameter
#elif EIGERD
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

// gotthard specific - adc phase
#elif GOTTHARDD
int setPhase(enum CLKINDEX ind, int val, int degrees);

#elif MYTHEN3D
int checkDetectorType();
int powerChip(int on);
int setPhase(enum CLKINDEX ind, int val, int degrees);
int getPhase(enum CLKINDEX ind, int degrees);
int getMaxPhase(enum CLKINDEX ind);
int validatePhaseinDegrees(enum CLKINDEX ind, int val, int retval);
// void       	setFrequency(enum CLKINDEX ind, int val);
int getFrequency(enum CLKINDEX ind);
int getVCOFrequency(enum CLKINDEX ind);
int getMaxClockDivider();
int setClockDivider(enum CLKINDEX ind, int val);
int getClockDivider(enum CLKINDEX ind);

#elif GOTTHARD2D
int checkDetectorType();
int powerChip(int on);
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
void setBadChannels(int nch, int *channels);
int *getBadChannels(int *nch);
#endif

#if defined(JUNGFRAUD) || defined(EIGERD)
int getTenGigaFlowControl();
int setTenGigaFlowControl(int value);
#endif
#if defined(JUNGFRAUD) || defined(EIGERD) || defined(MYTHEN3D)
int getTransmissionDelayFrame();
int setTransmissionDelayFrame(int value);
#endif
#ifdef EIGERD
int getTransmissionDelayLeft();
int setTransmissionDelayLeft(int value);
int getTransmissionDelayRight();
int setTransmissionDelayRight(int value);
#endif

// aquisition
int startStateMachine();
#ifdef VIRTUAL
void *start_timer(void *arg);
#endif
int stopStateMachine();
#ifdef MYTHEN3D
int softwareTrigger();
#endif
#ifdef EIGERD
int softwareTrigger(int block);
#endif
#if defined(EIGERD) || defined(MYTHEN3D)
int startReadOut();
#endif
enum runStatus getRunStatus();
void readFrame(int *ret, char *mess);
#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
void readandSendUDPFrames(int *ret, char *mess);
void unsetFifoReadStrobes();
void readSample(int ns);
uint32_t checkDataInFifo();
int checkFifoForEndOfAcquisition();
int readFrameFromFifo();
#endif

#if defined(GOTTHARDD) || defined(JUNGFRAUD) || defined(CHIPTESTBOARDD) ||     \
    defined(MOENCHD) || defined(MYTHEN3D) || defined(GOTTHARD2D)
u_int32_t runBusy();
#endif

#ifdef GOTTHARDD
u_int32_t runState(enum TLogLevel lev);
#endif

// common
#if defined(EIGERD) || defined(MYTHEN3D)
int copyModule(sls_detector_module *destMod, sls_detector_module *srcMod);
#endif
int calculateDataBytes();
int getTotalNumberOfChannels();
#if defined(MOENCHD) || defined(CHIPTESTBOARDD)
void getNumberOfChannels(int *nchanx, int *nchany);
#endif
int getNumberOfChips();
int getNumberOfDACs();
int getNumberOfChannelsPerChip();
