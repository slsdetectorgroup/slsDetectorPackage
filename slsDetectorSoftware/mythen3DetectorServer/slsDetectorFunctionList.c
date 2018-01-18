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



int setDacRegister(int dacnum,int dacvalue) {

  bus_w(DAC_NUM_REG, dacnum);
  bus_w(DAC_VAL_REG, dacvalue);
  bus_w(DAC_NUM_REG, dacnum | (1<<16));
  bus_w(DAC_NUM_REG, dacnum);
  printf("Wrote dac register value %d address %d\n",bus_r(DAC_VAL_REG),bus_r(DAC_NUM_REG)) ;
  return getDacRegister(dacnum);
}



int getDacRegister(int dacnum) {
  
  bus_w(DAC_NUM_REG, dacnum);
  printf("READ dac register value %d address %d\n",(int16_t)bus_r(DAC_VAL_OUT_REG),bus_r(DAC_NUM_REG)) ;
  return (int16_t)bus_r(DAC_VAL_OUT_REG);  
}



int nextDac(){ 
	return dacSPI(0xf<<DAC_CMD_OFF);
}



int dacSPI(int codata) {
  u_int32_t offw;
  int valw, vv; 
  int i, ddx,cdx;

  ddx=0; cdx=1;

  offw=DAC_REG;
  valw=bus_r(offw); 
  // codata=((cmd&0xf)<<DAC_CMD_OFF)|((val<<4)&0xfff0);
  printf("%08x\n",codata);
  valw=bus_r(offw);

  for (i=1;i<33;i++) {    

    if ((codata&(1<<(32-i)))) {
      vv=valw|(0x1<<ddx);
    }  else {
      vv=valw&(~(0x1<<ddx));
    }
    printf("%x",vv&0x1);
    bus_w16(offw,vv);//data
    bus_w16(offw,vv|(0x1<<cdx));//clkup
    bus_w16(offw,vv&(~(0x1<<cdx))); //cldwn

  } 
  printf("\n");
  return 1;
}



int setThisDac(int dacnum,  int dacvalue){ 

	u_int32_t codata, cmd; 
	int dacch;

	dacch=dacnum%8; 

	if (dacvalue>=0) {
		cmd=0x3;
  	} 
  	else if (dacvalue==-100) {
    	cmd=0x4;
  	}
  	codata=cmd<<DAC_CMD_OFF;
  	codata|=(dacch&0xf)<<16;
  	codata|=(dacvalue&0xfff)<<4;

  	return dacSPI(codata);
}


double setDAC(enum DACINDEX ind, double val, int imod, int mV){
	// if mV==1: retval1 is returned in mV
	// val must be DACu !!!

	//template initDACbyIndexDACU from mcb_funcs.c

	//check that slsDetectorServer_funcs.c set_dac() has all the specific dac enums
	//set dac and write to a register in fpga to remember dac value when server restarts


	//find out which dac is on the board (convert dacIndex into something else) -> conversion done in slsDetectorServer_funcs.c-> set_dac()
	
	//if normal dac:

	int dacval = val; // in DAC
	u_int32_t offw;
	u_int32_t ichip; // DAC-chip (3 chips with each 8 DACs)
	u_int16_t valw; 
	int i,ddx,csdx,cdx; // ddx=data, cdx=clk, csdx=chipselect

	//select dac-chip:
	if  (myDetectorType==JUNGFRAUCTB)
    	ichip=2-ind/8;
    	printf("This is a CTB\n");
    else 
    	ichip=ind/8;
    	printf("This is not a CTB\n");

	// if(val>0 && mV){ // convert to DACu, if val is given in mV
	// 	dacval = val * 4095 / 2500; // convert to DAC
	if(dacval<0 || dacval>4095){
		dacval = -1;
		printf("The DAC is out of range! Error!");
		return -1;
	}
	// }

	else if(dacval>=0 && dacval<=4095){
		printf("Setting of DAC %d to %d DACunits",ind,dacval);
		ddx=0; // data is first bit in DAC_REG
		cdx=1; // clk is 2nd bit in DAC_REG
		if  (myDetectorType==JUNGFRAUCTB) 
     		csdx=2; 
    	else 
     		csdx=ichip+2;

     	//setting int reference 
	    offw=DAC_REG;
	    valw=bus_r(offw)|0xff; // alles (ddx,cdx,csdx) auf 1 setzen (for START)
	    bus_w(offw,(valw)); // start point
		//chip select down:
		valw=((valw&(~(0x1<<csdx))));
		bus_w(offw,valw); 
		// clk down:
		valw=(valw&(~(0x1<<cdx)));
		bus_w(offw,valw); //clk dwn

		// change DAC only in the required dac-chip, skip all others:
		if  (myDetectorType==JUNGFRAUCTB) {
     		for (i=0; i<ichip; i++) { // skip first part
	    		nextDac();
	    		printf("next DAC\n");
      		}
    	}
    	// now you are at the dac-chip you want to change:
    	setThisDac(ind,dacvalue);
    	// skip the rest of the dac-chips to arrive at the end:
    	if  (myDetectorType==JUNGFRAUCTB) {
     		for (i=ichip+1; i<N_DAC/8; i++) {
	     		nextDac();
	      		printf("next DAC\n");
     		}
    	}
		//nextdac, setthisdac, nextdac: done
		//chipselect up:
		valw=bus_r(offw);
    	valw=(valw&(~(0x1<<cdx)));bus_w(offw,valw); //clk down
    	valw=(valw|(0x1<<csdx));
   	 	bus_w(offw,valw); //csdx up
    
    	valw=bus_r(offw)|0xff; // set all to 1 (up)
    	bus_w(offw,(valw)); // stop point =start point of course */

		//write dacvalreg:
		setDacRegister(ind,dacvalue);
	}
	//convert return value into mV if necessary
	int retval = getDacRegister(dacnum);
	double retval1; // only to return

	if(mV){ // return the DACvalue in mV
		retval1 = retval * 2500. / 4095.;
	}
	else{ // return the DACvalue in DACu
		retval1 = retval;
	}

	return retval1;
}

int setPower(int ind, int val) {
	// check dacindex!
  int dacindex=-1;
  int dacval=-1;
  int pwrindex=-1;
  int retval=-1;
  int retval1=-1;

  u_int32_t preg;

  int vchip=2700-(getDacRegister(19)*1000)/4095;
  int vmax=vchip-200;
  int vmin=600;
  
  printf("---------------------------------------------Current V_Chip is %d mV\n",vchip);

  switch (ind) {

  case V_POWER_CHIP:
    dacindex=19;
    pwrindex=-1;
    break;
  case V_POWER_A:
    dacindex=22;
    pwrindex=1;
    break;
  case V_POWER_B:
    dacindex=21;
    pwrindex=2;
    break;
  case V_POWER_C:
    dacindex=20;
    pwrindex=3;
    break;
  case V_POWER_D:
    dacindex=18;
    pwrindex=4;
    break;
  case V_POWER_IO:
    dacindex=23;
    pwrindex=0;
    break;
  case V_LIMIT:
    dacindex=-1;
    pwrindex=-1;
    break;
  default:
    pwrindex=-1;
    dacindex=-1;
  }

  if (val==-1) {
    printf("get\n");
    dacval=-1;
  } else {
    if (pwrindex>=0) {
      printf("vpower\n");
	dacval=((vmax-val)*4095)/(vmax-vmin);
	if (dacval<0)
	  dacval=0;
	if (dacval>4095)
	  dacval=-100;
	if (val==-100)
	  dacval=-100;
     
    
    } else if (dacindex>=0) {
      printf("vchip\n");
	dacval=((2700-val)*4095)/1000;
	if (dacval<0)
	  dacval=0;
	if (dacval>4095)
	  dacval=4095;
      
    } else {
      vLimit=val;
      printf("vlimit %d\n",vLimit );
    }
      
  }

  if (pwrindex>=0 && val!=-1) {
    preg=bus_r(POWER_ON_REG);
    printf("power reg is %08x\n",bus_r(POWER_ON_REG));
    printf("Switching off power %d\n", pwrindex);
    bus_w(POWER_ON_REG,preg&(~(1<<(16+pwrindex))));
    setDAC(dacindex,-100);
    printf("power reg is %08x\n",bus_r(POWER_ON_REG));
    retval=0;
  }
  
  if (dacindex>0 && dacval!=-100) {
    
    printf("Setting power %d to %d mV\n",ind,val);
    printf("Setting DAC %d to value %d\n",dacindex,dacval);
    retval=setDac(dacindex,dacval);
    if (pwrindex>=0 && dacval>=0 ) {
      preg=bus_r(POWER_ON_REG);
      printf("power reg is %08x\n",bus_r(POWER_ON_REG));
      printf("Switching on power %d\n", pwrindex);
      bus_w(POWER_ON_REG,preg|((1<<(16+pwrindex))));
      printf("power reg is %08x\n",bus_r(POWER_ON_REG));
    }
  }
  
  if (pwrindex>=0) {
    if (bus_r(POWER_ON_REG)&(1<<(16+pwrindex))){
      vmax=2700-(getDacRegister(19)*1000)/4095-200;
      printf("Vchip id %d mV\n",vmax+200);
      retval1=vmax-(retval*(vmax-vmin))/4095;
      printf("Vdac id %d mV\n",retval1);
	if (retval1>vmax)
	  retval1=vmax;
	if (retval1<vmin)
	  retval1=vmin;
	if (retval<0)
	  retval1=retval;
    } else
      retval1=0;
  } else if (dacindex>=0) {
    if (retval>=0) {
      retval1=2700-(retval*1000)/4095;
      printf("Vchip is %d mV\n",vmax);
    } else
      retval1=-1;
  } else {
    printf("Get vlimit %d\n",vLimit);
    retval=vLimit;
    retval1=vLimit;
  }

  return retval1;
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
	//FRAME_NUMBER   --> defined in sls_receiver_defs.h
	//ACQUISITION_TIME
	//FRAME_PERIOD -> how many frames per trigger
	//DELAY_AFTER_TRIGGER
	//GATES_NUMBER
	//PROBES_NUMBER = # counters
	//CYCLES_NUMBER -> how many triggers

	int64_t retval = -1; // return value to check

	switch(ind){ // only change the timer corresponding to ind

	case FRAME_NUMBER:
		if(val >= 0)
			printf("\nSetting #frames: %lld\n",(long long int)val);
		retval = set64BitReg(val,  SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG);
		printf("Getting #frames: %lld\n",(long long int)retval);
		break;

	case ACQUISITION_TIME: // defined in sls_receiver_defs.h
		if(val>=0){
			printf("\n Setting the exposure time: %lld ns \n",(long long int)val);
			val *= (1E-3 * CLK_RUN); // convert from ns to clk-cycles
			// CLK_RUN is defined in slsDetectorServer_defs.h
		}
		retval = set64BitReg(val,SET_EXPTIME_LSB_REG, SET_EXPTIME_MSB_REG )/(1E-3 * CLK_RUN); // set the register to val and read back, convert back to ns
		// SET_EXPTIME_LSB_REG are defined in RegisterDefs.h
		printf("Getting the exposure time: %lld ns \n", (long long int)retval);
		break;

	case FRAME_PERIOD: // how many frames per trigger
		if(val >= 0){
			printf("\nSetting period to %lldns\n",(long long int)val);
			val *= (1E-3 * CLK_SYNC);
		}
		retval = set64BitReg(val, SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG )/ (1E-3 * CLK_SYNC);
		// CLK_SYN is defined in slsDetectorServer_defs.h
		printf("Getting period: %lldns\n", (long long int)retval);
		break;

	case DELAY_AFTER_TRIGGER:
		if(val >= 0){
			printf("\nSetting delay to %lldns\n", (long long int)val);
			val *= (1E-3 * CLK_SYNC);
		}
		retval = set64BitReg(val, SET_DELAY_LSB_REG, SET_DELAY_MSB_REG) / (1E-3 * CLK_SYNC);
		printf("Getting delay: %lldns\n", (long long int)retval);
		break;

	// case PROBES_NUMBER:

	case CYCLES_NUMBER: // how many triggers
		if(val >= 0)
			printf("\nSetting #cycles to %lld\n", (long long int)val);
		retval = set64BitReg(val,  SET_CYCLES_LSB_REG, SET_CYCLES_MSB_REG);
		printf("Getting #cycles: %lld\n", (long long int)retval);
		break;

	default:
		cprintf(RED,"Warning: Timer Index not implemented for this detector: %d\n", ind);
		break;
	}
	return retval;
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
