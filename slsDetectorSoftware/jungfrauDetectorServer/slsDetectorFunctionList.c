//#ifdef SLS_DETECTOR_FUNCTION_LIST


#include "slsDetectorFunctionList.h"
#include "gitInfoJungfrau.h"
#include "AD9257.h"		// include "commonServerFunctions.h", which in turn includes "blackfin.h"
#include "programfpga.h"
#include "versionAPI.h"

#ifdef VIRTUAL
#include <pthread.h>
#include <time.h>
#endif
/* global variables */
//jungfrau doesnt require chips and chans (save memory)
sls_detector_module *detectorModules=NULL;
dacs_t *detectorDacs=NULL;
dacs_t *detectorAdcs=NULL;

enum detectorSettings thisSettings;
enum masterFlags masterMode = NO_MASTER;

int highvoltage = 0;
int dacValues[NDAC];
int32_t clkPhase[2] = {0, 0};

#ifdef VIRTUAL
pthread_t pthread_virtual_tid;
int virtual_status = 0;
int virtual_stop = 0;
#endif


/* basic tests */

int firmware_compatibility = OK;
int firmware_check_done = 0;
char firmware_message[MAX_STR_LENGTH];

int isFirmwareCheckDone() {
	return firmware_check_done;
}

int getFirmwareCheckResult(char** mess) {
	*mess = firmware_message;
	return firmware_compatibility;
}

void checkFirmwareCompatibility(int flag) {
#ifdef VIRTUAL
    cprintf(BLUE,"\n\n"
            "********************************************************\n"
            "************** Jungfrau Virtual Server *****************\n"
            "********************************************************\n\n");
    if (mapCSP0() == FAIL) {
    	strcpy(firmware_message,
				"FATAL ERROR: Could not map to memory. Dangerous to continue.\n");
		cprintf(RED,"%s\n\n", firmware_message);
		firmware_compatibility = FAIL;
		firmware_check_done = 1;
		return;
    }
    firmware_check_done = 1;
    return;
#endif

	defineGPIOpins();
	resetFPGA();
    if (mapCSP0() == FAIL) {
    	strcpy(firmware_message,
				"FATAL ERROR: Could not map to memory. Dangerous to continue.\n");
		cprintf(RED,"%s\n\n", firmware_message);
		firmware_compatibility = FAIL;
		firmware_check_done = 1;
		return;
    }

    // does check only if flag is 0 (by default), set by command line
	if ((!flag) && ((checkType() == FAIL) || (testFpga() == FAIL) || (testBus() == FAIL))) {
		strcpy(firmware_message,
				"FATAL ERROR: Could not pass basic tests of FPGA and bus. Dangerous to continue.\n");
		cprintf(RED,"%s\n\n", firmware_message);
		firmware_compatibility = FAIL;
		firmware_check_done = 1;
		return;
	}

	uint16_t hversion			= getHardwareVersionNumber();
	uint16_t hsnumber			= getHardwareSerialNumber();
	uint32_t ipadd				= getDetectorIP();
	uint64_t macadd				= getDetectorMAC();
	int64_t fwversion 			= getDetectorId(DETECTOR_FIRMWARE_VERSION);
	int64_t swversion 			= getDetectorId(DETECTOR_SOFTWARE_VERSION);
	int64_t sw_fw_apiversion    = 0;
	int64_t client_sw_apiversion = getDetectorId(CLIENT_SOFTWARE_API_VERSION);


	if (fwversion >= MIN_REQRD_VRSN_T_RD_API)
	    sw_fw_apiversion 	    = getDetectorId(SOFTWARE_FIRMWARE_API_VERSION);
	cprintf(BLUE,"\n\n"
			"********************************************************\n"
			"****************** Jungfrau Server *********************\n"
			"********************************************************\n\n"
			"Hardware Version:\t\t 0x%x\n"
			"Hardware Serial Nr:\t\t 0x%x\n"

			"Detector IP Addr:\t\t 0x%x\n"
			"Detector MAC Addr:\t\t 0x%llx\n\n"

			"Firmware Version:\t\t 0x%llx\n"
			"Software Version:\t\t 0x%llx\n"
			"F/w-S/w API Version:\t\t 0x%llx\n"
			"Required Firmware Version:\t 0x%x\n"
			"Client-Software API Version:\t 0x%llx\n"
			"\n"
			"********************************************************\n",
			hversion, hsnumber,
			ipadd,
			(long  long unsigned int)macadd,
			(long  long int)fwversion,
			(long  long int)swversion,
			(long  long int)sw_fw_apiversion,
			REQRD_FRMWR_VRSN,
			(long long int)client_sw_apiversion
	);

	// return if flag is not zero, debug mode
	if (flag) {
		firmware_check_done = 1;
		return;
	}


	//cant read versions
    printf("Testing Firmware-software compatibility ...\n");
	if(!fwversion || !sw_fw_apiversion){
		strcpy(firmware_message,
				"FATAL ERROR: Cant read versions from FPGA. Please update firmware.\n");
		cprintf(RED,"%s\n\n", firmware_message);
		firmware_compatibility = FAIL;
		firmware_check_done = 1;
		return;
	}

	//check for API compatibility - old server
	if(sw_fw_apiversion > REQRD_FRMWR_VRSN){
		sprintf(firmware_message,
				"FATAL ERROR: This detector software software version (0x%llx) is incompatible.\n"
				"Please update detector software (min. 0x%llx) to be compatible with this firmware.\n",
				(long long int)sw_fw_apiversion,
				(long long int)REQRD_FRMWR_VRSN);
		cprintf(RED,"%s\n\n", firmware_message);
		firmware_compatibility = FAIL;
		firmware_check_done = 1;
		return;
	}

	//check for firmware compatibility - old firmware
	if( REQRD_FRMWR_VRSN > fwversion){
		sprintf(firmware_message,
				"FATAL ERROR: This firmware version (0x%llx) is incompatible.\n"
				"Please update firmware (min. 0x%llx) to be compatible with this server.\n",
				(long long int)fwversion,
				(long long int)REQRD_FRMWR_VRSN);
		cprintf(RED,"%s\n\n", firmware_message);
		firmware_compatibility = FAIL;
		firmware_check_done = 1;
		return;
	}
	printf("Compatibility - success\n");
	firmware_check_done = 1;
}


int checkType() {
#ifdef VIRTUAL
    return OK;
#endif
	volatile u_int32_t type = ((bus_r(FPGA_VERSION_REG) & DETECTOR_TYPE_MSK) >> DETECTOR_TYPE_OFST);
	if (type != JUNGFRAU){
			cprintf(BG_RED,"This is not a Jungfrau Server (read %d, expected %d)\n",type, JUNGFRAU);
			return FAIL;
		}

	return OK;
}



u_int32_t testFpga(void) {
#ifdef VIRTUAL
    return OK;
#endif
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
	return ret;
}


int testBus() {
#ifdef VIRTUAL
    return OK;
#endif
	printf("\nTesting Bus...\n");

	int ret = OK;
	u_int32_t addr = SET_TRIGGER_DELAY_LSB_REG;
	int times = 1000 * 1000;
	int i = 0;

	for (i = 0; i < times; ++i) {
		bus_w(addr, i * 100);
		if (i * 100 != bus_r(addr)) {
			cprintf(RED,"ERROR: Mismatch! Wrote 0x%x, read 0x%x\n",
					i * 100, bus_r(addr));
			ret = FAIL;
		}
	}

	bus_w(addr, 0);

	if (ret == OK)
		printf("Successfully tested bus %d times\n", times);
	return ret;
}


int moduleTest( enum digitalTestMode arg, int imod){
	return OK;
}

int detectorTest( enum digitalTestMode arg){
#ifdef VIRTUAL
    return OK;
#endif
	switch(arg){
	case DETECTOR_FIRMWARE_TEST:	return testFpga();
	case DETECTOR_BUS_TEST: 		return testBus();
	//DETECTOR_MEMORY_TEST:testRAM
	//DETECTOR_SOFTWARE_TEST:
	default:
		cprintf(RED,"Warning: Test not implemented for this detector %d\n", (int)arg);
		break;
	}
	return OK;
}





/* Ids */

int64_t getDetectorId(enum idMode arg){
	int64_t retval = -1;

	switch(arg){
	case DETECTOR_SERIAL_NUMBER:
		return getDetectorNumber();// or getDetectorMAC()
	case DETECTOR_FIRMWARE_VERSION:
		return getFirmwareVersion();
	case SOFTWARE_FIRMWARE_API_VERSION:
	    return getFirmwareAPIVersion();
	case DETECTOR_SOFTWARE_VERSION:
		return  (GITDATE & 0xFFFFFF);
	case CLIENT_SOFTWARE_API_VERSION:
		return APIJUNGFRAU;
	default:
		return retval;
	}
}

u_int64_t getFirmwareVersion() {
#ifdef VIRTUAL
    return 0;
#endif
	return ((bus_r(FPGA_VERSION_REG) & BOARD_REVISION_MSK) >> BOARD_REVISION_OFST);
}

u_int64_t getFirmwareAPIVersion() {
#ifdef VIRTUAL
    return 0;
#endif
    return ((bus_r(API_VERSION_REG) & API_VERSION_MSK) >> API_VERSION_OFST);
}

u_int16_t getHardwareVersionNumber() {
#ifdef VIRTUAL
    return 0;
#endif
	return ((bus_r(MOD_SERIAL_NUM_REG) & HARDWARE_VERSION_NUM_MSK) >> HARDWARE_VERSION_NUM_OFST);
}

u_int16_t getHardwareSerialNumber() {
#ifdef VIRTUAL
    return 0;
#endif
	return ((bus_r(MOD_SERIAL_NUM_REG) & HARDWARE_SERIAL_NUM_MSK) >> HARDWARE_SERIAL_NUM_OFST);
}

u_int32_t getDetectorNumber(){
#ifdef VIRTUAL
    return 0;
#endif
	return bus_r(MOD_SERIAL_NUM_REG);
}

u_int64_t  getDetectorMAC() {
#ifdef VIRTUAL
    return 0;
#else
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
#endif
}

u_int32_t  getDetectorIP(){
#ifdef VIRTUAL
    return 0;
#endif
	char temp[50]="";
	u_int32_t res=0;
	//execute and get address
	char output[255];
	FILE* sysFile = popen("ifconfig  | grep 'inet addr:'| grep -v '127.0.0.1' | cut -d: -f2", "r");
	fgets(output, sizeof(output), sysFile);
	pclose(sysFile);

	//converting IPaddress to hex.
	char* pcword = strtok (output,".");
	while (pcword != NULL) {
		sprintf(output,"%02x",atoi(pcword));
		strcat(temp,output);
		pcword = strtok (NULL, ".");
	}
	strcpy(output,temp);
	sscanf(output, "%x", 	&res);
	//printf("ip:%x\n",res);

	return res;
}








/* initialization */

void initControlServer(){
    clkPhase[0] = 0; clkPhase[1] = 0;
	setupDetector();
	printf("\n");
}



void initStopServer() {

	usleep(CTRL_SRVR_INIT_TIME_US);
	if (mapCSP0() == FAIL) {
		cprintf(BG_RED, "Stop Server: Map Fail. Dangerous to continue. Goodbye!\n");
		exit(EXIT_FAILURE);
	}
}






/* set up detector */

void allocateDetectorStructureMemory(){
	printf("This Server is for 1 Jungfrau module (500k)\n");

	//Allocation of memory
	if (detectorModules!=NULL) free(detectorModules);
	if (detectorDacs!=NULL) free(detectorDacs);
	if (detectorAdcs!=NULL) free(detectorAdcs);
	detectorModules=malloc(sizeof(sls_detector_module));
	detectorDacs=malloc(NDAC*sizeof(dacs_t));
	detectorAdcs=malloc(NADC*sizeof(dacs_t));
#ifdef VERBOSE
	printf("modules from 0x%x to 0x%x\n",detectorModules, detectorModules+n);
	printf("dacs from 0x%x to 0x%x\n",detectorDacs, detectorDacs+n*NDAC);
	printf("adcs from 0x%x to 0x%x\n",detectorAdcs, detectorAdcs+n*NADC);
#endif
	(detectorModules)->dacs=detectorDacs;
	(detectorModules)->adcs=detectorAdcs;
	(detectorModules)->ndac=NDAC;
	(detectorModules)->nadc=NADC;
	(detectorModules)->nchip=NCHIP;
	(detectorModules)->nchan=NCHIP*NCHAN;
	(detectorModules)->module=0;
	(detectorModules)->gain=0;
	(detectorModules)->offset=0;
	(detectorModules)->reg=0;
	thisSettings = UNINITIALIZED;

	{ // initialize to -1
		int i = 0;
		for (i = 0; i < NDAC; ++i) {
			dacValues[i] = -1;
		}
	}
}



void setupDetector() {

	allocateDetectorStructureMemory();

	printf("Resetting PLL\n");
	resetPLL();
	resetCore();
	resetPeripheral();
	cleanFifos();
#ifndef VIRTUAL
	prepareADC();
#endif
	// initialize dac series
	initDac(0);		/* todo might work without */
	initDac(8); 	//only for old board compatibility

	//set dacs
	setDefaultDacs();

	bus_w(DAQ_REG, 0x0);         /* Only once at server startup */

	setSpeed(CLOCK_DIVIDER, HALF_SPEED);
	cleanFifos();
	resetCore();

	configureASICTimer();
	bus_w(ADC_PORT_INVERT_REG, ADC_PORT_INVERT_VAL);

	//Initialization of acquistion parameters
	setSettings(DEFAULT_SETTINGS,-1);

	setTimer(FRAME_NUMBER, DEFAULT_NUM_FRAMES);
	setTimer(CYCLES_NUMBER, DEFAULT_NUM_CYCLES);
	setTimer(ACQUISITION_TIME, DEFAULT_EXPTIME);
	setTimer(FRAME_PERIOD, DEFAULT_PERIOD);
	setTimer(DELAY_AFTER_TRIGGER, DEFAULT_DELAY);
	setTimer(STORAGE_CELL_NUMBER, DEFAULT_NUM_STRG_CLLS);
	selectStoragecellStart(DEFAULT_STRG_CLL_STRT);
	/*setSpeed(CLOCK_DIVIDER, HALF_SPEED); depends if all the previous stuff works*/
	setTiming(DEFAULT_TIMING_MODE);
	setHighVoltage(DEFAULT_HIGH_VOLTAGE);

	/* temporary set up until new firmware fixes bug */
	// set temperature threshold
	setThresholdTemperature(DEFAULT_TMP_THRSHLD);
	// reset temp event
	setTemperatureEvent(0);


}


int setDefaultDacs() {
	int ret = OK;
	printf("Setting Default Dac values\n");
	{
		int i = 0;
		int retval[2]={-1,-1};
		const int defaultvals[NDAC] = DEFAULT_DAC_VALS;
		for(i = 0; i < NDAC; ++i) {
			// if not already default, set it to default
			if (dacValues[i] != defaultvals[i]) {
				setDAC((enum DACINDEX)i,defaultvals[i],0,0,retval);
				if (retval[0] != defaultvals[i]) {
					cprintf(RED, "Warning: Setting dac %d failed, wrote %d, read %d\n",i ,defaultvals[i], retval[0]);
					ret = FAIL;
				}
			}
		}
	}
	return ret;
}




/* firmware functions (resets) */

int powerChip (int on){
	if(on != -1){
		if(on){
			cprintf(BLUE, "\n*** Powering on the chip ***\n");
			bus_w(CHIP_POWER_REG, bus_r(CHIP_POWER_REG) | CHIP_POWER_ENABLE_MSK);
		}
		else{
			cprintf(BLUE, "\n*** Powering off the chip*** \n");
			bus_w(CHIP_POWER_REG, bus_r(CHIP_POWER_REG) & ~CHIP_POWER_ENABLE_MSK);
		}
	}

	//return ((bus_r(CHIP_POWER_REG) & CHIP_POWER_ENABLE_MSK) >> CHIP_POWER_ENABLE_OFST);
	/**temporary fix until power reg status can be read */
	return ((bus_r(CHIP_POWER_REG) & CHIP_POWER_STATUS_MSK) >> CHIP_POWER_STATUS_OFST);
}



int autoCompDisable(int on) {
    if(on != -1){
        if(on){
            cprintf(BLUE, "\n*** Auto comp disable mode: enabling ***\n");
            bus_w(VREF_COMP_MOD_REG, bus_r(VREF_COMP_MOD_REG) | VREF_COMP_MOD_ENABLE_MSK);
        }
        else{
            cprintf(BLUE, "\n*** Auto comp disable mode: disabling *** \n");
            bus_w(VREF_COMP_MOD_REG, bus_r(VREF_COMP_MOD_REG) & ~VREF_COMP_MOD_ENABLE_MSK);
        }
    }

    return (bus_r(VREF_COMP_MOD_REG) & VREF_COMP_MOD_ENABLE_MSK);
}


void cleanFifos() {
#ifdef VIRTUAL
    return;
#endif
	printf("\nClearing Acquisition Fifos\n");
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_ACQ_FIFO_CLR_MSK);
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_ACQ_FIFO_CLR_MSK);
}

void resetCore() {
#ifdef VIRTUAL
    return;
#endif
	printf("\nResetting Core\n");
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_CORE_RST_MSK);
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_CORE_RST_MSK);
}

void resetPeripheral() {
#ifdef VIRTUAL
    return;
#endif
	printf("\nResetting Peripheral\n");
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_PERIPHERAL_RST_MSK);
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_PERIPHERAL_RST_MSK);
}

int adcPhase(int st){ /**carlos needed clkphase 1 and 2?  cehck with Aldo */
	printf("Setting ADC Phase to %d\n",st);
	if (st > 65535 || st < -65535)
		return clkPhase[0];

	clkPhase[1] = st - clkPhase[0];
	if (clkPhase[1] == 0)
	    return clkPhase[0];

	configurePll();
	clkPhase[0] = st;
	return clkPhase[0];
}

int getPhase() {
	return clkPhase[0];
}

void configureASICTimer() {
    printf("\nConfiguring ASIC Timer\n");
    bus_w(ASIC_CTRL_REG, (bus_r(ASIC_CTRL_REG) & ~ASIC_CTRL_PRCHRG_TMR_MSK) | ASIC_CTRL_PRCHRG_TMR_VAL);
    bus_w(ASIC_CTRL_REG, (bus_r(ASIC_CTRL_REG) & ~ASIC_CTRL_DS_TMR_MSK) | ASIC_CTRL_DS_TMR_VAL);
}








/* set parameters - nmod, dr, roi */

int setNMod(int nm, enum dimension dim){
	return NMOD;
}


int getNModBoard(enum dimension arg){
	return NMAXMOD;
}


int setDynamicRange(int dr){
	return DYNAMIC_RANGE;
}




/* parameters - readout */

int setSpeed(enum speedVariable arg, int val) {

	if (arg != CLOCK_DIVIDER)
		return -1;

	// setting
	if(val >= 0) {

        // stop state machine if running
        if(runBusy())
            stopStateMachine();

        uint32_t txndelay_msk = 0;

		switch(val){

		// todo in firmware, for now setting half speed
		case FULL_SPEED://40
			printf("\nSetting Half Speed (20 MHz):\n");

			printf("Setting Sample Reg to 0x%x\n", SAMPLE_ADC_HALF_SPEED);
			bus_w(SAMPLE_REG, SAMPLE_ADC_HALF_SPEED);

	        txndelay_msk = (bus_r(CONFIG_REG) & CONFIG_TDMA_TIMESLOT_MSK); // read config tdma timeslot value
			printf("Setting Config Reg to 0x%x\n", CONFIG_HALF_SPEED | txndelay_msk);
			bus_w(CONFIG_REG, CONFIG_HALF_SPEED | txndelay_msk);

			printf("Setting ADC Ofst Reg to 0x%x\n", ADC_OFST_HALF_SPEED_VAL);
			bus_w(ADC_OFST_REG, ADC_OFST_HALF_SPEED_VAL);

			printf("Setting ADC Phase Reg to 0x%x\n", ADC_PHASE_HALF_SPEED);
			adcPhase(ADC_PHASE_HALF_SPEED);

			break;
		case HALF_SPEED:
			printf("\nSetting Half Speed (20 MHz):\n");

			printf("Setting Sample Reg to 0x%x\n", SAMPLE_ADC_HALF_SPEED);
			bus_w(SAMPLE_REG, SAMPLE_ADC_HALF_SPEED);

			txndelay_msk = (bus_r(CONFIG_REG) & CONFIG_TDMA_TIMESLOT_MSK); // read config tdma timeslot value
			printf("Setting Config Reg to 0x%x\n", CONFIG_HALF_SPEED | txndelay_msk);
			bus_w(CONFIG_REG, CONFIG_HALF_SPEED | txndelay_msk);

			printf("Setting ADC Ofst Reg to 0x%x\n", ADC_OFST_HALF_SPEED_VAL);
			bus_w(ADC_OFST_REG, ADC_OFST_HALF_SPEED_VAL);

			printf("Setting ADC Phase Reg to 0x%x\n", ADC_PHASE_HALF_SPEED);
			adcPhase(ADC_PHASE_HALF_SPEED);

			break;
		case QUARTER_SPEED:
			printf("\nSetting Half Speed (10 MHz):\n");

			printf("Setting Sample Reg to 0x%x\n", SAMPLE_ADC_QUARTER_SPEED);
			bus_w(SAMPLE_REG, SAMPLE_ADC_QUARTER_SPEED);

			txndelay_msk = (bus_r(CONFIG_REG) & CONFIG_TDMA_TIMESLOT_MSK); // read config tdma timeslot value
			printf("Setting Config Reg to 0x%x\n", CONFIG_QUARTER_SPEED | txndelay_msk);
			bus_w(CONFIG_REG, CONFIG_QUARTER_SPEED | txndelay_msk);

			printf("Setting ADC Ofst Reg to 0x%x\n", ADC_OFST_QUARTER_SPEED_VAL);
			bus_w(ADC_OFST_REG, ADC_OFST_QUARTER_SPEED_VAL);

			printf("Setting ADC Phase Reg to 0x%x\n", ADC_PHASE_QUARTER_SPEED);
			adcPhase(ADC_PHASE_QUARTER_SPEED);

			break;
		}
		printf("\n");
	}

	//getting
	u_int32_t speed = bus_r(CONFIG_REG) & CONFIG_READOUT_SPEED_MSK;
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






/* parameters - timer */
int selectStoragecellStart(int pos) {
    if (pos >= 0) {
        bus_w(DAQ_REG, bus_r(DAQ_REG) & ~DAQ_STRG_CELL_SLCT_MSK);
        bus_w(DAQ_REG, bus_r(DAQ_REG) | ((pos << DAQ_STRG_CELL_SLCT_OFST) & DAQ_STRG_CELL_SLCT_MSK));
    }
    return ((bus_r(DAQ_REG) & DAQ_STRG_CELL_SLCT_MSK) >> DAQ_STRG_CELL_SLCT_OFST);
}



int64_t setTimer(enum timerIndex ind, int64_t val) {

	int64_t retval = -1;
	switch(ind){

	case FRAME_NUMBER:
		if(val >= 0)
			printf("\nSetting #frames: %lld\n",(long long int)val);
		retval = set64BitReg(val,  SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG);
		printf("Getting #frames: %lld\n",(long long int)retval);
		break;

	case ACQUISITION_TIME:
		if(val >= 0){
			printf("\nSetting exptime: %lldns\n", (long long int)val);
			val *= (1E-3 * CLK_RUN);
			val -= ACQ_TIME_MIN_CLOCK;
			if(val < 0) val = 0;
		}
		retval = (set64BitReg(val, SET_EXPTIME_LSB_REG, SET_EXPTIME_MSB_REG) + ACQ_TIME_MIN_CLOCK) / (1E-3 * CLK_RUN);
		printf("Getting exptime: %lldns\n", (long long int)retval);
		break;

	case FRAME_PERIOD:
		if(val >= 0){
			printf("\nSetting period to %lldns\n",(long long int)val);
			val *= (1E-3 * CLK_SYNC);
		}
		retval = set64BitReg(val, SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG )/ (1E-3 * CLK_SYNC);
		printf("Getting period: %lldns\n", (long long int)retval);
		break;

	case DELAY_AFTER_TRIGGER:
		if(val >= 0){
			printf("\nSetting delay to %lldns\n", (long long int)val);
			val *= (1E-3 * CLK_SYNC);
		}
		retval = set64BitReg(val, SET_TRIGGER_DELAY_LSB_REG, SET_TRIGGER_DELAY_MSB_REG) / (1E-3 * CLK_SYNC);
		printf("Getting delay: %lldns\n", (long long int)retval);
		break;

	case CYCLES_NUMBER:
		if(val >= 0)
			printf("\nSetting #cycles to %lld\n", (long long int)val);
		retval = set64BitReg(val,  SET_CYCLES_LSB_REG, SET_CYCLES_MSB_REG);
		printf("Getting #cycles: %lld\n", (long long int)retval);
		break;

	case STORAGE_CELL_NUMBER:
        if(val >= 0) {
            printf("\nSetting #storage cells to %lld\n", (long long int)val);
            bus_w(CONTROL_REG, (bus_r(CONTROL_REG) & ~CONTROL_STORAGE_CELL_NUM_MSK) |
                    ((val << CONTROL_STORAGE_CELL_NUM_OFST) & CONTROL_STORAGE_CELL_NUM_MSK));
        }
        retval = ((bus_r(CONTROL_REG) & CONTROL_STORAGE_CELL_NUM_MSK) >> CONTROL_STORAGE_CELL_NUM_OFST);
        printf("Getting #storage cells: %lld\n", (long long int)retval);
        break;

	default:
		cprintf(RED,"Warning: Timer Index not implemented for this detector: %d\n", ind);
		break;
	}

	return retval;

}



int64_t getTimeLeft(enum timerIndex ind){
#ifdef VIRTUAL
    return 0;
#endif
	int64_t retval = -1;
	switch(ind){

	case FRAME_NUMBER:
		retval = get64BitReg(GET_FRAMES_LSB_REG, GET_FRAMES_MSB_REG);
		printf("Getting number of frames left: %lld\n",(long long int)retval);
		break;

	case FRAME_PERIOD:
		retval = get64BitReg(GET_PERIOD_LSB_REG, GET_PERIOD_MSB_REG) / (1E-3 * CLK_SYNC);
		printf("Getting period left: %lldns\n", (long long int)retval);
		break;
/*
	case DELAY_AFTER_TRIGGER:
		retval = get64BitReg(xxx) / (1E-3 * CLK_SYNC);
		printf("Getting delay left: %lldns\n", (long long int)retval);
		break;
*/
	case CYCLES_NUMBER:
		retval = get64BitReg(GET_CYCLES_LSB_REG, GET_CYCLES_MSB_REG);
		printf("Getting number of cycles left: %lld\n", (long long int)retval);
		break;

	case ACTUAL_TIME:
		retval = get64BitReg(TIME_FROM_START_LSB_REG, TIME_FROM_START_MSB_REG) / (1E-9 * CLK_SYNC);
		printf("Getting actual time (time from start): %lld\n", (long long int)retval);
		break;

	case MEASUREMENT_TIME:
		retval = get64BitReg(START_FRAME_TIME_LSB_REG, START_FRAME_TIME_MSB_REG) / (1E-9 * CLK_SYNC);
		printf("Getting measurement time (timestamp/ start frame time): %lld\n", (long long int)retval);
		break;

	case FRAMES_FROM_START:
	case FRAMES_FROM_START_PG:
		retval = get64BitReg(FRAMES_FROM_START_PG_LSB_REG, FRAMES_FROM_START_PG_MSB_REG);
		printf("Getting frames from start run control %lld\n", (long long int)retval);
		break;

	default:
		cprintf(RED, "Warning: Remaining Timer index not implemented for this detector: %d\n", ind);
		break;
	}

	return retval;
}






/* parameters - channel, chip, module, settings */


int setModule(sls_detector_module myMod){
	int retval[2];
	int i;

	//#ifdef VERBOSE
	printf("Setting module with settings %d\n",myMod.reg);
	//#endif

	setSettings( (enum detectorSettings)myMod.reg,-1);

	//copy module locally
	if (detectorModules)
		copyModule(detectorModules,&myMod);

	//set dac values
	for(i=0;i<myMod.ndac;i++)
		setDAC((enum DACINDEX)i,myMod.dacs[i],myMod.module,0,retval);

	return thisSettings;
}


int getModule(sls_detector_module *myMod){
	int i;
	int retval[2];

	//dacs
	for(i=0;i<NDAC;i++)
		setDAC((enum DACINDEX)i,-1,-1,0,retval);

	//copy from local copy
	if (detectorModules)
		copyModule(myMod,detectorModules);
	else
		return FAIL;
	return OK;
}



enum detectorSettings setSettings(enum detectorSettings sett, int imod){
	if(sett == UNINITIALIZED)
		return thisSettings;

	// set settings
	if(sett != GET_SETTINGS) {
	    switch (sett) {
	    case DYNAMICGAIN:
	        bus_w(DAQ_REG, bus_r(DAQ_REG) & ~DAQ_SETTINGS_MSK);
            printf("\nConfigured settings - Dyanmic Gain, DAQ Reg: 0x%x\n", bus_r(DAQ_REG));
	        break;
	    case DYNAMICHG0:
            bus_w(DAQ_REG, bus_r(DAQ_REG) & ~DAQ_SETTINGS_MSK);
            bus_w(DAQ_REG, bus_r(DAQ_REG) | DAQ_FIX_GAIN_HIGHGAIN_VAL);
            printf("\nConfigured settings - Dyanmic High Gain 0, DAQ Reg: 0x%x\n", bus_r(DAQ_REG));
            break;
	    case FIXGAIN1:
            bus_w(DAQ_REG, bus_r(DAQ_REG) & ~DAQ_SETTINGS_MSK);
            bus_w(DAQ_REG, bus_r(DAQ_REG) | DAQ_FIX_GAIN_STG_1_VAL);
            printf("\nConfigured settings - Fix Gain 1, DAQ Reg: 0x%x\n", bus_r(DAQ_REG));
            break;
	    case FIXGAIN2:
            bus_w(DAQ_REG, bus_r(DAQ_REG) & ~DAQ_SETTINGS_MSK);
            bus_w(DAQ_REG, bus_r(DAQ_REG) | DAQ_FIX_GAIN_STG_2_VAL);
            printf("\nConfigured settings - Fix Gain 2, DAQ Reg: 0x%x\n", bus_r(DAQ_REG));
            break;
	    case FORCESWITCHG1:
            bus_w(DAQ_REG, bus_r(DAQ_REG) & ~DAQ_SETTINGS_MSK);
            bus_w(DAQ_REG, bus_r(DAQ_REG) | DAQ_FRCE_GAIN_STG_1_VAL);
            printf("\nConfigured settings - Force Switch Gain 1, DAQ Reg: 0x%x\n", bus_r(DAQ_REG));
            break;
	    case FORCESWITCHG2:
            bus_w(DAQ_REG, bus_r(DAQ_REG) & ~DAQ_SETTINGS_MSK);
            bus_w(DAQ_REG, bus_r(DAQ_REG) | DAQ_FRCE_GAIN_STG_2_VAL);
            printf("\nConfigured settings - Force Switch Gain 2, DAQ Reg: 0x%x\n", bus_r(DAQ_REG));
            break;
	    default:
	        cprintf(RED, "Error: This settings is not defined for this detector %d\n", (int)sett);
	        return -1;
	    }

		thisSettings = sett;
	}

	return getSettings();

}


enum detectorSettings getSettings(){

	uint32_t val = bus_r(DAQ_REG) & DAQ_SETTINGS_MSK;
	printf("\nGetting Settings\n Reading DAQ Register :0x%x\n", val);

	switch(val) {
	case DAQ_FIX_GAIN_DYNMC_VAL:
        thisSettings = DYNAMICGAIN;
        printf("Settings read: DYNAMICGAIN\n");
        break;
    case DAQ_FIX_GAIN_HIGHGAIN_VAL:
        thisSettings = DYNAMICHG0;
        printf("Settings read: DYNAMICHG0\n");
        break;
    case DAQ_FIX_GAIN_STG_1_VAL:
        thisSettings = FIXGAIN1;
        printf("Settings read: FIXGAIN1\n");
        break;
    case DAQ_FIX_GAIN_STG_2_VAL:
        thisSettings = FIXGAIN2;
        printf("Settings read: FIXGAIN2\n");
        break;
    case DAQ_FRCE_GAIN_STG_1_VAL:
        thisSettings = FORCESWITCHG1;
        printf("Settings read: FORCESWITCHG1\n");
        break;
    case DAQ_FRCE_GAIN_STG_2_VAL:
        thisSettings = FORCESWITCHG2;
        printf("Settings read: FORCESWITCHG2\n");
        break;
    default:
        thisSettings = UNDEFINED;
        printf("Settings read: Undefined. Value read:0x%x\n", val);
	}

	return thisSettings;
}





/* parameters - dac, adc, hv */






void initDac(int dacnum) {
#ifdef VIRTUAL
    return;
#endif
	printf("\nInitializing dac for %d to \n",dacnum);

	u_int32_t codata;
	int csdx 		= dacnum / NDAC + DAC_SERIAL_CS_OUT_OFST; 	// old board (16 dacs),so can be DAC_SERIAL_CS_OUT_OFST or +1
	int dacchannel 	= 0xf;										// all channels
	int dacvalue	= 0x6; 										// can be any random value (just writing to power up)
	printf(" Write to Input Register\n"
			" Chip select bit:%d\n"
			" Dac Channel:0x%x\n"
			" Dac Value:0x%x\n",
			csdx, dacchannel, dacvalue);

	codata = LTC2620_DAC_CMD_WRITE +											// command to write to input register
			((dacchannel << LTC2620_DAC_ADDR_OFST) & LTC2620_DAC_ADDR_MSK) +	// all channels
			((dacvalue << LTC2620_DAC_DATA_OFST) & LTC2620_DAC_DATA_MSK);		// any random value
	serializeToSPI(SPI_REG, codata, (0x1 << csdx), LTC2620_DAC_NUMBITS,
			DAC_SERIAL_CLK_OUT_MSK, DAC_SERIAL_DIGITAL_OUT_MSK, DAC_SERIAL_DIGITAL_OUT_OFST);
}









int voltageToDac(int value){
	int vmin = 0;
	int vmax = 2500;
	int nsteps = 4096;
	if ((value < vmin) || (value > vmax)) {
		cprintf(RED,"Voltage value (to convert to dac value) is outside bounds: %d\n", value);
		return -1;
	}
	return (int)(((value - vmin) / (vmax - vmin)) * (nsteps - 1) + 0.5);
}

int dacToVoltage(unsigned int digital){
	int vmin = 0;
	int vmax = 2500;
	int nsteps = 4096;
	int v = vmin + (vmax - vmin) * digital / (nsteps - 1);
	if((v < 0) || (v > nsteps - 1)) {
		cprintf(RED,"Voltage value (converted from dac value) is outside bounds: %d\n", v);
		return -1;
	}
	return v;
}



void setDAC(enum DACINDEX ind, int val, int imod, int mV, int retval[]){
	int dacval = val;

	//if set and mv, convert to dac
	if (val > 0 && mV) {
		val = voltageToDac(val); //gives -1 on error
	}

	if ( (val >= 0) || (val == -100)) {
#ifdef VIRTUAL
	    dacValues[ind] = dacval;
#else
		u_int32_t codata;
		int csdx 		= ind / NDAC + DAC_SERIAL_CS_OUT_OFST; 	// old board (16 dacs),so can be DAC_SERIAL_CS_OUT_OFST or +1
		int dacchannel 	= ind % NDAC;							// 0-8, dac channel number (also for dacnum 9-15 in old board)

		printf("\nSetting of DAC %d : %d dac units (%d mV)\n",ind, dacval, val);
		// command
		if (val >= 0) {
			printf(" Write to Input Register and Update\n");
			codata = LTC2620_DAC_CMD_SET;

		} else if (val == -100) {
			printf(" POWER DOWN\n");
			codata = LTC2620_DAC_CMD_POWER_DOWN;
		}
		// address
		printf(" Chip select bit:%d\n"
				" Dac Channel:0x%x\n"
				" Dac Value:0x%x\n",
				csdx, dacchannel, val);
		codata += ((dacchannel << LTC2620_DAC_ADDR_OFST) & LTC2620_DAC_ADDR_MSK) +
				((val << LTC2620_DAC_DATA_OFST) & LTC2620_DAC_DATA_MSK);
		// to spi
		serializeToSPI(SPI_REG, codata, (0x1 << csdx), LTC2620_DAC_NUMBITS,
				DAC_SERIAL_CLK_OUT_MSK, DAC_SERIAL_DIGITAL_OUT_MSK, DAC_SERIAL_DIGITAL_OUT_OFST);

		dacValues[ind] = dacval;

		if (ind == VREF_COMP) {
		    bus_w (VREF_COMP_MOD_REG, (bus_r(VREF_COMP_MOD_REG) &~ (VREF_COMP_MOD_MSK))   // reset
		            | ((val << VREF_COMP_MOD_OFST) & VREF_COMP_MOD_MSK));   // or it with value
		}
#endif
	}

	printf("Getting DAC %d : ",ind);
	retval[0] = dacValues[ind];		printf("%d dac units ", retval[0]);
	retval[1] = dacToVoltage(retval[0]);printf("(%d mV)\n", retval[1]);
}


int getADC(enum ADCINDEX ind,  int imod){
#ifdef VIRTUAL
    return 0;
#endif
	char tempnames[2][40]={"VRs/FPGAs Temperature", "ADCs/ASICs Temperature"};
	printf("Getting Temperature for %s\n",tempnames[ind]);
	u_int32_t addr = GET_TEMPERATURE_TMP112_REG;
	uint32_t regvalue = bus_r(addr);
	uint32_t value = regvalue & TEMPERATURE_VALUE_MSK;
	double retval = value;

	// negative
	if (regvalue & TEMPERATURE_POLARITY_MSK) {
		// 2s complement
		int ret = (~value) + 1;
		// attach negative sign
		ret = 0 - value;
		retval = ret;
	}

	// conversion
	retval *= 625.0/10.0;
	printf("\nReal Temperature %s: %f °C\n",tempnames[ind],retval/1000.00);
	return retval;
}



int setHighVoltage(int val){
#ifdef VIRTUAL
    if (val >= 0)
        highvoltage = val;
    return highvoltage;
#endif
	u_int32_t dacvalue;
	float alpha		= 0.55;
	// setting hv
	if (val >= 0) {
		// limit values
		if (val < 60) {
			dacvalue = 0;
			val = 0;
		} else if (val >= 200) {
			dacvalue = 0x1;
			val = 200;
		} else {
			dacvalue = 1. + (200.-val) / alpha;
			val=200.-(dacvalue-1)*alpha;
		}
		printf ("\nSetting High voltage to %d (dacval %d)\n",val, dacvalue);
		dacvalue &= MAX1932_HV_DATA_MSK;
		serializeToSPI(SPI_REG, dacvalue, HV_SERIAL_CS_OUT_MSK, MAX1932_HV_NUMBITS,
				HV_SERIAL_CLK_OUT_MSK, HV_SERIAL_DIGITAL_OUT_MSK, HV_SERIAL_DIGITAL_OUT_OFST);
		highvoltage = val;
	}
	return highvoltage;
}






/* parameters - timing, extsig */


enum externalCommunicationMode setTiming( enum externalCommunicationMode arg){

	if(arg != GET_EXTERNAL_COMMUNICATION_MODE){
		switch((int)arg){
		case AUTO_TIMING:			bus_w(EXT_SIGNAL_REG, bus_r(EXT_SIGNAL_REG) & ~EXT_SIGNAL_MSK);	break;
		case TRIGGER_EXPOSURE:		bus_w(EXT_SIGNAL_REG, bus_r(EXT_SIGNAL_REG) | EXT_SIGNAL_MSK);	break;
		default:
			cprintf(RED,"Unknown timing mode %d\n", arg);
			return GET_EXTERNAL_COMMUNICATION_MODE;
		}
	}
	if (bus_r(EXT_SIGNAL_REG) == EXT_SIGNAL_MSK)
		return TRIGGER_EXPOSURE;
	return AUTO_TIMING;
}





/* configure mac */


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



int configureMAC(uint32_t destip, uint64_t destmac, uint64_t sourcemac, uint32_t sourceip, uint32_t udpport, uint32_t udpport2, int ival){
#ifdef VIRTUAL
    return 0;
#endif
	cprintf(BLUE, "\n*** Configuring MAC ***\n");
	uint32_t sourceport  =  DEFAULT_TX_UDP_PORT;

	printf("Source IP   : %d.%d.%d.%d \t\t(0x%08x)\n",(sourceip>>24)&0xff,(sourceip>>16)&0xff,(sourceip>>8)&0xff,(sourceip)&0xff, sourceip);
	printf("Source MAC  : %02x:%02x:%02x:%02x:%02x:%02x \t(0x%010llx)\n",
			(unsigned int)((sourcemac>>40)&0xFF),
			(unsigned int)((sourcemac>>32)&0xFF),
			(unsigned int)((sourcemac>>24)&0xFF),
			(unsigned int)((sourcemac>>16)&0xFF),
			(unsigned int)((sourcemac>>8)&0xFF),
			(unsigned int)((sourcemac>>0)&0xFF),
			(long  long unsigned int)sourcemac);
	printf("Source Port : %d \t\t\t(0x%08x)\n",sourceport, sourceport);

	printf("Dest. IP    : %d.%d.%d.%d \t\t(0x%08x)\n",(destip>>24)&0xff,(destip>>16)&0xff,(destip>>8)&0xff,(destip)&0xff, destip);
	printf("Dest. MAC   : %02x:%02x:%02x:%02x:%02x:%02x \t(0x%010llx)\n",
			(unsigned int)((destmac>>40)&0xFF),
			(unsigned int)((destmac>>32)&0xFF),
			(unsigned int)((destmac>>24)&0xFF),
			(unsigned int)((destmac>>16)&0xFF),
			(unsigned int)((destmac>>8)&0xFF),
			(unsigned int)((destmac>>0)&0xFF),
			(long  long unsigned int)destmac);
	printf("Dest. Port  : %d \t\t\t(0x%08x)\n",udpport, udpport);

	long int checksum=calcChecksum(sourceip, destip);
	bus_w(TX_IP_REG, sourceip);
	bus_w(RX_IP_REG, destip);

/*
	bus_w(TX_MAC_LSB_REG,(destmac>>32)&0xFFFFFFFF);//rx_udpmacH_AReg_c
	bus_w(TX_MAC_MSB_REG,(destmac)&0xFFFFFFFF);//rx_udpmacL_AReg_c
	bus_w(RX_MAC_MSB_REG,(sourcemac>>32)&0xFFFFFFFF);//detectormacH_AReg_c
	bus_w(RX_MAC_LSB_REG,(sourcemac)&0xFFFFFFFF);//detectormacL_AReg_c
	bus_w(UDP_PORT_REG,((sourceport&0xFFFF)<<16)+(udpport&0xFFFF));//udpports_AReg_c
*/
	uint32_t val = 0;

	val = ((sourcemac >> LSB_OF_64_BIT_REG_OFST) & BIT_32_MSK);
	bus_w(TX_MAC_LSB_REG, val);
#ifdef VERBOSE
	printf("Read from TX_MAC_LSB_REG: 0x%08x\n", bus_r(TX_MAC_LSB_REG));
#endif

	val = ((sourcemac >> MSB_OF_64_BIT_REG_OFST) & BIT_32_MSK);
	bus_w(TX_MAC_MSB_REG,val);
#ifdef VERBOSE
	printf("Read from TX_MAC_MSB_REG: 0x%08x\n", bus_r(TX_MAC_MSB_REG));
#endif

	val = ((destmac >> LSB_OF_64_BIT_REG_OFST) & BIT_32_MSK);
	bus_w(RX_MAC_LSB_REG, val);
#ifdef VERBOSE
	printf("Read from RX_MAC_LSB_REG: 0x%08x\n", bus_r(RX_MAC_LSB_REG));
#endif

	val = ((destmac >> MSB_OF_64_BIT_REG_OFST) & BIT_32_MSK);
	bus_w(RX_MAC_MSB_REG, val);
#ifdef VERBOSE
	printf("Read from RX_MAC_MSB_REG: 0x%08x\n", bus_r(RX_MAC_MSB_REG));
#endif

	val = (((sourceport << UDP_PORT_TX_OFST) & UDP_PORT_TX_MSK) |
			((udpport << UDP_PORT_RX_OFST) & UDP_PORT_RX_MSK));
	bus_w(UDP_PORT_REG, val);
#ifdef VERBOSE
	printf("Read from UDP_PORT_REG: 0x%08x\n", bus_r(UDP_PORT_REG));
#endif

	bus_w(TX_IP_CHECKSUM_REG,(checksum << TX_IP_CHECKSUM_OFST) & TX_IP_CHECKSUM_MSK);
#ifdef VERBOSE
	printf("Read from TX_IP_CHECKSUM_REG: 0x%08x\n", bus_r(TX_IP_CHECKSUM_REG));
#endif
	cleanFifos();
	resetCore();

	usleep(500 * 1000); /* todo maybe without */
	return 0;
}


int setDetectorPosition(int pos[]) {
	int ret = OK;

	bus_w(COORD_0_REG, bus_r(COORD_0_REG) | ((pos[0] << COORD_0_X_OFST) & COORD_0_X_MSK));
	if ((bus_r(COORD_0_REG) &  COORD_0_X_MSK) != ((pos[0] << COORD_0_X_OFST) & COORD_0_X_MSK))
		ret = FAIL;

	bus_w(COORD_0_REG, bus_r(COORD_0_REG) | ((pos[1] << COORD_0_Y_OFST) & COORD_0_Y_MSK));
	if ((bus_r(COORD_0_REG) &  COORD_0_Y_MSK) != ((pos[1] << COORD_0_Y_OFST) & COORD_0_Y_MSK))
		ret = FAIL;

	bus_w(COORD_1_REG, bus_r(COORD_1_REG) | ((pos[2] << COORD_0_Z_OFST) & COORD_0_Z_MSK));
	if ((bus_r(COORD_1_REG) &  COORD_0_Z_MSK) != ((pos[2] << COORD_0_Z_OFST) & COORD_0_Z_MSK))
		ret = FAIL;

	if (ret == OK)
		printf("Position set to [%d, %d, %d]\n", pos[0], pos[1], pos[2]);
	return ret;
}



/* jungfrau specific - pll, flashing fpga */



void resetPLL() {
#ifdef VIRTUAL
    return;
#endif
	// reset PLL Reconfiguration and PLL
	bus_w(PLL_CONTROL_REG, bus_r(PLL_CONTROL_REG) | PLL_CTRL_RECONFIG_RST_MSK | PLL_CTRL_RST_MSK);
	usleep(100);
	bus_w(PLL_CONTROL_REG, bus_r(PLL_CONTROL_REG) & ~PLL_CTRL_RECONFIG_RST_MSK & ~PLL_CTRL_RST_MSK);
}


u_int32_t setPllReconfigReg(u_int32_t reg, u_int32_t val) {
#ifdef VIRTUAL
    return val;
#endif
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
#ifdef VIRTUAL
    return;
#endif
	u_int32_t val;
	int32_t phase=0, inv=0;
	// ensuring PLL is never configured with same phase
    if (clkPhase[1] == 0) {
        return;
    }

	printf("Configuring PLL with phase in %d\n", clkPhase[1]);
	if (clkPhase[1]>0) {
		inv=0;
		phase=clkPhase[1];
	}  else {
		inv=1;
		phase=-1*clkPhase[1];
	}
	printf(" phase out %d (0x%08x)\n", phase, phase);

	if (inv) {
		val = ((phase << PLL_SHIFT_NUM_SHIFTS_OFST) & PLL_SHIFT_NUM_SHIFTS_MSK) + PLL_SHIFT_CNT_SLCT_C1_VAL + PLL_SHIFT_UP_DOWN_NEG_VAL;
		printf(" phase word 0x%08x\n", val);
		setPllReconfigReg(PLL_PHASE_SHIFT_REG, val);
	} else {
		val = ((phase << PLL_SHIFT_NUM_SHIFTS_OFST) & PLL_SHIFT_NUM_SHIFTS_MSK) + PLL_SHIFT_CNT_SLCT_C0_VAL + PLL_SHIFT_UP_DOWN_NEG_VAL;
		printf(" phase word 0x%08x\n", val);
		setPllReconfigReg(PLL_PHASE_SHIFT_REG, val);

		printf(" phase word 0x%08x\n", val);
		val = ((phase << PLL_SHIFT_NUM_SHIFTS_OFST) & PLL_SHIFT_NUM_SHIFTS_MSK) + PLL_SHIFT_CNT_SLCT_C2_VAL;
		setPllReconfigReg(PLL_PHASE_SHIFT_REG, val);
	}
	usleep(10000);
}



int setThresholdTemperature(int val) {

    if (val >= 0) {
        printf("\nThreshold Temperature: %d\n", val);

        val *= (10.0/625.0);
#ifdef VERBOSE
        printf("Converted Threshold Temperature: %d\n", val);
#endif
        bus_w(TEMP_CTRL_REG, (bus_r(TEMP_CTRL_REG) &~(TEMP_CTRL_PROTCT_THRSHLD_MSK) &~(TEMP_CTRL_OVR_TMP_EVNT_MSK))
                | (((val  << TEMP_CTRL_PROTCT_THRSHLD_OFST) & TEMP_CTRL_PROTCT_THRSHLD_MSK)));
#ifdef VERBOSE
        printf("Converted Threshold Temperature set to %d\n", ((bus_r(TEMP_CTRL_REG) & TEMP_CTRL_PROTCT_THRSHLD_MSK) >> TEMP_CTRL_PROTCT_THRSHLD_OFST));
#endif
    }
    uint32_t temp = ((bus_r(TEMP_CTRL_REG) & TEMP_CTRL_PROTCT_THRSHLD_MSK) >> TEMP_CTRL_PROTCT_THRSHLD_OFST);

    // conversion
    temp *= (625.0/10.0);
    printf("Threshold Temperature  %f °C\n",(double)temp/1000.00);

    return temp;

}


int setTemperatureControl(int val) {
    if (val >= 0) {
        // binary value
        if (val > 0 ) val = 1;
        printf("\nTemperature control: %d\n", val);
        bus_w(TEMP_CTRL_REG, (bus_r(TEMP_CTRL_REG)  &~(TEMP_CTRL_PROTCT_ENABLE_MSK) &~(TEMP_CTRL_OVR_TMP_EVNT_MSK))
                | (((val  << TEMP_CTRL_PROTCT_ENABLE_OFST) & TEMP_CTRL_PROTCT_ENABLE_MSK)));
#ifdef VERBOSE
        printf("Temperature control set to %d\n", ((bus_r(TEMP_CTRL_REG) & TEMP_CTRL_PROTCT_ENABLE_MSK) >> TEMP_CTRL_PROTCT_ENABLE_OFST));
#endif
    }
    return ((bus_r(TEMP_CTRL_REG) & TEMP_CTRL_PROTCT_ENABLE_MSK) >> TEMP_CTRL_PROTCT_ENABLE_OFST);
}


int setTemperatureEvent(int val) {
#ifdef VIRTUAL
    return 0;
#endif
    if (val >= 0) {
        // set bit to clear it
        val = 1;
        printf("\nTemperature Event: %d\n", val);
        bus_w(TEMP_CTRL_REG, (bus_r(TEMP_CTRL_REG)   &~TEMP_CTRL_OVR_TMP_EVNT_MSK)
                | (((val  << TEMP_CTRL_OVR_TMP_EVNT_OFST) & TEMP_CTRL_OVR_TMP_EVNT_MSK)));
#ifdef VERBOSE
        printf("Temperature Event set to %d\n", ((bus_r(TEMP_CTRL_REG) & TEMP_CTRL_OVR_TMP_EVNT_MSK) >> TEMP_CTRL_OVR_TMP_EVNT_OFST));
#endif
    }
    return ((bus_r(TEMP_CTRL_REG) & TEMP_CTRL_OVR_TMP_EVNT_MSK) >> TEMP_CTRL_OVR_TMP_EVNT_OFST);
}



int setNetworkParameter(enum NETWORKINDEX mode, int value) {
    if (mode != TXN_FRAME)
        return -1;

    if (value >= 0) {
        printf("\nSetting transmission delay: %d\n", value);
        bus_w(CONFIG_REG, (bus_r(CONFIG_REG) &~CONFIG_TDMA_TIMESLOT_MSK)
                | (((value  << CONFIG_TDMA_TIMESLOT_OFST) & CONFIG_TDMA_TIMESLOT_MSK)));
        if (value == 0)
            bus_w(CONFIG_REG, bus_r(CONFIG_REG) &~ CONFIG_TDMA_MSK);
        else
            bus_w(CONFIG_REG, bus_r(CONFIG_REG) | CONFIG_TDMA_MSK);
#ifdef VERBOSE
        printf("Transmission delay set to %d\n", ((bus_r(CONFIG_REG) & CONFIG_TDMA_TIMESLOT_MSK) >> CONFIG_TDMA_TIMESLOT_OFST));
#endif
    }

    return ((bus_r(CONFIG_REG) & CONFIG_TDMA_TIMESLOT_MSK) >> CONFIG_TDMA_TIMESLOT_OFST);
}





/* aquisition */

int startStateMachine(){
#ifdef VIRTUAL
	virtual_status = 1;
	virtual_stop = 0;
	if(pthread_create(&pthread_virtual_tid, NULL, &start_timer, NULL)) {
		virtual_status = 0;
		cprintf(RED,"Could not start Virtual acquisition thread\n");
		return FAIL;
	}
	cprintf(GREEN,"***Virtual Acquisition started\n");
	return OK;
#endif
	printf("*******Starting State Machine*******\n");

	cleanFifos();

	//start state machine
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_START_ACQ_MSK);
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_START_ACQ_MSK);

	printf("Status Register: %08x\n",bus_r(STATUS_REG));
	return OK;
}


#ifdef VIRTUAL
void* start_timer(void* arg) {
	int wait_in_s = 	(setTimer(FRAME_NUMBER, -1) *
						setTimer(CYCLES_NUMBER, -1) *
						(setTimer(STORAGE_CELL_NUMBER, -1) + 1) *
						(setTimer(FRAME_PERIOD, -1)/(1E9)));
	cprintf(GREEN,"going to wait for %d s\n", wait_in_s);
	while(!virtual_stop && (wait_in_s >= 0)) {
		usleep(1000 * 1000);
		wait_in_s--;
	}
	cprintf(GREEN,"Virtual Timer Done***\n");

	virtual_status = 0;
	return NULL;
}
#endif

int stopStateMachine(){
	cprintf(BG_RED,"*******Stopping State Machine*******\n");
#ifdef VIRTUAL
	virtual_stop = 0;
	return OK;
#endif
	//stop state machine
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_STOP_ACQ_MSK);
	usleep(100);
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_STOP_ACQ_MSK);

	printf("Status Register: %08x\n",bus_r(STATUS_REG));
	return OK;
}





enum runStatus getRunStatus(){
#ifdef VIRTUAL
	if(virtual_status == 0){
		printf("Status: IDLE\n");
		return IDLE;
	}else{
		printf("Status: RUNNING...\n");
		return RUNNING;
	}
#endif
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
	    // stopped or error
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



void readFrame(int *ret, char *mess){
#ifdef VIRTUAL
	while(virtual_status) {
		//cprintf(RED,"Waiting for finished flag\n");
		usleep(5000);
	}
	*ret = (int)FINISHED;
	strcpy(mess,"acquisition successfully finished\n");
	return;
#endif
	// wait for status to be done
	while(runBusy()){
		usleep(500);
	}

	// frames left to give status
	int64_t retval = getTimeLeft(FRAME_NUMBER) + 1;
	if ( retval > 0) {
		*ret = (int)FAIL;
		sprintf(mess,"no data and run stopped: %lld frames left\n",(long  long int)retval);
		cprintf(RED,"%s\n",mess);
	} else {
		*ret = (int)FINISHED;
		sprintf(mess,"acquisition successfully finished\n");
		printf("%s",mess);
	}
}



u_int32_t runBusy(void) {
#ifdef VIRTUAL
    return virtual_status;
#endif
	u_int32_t s = ((bus_r(STATUS_REG) & RUN_BUSY_MSK) >> RUN_BUSY_OFST);
#ifdef VERBOSE
	printf("Status Register: %08x\n", s);
#endif
	return s;
}








/* common */

//jungfrau doesnt require chips and chans (save memory)
int copyModule(sls_detector_module *destMod, sls_detector_module *srcMod){

	int idac, iadc;
	int ret=OK;

#ifdef VERBOSE
	printf("Copying module %x to module %x\n",srcMod,destMod);
#endif

	if (srcMod->module>=0) {
#ifdef VERBOSE
		printf("Copying module number %d to module number %d\n",srcMod->module,destMod->module);
#endif
		destMod->module=srcMod->module;
	}
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


int calculateDataBytes(){
	return DATA_BYTES;
}

int getTotalNumberOfChannels(){return ((int)getNumberOfChannelsPerModule() * (int)getTotalNumberOfModules());}
int getTotalNumberOfChips(){return ((int)getNumberOfChipsPerModule() * (int)getTotalNumberOfModules());}
int getTotalNumberOfModules(){return NMOD;}
int getNumberOfChannelsPerModule(){return  ((int)getNumberOfChannelsPerChip() * (int)getTotalNumberOfChips());}
int getNumberOfChipsPerModule(){return  NCHIP;}
int getNumberOfDACsPerModule(){return  NDAC;}
int getNumberOfADCsPerModule(){return  NADC;}
int getNumberOfChannelsPerChip(){return  NCHAN;}



/* sync */

enum masterFlags setMaster(enum masterFlags arg){
	return NO_MASTER;
}

enum synchronizationMode setSynchronization(enum synchronizationMode arg){
	return NO_SYNCHRONIZATION;
}






//#endif
