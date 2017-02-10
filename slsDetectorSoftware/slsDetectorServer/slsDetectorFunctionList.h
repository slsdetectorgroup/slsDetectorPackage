#ifdef SLS_DETECTOR_FUNCTION_LIST

#ifndef SLS_DETECTOR_FUNCTION_LIST_H
#define SLS_DETECTOR_FUNCTION_LIST_H

#include "sls_detector_defs.h"
#include "slsDetectorServer_defs.h"

#include <stdlib.h>




/****************************************************
This functions are used by the slsDetectroServer_funcs interface.
Here are the definitions, but the actual implementation should be done for each single detector.

****************************************************/



void getModuleConfiguration();
int initDetector();
int initDetectorStop();

int setNMod(int nm, enum dimension dim);
int getNModBoard(enum dimension arg);

int64_t getModuleId(enum idMode arg, int imod);
int64_t getDetectorId(enum idMode arg);
int  getDetectorNumber();
u_int64_t  getDetectorMAC();
int  getDetectorIP();

int moduleTest( enum digitalTestMode arg, int imod);
int detectorTest( enum digitalTestMode arg);


void setDAC(enum detDacIndex ind, int val, int imod, int mV, int retval[]);
int getADC(enum detAdcIndex ind,  int imod);


#if defined(EIGERD) || defined(GOTTHARD)
int setHighVoltage(int val, int imod);
#endif

#ifdef EIGERD
int setIODelay(int val, int imod);
int enableTenGigabitEthernet(int val);
int setCounterBit(int val);
int pulsePixel(int n, int x, int y);
int pulsePixelNMove(int n, int x, int y);
int pulseChip(int n);
int64_t setRateCorrection(int64_t custom_tau_in_nsec);
int getRateCorrectionEnable();
int getDefaultSettingsTau_in_nsec();
void setDefaultSettingsTau_in_nsec(int t);
int64_t getCurrentTau();
#endif

#if defined(MYTHEND) || defined(GOTTHARDD)
u_int32_t writeRegister(u_int32_t offset, u_int32_t data);
u_int32_t readRegister(u_int32_t offset);
#endif

#ifdef MYTHEND
int setChannel(sls_detector_channel myChan);
int getChannel(sls_detector_channel *myChan);
int setChip(sls_detector_chip myChip);
int getChip(sls_detector_chip *myChip);
#endif


#ifdef EIGERD
int setModule(sls_detector_module myMod, int delay);
#else
int setModule(sls_detector_module myMod);
#endif

int getModule(sls_detector_module *myMod);
enum detectorSettings setSettings(enum detectorSettings sett, int imod);
enum detectorSettings getSettings();

#if defined(MYTHEND) || defined(EIGERD)
int getThresholdEnergy(int imod);
int setThresholdEnergy(int ev, int imod);
#endif

int startStateMachine();
int stopStateMachine();
int startReadOut();
enum runStatus getRunStatus();
void readFrame(int *ret, char *mess);


int64_t setTimer(enum timerIndex ind, int64_t val);
int64_t getTimeLeft(enum timerIndex ind);


int setDynamicRange(int dr);
int setROI(int n, ROI arg[], int *retvalsize, int *ret);


#if defined(EIGERD) || defined(MYTHEND)
enum readOutFlags setReadOutFlags(enum readOutFlags val);
int setSpeed(enum speedVariable arg, int val);
int executeTrimming(enum trimMode mode, int par1, int par2, int imod);
#endif


#ifndef MYTHEND
int configureMAC(int ipad, long long int macad, long long int detectormacadd, int detipad, int udpport, int udpport2, int ival);
#endif

#ifdef GOTTHARDD
int loadImage(enum imageType index, char *imageVals);
int readCounterBlock(int startACQ, char *counterVals);
int resetCounterBlock(int startACQ);
int startReceiver(int d);
int calibratePedestal(int frames);
#endif




int copyModule(sls_detector_module *destMod, sls_detector_module *srcMod);

int calculateDataBytes();
int getTotalNumberOfChannels();
int getTotalNumberOfChips();
int getTotalNumberOfModules();
int getNumberOfChannelsPerChip();
int getNumberOfChannelsPerModule();
int getNumberOfChipsPerModule();
int getNumberOfDACsPerModule();
int getNumberOfADCsPerModule();
#ifdef EIGERD
int getNumberOfGainsPerModule();
int getNumberOfOffsetsPerModule();
#endif

enum externalSignalFlag getExtSignal(int signalindex);
enum externalSignalFlag setExtSignal(int signalindex,  enum externalSignalFlag flag);
enum externalCommunicationMode setTiming( enum externalCommunicationMode arg);
enum masterFlags setMaster(enum masterFlags arg);
enum synchronizationMode setSynchronization(enum synchronizationMode arg);

#ifdef EIGERD
int startReceiver(int d);
void setExternalGating(int enable[]);
int setAllTrimbits(int val);
int getAllTrimbits();
int getBebFPGATemp();
int activate(int enable);
int setNetworkParameter(enum detNetworkParameter mode, int value);
#endif


#endif



#endif
