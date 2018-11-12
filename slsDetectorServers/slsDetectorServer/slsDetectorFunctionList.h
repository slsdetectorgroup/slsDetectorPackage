#include "sls_detector_defs.h"
#include "slsDetectorServer_defs.h" // DAC_INDEX, ADC_INDEX, also include RegisterDefs.h
#ifdef GOTTHARDD
#include "logger.h"                 // runState(enum TLogLevel)
#endif
#include <stdlib.h>
#include <stdio.h>					// FILE


/****************************************************
This functions are used by the slsDetectroServer_funcs interface.
Here are the definitions, but the actual implementation should be done for each single detector.

****************************************************/


// basic tests
#if defined(EIGERD) || defined(JUNGFRAUD) || defined(GOTTHARDD)
int			isFirmwareCheckDone();
int			getFirmwareCheckResult(char** mess);
#endif
void 		basictests();
#if defined(GOTTHARDD) || defined(JUNGFRAUD)
int 		checkType();
u_int32_t 	testFpga(void);
int 		testBus(void);
#endif

#ifdef GOTTHARDD
int         detectorTest(enum digitalTestMode arg,  int ival);
#elif JUNGFRAUD
int 		detectorTest(enum digitalTestMode arg);
#endif

// Ids
int64_t 	getDetectorId(enum idMode arg);
u_int64_t  	getFirmwareVersion();
#ifdef JUNGFRAUD
u_int64_t   getFirmwareAPIVersion();
u_int16_t 	getHardwareVersionNumber();
u_int16_t 	getHardwareSerialNumber();
#endif
u_int32_t	getDetectorNumber();
u_int64_t  	getDetectorMAC();
u_int32_t  	getDetectorIP();
#ifdef GOTTHARDD
u_int32_t   getBoardRevision();
#endif


// initialization
void 		initControlServer();
void		initStopServer();
#ifdef EIGERD
void 		getModuleConfiguration();
#endif

// set up detector
#ifdef EIGERD
void		allocateDetectorStructureMemory();
#endif
void 		setupDetector();
#if defined(GOTTHARDD) || defined(JUNGFRAUD)
int			setDefaultDacs();
#endif


// advanced read/write reg
#ifdef JUNGFRAUD
extern u_int32_t	writeRegister(u_int32_t offset, u_int32_t data);	// blackfin.h
extern u_int32_t  	readRegister(u_int32_t offset);						// blackfin.h
#elif EIGERD
uint32_t	writeRegister(uint32_t offset, uint32_t data);
uint32_t  	readRegister(uint32_t offset);
#else
uint32_t    writeRegister16And32(uint32_t offset, uint32_t data);
uint32_t    readRegister16And32(uint32_t offset);
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
#elif GOTTHARDD
void        setPhaseShiftOnce();
void        setPhaseShift(int numphaseshift);
void        configureADC();
void        cleanFifos();
void        setADCSyncRegister();
void        setDAQRegister();
void        setChipOfInterestRegister(int adc);
void        setROIADC(int adc);
void        setGbitReadout();
int         readConfigFile();
void        setMasterSlaveConfiguration();
#endif

// parameters - dr, roi
int 		setDynamicRange(int dr);
#ifdef GOTTHARDD
ROI* 		setROI(int n, ROI arg[], int *retvalsize, int *ret);
#endif

// parameters - readout
#ifndef GOTTHARDD
enum speedVariable 		setSpeed(int val);
#endif
#ifdef EIGERD
enum 		readOutFlags setReadOutFlags(enum readOutFlags val);
#endif

// parameters - timer
#ifdef JUNGFRAUD
int         selectStoragecellStart(int pos);
#endif
int64_t 	setTimer(enum timerIndex ind, int64_t val);
int64_t 	getTimeLeft(enum timerIndex ind);
#if defined(JUNGFRAUD) || (GOTTHARDD)
int         validateTimer(enum timerIndex ind, int64_t val, int64_t retval);
#endif

// parameters - module, settings
int 		setModule(sls_detector_module myMod, char* mess);
int 		getModule(sls_detector_module *myMod);
enum 		detectorSettings setSettings(enum detectorSettings sett);
enum 		detectorSettings getSettings();


// parameters - threshold
#ifdef EIGERD
int 		getThresholdEnergy();
int 		setThresholdEnergy(int ev);
#endif

// parameters - dac, adc, hv
#if defined(GOTTHARDD) || defined(JUNGFRAUD)
void        serializeToSPI(u_int32_t addr, u_int32_t val, u_int32_t csmask, int numbitstosend, u_int32_t clkmask, u_int32_t digoutmask, int digofset); //commonServerFunction.h
void        initDac(int dacnum);
int         voltageToDac(int value);
int         dacToVoltage(unsigned int digital);
#endif
#ifdef GOTTHARDD
extern void setAdc9257(int addr, int val);      // AD9257.h
extern void setAdc9252(int addr, int val);      // AD9252.h (old board)
#elif JUNGFRAUD
extern void setAdc9257(int addr, int val);      // AD9257.h
#endif

void 		setDAC(enum DACINDEX ind, int val, int mV, int retval[]);
/*#ifdef GOTTHARDD
void        initDAC(int dac_addr, int value);
void        clearDACSregister();
void        nextDAC();
void        program_one_dac(int addr, int value);
u_int32_t   putout(char *s);
#endif*/
int 		getADC(enum ADCINDEX ind);

int 		setHighVoltage(int val);



// parameters - timing, extsig
void 		setTiming( enum externalCommunicationMode arg);
enum 		externalCommunicationMode getTiming();
#ifdef GOTTHARDD
void        setExtSignal(enum externalSignalFlag  mode);
int         getExtSignal();
#endif

// configure mac
#ifdef GOTTHARDD
void        calcChecksum(mac_conf* mac, int sourceip, int destip);
#elif JUNGFRAUD
long int 	calcChecksum(int sourceip, int destip);
#endif
#ifdef GOTTHARDD
int         getAdcConfigured();
#endif
int 		configureMAC(uint32_t destip, uint64_t destmac, uint64_t sourcemac, uint32_t sourceip, uint32_t udpport, uint32_t udpport2);
#if defined(JUNGFRAUD) || defined(EIGERD)
int 		setDetectorPosition(int pos[]);
#endif


// very detector specific

// gotthard specific - image, pedestal
#ifdef GOTTHARDD
void 		loadImage(enum imageType index, short int imageVals[]);
int 		readCounterBlock(int startACQ, short int counterVals[]);
int			resetCounterBlock(int startACQ);

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
#ifdef EIGERD
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
#ifdef EIGERD
int 		startReadOut();
#endif
enum 		runStatus getRunStatus();
void 		readFrame(int *ret, char *mess);
#if defined(GOTTHARDD) || defined(JUNGFRAUD)
u_int32_t 	runBusy();
#endif
#ifdef GOTTHARDD
u_int32_t   runState(enum TLogLevel lev);
#endif


//common
#ifdef EIGERD
int 		copyModule(sls_detector_module *destMod, sls_detector_module *srcMod);
#endif
int 		calculateDataBytes();
int 		getTotalNumberOfChannels();
int 		getNumberOfChips();
int 		getNumberOfDACs();
int 		getNumberOfChannelsPerChip();


