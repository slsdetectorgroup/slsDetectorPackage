#ifndef SLS_DETECTOR_FUNCTION_LIST
#define SLS_DETECTOR_FUNCTION_LIST

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>
#include <asm/page.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>

/****************************************************
This functions are used by the slsDetectroServer_funcs interface.
Here are the definitions, but the actual implementation should be done for each single detector.

****************************************************/


//if b>0 all the detector must be initialized, otherwise it is just the stop server
int initializeDetector(int b);

/**
   sets number of modules
   \param nm number of modules (-1 gets)
   \param dim dimension
   \returns number of modules

   will probably be changed in set ROI mask
*/
int setNMod(int nm, enum dimension dim);

/**
   returns the maximum number of modules in one dimension
   \param arg dimension
   \returns max number of modules of the baord
*/
int getNModBoard(enum dimension arg);

enum externalSignalFlag getExtSignal(int signalindex);
enum externalSignalFlag setExtSignal(int signalindex,  enum externalSignalFlag flag);
enum externalCommunicationMode setTiming( enum externalCommunicationMode arg);
int64_t getModuleId(enum idMode arg, int imod);
int64_t getDetectorId(enum idMode arg);


int moduleTest( enum digitalTestMode arg, int imod);
int detectorTest( enum digitalTestMode arg);

//write register
int bus_w(int addr,int val);
//read register
int bus_r(int addr);

double setDAC(enum dacIndex ind, double val, int imod);
double getADC(enum dacIndex ind,  int imod);

int setChannel(sls_detector_channel myChan);
int getChannel(sls_detector_channel *myChan);

int setChip(sls_detector_chip myChip);
int getChip(sls_detector_chip *myChip);

int setModule(sls_detector_module myChan);
int getModule(sls_detector_module *myChan);
int getThresholdEnergy(int imod);
int setThresholdEnergy(int thr, int imod);

enum detectorSettings setSettings(enum detectorSettings sett, int imod);

int startAcquisition();
int stopAcquisition();
int startReadOut();


enum runStatus getRunStatus();
char *readFrame(int *ret, char *mess);


int64_t setTimer(enum timerIndex ind, int64_t val);
int64_t getTimeLeft(enum timerIndex ind);
int setDynamicRange(int dr);

int setROI(int mask); //////?????????????????
int getROI(int *mask); //////////?????????????????????


int setSpeed(enum speedVariable arg, int val);
enum readOutFlags setReadOutFlags(enum readOutFlags val);


int executeTrimming(enum trimMode mode, int par1, int par2, int imod);
enum masterFlags setMaster(enum masterFlags arg);
enum synchronizationMode setSynchronization(enum synchronizationMode arg);



int configureMAC(int ipad, long long int imacadd, long long int iservermacadd, int dtb);
int loadImage(enum imageType index, char *imageVals);
int readCounterBlock(int startACQ, char *counterVals);
int resetCounterBlock(int startACQ);

int calculateDataBytes();

int getTotalNumberOfChannels();
int getTotalNumberOfChips();
int getTotalNumberOfModules();
int getNumberOfChannelsPerChip();
int getNumberOfChannelsPerChip();
int getNumberOfChannelsPerModule();
int getNumberOfChipsPerModule();
int getNumberOfDACsPerModule();
int getNumberOfADCsPerModule();


#endif
