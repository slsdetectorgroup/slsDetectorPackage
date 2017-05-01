//#define TESTADC
#define TESTADC1


//#define TIMEDBG 
#include "server_defs.h"
#include "firmware_funcs.h"
#include "mcb_funcs.h"
#include "registers_m.h"
#include "gitInfoJungfrau.h"

//#define VERBOSE
//#define VERYVERBOSE


#ifdef SHAREDMEMORY
#include "sharedmemory.h"
#endif

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <sys/time.h>
#include <stdlib.h>  /* exit() */
#include <string.h>  /* memset(), memcpy() */
#include <sys/utsname.h>   /* uname() */
#include <sys/types.h>
#include <sys/socket.h>   /* socket(), bind(),
                         listen(), accept() */
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>  /* fork(), write(), close() */
#include <time.h> 
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


extern enum detectorType myDetectorType;

typedef struct ip_header_struct {
	u_int16_t     ip_len;
	u_int8_t      ip_tos;
	u_int8_t      ip_ihl:4 ,ip_ver:4;
	u_int16_t     ip_offset:13,ip_flag:3;
	u_int16_t     ip_ident;
	u_int16_t     ip_chksum;
	u_int8_t      ip_protocol;
	u_int8_t      ip_ttl;
	u_int32_t     ip_sourceip;
	u_int32_t     ip_destip;
} ip_header;


struct timeval tss,tse,tsss; //for timing




u_int32_t CSP0BASE;
FILE *debugfp, *datafp;
int fr;
int wait_time;
int *fifocntrl;
const int nModY=1;
int nModBoard;
int nModX=NMAXMOD;
int dynamicRange=16;
int nSamples=1;
size_t dataBytes=NMAXMOD*NCHIP*NCHAN*2;
int storeInRAM=0;
int ROI_flag=0;
int adcConfigured=-1;
int ram_size=0;

int64_t totalTime=1;
u_int32_t progressMask=0;
int phase_shift=0;//DEFAULT_PHASE_SHIFT;
int ipPacketSize=DEFAULT_IP_PACKETSIZE;
int udpPacketSize=DEFAULT_UDP_PACKETSIZE;
int clockdivider_exptime = 40;
int clockdivider_fc = 20;
/*
#ifndef NEW_PLL_RECONFIG
u_int32_t clkDivider[2]={32,16};
#else
u_int32_t clkDivider[2]={40,20};
#endif
 */

int32_t clkPhase[2]={0,0};

u_int32_t adcDisableMask=0;

int ififostart, ififostop, ififostep, ififo;

int masterMode=NO_MASTER, syncMode=NO_SYNCHRONIZATION, timingMode=AUTO_TIMING;

enum externalSignalFlag  signals[4]={EXT_SIG_OFF, EXT_SIG_OFF, EXT_SIG_OFF, EXT_SIG_OFF};

char mtdvalue[10];





int mapCSP0(void) {
	printf("Mapping memory\n");
#ifndef VIRTUAL
	int fd;
	fd = open("/dev/mem", O_RDWR | O_SYNC, 0);
	if (fd == -1) {
		printf("\nCan't find /dev/mem!\n");
		return FAIL;
	}
	//printf("/dev/mem opened\n");

	CSP0BASE = (u_int32_t)mmap(0, MEM_SIZE, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, CSP0);
	if (CSP0BASE == (u_int32_t)MAP_FAILED) {
		printf("\nCan't map memmory area!!\n");
		return FAIL;
	}
	//printf("CSP0 mapped\n");

#endif
#ifdef VIRTUAL
	CSP0BASE = malloc(MEM_SIZE);
	printf("memory allocated\n");
#endif
#ifdef SHAREDMEMORY 
	if ( (res=inism(SMSV))<0) {
		printf("error attaching shared memory! %i",res);
		return FAIL;
	}
#endif
	//printf("CSPObase is 0x%08x \n",CSP0BASE);
	printf("CSPOBASE mapped from %08x to %08x\n",CSP0BASE,CSP0BASE+MEM_SIZE);

	printf("statusreg=%08x\n",bus_r(STATUS_REG));
	return OK;
}



void defineGPIOpins(){
	//define the gpio pins
	system("echo 7 > /sys/class/gpio/export");
	system("echo 9 > /sys/class/gpio/export");
	//define their direction
	system("echo in  > /sys/class/gpio/gpio7/direction");
	system("echo out > /sys/class/gpio/gpio9/direction");
}

void resetFPGA(){
	cprintf(BLUE,"\n*** Reseting FPGA ***\n");
	FPGAdontTouchFlash();
	FPGATouchFlash();
	usleep(250*1000);
}

void FPGAdontTouchFlash(){
	//tell FPGA to not touch flash
	system("echo 0 > /sys/class/gpio/gpio9/value");
	//usleep(100*1000);
}

void FPGATouchFlash(){
	//tell FPGA to touch flash to program itself
	system("echo 1 > /sys/class/gpio/gpio9/value");
}

int powerChip (int on){
	if(on != -1){
		if(on){
			printf("\nPowering on the chip\n");
			bus_w(POWER_ON_REG,0x1);
		}
		else{
			printf("\nPowering off the chip\n");
			bus_w(POWER_ON_REG,0x0);
		}
	}

	return bus_r(POWER_ON_REG);
}


void initializeDetector(){
	printf("Initializing Detector\n");

	int i;
	//printf("Bus test... ");
	for (i=0; i<1000000; i++) {
		bus_w(SET_DELAY_LSB_REG, i*100);
		bus_r(FPGA_VERSION_REG);
		if (i*100!=bus_r(SET_DELAY_LSB_REG))
			cprintf(RED,"Bus Test ERROR: wrote 0x%x, read 0x%x\n",i*100,bus_r(SET_DELAY_LSB_REG));
	}
	//printf("Finished\n");

	//confirm the detector type
	if (((bus_r(PCB_REV_REG) & DETECTOR_TYPE_MASK)>>DETECTOR_TYPE_OFFSET) != JUNGFRAU_MODULE_ID){
		cprintf(BG_RED,"This is not a Jungfrau Server (enum:%d)\n",myDetectorType);
		exit(-1);
	}
	cprintf(BLUE,"\n\n********************************************************\n"
			"*********************Jungfrau Server********************\n"
			"********************************************************\n");

	//print version
	cprintf(BLUE,"\n"
			"Firmware Version:\t 0x%x\n"
			"Software Version:\t %llx\n"
			//"F/w-S/w API Version:\t\t %lld\n"
			//"Required Firmware Version:\t %d\n"
			"Fixed Pattern:\t\t 0x%x\n"
			"Board Revision:\t\t 0x%x\n"
			"\n********************************************************\n",
			bus_r(FPGA_VERSION_REG),(long long unsigned int)(((int64_t)SVNREV <<32) | (int64_t)SVNDATE)
			//,sw_fw_apiversion,	REQUIRED_FIRMWARE_VERSION
			,bus_r(FIX_PATT_REG),(bus_r(PCB_REV_REG)&BOARD_REVISION_MASK)
	);


	printf("Resetting PLL\n");
	resetPLL();
	bus_w16(CONTROL_REG, SYNC_RESET);
	bus_w16(CONTROL_REG, 0);
	bus_w16(CONTROL_REG, GB10_RESET_BIT);
	bus_w16(CONTROL_REG, 0);

#ifdef MCB_FUNCS
	initDetector();
#endif
	/*some registers set, which is in common with jungfrau, please check */
	prepareADC();
	/*some registers set, which is in common with jungfrau, please check */
	initDac(0);    initDac(8); //initializes the two dacs


	//set dacs
	printf("Setting Default Dac values\n");
	enum dacNames{VB_COMP,VDD_PROT,VIN_COM,VREF_PRECH,VB_PIXBUF,VB_DS,VREF_DS,VREF_COMP};
	int retval = -1;
	int dacvalues[8][2]={
			{VB_COMP,	1220},
			{VDD_PROT, 	3000},
			{VIN_COM, 	1053},
			{VREF_PRECH,1450},
			{VB_PIXBUF, 750},
			{VB_DS,		1000},
			{VREF_DS,	480},
			{VREF_COMP, 420},
	};
	for(i=0;i<8;++i){
		retval=setDac(dacvalues[i][0], dacvalues[i][1]);
		if(retval!=dacvalues[i][1])
			printf("Error: Setting dac %d failed, wrote %d, read %d\n",dacvalues[i][0],dacvalues[i][1],retval);
	}


	//done from config file
	//printf("\nPowering on the chip\n");
	//bus_w(POWER_ON_REG,0x1);

	/* Only once */
	bus_w(CONFGAIN_REG,0x0);

	printf("Resetting ADC\n");
	writeADC(ADCREG1,0x3); writeADC(ADCREG1,0x0);
	writeADC(ADCREG2,0x40);
	writeADC(ADCREG3,0xf);
	writeADC(ADCREG4,0x3f);
	printf("Configuring Vrefs\n");
	writeADC(ADCREG_VREFS,0x2);
	printf("Setting ADC Inversion\n");// (by trial and error)
	bus_w(ADC_INVERSION_REG,0x453b2a9c);

	adcPipeline(HALFSPEED_ADC_PIPELINE);
	dbitPipeline(HALFSPEED_DBIT_PIPELINE);
	adcPhase(HALFSPEED_ADC_PHASE); //set adc_clock_phase in unit of 1/(52) clock period (by trial and error)

	printf("Reset mem machine fifos\n");
	bus_w(MEM_MACHINE_FIFOS_REG,0x4000);
	bus_w(MEM_MACHINE_FIFOS_REG,0x0);
	printf("Reset run control\n");
	bus_w(MEM_MACHINE_FIFOS_REG,0x0400);
	bus_w(MEM_MACHINE_FIFOS_REG,0x0);
	initSpeedConfGain(HALFSPEED_CONF);
	setSettings(DYNAMICGAIN,-1);



	//Initialization of acquistion parameters
	setFrames(1*1000*1000);
	setTrains(-1);
	setExposureTime(10*1000);
	setPeriod(2*1000*1000);
	setDelay(0);
	setGates(0);


	setTiming(GET_EXTERNAL_COMMUNICATION_MODE);
	setMaster(GET_MASTER);
	setSynchronization(GET_SYNCHRONIZATION_MODE);

}


u_int16_t bus_r16(u_int32_t offset){
	volatile u_int16_t *ptr1;
	ptr1=(u_int16_t*)(CSP0BASE+offset*2);
	return *ptr1;
}

u_int16_t bus_w16(u_int32_t offset, u_int16_t data) {
	volatile u_int16_t  *ptr1;
	ptr1=(u_int16_t*)(CSP0BASE+offset*2);
	*ptr1=data;
	return OK;
}

/** ramType is DARK_IMAGE_REG or GAIN_IMAGE_REG */
u_int16_t ram_w16(u_int32_t ramType, int adc, int adcCh, int Ch, u_int16_t data) { 
	unsigned int adr = (ramType | adc << 8 | adcCh << 5 | Ch );
	// printf("Writing to addr:%x\n",adr);
	return bus_w16(adr,data);
}

/** ramType is DARK_IMAGE_REG or GAIN_IMAGE_REG */
u_int16_t ram_r16(u_int32_t ramType, int adc, int adcCh, int Ch){
	unsigned int adr = (ramType | adc << 8 | adcCh << 5 | Ch );
	//  printf("Reading from addr:%x\n",adr);
	return bus_r16(adr);
}

u_int32_t bus_w(u_int32_t offset, u_int32_t data) {
	volatile  u_int32_t  *ptr1;

	ptr1=(u_int32_t*)(CSP0BASE+offset*2);
	*ptr1=data;

	return OK;
}


u_int32_t bus_r(u_int32_t offset) {
	volatile u_int32_t  *ptr1;
	ptr1=(u_int32_t*)(CSP0BASE+offset*2);
	return *ptr1;
}


int setPhaseShiftOnce(){
	u_int32_t addr, reg;
	int i;
	addr=MULTI_PURPOSE_REG;
	reg=bus_r(addr);
#ifdef VERBOSE
	printf("Multipurpose reg:%x\n",reg);
#endif

	//Checking if it is power on(negative number)
	// if(((reg&0xFFFF0000)>>16)>0){
	//bus_w(addr,0x0);   //clear the reg

	if(reg==0){
		printf("\nImplementing phase shift of %d\n",phase_shift);
		for (i=1;i<phase_shift;i++) {
			bus_w(addr,(INT_RSTN_BIT|ENET_RESETN_BIT|SW1_BIT|PHASE_STEP_BIT));//0x2821
			bus_w(addr,(INT_RSTN_BIT|ENET_RESETN_BIT|(SW1_BIT&~PHASE_STEP_BIT)));//0x2820
		}
#ifdef VERBOSE
		printf("Multipupose reg now:%x\n",bus_r(addr));
#endif
	}
	return OK;
}


int cleanFifo(){
	return OK;
}


int setDAQRegister(){
	return OK;
}


// direct pattern output 
u_int32_t putout(char *s, int modnum) {
	int i;
	u_int32_t pat;
	int addr;

	if (strlen(s)<16) {
		fprintf(stdout," *** putout error: incorrect pattern length ***\n");
		fprintf(stdout," %s \n",s);
		return FAIL;
	}

	pat=0;
	for (i=0;i<16;i++) {
		if (s[i]=='1') pat=pat+(1<<(15-i));
	}
	//addr=DAC_REG+(modnum<<4);
	addr=DAC_REG;//+(modnum<<SHIFTMOD); commented by dhanya
	bus_w(addr, pat);

	return OK;
}


// read direct input 
u_int32_t readin(int modnum) {
	return 0;
}

u_int32_t setPllReconfigReg(u_int32_t reg, u_int32_t val, int trig) {
	u_int32_t vv;
	// printf("*********** pll busy: %08x\n",bus_r(STATUS_REG)&PLL_RECONFIG_BUSY);
	bus_w(PLL_PARAM_REG,val);
	vv=reg<<PLL_CNTR_ADDR_OFF;
	bus_w(PLL_CNTRL_REG,vv);
	usleep(10000);
	bus_w(PLL_CNTRL_REG,vv|(1<<PLL_CNTR_WRITE_BIT) );//15 is trigger for the tap
	bus_w(PLL_CNTRL_REG,vv);
	usleep(10000);

	return val;

}

u_int32_t getPllReconfigReg(u_int32_t reg, int trig) {

	u_int32_t val=reg<<PLL_CNTR_ADDR_OFF;
	u_int32_t vv;
	//  printf("cntrlreg: %08x\n",PLL_CNTRL_REG);
	// printf("wrote: %08x\n",val);
	bus_w(PLL_CNTRL_REG,val);
	//  printf("read: %08x\n",bus_r(PLL_CNTRL_REG));
	usleep(100);
	val=(1<<PLL_CNTR_READ_BIT)|(reg<<PLL_CNTR_ADDR_OFF)|(trig<<15);
	//  printf("wrote: %08x\n",val);
	bus_w(PLL_CNTRL_REG,val);//15 is trigger for the tap
	//  printf("read: %08x\n",bus_r(PLL_CNTRL_REG));
	//  usleep(100);
	val=(reg<<PLL_CNTR_ADDR_OFF);
	//  printf("wrote: %08x\n",val);
	bus_w(PLL_CNTRL_REG,val);
	usleep(100);

	val=0;
	// printf("wrote: %08x\n",val);
	bus_w(PLL_CNTRL_REG,val);

	while(bus_r(STATUS_REG)&PLL_RECONFIG_BUSY) {
		printf("get: reconfig busy");
	}
	return vv;
}


void resetPLL() {
	bus_w(PLL_CNTRL_REG,(1<<PLL_CNTR_RECONFIG_RESET_BIT)|(1<<PLL_CNTR_PLL_RESET_BIT)); //reset PLL and pll reconfig
	usleep(100);
	bus_w(PLL_CNTRL_REG, 0);
}



void configurePll(int i) {
	u_int32_t l=0x0c;
	u_int32_t h=0x0d;
	u_int32_t val;
	int32_t phase=0, inv=0;
	u_int32_t tot;
	u_int32_t odd=1;//0;
	//   printf("PLL reconfig reset\N");   bus_w(PLL_CNTRL_REG,(1<<PLL_CNTR_RECONFIG_RESET_BIT));  usleep(100);  bus_w(PLL_CNTRL_REG, 0);
#ifndef NEW_PLL_RECONFIG
	printf("PLL mode\n");   setPllReconfigReg(PLL_MODE_REG,1,0);
	//  usleep(10000);
#endif

	if (i<2) {
		tot= PLL_VCO_FREQ_MHZ/clockdivider_fc; /* which clock divider?????? Is it called? clean up!! */
		l=tot/2;
		h=l;
		if (tot>2*l) {
			h=l+1;
			odd=1;
		}
		printf("Counter %d: Low is %d, High is %d\n",i, l,h);
		val= (i<<18)| (odd<<17) | l | (h<<8);
		printf("Counter %d, val: %08x\n", i,  val);
		setPllReconfigReg(PLL_C_COUNTER_REG, val,0);
		//  usleep(20);
		//change sync at the same time as
		if (i>0) {
			val= (2<<18)| (odd<<17) | l | (h<<8);
			printf("Counter %d, val: %08x\n", i,  val);
			setPllReconfigReg(PLL_C_COUNTER_REG, val,0);
		}

	} else {
		//  if (mode==1) {
		//  } else {
		printf("phase in %d\n",clkPhase[1]);

		if (clkPhase[1]>0) {
			inv=0;
			phase=clkPhase[1];
		}  else {
			inv=1;
			phase=-1*clkPhase[1];
		}

		printf("phase out %d %08x\n",phase,phase);
		if (inv) {
			val=phase | (1<<16);// |  (inv<<21);
			printf("**************** phase word %08x\n",val);

			//  printf("Phase, val: %08x\n", val);
			setPllReconfigReg(PLL_PHASE_SHIFT_REG,val,0); //shifts counter 0
		} else {


			val=phase ;// |  (inv<<21);
			printf("**************** phase word %08x\n",val);

			//  printf("Phase, val: %08x\n", val);
			setPllReconfigReg(PLL_PHASE_SHIFT_REG,val,0); //shifts counter 0
#ifndef NEW_PLL_RECONFIG
			printf("Start reconfig\n");  setPllReconfigReg(PLL_START_REG, 1,0);

			// bus_w(PLL_CNTRL_REG, 0);
			printf("Status register\n"); getPllReconfigReg(PLL_STATUS_REG,0);
			// sleep(1);

			printf("PLL mode\n");   setPllReconfigReg(PLL_MODE_REG,1,0);
			//  usleep(10000);

#endif
			printf("**************** phase word %08x\n",val);

			val=phase | (2<<16);// |  (inv<<21);
			//  printf("Phase, val: %08x\n", val);
			setPllReconfigReg(PLL_PHASE_SHIFT_REG,val,0); //shifts counter 0
		}
	}
#ifndef NEW_PLL_RECONFIG
	printf("Start reconfig\n");  setPllReconfigReg(PLL_START_REG, 1,0);

	// bus_w(PLL_CNTRL_REG, 0);
	printf("Status register\n"); getPllReconfigReg(PLL_STATUS_REG,0);
	// sleep(1);
#endif 
	//  printf("PLL mode\n");   setPllReconfigReg(PLL_MODE_REG,0,0);
	usleep(10000);
	if (i<2) {
		printf("reset pll\n");
		bus_w(PLL_CNTRL_REG,((1<<PLL_CNTR_PLL_RESET_BIT))); //reset PLL
		usleep(100);
		bus_w(PLL_CNTRL_REG, 0);
	}
}





u_int32_t setClockDivider(int d) {

	enum clkspeed{FULL,HALF,QUARTER};

	if(d!=-1){
		switch(d){
		//stop state machine if running
		if(runBusy())
			stopStateMachine();

		case FULL:
			printf("Setting Half Speed (40 MHz)\n");
			/**to be done*/
			dbitPipeline(HALFSPEED_DBIT_PIPELINE);
			adcPipeline(HALFSPEED_ADC_PIPELINE);
			initSpeedConfGain(HALFSPEED_CONF);
			adcPhase(HALFSPEED_ADC_PHASE);
			break;
		case HALF:
			printf("Setting Half Speed (20 MHz)\n");
			dbitPipeline(HALFSPEED_DBIT_PIPELINE);
			adcPipeline(HALFSPEED_ADC_PIPELINE);
			initSpeedConfGain(HALFSPEED_CONF);
			adcPhase(HALFSPEED_ADC_PHASE);
			break;
		case QUARTER:
			printf("Setting Half Speed (10 MHz)\n");
			dbitPipeline(QUARTERSPEED_DBIT_PIPELINE);
			adcPipeline(QUARTERSPEED_ADC_PIPELINE);
			initSpeedConfGain(QUARTERSPEED_CONF);
			adcPhase(QUARTERSPEED_ADC_PHASE);
			break;
		}
	}
	return getClockDivider();
}



int adcPhase(int st){
	printf("\nSetting ADC Phase to %d\n",st);
	if (st>65535 || st<-65535)
		return clkPhase[0];
#ifdef NEW_PLL_RECONFIG
	printf("reset pll\n");
	bus_w(PLL_CNTRL_REG,((1<<PLL_CNTR_PLL_RESET_BIT))); //reset PLL
	usleep(100);
	bus_w(PLL_CNTRL_REG, 0);
	clkPhase[1]=st;
#else
	clkPhase[1]=st-clkPhase[0];
#endif
	printf("phase %d\n", clkPhase[1] );
	configurePll(2);
	clkPhase[0]=st;

	return clkPhase[0];
}


int getPhase() {
	return clkPhase[0];

};


u_int32_t getClockDivider(int ic) {

	enum clkspeed{FULL,HALF,QUARTER};
	switch(initSpeedConfGain(-1)){
	//case FULLSPEED_CONF:
	//return FULL;
	case HALFSPEED_CONF:
		return HALF;
	case QUARTERSPEED_CONF:
		return QUARTER;
	default:
		return -1;
	}
}


u_int32_t adcPipeline(int d) {
	if (d>=0){
		printf("Setting ADC Pipeline to 0x%x\n",d);
		bus_w(ADC_PIPELINE_REG, d);
	}
	return bus_r(ADC_PIPELINE_REG);

}


u_int32_t dbitPipeline(int d){
	if (d>=0){
		printf("Setting DBIT Pipeline to 0x%x\n",d);
		bus_w(DBIT_PIPELINE_REG, d);
	}
	return bus_r(DBIT_PIPELINE_REG);
}

u_int32_t setSetLength(int d) {
	return 0;
}

u_int32_t getSetLength() {
	return 0;
}

u_int32_t setOversampling(int d) {

	if (d>=0 && d<=255)
		bus_w(OVERSAMPLING_REG, d);

	return bus_r(OVERSAMPLING_REG);
}


u_int32_t setWaitStates(int d1) {
	return 0;
}

u_int32_t getWaitStates() {
	return 0;
}


u_int32_t setTotClockDivider(int d) {
	return 0;
}

u_int32_t getTotClockDivider() {
	return 0;
}


u_int32_t setTotDutyCycle(int d) {
	return 0;
}

u_int32_t getTotDutyCycle() {
	return 0;
}


u_int32_t setExtSignal(int d, enum externalSignalFlag  mode) {

	u_int32_t c;
	c=bus_r(EXT_SIGNAL_REG);

	if (d>=0 && d<4) {
		signals[d]=mode;
#ifdef VERBOSE
		printf("settings signal variable number %d to value %04x\n", d, signals[d]);
#endif

		// if output signal, set it!

		switch (mode) {
		case GATE_IN_ACTIVE_HIGH:
		case GATE_IN_ACTIVE_LOW:
			if (timingMode==GATE_FIX_NUMBER || timingMode==GATE_WITH_START_TRIGGER)
				setFPGASignal(d,mode);
			else
				setFPGASignal(d,SIGNAL_OFF);
			break;
		case TRIGGER_IN_RISING_EDGE:
		case TRIGGER_IN_FALLING_EDGE:
			if (timingMode==TRIGGER_EXPOSURE || timingMode==GATE_WITH_START_TRIGGER)
				setFPGASignal(d,mode);
			else
				setFPGASignal(d,SIGNAL_OFF);
			break;
		case RO_TRIGGER_IN_RISING_EDGE:
		case RO_TRIGGER_IN_FALLING_EDGE:
			if (timingMode==TRIGGER_READOUT)
				setFPGASignal(d,mode);
			else
				setFPGASignal(d,SIGNAL_OFF);
			break;
		case MASTER_SLAVE_SYNCHRONIZATION:
			setSynchronization(syncMode);
			break;
		default:
			setFPGASignal(d,mode);
			break;
		}

		setTiming(GET_EXTERNAL_COMMUNICATION_MODE);
	}

	return getExtSignal(d);
}



u_int32_t setFPGASignal(int d, enum externalSignalFlag  mode) {


	int modes[]={EXT_SIG_OFF, EXT_GATE_IN_ACTIVEHIGH, EXT_GATE_IN_ACTIVELOW,EXT_TRIG_IN_RISING,EXT_TRIG_IN_FALLING,EXT_RO_TRIG_IN_RISING, EXT_RO_TRIG_IN_FALLING,EXT_GATE_OUT_ACTIVEHIGH, EXT_GATE_OUT_ACTIVELOW, EXT_TRIG_OUT_RISING, EXT_TRIG_OUT_FALLING, EXT_RO_TRIG_OUT_RISING, EXT_RO_TRIG_OUT_FALLING};

	u_int32_t c;
	int off=d*SIGNAL_OFFSET;
	c=bus_r(EXT_SIGNAL_REG);


	if (mode<=RO_TRIGGER_OUT_FALLING_EDGE &&  mode>=0) {
#ifdef VERBOSE
		printf("writing signal register number %d mode %04x\n",d, modes[mode]);
#endif
		bus_w(EXT_SIGNAL_REG,((modes[mode])<<off)|(c&~(SIGNAL_MASK<<off)));
	}
	return getExtSignal(d);
}



int getExtSignal(int d) {

	if (d>=0 && d<4) {
#ifdef VERBOSE
		printf("gettings signal variable number %d  value %04x\n", d, signals[d]);
#endif
		return signals[d];
	} else
		return -1;
}


int getFPGASignal(int d) {

	int modes[]={SIGNAL_OFF, GATE_IN_ACTIVE_HIGH, GATE_IN_ACTIVE_LOW,TRIGGER_IN_RISING_EDGE, TRIGGER_IN_FALLING_EDGE,RO_TRIGGER_IN_RISING_EDGE, RO_TRIGGER_IN_FALLING_EDGE, GATE_OUT_ACTIVE_HIGH,   GATE_OUT_ACTIVE_LOW, TRIGGER_OUT_RISING_EDGE, TRIGGER_OUT_FALLING_EDGE, RO_TRIGGER_OUT_RISING_EDGE,RO_TRIGGER_OUT_FALLING_EDGE};

	int off=d*SIGNAL_OFFSET;
	int mode=((bus_r(EXT_SIGNAL_REG)&(SIGNAL_MASK<<off))>>off);

	if (mode<=RO_TRIGGER_OUT_FALLING_EDGE) {
		if (modes[mode]!=SIGNAL_OFF && signals[d]!=MASTER_SLAVE_SYNCHRONIZATION)
			signals[d]=modes[mode];
#ifdef VERYVERBOSE
		printf("gettings signal register number %d  value %04x\n", d, modes[mode]);
#endif
		return modes[mode];
	} else
		return -1;

}





/*
enum externalCommunicationMode{
  GET_EXTERNAL_COMMUNICATION_MODE,
  AUTO,
  TRIGGER_EXPOSURE_SERIES,
  TRIGGER_EXPOSURE_BURST,
  TRIGGER_READOUT,
  TRIGGER_COINCIDENCE_WITH_INTERNAL_ENABLE,
  GATE_FIX_NUMBER,
  GATE_FIX_DURATION,
  GATE_WITH_START_TRIGGER,
  GATE_COINCIDENCE_WITH_INTERNAL_ENABLE
};
 */


int setTiming(int ti) {

	int ret=GET_EXTERNAL_COMMUNICATION_MODE;

	int g=-1, t=-1, rot=-1;

	int i;

	switch (ti) {
	case AUTO_TIMING:
		printf("\nSetting timing to auto\n");
		timingMode=ti;
		// disable all gates/triggers in except if used for master/slave synchronization
		for (i=0; i<4; i++) {
			if (getFPGASignal(i)>0 && getFPGASignal(i)<GATE_OUT_ACTIVE_HIGH && signals[i]!=MASTER_SLAVE_SYNCHRONIZATION)
				setFPGASignal(i,SIGNAL_OFF);
		}
		break;

	case   TRIGGER_EXPOSURE:
		printf("\nSetting timing to trigger exposure\n");
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
		printf("\nSetting timing to trigger readout\n");
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
		printf("\nSetting timing to gate fix number\n");
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
		printf("\nSetting timing to gate with start trigger\n");
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
		break;

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

	// timingMode=ret;

	return ret;

}



int setConfigurationRegister(int d) {
#ifdef VERBOSE
	printf("Setting configuration register to %x",d);
#endif
	if (d>=0) {
		bus_w(CONFIG_REG,d);
	}
#ifdef VERBOSE
	printf("configuration register is %x", bus_r(CONFIG_REG));
#endif
	return bus_r(CONFIG_REG);
}

int setToT(int d) {
	//int ret=0;
	int reg;
#ifdef VERBOSE
	printf("Setting ToT to %d\n",d);
#endif
	reg=bus_r(CONFIG_REG);
#ifdef VERBOSE
	printf("Before: ToT is %x\n", reg);
#endif
	if (d>0) {
		bus_w(CONFIG_REG,reg|TOT_ENABLE_BIT);
	} else if (d==0) {
		bus_w(CONFIG_REG,reg&(~TOT_ENABLE_BIT));
	}
	reg=bus_r(CONFIG_REG);
#ifdef VERBOSE
	printf("ToT is %x\n", reg);
#endif
	if (reg&TOT_ENABLE_BIT)
		return 1;
	else
		return 0;
}

int setContinousReadOut(int d) {
	//int ret=0;
	int reg;
#ifdef VERBOSE
	printf("Setting Continous readout to %d\n",d);
#endif
	reg=bus_r(CONFIG_REG);
#ifdef VERBOSE
	printf("Before: Continous readout is %x\n", reg);
#endif
	if (d>0) {
		bus_w(CONFIG_REG,reg|CONT_RO_ENABLE_BIT);
	} else if (d==0) {
		bus_w(CONFIG_REG,reg&(~CONT_RO_ENABLE_BIT));
	}
	reg=bus_r(CONFIG_REG);
#ifdef VERBOSE
	printf("Continous readout is %x\n", reg);
#endif
	if (reg&CONT_RO_ENABLE_BIT)
		return 1;
	else
		return 0;
}





u_int64_t  getDetectorNumber() {
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
	return res;
}

u_int32_t  getFirmwareVersion() {
	return bus_r(FPGA_VERSION_REG);
}

u_int32_t  getFirmwareSVNVersion(){
	return bus_r(FPGA_SVN_REG);
}


// for fpga test 
u_int32_t testFpga(void) {
	printf("Testing FPGA:\n");
	volatile u_int32_t val,addr,val2;
	int result=OK,i;
	//fixed pattern
	val=bus_r(FIX_PATT_REG);
	if (val==FIXED_PATT_VAL) {
		printf("fixed pattern ok!! %08x\n",val);
	} else {
		printf("fixed pattern wrong!! %08x\n",val);
		result=FAIL;
	}

	//dummy register
	addr = DUMMY_REG;
	for(i=0;i<1000000;i++)
	{
		val=0x5A5A5A5A-i;
		bus_w(addr, val);
		val=bus_r(addr);
		if (val!=0x5A5A5A5A-i) {
			printf("ATTEMPT:%d:\tFPGA dummy register wrong!! %x instead of %x \n",i,val,0x5A5A5A5A-i);
			result=FAIL;
		}
		val=(i+(i<<10)+(i<<20));
		bus_w(addr, val);
		val2=bus_r(addr);
		if (val2!=val) {
			printf("ATTEMPT:%d:\tFPGA dummy register wrong!! read %x instead of %x.\n",i,val2,val);
			result=FAIL;
		}
		val=0x0F0F0F0F;
		bus_w(addr, val);
		val=bus_r(addr);
		if (val!=0x0F0F0F0F) {
			printf("ATTEMPT:%d:\tFPGA dummy register wrong!! %x instead of 0x0F0F0F0F \n",i,val);
			result=FAIL;
		}
		val=0xF0F0F0F0;
		bus_w(addr, val);
		val=bus_r(addr);
		if (val!=0xF0F0F0F0)  {
			printf("ATTEMPT:%d:\tFPGA dummy register wrong!! %x instead of 0xF0F0F0F0 \n\n",i,val);
			result=FAIL;
		}
	}
	if(result==OK)
	{
		printf("----------------------------------------------------------------------------------------------");
		printf("\nATTEMPT 1000000: FPGA DUMMY REGISTER OK!!!\n");
		printf("----------------------------------------------------------------------------------------------");
	}
	printf("\n");
	return result;
}


u_int32_t testRAM(void) {
	int result=OK;
	printf("TestRAM not implemented\n");
	return result;
}


int getNModBoard() {
	return 1;
}

int setNMod(int n) {
	return 1;
}

int getNMod() {
	return 1;
}


// fifo test
int testFifos(void) {
	printf("Fifo test not implemented!\n");
	bus_w16(CONTROL_REG, START_FIFOTEST_BIT);
	bus_w16(CONTROL_REG, 0x0);
	return OK;
}



// program dacq settings

int64_t set64BitReg(int64_t value, int aLSB, int aMSB){
	int64_t v64;
	u_int32_t vLSB,vMSB;
	if (value!=-1) {
		vLSB=value&(0xffffffff);
		bus_w(aLSB,vLSB);
		v64=value>> 32;
		vMSB=v64&(0xffffffff);
		bus_w(aMSB,vMSB);
	}
	return get64BitReg(aLSB, aMSB);

}

int64_t get64BitReg(int aLSB, int aMSB){
	int64_t v64;
	u_int32_t vLSB,vMSB;
	vLSB=bus_r(aLSB);
	vMSB=bus_r(aMSB);
	v64=vMSB;
	v64=(v64<<32) | vLSB;

	printf("reg64(%x,%x) %x %x %llx\n", aLSB, aMSB, vLSB, vMSB, v64);

	return v64;
}

int64_t setFrames(int64_t value){
	if(value!=-1)
		printf("\nSetting number of frames to %lld\n",(long long int)value);

	int64_t retval = set64BitReg(value,  SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG);
	printf("Getting number of frames: %lld\n",(long long int)retval);
	return retval;
}

int64_t getFrames(){
	return get64BitReg(GET_FRAMES_LSB_REG, GET_FRAMES_MSB_REG);
}

int64_t setExposureTime(int64_t value){
	if (value!=-1){
		printf("\nSetting exptime to %lldns\n",(long long int)value);
		value*=(1E-3*clockdivider_exptime);
	}
	int64_t retval = set64BitReg(value,SET_EXPTIME_LSB_REG, SET_EXPTIME_MSB_REG)/(1E-3*clockdivider_exptime);//(1E-9*CLK_FREQ);
	printf("Getting exptime: %lldns\n",(long long int)retval);
	return retval;
}

int64_t getExposureTime(){
	return get64BitReg(GET_EXPTIME_LSB_REG, GET_EXPTIME_MSB_REG)/(1E-3*clockdivider_exptime);//(1E-9*CLK_FREQ);
}

int64_t setGates(int64_t value){
	if(value!=-1)
		printf("\nSetting number of gates to %lld\n",(long long int)value);

	int64_t retval = set64BitReg(value, SET_GATES_LSB_REG, SET_GATES_MSB_REG);
	printf("Getting number of gates: %lld\n",(long long int)retval);
	return retval;
}

int64_t getGates(){
	return get64BitReg(GET_GATES_LSB_REG, GET_GATES_MSB_REG);
}

int64_t setPeriod(int64_t value){
	if (value!=-1){
		printf("\nSetting period to %lldns\n",(long long int)value);
		value*=(1E-3*clockdivider_fc);
	}

	int64_t retval = set64BitReg(value,SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG)/(1E-3*clockdivider_fc);//(1E-9*CLK_FREQ);
	printf("Getting period: %lldns\n",(long long int)retval);
	return retval;
}

int64_t getPeriod(){
	return get64BitReg(GET_PERIOD_LSB_REG, GET_PERIOD_MSB_REG)/(1E-3*clockdivider_fc);//(1E-9*CLK_FREQ);
}

int64_t setDelay(int64_t value){
	if (value!=-1){
		printf("\nSetting delay to %lldns\n",(long long int)value);
		value*=(1E-3*clockdivider_fc);
	}

	int64_t retval = set64BitReg(value,SET_DELAY_LSB_REG, SET_DELAY_MSB_REG)/(1E-3*clockdivider_fc);//(1E-9*CLK_FREQ);
	printf("Getting delay: %lldns\n",(long long int)retval);
	return retval;
}

int64_t getDelay(){
	return get64BitReg(GET_DELAY_LSB_REG, GET_DELAY_MSB_REG)/(1E-3*clockdivider_fc);//(1E-9*CLK_FREQ);
}

int64_t setTrains(int64_t value){
	if(value!=-1)
		printf("\nSetting number of cycles to %lld\n",(long long int)value);

	int64_t retval = set64BitReg(value,  SET_CYCLES_LSB_REG, SET_CYCLES_MSB_REG);
	printf("Getting number of cycles: %lld\n",(long long int)retval);
	return retval;
}

int64_t getTrains(){
	return get64BitReg(GET_CYCLES_LSB_REG, GET_CYCLES_MSB_REG);
}


int64_t setSamples(int64_t value){
  if (value>=0)
    bus_w(NSAMPLES_REG,value);
  return bus_r(NSAMPLES_REG);
}

int64_t getSamples(){
  return bus_r(NSAMPLES_REG);//get64BitReg(GET_CYCLES_LSB_REG, GET_CYCLES_MSB_REG);
}


int64_t setProbes(int64_t value){
	return 0;
}


int64_t setProgress() {
	return 0;

}


int64_t getProgress() {


	//should be done in firmware!!!!

	return 0;

}

int64_t getActualTime(){
	return get64BitReg(GET_ACTUAL_TIME_LSB_REG, GET_ACTUAL_TIME_MSB_REG)/(1E-9*CLK_FREQ);
}

int64_t getMeasurementTime(){
	int64_t v=get64BitReg(GET_MEASUREMENT_TIME_LSB_REG, GET_MEASUREMENT_TIME_MSB_REG);
	return v/(1E-9*CLK_FREQ);
}

int64_t getFramesFromStart(){
	int64_t v=get64BitReg(FRAMES_FROM_START_LSB_REG, FRAMES_FROM_START_MSB_REG);
	int64_t v1=get64BitReg(FRAMES_FROM_START_PG_LSB_REG, FRAMES_FROM_START_PG_MSB_REG);

	printf("Frames from start data streaming %lld\n",v);
	printf("Frames from start run control %lld\n",v1);

	return v;
}


ROI *setROI(int nroi,ROI* arg,int *retvalsize, int *ret) {
	cprintf(RED,"ROI Not implemented yet\n");
	return NULL;
}


int loadImage(int index, short int ImageVals[]){
	printf("loadImage Not implemented yet\n");
	return OK;
}



int64_t getProbes(){
	return 0;
}


int setDACRegister(int idac, int val, int imod) {
	u_int32_t addr, reg, mask;
	int off;
#ifdef VERBOSE
	if(val==-1)
		printf("Getting dac register%d module %d\n",idac,imod);
	else
		printf("Setting dac register %d module %d to %d\n",idac,imod,val);
#endif

	switch(idac){
	case 0:
	case 1:
	case 2:
		addr=MOD_DACS1_REG;
		break;
	case 3:
	case 4:
	case 5:
		addr=MOD_DACS2_REG;
		break;
	case 6:
	case 7:
		addr=MOD_DACS3_REG;
		break;
	default:
		printf("weird idac value %d\n",idac);
		return -1;
		break;
	}
	//saving only the msb
	val=val>>2;

	off=(idac%3)*10;
	mask=~((0x3ff)<<off);

	if (val>=0 && val<DAC_DR) {
		reg=bus_r(addr+(imod<<SHIFTMOD));
		reg&=mask;
		reg|=(val<<off);
		bus_w(addr+(imod<<SHIFTMOD),reg);
	}
	val=(bus_r(addr+(imod<<SHIFTMOD))>>off)&0x3ff;
	//since we saved only the msb
	val=val<<2;

	//val=(bus_r(addr)>>off)&0x3ff;


#ifdef VERBOSE
	printf("Dac %d module %d register is %d\n\n",idac,imod,val);
#endif
	return val;
}


int getTemperature(int tempSensor, int imod){
	int val;
	imod=0;//ignoring more than 1 mod for now
	int i,j,repeats=6;
	u_int32_t tempVal=0;
#ifdef VERBOSE
	char cTempSensor[2][100]={"ADCs/ASICs","VRs/FPGAs"};
	printf("Getting Temperature of module:%d for the %s for tempsensor:%d\n",imod,cTempSensor[tempSensor],tempSensor);
#endif
	bus_w(TEMP_IN_REG,(T1_CLK_BIT)|(T1_CS_BIT)|(T2_CLK_BIT)|(T2_CS_BIT));//standby
	bus_w(TEMP_IN_REG,((T1_CLK_BIT)&~(T1_CS_BIT))|(T2_CLK_BIT));//high clk low cs

	for(i=0;i<20;i++) {
		//repeats is number of register writes for delay
		for(j=0;j<repeats;j++)
			bus_w(TEMP_IN_REG,~(T1_CLK_BIT)&~(T1_CS_BIT)&~(T2_CLK_BIT)&~(T2_CS_BIT));//low clk low cs
		for(j=0;j<repeats;j++)
			bus_w(TEMP_IN_REG,((T1_CLK_BIT)&~(T1_CS_BIT))|(T2_CLK_BIT));//high clk low cs

		if(i<=10){//only the first time
			if(!tempSensor)
				tempVal= (tempVal<<1) + (bus_r(TEMP_OUT_REG) & (1));//adc
			else
				tempVal= (tempVal<<1) + ((bus_r(TEMP_OUT_REG) & (2))>>1);//fpga
		}
	}

	bus_w(TEMP_IN_REG,(T1_CLK_BIT)|(T1_CS_BIT)|(T2_CLK_BIT)|(T2_CS_BIT));//standby
	val=((int)tempVal)/4.0;

#ifdef VERBOSE
	printf("Temperature of module:%d for the %s is %.2fC\n",imod,cTempSensor[tempSensor],val);
#endif
	return val;
}



int initHighVoltage(int val, int imod){


	u_int32_t offw,codata;
	u_int16_t valw, dacvalue;
	int i,ddx,csdx,cdx;
	float alpha=0.55;

	if (val>=0) {
		if (val<60) {
			dacvalue=0;
			val=60;
		} else if (val>=200) {
			dacvalue=0x1;
			val=200;
		} else {
			dacvalue=1.+(200.-val)/alpha;
			val=200.-(dacvalue-1)*alpha;
		}
		printf ("****************************** setting val %d, dacval %d\n",val, dacvalue);
		offw=DAC_REG;

		ddx=8; csdx=10; cdx=9;
		codata=((dacvalue)&0xff);




		valw=0xffff; bus_w(offw,(valw)); // start point
		valw=((valw&(~(0x1<<csdx))));bus_w(offw,valw); //chip sel bar down
		for (i=0;i<8;i++) {
			valw=(valw&(~(0x1<<cdx)));bus_w(offw,valw); //cldwn
			valw=((valw&(~(0x1<<ddx)))+(((codata>>(7-i))&0x1)<<ddx));bus_w(offw,valw);//write data (i)
			valw=((valw&(~(0x1<<cdx)))+(0x1<<cdx));bus_w(offw,valw);//clkup
		}
		valw=((valw&(~(0x1<<csdx)))+(0x1<<csdx));bus_w(offw,valw); //csup

		valw=(valw&(~(0x1<<cdx)));bus_w(offw,valw); //cldwn


		valw=0xffff; bus_w(offw,(valw)); // stop point =start point of course */


		printf("Writing %d in HVDAC  \n",dacvalue);

		bus_w(HV_REG,val);
	}



	return bus_r(HV_REG);



	//  return val;
}



int initConfGain(int isettings,int val,int imod){
	int retval;
	u_int32_t addr=CONFGAIN_REG;
	if(isettings!=-1){
		//#ifdef VERBOSE
		printf("Setting Gain with val:0x%x\n",val);
		//#endif
		bus_w(addr,(val|(bus_r(addr)&~GAIN_MASK)));
	}
	retval=(bus_r(addr)&GAIN_MASK);
	//#ifdef VERBOSE
	printf("Value read from Gain reg is 0x%x\n",retval);
	printf("Gain Reg Value is 0x%x\n",bus_r(addr));
	//#endif
	return retval;
}



int initSpeedConfGain(int val){
	int retval;
	u_int32_t addr=CONFGAIN_REG;
	if(val!=-1){
		//#ifdef VERBOSE
		printf("\nSetting Speed of Gain reg with val:0x%x\n",val);
		//#endif
		bus_w(addr,((val<<SPEED_GAIN_OFFSET)|(bus_r(addr)&~SPEED_GAIN_MASK)));
	}
	retval=((bus_r(addr)&SPEED_GAIN_MASK)>>SPEED_GAIN_OFFSET);
	//#ifdef VERBOSE
	printf("Value read from Speed of Gain reg is 0x%x\n",retval);
	printf("Gain Reg Value is 0x%x\n",bus_r(addr));
	//#endif
	return retval;
}


int setADC(int adc){
	int reg,nchips,mask,nchans;

	if(adc==-1)	ROI_flag=0;
	else		ROI_flag=1;

	//	setDAQRegister();//token timing
	cleanFifo();//adc sync

	//with moench module all adc
	//set packet size
	ipPacketSize= DEFAULT_IP_PACKETSIZE;
	udpPacketSize=DEFAULT_UDP_PACKETSIZE;
	//set channel mask
	nchips = NCHIP;
	nchans = NCHANS;
	mask = ACTIVE_ADC_MASK;

	//set channel mask
	reg = (nchans*nchips)<<CHANNEL_OFFSET;
	reg&=CHANNEL_MASK;
	reg|=(ACTIVE_ADC_MASK & mask);
	bus_w(CHIP_OF_INTRST_REG,reg);

	//#ifdef DDEBUG
	printf("Chip of Interest Reg:%x\n",bus_r(CHIP_OF_INTRST_REG));
	//#endif

	adcConfigured = adc;

	return adcConfigured;
}



long int calcChecksum(int sourceip, int destip) {


	ip_header ip;
	int count;
	unsigned short *addr;
	long int sum = 0;
	long int checksum;

	ip.ip_ver            = 0x4;
	ip.ip_ihl            = 0x5;
	ip.ip_tos            = 0x0;
	ip.ip_len            = 0x2052;//ipPacketSize;//fixed in firmware
	ip.ip_ident          = 0x0000;
	ip.ip_flag           = 0x2; //not nibble aligned (flag& offset
	ip.ip_offset         = 0x000;
	ip.ip_ttl            = 0x40;
	ip.ip_protocol       = 0x11;
	ip.ip_chksum         = 0x0000 ; // pseudo
	ip.ip_sourceip       = sourceip;
	ip.ip_destip         = destip;


	count=sizeof(ip);
	addr=&(ip); /* warning: assignment from incompatible pointer type */
	while( count > 1 )  {
		sum += *addr++;
		count -= 2;
	}
	if( count > 0 )  sum += *addr;                     // Add left-over byte, if any
	while (sum>>16) sum = (sum & 0xffff) + (sum >> 16);// Fold 32-bit sum to 16 bits
	checksum = (~sum)&0xffff;

	printf("IP checksum is 0x%lx\n",checksum);

	return checksum;
}



#ifdef NEW_GBE_INTERFACE
int  writeGbeReg(int ivar,  uint32_t val, int addr, int interface) {
	/* #define GBE_CTRL_WSTROBE 0  */
	/* #define GBE_CTRL_VAR_OFFSET 16 */
	/* #define GBE_CTRL_VAR_MASK 0XF */
	/* #define GBE_CTRL_RAMADDR_OFFSET 24 */
	/* #define GBE_CTRL_RAMADDR_MASK 0X3F */
	/* #define GBE_CTRL_INTERFACE 23 */
	uint32_t ctrl=((ivar&GBE_CTRL_VAR_MASK)<<GBE_CTRL_VAR_OFFSET)|((addr&GBE_CTRL_RAMADDR_MASK)<<GBE_CTRL_RAMADDR_OFFSET)| (interface<<GBE_CTRL_INTERFACE);
	bus_w(GBE_CNTRL_REG,ctrl);
	bus_w(GBE_PARAM_REG,val);
	bus_w(GBE_CNTRL_REG,ctrl|(1<<GBE_CTRL_WSTROBE));
	usleep(100);
	bus_w(GBE_CNTRL_REG,ctrl);


}
#endif


int configureInterface(uint32_t destip,uint64_t destmac,uint64_t  sourcemac,int sourceip,int ival,uint32_t destport,  uint32_t sourceport, int interface) {
	//int configureMAC(int ipad,long long int macad,long long int detectormacad, int detipad, int ival, int udpport){


	/*volatile u_int32_t conf= bus_r(CONFIG_REG);*/
	long int checksum=calcChecksum(sourceip, destip);
#ifdef NEW_GBE_INTERFACE

	printf("Configure interface %d\n",interface);
	const int nvar=12;
	uint32_t vals[nvar];
	int ivar;
	int addr=0;
	vals[RX_UDP_IP_ADDR]=destip;
	vals[RX_UDP_PORTS_ADDR]=destport;
	vals[RX_UDP_MAC_L_ADDR]=(destmac)&0xFFFFFFFF;
	vals[RX_UDP_MAC_H_ADDR]=(destmac>>32)&0xFFFFFFFF;
	vals[IPCHECKSUM_ADDR]=checksum;
	vals[GBE_DELAY_ADDR]=0;
	vals[GBE_RESERVED1_ADDR]=sourceport;
	vals[GBE_RESERVED2_ADDR]=interface;
	vals[DETECTOR_MAC_L_ADDR]=(sourcemac)&0xFFFFFFFF;
	vals[DETECTOR_MAC_H_ADDR]=(sourcemac>>32)&0xFFFFFFFF;
	vals[DETECTOR_IP_ADDR]=sourceip;

	for (ivar=0; ivar<nvar; ivar++) {
		writeGbeReg(ivar, vals[ivar], addr, interface);
	}


#else
	bus_w(DETECTORIP_AREG,sourceip);//detectorip_AReg_c
	bus_w(RX_UDP_AREG,destip);//rx_udpip_AReg_c

	bus_w(RX_UDPMACH_AREG,(destmac>>32)&0xFFFFFFFF);//rx_udpmacH_AReg_c
	bus_w(RX_UDPMACL_AREG,(destmac)&0xFFFFFFFF);//rx_udpmacL_AReg_c
	bus_w(DETECTORMACH_AREG,(sourcemac>>32)&0xFFFFFFFF);//detectormacH_AReg_c
	bus_w(DETECTORMACL_AREG,(sourcemac)&0xFFFFFFFF);//detectormacL_AReg_c
	bus_w(UDPPORTS_AREG,((sourceport&0xFFFF)<<16)+(destport&0xFFFF));//udpports_AReg_c
	bus_w(IPCHKSUM_AREG,(checksum&0xFFFF));//ipchksum_AReg_c


#endif

	//bus_w(CONTROL_REG,GB10_RESET_BIT);
	//usleep(50 * 1000);
	bus_w(CONTROL_REG,0);
	//usleep(500* 1000);
	//bus_w(CONFIG_REG,conf | GB10_NOT_CPU_BIT);
	printf("System status register is %08x\n",bus_r(SYSTEM_STATUS_REG));


	/*
	bus_w(CONTROL_REG,GB10_RESET_BIT);
	bus_w(CONTROL_REG,0);
	usleep(500 * 1000);
	bus_w(CONFIG_REG,conf | GB10_NOT_CPU_BIT);
	printf("System status register is %08x\n",bus_r(SYSTEM_STATUS_REG));
	*/

	printf("Reset mem machine fifos\n");
	bus_w(MEM_MACHINE_FIFOS_REG,0x4000);
	bus_w(MEM_MACHINE_FIFOS_REG,0x0);
	printf("Reset run control\n");
	bus_w(MEM_MACHINE_FIFOS_REG,0x0400);
	bus_w(MEM_MACHINE_FIFOS_REG,0x0);

	usleep(500 * 1000);

	return 0; //any value doesnt matter - dhanya

}











int configureMAC(uint32_t destip,uint64_t destmac,uint64_t  sourcemac,int sourceip,int ival,uint32_t destport) {
	//int configureMAC(int ipad,long long int macad,long long int detectormacad, int detipad, int ival, int udpport){





	/*volatile u_int32_t conf= bus_r(CONFIG_REG);*/
	uint32_t sourceport  =  0x7e9a; // 0xE185;
	int interface=0;
	int ngb;

#ifdef NEW_GBE_INTERFACE
	ngb=2;
	printf("--------- New XGB interface\n");
#else
	ngb=1;
	printf("********* Old XGB interface\n");
#endif

	for (interface=0; interface <ngb; interface++)
		configureInterface(destip, destmac, sourcemac+interface, sourceip+interface, ival, destport+interface, sourceport+interface, interface);


	return OK;

}


int getAdcConfigured(){
	return adcConfigured;
}

u_int32_t runBusy(void) {
	u_int32_t s = bus_r(STATUS_REG) & 1;
#ifdef VERBOSE
	printf("status %04x\n",s);
#endif
	return s;
}

u_int32_t dataPresent(void) {
	return bus_r(LOOK_AT_ME_REG);
}

u_int32_t runState(void) {
	int s=bus_r(STATUS_REG);
#ifdef SHAREDMEMORY
	if (s&RUN_BUSY_BIT)
		write_status_sm("Running");
	else
		write_status_sm("Stopped");
#endif
#ifdef VERBOSE
	printf("status %04x\n",s);
#endif

	/*  if (s==0x62001)
    exit(-1);*/
	return s;
}


// State Machine

int startStateMachine(){
	//int i;
	//#ifdef VERBOSE
	printf("*******Starting State Machine*******\n");
	//#endif
	//	cleanFifo();
	// fifoReset();
#ifdef SHAREDMEMORY
	write_stop_sm(0);
	write_status_sm("Started");
#endif
	//start state machine
	bus_w16(CONTROL_REG, FIFO_RESET_BIT);
	bus_w16(CONTROL_REG, 0x0);
	bus_w16(CONTROL_REG, START_ACQ_BIT |  START_EXPOSURE_BIT);
	bus_w16(CONTROL_REG, 0x0);

	printf("statusreg=%08x\n",bus_r(STATUS_REG));
	return OK;
}




int stopStateMachine(){
	//#ifdef VERBOSE
	cprintf(BG_RED,"*******Stopping State Machine*******\n");
	//#endif
#ifdef SHAREDMEMORY
	write_stop_sm(1);
	write_status_sm("Stopped");
#endif
	// for(i=0;i<100;i++){
	//stop state machine
	bus_w16(CONTROL_REG, STOP_ACQ_BIT);
	usleep(100);
	bus_w16(CONTROL_REG, 0x0);

	printf("statusreg=%08x\n",bus_r(STATUS_REG));
	return OK;
}


int startReadOut(){
	u_int32_t status;
#ifdef VERBOSE
	printf("Starting State Machine Readout\n");
#endif
	status=bus_r(STATUS_REG)&RUN_BUSY_BIT;
#ifdef DEBUG
	printf("State machine status is %08x\n",bus_r(STATUS_REG));
#endif
	bus_w16(CONTROL_REG,  START_ACQ_BIT |START_READOUT_BIT);   //  start readout
	usleep(100);
	bus_w16(CONTROL_REG,  0x0);
	return OK;
}


// fifo routines

u_int32_t fifoReset(void) {
	return -1;
}


u_int32_t setNBits(u_int32_t n) {
	return -1;
}

u_int32_t getNBits(){
	return -1;
}


u_int32_t fifoReadCounter(int fifonum){
	return -1;
}

u_int32_t  fifoReadStatus(){
	return bus_r(STATUS_REG)&(SOME_FIFO_FULL_BIT | ALL_FIFO_EMPTY_BIT);
}


u_int32_t  fifo_full(void){
	return bus_r(STATUS_REG)&SOME_FIFO_FULL_BIT;
}



void waitForAcquisitionEnd(){
	while(runBusy()){
		usleep(500);
	}
}










u_int32_t* decode_data(int *datain)
{
	u_int32_t *dataout;
	// const char one=1;
	const int bytesize=8;
	char *ptr=(char*)datain;
	//int nbits=dynamicRange;
	int  ipos=0, ichan=0;;
	//int nch, boff=0;
	int ibyte;//, ibit;
	char iptr;

#ifdef VERBOSE
	printf("Decoding data for DR %d\n",dynamicRange);
#endif
	dataout=malloc(NCHAN*NCHIP*nModX*4);
	ichan=0;
	switch (dynamicRange) {
	case 1:
		for (ibyte=0; ibyte<dataBytes; ibyte++) {
			iptr=ptr[ibyte];
			for (ipos=0; ipos<bytesize; ipos++) {
				dataout[ichan]=(iptr>>(ipos))&0x1;
				ichan++;
			}
		}
		break;
	case 4:
		for (ibyte=0; ibyte<dataBytes; ibyte++) {
			iptr=ptr[ibyte]&0xff;
			for (ipos=0; ipos<2; ipos++) {
				dataout[ichan]=(iptr>>(ipos*4))&0xf;
				ichan++;
			}
		}
		break;
	case 8:
		for (ichan=0; ichan<dataBytes; ichan++) {
			dataout[ichan]=ptr[ichan]&0xff;
		}
		break;
	case 16:
		for (ichan=0; ichan<NCHAN*NCHIP*nModX; ichan++) {
			dataout[ichan]=0;
			for (ibyte=0; ibyte<2; ibyte++) {
				iptr=ptr[ichan*2+ibyte];
				dataout[ichan]|=((iptr<<(ibyte*bytesize))&(0xff<<(ibyte*bytesize)));
			}
		}
		break;
	default:
		for (ichan=0; ichan<NCHAN*NCHIP*nModX; ichan++)
			dataout[ichan]=datain[ichan]&0xffffff;
		break;
	}

#ifdef VERBOSE
printf("decoded %d  channels\n",ichan);
#endif
return dataout;
}



int setDynamicRange(int dr) {
	return dynamicRange;
}


int getDynamicRange() {
	return dynamicRange;
}

int testBus() {
	u_int32_t j;
	u_int64_t i, n, nt;
	// char cmd[100];
	u_int32_t val=0x0;
	int ifail=OK;
	// printf("%s\n",cmd);
	// system(cmd);
	i=0;

	n=1000000;
	nt=n/100;
	printf("testing bus %d times\n",(int)n);
	while (i<n) {
		// val=bus_r(FIX_PATT_REG);
		bus_w(DUMMY_REG,val);
		bus_w(FIX_PATT_REG,0);
		j=bus_r(DUMMY_REG);
		//if (i%10000==1)
		if (j!=val){
			printf("%d :  read wrong value %08x instead of %08x\n",(int)i,j, val);
			ifail++;
			//return FAIL;
		}// else
		// printf("%d : value OK 0x%08x\n",i,j);
		if ((i%nt)==0)
			printf("%lld cycles OK\n",i);
		val+=0xbbbbb;
		i++;
	}
	return ifail;
}


int setStoreInRAM(int b) {
	if (b>0)
		storeInRAM=1;
	else
		storeInRAM=0;
	return  OK;
}

int getChannels() {
	int nch=32;
	int i;
	for (i=0; i<NCHAN; i++) {
		if (adcDisableMask & (1<<i)) nch--;
	}
	return nch;
}

int allocateRAM() {
	return OK;
}


int writeADC(int addr, int val) {


	u_int32_t valw,codata,csmask;
	int i,cdx,ddx;
	cdx=0; ddx=1;
	csmask=0xfc; //  1111100

	codata=val + (addr<< 8);
	printf("***** ADC SPI WRITE TO REGISTER %04X value %04X\n",addr,val);
	// start point
	valw=0xff;
	bus_w16(ADC_WRITE_REG,(valw));

	//chip sel bar down
	valw=((0xffffffff&(~csmask)));
	bus_w16(ADC_WRITE_REG,valw);

	for (i=0;i<24;i++) {
		//cldwn
		valw=valw&(~(0x1<<cdx));
		bus_w16(ADC_WRITE_REG,valw);
		// usleep(0);

		//write data (i)
		valw=(valw&(~(0x1<<ddx)))+(((codata>>(23-i))&0x1)<<ddx);
		bus_w16(ADC_WRITE_REG,valw);
		//  usleep(0);

		//clkup
		valw=valw+(0x1<<cdx);
		bus_w16(ADC_WRITE_REG,valw);
		// usleep(0);
	}

	// stop point =start point
	valw=valw&(~(0x1<<cdx));
	// usleep(0);
	valw=0xff;
	bus_w16(ADC_WRITE_REG,(valw));

	//usleep in between
	//   usleep(50000);

	return OK;
}



int prepareADC(){
	printf("Preparing ADC\n");
	u_int32_t codata,csmask;
	int cdx,ddx;
	cdx=0; ddx=1;
	csmask=0x7c; //  1111100

	codata=0;
	writeADC(0x08,0x3);
	writeADC(0x08,0x0);
	writeADC(0x16,0x01);//output clock phase
	// writeADC(0x16,0x07);//output clock phase
	//   writeADC(0x16,0x4);//output clock phase
	//  writeADC(0x18,0x0);// vref 1V
	writeADC(0x14,0x40);//lvds reduced range -- offset binary
	writeADC(0xD,0x0);//no test mode

#ifdef TESTADC

	////////////TEST ADC!!!!!!!!!!
	printf("***************************************** *******\n");
	printf("******* PUTTING ADC IN TEST MODE!!!!!!!!! *******\n");
	printf("***************************************** *******\n");

	//    writeADC(0xD,0x4);//ALTERNATING CHECKERBOARD
	//  writeADC(0xD,0x7);//ONE/ZERO WORD TOGGLE
	/*     writeADC(0x19,0xf0);//user input */
	/*     writeADC(0x1A,0xf0);//user input */
	/*     writeADC(0x1B,0x0f);//user input */
	/*     writeADC(0x1C,0x0f);//user input */
	/*      writeADC(0xD,0x48);//user input, alternate */
	/*     //writeADC(0xD,0xA);//1xsync */
	//    writeADC(0xD,0xB);//1xbit high
	writeADC(0xD,0xC);//1xmixed frequqncy
#endif

	bus_w(ADC_LATCH_DISABLE_REG,0x0); // enable all ADCs
	bus_w(CONFGAIN_REG,0x12); //adc pipeline=18
	bus_w(CONFGAIN_REG,0xbbbbbbbb);
	//   bus_w(ADC_INVERSION_REG,0x1f6170c6);

	return OK;

}


int clearRAM() {
	return OK;
}




int setMaster(int f) {

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
		break;
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
		break;

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
		break;
	}
	return masterMode;
}



int setSynchronization(int s) {

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
		break;
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
	return NO_SYNCHRONIZATION;

}



int readCounterBlock(int startACQ, short int CounterVals[]){

	//char *counterVals=NULL;
	//counterVals=realloc(counterVals,dataBytes);

	u_int32_t val;
	volatile u_int16_t *ptr;

	u_int32_t address = COUNTER_MEMORY_REG;
	ptr=(u_int16_t*)(CSP0BASE+address*2);


	if (runBusy()) {
		if(stopStateMachine()==FAIL)
			return FAIL;
		//waiting for the last frame read to be done
		while(runBusy())  usleep(500);
#ifdef VERBOSE
		printf("State machine stopped\n");
#endif
	}

	val=bus_r(MULTI_PURPOSE_REG);
#ifdef VERBOSE
	printf("Value of multipurpose reg:%d\n",bus_r(MULTI_PURPOSE_REG));
#endif

	memcpy(CounterVals,(u_int16_t *)ptr,dataBytes);
#ifdef VERBOSE
	int i;
	printf("Copied counter memory block with size of %d bytes..\n",dataBytes);
	for(i=0;i<6;i++)
		printf("%d: %d\t",i,CounterVals[i]);
#endif


	bus_w(MULTI_PURPOSE_REG,(val&~RESET_COUNTER_BIT));
#ifdef VERBOSE
	printf("\nClearing bit 2 of multipurpose reg:%d\n",bus_r(MULTI_PURPOSE_REG));
#endif

	if(startACQ==1){
		startStateMachine();
		if(runBusy())
			printf("State machine RUNNING\n");
		else
			printf("State machine IDLE\n");
	}

	/*		if(sizeof(CounterVals)<=0){
			printf("ERROR:size of counterVals=%d\n",(int)sizeof(CounterVals));
			return FAIL;
		}*/


	return OK;
}




int resetCounterBlock(int startACQ){

	char *counterVals=NULL;
	counterVals=realloc(counterVals,dataBytes);

	int ret = OK;
	u_int32_t val;
	volatile u_int16_t *ptr;


	u_int32_t address = COUNTER_MEMORY_REG;
	ptr=(u_int16_t*)(CSP0BASE+address*2);


	if (runBusy()) {
		if(stopStateMachine()==FAIL)
			return FAIL;
		//waiting for the last frame read to be done
		while(runBusy())  usleep(500);
#ifdef VERBOSE
		printf("State machine stopped\n");
#endif
	}

	val=bus_r(MULTI_PURPOSE_REG);
#ifdef VERBOSE
	printf("Value of multipurpose reg:%d\n",bus_r(MULTI_PURPOSE_REG));
#endif


	bus_w(MULTI_PURPOSE_REG,(val|RESET_COUNTER_BIT));
#ifdef VERBOSE
	printf("Setting bit 2 of multipurpose reg:%d\n",bus_r(MULTI_PURPOSE_REG));
#endif


	memcpy(counterVals,(u_int16_t*)ptr,dataBytes);/*warning: passing argument 2 of ‘memcpy’ discards qualifiers from pointer target type*/
#ifdef VERBOSE
	int i;
	printf("Copied counter memory block with size of %d bytes..\n",(int)sizeof(counterVals));
	for(i=0;i<6;i=i+2)
		printf("%d: %d\t",i,*(counterVals+i));
#endif


	bus_w(MULTI_PURPOSE_REG,(val&~RESET_COUNTER_BIT));
#ifdef VERBOSE
	printf("\nClearing bit 2 of multipurpose reg:%d\n",bus_r(MULTI_PURPOSE_REG));
#endif

	if(startACQ==1){
		startStateMachine();
		if(runBusy())
			printf("State machine RUNNING\n");
		else
			printf("State machine IDLE\n");
	}

	if(sizeof(counterVals)<=0){
		printf("ERROR:size of counterVals=%d\n",(int)sizeof(counterVals));
		ret = FAIL;
	}

	return ret;

}



int calibratePedestal(int frames){
	printf("---------------------------\n");
	printf("In Calibrate Pedestal\n");
	int64_t framesBefore = getFrames();
	int64_t periodBefore = getPeriod();
	setFrames(frames);
	setPeriod(1000000);
	int dataret = OK;

	double avg[1280];
	int numberFrames = 0;

	int adc = 3;
	int adcCh = 3;
	int Ch = 3;


	int i = 0;
	for(i =0; i < 1280; i++){

		avg[i] = 0.0;
	}

	startStateMachine();

	while(dataret==OK){
		//got data
		waitForAcquisitionEnd();
		if (getFrames()>-2) {
			dataret=FAIL;
			printf("no data and run stopped: %d frames left\n",(int)(getFrames()+2));
		} else {
			dataret=FINISHED;
			printf("acquisition successfully finished\n");
		}
	}



	//double nf = (double)numberFrames;
	for(i =0; i < 1280; i++){
		adc = i / 256;
		adcCh = (i - adc * 256) / 32;
		Ch = i - adc * 256 - adcCh * 32;
		adc--;
		double v2 = avg[i];
		avg[i] = avg[i]/ ((double)numberFrames/(double)frames);
		unsigned short v = (unsigned short)avg[i];
		printf("setting avg for channel %i(%i,%i,%i): %i (double= %f (%f))\t", i,adc,adcCh,Ch, v,avg[i],v2);
		v=i*100;
		ram_w16(DARK_IMAGE_REG,adc,adcCh,Ch,v-4096);
		if(ram_r16(DARK_IMAGE_REG,adc,adcCh,Ch) !=  v-4096){
			printf("value is wrong (%i,%i,%i): %i \n",adc,adcCh,Ch,  ram_r16(DARK_IMAGE_REG,adc,adcCh,Ch));
		}
	}



	printf("frames: %i\n",numberFrames);
	printf("corrected avg by: %f\n",(double)numberFrames/(double)frames);

	printf("restoring previous condition\n");
	setFrames(framesBefore);
	setPeriod(periodBefore);

	printf("---------------------------\n");
	return 0;
}




uint64_t readPatternWord(int addr) {
	uint64_t word=0;
	int cntrl=0;

	if (addr>=MAX_PATTERN_LENGTH)
		return -1;


	printf("read %x\n",addr);
	cntrl= (addr&APATTERN_MASK) << PATTERN_CTRL_ADDR_OFFSET;
	bus_w(PATTERN_CNTRL_REG, cntrl);
	usleep(1000);
	bus_w(PATTERN_CNTRL_REG, cntrl | (1<< PATTERN_CTRL_READ_BIT)  );
	usleep(1000);
	printf("reading\n");
	word=get64BitReg(PATTERN_OUT_LSB_REG,PATTERN_OUT_MSB_REG);
	printf("read %llx\n", word);
	usleep(1000);
	bus_w(PATTERN_CNTRL_REG, cntrl);
	printf("done\n");

	return word;
}

uint64_t writePatternWord(int addr, uint64_t word) {


	int cntrl=0;
	if (addr>=MAX_PATTERN_LENGTH)
		return -1;

	printf("write %x %llx\n",addr, word);
	if (word!=-1){

		set64BitReg(word,PATTERN_IN_REG_LSB,PATTERN_IN_REG_MSB);


		cntrl= (addr&APATTERN_MASK) << PATTERN_CTRL_ADDR_OFFSET;
		bus_w(PATTERN_CNTRL_REG, cntrl);
		usleep(1000);
		bus_w(PATTERN_CNTRL_REG, cntrl | (1<< PATTERN_CTRL_WRITE_BIT)  );
		usleep(1000);
		bus_w(PATTERN_CNTRL_REG, cntrl);
		return word;
	} else
		return readPatternWord(addr);
}

uint64_t writePatternIOControl(uint64_t word) {
	return FAIL;
}

uint64_t writePatternClkControl(uint64_t word) {
	return FAIL;
}

int setPatternLoop(int level, int *start, int *stop, int *n) {
	int ret=OK;
	int lval=0;

	int nreg;
	int areg;

	switch (level ) {
	case 0:
		nreg=PATTERN_N_LOOP0_REG;
		areg=PATTERN_LOOP0_AREG;
		break;
	case 1:
		nreg=PATTERN_N_LOOP1_REG;
		areg=PATTERN_LOOP1_AREG;
		break;
	case 2:
		nreg=PATTERN_N_LOOP2_REG;
		areg=PATTERN_LOOP2_AREG;
		break;
	case -1:
		nreg=-1;
		areg=PATTERN_LIMITS_AREG;
		break;
	default:
		return FAIL;
	}

	printf("level %d start %x stop %x nl %d\n",level, *start, *stop, *n);
	if (nreg>=0) {
		if ((*n)>=0) bus_w(nreg, *n);
		printf ("n %d\n",*n);
		*n=bus_r(nreg);
		printf ("n %d\n",*n);

	}

	printf("level %d start %x stop %x nl %d\n",level, *start, *stop, *n);
	lval=bus_r(areg);
	/*     printf("l=%x\n",bus_r16(areg)); */
	/*     printf("m=%x\n",bus_r16_m(areg)); */





	printf("lval %x\n",lval);
	if (*start==-1) *start=(lval>> ASTART_OFFSET)   & APATTERN_MASK;
	printf("start %x\n",*start);


	if (*stop==-1) *stop=(lval>> ASTOP_OFFSET)   & APATTERN_MASK;
	printf("stop %x\n",*stop);

	lval= ((*start & APATTERN_MASK) << ASTART_OFFSET) | ((*stop & APATTERN_MASK) << ASTOP_OFFSET);
	printf("lval %x\n",lval);

	bus_w(areg,lval);
	printf("lval %x\n",lval);


	return ret;
}


int setPatternWaitAddress(int level, int addr) {
	int reg;

	switch (level) {
	case 0:
		reg=PATTERN_WAIT0_AREG;
		break;
	case 1:
		reg=PATTERN_WAIT1_AREG;
		break;
	case 2:
		reg=PATTERN_WAIT2_AREG;
		break;
	default:
		return -1;
	};
	//  printf("BEFORE *********PATTERN IOCTRL IS %llx (%x)\n",writePatternIOControl(-1), PATTERN_IOCTRL_REG_MSB);

	// printf ("%d addr %x (%x)\n",level,addr,reg);
	if (addr>=0) bus_w(reg, addr);
	// printf ("%d addr %x %x (%x) \n",level,addr, bus_r(reg), reg);

	// printf("AFTER *********PATTERN IOCTRL IS %llx (%x)\n",writePatternIOControl(-1), PATTERN_IOCTRL_REG_MSB);

	return bus_r(reg);
}


uint64_t setPatternWaitTime(int level, uint64_t t) {
	int reglsb;
	int regmsb;


	switch (level) {
	case 0:
		reglsb=PATTERN_WAIT0_TIME_REG_LSB;
		regmsb=PATTERN_WAIT0_TIME_REG_MSB;
		break;
	case 1:
		reglsb=PATTERN_WAIT1_TIME_REG_LSB;
		regmsb=PATTERN_WAIT1_TIME_REG_MSB;
		break;
	case 2:
		reglsb=PATTERN_WAIT2_TIME_REG_LSB;
		regmsb=PATTERN_WAIT2_TIME_REG_MSB;
		break;
	default:
		return -1;
	}


	if (t>=0) set64BitReg(t,reglsb,regmsb);
	return get64BitReg(reglsb,regmsb);

}


void initDac(int dacnum) {
	printf("\nInitializing dac for %d\n",dacnum);

	u_int32_t offw,codata;
	u_int16_t valw;
	int i,ddx,csdx,cdx;



	//setting int reference
	offw=DAC_REG;


	ddx=0; cdx=1;
	csdx=dacnum/8+2;


	printf("data bit=%d, clkbit=%d, csbit=%d\n",ddx,cdx,csdx);
	codata=(((0x6<<4)+(0xf<<16))+((0x0<<4)&0xfff0));

	valw=0xffff; bus_w(offw,(valw)); // start point
	valw=((valw&(~(0x1<<csdx))));bus_w(offw,valw); //chip sel bar down
	for (i=1;i<25;i++) {

		valw=(valw&(~(0x1<<cdx)));bus_w(offw,valw); //cldwn
		valw=((valw&(~(0x1<<ddx)))+(((codata>>(24-i))&0x1)<<ddx));bus_w(offw,valw);//write data (i)
		//	printf("%d ", ((codata>>(24-i))&0x1));


		valw=((valw&(~(0x1<<cdx)))+(0x1<<cdx));bus_w(offw,valw);//clkup
	}
	// printf("\n ");


	valw=((valw&(~(0x1<<csdx)))+(0x1<<csdx));bus_w(offw,valw); //csup

	valw=(valw&(~(0x1<<cdx)));bus_w(offw,valw); //cldwn




	valw=0xffff; bus_w(offw,(valw)); // stop point =start point of course */


	//end of setting int reference







}

int setDacRegister(int dacnum,int dacvalue) {
	int val;
	if (dacvalue==-100)
		dacvalue=0xffff;


	if (dacnum%2) {
		val=((dacvalue & 0xffff)<<16) | getDacRegister(dacnum-1);
	}  else {
		val=(getDacRegister(dacnum+1)<<16) | (dacvalue & 0xffff);

	}

	printf("Dac register %x wrote %08x\n",(DAC_REG_OFF+dacnum/2)<<11,val);
	bus_w((DAC_REG_OFF+dacnum/2)<<11, val);

	return getDacRegister(dacnum);


}
int getDacRegister(int dacnum) {

	int retval;

	retval=bus_r((DAC_REG_OFF+dacnum/2)<<11);
	printf("Dac register %x read %08x\n",(DAC_REG_OFF+dacnum/2)<<11,retval);
	if (dacnum%2)
		return (retval>>16)&0xffff;
	else
		return retval&0xffff;

}


int setDac(int dacnum,int dacvalue){
	printf("\nSetting of DAC %d with value %d\n",dacnum,dacvalue);

	u_int32_t offw,codata;
	u_int16_t valw;
	int i,ddx,csdx,cdx;

	int dacch=0;

	if (dacvalue>=0) {

		//setting int reference
		offw=DAC_REG;


		ddx=0; cdx=1;
		csdx=dacnum/8+2;

		dacch=dacnum%8;

		printf("data bit=%d, clkbit=%d, csbit=%d\n",ddx,cdx,csdx);
		//modified to power down single channels
		//   codata=((((0x2)<<4)+((dacch)&0xf))<<16)+((dacvalue<<4)&0xfff0);
		codata=((((0x3)<<4)+((dacch)&0xf))<<16)+((dacvalue<<4)&0xfff0);
		valw=0xffff; bus_w(offw,(valw)); // start point
		valw=((valw&(~(0x1<<csdx))));bus_w(offw,valw); //chip sel bar down
		for (i=1;i<25;i++) {
			valw=(valw&(~(0x1<<cdx)));bus_w(offw,valw); //cldwn
			valw=((valw&(~(0x1<<ddx)))+(((codata>>(24-i))&0x1)<<ddx));bus_w(offw,valw);//write data (i)
			//	printf("%d ", ((codata>>(24-i))&0x1));
			valw=((valw&(~(0x1<<cdx)))+(0x1<<cdx));bus_w(offw,valw);//clkup
		}
		// printf("\n ");
		valw=((valw&(~(0x1<<csdx)))+(0x1<<csdx));bus_w(offw,valw); //csup
		valw=(valw&(~(0x1<<cdx)));bus_w(offw,valw); //cldwn
		valw=0xffff; bus_w(offw,(valw)); // stop point =start point of course */

		//printf("Writing %d in DAC(0-15) %d \n",dacvalue,dacnum);
		printf("Writing %d in DAC %d \n",dacvalue,dacnum);
		setDacRegister(dacnum,dacvalue);
	} else if (dacvalue==-100) {

		printf("switching off dac %d\n", dacnum);
		csdx=dacnum/8+2;
		dacch=dacnum%8;
		ddx=0; cdx=1;
		codata=((((0x4)<<4)+((dacch)&0xf))<<16)+((dacvalue<<4)&0xfff0);
		valw=0xffff; bus_w(offw,(valw)); // start point
		valw=((valw&(~(0x1<<csdx))));bus_w(offw,valw); //chip sel bar down
		for (i=1;i<25;i++) {
			valw=(valw&(~(0x1<<cdx)));bus_w(offw,valw); //cldwn
			valw=((valw&(~(0x1<<ddx)))+(((codata>>(24-i))&0x1)<<ddx));bus_w(offw,valw);//write data (i)
			//	printf("%d ", ((codata>>(24-i))&0x1));
			valw=((valw&(~(0x1<<cdx)))+(0x1<<cdx));bus_w(offw,valw);//clkup
		}
		// printf("\n ");
		valw=((valw&(~(0x1<<csdx)))+(0x1<<csdx));bus_w(offw,valw); //csup
		valw=(valw&(~(0x1<<cdx)));bus_w(offw,valw); //cldwn
		valw=0xffff; bus_w(offw,(valw)); // stop point =start point of course */


		printf("Writing %d in DAC %d \n",dacvalue,dacnum);
		setDacRegister(dacnum,dacvalue);
	}


	return getDacRegister(dacnum);
}


void eraseFlash(){
#ifdef VERY_VERBOSE
	printf("\n at eraseFlash \n");
#endif

	char command[255];
	sprintf(command,"flash_eraseall %s",mtdvalue);
	system(command);
	printf("flash erased\n");
}


int startWritingFPGAprogram(FILE** filefp){
#ifdef VERY_VERBOSE
	printf("\n at startWritingFPGAprogram \n");
#endif

	//getting the drive
	char output[255];
	FILE* fp = popen("awk \'$4== \"\\\"bitfile(spi)\\\"\" {print $1}\' /proc/mtd", "r");
	fgets(output, sizeof(output), fp);
	pclose(fp);
	strcpy(mtdvalue,"/dev/");
	char* pch = strtok(output,":");
	if(pch == NULL){
		cprintf(RED,"Could not get mtd value\n");
		return FAIL;
	}
	strcat(mtdvalue,pch);
	printf ("\nFlash drive found: %s\n",mtdvalue);


	FPGAdontTouchFlash();

	//writing the program to flash
	*filefp = fopen(mtdvalue, "w");
	if(*filefp == NULL){
		cprintf(RED,"Unable to open %s in write mode\n",mtdvalue);
		return FAIL;
	}
	printf("flash ready for writing\n");

	return OK;
}

int stopWritingFPGAprogram(FILE* filefp){
#ifdef VERY_VERBOSE
	printf("\n at stopWritingFPGAprogram \n");
#endif

	int wait = 0;
	if(filefp!= NULL){
		fclose(filefp);
		wait = 1;
	}

	//touch and program
	FPGATouchFlash();

	if(wait){
#ifdef VERY_VERBOSE
		printf("Waiting for FPGA to program from flash\n");
#endif
		//waiting for success or done
		char output[255];
		int res=0;
		while(res == 0){
			FILE* sysFile = popen("cat /sys/class/gpio/gpio7/value", "r");
			fgets(output, sizeof(output), sysFile);
			pclose(sysFile);
			sscanf(output,"%d",&res);
#ifdef VERY_VERBOSE
			printf("gpi07 returned %d\n",res);
#endif
		}
	}
	printf("FPGA has picked up the program from flash\n\n");


	return OK;
}

int writeFPGAProgram(char* fpgasrc, size_t fsize, FILE* filefp){
#ifdef VERY_VERBOSE
	printf("\n at writeFPGAProgram \n");
	cprintf(BLUE,"address of fpgasrc:%p\n",(void *)fpgasrc);
	cprintf(BLUE,"fsize:%d\n",fsize);
	cprintf(BLUE,"pointer:%p\n",(void*)filefp);
#endif

	if(fwrite((void*)fpgasrc , sizeof(char) , fsize , filefp )!= fsize){
		cprintf(RED,"Could not write FPGA source to flash\n");
		return FAIL;
	}
#ifdef VERY_VERBOSE
	cprintf(BLUE,"program written to flash\n");
#endif
	return OK;
}



