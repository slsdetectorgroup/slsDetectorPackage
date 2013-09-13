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




int initDetector();

int setNMod(int nm, enum dimension dim);
int getNModBoard(enum dimension arg);

int64_t getModuleId(enum idMode arg, int imod);
int64_t getDetectorId(enum idMode arg);
u_int64_t  getDetectorMAC();
int  getDetectorNumber();

int moduleTest( enum digitalTestMode arg, int imod);
int detectorTest( enum digitalTestMode arg);


int setDAC(enum detDacIndex ind, int val, int imod);
int getADC(enum detDacIndex ind,  int imod);

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

int setModule(sls_detector_module myMod);
int getModule(sls_detector_module *myMod);

enum detectorSettings setSettings(enum detectorSettings sett, int imod);

#if defined(MYTHEND) || defined(EIGERD)
int getThresholdEnergy(int imod);
int setThresholdEnergy(int thr, int imod);
#endif

int startStateMachine();
int stopStateMachine();
int startReadOut();
enum runStatus getRunStatus();
char *readFrame(int *ret, char *mess);


int64_t setTimer(enum timerIndex ind, int64_t val);
int64_t getTimeLeft(enum timerIndex ind);


int setDynamicRange(int dr);
int setROI(int n, ROI arg[], int *retvalsize, int *ret);


#ifdef MYTHEND
enum readOutFlags setReadOutFlags(enum readOutFlags val);
int setSpeed(enum speedVariable arg, int val);
#endif

#if defined(EIGERD) || defined(MYTHEND)
int executeTrimming(enum trimMode mode, int par1, int par2, int imod);
#endif


#ifdef GOTTHARDD
int configureMAC(int ipad, long long int imacadd, long long int iservermacadd, int dtb);
int loadImage(enum imageType index, char *imageVals);
int readCounterBlock(int startACQ, char *counterVals);
int resetCounterBlock(int startACQ);
int startReceiver(int d);
int calibratePedestal(int frames);
#endif


int calculateDataBytes();
int getTotalNumberOfChannels();
int getTotalNumberOfChips();
int getTotalNumberOfModules();
int getNumberOfChannelsPerChip();
int getNumberOfChannelsPerModule();
int getNumberOfChipsPerModule();
int getNumberOfDACsPerModule();
int getNumberOfADCsPerModule();


enum externalSignalFlag getExtSignal(int signalindex);
enum externalSignalFlag setExtSignal(int signalindex,  enum externalSignalFlag flag);
enum externalCommunicationMode setTiming( enum externalCommunicationMode arg);
enum masterFlags setMaster(enum masterFlags arg);
enum synchronizationMode setSynchronization(enum synchronizationMode arg);



#endif



#endif
