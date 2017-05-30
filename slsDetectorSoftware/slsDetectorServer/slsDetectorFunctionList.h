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


// basic tests
void 		checkFirmwareCompatibility();
#ifdef JUNGFRAUD
int 		checkType();
u_int32_t 	testFpga(void);
int 		testBus(void);
#endif
int 		moduleTest( enum digitalTestMode arg, int imod);
int 		detectorTest( enum digitalTestMode arg);


// Ids
int64_t 	getDetectorId(enum idMode arg);
u_int64_t  	getFirmwareVersion();
#ifdef MYTHEND
int64_t 	getModuleId(enum idMode arg, int imod);
#elif JUNGFRAUD
u_int16_t 	getHardwareVersionNumber();
u_int16_t 	getHardwareSerialNumber();
#endif
u_int32_t	getDetectorNumber();
u_int64_t  	getDetectorMAC();
u_int32_t  	getDetectorIP();


// initialization
void 		initControlServer();
void		initStopServer();
#ifdef EIGERD
void 		getModuleConfiguration();
#endif
#ifdef JUNGFRAUD
int 		mapCSP0(void);
void 		bus_w16(u_int32_t offset, u_int16_t data);
u_int16_t 	bus_r16(u_int32_t offset);
void 		bus_w(u_int32_t offset, u_int32_t data);
u_int32_t 	bus_r(u_int32_t offset);
int64_t 	set64BitReg(int64_t value, int aLSB, int aMSB);
int64_t 	get64BitReg(int aLSB, int aMSB);
void 		defineGPIOpins();
void 		resetFPGA();
void 		FPGAdontTouchFlash();
void 		FPGATouchFlash();
#endif

// set up detector
void		allocateDetectorStructureMemory();
void 		setupDetector();


// advanced read/write reg
#ifndef EIGERD
u_int32_t 	writeRegister(u_int32_t offset, u_int32_t data);
u_int32_t 	readRegister(u_int32_t offset);
#endif


// firmware functions (resets)
#ifdef JUNGFRAUD
int 		powerChip (int on);
void 		cleanFifos();
void 		resetCore();
void 		resetPeripheral();
int 		adcPhase(int st);
int 		getPhase();
#endif

// parameters - nmod, dr, roi
int setNMod(int nm, enum dimension dim);	// mythen specific, but for detector compatibility as a get
int getNModBoard(enum dimension arg);		// mythen specific, but for detector compatibility as a get
int setDynamicRange(int dr);
#ifdef MYTHEND
int setROI(int n, ROI arg[], int *retvalsize, int *ret);
#endif

// parameters - readout
int setSpeed(enum speedVariable arg, int val);
#if defined(EIGERD) || defined(MYTHEND)
enum readOutFlags setReadOutFlags(enum readOutFlags val);
int executeTrimming(enum trimMode mode, int par1, int par2, int imod);
#endif

// parameters - timer
int64_t setTimer(enum timerIndex ind, int64_t val);
int64_t getTimeLeft(enum timerIndex ind);

// parameters - channel, chip, module, settings
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
// parameters - threshold
int getThresholdEnergy(int imod);
int setThresholdEnergy(int ev, int imod);
#endif

// parameters - dac, adc, hv
void setDAC(enum detDacIndex ind, int val, int imod, int mV, int retval[]);
int getADC(enum detAdcIndex ind,  int imod);
#ifndef MYTHEND
int setHighVoltage(int val);
#endif

// parameters - timing, extsig
#ifdef MYTHEND
enum externalSignalFlag getExtSignal(int signalindex);
enum externalSignalFlag setExtSignal(int signalindex,  enum externalSignalFlag flag);
#endif
enum externalCommunicationMode setTiming( enum externalCommunicationMode arg);

// configure mac
#ifdef JUNGFRAUD
long int calcChecksum(int sourceip, int destip);
#endif
#ifndef MYTHEND
int configureMAC(uint32_t destip, uint64_t destmac, uint64_t sourcemac, uint32_t sourceip, uint32_t udpport, uint32_t udpport2, int ival);
#endif


// very detector specific
#ifdef GOTTHARDD
// gotthard specific - image, pedestal
int loadImage(enum imageType index, char *imageVals);
int readCounterBlock(int startACQ, char *counterVals);
int resetCounterBlock(int startACQ);
int calibratePedestal(int frames);

#elif JUNGFRAUD
// jungfrau specific - flashing fpga
void 		eraseFlash();
int 		startWritingFPGAprogram(FILE** filefp);
int 		stopWritingFPGAprogram(FILE* filefp);
int 		writeFPGAProgram(char* fpgasrc, size_t fsize, FILE* filefp);

#elif EIGERD
//eiger specific - iodelay, 10g, pulse, rate, temp, activate, delay nw parameter
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
void setExternalGating(int enable[]);
int setAllTrimbits(int val);
int getAllTrimbits();
int getBebFPGATemp();
int activate(int enable);
int setNetworkParameter(enum detNetworkParameter mode, int value);
#endif




// aquisition
#ifdef defined(EIGERD) || defined(GOTTHARD)
int startReceiver(int d);
#endif
int startStateMachine();
int stopStateMachine();
int startReadOut();
enum runStatus getRunStatus();
void readFrame(int *ret, char *mess);


//common
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

// sync
enum masterFlags setMaster(enum masterFlags arg);
enum synchronizationMode setSynchronization(enum synchronizationMode arg);



#endif
#endif
