#include "sls_detector_defs.h"
#include "slsDetectorServer_defs.h" // DAC_INDEX, ADC_INDEX, also include RegisterDefs.h
#ifdef GOTTHARDD
#include "clogger.h"                 // runState(enum TLogLevel)
#endif
#include <stdlib.h>
#include <stdio.h>					// FILE
#include <sys/types.h>

/****************************************************
This functions are used by the slsDetectroServer_funcs interface.
Here are the definitions, but the actual implementation should be done for each single detector.

****************************************************/

enum interfaceType {OUTER, INNER};
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
}udpStruct;


// basic tests
int			isFirmwareCheckDone();
int			getFirmwareCheckResult(char** mess);
void 		basictests();
#if defined(GOTTHARDD) || defined(JUNGFRAUD) || defined(CHIPTESTBOARDD) || defined(MOENCHD) || defined(MYTHEN3D) || defined(GOTTHARD2D)
int 		checkType();
int 		testFpga();
int 		testBus();
#endif

#ifdef GOTTHARDD
int         detectorTest(enum digitalTestMode arg,  int ival);
int 		testImage(int ival);
#elif defined(JUNGFRAUD) || defined(CHIPTESTBOARDD) || defined(MOENCHD)
int 		detectorTest(enum digitalTestMode arg);
#endif

// Ids
int64_t 	getDetectorId(enum idMode arg);
u_int64_t  	getFirmwareVersion();
u_int64_t   getFirmwareAPIVersion();
#if defined(JUNGFRAUD) || defined(CHIPTESTBOARDD) || defined(MOENCHD) || defined(MYTHEN3D) || defined(GOTTHARD2D)
u_int16_t 	getHardwareVersionNumber();
#endif
#if defined(JUNGFRAUD) || defined(CHIPTESTBOARDD) || defined(MOENCHD)
u_int16_t 	getHardwareSerialNumber();
#endif
#ifdef JUNGFRAUD
int			isHardwareVersion2();
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
#endif

#if defined(GOTTHARDD) || defined(JUNGFRAUD) || defined(MYTHEN3D) || GOTTHARD2D
int			setDefaultDacs();
#endif


// advanced read/write reg
#ifdef EIGERD
int			writeRegister(uint32_t offset, uint32_t data);
int  		readRegister(uint32_t offset, uint32_t* retval);
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
#ifdef GOTTHARDD
int 		setROI(ROI arg);
ROI			getROI();
#endif
#ifdef JUNGFRAUD
void 		setADCInvertRegister(uint32_t val);
uint32_t 	getADCInvertRegister();
#endif
#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
int 		setADCEnableMask(uint32_t mask);
uint32_t 	getADCEnableMask();
void 		setADCInvertRegister(uint32_t val);
uint32_t 	getADCInvertRegister();
int			setExternalSamplingSource(int val);
int			setExternalSampling(int val);
#endif

// parameters - readout
#if defined(CHIPTESTBOARDD) || defined(MOENCHD) || defined(JUNGFRAUD)
void 		setSpeed(enum speedVariable ind, int val, int mode);
int         getSpeed(enum speedVariable ind, int mode);
#else
#ifndef GOTTHARD2D
void 		setSpeed(enum speedVariable ind, int val);
int         getSpeed(enum speedVariable ind);
#endif
#endif

#ifdef EIGERD
int			setParallelMode(int mode);
int 		getParallelMode();
int			setOverFlowMode(int mode);
int 		getOverFlowMode();
void		setStoreInRamMode(int mode);
int 		getStoreInRamMode();
#endif
#ifdef CHIPTESTBOARDD
int 		setReadoutMode(enum readoutMode mode);
int 		getReadoutMode();
#endif



// parameters - timer
#ifdef JUNGFRAUD
int         selectStoragecellStart(int pos);
#endif
#if defined(JUNGFRAUD) || defined(EIGERD) 
int 		setStartingFrameNumber(uint64_t value);
int			getStartingFrameNumber(uint64_t* value);
#endif
int64_t 	setTimer(enum timerIndex ind, int64_t val);
int64_t 	getTimeLeft(enum timerIndex ind);
#if defined(JUNGFRAUD) || defined(GOTTHARDD) || defined(CHIPTESTBOARDD) || defined(MOENCHD) || defined(MYTHEN3D) || defined(GOTTHARD2D)
int         validateTimer(enum timerIndex ind, int64_t val, int64_t retval);
#endif

// parameters - module, settings
#if (!defined(CHIPTESTBOARDD)) && (!defined(MOENCHD)) && (!defined(MYTHEN3D)) && (!defined(GOTTHARD2D))
int 		setModule(sls_detector_module myMod, char* mess);
int 		getModule(sls_detector_module *myMod);
enum 		detectorSettings setSettings(enum detectorSettings sett);
#endif
#if !defined(MYTHEN3D) && !defined(GOTTHARD2D)
enum 		detectorSettings getSettings();
#endif

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
extern int AD9257_GetVrefVoltage(int mV);                  // AD9257.h
extern int AD9257_SetVrefVoltage(int val, int mV);   // AD9257.h
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

#ifndef MOENCHD
#if !defined(MYTHEN3D) && !defined(GOTTHARD2D)
int 		getADC(enum ADCINDEX ind);
#endif
#endif

int 		setHighVoltage(int val);



// parameters - timing, extsig
#if !defined(MYTHEN3D) && !defined(GOTTHARD2D)
void 		setTiming( enum timingMode arg);
enum 		timingMode getTiming();
#endif
#ifdef GOTTHARDD
void        setExtSignal(enum externalSignalFlag  mode);
int         getExtSignal();
#endif

// configure mac
#ifdef GOTTHARDD
void        calcChecksum(mac_conf* mac, int sourceip, int destip);
#elif JUNGFRAUD
void 	setNumberofUDPInterfaces(int val);
int 	getNumberofUDPInterfaces();
void 	selectPrimaryInterface(int val);
int 	getPrimaryInterface();
void 	setupHeader(int iRxEntry, enum interfaceType type, uint32_t destip, uint64_t destmac, uint32_t destport, uint64_t sourcemac, uint32_t sourceip, uint32_t sourceport);
#endif
#if defined(JUNGFRAUD) || defined(GOTTHARD2D)
void 	calcChecksum(udp_header* udp);
#endif
#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
long int 	calcChecksum(int sourceip, int destip);
#endif
#ifdef GOTTHARDD
int         getAdcConfigured();
#endif



int 		configureMAC();
int 		setDetectorPosition(int pos[]);
int*		getDetectorPosition();
int			isConfigurable();


#ifdef EIGERD
int			setQuad(int value);
int			getQuad();
int			setInterruptSubframe(int value);
int			getInterruptSubframe();
int 		setReadNLines(int value);
int			getReadNLines();
#endif
#if defined(CHIPTESTBOARDD) || defined(MOENCHD) || defined(EIGERD)
int 		enableTenGigabitEthernet(int val);
#endif


// very detector specific

// moench specific - powerchip
#ifdef MOENCHD
int         powerChip (int on);
#endif

// chip test board or moench specific - configure frequency, phase, pll, flashing firmware
#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
void        configurePhase(enum CLKINDEX ind, int val, int degrees);
int         getPhase(enum CLKINDEX ind, int degrees);
int         getMaxPhase(enum CLKINDEX ind);
int 		validatePhaseinDegrees(enum speedVariable ind, int val, int retval);
void        configureFrequency(enum CLKINDEX ind, int val);
int         getFrequency(enum CLKINDEX ind);
void        configureSyncFrequency(enum CLKINDEX ind);
void        setAdcOffsetRegister(int adc, int val);
int         getAdcOffsetRegister(int adc);
extern void eraseFlash();                                                   // programfpga.h
extern int  startWritingFPGAprogram(FILE** filefp);                         // programfpga.h
extern void stopWritingFPGAprogram(FILE* filefp);                           // programfpga.h
extern int  writeFPGAProgram(char* fpgasrc, size_t fsize, FILE* filefp);    // programfpga.h
// patterns
uint64_t    writePatternIOControl(uint64_t word);
uint64_t    writePatternClkControl(uint64_t word);
uint64_t    readPatternWord(int addr);
uint64_t    writePatternWord(int addr, uint64_t word);
int         setPatternWaitAddress(int level, int addr);
uint64_t    setPatternWaitTime(int level, uint64_t t);
void        setPatternLoop(int level, int *startAddr, int *stopAddr, int *nLoop);
int			setLEDEnable(int enable);
void		setDigitalIODelay(uint64_t pinMask, int delay);
void 		setPatternMask(uint64_t mask);
uint64_t	getPatternMask();
void 		setPatternBitMask(uint64_t mask);
uint64_t	getPatternBitMask();
#endif

// jungfrau specific - powerchip, autocompdisable, clockdiv, asictimer, clock, pll, flashing firmware
#ifdef JUNGFRAUD
void 		initReadoutConfiguration();
int         powerChip (int on);
int         autoCompDisable(int on);
void        configureASICTimer();
void        setClockDivider(int val);
int         getClockDivider();
void        setAdcPhase(int val, int degrees);
int         getPhase(int degrees);
int			getMaxPhaseShift();
int 		validatePhaseinDegrees(int val, int retval);
int         setThresholdTemperature(int val);
int         setTemperatureControl(int val);
int         setTemperatureEvent(int val);
extern void eraseFlash();													// programfpga.h
extern int 	startWritingFPGAprogram(FILE** filefp);							// programfpga.h
extern void stopWritingFPGAprogram(FILE* filefp);							// programfpga.h
extern int 	writeFPGAProgram(char* fpgasrc, size_t fsize, FILE* filefp);	// programfpga.h
void		alignDeserializer();

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

#elif MYTHEN3D
uint64_t    readPatternWord(int addr);
uint64_t    writePatternWord(int addr, uint64_t word);
int         setPatternWaitAddress(int level, int addr);
uint64_t    setPatternWaitTime(int level, uint64_t t);
void        setPatternLoop(int level, int *startAddr, int *stopAddr, int *nLoop);


#elif GOTTHARD2D
int        	setPhase(enum CLKINDEX ind, int val, int degrees);
int         getPhase(enum CLKINDEX ind, int degrees);
int         getMaxPhase(enum CLKINDEX ind);
int 		validatePhaseinDegrees(enum CLKINDEX ind, int val, int retval);
//int       	setFrequency(enum CLKINDEX ind, int val);
int         getFrequency(enum CLKINDEX ind);
int         getVCOFrequency(enum CLKINDEX ind);
int       	getMaxClockDivider();
int       	setClockDivider(enum CLKINDEX ind, int val);
int         getClockDivider(enum CLKINDEX ind);
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
uint32_t	checkDataInFifo();
int         checkFifoForEndOfAcquisition();
int         readFrameFromFifo();
#endif

#if defined(GOTTHARDD) || defined(JUNGFRAUD) || defined(CHIPTESTBOARDD) || defined(MOENCHD) || defined(MYTHEN3D) || defined(GOTTHARD2D)
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


