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
int			isInitCheckDone();
int			getInitResult(char** mess);
void 		basictests();
#if defined(GOTTHARDD) || defined(JUNGFRAUD) || defined(CHIPTESTBOARDD) || defined(MOENCHD) || defined(MYTHEN3D) || defined(GOTTHARD2D)
int 		checkType();
int 		testFpga();
int 		testBus();
#endif

#ifdef GOTTHARDD
void 		setTestImageMode(int ival);
int			getTestImageMode();
#endif

// Ids
u_int64_t 	getServerVersion();
u_int64_t 	getClientServerAPIVersion();
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
int         updateDatabytesandAllocateRAM();
void        updateDataBytes();
#endif

#if defined(GOTTHARDD) || defined(JUNGFRAUD) || defined(MYTHEN3D)
int			setDefaultDacs();
#endif
#ifdef GOTTHARD2D
int         readConfigFile();
#endif


// advanced read/write reg
#ifdef EIGERD
int			writeRegister(uint32_t offset, uint32_t data);
int  		readRegister(uint32_t offset, uint32_t* retval);
#elif GOTTHARDD
uint32_t    writeRegister16And32(uint32_t offset, uint32_t data); //FIXME its not there in ctb or moench?
uint32_t    readRegister16And32(uint32_t offset);
#else
extern u_int32_t    writeRegister(u_int32_t offset, u_int32_t data);    // blackfin.h or nios.h
extern u_int32_t    readRegister(u_int32_t offset);                     // blackfin.h or nios.h
#endif


// firmware functions (resets)
#if defined(JUNGFRAUD) || defined(CHIPTESTBOARDD) || defined(MOENCHD) || defined(MYTHEN3D) || defined(GOTTHARD2D)
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
void 		setADCEnableMask_10G(uint32_t mask);
uint32_t 	getADCEnableMask_10G();
void 		setADCInvertRegister(uint32_t val);
uint32_t 	getADCInvertRegister();
int			setExternalSamplingSource(int val);
int			setExternalSampling(int val);
#endif

// parameters - readout
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
void		setNumFrames(int64_t val);
int64_t 	getNumFrames();
void		setNumTriggers(int64_t val);
int64_t 	getNumTriggers();
int			setExpTime(int64_t val);
int64_t 	getExpTime();
int			setPeriod(int64_t val);
int64_t 	getPeriod();
#ifdef GOTTHARD2D
void		setNumFramesBurst(int64_t val);
int64_t		getNumFramesBurst();
void		setNumFramesCont(int64_t val);
int64_t		getNumFramesCont();
int			setExptimeBurst(int64_t val);
int			setExptimeCont(int64_t val);
int			setExptimeBoth(int64_t val);
int64_t		getExptimeBoth();
int			setPeriodBurst(int64_t val);
int64_t		getPeriodBurst();
int			setPeriodCont(int64_t val);
int64_t		getPeriodCont();
#endif
#ifdef EIGERD
int			setSubExpTime(int64_t val);
int64_t 	getSubExpTime();
int			setDeadTime(int64_t val);
int64_t 	getDeadTime();
int64_t		getMeasuredPeriod();
int64_t		getMeasuredSubPeriod();
#endif
#ifdef JUNGFRAUD
void		setNumAdditionalStorageCells(int val);
int 		getNumAdditionalStorageCells();
int			setStorageCellDelay(int64_t val);
int64_t 	getStorageCellDelay();
#endif
#if defined(CHIPTESTBOARDD) || defined(MOENCHD)
int			setNumAnalogSamples(int val);
int 		getNumAnalogSamples();
int			setNumDigitalSamples(int val);
int 		getNumDigitalSamples();
#endif
#ifdef MYTHEN3D
void		setCounterMask(uint32_t arg);
uint32_t	getCounterMask();
#endif

#if defined(JUNGFRAUD) || defined(GOTTHARDD) || defined(CHIPTESTBOARDD) || defined(MOENCHD) || defined(MYTHEN3D) || defined(GOTTHARD2D)
int			setDelayAfterTrigger(int64_t val);
int64_t 	getDelayAfterTrigger();
int64_t		getNumFramesLeft();
int64_t		getNumTriggersLeft();
int64_t		getDelayAfterTriggerLeft();
int64_t		getPeriodLeft();
#endif
#ifdef GOTTHARDD
int64_t		getExpTimeLeft();
#endif
#if defined(JUNGFRAUD) || defined(CHIPTESTBOARDD) || defined(MOENCHD) || defined(MYTHEN3D) || defined(GOTTHARD2D)
int64_t		getFramesFromStart();
int64_t		getActualTime();
int64_t		getMeasurementTime();
#endif



// parameters - module, settings
#if (!defined(CHIPTESTBOARDD)) && (!defined(MOENCHD)) && (!defined(MYTHEN3D)) && (!defined(GOTTHARD2D))
int 		setModule(sls_detector_module myMod, char* mess);
int 		getModule(sls_detector_module *myMod);
#endif
#if (!defined(CHIPTESTBOARDD)) && (!defined(MOENCHD)) && (!defined(MYTHEN3D)) 
enum 		detectorSettings setSettings(enum detectorSettings sett);
#endif
#if !defined(MYTHEN3D)
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

#ifdef GOTTHARD2D
int			setOnChipDAC(enum ONCHIP_DACINDEX ind, int chipIndex, int val);
int			getOnChipDAC(enum ONCHIP_DACINDEX ind, int chipIndex);
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
void 		setTiming( enum timingMode arg);
enum 		timingMode getTiming();
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
#if defined(JUNGFRAUD) || defined(GOTTHARD2D) || defined(MYTHEN3D) || defined(CHIPTESTBOARDD) || defined(MOENCHD)
void 	calcChecksum(udp_header* udp);
#endif
#ifdef GOTTHARDD
int         getAdcConfigured();
#endif



int 		configureMAC();
int 		setDetectorPosition(int pos[]);
int*		getDetectorPosition();


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
int        	setPhase(enum CLKINDEX ind, int val, int degrees);
int         getPhase(enum CLKINDEX ind, int degrees);
int         getMaxPhase(enum CLKINDEX ind);
int 		validatePhaseinDegrees(enum CLKINDEX ind, int val, int retval);
int       	setFrequency(enum CLKINDEX ind, int val);
int         getFrequency(enum CLKINDEX ind);
void        configureSyncFrequency(enum CLKINDEX ind);
void        setPipeline(enum CLKINDEX ind, int val);
int         getPipeline(enum CLKINDEX ind);
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
int       	setClockDivider(enum CLKINDEX ind, int val);
int         getClockDivider(enum CLKINDEX ind);
int        	setPhase(enum CLKINDEX ind, int val, int degrees);
int         getPhase(enum CLKINDEX ind, int degrees);
int         getMaxPhase(enum CLKINDEX ind);
int 		validatePhaseinDegrees(enum CLKINDEX ind, int val, int retval);
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
int       	setClockDivider(enum CLKINDEX ind, int val);
int         getClockDivider(enum CLKINDEX ind);
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

// gotthard specific - adc phase
#elif GOTTHARDD
int        	setPhase(enum CLKINDEX ind, int val, int degrees);

#elif MYTHEN3D
uint64_t    readPatternWord(int addr);
uint64_t    writePatternWord(int addr, uint64_t word);
int         setPatternWaitAddress(int level, int addr);
uint64_t    setPatternWaitTime(int level, uint64_t t);
void        setPatternLoop(int level, int *startAddr, int *stopAddr, int *nLoop);
int			checkDetectorType();
int         powerChip (int on);
int        	setPhase(enum CLKINDEX ind, int val, int degrees);
int         getPhase(enum CLKINDEX ind, int degrees);
int         getMaxPhase(enum CLKINDEX ind);
int 		validatePhaseinDegrees(enum CLKINDEX ind, int val, int retval);
//void       	setFrequency(enum CLKINDEX ind, int val);
int         getFrequency(enum CLKINDEX ind);
int         getVCOFrequency(enum CLKINDEX ind);
int       	getMaxClockDivider();
int       	setClockDivider(enum CLKINDEX ind, int val);
int         getClockDivider(enum CLKINDEX ind);

#elif GOTTHARD2D
int			checkDetectorType();
int         powerChip (int on);
int        	setPhase(enum CLKINDEX ind, int val, int degrees);
int         getPhase(enum CLKINDEX ind, int degrees);
int         getMaxPhase(enum CLKINDEX ind);
int 		validatePhaseinDegrees(enum CLKINDEX ind, int val, int retval);
//void       	setFrequency(enum CLKINDEX ind, int val);
int         getFrequency(enum CLKINDEX ind);
int         getVCOFrequency(enum CLKINDEX ind);
int       	getMaxClockDivider();
int       	setClockDivider(enum CLKINDEX ind, int val);
int         getClockDivider(enum CLKINDEX ind);
int 		setInjectChannel(int offset, int increment);
void		getInjectedChannels(int* offset, int* increment);
int			setVetoReference(int gainIndex, int value);
int			setVetoPhoton(int chipIndex, int gainIndex, int* values); 
int			getVetoPhoton(int chipIndex, int* retvals);
int			configureSingleADCDriver(int chipIndex);
int			configureADC();
int			setBurstMode(int burst);
int			getBurstMode();
void		setBurstType(enum burstModeType val);
enum burstModeType getBurstType();
#endif





#if defined(JUNGFRAUD) || defined(EIGERD)
int 		getTenGigaFlowControl();
int 		setTenGigaFlowControl(int value);
int 		getTransmissionDelayFrame();
int 		setTransmissionDelayFrame(int value);
#endif
#ifdef EIGERD
int 		getTransmissionDelayLeft();
int 		setTransmissionDelayLeft(int value);
int 		getTransmissionDelayRight();
int 		setTransmissionDelayRight(int value);
#endif




// aquisition
#ifdef GOTTHARD2D
int			updateAcquisitionRegisters(char* mess);
#endif
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


