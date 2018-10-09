
#include "server_defs.h"
#include "firmware_funcs.h"
#include "registers_g.h"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <math.h>


u_int64_t CSP0BASE;
int phase_shift=DEFAULT_PHASE_SHIFT;
int ipPacketSize=DEFAULT_IP_PACKETSIZE;
int udpPacketSize=DEFAULT_UDP_PACKETSIZE;


int timingMode=AUTO_TIMING;
enum externalSignalFlag signalMode=EXT_SIG_OFF;
int ROI_flag=0;
int adcConfigured=-1;
int detectorFirstServer = 1;
enum detectorSettings thisSettings;
ROI rois[MAX_ROIS];
int nROI=0;

//for the 25um detectors
int masterflags = NO_MASTER;
int masterdefaultdelay = 62;
int patternphase = 0;
int adcphase = 0;
int slavepatternphase = 0;
int slaveadcphase = 0;
int rsttosw1delay = 2;
int startacqdelay = 1;


sls_detector_module *detectorModules=NULL;
int *detectorDacs=NULL;
int *detectorAdcs=NULL;


/**
   ENEt conf structs
 */
typedef struct mac_header_struct{
	u_int8_t    mac_dest_mac2;
	u_int8_t    mac_dest_mac1;
	u_int8_t    mac_dummy1;
	u_int8_t    mac_dummy2;
	u_int8_t    mac_dest_mac6;
	u_int8_t    mac_dest_mac5;
	u_int8_t    mac_dest_mac4;
	u_int8_t    mac_dest_mac3;
	u_int8_t    mac_src_mac4;
	u_int8_t    mac_src_mac3;
	u_int8_t    mac_src_mac2;
	u_int8_t    mac_src_mac1;
	u_int16_t   mac_ether_type;
	u_int8_t    mac_src_mac6;
	u_int8_t    mac_src_mac5;
} mac_header;

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

typedef struct udp_header_struct{
	u_int16_t   udp_destport;
	u_int16_t   udp_srcport;
	u_int16_t   udp_chksum;
	u_int16_t   udp_len;
} udp_header;

typedef struct mac_conf_struct{
	mac_header  mac;
	ip_header   ip;
	udp_header  udp;
	u_int32_t   npack;
	u_int32_t   lpack;
	u_int32_t   npad;
	u_int32_t   cdone;
} mac_conf;

typedef struct tse_conf_struct{
	u_int32_t   rev;                    //0x0
	u_int32_t   scratch;
	u_int32_t   command_config;
	u_int32_t   mac_0;                  //0x3
	u_int32_t   mac_1;
	u_int32_t   frm_length;
	u_int32_t   pause_quant;
	u_int32_t   rx_section_empty;       //0x7
	u_int32_t   rx_section_full;
	u_int32_t   tx_section_empty;
	u_int32_t   tx_section_full;
	u_int32_t   rx_almost_empty;        //0xB
	u_int32_t   rx_almost_full;
	u_int32_t   tx_almost_empty;
	u_int32_t   tx_almost_full;
	u_int32_t   mdio_addr0;             //0xF
	u_int32_t   mdio_addr1;
}tse_conf;



int mapCSP0(void) {
	printf("Mapping memory\n");
#ifndef VIRTUAL
	int fd;
	fd = open("/dev/mem", O_RDWR | O_SYNC, 0);
	if (fd == -1) {
		printf("\nCan't find /dev/mem!\n");
		return FAIL;
	}
	printf("/dev/mem opened\n");

	CSP0BASE = (u_int32_t)mmap(0, MEM_SIZE, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, CSP0);
	if (CSP0BASE == (u_int32_t)MAP_FAILED) {
		printf("\nCan't map memmory area!!\n");
		return FAIL;
	}
	printf("CSP0 mapped\n");

#endif
#ifdef VIRTUAL
	CSP0BASE = malloc(MEM_SIZE);
	printf("memory allocated\n");
#endif
	printf("CSPObase is 0x%llx \n",CSP0BASE);
	printf("CSPOBASE=from %llx to %llx\n",CSP0BASE,CSP0BASE+MEM_SIZE);

	printf("statusreg=%08x\n",bus_r(STATUS_REG));
	printf("\n\n");
	return OK;
}

u_int16_t bus_r16(u_int32_t offset){
	volatile u_int16_t *ptr1;
	ptr1=(u_int16_t*)(CSP0BASE+offset*2);
	return *ptr1;
}

u_int16_t bus_w16(u_int32_t offset, u_int16_t data) {
	volatile u_int16_t *ptr1;
	ptr1=(u_int16_t*)(CSP0BASE+offset*2);
	*ptr1=data;
	return OK;
}


u_int32_t bus_w(u_int32_t offset, u_int32_t data) {
	volatile u_int32_t *ptr1;

	ptr1=(u_int32_t*)(CSP0BASE+offset*2);
	*ptr1=data;

	return OK;
}


u_int32_t bus_r(u_int32_t offset) {
	volatile u_int32_t *ptr1;

	ptr1=(u_int32_t*)(CSP0BASE+offset*2);
	return *ptr1;
}



int initDetector() {
#ifdef VERBOSE
	printf("Board is for 1 module\n");
#endif
	detectorModules=malloc(sizeof(sls_detector_module));
	detectorDacs=malloc(NDAC*sizeof(int));
	detectorAdcs=malloc(NADC*sizeof(int));
#ifdef VERBOSE
	printf("modules from 0x%x to 0x%x\n",(unsigned int)(detectorModules), (unsigned int)(detectorModules));
	printf("dacs from 0x%x to 0x%x\n",(unsigned int)(detectorDacs), (unsigned int)(detectorDacs));
	printf("adcs from 0x%x to 0x%x\n",(unsigned int)(detectorAdcs), (unsigned int)(detectorAdcs));
#endif
	(detectorModules)->dacs=detectorDacs;
	(detectorModules)->adcs=detectorAdcs;
	(detectorModules)->ndac=NDAC;
	(detectorModules)->nadc=NADC;
	(detectorModules)->nchip=NCHIP;
	(detectorModules)->nchan=NCHIP*NCHAN;
	(detectorModules)->gain=0;
	(detectorModules)->offset=0;
	(detectorModules)->reg=0;


	thisSettings=UNINITIALIZED;

	testFpga();

	//gotthard specific
	setPhaseShiftOnce();
	configureADC();
	setADC(-1); //already does setdaqreg and clean fifo.

	setSettings(DYNAMICGAIN);
	setDefaultDacs();


	//Initialization
	setFrames(1);
	setTrains(1);
	setExposureTime(1e6);
	setPeriod(1e9);
	setDelay(0);
	setGates(0);
	setTiming(GET_EXTERNAL_COMMUNICATION_MODE);
	sendviaUDP(1);
	setMasterSlaveConfiguration();

	return OK;
}


int setDefaultDacs() {
	printf("Setting Default Dac values\n");

	int ret = OK;
	int i = 0;
	int retval[2]={-1,-1};
	const int defaultvals[NDAC] = DEFAULT_DAC_VALS;

	for(i = 0; i < NDAC; ++i) {
		// if not already default, set it to default
		if (setDACRegister(i,  -1) != defaultvals[i]) {
			setDAC(i, defaultvals[i], 0, retval);
			if (abs(retval[0] - defaultvals[i])<=3) {
				cprintf(RED, "Warning: Setting dac %d failed, wrote %d, read %d\n",i ,defaultvals[i], retval[0]);
				ret = FAIL;
			}
		}
	}

	return ret;
}





void setMasterSlaveConfiguration(){

	// global master default delay picked from config file
	FILE* fd=fopen(CONFIG_FILE,"r");
	if(fd==NULL){
		cprintf(RED,"\nWarning: Could not open file\n");
		return;
	}
	cprintf(BLUE,"config file %s opened\n", CONFIG_FILE);

	char key[256];
	char value[256];
	char line[256];
	int ival=0;
	u_int32_t val=0;

	while (fgets(line, sizeof(line), fd)) {
		if(line[0] == '#')
			continue;
		sscanf(line, "%s %s\n", key, value);
		if	(!strcasecmp(key,"masterflags")) {
			if	(!strcasecmp(value,"is_master")) {
				masterflags = IS_MASTER;
			}
			else if (!strcasecmp(value,"is_slave")) {
				masterflags = IS_SLAVE;
			}
			else  if (!strcasecmp(value,"no_master")){
				masterflags = NO_MASTER;
			}
			else {
				cprintf(RED,"could not scan masterflags %s value from config file\n",value);
				fclose(fd);
				exit(EXIT_FAILURE);
			}

			if (!detectorFirstServer) {
				cprintf(BLUE, "Server has been started up before. Ignoring rest of config file\n");
				fclose(fd);
				return;
			}
		}
		else {
			if(sscanf(value,"%d",&ival)<=0) {
				cprintf(RED,"could not scan patternphase %s value from config file\n",value);
				fclose(fd);
				exit(EXIT_FAILURE);
			}

			if (!strcasecmp(key,"masterdefaultdelay"))
				masterdefaultdelay = ival;
			else if (!strcasecmp(key,"patternphase"))
				patternphase = ival;
			else if (!strcasecmp(key,"adcphase"))
				adcphase = ival;
			else if (!strcasecmp(key,"slavepatternphase"))
				slavepatternphase = ival;
			else if (!strcasecmp(key,"slaveadcphase"))
				slaveadcphase = ival;
			else if (!strcasecmp(key,"rsttosw1delay"))
				rsttosw1delay = ival;
			else if (!strcasecmp(key,"startacqdelay"))
				startacqdelay = ival;
			else {
				cprintf(RED,"could not scan parameter name %s from config file\n",key);
				fclose(fd);
				exit(EXIT_FAILURE);
			}
		}

	}
	cprintf(BLUE,
			"masterflags: %d\n"
			"masterdefaultdelay:%d\n"
			"patternphase:%d\n"
			"adcphase:%d\n"
			"slavepatternphase:%d\n"
			"slaveadcphase:%d\n"
			"rsttosw1delay:%d\n"
			"startacqdelay:%d\n",
			masterflags,
			masterdefaultdelay,
			patternphase,
			adcphase,
			slavepatternphase,
			slaveadcphase,
			rsttosw1delay,
			startacqdelay);



	if (masterflags == IS_MASTER) {
		// set delay
		setDelay(0);

		/* Set pattern phase for the master module */
		val=bus_r(MULTI_PURPOSE_REG);
		val = (val & (~(PLL_CLK_SEL_MSK))) | PLL_CLK_SEL_MASTER_VAL;
		bus_w(MULTI_PURPOSE_REG,val);
		setPhaseShift(patternphase);
		/* Set adc phase for the master module */
		val=bus_r(MULTI_PURPOSE_REG);
		val = (val & (~(PLL_CLK_SEL_MSK))) | PLL_CLK_SEL_MASTER_ADC_VAL;
		bus_w(MULTI_PURPOSE_REG,val);
		setPhaseShift(adcphase);
		/* Set pattern phase for the slave module */
		val=bus_r(MULTI_PURPOSE_REG);
		val = (val & (~(PLL_CLK_SEL_MSK))) | PLL_CLK_SEL_SLAVE_VAL;
		bus_w(MULTI_PURPOSE_REG,val);
		setPhaseShift(slavepatternphase);
		/* Set adc phase for the slave module */
		val=bus_r(MULTI_PURPOSE_REG);
		val = (val & (~(PLL_CLK_SEL_MSK))) | PLL_CLK_SEL_SLAVE_ADC_VAL;
		bus_w(MULTI_PURPOSE_REG,val);
		setPhaseShift(slaveadcphase);
		/* Set start acq delay */
		val=bus_r(MULTI_PURPOSE_REG);
#ifdef VERBOSE
		printf("Multipurpose reg:0x%x\n",bus_r(MULTI_PURPOSE_REG));
#endif
		val = (val & (~(START_ACQ_DELAY_MSK))) | ((startacqdelay << START_ACQ_DELAY_OFFSET) & (START_ACQ_DELAY_MSK));
		bus_w(MULTI_PURPOSE_REG,val);
		printf("Start acq delay set. Multipurpose reg: 0x%x\n",bus_r(MULTI_PURPOSE_REG));
	}

	/* Set RST to SW1 delay */
	val=bus_r(MULTI_PURPOSE_REG);
#ifdef VERBOSE
	printf("Multipurpose reg:0x%x\n",bus_r(MULTI_PURPOSE_REG));
#endif
	val = (val & (~(RST_TO_SW1_DELAY_MSK))) | ((rsttosw1delay << RST_TO_SW1_DELAY_OFFSET) & (RST_TO_SW1_DELAY_MSK));
	bus_w(MULTI_PURPOSE_REG,val);
	printf("RST to SW1 delay set. Multipurpose reg:0x%x\n",bus_r(MULTI_PURPOSE_REG));

	fclose(fd);
}




int configureADC(){
	printf("Preparing ADC\n");
	u_int32_t valw,codata,csmask;
	int i,j,cdx,ddx;
	cdx=0; ddx=1;
	csmask=0x7c; //  1111100

	for(j=0;j<3;j++){
		//command and value;
		codata = 0;
		if(j==0)
			codata=(0x08<<8)+(0x3);//Power modes(global) //reset
		else if(j==1)
			codata=(0x08<<8)+(0x0);//Power modes(global) //chip run
		else
			codata = (0x14<<8)+(0x0);//Output mode //offset binary


		// start point
		valw=0xffffffff;
		bus_w(ADC_SPI_REG,(valw));

		//chip sel bar down
		valw=((0xffffffff&(~csmask)));
		bus_w(ADC_SPI_REG,valw);

		for (i=0;i<24;i++) {
			//cldwn
			valw=valw&(~(0x1<<cdx));
			bus_w(ADC_SPI_REG,valw);
			//usleep(0);

			//write data (i)
			valw=(valw&(~(0x1<<ddx)))+(((codata>>(23-i))&0x1)<<ddx);
			bus_w(ADC_SPI_REG,valw);
			//usleep(0);

			//clkup
			valw=valw+(0x1<<cdx);
			bus_w(ADC_SPI_REG,valw);
			//usleep(0);
		}

		valw |= csmask;
		bus_w(ADC_SPI_REG,valw);
		//usleep(0);

		// stop point =start point
		valw=valw&(~(0x1<<cdx));
		bus_w(ADC_SPI_REG,(valw));

		valw = 0xffffffff;
		bus_w(ADC_SPI_REG,(valw));

		//usleep in between
		usleep(50000);
	}

	return OK;
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
		detectorFirstServer = 1;
		printf("\nImplementing phase shift of %d\n",phase_shift);
		for (i=1;i<phase_shift;i++) {
			bus_w(addr,(INT_RSTN_BIT|ENET_RESETN_BIT|SW1_BIT|PHASE_STEP_BIT));//0x2821
			bus_w(addr,(INT_RSTN_BIT|ENET_RESETN_BIT|(SW1_BIT&~PHASE_STEP_BIT)));//0x2820
		}
#ifdef VERBOSE
		printf("Multipupose reg now:%x\n",bus_r(addr));
#endif
	} else detectorFirstServer = 0;

	return OK;
}




int setPhaseShift(int numphaseshift){
	u_int32_t addr, reg;
	int i;
	addr=MULTI_PURPOSE_REG;
	reg=bus_r(addr);
#ifdef VERBOSE
	printf("Multipurpose reg:%x\n",reg);
#endif

	printf("\nImplementing phase shift of %d\n",numphaseshift);
	for (i=0;i<numphaseshift;i++) {
		bus_w(addr,reg | PHASE_STEP_BIT);
		bus_w(addr,reg & (~PHASE_STEP_BIT));
	}

#ifdef VERBOSE
	printf("Multipupose reg now:%x\n",bus_r(addr));
#endif

	return OK;
}





int cleanFifo(){
	u_int32_t addr, reg, val;
	printf("Cleaning FIFO\n");
	addr=ADC_SYNC_REG;

	//88332214
	if (ROI_flag==0) {
		val=ADCSYNC_VAL | ADCSYNC_CLEAN_FIFO_BITS | TOKEN_RESTART_DELAY;
		bus_w(addr,val);
		//88032214
		val=ADCSYNC_VAL | TOKEN_RESTART_DELAY;
		bus_w(addr,val);
	}
	else {
		//1b332214
		val=ADCSYNC_VAL | ADCSYNC_CLEAN_FIFO_BITS | TOKEN_RESTART_DELAY_ROI;
		bus_w(addr,val);
		//1b032214
		val=ADCSYNC_VAL | TOKEN_RESTART_DELAY_ROI;
		bus_w(addr,val);

	}
	reg=bus_r(addr);

	printf("ADC SYNC reg 0x19:%x\n",reg);
	return OK;
}


int setDAQRegister()
{
	u_int32_t addr, reg, val;
	addr=DAQ_REG;

	//depended on adcval
	int packetlength=0x7f;
	if(!ROI_flag) packetlength=0x13f;

	//depended on pcb rev
	int tokenTiming = TOKEN_TIMING_REV2;
	if((bus_r(PCB_REV_REG)&BOARD_REVISION_MASK)==1)
		tokenTiming= TOKEN_TIMING_REV1;


	val = (packetlength<<16) + tokenTiming;
	//val=34+(42<<8)+(packetlength<<16);

	reg=bus_r(addr);
	bus_w(addr,val);
	reg=bus_r(addr);
	//#ifdef VERBOSE
	printf("DAQ reg 0x15:%x\n",reg);
	//#endif

	return OK;
}


// direct pattern output
u_int32_t putout(char *s) {
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
	addr=MCB_CNTRL_REG_OFF;
	bus_w(addr, pat);

	return OK;
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




int sendviaUDP(int start) {
	u_int32_t addr=CONFIG_REG;
#ifdef VERBOSE
	if(start)
		printf("Setting up detector to send to Receiver\n");
	else
		printf("Setting up detector to send to CPU\n");
#endif
	int reg=bus_r(addr);
	//for start recever, write 0 and for stop, write 1
	if (!start)
		bus_w(CONFIG_REG,reg|CPU_OR_RECEIVER_BIT);
	else
		bus_w(CONFIG_REG,reg&(~CPU_OR_RECEIVER_BIT));

	reg=bus_r(addr);
	//#ifdef VERBOSE
	printf("Config Reg %x\n", reg);
	//#endif
	int d =reg&CPU_OR_RECEIVER_BIT;
	if(d!=0) d=1;
	if(d!=start)
		return OK;
	else
		return FAIL;
}






int setDACRegister(int idac, int val) {
	u_int32_t addr, reg, mask;
	int off;
#ifdef VERBOSE
	if(val==-1)
		printf("Getting dac register%d \n",idac);
	else
		printf("Setting dac register %d to %d\n",idac,val);
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
		reg=bus_r(addr+(0<<SHIFTMOD));
		reg&=mask;
		reg|=(val<<off);
		bus_w(addr+(0<<SHIFTMOD),reg);
	}
	val=(bus_r(addr+(0<<SHIFTMOD))>>off)&0x3ff;
	//since we saved only the msb
	val=val<<2;

	//val=(bus_r(addr)>>off)&0x3ff;


#ifdef VERBOSE
	printf("Dac %d register is %d\n\n",idac,val);
#endif
	return val;
}









u_int32_t setExtSignal(enum externalSignalFlag  mode) {

	u_int32_t c;
	c = bus_r(EXT_SIGNAL_REG);

#ifdef VERBOSE
	printf("settings signal variable number %d to value %04x\n", d, signals[d]);
#endif

	switch (mode) {
	case TRIGGER_IN_RISING_EDGE:
	case TRIGGER_IN_FALLING_EDGE:

		// set variable
		signalMode = mode;

		// set signal only if trigger mode
		if (timingMode==TRIGGER_EXPOSURE)
			setFPGASignal(mode);
		// switch off if not trigger mode, but variable remembers value
		else
			setFPGASignal(SIGNAL_OFF);
		break;

	default:
		mode = SIGNAL_OFF;
		signalMode = mode;
		setFPGASignal(mode);
		break;
	}

	return getExtSignal();
}


int getExtSignal() {
	return signalMode;
}


u_int32_t setFPGASignal(enum externalSignalFlag  mode) {

	u_int32_t c = bus_r(EXT_SIGNAL_REG);
	// offset is 0 as we only deal with the first signal index now. //int off = d * SIGNAL_OFFSET;

	// check and then write
	if ((mode == SIGNAL_OFF) || (mode == TRIGGER_IN_RISING_EDGE) || (mode == TRIGGER_IN_FALLING_EDGE)) {
#ifdef VERBOSE
		printf("writing signal register number %d mode %04x\n",0, (int) mode);
#endif
		bus_w(EXT_SIGNAL_REG,( (c &~ (SIGNAL_MASK))  | ((int)mode) ));
	}
	return getFPGASignal();
}




int getFPGASignal() {

	// offset is 0 as we only deal with the first signal index now. //int off = d * SIGNAL_OFFSET;
	int mode = ((bus_r(EXT_SIGNAL_REG)) & SIGNAL_MASK);

	// check and then update variable
	if ((mode == SIGNAL_OFF) || (mode == TRIGGER_IN_RISING_EDGE) || (mode == TRIGGER_IN_FALLING_EDGE)) {
#ifdef VERYVERBOSE
		printf("gettings signal register number %d  value %04x\n", d, (int)mode);
#endif
		return (int)mode;
	} else
		return -1;
}





int setTiming(int ti) {

	// set
	if (ti != GET_EXTERNAL_COMMUNICATION_MODE) {

		// trigger
		if (ti == TRIGGER_EXPOSURE) {
			timingMode = ti;
			if ((signalMode == TRIGGER_IN_RISING_EDGE) || (signalMode == TRIGGER_IN_FALLING_EDGE))
				setFPGASignal(signalMode);
			else
				setFPGASignal(SIGNAL_OFF); // only if both (timing & extsig) configured, its set to trigger, else off
		}

		// auto
		else {
			timingMode = AUTO_TIMING;
			setFPGASignal(SIGNAL_OFF);
		}
	}

	// get
	int s = getFPGASignal();
	if ((s == TRIGGER_IN_RISING_EDGE) || (s == TRIGGER_IN_FALLING_EDGE))
		return TRIGGER_EXPOSURE;
	return AUTO_TIMING;
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


int initHighVoltage(int val){
#ifdef VERBOSE
	printf("Setting/Getting High Voltage with val:%d\n",val);
#endif
	volatile u_int32_t addr=HV_REG;
	int writeVal,writeVal2;
	switch(val){
	case -1: break;
	case 0:  writeVal=0x0; writeVal2=0x0; break;
	case 90: writeVal=0x0; writeVal2=0x1; break;
	case 110:writeVal=0x2; writeVal2=0x3; break;
	case 120:writeVal=0x4; writeVal2=0x5; break;
	case 150:writeVal=0x6; writeVal2=0x7; break;
	case 180:writeVal=0x8; writeVal2=0x9; break;
	case 200:writeVal=0xA; writeVal2=0xB; break;
	default :printf("Invalid voltage\n");return -2;break;
	}
	//to set value
	if(val!=-1){
		//set value to converted value
		bus_w(addr,writeVal);
		bus_w(addr,writeVal2);
#ifdef VERBOSE
		printf("Value sent is %d and then %d\n",writeVal,writeVal2);
#endif 
	}
	//read value and return the converted value
	val=bus_r(addr);
#ifdef VERBOSE
	printf("Value read from reg is %d\n",val);
#endif 
	switch(val){
	case 0x0:val=0;break;
	case 0x1:val=90;break;
	case 0x3:val=110;break;
	case 0x5:val=120;break;
	case 0x7:val=150;break;
	case 0x9:val=180;break;
	case 0xB:val=200;break;
	default:printf("Weird value read:%d\n",val);return -3;break;
	}
#ifdef VERBOSE
	printf("High voltage is %d\n",val);
#endif  
	return val;
}






int getTemperature(int tempSensor){
	int val;
	int i,j,repeats=6;
	u_int32_t tempVal=0;
#ifdef VERBOSE
	char cTempSensor[2][100]={"ADCs/ASICs","VRs/FPGAs"};
	printf("Getting Temperature for the %s for tempsensor:%d\n",cTempSensor[tempSensor],tempSensor);
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
	printf("Temperature for the %s is %.2fC\n",cTempSensor[tempSensor],val);
#endif
	return val;
}




int setSettings(int i) {
#ifdef VERBOSE
	if(i==-1)
		printf("\nReading settings of detector...\n");
	else
		printf("\ninside set settings wit settings=%d...\n",i);
#endif
	int confgain[] = CONF_GAIN;
	int isett=-2,retval;

	//reading settings
	if(i==GET_SETTINGS){
		retval=initConfGain(i,i);
		if(retval==i)
			isett=UNDEFINED;
	}
	//writing settings
	else{
		retval=initConfGain(i,confgain[i]);
		if(retval!=i)
			isett=UNDEFINED;
	}
	//if error while read/writing
	if(isett==UNDEFINED)
		printf("Error:Weird Value read back from the Gain/Settings Reg\n");
	else{
		//validating the settings read back
		if((retval>=HIGHGAIN)&&(retval<=VERYHIGHGAIN))
			isett=retval;
		else{
			isett=UNDEFINED;
			printf("Error:Wrong Settings Read out:%d\n",retval);
		}
	}
	thisSettings=isett;
	//#ifdef VERBOSE
	printf("detector settings are %d\n",thisSettings);
	//#endif
	return thisSettings;
}





int initConfGain(int isettings,int val){
	int retval;
	u_int32_t addr=GAIN_REG;

	if(val!=-1){
#ifdef VERBOSE
		printf("Setting Gain with val:%d\n",val);
#endif
		bus_w(addr,((val<<GAIN_OFFSET)|(bus_r(addr)&~GAIN_MASK)));
	}
	retval=(bus_r(addr)&GAIN_MASK);
#ifdef VERBOSE
	printf("Value read from Gain reg is %d\n",retval);
#endif
	if((val!=-1)&&(retval!=val))
		return -1;

	if(isettings!=-1){
#ifdef VERBOSE
		printf("Writing Settings with val:%d\n",isettings);
#endif
		bus_w(addr,((isettings<<SETTINGS_OFFSET)|(bus_r(addr)&~SETTINGS_MASK)));
	}
	retval=((bus_r(addr)&SETTINGS_MASK)>>SETTINGS_OFFSET);
#ifdef VERBOSE
	printf("Settings read from reg is %d\n",retval);
#endif
	if((isettings!=-1)&&(retval!=isettings)){
		printf("\n\nSettings r\n\n");
		return -1;
	}

	return retval;
}



ROI* setROI(int n, ROI arg[], int *retvalsize, int *ret){

	int i,adc;
	ROI temp;

	if(n>=0){

		//clear rois
		for(i=0;i<MAX_ROIS;i++)
			rois[i]=temp;

		if(n==0)
			adc=-1;
		else{
			//if its for 1 adc or general
			if ((arg[0].xmin==0) && (arg[0].xmax==NCHIP*NCHAN))
				adc=-1;
			else{
				//adc = mid value/numchans also for only 1 roi
				adc = ((((arg[0].xmax)+(arg[0].xmin))/2)/(NCHAN*NCHIPS_PER_ADC));
				if((adc>=0) && (adc<=4));
				else {
					printf("warning:adc value greater than 5. deleting roi\n");
					adc=-1;
				}
			}
		}
		printf("\tGoing to enable adc: %d\n", adc);

		//set rois for just 1 adc - take only 1st roi
		if(adc!=-1){
			rois[0].xmin=adc*(NCHAN*NCHIPS_PER_ADC);
			rois[0].xmax=(adc+1)*(NCHAN*NCHIPS_PER_ADC)-1;
			rois[0].ymin=-1;
			rois[0].ymax=-1;
			nROI = 1;
			printf("\tActual xmin:%d xmax:%d\n",rois[0].xmin,rois[0].xmax);
		}else
			nROI = 0;

		if((n!=0) && ((arg[0].xmin!=rois[0].xmin)||
				(arg[0].xmax!=rois[0].xmax)||
				(arg[0].ymin!=rois[0].ymin)||
				(arg[0].ymax!=rois[0].ymax)))
			*ret=FAIL;
		if(n!=nROI)
			*ret=FAIL;

		//set adc of interest
		setADC(adc);
	}

	//#ifdef VERBOSE
	if (nROI) {
		printf("Rois:\n");
		for( i=0;i<nROI;i++)
			printf("\t%d\t%d\t%d\t%d\n\n",rois[i].xmin,rois[i].xmax,rois[i].ymin,rois[i].ymax);
	}else printf("Rois: 0\n\n");
	//#endif
	*retvalsize = nROI;
	return rois;
}






int setADC(int adc){
	int reg,nchips,mask;

	if(adc==-1)	ROI_flag=0;
	else		ROI_flag=1;

	setDAQRegister();//token timing
	cleanFifo();//adc sync

	//all adc
	if(adc==-1){
		//set packet size
		ipPacketSize= DEFAULT_IP_PACKETSIZE;
		udpPacketSize=DEFAULT_UDP_PACKETSIZE;
		//set channel mask
		nchips = NCHIP;
		mask = ACTIVE_ADC_MASK;
	}
	//1 adc
	else{
		ipPacketSize= ADC1_IP_PACKETSIZE;
		udpPacketSize=ADC1_UDP_PACKETSIZE;
		//set channel mask
		nchips = NCHIPS_PER_ADC;
		mask = 1<<adc;
	}

	//set channel mask
	reg = (NCHAN*nchips)<<CHANNEL_OFFSET;
	reg&=CHANNEL_MASK;
	reg|=(ACTIVE_ADC_MASK & mask);
	bus_w(CHIP_OF_INTRST_REG,reg);

	printf("Chip of Interest:%x\n",bus_r(CHIP_OF_INTRST_REG));

	adcConfigured = adc;

	return adcConfigured;
}



int configureMAC(int ipad,long long int macad,long long int detectormacad, int detipad, int ival, int udpport){

#ifdef VERBOSE
	printf("Chip of Intrst Reg:%x\n",bus_r(CHIP_OF_INTRST_REG));
	printf("IP Packet Size:%d\n",ipPacketSize);
	printf("UDP Packet Size:%d\n",udpPacketSize);
#endif

	//configuring mac
	u_int32_t addrr=MULTI_PURPOSE_REG;
	u_int32_t offset=ENET_CONF_REG, offset2=TSE_CONF_REG;
	mac_conf *mac_conf_regs;
	tse_conf *tse_conf_regs;
	long int sum = 0;
	long int checksum;
	int count;
	unsigned short *addr;

	mac_conf_regs=(mac_conf*)(CSP0BASE+offset*2);
	tse_conf_regs=(tse_conf*)(CSP0BASE+offset2*2);

	printf("***Configuring MAC*** \n");


	if(ival)
		bus_w(addrr, bus_r(addrr) | (RESET_BIT|DIGITAL_TEST_BIT)); //0x080,reset mac (reset)
	else
		bus_w(addrr, bus_r(addrr) | RESET_BIT); //0x080,reset mac (reset)

#ifdef VERBOSE
	printf("Value read from Multi-purpose Reg:%x\n",bus_r(addrr));
#endif 
	//  if(val!=0x080) return -1;

	usleep(500000);

	bus_w(addrr, bus_r(addrr) &(~ RESET_BIT));/* release reset */

	if(ival)
		bus_w(addrr, bus_r(addrr) | (ENET_RESETN_BIT|WRITE_BACK_BIT|DIGITAL_TEST_BIT)); //0x840,write shadow regs(enet reset,write bak)
	else
		bus_w(addrr, bus_r(addrr) | (ENET_RESETN_BIT|WRITE_BACK_BIT)); //0x840,write shadow regs(enet reset,write bak)

#ifdef VERBOSE
	printf("Value read from Multi-purpose Reg:%x\n",bus_r(addrr));
#endif 
	//  if(val!=0x840) return -1;

	bus_w(addrr, bus_r(addrr) &(~WRITE_BACK_BIT));/* release write_back */

	if(ival)
		bus_w(addrr, bus_r(addrr) | (ENET_RESETN_BIT|DIGITAL_TEST_BIT)); //0x800,nreset phy(enet reset)
	else
		bus_w(addrr, bus_r(addrr) | ENET_RESETN_BIT); //0x800,nreset phy(enet reset)

#ifdef VERBOSE
	printf("Value read from Multi-purpose Reg:%x\n",bus_r(addrr));
#endif 
	//  if(val!=0x800) return -1;


	mac_conf_regs->mac.mac_dest_mac1  =((macad>>(8*5))&0xFF);// 0x00; //pc7060
	mac_conf_regs->mac.mac_dest_mac2  =((macad>>(8*4))&0xFF);// 0x19; //pc7060
	mac_conf_regs->mac.mac_dest_mac3  =((macad>>(8*3))&0xFF);// 0x99; //pc7060
	mac_conf_regs->mac.mac_dest_mac4  =((macad>>(8*2))&0xFF);// 0x24; //pc7060
	mac_conf_regs->mac.mac_dest_mac5  =((macad>>(8*1))&0xFF);// 0xEB; //pc7060
	mac_conf_regs->mac.mac_dest_mac6  =((macad>>(8*0))&0xFF);// 0xEE; //pc7060
	/*
  mac_conf_regs->mac.mac_src_mac1   = 0x00;     
  mac_conf_regs->mac.mac_src_mac2   = 0xAA;     
  mac_conf_regs->mac.mac_src_mac3   = 0xBB;     
  mac_conf_regs->mac.mac_src_mac4   = 0xCC;     
  mac_conf_regs->mac.mac_src_mac5   = 0xDD;     
  mac_conf_regs->mac.mac_src_mac6   = 0xEE;     
	 */
	mac_conf_regs->mac.mac_src_mac1  =((detectormacad>>(8*5))&0xFF);
	mac_conf_regs->mac.mac_src_mac2  =((detectormacad>>(8*4))&0xFF);
	mac_conf_regs->mac.mac_src_mac3  =((detectormacad>>(8*3))&0xFF);
	mac_conf_regs->mac.mac_src_mac4  =((detectormacad>>(8*2))&0xFF);
	mac_conf_regs->mac.mac_src_mac5  =((detectormacad>>(8*1))&0xFF);
	mac_conf_regs->mac.mac_src_mac6  =((detectormacad>>(8*0))&0xFF);
	mac_conf_regs->mac.mac_ether_type   = 0x0800;   //ipv4


	mac_conf_regs->ip.ip_ver            = 0x4;
	mac_conf_regs->ip.ip_ihl            = 0x5;
	mac_conf_regs->ip.ip_tos            = 0x0;
	mac_conf_regs->ip.ip_len            = ipPacketSize;//0x0522;   // was 0x0526;
	mac_conf_regs->ip.ip_ident          = 0x0000;
	mac_conf_regs->ip.ip_flag           = 0x2;
	mac_conf_regs->ip.ip_offset         = 0x00;
	mac_conf_regs->ip.ip_ttl            = 0x70;
	mac_conf_regs->ip.ip_protocol       = 0x11;
	mac_conf_regs->ip.ip_chksum         = 0x0000 ; //6E42 now is automatically computed
	mac_conf_regs->ip.ip_sourceip       = detipad; //0x8181CA2E;129.129.202.46
	mac_conf_regs->ip.ip_destip         = ipad; //CA57

#ifdef VERBOSE
	printf("mac_dest:%llx %x:%x:%x:%x:%x:%x\n",
			macad,
			mac_conf_regs->mac.mac_dest_mac1,
			mac_conf_regs->mac.mac_dest_mac2,
			mac_conf_regs->mac.mac_dest_mac3,
			mac_conf_regs->mac.mac_dest_mac4,
			mac_conf_regs->mac.mac_dest_mac5,
			mac_conf_regs->mac.mac_dest_mac6);
	printf("mac_src:%llx %x:%x:%x:%x:%x:%x\n",
			detectormacad,
			mac_conf_regs->mac.mac_src_mac1,
			mac_conf_regs->mac.mac_src_mac2,
			mac_conf_regs->mac.mac_src_mac3,
			mac_conf_regs->mac.mac_src_mac4,
			mac_conf_regs->mac.mac_src_mac5,
			mac_conf_regs->mac.mac_src_mac6);
	printf("ip_ttl:%x\n",mac_conf_regs->ip.ip_ttl);
#endif

	//checksum
	count=sizeof(mac_conf_regs->ip);
	addr=&(mac_conf_regs->ip);
	while( count > 1 )  {
		sum += *addr++;
		count -= 2;
	}
	if( count > 0 )  sum += *addr;                     // Add left-over byte, if any
	while (sum>>16) sum = (sum & 0xffff) + (sum >> 16);// Fold 32-bit sum to 16 bits
	checksum = (~sum)&0xffff;
	mac_conf_regs->ip.ip_chksum   =  checksum;
	//#ifdef VERBOSE
	printf("IP header checksum is 0x%x s\n",(unsigned int)(checksum));
	//#endif

	mac_conf_regs->udp.udp_srcport      = 0xE185;
	mac_conf_regs->udp.udp_destport     = udpport;//0xC351;
	mac_conf_regs->udp.udp_len          = udpPacketSize;//0x050E;    //was  0x0512;
	mac_conf_regs->udp.udp_chksum       = 0x0000;

#ifdef VERBOSE
	printf("Configuring TSE\n");
#endif
	tse_conf_regs->rev                 = 0xA00;
	tse_conf_regs->scratch             = 0xCCCCCCCC;
	tse_conf_regs->command_config      = 0xB;
	tse_conf_regs->mac_0               = 0x17231C00;
	tse_conf_regs->mac_1               = 0xCB4A;
	tse_conf_regs->frm_length          = 0x5DC;      //max frame length (1500 bytes) (was 0x41C)
	tse_conf_regs->pause_quant         = 0x0;
	tse_conf_regs->rx_section_empty    = 0x7F0;
	tse_conf_regs->rx_section_full     = 0x10;
	tse_conf_regs->tx_section_empty    = 0x3F8;      //was 0x7F0;
	tse_conf_regs->tx_section_full     = 0x16;
	tse_conf_regs->rx_almost_empty     = 0x8;
	tse_conf_regs->rx_almost_full      = 0x8;
	tse_conf_regs->tx_almost_empty     = 0x8;
	tse_conf_regs->tx_almost_full      = 0x3;
	tse_conf_regs->mdio_addr0          = 0x12;
	tse_conf_regs->mdio_addr1          = 0x0;
	mac_conf_regs->cdone               = 0xFFFFFFFF;


	if(ival)
		bus_w(addrr, bus_r(addrr) | (INT_RSTN_BIT|ENET_RESETN_BIT|WRITE_BACK_BIT|DIGITAL_TEST_BIT)); //0x2840,write shadow regs..
	else
		bus_w(addrr, bus_r(addrr) | (INT_RSTN_BIT|ENET_RESETN_BIT|WRITE_BACK_BIT)); //0x2840,write shadow regs..

#ifdef VERBOSE
	printf("Value read from Multi-purpose Reg:%x\n",bus_r(addrr));
#endif 
	//  if(val!=0x2840) return -1;

	usleep(100000);

	bus_w(addrr, bus_r(addrr) &(~WRITE_BACK_BIT));

	if(ival)
		bus_w(addrr, bus_r(addrr) | (INT_RSTN_BIT|ENET_RESETN_BIT|SW1_BIT|DIGITAL_TEST_BIT)); //0x2820,write shadow regs..
	else
		bus_w(addrr, bus_r(addrr) | (INT_RSTN_BIT|ENET_RESETN_BIT|SW1_BIT)); //0x2820,write shadow regs..

#ifdef VERBOSE
	printf("Value read from Multi-purpose Reg:%x\n",bus_r(addrr));
#endif 
	//  if(val!=0x2820) return -1;

	usleep(1000 * 1000);

	return adcConfigured;
}


int getAdcConfigured(){
	return adcConfigured;
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
	return v64;
}

int64_t setFrames(int64_t value){
	return set64BitReg(value,  SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG);
}

int64_t getFrames(){
	return get64BitReg(GET_FRAMES_LSB_REG, GET_FRAMES_MSB_REG);
}

int64_t setExposureTime(int64_t value){
	/* time is in ns */
	if (value!=-1) {
		value = (value * 1E-3 * CLK_FREQ ) + 0.5;
	}
	return (set64BitReg(value,SET_EXPTIME_LSB_REG, SET_EXPTIME_MSB_REG) /
			(1E-3 * CLK_FREQ)) + 0.5;
}

int64_t getExposureTime(){
	return (get64BitReg(GET_EXPTIME_LSB_REG, GET_EXPTIME_MSB_REG) /
			(1E-3 * CLK_FREQ)) + 0.5;
}

int64_t setGates(int64_t value){
	return set64BitReg(value, SET_GATES_LSB_REG, SET_GATES_MSB_REG);
}

int64_t getGates(){
	return get64BitReg(GET_GATES_LSB_REG, GET_GATES_MSB_REG);
}

int64_t setPeriod(int64_t value){
	/* time is in ns */
	if (value!=-1) {
		value = (value * 1E-3 * CLK_FREQ ) + 0.5;
	}
	return (set64BitReg(value,SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG) /
			(1E-3 * CLK_FREQ)) + 0.5;
}

int64_t getPeriod(){
	return (get64BitReg(GET_PERIOD_LSB_REG, GET_PERIOD_MSB_REG) /
			(1E-3 * CLK_FREQ)) + 0.5;
}

int64_t setDelay(int64_t value){
	/* time is in ns */
	if (value!=-1) {
		if (masterflags == IS_MASTER) {
			value += masterdefaultdelay;
			cprintf(BLUE,"Actual delay for master: %lld\n", (long long int) value);
		}
		value = (value * 1E-3 * CLK_FREQ ) + 0.5;
	}
	int64_t retval = (set64BitReg(value,SET_DELAY_LSB_REG, SET_DELAY_MSB_REG) /
			(1E-3 * CLK_FREQ)) + 0.5;
	if (masterflags == IS_MASTER) {
		cprintf(BLUE,"Actual delay read from master: %lld\n", (long long int) retval);
		retval -= masterdefaultdelay;
	}

	return retval;
}

int64_t getDelay(){
	return (get64BitReg(GET_DELAY_LSB_REG, GET_DELAY_MSB_REG) /
			(1E-3 * CLK_FREQ)) + 0.5;
}

int64_t setTrains(int64_t value){
	return set64BitReg(value,  SET_TRAINS_LSB_REG, SET_TRAINS_MSB_REG);
}

int64_t getTrains(){
	return get64BitReg(GET_TRAINS_LSB_REG, GET_TRAINS_MSB_REG);
}



int64_t getActualTime(){
	return (get64BitReg(GET_ACTUAL_TIME_LSB_REG, GET_ACTUAL_TIME_MSB_REG) /
			(1E-3 * CLK_FREQ)) + 0.5;
}

int64_t getMeasurementTime(){
	int64_t v=get64BitReg(GET_MEASUREMENT_TIME_LSB_REG, GET_MEASUREMENT_TIME_MSB_REG);
	return (v / (1E-3 * CLK_FREQ)) + 0.5;
}






u_int32_t  fifoReadStatus() {
	return bus_r(STATUS_REG)&(SOME_FIFO_FULL_BIT | ALL_FIFO_EMPTY_BIT);
}

u_int32_t  fifo_full(void) {
	return bus_r(STATUS_REG)&SOME_FIFO_FULL_BIT;
}


u_int32_t runBusy(void) {
	u_int32_t s = bus_r(STATUS_REG) & RUN_BUSY_BIT;
	return s;
}

u_int32_t runState(void) {
	int s=bus_r(STATUS_REG);
#ifdef VERBOSE
printf("status %04x\n",s);
#endif

return s;
}


// State Machine

int startStateMachine(){

	//#ifdef VERBOSE
	printf("*******Starting State Machine*******\n");
	//#endif
	cleanFifo();
	// fifoReset();

	bus_w16(CONTROL_REG, START_ACQ_BIT |  START_EXPOSURE_BIT);
	bus_w16(CONTROL_REG, 0x0);
	printf("statusreg=%08x\n",bus_r(STATUS_REG));
	return OK;
}




int stopStateMachine(){

	//#ifdef VERBOSE
	cprintf(BG_RED,"*******Stopping State Machine*******\n");
	//#endif
	bus_w16(CONTROL_REG, STOP_ACQ_BIT);
	bus_w16(CONTROL_REG, 0x0);
	usleep(500);
	// if (!runBusy())
	if(!(bus_r(STATUS_REG)&RUNMACHINE_BUSY_BIT))
		return OK;
	else
		return FAIL;
}


int startReadOut(){
	u_int32_t status;
#ifdef VERBOSE
	printf("Starting State Machine Readout\n");
#endif
	status=bus_r(STATUS_REG)&RUN_BUSY_BIT;

	bus_w16(CONTROL_REG,  START_ACQ_BIT |START_READOUT_BIT);   //  start readout
	bus_w16(CONTROL_REG,  0x0);
	return OK;
}




void waitForAcquisitionFinish(){
	volatile u_int32_t t = bus_r(LOOK_AT_ME_REG);
#ifdef VERBOSE
	printf("lookatmereg=x%x\n",t);
#endif
	while((t&0x1)==0) {
		if (runBusy()==0) {
			t = bus_r(LOOK_AT_ME_REG);
			if ((t&0x1)==0) {
#ifdef VERBOSE
				printf("no frame found - exiting ");
				printf("%08x %08x\n", runState(), bus_r(LOOK_AT_ME_REG));
#endif
				return;
			} else {
#ifdef VERBOSE
				printf("no frame found %x status %x\n", bus_r(LOOK_AT_ME_REG),runState());
#endif
				break;
			}
		}
		t = bus_r(LOOK_AT_ME_REG);
	}
}


int getStatus() {
	u_int32_t retval= runState();
	enum runStatus s = IDLE;
	int ret = OK;

	printf("\n\nSTATUS=%08x\n",retval);

	//stopped (external stop, also maybe fifo full)
	if (runState() & STOPPED_BIT){
		printf("-----------------------------------STOPPED--------------------------------------x%0x\n",retval);
		s=STOPPED;

		retval= runState();
		printf("reading again STATUS=%08x\n",retval);
		if (runState() & STOPPED_BIT){
			printf("-----------------------------------ERROR--------------------------------------x%0x\n",retval);
			s=ERROR;
		}
	}


	//error
	else if(retval&SOME_FIFO_FULL_BIT){
		printf("-----------------------------------ERROR--------------------------------------x%0x\n",retval);
		s=ERROR;
	}

	//runbusy=0
	// else if(!(retval&RUNMACHINE_BUSY_BIT)){ //commented by Anna 24.10.2012
	else if(!(retval&RUN_BUSY_BIT)){ // by Anna 24.10.2012

		//and readbusy=1, its last frame read
		if((retval&READMACHINE_BUSY_BIT)  ){ //


			printf("-----------------------------------READ MACHINE BUSY--------------------------\n");
			s=TRANSMITTING;
		} else if (retval&ALL_FIFO_EMPTY_BIT) {
			printf("-----------------------------------DATA IN FIFO--------------------------\n");
			s=TRANSMITTING;

		}
		//and readbusy=0,idle
		else if(!(retval&0xffff)){
			//if(!(retval&0x00000001)){
			printf("-----------------------------------IDLE--------------------------------------\n");
			s=IDLE;
		} else {
			printf("-----------------------------------Unknown status %08x--------------------------------------\n", retval);
			s=ERROR;
			ret=FAIL;
		}
	}
	//if runbusy=1
	else {
		if (retval&WAITING_FOR_TRIGGER_BIT){
			printf("-----------------------------------WAITING-----------------------------------\n");
			s=WAITING;
		}
		else{
			printf("-----------------------------------RUNNING-----------------------------------\n");
			s=RUNNING;
		}
	}

	return s;
}



int loadImage(int index, short int ImageVals[]){
	u_int32_t address;
	switch (index) {
	case DARK_IMAGE :
		address = DARK_IMAGE_REG;
		break;
	case GAIN_IMAGE :
		address = GAIN_IMAGE_REG;
		break;
	}
	volatile u_int16_t *ptr;
	ptr=(u_int16_t*)(CSP0BASE+address*2);
#ifdef VERBOSE
int i;
for(i=0;i<6;i++)
	printf("%d:%d\t",i,ImageVals[i]);
#endif
memcpy((char*)ptr,(char*)ImageVals ,DATA_BYTES);
#ifdef VERBOSE
printf("\nLoaded x%08x address with image of index %d\n",(unsigned int)(ptr),index);
#endif
return OK;
}



int readCounterBlock(int startACQ, short int CounterVals[]){


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

	memcpy((char*)CounterVals,(char*)ptr,DATA_BYTES);
#ifdef VERBOSE
	int i;
	printf("Copied counter memory block with size of %d bytes..\n",DATA_BYTES);
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


	return OK;
}




int resetCounterBlock(int startACQ){

	char *counterVals=NULL;
	counterVals=realloc(counterVals,DATA_BYTES);

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


	memcpy((char*)counterVals,(char*)ptr,DATA_BYTES);
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






int copyModule(sls_detector_module *destMod, sls_detector_module *srcMod) {

	int idac, iadc;

	int ret=OK;

#ifdef VERBOSE
	printf("Copying module %x to module %x\n",(unsigned int)(srcMod),(unsigned int)(destMod));
#endif

	if (srcMod->serialnumber>=0){
		destMod->serialnumber=srcMod->serialnumber;
	}
	if ((srcMod->nchip)>(destMod->nchip)) {
		printf("Number of chip of source is larger than number of chips of destination\n");
		return FAIL;
	}
	if ((srcMod->nchan)>(destMod->nchan)) {
		printf("Number of channels of source is larger than number of channels of destination\n");
		return FAIL;
	}
	if ((srcMod->ndac)>(destMod->ndac)) {
		printf("Number of dacs of source is larger than number of dacs of destination\n");
		return FAIL;
	}
	if ((srcMod->nadc)>(destMod->nadc)) {
		printf("Number of dacs of source is larger than number of dacs of destination\n");
		return FAIL;
	}

#ifdef VERBOSE
	printf("DACs: src %d, dest %d\n",srcMod->ndac,destMod->ndac);
	printf("ADCs: src %d, dest %d\n",srcMod->nadc,destMod->nadc);
	printf("Chips: src %d, dest %d\n",srcMod->nchip,destMod->nchip);
	printf("Chans: src %d, dest %d\n",srcMod->nchan,destMod->nchan);

#endif

	destMod->ndac=srcMod->ndac;
	destMod->nadc=srcMod->nadc;
	destMod->nchip=srcMod->nchip;
	destMod->nchan=srcMod->nchan;
	if (srcMod->reg>=0)
		destMod->reg=srcMod->reg;
#ifdef VERBOSE
	printf("Copying register %x (%x)\n",destMod->reg,srcMod->reg );
#endif
	if (srcMod->gain>=0)
		destMod->gain=srcMod->gain;
	if (srcMod->offset>=0)
		destMod->offset=srcMod->offset;

	for (idac=0; idac<(srcMod->ndac); idac++) {
		if (*((srcMod->dacs)+idac)>=0)
			*((destMod->dacs)+idac)=*((srcMod->dacs)+idac);
	}
	for (iadc=0; iadc<(srcMod->nadc); iadc++) {
		if (*((srcMod->adcs)+iadc)>=0)
			*((destMod->adcs)+iadc)=*((srcMod->adcs)+iadc);
	}
	return ret;
}







int setDAC(int ind, int val,int mV, int retval[]) {

	if(mV)
		val = (val*4096)/2500;

	if (val>=0)
		initDAC(ind,val);
	clearDACSregister();

	retval[0] = setDACRegister(ind, -1);
	retval[1] = (retval[0]*2500)/4096;
	return retval[0];

}



int getDAC(int ind) {
	return setDACRegister(ind, -1);
}







int setModule(sls_detector_module myMod) {

	printf("\ninside setmodule..\n");

	int v[NDAC];
	v[VREF_DS]=(myMod.dacs)[0];
	v[VCASCN_PB]=(myMod.dacs)[1];
	v[VCASCP_PB]=(myMod.dacs)[2];
	v[VOUT_CM]=(myMod.dacs)[3];
	v[VCASC_OUT]=(myMod.dacs)[4];
	v[VIN_CM]=(myMod.dacs)[5];
	v[VREF_COMP]=(myMod.dacs)[6];
	v[IB_TESTC]=(myMod.dacs)[7];

#ifdef VERBOSE
	printf("vrefds=%d\n",v[VREF_DS]);
	printf("vcascn=%d\n",v[VCASCN_PB]);
	printf("vcascp=%d\n",v[VCASCP_PB]);
	printf("vout=%d\n",v[VOUT_CM]);
	printf("vcasc=%d\n",v[VCASC_OUT]);
	printf("vin=%d\n",v[VIN_CM]);
	printf("vref_comp=%d\n",v[VREF_COMP]);
	printf("ib_testc=%d\n",v[IB_TESTC]);
#endif

	initDACs(v);
	clearDACSregister();

	if (detectorModules) {
		copyModule(detectorModules,&myMod);
	}

	//setting the conf gain and the settings register
	setSettings(myMod.reg);

	return myMod.reg;
}



void getModule(sls_detector_module* myMod) {
	if (detectorModules) {
		copyModule(myMod,detectorModules);
	}
}



void initDACs(int* v) {
#ifdef VERBOSE
	printf("\n..inside initdacs\n");
#endif
	int iaddr;
	for (iaddr=0; iaddr<8; iaddr++) {
		initDAC(iaddr, v[iaddr]);
	}
}



void initDAC(int dac_addr, int value) {
#ifdef VERBOSE
	printf("Programming dac %d with value %d\n", dac_addr, value);
#endif
	clearDACSregister();
	if (value >= 0) {
		program_one_dac(dac_addr,value);
	}
	nextDAC();
}





void clearDACSregister() {
	putout("1111111111111111");//reset
	putout("1111111111111110");//cs down
}


void nextDAC() {
	putout("1111111111111011");//cs up
	putout("1111111111111001");//clk down
	putout("1111111111111111");//reset
}



void program_one_dac(int addr, int value) {

#ifdef VERBOSE
	printf("programming dac %d value %d module %d\n",addr, value);
#endif


	int i,idac,bit, control;
	int v=value;

	control=32+addr;
	value=(value<<4) |  (control<< 16);


	for (i=0;i<24;i++) {
		bit=value & (1<<(23-i));
		if (bit) {
			putout("1111111111111100");//clk down
			putout("1111111111111100");//write data
			putout("1111111111111110");//clk up
		}
		else
		{
			putout("1111111111111000");//clk down
			putout("1111111111111000");//write data
			putout("1111111111111010");//clk up
		}
	}

	idac=addr;


	if (detectorDacs) {
		detectorDacs[idac]=v;
#ifdef VERBOSE
		printf("index=%d, val=%d addr=%x\n", idac, v, (unsigned int)(detectorDacs+idac));
#endif
		setDACRegister(idac,v);
	}
}










