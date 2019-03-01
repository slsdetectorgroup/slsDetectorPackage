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
int			isFirmwareCheckDone();
int			getFirmwareCheckResult(char** mess);
void 		basictests();
#if defined(GOTTHARDD) || defined(JUNGFRAUD) || defined(CHIPTESTBOARDD) || defined(MOENCHD)
int 		checkType();
u_int32_t 	testFpga(void);
int 		testBus(void);
#endif

#ifdef GOTTHARDD
int         detectorTest(enum digitalTestMode arg,  int ival);
#elif defined(JUNGFRAUD) || defined(CHIPTESTBOARDD) || defined(MOENCHD)
int 		detectorTest(enum digitalTestMode arg);
#endif

// Ids
int64_t 	getDetectorId(enum idMode arg);
u_int64_t  	getFirmwareVersion();
#if defined(JUNGFRAUD) || defined(CHIPTESTBOARDD) || defined(MOENCHD)
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
#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
int         allocateRAM();
void        updateDataBytes();
int         getChannels();
#endif

#if defined(GOTTHARDD) || defined(JUNGFRAUD)
int			setDefaultDacs();
#endif


// advanced read/write reg
#ifdef EIGERD
uint32_t	writeRegister(uint32_t offset, uint32_t data);
uint32_t  	readRegister(uint32_t offset);
#elif GOTTHARDD
uint32_t    writeRegister16And32(uint32_t offset, uint32_t data); //FIXME its not there in ctb or moench?
uint32_t    readRegister16And32(uint32_t offset);
#else
extern u_int32_t    writeRegister(u_int32_t offset, u_int32_t data);    // blackfin.h
extern u_int32_t    readRegister(u_int32_t offset);                     // blackfin.h
#endif


// firmware functions (resets)
#ifdef JUNGFRAUD
void 		cleanFifos();
void 		resetCore();
void 		resetPeripheral();
#elif GOTTHARDD
void        setPhaseShiftOnce();
void        setPhaseShift(int numphaseshift);
void        cleanFifos();
void        setADCSyncRegister();
void        setDAQRegister();
void        setChipOfInterestRegister(int adc);
void        setROIADC(int adc);
void        setGbitReadout();
int         readConfigFile();
void        setMasterSlaveConfiguration();
#elif CHIPTESTBOARDD
void        cleanFifos();
void        resetCore();
void        resetPeripheral();
#elif MOENCHD
void        cleanFifos();
void        resetCore();
void        resetPeripheral();
#endif

// parameters - dr, roi
int 		setDynamicRange(int dr);
#if defined(GOTTHARDD) || defined(CHIPTESTBOARDD) || defined(MOENCHD)
ROI* 		setROI(int n, ROI arg[], int *retvalsize, int *ret);
#endif

// parameters - readout
#ifndef GOTTHARDD
void 		setSpeed(enum speedVariable ind, int val);
int         getSpeed(enum speedVariable ind);
#endif

#if defined(EIGERD) || defined(CHIPTESTBOARDD) || defined(MOENCHD)
enum 		readOutFlags setReadOutFlags(enum readOutFlags val);
#endif

// parameters - timer
#ifdef JUNGFRAUD
int         selectStoragecellStart(int pos);
#endif
int64_t 	setTimer(enum timerIndex ind, int64_t val);
int64_t 	getTimeLeft(enum timerIndex ind);
#if defined(JUNGFRAUD) || defined(GOTTHARDD) || defined(CHIPTESTBOARDD) || defined(MOENCHD)
int         validateTimer(enum timerIndex ind, int64_t val, int64_t retval);
#endif

// parameters - module, settings
#if (!defined(CHIPTESTBOARDD)) && (!defined(MOENCHD))
int 		setModule(sls_detector_module myMod, char* mess);
int 		getModule(sls_detector_module *myMod);
enum 		detectorSettings setSettings(enum detectorSettings sett);
#endif
enum 		detectorSettings getSettings();

// parameters - threshold
#ifdef EIGERD
int 		getThresholdEnergy();
int 		setThresholdEnergy(int ev);
#endif

// parameters - dac, adc, hv
#ifdef GOTTHARDD
extern void AD9252_Set(int addr, int val);      // AD9252.h (old board)
#endif
#if defined(GOTTHARDD) || defined(JUNGFRAUD) || defined(CHIPTESTBOARDD) || defined(MOENCHD)
extern void AD9257_Set(int addr, int val);      // AD9257.h
#endif
#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
extern int AD9257_GetMaxValidVref();                   // AD9257.h
extern void AD9257_SetVrefVoltage(int val);             // AD9257.h
#endif

void 		setDAC(enum DACINDEX ind, int val, int mV);
int         getDAC(enum DACINDEX ind, int mV);
int         getMaxDacSteps();
#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
int         dacToVoltage(int dac);
int         checkVLimitCompliant(int mV);
int         checkVLimitDacCompliant(int dac);
int         getVLimit();
void        setVLimit(int l);
#endif

#ifdef CHIPTESTBOARDD
int         isVchipValid(int val);
int         getVchip();
void        setVchip(int val);
int         getVChipToSet(enum DACINDEX ind, int val);
int         getDACIndexFromADCIndex(enum ADCINDEX ind);
int         getADCIndexFromDACIndex(enum DACINDEX ind);
int         isPowerValid(enum DACINDEX ind, int val);
int         getPower();
void        setPower(enum DACINDEX ind, int val);
void        powerOff();
#endif
/*#ifdef GOTTHARDD
void        initDAC(int dac_addr, int value);
void        clearDACSregister();
void        nextDAC();
void        program_one_dac(int addr, int value);
u_int32_t   putout(char *s);
#endif*/
#ifndef MOENCHD
int 		getADC(enum ADCINDEX ind);
#endif

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
#endif
#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
long int 	calcChecksum(int sourceip, int destip);
#endif
#ifdef GOTTHARDD
int         getAdcConfigured();
#endif
int 		configureMAC(uint32_t destip, uint64_t destmac, uint64_t sourcemac, uint32_t sourceip, uint32_t udpport, uint32_t udpport2);
#if defined(JUNGFRAUD) || defined(EIGERD)
int 		setDetectorPosition(int pos[]);
#endif
#if defined(CHIPTESTBOARDD) || defined(MOENCHD) || defined(EIGERD)
int 		enableTenGigabitEthernet(int val);
#endif


// very detector specific

// moench specific - powerchip
#ifdef MOENCHD
int         powerChip (int on);
#endif

// chip test board specific - sendudp, pll, flashing firmware
#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
void        configurePhase(enum CLKINDEX ind, int val);
int         getPhase(enum CLKINDEX ind);
void        configureFrequency(enum CLKINDEX ind, int val);
int         getFrequency(enum CLKINDEX ind);
void        configureSyncFrequency(enum CLKINDEX ind);
void        setAdcOffsetRegister(int adc, int val);
int         getAdcOffsetRegister(int adc);
extern void eraseFlash();                                                   // programfpga.h
extern int  startWritingFPGAprogram(FILE** filefp);                         // programfpga.h
extern void stopWritingFPGAprogram(FILE* filefp);                           // programfpga.h
extern int  writeFPGAProgram(char* fpgasrc, size_t fsize, FILE* filefp);    // programfpga.h
// ctb patterns
uint64_t    writePatternIOControl(uint64_t word);
uint64_t    writePatternClkControl(uint64_t word);
uint64_t    readPatternWord(int addr);
uint64_t    writePatternWord(int addr, uint64_t word);
int         setPatternWaitAddress(int level, int addr);
uint64_t    setPatternWaitTime(int level, uint64_t t);
void         setPatternLoop(int level, int *startAddr, int *stopAddr, int *nLoop);
int			setLEDEnable(int enable);
void			setDigitalIODelay(uint64_t pinMask, int delay);
#endif

// gotthard specific - image, pedestal
#ifdef GOTTHARDD
void 		loadImage(enum imageType index, short int imageVals[]);
int 		readCounterBlock(int startACQ, short int counterVals[]);
int			resetCounterBlock(int startACQ);

// jungfrau specific - powerchip, autocompdisable, clockdiv, asictimer, clock, pll, flashing firmware
#elif JUNGFRAUD
int         powerChip (int on);
int         autoCompDisable(int on);
void        configureASICTimer();
void        setClockDivider(int val);
int         getClockDivider();
int         setAdcPhase(int st);
int         getPhase();
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

// eiger specific - iodelay, pulse, rate, temp, activate, delay nw parameter
#elif EIGERD
int 		setIODelay(int val);
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
#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
void 		readandSendUDPFrames(int *ret, char *mess);
void        unsetFifoReadStrobes();
void        readSample(int ns);
int         checkDataPresent();
int         readFrameFromFifo();
#endif

#if defined(GOTTHARDD) || defined(JUNGFRAUD) || defined(CHIPTESTBOARDD) || defined(MOENCHD)
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


