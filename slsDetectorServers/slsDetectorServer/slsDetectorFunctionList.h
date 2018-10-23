#include "sls_detector_defs.h"
#include "slsDetectorServer_defs.h" // DAC_INDEX, ADC_INDEX, also include RegisterDefs.h

#include <stdlib.h>
#include <stdio.h>					// FILE


/****************************************************
This functions are used by the slsDetectroServer_funcs interface.
Here are the definitions, but the actual implementation should be done for each single detector.

****************************************************/


// basic tests
#if defined(EIGERD) || defined(JUNGFRAUD) || defined(GOTTHARD)
int			isFirmwareCheckDone();
int			getFirmwareCheckResult(char** mess);
#endif

void 		checkFirmwareCompatibility();
#ifdef JUNGFRAUD
int 		checkType();
u_int32_t 	testFpga(void);
int 		testBus(void);
#endif

#ifdef JUNGFRAUD
int 		detectorTest( enum digitalTestMode arg);
#endif

// Ids
int64_t 	getDetectorId(enum idMode arg);
u_int64_t  	getFirmwareVersion();
#ifdef JUNGFRAUD
u_int64_t   getFirmwareAPIVersion();
u_int16_t 	getHardwareVersionNumber();
u_int16_t 	getHardwareSerialNumber();
#endif
#ifdef EIGERD
u_int32_t	getDetectorNumber();
#endif
u_int64_t  	getDetectorMAC();
u_int32_t  	getDetectorIP();


// initialization
void 		initControlServer();
void		initStopServer();
#ifdef EIGERD
void 		getModuleConfiguration();
#endif

// set up detector
void		allocateDetectorStructureMemory();
void 		setupDetector();
#ifdef JUNGFRAUD
int			setDefaultDacs();
#endif


// advanced read/write reg
#ifndef EIGERD
extern u_int32_t	writeRegister(u_int32_t offset, u_int32_t data);	// blackfin.h
extern u_int32_t  	readRegister(u_int32_t offset);						// blackfin.h
#else
uint32_t	writeRegister(uint32_t offset, uint32_t data);
uint32_t  	readRegister(uint32_t offset);
#endif


// firmware functions (resets)
#ifdef JUNGFRAUD
int 		powerChip (int on);
void 		cleanFifos();
void 		resetCore();
void 		resetPeripheral();
int         autoCompDisable(int on);
int 		adcPhase(int st);
int 		getPhase();
void        configureASICTimer();
#endif

// parameters - dr, roi
int 		setDynamicRange(int dr);
#ifdef GOTTHARD
int 		setROI(int n, ROI arg[], int *retvalsize, int *ret);
#endif

// parameters - readout
enum speedVariable 		setSpeed(int val);
#ifdef EIGERD
enum 		readOutFlags setReadOutFlags(enum readOutFlags val);
#endif

// parameters - timer
#ifdef JUNGFRAUD
int         selectStoragecellStart(int pos);
#endif
int64_t 	setTimer(enum timerIndex ind, int64_t val);
int64_t 	getTimeLeft(enum timerIndex ind);


// parameters - module, settings

#ifdef EIGERD
int 		setModule(sls_detector_module myMod, int delay);
#else
int 		setModule(sls_detector_module myMod);
#endif
int 		getModule(sls_detector_module *myMod);
enum 		detectorSettings setSettings(enum detectorSettings sett);
enum 		detectorSettings getSettings();


// parameters - threshold
#ifdef EIGERD
int 		getThresholdEnergy();
int 		setThresholdEnergy(int ev);
#endif

// parameters - dac, adc, hv
#ifdef JUNGFRAUD
void 		serializeToSPI(u_int32_t addr, u_int32_t val, u_int32_t csmask, int numbitstosend, u_int32_t clkmask, u_int32_t digoutmask, int digofset);
void 		initDac(int dacnum);
int         voltageToDac(int value);
int         dacToVoltage(unsigned int digital);
#endif

#ifdef JUNGFRAUD
extern void	setAdc(int addr, int val);		// AD9257.h
#endif

void 		setDAC(enum DACINDEX ind, int val, int mV, int retval[]);
int 		getADC(enum ADCINDEX ind);

int 		setHighVoltage(int val);



// parameters - timing, extsig
void 		setTiming( enum externalCommunicationMode arg);
enum 		externalCommunicationMode getTiming();

// configure mac
#ifdef JUNGFRAUD
long int 	calcChecksum(int sourceip, int destip);
#endif
#ifdef GOTTHARDD
int 		configureMAC(uint32_t destip, uint64_t destmac, uint64_t sourcemac, uint32_t sourceip, uint32_t udpport, uint32_t udpport2, int ival);
#else
int 		configureMAC(uint32_t destip, uint64_t destmac, uint64_t sourcemac, uint32_t sourceip, uint32_t udpport, uint32_t udpport2);
#endif
#if defined(JUNGFRAUD) || defined(EIGERD)
int 		setDetectorPosition(int pos[]);
#endif


// very detector specific

// gotthard specific - image, pedestal
#ifdef GOTTHARDD
int 		loadImage(enum imageType index, char *imageVals);
int 		readCounterBlock(int startACQ, char *counterVals);
int			resetCounterBlock(int startACQ);
int 		calibratePedestal(int frames);

// jungfrau specific - pll, flashing firmware
#elif JUNGFRAUD
void 		resetPLL();
u_int32_t 	setPllReconfigReg(u_int32_t reg, u_int32_t val);
void 		configurePll();
int         setThresholdTemperature(int val);
int         setTemperatureControl(int val);
int         setTemperatureEvent(int val);
extern void eraseFlash();													// programfpga.h
extern int 	startWritingFPGAprogram(FILE** filefp);							// programfpga.h
extern void stopWritingFPGAprogram(FILE* filefp);							// programfpga.h
extern int 	writeFPGAProgram(char* fpgasrc, size_t fsize, FILE* filefp);	// programfpga.h

// eiger specific - iodelay, 10g, pulse, rate, temp, activate, delay nw parameter
#elif EIGERD
int 		setIODelay(int val);
int 		enableTenGigabitEthernet(int val);
int 		setCounterBit(int val);
int 		pulsePixel(int n, int x, int y);
int 		pulsePixelNMove(int n, int x, int y);
int 		pulseChip(int n);
int64_t 	setRateCorrection(int64_t custom_tau_in_nsec);
int 		getRateCorrectionEnable();
int 		getDefaultSettingsTau_in_nsec();
void 		setDefaultSettingsTau_in_nsec(int t);
int64_t 	getCurrentTau();
void 		setExternalGating(int enable[]);
int 		setAllTrimbits(int val);
int 		getAllTrimbits();
int 		getBebFPGATemp();
int 		activate(int enable);
#endif
#if defined(JUNGFRAUD) || defined(EIGERD)
int         setNetworkParameter(enum NETWORKINDEX mode, int value);
#endif




// aquisition
#if defined(EIGERD) || defined(GOTTHARD)
int 		prepareAcquisition();
#endif
int 		startStateMachine();
#ifdef VIRTUAL
void* start_timer(void* arg);
#endif
int 		stopStateMachine();
#ifdef EIGERD
int			softwareTrigger();
#endif
#ifndef JUNGFRAUD
int 		startReadOut();
#endif
enum 		runStatus getRunStatus();
void 		readFrame(int *ret, char *mess);
#ifdef JUNGFRAUD
u_int32_t 	runBusy(void);
#endif


//common
int 		copyModule(sls_detector_module *destMod, sls_detector_module *srcMod);
int 		calculateDataBytes();
int 		getTotalNumberOfChannels();
int 		getNumberOfChips();
int 		getNumberOfDACs();
int 		getNumberOfADCs();
#ifdef EIGERD
int 		getNumberOfGains();
int 		getNumberOfOffsets();
#endif
int 		getNumberOfChannelsPerChip();


