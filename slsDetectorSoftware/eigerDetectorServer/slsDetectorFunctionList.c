#ifdef SLS_DETECTOR_FUNCTION_LIST


#include <stdio.h>
#include <string.h>


#include "slsDetectorFunctionList.h"
#include "svnInfoEiger.h"
#include "EigerHighLevelFunctions.c"


enum detectorSettings thisSettings = STANDARD;
//static const string dacNames[16] = {"Svp","Svn","Vtr","Vrf","Vrs","Vtgstv","Vcmp_ll","Vcmp_lr","Cal","Vcmp_rl","Vcmp_rr","Rxb_rb","Rxb_lb","Vcp","Vcn","Vis"};


int initDetector(){
  printf("EIGER 10\n");
  return 1;
}


int setNMod(int nm, enum dimension dim){
	return 1;
}



int getNModBoard(enum dimension arg){
	return 1;
}



int64_t getModuleId(enum idMode arg, int imod){
  /*
	switch(arg){
	case MODULE_SERIAL_NUMBER:
		return getDetectorNumber();
	case MODULE_FIRMWARE_VERSION:
		return FIRMWAREREV;
	default:
		break;
	}
  */
	return -1;
}




int64_t getDetectorId(enum idMode arg){
	int64_t retval = -1;
	/*
	switch(arg){
	case DETECTOR_SERIAL_NUMBER:
		retval =  getDetectorMAC();
		break;
	case DETECTOR_FIRMWARE_VERSION:
		return FIRMWAREREV;
	case DETECTOR_SOFTWARE_VERSION:
		retval= SVNREV;
		retval= (retval <<32) | SVNDATE;
		break;
	default:
		break;
	}
	*/
	return retval;
}



int getDetectorNumber(){
  /*
	char output[255]="";
	int res=0;
	FILE* sysFile = popen("hostname", "r");
	fgets(output, sizeof(output), sysFile);
	pclose(sysFile);
	sscanf(output,"%x",&res);
	return res;
  */
  return 0;
}


u_int64_t  getDetectorMAC() {
  /*
	char output[255],mac[255]="";
	u_int64_t res=0;
	FILE* sysFile = popen("ifconfig eth0 | grep HWaddr | cut -d \" \" -f 11", "r");
	fgets(output, sizeof(output), sysFile);
	pclose(sysFile);
	//getting rid of ":"
	char * pch;
	pch = strtok (output,":");
	while (pch != NULL){
		strcat(mac,pch);
		pch = strtok (NULL, ":");
	}
	sscanf(mac,"%llx",&res);
	printf("mac:%llx\n",res);
	return res;
  */
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






int setDAC(enum detDacIndex ind, int val, int imod){
	char iname[10];
	strcpy(iname,EigerGetDACName((int)ind));
//#ifdef VERBOSE
	printf("Setting dac %d: %s to %d mV\n",ind, iname,val);
//#endif
	if(val >= 0)
		EigerSetDAC(iname,val/1000);

	return (EigerGetDAC(iname)*1000);
}



int getADC(enum detDacIndex ind,  int imod){
	//get adc value
	return 0;
}



int setModule(sls_detector_module myMod){
  
  /*
#ifdef VERBOSE
	printf("Setting module with settings %d\n",myMod.reg);
#endif

	//int nchip = myMod.nchip;
	//int nchan = (myMod.nchan)/nchip;
	int i;

	for(i=0;i<myMod.ndac;i++)
		setDAC((detDacIndex)i,myMod.dacs[i],myMod.module);

	thisSettings = (detectorSettings)myMod.reg;
  */
  return 0;
}

int getModule(sls_detector_module *myMod){
  /*
	int i;
	for(i=0;i<myMod->ndac;i++)
		myMod->dacs[i]= dacvalues[i];

	//template getModulebyNumber() from mcb_funcs.c
	*/
	return OK;
}

int getThresholdEnergy(int imod){
	printf("Threshold energy: %d\n",EigerGetPhotonEnergy());
	return EigerGetPhotonEnergy();
}


int setThresholdEnergy(int thr, int imod){
	printf("Setting threshold energy:%d\n",thr);
	EigerSetPhotonEnergy(thr);
	return EigerGetPhotonEnergy();
}



enum detectorSettings setSettings(enum detectorSettings sett, int imod){
	//template setSettings() from mcb_funcs.c
	//reads the dac registers from fpga to confirm which settings, if weird, undefined

	return thisSettings;
}

int startStateMachine(){
	printf("Going to start acquisition\n");
	EigerStartAcquisition();
	return OK;
}


int stopStateMachine(){
	printf("Going to stop acquisition\n");
	EigerStopAcquisition();
	return OK;
}


int startReadOut(){
	//template startReadOut() from firmware_funcs.c
	//send fpga start readout
	return FAIL;
}


enum runStatus getRunStatus(){
	int i = EigerRunStatus();
	printf("Status:%d ",i);
	if(i== 0){
		printf(" returning %d\n",IDLE);
		return IDLE;
	}else{
		printf(" returning %d\n",RUNNING);
		return RUNNING;
	}
}


char *readFrame(int *ret, char *mess){
	//template fifo_read_event() from firmware_funcs.c
	//checks if state machine running and if fifo has data(look_at_me_reg) and accordingly reads frame
	// memcpy(now_ptr, values, dataBytes);
	//returns ptr to values
	return NULL;
}


int64_t setTimer(enum timerIndex ind, int64_t val){

	switch(ind){
	case FRAME_NUMBER:
		if(val >= 0){
			printf(" Setting number of frames: %d\n",(unsigned int)val);
			EigerSetNumberOfExposures((unsigned int)val);
		}return EigerGetNumberOfExposures();
	case ACQUISITION_TIME:
		if(val >= 0){
			printf(" Setting exp time: %fs\n",val/(1E9));
			EigerSetExposureTime(val/(1E9));
		}return EigerGetExposureTime();
	case FRAME_PERIOD:
		if(val >= 0){
			printf(" Setting acq period: %fs\n",val/(1E9));
			EigerSetExposurePeriod(val/(1E9));
		}return (EigerGetExposurePeriod()*(1E9));
/*	case DELAY_AFTER_TRIGGER:
		if(val >= 0)
			EigerSetNumberOfExposures((unsigned int)val);
		return EigerGetNumberOfExposures();
	case GATES_NUMBER:
		if(val >= 0)
			EigerSetNumberOfExposures((unsigned int)val);
		return EigerGetNumberOfExposures();
	case PROBES_NUMBER:
		if(val >= 0)
			EigerSetNumberOfExposures((unsigned int)val);
		return EigerGetNumberOfExposures();*/
	case CYCLES_NUMBER:
		if(val >= 0){
			printf(" Setting number of triggers: %d\n",(unsigned int)val);
			EigerSetNumberOfExposureSeries((unsigned int)val);
		}return EigerGetNumberOfExposureSeries();
	default:
		printf("unknown timer index: %d\n",ind);
		break;
	}

	return -1;
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
	if(dr > 0){
		printf(" Setting dynamic range: %d\n",dr);
		EigerSetDynamicRange(dr);
	}return EigerGetDynamicRange();
}



enum readOutFlags setReadOutFlags(enum readOutFlags val){
	//template setStoreInRAM from firmware_funcs.c
  return GET_READOUT_FLAGS;
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



int calculateDataBytes(){
	return 0;
}

int getTotalNumberOfChannels(){return 1;};//NCHIP*NCHAN*nModBoard;}
int getTotalNumberOfChips(){return 1;};//NCHIP*nModBoard;}
int getTotalNumberOfModules(){return 1;}//nModBoard;}
int getNumberOfChannelsPerChip(){return  1;}//NCHAN;}
int getNumberOfChannelsPerModule(){return  1;}//NCHAN*NCHIP;}
int getNumberOfChipsPerModule(){return  1;}//NCHIP;}
int getNumberOfDACsPerModule(){return  1;}//NDAC;}
int getNumberOfADCsPerModule(){return  1;}//NADC;}







enum externalSignalFlag getExtSignal(int signalindex){
	//template getExtSignal from firmware_funcs.c
	//return signals[signalindex];
	return GET_EXTERNAL_SIGNAL_FLAG;
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
			if (timingMode==TRIGGER_READOUT)
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

	enum externalCommunicationMode ret=GET_EXTERNAL_COMMUNICATION_MODE;
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
