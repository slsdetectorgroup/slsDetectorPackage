#ifdef SLS_DETECTOR_FUNCTION_LIST

#include "slsDetectorFunctionList.h"
#include "slsDetectorServer_defs.h"

#include <stdio.h>
#include <string.h>


const int nChans=NCHAN;
const int nChips=NCHIP;
const int nDacs=NDAC;
const int nAdcs=NADC;
const int allSelected=-2;
const int noneSelected=-1;

sls_detector_module *detectorModules=NULL;
int *detectorChips=NULL;
int *detectorChans=NULL;
dacs_t *detectorDacs=NULL;
dacs_t *detectorAdcs=NULL;

int nModY		=	NMAXMOD;
int nModX		=	NMAXMOD;
int dynamicRange=	DYNAMIC_RANGE;
int dataBytes	=	NMAXMOD*NCHIP*NCHAN*2;
int masterMode	=	NO_MASTER;
int syncMode	=	NO_SYNCHRONIZATION;
int timingMode	=	AUTO_TIMING;



enum detectorSettings thisSettings;
int sChan, sChip, sMod, sDac, sAdc;
int nModBoard;
extern int dataBytes;


int initializeDetectorStructure(){

	int imod;
	int n=getNModBoard(X)*getNModBoard(Y);
#ifdef VERBOSE
	printf("Board is for %d modules\n",n);
#endif
	detectorModules=malloc(n*sizeof(sls_detector_module));
	detectorChips=malloc(n*NCHIP*sizeof(int));
	detectorChans=malloc(n*NCHIP*NCHAN*sizeof(int));
	detectorDacs=malloc(n*NDAC*sizeof(int));
	detectorAdcs=malloc(n*NADC*sizeof(int));
#ifdef VERBOSE
	printf("modules from 0x%x to 0x%x\n",(unsigned int)(detectorModules), (unsigned int)(detectorModules+n));
	printf("chips from 0x%x to 0x%x\n",(unsigned int)(detectorChips), (unsigned int)(detectorChips+n*NCHIP));
	printf("chans from 0x%x to 0x%x\n",(unsigned int)(detectorChans), (unsigned int)(detectorChans+n*NCHIP*NCHAN));
	printf("dacs from 0x%x to 0x%x\n",(unsigned int)(detectorDacs), (unsigned int)(detectorDacs+n*NDAC));
	printf("adcs from 0x%x to 0x%x\n",(unsigned int)(detectorAdcs), (unsigned int)(detectorAdcs+n*NADC));
#endif
	for (imod=0; imod<n; imod++) {
		(detectorModules+imod)->dacs=detectorDacs+imod*NDAC;
		(detectorModules+imod)->adcs=detectorAdcs+imod*NADC;
		(detectorModules+imod)->chipregs=detectorChips+imod*NCHIP;
		(detectorModules+imod)->chanregs=detectorChans+imod*NCHIP*NCHAN;
		(detectorModules+imod)->ndac=NDAC;
		(detectorModules+imod)->nadc=NADC;
		(detectorModules+imod)->nchip=NCHIP;
		(detectorModules+imod)->nchan=NCHIP*NCHAN;
		(detectorModules+imod)->module=imod;
		(detectorModules+imod)->gain=0;
		(detectorModules+imod)->offset=0;
		(detectorModules+imod)->reg=0;
		/* initialize registers, dacs, retrieve sn, adc values etc */
	}
	thisSettings=UNINITIALIZED;
	sChan=noneSelected;
	sChip=noneSelected;
	sMod=noneSelected;
	sDac=noneSelected;
	sAdc=noneSelected;

	return OK;
}






int setupDetector(){
	//testFpga();
	//testRAM();

	//setSettings(GET_SETTINGS,-1);
	//setFrames(1);
	//setTrains(1);
	//setExposureTime(1e6);
	//setPeriod(1e9);
	//setDelay(0);
	//setGates(0);

	//setTiming(GET_EXTERNAL_COMMUNICATION_MODE);
	//setMaster(GET_MASTER);
	//setSynchronization(GET_SYNCHRONIZATION_MODE);
	return OK;
}




int setNMod(int nm, enum dimension dim){
	return 1;
}



int getNModBoard(enum dimension arg){
	return 1;
}








int64_t getModuleId(enum idMode arg, int imod){
	//DETECTOR_SERIAL_NUMBER
	//DETECTOR_FIRMWARE_VERSION
	return 0;
}




int64_t getDetectorId(enum idMode arg){
	//DETECTOR_SOFTWARE_VERSION defined in slsDetector_defs.h?
	return 0;
}





int moduleTest( enum digitalTestMode arg, int imod){
	//template testShiftIn from mcb_funcs.c

	//CHIP_TEST
	//testShiftIn
	//testShiftOut
	//testShiftStSel
	//testDataInOutMux
	//testExtPulseMux
	//testOutMux
	//testFpgaMux

	return OK;
}





int detectorTest( enum digitalTestMode arg){
	//templates from firmware_funcs.c

	//DETECTOR_FIRMWARE_TEST:testFpga()
	//DETECTOR_MEMORY_TEST:testRAM()
	//DETECTOR_BUS_TEST:testBus()
	//DETECTOR_SOFTWARE_TEST:testFpga()
	return OK;
}






double setDAC(enum dacIndex ind, double val, int imod){
	//template initDACbyIndexDACU from mcb_funcs.c

	//check that slsDetectorServer_funcs.c set_dac() has all the specific dac enums
	//set dac and write to a register in fpga to remember dac value when server restarts
	return 0;
}



double getADC(enum dacIndex ind,  int imod){
	//get adc value
	return 0;
}




int setChannel(sls_detector_channel myChan){
	//template initChannelByNumber() from mcb_funcs.c

	return myChan.reg;
}


int getChannel(sls_detector_channel *myChan){
	//template getChannelbyNumber() from mcb_funcs.c
	return FAIL;
}



int setChip(sls_detector_chip myChip){
	//template initChipbyNumber() from mcb_funcs.c
	return myChip.reg;
}


int getChip(sls_detector_chip *myChip){
	//template getChipbyNumber() from mcb_funcs.c
	return FAIL;
}

int setModule(sls_detector_module myChan){
	//template initModulebyNumber() from mcb_funcs.c
	return OK;
}

int getModule(sls_detector_module *myChan){
	//template getModulebyNumber() from mcb_funcs.c
	return FAIL;
}

int getThresholdEnergy(int imod){
	//template getThresholdEnergy() from mcb_funcs.c
	//depending on settings
	return FAIL;
}


int setThresholdEnergy(int thr, int imod){
	//template getThresholdEnergy() from mcb_funcs.c
	//depending on settings
	return FAIL;
}



enum detectorSettings setSettings(enum detectorSettings sett, int imod){
	//template setSettings() from mcb_funcs.c
	//reads the dac registers from fpga to confirm which settings, if weird, undefined

	return OK;
}

int startStateMachine(){
	//template startStateMachine() from firmware_funcs.c
	/*
	fifoReset();
	now_ptr=(char*)ram_values;
	//send start acquisition to fpga
	 */
	return FAIL;
}


int stopStateMachine(){
	//template stopStateMachine() from firmware_funcs.c
	// send stop to fpga
	//if status = busy after 500us, return FAIL
	return FAIL;
}


int startReadOut(){
	//template startReadOut() from firmware_funcs.c
	//send fpga start readout
	return FAIL;
}


enum runStatus getRunStatus(){
	//template runState() from firmware_funcs.c
	//get status from fpga
	return ERROR;
}


char *readFrame(int *ret, char *mess){
	//template fifo_read_event() from firmware_funcs.c
	//checks if state machine running and if fifo has data(look_at_me_reg) and accordingly reads frame
	// memcpy(now_ptr, values, dataBytes);
	//returns ptr to values
	return NULL;
}


int64_t setTimer(enum timerIndex ind, int64_t val){
	//template setDelay() from firmware_funcs.c
	//writes to reg
	//FRAME_NUMBER
	//ACQUISITION_TIME
	//FRAME_PERIOD
	//DELAY_AFTER_TRIGGER
	//GATES_NUMBER
	//PROBES_NUMBER
	//CYCLES_NUMBER
	return 0;
}


int64_t getTimeLeft(enum timerIndex ind){
	//template getDelay() from firmware_funcs.c
	//reads from reg
	//FRAME_NUMBER
	//ACQUISITION_TIME
	//FRAME_PERIOD
	//DELAY_AFTER_TRIGGER
	//GATES_NUMBER
	//PROBES_NUMBER
	//CYCLES_NUMBER
	return -1;
}


int setDynamicRange(int dr){
	//template setDynamicRange() from firmware_funcs.c
	return 0;
}


enum readOutFlags setReadOutFlags(enum readOutFlags val){
	//template setStoreInRAM from firmware_funcs.c
	return -1;
}




int setROI(int n, ROI arg[], int *retvalsize, int *ret){
	return FAIL;
}



int setSpeed(enum speedVariable arg, int val){
	//template setClockDivider() from firmware_funcs.c
	//CLOCK_DIVIDER
	//WAIT_STATES
	//SET_SIGNAL_LENGTH
	//TOT_CLOCK_DIVIDER
	//TOT_DUTY_CYCLE

	//returns eg getClockDivider from firmware_funcs.c
	return 0;
}



int executeTrimming(enum trimMode mode, int par1, int par2, int imod){
	// template  trim_with_noise from trimming_funcs.c
	return FAIL;
}




int configureMAC(int ipad, long long int imacadd, long long int iservermacadd, int dtb){
	//detector specific.
	return FAIL;
}


int loadImage(enum imageType index, char *imageVals){
	//detector specific.
	return FAIL;
}


int readCounterBlock(int startACQ, char *counterVals){
	//detector specific.
	return FAIL;
}

int resetCounterBlock(int startACQ){
	//detector specific.
	return FAIL;
}

int startReceiver(int d){

	return 0;
}

int calibratePedestal(int frames){

	return 0;
}

int calculateDataBytes(){
	return 0;
}

int getTotalNumberOfChannels(){return 0;}
int getTotalNumberOfChips(){return 0;}
int getTotalNumberOfModules(){return 0;}
int getNumberOfChannelsPerChip(){return 0;}
int getNumberOfChannelsPerModule(){return 0;}
int getNumberOfChipsPerModule(){return 0;}
int getNumberOfDACsPerModule(){return 0;}
int getNumberOfADCsPerModule(){return 0;}







enum externalSignalFlag getExtSignal(int signalindex){
	//template getExtSignal from firmware_funcs.c
	//return signals[signalindex];
	return -1;
}





enum externalSignalFlag setExtSignal(int signalindex,  enum externalSignalFlag flag){
	//template setExtSignal from firmware_funcs.c

	//in short..sets signals array, checks if agrees with timing mode, writes to fpga reg, calls synchronization and then settiming
	/*
	if (signalindex>=0 && signalindex<4) {
		signals[signalindex]=flag;
#ifdef VERBOSE
		printf("settings signal variable number %d to value %04x\n", signalindex, signals[signalindex]);
#endif
		// if output signal, set it!
		switch (flag) {
		case GATE_IN_ACTIVE_HIGH:
		case GATE_IN_ACTIVE_LOW:
			if (timingMode==GATE_FIX_NUMBER || timingMode==GATE_WITH_START_TRIGGER)//timingMode = AUTO_TIMING by default and is set in setTiming()
				setFPGASignal(signalindex,flag);	//not implemented here, checks if flag within limits and writes to fpga reg
			else
				setFPGASignal(signalindex,SIGNAL_OFF);
			break;
		case TRIGGER_IN_RISING_EDGE:
		case TRIGGER_IN_FALLING_EDGE:
			if (timingMode==TRIGGER_EXPOSURE || timingMode==GATE_WITH_START_TRIGGER)
				setFPGASignal(signalindex,flag);
			else
				setFPGASignal(signalindex,SIGNAL_OFF);
			break;
		case RO_TRIGGER_IN_RISING_EDGE:
		case RO_TRIGGER_IN_FALLING_EDGE:
			if (timingMode==BURST_TRIGGER)
				setFPGASignal(signalindex,flag);
			else
				setFPGASignal(signalindex,SIGNAL_OFF);
			break;
		case MASTER_SLAVE_SYNCHRONIZATION:
			setSynchronization(syncMode);//syncmode = NO_SYNCHRONIZATION by default and set with this function
			break;
		default:
			setFPGASignal(signalindex,mode);
		}

		setTiming(GET_EXTERNAL_COMMUNICATION_MODE);
	}
	 */
	return getExtSignal(signalindex);
}






enum externalCommunicationMode setTiming( enum externalCommunicationMode arg){
	//template setTiming from firmware_funcs.c
	//template getFPGASignal from firmware_funcs.c


	//getFPGASignal(signalindex) used later on in this fucntion
	//gets flag from fpga reg, checks if flag within limits,
	//if( flag=SIGNAL_OFF and signals[signalindex]==MASTER_SLAVE_SYNCHRONIZATION), return -1, (ensures masterslaveflag !=off now)
	//else return flag

	int ret=GET_EXTERNAL_COMMUNICATION_MODE;
	//sets timingmode variable
	//ensures that the signals are in acceptance with timing mode and according sets the timing mode
	/*
	int g=-1, t=-1, rot=-1;

	int i;

	switch (ti) {
	case AUTO_TIMING:
		timingMode=ti;
		// disable all gates/triggers in except if used for master/slave synchronization
		for (i=0; i<4; i++) {
			if (getFPGASignal(i)>0 && getFPGASignal(i)<GATE_OUT_ACTIVE_HIGH && signals[i]!=MASTER_SLAVE_SYNCHRONIZATION)
				setFPGASignal(i,SIGNAL_OFF);
		}
		break;

	case   TRIGGER_EXPOSURE:
		timingMode=ti;
		// if one of the signals is configured to be trigger, set it and unset possible gates
		for (i=0; i<4; i++) {
			if (signals[i]==TRIGGER_IN_RISING_EDGE ||  signals[i]==TRIGGER_IN_FALLING_EDGE)
				setFPGASignal(i,signals[i]);
			else if (signals[i]==GATE_IN_ACTIVE_HIGH || signals[i]==GATE_IN_ACTIVE_LOW)
				setFPGASignal(i,SIGNAL_OFF);
			else if (signals[i]==RO_TRIGGER_IN_RISING_EDGE ||  signals[i]==RO_TRIGGER_IN_FALLING_EDGE)
				setFPGASignal(i,SIGNAL_OFF);
		}
		break;



	case  TRIGGER_READOUT:
		timingMode=ti;
		// if one of the signals is configured to be trigger, set it and unset possible gates
		for (i=0; i<4; i++) {
			if (signals[i]==RO_TRIGGER_IN_RISING_EDGE ||  signals[i]==RO_TRIGGER_IN_FALLING_EDGE)
				setFPGASignal(i,signals[i]);
			else if (signals[i]==GATE_IN_ACTIVE_HIGH || signals[i]==GATE_IN_ACTIVE_LOW)
				setFPGASignal(i,SIGNAL_OFF);
			else if (signals[i]==TRIGGER_IN_RISING_EDGE ||  signals[i]==TRIGGER_IN_FALLING_EDGE)
				setFPGASignal(i,SIGNAL_OFF);
		}
		break;

	case GATE_FIX_NUMBER:
		timingMode=ti;
		// if one of the signals is configured to be trigger, set it and unset possible gates
		for (i=0; i<4; i++) {
			if (signals[i]==RO_TRIGGER_IN_RISING_EDGE ||  signals[i]==RO_TRIGGER_IN_FALLING_EDGE)
				setFPGASignal(i,SIGNAL_OFF);
			else if (signals[i]==GATE_IN_ACTIVE_HIGH || signals[i]==GATE_IN_ACTIVE_LOW)
				setFPGASignal(i,signals[i]);
			else if (signals[i]==TRIGGER_IN_RISING_EDGE ||  signals[i]==TRIGGER_IN_FALLING_EDGE)
				setFPGASignal(i,SIGNAL_OFF);
		}
		break;


	case GATE_WITH_START_TRIGGER:
		timingMode=ti;
		for (i=0; i<4; i++) {
			if (signals[i]==RO_TRIGGER_IN_RISING_EDGE ||  signals[i]==RO_TRIGGER_IN_FALLING_EDGE)
				setFPGASignal(i,SIGNAL_OFF);
			else if (signals[i]==GATE_IN_ACTIVE_HIGH || signals[i]==GATE_IN_ACTIVE_LOW)
				setFPGASignal(i,signals[i]);
			else if (signals[i]==TRIGGER_IN_RISING_EDGE ||  signals[i]==TRIGGER_IN_FALLING_EDGE)
				setFPGASignal(i,signals[i]);
		}
		break;

	default:
		;

	}


	for (i=0; i<4; i++) {
		if (signals[i]!=MASTER_SLAVE_SYNCHRONIZATION) {
			if (getFPGASignal(i)==RO_TRIGGER_IN_RISING_EDGE ||  getFPGASignal(i)==RO_TRIGGER_IN_FALLING_EDGE)
				rot=i;
			else if (getFPGASignal(i)==GATE_IN_ACTIVE_HIGH || getFPGASignal(i)==GATE_IN_ACTIVE_LOW)
				g=i;
			else if (getFPGASignal(i)==TRIGGER_IN_RISING_EDGE ||  getFPGASignal(i)==TRIGGER_IN_FALLING_EDGE)
				t=i;
		}
	}


	if (g>=0 && t>=0 && rot<0) {
		ret=GATE_WITH_START_TRIGGER;
	} else if (g<0 && t>=0 && rot<0) {
		ret=TRIGGER_EXPOSURE;
	} else if (g>=0 && t<0 && rot<0) {
		ret=GATE_FIX_NUMBER;
	} else if (g<0 && t<0 && rot>0) {
		ret=TRIGGER_READOUT;
	} else if (g<0 && t<0 && rot<0) {
		ret=AUTO_TIMING;
	}

	 */
	return ret;
}



enum masterFlags setMaster(enum masterFlags arg){
	//template setMaster from firmware_funcs.c
	/*
	int i;
	switch(f) {
	case NO_MASTER:
		// switch of gates or triggers
		masterMode=NO_MASTER;
		for (i=0; i<4; i++) {
			if (signals[i]==MASTER_SLAVE_SYNCHRONIZATION) {
				setFPGASignal(i,SIGNAL_OFF);
			}
		}
		break;
	case IS_MASTER:
		// configure gate or trigger out
		masterMode=IS_MASTER;
		for (i=0; i<4; i++) {
			if (signals[i]==MASTER_SLAVE_SYNCHRONIZATION) {
				switch (syncMode) {
				case NO_SYNCHRONIZATION:
					setFPGASignal(i,SIGNAL_OFF);
					break;
				case MASTER_GATES:
					setFPGASignal(i,GATE_OUT_ACTIVE_HIGH);
					break;
				case MASTER_TRIGGERS:
					setFPGASignal(i,TRIGGER_OUT_RISING_EDGE);
					break;
				case SLAVE_STARTS_WHEN_MASTER_STOPS:
					setFPGASignal(i,RO_TRIGGER_OUT_RISING_EDGE);
					break;
				default:
					;
				}
			}
		}
		break;
	case IS_SLAVE:
		// configure gate or trigger in
		masterMode=IS_SLAVE;
		for (i=0; i<4; i++) {
			if (signals[i]==MASTER_SLAVE_SYNCHRONIZATION) {
				switch (syncMode) {
				case NO_SYNCHRONIZATION:
					setFPGASignal(i,SIGNAL_OFF);
					break;
				case MASTER_GATES:
					setFPGASignal(i,GATE_IN_ACTIVE_HIGH);
					break;
				case MASTER_TRIGGERS:
					setFPGASignal(i,TRIGGER_IN_RISING_EDGE);
					break;
				case SLAVE_STARTS_WHEN_MASTER_STOPS:
					setFPGASignal(i,TRIGGER_IN_RISING_EDGE);
					break;
				default:
					;
				}
			}
		}
		break;
	default:
		//do nothing
		;
	}

	switch(masterMode) {
	case NO_MASTER:
		return NO_MASTER;


	case IS_MASTER:
		for (i=0; i<4; i++) {
			if (signals[i]==MASTER_SLAVE_SYNCHRONIZATION) {
				switch (syncMode) {
				case NO_SYNCHRONIZATION:
					return IS_MASTER;
				case MASTER_GATES:
					if (getFPGASignal(i)==GATE_OUT_ACTIVE_HIGH)
						return IS_MASTER;
					else
						return NO_MASTER;
				case MASTER_TRIGGERS:
					if (getFPGASignal(i)==TRIGGER_OUT_RISING_EDGE)
						return IS_MASTER;
					else
						return NO_MASTER;
				case SLAVE_STARTS_WHEN_MASTER_STOPS:
					if (getFPGASignal(i)==RO_TRIGGER_OUT_RISING_EDGE)
						return IS_MASTER;
					else
						return NO_MASTER;
				default:
					return NO_MASTER;
				}

			}
		}

	case IS_SLAVE:
		for (i=0; i<4; i++) {
			if (signals[i]==MASTER_SLAVE_SYNCHRONIZATION) {
				switch (syncMode) {
				case NO_SYNCHRONIZATION:
					return IS_SLAVE;
				case MASTER_GATES:
					if (getFPGASignal(i)==GATE_IN_ACTIVE_HIGH)
						return IS_SLAVE;
					else
						return NO_MASTER;
				case MASTER_TRIGGERS:
				case SLAVE_STARTS_WHEN_MASTER_STOPS:
					if (getFPGASignal(i)==TRIGGER_IN_RISING_EDGE)
						return IS_SLAVE;
					else
						return NO_MASTER;
				default:
					return NO_MASTER;
				}

			}
		}

	}
	 */

	return NO_MASTER;
}



enum synchronizationMode setSynchronization(enum synchronizationMode arg){
	/*
	int i;

	switch(s) {
	case NO_SYNCHRONIZATION:
		syncMode=NO_SYNCHRONIZATION;
		for (i=0; i<4; i++) {
			if (signals[i]==MASTER_SLAVE_SYNCHRONIZATION) {
				setFPGASignal(i,SIGNAL_OFF);
			}
		}
		break;
		// disable external signals?
	case MASTER_GATES:
		// configure gate in or out
		syncMode=MASTER_GATES;
		for (i=0; i<4; i++) {
			if (signals[i]==MASTER_SLAVE_SYNCHRONIZATION) {
				if (masterMode==IS_MASTER)
					setFPGASignal(i,GATE_OUT_ACTIVE_HIGH);
				else if (masterMode==IS_SLAVE)
					setFPGASignal(i,GATE_IN_ACTIVE_HIGH);
			}
		}

		break;
	case MASTER_TRIGGERS:
		// configure trigger in or out
		syncMode=MASTER_TRIGGERS;
		for (i=0; i<4; i++) {
			if (signals[i]==MASTER_SLAVE_SYNCHRONIZATION) {
				if (masterMode==IS_MASTER)
					setFPGASignal(i,TRIGGER_OUT_RISING_EDGE);
				else if (masterMode==IS_SLAVE)
					setFPGASignal(i,TRIGGER_IN_RISING_EDGE);
			}
		}
		break;


	case SLAVE_STARTS_WHEN_MASTER_STOPS:
		// configure trigger in or out
		syncMode=SLAVE_STARTS_WHEN_MASTER_STOPS;
		for (i=0; i<4; i++) {
			if (signals[i]==MASTER_SLAVE_SYNCHRONIZATION) {
				if (masterMode==IS_MASTER)
					setFPGASignal(i,RO_TRIGGER_OUT_RISING_EDGE);
				else if (masterMode==IS_SLAVE)
					setFPGASignal(i,TRIGGER_IN_RISING_EDGE);
			}
		}
		break;


	default:
		//do nothing
		;
	}

	switch (syncMode) {

	case NO_SYNCHRONIZATION:
		return NO_SYNCHRONIZATION;

	case MASTER_GATES:

		for (i=0; i<4; i++) {
			if (signals[i]==MASTER_SLAVE_SYNCHRONIZATION) {
				if (masterMode==IS_MASTER && getFPGASignal(i)==GATE_OUT_ACTIVE_HIGH)
					return MASTER_GATES;
				else if (masterMode==IS_SLAVE && getFPGASignal(i)==GATE_IN_ACTIVE_HIGH)
					return MASTER_GATES;
			}
		}
		return NO_SYNCHRONIZATION;

	case MASTER_TRIGGERS:
		for (i=0; i<4; i++) {
			if (signals[i]==MASTER_SLAVE_SYNCHRONIZATION) {
				if (masterMode==IS_MASTER && getFPGASignal(i)==TRIGGER_OUT_RISING_EDGE)
					return MASTER_TRIGGERS;
				else if (masterMode==IS_SLAVE && getFPGASignal(i)==TRIGGER_IN_RISING_EDGE)
					return MASTER_TRIGGERS;
			}
		}
		return NO_SYNCHRONIZATION;

	case SLAVE_STARTS_WHEN_MASTER_STOPS:
		for (i=0; i<4; i++) {
			if (signals[i]==MASTER_SLAVE_SYNCHRONIZATION) {
				if (masterMode==IS_MASTER &&	getFPGASignal(i)==RO_TRIGGER_OUT_RISING_EDGE)
					return SLAVE_STARTS_WHEN_MASTER_STOPS;
				else if (masterMode==IS_SLAVE && getFPGASignal(i)==TRIGGER_IN_RISING_EDGE)
					return SLAVE_STARTS_WHEN_MASTER_STOPS;
			}
		}
		return NO_SYNCHRONIZATION;

	default:
		return NO_SYNCHRONIZATION;

	}


	 */
	return NO_SYNCHRONIZATION;
}



#endif
