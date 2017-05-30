//#define TESTADC


#include "server_defs.h"
#include "firmware_funcs.h"
#include "mcb_funcs.h"
#include "registers_m.h"
#include "gitInfoJungfrau.h"


#include <stdio.h>
#include <stdlib.h>  /* exit() */
#include <stdarg.h>
#include <string.h>  /* memset(), memcpy() */
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/utsname.h>   /* uname() */
#include <sys/types.h>
#include <sys/socket.h>   /* socket(), bind(), listen(), accept() */
#include <unistd.h>  /* fork(), write(), close() */
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h> 
#include <errno.h>
#include <fcntl.h>




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


u_int32_t CSP0BASE = 0;
int highvoltage = 0;
int dacValues[NDAC];
int32_t clkPhase[2] = {0, 0};
char mtdvalue[10];
int masterMode=NO_MASTER, syncMode=NO_SYNCHRONIZATION, timingMode=AUTO_TIMING;
enum externalSignalFlag signals[4]={EXT_SIG_OFF, EXT_SIG_OFF, EXT_SIG_OFF, EXT_SIG_OFF};






int mapCSP0(void) {
	printf("Mapping memory\n");
#ifndef VIRTUAL
	int fd;
	fd = open("/dev/mem", O_RDWR | O_SYNC, 0);
	if (fd == -1) {
		printf("\nCan't find /dev/mem!\n");
		return FAIL;
	}
#ifdef VERBOSE
	printf("/dev/mem opened\n");
#endif
	CSP0BASE = (u_int32_t)mmap(0, MEM_SIZE, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, CSP0);
	if (CSP0BASE == (u_int32_t)MAP_FAILED) {
		printf("\nCan't map memmory area!!\n");
		return FAIL;
	}
	printf("CSPOBASE mapped from %08x to %08x\n",CSP0BASE,CSP0BASE+MEM_SIZE);
#else
	CSP0BASE = malloc(MEM_SIZE);
	printf("memory allocated\n");
#endif
	printf("Status Register: %08x\n",bus_r(STATUS_REG));
	return OK;
}


u_int16_t bus_w16(u_int32_t offset, u_int16_t data) {
	volatile u_int16_t  *ptr1;
	ptr1=(u_int16_t*)(CSP0BASE+offset*2);
	*ptr1=data;
	return OK;
}

u_int16_t bus_r16(u_int32_t offset){
	volatile u_int16_t *ptr1;
	ptr1=(u_int16_t*)(CSP0BASE+offset*2);
	return *ptr1;
}

// ramType is DARK_IMAGE_REG or GAIN_IMAGE_REG
u_int16_t ram_w16(u_int32_t ramType, int adc, int adcCh, int Ch, u_int16_t data) {
	unsigned int adr = (ramType | adc << 8 | adcCh << 5 | Ch );
	return bus_w16(adr,data);
}

// ramType is DARK_IMAGE_REG or GAIN_IMAGE_REG
u_int16_t ram_r16(u_int32_t ramType, int adc, int adcCh, int Ch){
	unsigned int adr = (ramType | adc << 8 | adcCh << 5 | Ch );
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




void initializeDetector(){
	printf("Initializing Detector\n");

	//initial test
	if ( (checkType() == FAIL) || (testFpga() == FAIL) || (testBus() == FAIL) ) {
		cprintf(BG_RED, "Dangerous to continue. Goodbye!\n");
		exit(-1);
	}

	printf("Resetting PLL\n");
	resetPLL();
	resetCore();
	resetPeripheral();
	cleanFifos();

	//allocating module structure for the detector in the server
#ifdef MCB_FUNCS
	initDetector();
#endif

	prepareADC();

	// initialize dac series
	initDac(0);		/* todo might work without */
	initDac(8); 	//only for old board compatibility

	//set dacs
	printf("Setting Default Dac values\n")
	{
		int i = 0;
		int retval = -1;
		const int defaultvals[NDAC] = DEFAULT_DAC_VALS;
		for(i = 0; i < NDAC; ++i) {
			retval = setDac(i, defaultvals[i]);
			if (retval != defaultvals[i])
				cprintf(RED, "Error: Setting dac %d failed, wrote %d, read %d\n",i ,defaultvals[i], retval);
		}
	}

	bus_w(DAQ_REG, 0x0); 		/* Only once at server startup */

	setClockDivider(HALF_SPEED);
	cleanFifos();	/* todo might work without */
	resetCore();	/* todo might work without */


	//Initialization of acquistion parameters
	setSettings(DEFAULT_SETTINGS,-1);
	setFrames(DEFAULT_NUM_FRAMES);
	setTrains(DEFAULT_NUM_CYCLES);
	setExposureTime(DEFAULT_EXPTIME);
	setPeriod(DEFAULT_PERIOD);
	setDelay(DEFAULT_DELAY);
	setHighVoltage(DEFAULT_HIGH_VOLTAGE);

}

int checkType() {
	volatile u_int32_t type = ((bus_r(FPGA_VERSION_REG) & DETECTOR_TYPE_MSK) >> DETECTOR_TYPE_OFST);
	if (type != JUNGFRAU){
			cprintf(BG_RED,"This is not a Jungfrau Server (read %d, expected %d)\n",type, JUNGFRAU);
			return FAIL;
		}


	printVersions();

	return OK;
}

void printVersions() {
	cprintf(BLUE,"\n\n"
			"********************************************************\n"
			"*********************Jungfrau Server********************\n"
			"********************************************************\n\n"
			"Firmware Version:\t 0x%llx\n"
			"Software Version:\t 0x%llx\n"
			//"F/w-S/w API Version:\t\t %lld\n"
			//"Required Firmware Version:\t %d\n"
			"********************************************************\n",
			(long long unsigned int)getId(DETECTOR_FIRMWARE_VERSION),
			(long long unsigned int)getId(DETECTOR_SOFTWARE_VERSION)
			//,(long long unsigned int)getId(SOFTWARE_FIRMWARE_API_VERSION)
			//REQUIRED_FIRMWARE_VERSION);
	);
}






int testFifos(void) {
	printf("Fifo test not implemented!\n");
	return OK;
}

u_int32_t testFpga(void) {
	printf("\nTesting FPGA...\n");

	//fixed pattern
	int ret = OK;
	volatile u_int32_t val = bus_r(FIX_PATT_REG);
	if (val == FIX_PATT_VAL) {
		printf("Fixed pattern: successful match 0x%08x\n",val);
	} else {
		cprintf(RED,"Fixed pattern does not match! Read 0x%08x, expected 0x%08x\n", val, FIX_PATT_VAL);
		ret = FAIL;
	}
	printf("\n");
	return ret;
}

u_int32_t testRAM(void) {
	int result=OK;
	printf("RAM Test not implemented!\n");
	return result;
}

int testBus() {
	printf("\nTesting Bus...\n");

	int ret = OK;
	u_int32_t addr = SET_DELAY_LSB_REG;
	int times = 1000 * 1000;
	int i = 0;

	for (i = 0; i < times; ++i) {
		bus_w(addr, i * 100);
		if (i * 100 != bus_r(SET_DELAY_LSB_REG)) {
			cprintf(RED,"ERROR: Mismatch! Wrote 0x%x, read 0x%x\n", i * 100, bus_r(SET_DELAY_LSB_REG));
			ret = FAIL;
		}
	}

	if (ret == OK)
		printf("Successfully tested bus %d times\n", times);

	printf("\n");
	return ret;
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

u_int64_t  getFirmwareVersion() {
	return ((bus_r(FPGA_VERSION_REG) & BOARD_REVISION_MSK) >> BOARD_REVISION_OFST);
}

int64_t getId(enum idMode arg) {
	int64_t retval = -1;

	switch(arg){
	case DETECTOR_SERIAL_NUMBER:
		retval =  getDetectorNumber();
		break;
	case DETECTOR_FIRMWARE_VERSION:
		retval=getFirmwareSVNVersion();
		retval=(retval <<32) | getFirmwareVersion();
		break;
	case DETECTOR_SOFTWARE_VERSION:
		retval= SVNREV;
		retval= (retval <<32) | SVNDATE;
		break;
	default:
		printf("Required unknown id %d \n", arg);
		break;
	}
	return retval;
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








long int calcChecksum(int sourceip, int destip) {

	ip_header ip;
	int count;
	unsigned short *addr;
	long int sum = 0;
	long int checksum;
	ip.ip_ver            = 0x4;
	ip.ip_ihl            = 0x5;
	ip.ip_tos            = 0x0;
	ip.ip_len            = IP_PACKETSIZE;
	ip.ip_ident          = 0x0000;
	ip.ip_flag           = 0x2; 	//not nibble aligned (flag& offset
	ip.ip_offset         = 0x000;
	ip.ip_ttl            = 0x40;
	ip.ip_protocol       = 0x11;
	ip.ip_chksum         = 0x0000 ; // pseudo
	ip.ip_sourceip       = sourceip;
	ip.ip_destip         = destip;

	count = sizeof(ip);
	addr =& (ip); /* warning: assignment from incompatible pointer type */
	while( count > 1 )  {
		sum += *addr++;
		count -= 2;
	}
	if (count > 0)  sum += *addr;                     // Add left-over byte, if any
	while (sum>>16) sum = (sum & 0xffff) + (sum >> 16);// Fold 32-bit sum to 16 bits
	checksum = (~sum) & 0xffff;

	printf("IP checksum is 0x%lx\n",checksum);

	return checksum;
}



void configureMAC(uint32_t destip, uint64_t destmac, uint64_t sourcemac, int sourceip, uint32_t destport) {
	uint32_t sourceport  =  DEFAULT_TX_UDP_PORT;
	long int checksum=calcChecksum(sourceip, destip);

	bus_w(TX_IP_REG, sourceip);
	bus_w(RX_IP_REG, destip);
	bus_w(RX_MAC_LSB_REG, (destmac << RX_MAC_LSB_OFST) & RX_MAC_LSB_MSK);
	bus_w(RX_MAC_MSB_REG, (destmac << RX_MAC_MSB_OFST) & RX_MAC_MSB_MSK);
	bus_w(TX_MAC_LSB_REG, (sourcemac << TX_MAC_LSB_OFST) & TX_MAC_LSB_MSK);
	bus_w(TX_MAC_MSB_REG, (sourcemac << TX_MAC_MSB_OFST) & TX_MAC_MSB_MSK);
	bus_w(UDP_PORT_REG,
			((sourceport << UDP_PORT_TX_OFST) & UDP_PORT_TX_MSK) |
			((destport << UDP_PORT_RX_OFST) & UDP_PORT_RX_MSK));
	bus_w(TX_IP_CHECKSUM_REG,(checksum << TX_IP_CHECKSUM_OFST) & TX_IP_CHECKSUM_MSK);

	cleanFifos();
	resetCore();

	usleep(500 * 1000); /* todo maybe without */
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
		value*=(1E-3*CLK_RUN);
	}
	int64_t retval = set64BitReg(value,SET_EXPTIME_LSB_REG, SET_EXPTIME_MSB_REG)/(1E-3*CLK_RUN);
	printf("Getting exptime: %lldns\n",(long long int)retval);
	return retval;
}

int64_t getExposureTime(){
	return 0;
}

int64_t setGates(int64_t value){
	return 0;
}

int64_t getGates(){
	return 0;
}

int64_t setDelay(int64_t value){
	if (value!=-1){
		printf("\nSetting delay to %lldns\n",(long long int)value);
		value*=(1E-3*CLK_SYNC);
	}

	int64_t retval = set64BitReg(value,SET_DELAY_LSB_REG, SET_DELAY_MSB_REG)/(1E-3*CLK_SYNC);
	printf("Getting delay: %lldns\n",(long long int)retval);
	return retval;
}

int64_t getDelay(){
	return get64BitReg(GET_DELAY_LSB_REG, GET_DELAY_MSB_REG)/(1E-3*CLK_SYNC);
}

int64_t setPeriod(int64_t value){
	if (value!=-1){
		printf("\nSetting period to %lldns\n",(long long int)value);
		value*=(1E-3*CLK_SYNC);
	}

	int64_t retval = set64BitReg(value,SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG)/(1E-3*CLK_SYNC);
	printf("Getting period: %lldns\n",(long long int)retval);
	return retval;
}

int64_t getPeriod(){
	return get64BitReg(GET_PERIOD_LSB_REG, GET_PERIOD_MSB_REG)/(1E-3*CLK_SYNC);
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

int64_t setProbes(int64_t value){
	return 0;
}

int64_t getProbes(){
	return 0;
}



int64_t getActualTime(){
	return get64BitReg(TIME_FROM_START_LSB_REG, TIME_FROM_START_MSB_REG)/(1E-9*CLK_SYNC);
}

int64_t getMeasurementTime(){
	return get64BitReg(START_FRAME_TIME_LSB_REG, START_FRAME_TIME_MSB_REG)/(1E-9*CLK_SYNC);

}

int64_t getFramesFromStart(){
	int64_t v1=get64BitReg(FRAMES_FROM_START_PG_LSB_REG, FRAMES_FROM_START_PG_MSB_REG);
	printf("Frames from start run control %lld\n",v1);
	return v1;
}






u_int32_t runBusy(void) {
	u_int32_t s = ((bus_r(STATUS_REG) & RUN_BUSY_MSK) >> RUN_BUSY_OFST);
#ifdef VERBOSE
	printf("Status Register: %08x\n", s);
#endif
	return s;
}




// State Machine

int startStateMachine(){
	printf("*******Starting State Machine*******\n");

	cleanFifos();

	//start state machine
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_START_ACQ_MSK);
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_START_ACQ_MSK);

	printf("Status Register: %08x\n",bus_r(STATUS_REG));
	return OK;
}




int stopStateMachine(){
	cprintf(BG_RED,"*******Stopping State Machine*******\n");

	//stop state machine
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_STOP_ACQ_MSK);
	usleep(100);
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_STOP_ACQ_MSK);

	printf("Status Register: %08x\n",bus_r(STATUS_REG));
	return OK;
}



enum runStatus getStatus() {
#ifdef VERBOSE
	printf("Getting status\n");
#endif

	enum runStatus s;
	u_int32_t retval = bus_r(STATUS_REG);
	printf("Status Register: %08x\n",retval);

	//running
	if(((retval & RUN_BUSY_MSK) >> RUN_BUSY_OFST)) {
		if ((retval & WAITING_FOR_TRIGGER_MSK) >> WAITING_FOR_TRIGGER_OFST) {
			printf("-----------------------------------WAITING-----------------------------------\n");
			s=WAITING;
		}
		else{
			printf("-----------------------------------RUNNING-----------------------------------\n");
			s=RUNNING;
		}
	}

	//not running
	else {
		if ((retval & STOPPED_MSK) >> STOPPED_OFST) {
			printf("-----------------------------------STOPPED--------------------------\n");
			s=STOPPED;
		} else if ((retval & RUNMACHINE_BUSY_MSK) >> RUNMACHINE_BUSY_OFST) {
			printf("-----------------------------------READ MACHINE BUSY--------------------------\n");
			s=TRANSMITTING;
		} else if (!retval) {
			printf("-----------------------------------IDLE--------------------------------------\n");
			s=IDLE;
		} else {
			printf("-----------------------------------Unknown status %08x--------------------------------------\n", retval);
			s=ERROR;
		}
	}

	return s;
}


void waitForAcquisitionEnd(){
	while(runBusy()){
		usleep(500);
	}
}










void serializeToSPI(u_int32_t addr, u_int32_t val, u_int16_t csmask, int numbitstosend, u_int16_t clkmask, u_int16_t digoutmask, int digofset) {
#ifdef VERBOSE
	if (numbitstosend == 16)
		printf("Writing to ADC SPI Register: 0x%04x\n",val);
	else
		printf("Writing to SPI Register: 0x%08x\n", val);
#endif

	u_int16_t valw;

	// start point
	valw = 0xffff; 		/**todo testwith old board 0xff for adc_spi */			// old board compatibility (not using specific bits)
	bus_w16 (addr, valw);

	// chip sel bar down
	valw &= ~csmask; /* todo with test: done a bit different, not with previous value */
	bus_w16 (addr, valw);

	{
		int i = 0;
		for (i = 0; i < numbitstosend; ++i) {

			// clk down
			valw &= ~clkmask;
			bus_w16 (addr, valw);

			// write data (i)
			valw = ((valw & ~digoutmask) + 										// unset bit
					(((val >> (numbitstosend - 1 - i)) & 0x1) << digofset)); 	// each bit from val starting from msb
			bus_w16 (addr, valw);

			// clk up
			valw |= clkmask ;
			bus_w16 (addr, valw);
		}
	}

	// chip sel bar up
	valw |= csmask; /* todo with test: not done for spi */
	bus_w16 (addr, valw);

	//clk down
	valw &= ~clkmask;
	bus_w16 (addr, valw);

	// stop point = start point of course
	valw = 0xffff; 		/**todo testwith old board 0xff for adc_spi */			// old board compatibility (not using specific bits)
	bus_w16 (addr, valw);

	printf("\n");
}



void initDac(int dacnum) {
	printf("\n Initializing dac for %d to \n",dacnum);

	u_int32_t codata;
	int csdx 		= dacnum / NDAC + DAC_SERIAL_CS_OUT_OFST; 	// old board (16 dacs),so can be DAC_SERIAL_CS_OUT_OFST or +1
	int dacchannel 	= 0xf;										// all channels
	int dacvalue	= 0x6; 										// can be any random value (just writing to power up)
	printf("Write to Input Register\n"
			"Chip select bit:%d\n"
			"Dac Channel:0x%x\n3"
			"Dac Value:0x%x",
			csdx, dacchannel, dacvalue);

	codata = LTC2620_DAC_CMD_WRITE +											// command to write to input register
			((dacchannel << LTC2620_DAC_ADDR_OFST) & LTC2620_DAC_ADDR_MSK) +	// all channels
			((dacvalue << LTC2620_DAC_DATA_OFST) & LTC2620_DAC_DATA_MSK);		// any random value
	serializeToSPI(SPI_REG, codata, (0x1 << csdx), LTC2620_DAC_NUMBITS,
			DAC_SERIAL_CLK_OUT_MSK, DAC_SERIAL_DIGITAL_OUT_MSK, DAC_SERIAL_DIGITAL_OUT_OFST);
}


int setDac(int dacnum, int dacvalue){

	u_int32_t codata;
	int csdx 		= dacnum / NDAC + DAC_SERIAL_CS_OUT_OFST; 	// old board (16 dacs),so can be DAC_SERIAL_CS_OUT_OFST or +1
	int dacchannel 	= dacnum % NDAC;							// 0-8, dac channel number (also for dacnum 9-15 in old board)

	if ( (dacvalue >= 0) || (dacvalue == -100)) {
		printf("\n Setting of DAC %d with value %d\n",dacnum, dacvalue);
		// command
		if (dacvalue >= 0) {
			printf("Write to Input Register and Update\n");
			codata = LTC2620_DAC_CMD_SET;

		} else if (dacvalue == -100) {
			printf("POWER DOWN\n");
			codata = LTC2620_DAC_CMD_POWER_DOWN;
		}
		// address
		printf("Chip select bit:%d\n"
				"Dac Channel:0x%x\n3"
				"Dac Value:0x%x",
				csdx, dacchannel, dacvalue);
		codata += ((dacchannel << LTC2620_DAC_ADDR_OFST) & LTC2620_DAC_ADDR_MSK) +
				  ((dacvalue << LTC2620_DAC_DATA_OFST) & LTC2620_DAC_DATA_MSK);
		// to spi
		serializeToSPI(SPI_REG, codata, (0x1 << csdx), LTC2620_DAC_NUMBITS,
				DAC_SERIAL_CLK_OUT_MSK, DAC_SERIAL_DIGITAL_OUT_MSK, DAC_SERIAL_DIGITAL_OUT_OFST);

		dacValues[dacnum] = dacvalue;
	} else
		printf("\nGetting DAC %d\n",dacnum);
	return dacValues[dacnum];
}



int setHighVoltage(int val, int imod){

	u_int32_t dacvalue;
	float alpha		= 0.55;
	// setting hv
	if (val >= 0) {
		// limit values
		if (val < 60) {
			dacvalue = 0;
			val = 60;
		} else if (val >= 200) {
			dacvalue = 0x1;
			val = 200;
		} else {
			dacvalue = 1. + (200.-val) / alpha;
			val=200.-(dacvalue-1)*alpha;
		}
		printf ("\n Setting High voltage to %d (dacval %d)\n",val, dacvalue);
		dacvalue &= MAX1932_HV_DATA_MSK;
		serializeToSPI(SPI_REG, dacvalue, HV_SERIAL_CS_OUT_MSK, MAX1932_HV_NUMBITS,
				HV_SERIAL_CLK_OUT_MSK, HV_SERIAL_DIGITAL_OUT_MSK, HV_SERIAL_DIGITAL_OUT_OFST);
		highvoltage = val;
	}
	return highvoltage;
}



void setAdc(int addr, int val) {

	u_int32_t codata;
	codata = val + (addr << 8);
	printf("\n Setting Adc spi register. Addr: 0x%04x Value: 0x%04x\n", addr, val);
	serializeToSPI(ADC_SPI_REG, codata, ADC_SERIAL_CS_OUT_MSK, AD9257_ADC_NUMBITS,
			ADC_SERIAL_CLK_OUT_MSK, ADC_SERIAL_DATA_OUT_MSK, ADC_SERIAL_DATA_OUT_OFST);
}




void prepareADC(){
	printf("Preparing ADC\n");

	//power mode reset
	setAdc(AD9257_POWER_MODE_REG,
			(AD9257_INT_RESET_VAL << AD9257_POWER_INTERNAL_OFST) & AD9257_POWER_INTERNAL_MSK);
	//power mode chip run
	setAdc(AD9257_POWER_MODE_REG,
			(AD9257_INT_CHIP_RUN_VAL << AD9257_POWER_INTERNAL_OFST) & AD9257_POWER_INTERNAL_MSK);

	//output clock phase
	setAdc(AD9257_OUT_PHASE_REG,
			(AD9257_OUT_CLK_60_VAL << AD9257_OUT_CLK_OFST) & AD9257_OUT_CLK_MSK);

	// lvds-iee reduced , binary offset
	setAdc(AD9257_OUT_MODE_REG,
			(AD9257_OUT_LVDS_IEEE_VAL << AD9257_OUT_LVDS_OPT_OFST) & AD9257_OUT_LVDS_OPT_MSK);

	// all devices on chip to receive next command
	setAdc(AD9257_DEV_IND_2_REG,
			AD9257_CHAN_H_MSK | AD9257_CHAN_G_MSK | AD9257_CHAN_F_MSK | AD9257_CHAN_E_MSK);
	setAdc(AD9257_DEV_IND_1_REG,
			AD9257_CHAN_D_MSK | AD9257_CHAN_C_MSK | AD9257_CHAN_B_MSK | AD9257_CHAN_A_MSK |
			AD9257_CLK_CH_DCO_MSK | AD9257_CLK_CH_IFCO_MSK);

	// vref 1.33
	setAdc(AD9257_VREF_REG,
			(AD9257_VREF_1_33_VAL << AD9257_VREF_OFST) & AD9257_VREF_MSK);

	// no test mode
	setAdc(AD9257_TEST_MODE_REG,
			(AD9257_NONE_VAL << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK);

#ifdef TESTADC
	printf("***************************************** *******\n");
	printf("******* PUTTING ADC IN TEST MODE!!!!!!!!! *******\n");
	printf("***************************************** *******\n");
	// mixed bit frequency test mode
	setAdc(AD9257_TEST_MODE_REG,
			(AD9257_MIXED_BIT_FREQ_VAL << AD9257_OUT_TEST_OFST) & AD9257_OUT_TEST_MSK);
#endif
}









int setDynamicRange(int dr) {
	return DYNAMIC_RANGE;
}

int getDynamicRange() {
	return DYNAMIC_RANGE;
}

int getNModBoard() {
	return NMOD;
}

int setNMod(int n) {
	return NMOD;
}

int getNMod() {
	return NMOD;
}





int powerChip (int on){
	if(on != -1){
		if(on){
			printf("\nPowering on the chip\n");
			bus_w(CHIP_POWER_REG, bus_r(CHIP_POWER_REG) | CHIP_POWER_ENABLE_MSK);
		}
		else{
			printf("\nPowering off the chip\n");
			bus_w(CHIP_POWER_REG, bus_r(CHIP_POWER_REG) & ~CHIP_POWER_ENABLE_MSK);
		}
	}
	return bus_r(CHIP_POWER_REG);
}



void cleanFifos() {
	printf("Clearing Acquisition Fifos\n");
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_ACQ_FIFO_CLR_MSK);
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_ACQ_FIFO_CLR_MSK);
}

void resetCore() {
	printf("Resetting Core\n");
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_CORE_RST_MSK);
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_CORE_RST_MSK);
}

void resetPeripheral() {
	printf("Resetting Peripheral\n");
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_PERIPHERAL_RST_MSK);
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_PERIPHERAL_RST_MSK);
}




int adcPhase(int st){ /**carlos needed clkphase 1 and 2?  cehck with Aldo */
	printf("\nSetting ADC Phase to %d\n",st);
	if (st > 65535 || st < -65535)
		return clkPhase[0];
	clkPhase[1] = st - clkPhase[0];

	printf("phase %d\n", clkPhase[1] );
	configurePll();
	clkPhase[0] = st;
	return clkPhase[0];
}


int getPhase() {
	return clkPhase[0];
}






u_int32_t setClockDivider(int d) {
	if(d!=-1){
		switch(d){
		//stop state machine if running
		if(runBusy())
			stopStateMachine();

		case FULL_SPEED://40
			printf("Setting Half Speed (20 MHz):\n");
			/**to be done*/

			printf("Setting Sample Reg to 0x%x\n", SAMPLE_ADC_HALF_SPEED);		bus_w(SAMPLE_REG, SAMPLE_ADC_HALF_SPEED);
			printf("Setting Config Reg to 0x%x\n", CONFIG_HALF_SPEED);			bus_w(CONFIG_REG, CONFIG_HALF_SPEED);
			printf("Setting ADC Ofst Reg to 0x%x\n", ADC_OFST_HALF_SPEED_VAL);	bus_w(ADC_OFST_REG, ADC_OFST_HALF_SPEED_VAL);
			printf("Setting ADC Phase Reg to 0x%x\n", ADC_PHASE_HALF_SPEED);	adcPhase(ADC_PHASE_HALF_SPEED);
			break;
		case HALF_SPEED:
			printf("Setting Half Speed (20 MHz):\n");
			printf("Setting Sample Reg to 0x%x\n", SAMPLE_ADC_HALF_SPEED);		bus_w(SAMPLE_REG, SAMPLE_ADC_HALF_SPEED);
			printf("Setting Config Reg to 0x%x\n", CONFIG_HALF_SPEED);			bus_w(CONFIG_REG, CONFIG_HALF_SPEED);
			printf("Setting ADC Ofst Reg to 0x%x\n", ADC_OFST_HALF_SPEED_VAL);	bus_w(ADC_OFST_REG, ADC_OFST_HALF_SPEED_VAL);
			printf("Setting ADC Phase Reg to 0x%x\n", ADC_PHASE_HALF_SPEED);	adcPhase(ADC_PHASE_HALF_SPEED);
			break;
		case QUARTER_SPEED:
			printf("Setting Half Speed (10 MHz):\n");
			printf("Setting Sample Reg to 0x%x\n", SAMPLE_ADC_QUARTER_SPEED);		bus_w(SAMPLE_REG, SAMPLE_ADC_QUARTER_SPEED);
			printf("Setting Config Reg to 0x%x\n", CONFIG_QUARTER_SPEED);			bus_w(CONFIG_REG, CONFIG_QUARTER_SPEED);
			printf("Setting ADC Ofst Reg to 0x%x\n", ADC_OFST_QUARTER_SPEED_VAL);	bus_w(ADC_OFST_REG, ADC_OFST_QUARTER_SPEED_VAL);
			printf("Setting ADC Phase Reg to 0x%x\n", ADC_PHASE_QUARTER_SPEED);		adcPhase(ADC_PHASE_QUARTER_SPEED);
			break;
		}
		printf("\n");
	}
	return getClockDivider();
}






u_int32_t getClockDivider(int ic) {

	u_int32_t val = bus_r(CONFIG_REG);
	int speed = val & CONFIG_READOUT_SPEED_MSK;
	switch(speed){
	case CONFIG_FULL_SPEED_40MHZ_VAL:
		return FULL_SPEED;
	case CONFIG_HALF_SPEED_20MHZ_VAL:
		return HALF_SPEED;
	case CONFIG_QUARTER_SPEED_10MHZ_VAL:
		return QUARTER_SPEED;
	default:
		return -1;
	}
}












/**carlos  shouldnt exist what sort of temperatre?s tempr in reg  is 1b<11 and temp pit is 1c<11 ..firmware (only 0x1c 32 bit ) Aldo.. is it connected??*/
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











void resetPLL() {
	// reset PLL Reconfiguration and PLL
	bus_w(PLL_CONTROL_REG, bus_r(PLL_CONTROL_REG) | PLL_CTRL_RECONFIG_RST_MSK | PLL_CTRL_RST_MSK);
	usleep(100);
	bus_w(PLL_CONTROL_REG, bus_r(PLL_CONTROL_REG) & ~PLL_CTRL_RECONFIG_RST_MSK & ~PLL_CTRL_RST_MSK);
}


u_int32_t setPllReconfigReg(u_int32_t reg, u_int32_t val) {

	// set parameter
	bus_w(PLL_PARAM_REG, val);

	// set address
	bus_w(PLL_CONTROL_REG, (reg << PLL_CTRL_ADDR_OFST) & PLL_CTRL_ADDR_MSK);
	usleep(10*1000);

	//write parameter
	bus_w(PLL_CONTROL_REG, bus_r(PLL_CONTROL_REG) | PLL_CTRL_WR_PARAMETER_MSK);
	bus_w(PLL_CONTROL_REG, bus_r(PLL_CONTROL_REG) & ~PLL_CTRL_WR_PARAMETER_MSK);
	usleep(10*1000);

	return val;
}






void configurePll() {
	u_int32_t val;
	int32_t phase=0, inv=0;

	printf("phase in %d\n", clkPhase[1]);
	if (clkPhase[1]>0) {
		inv=0;
		phase=clkPhase[1];
	}  else {
		inv=1;
		phase=-1*clkPhase[1];
	}
	printf("phase out %d %08x\n", phase, phase);

	if (inv) {
		val = ((phase << PLL_SHIFT_NUM_SHIFTS_OFST) & PLL_SHIFT_NUM_SHIFTS_MSK) | PLL_SHIFT_CNT_SLCT_C1_VAL | PLL_SHIFT_UP_DOWN_NEG_VAL;
		printf("**************** phase word %08x\n", val);
		setPllReconfigReg(PLL_PHASE_SHIFT_REG, val);
	} else {
		val = ((phase << PLL_SHIFT_NUM_SHIFTS_OFST) & PLL_SHIFT_NUM_SHIFTS_MSK) | PLL_SHIFT_CNT_SLCT_C0_VAL | PLL_SHIFT_UP_DOWN_NEG_VAL;
		printf("**************** phase word %08x\n", val);
		setPllReconfigReg(PLL_PHASE_SHIFT_REG, val);

		printf("**************** phase word %08x\n", val);
		val = ((phase << PLL_SHIFT_NUM_SHIFTS_OFST) & PLL_SHIFT_NUM_SHIFTS_MSK) | PLL_SHIFT_CNT_SLCT_C2_VAL;
		setPllReconfigReg(PLL_PHASE_SHIFT_REG, val);
	}
	usleep(10000);
}









int loadImage(int index, short int ImageVals[]){
	printf("loadImage Not implemented yet\n");
	return OK;
}


int readCounterBlock(int startACQ, short int CounterVals[]){

//not implemented
	return FAIL;
}




int resetCounterBlock(int startACQ){

	//not implemented
		return FAIL;

}



int calibratePedestal(int frames){
	//not implemented
	return FAIL;
}







int setTiming(int ti) {

	if(ti != GET_EXTERNAL_COMMUNICATION_MODE){
		switch((int)ti){
		case AUTO_TIMING:			bus_w(EXT_SIGNAL_REG, bus_r(EXT_SIGNAL_REG) & ~EXT_SIGNAL_MSK);	break;
		case TRIGGER_EXPOSURE:		bus_w(EXT_SIGNAL_REG, bus_r(EXT_SIGNAL_REG) | EXT_SIGNAL_MSK);	break;
		default:
			cprintf(RED,"Unknown timing mode %d\n", ti);
			return GET_EXTERNAL_COMMUNICATION_MODE;
		}
	}
	if (bus_r(EXT_SIGNAL_REG) == EXT_SIGNAL_MSK)
		return TRIGGER_EXPOSURE;
	return AUTO_TIMING;
}



int setMaster(int f) {
	return NO_MASTER;
}



int setSynchronization(int s) {
	return NO_SYNCHRONIZATION;

}


