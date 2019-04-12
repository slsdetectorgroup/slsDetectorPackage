#include "slsDetectorFunctionList.h"
#include "versionAPI.h"
#include "logger.h"

#include "AD9257.h"		// commonServerFunctions.h, blackfin.h, ansi.h
#include "LTC2620.h"    // dacs
#include "MAX1932.h"    // hv
#include "ALTERA_PLL.h" // pll
#ifndef VIRTUAL
#include "programfpga.h"
#else
#include "blackfin.h"
#include <string.h>
#include <unistd.h>     // usleep
#include <pthread.h>
#include <time.h>
#endif

// Global variable from slsDetectorServer_funcs
extern int debugflag;

int firmware_compatibility = OK;
int firmware_check_done = 0;
char firmware_message[MAX_STR_LENGTH];

#ifdef VIRTUAL
pthread_t pthread_virtual_tid;
int virtual_status = 0;
int virtual_stop = 0;
#endif

enum detectorSettings thisSettings = UNINITIALIZED;
int highvoltage = 0;
int dacValues[NDAC] = {0};
int adcPhase = 0;


int isFirmwareCheckDone() {
	return firmware_check_done;
}

int getFirmwareCheckResult(char** mess) {
	*mess = firmware_message;
	return firmware_compatibility;
}

void basictests() {
    firmware_compatibility = OK;
    firmware_check_done = 0;
    memset(firmware_message, 0, MAX_STR_LENGTH);
#ifdef VIRTUAL
    FILE_LOG(logINFOBLUE, ("******** Jungfrau Virtual Server *****************\n"));
    if (mapCSP0() == FAIL) {
    	strcpy(firmware_message,
				"Could not map to memory. Dangerous to continue.\n");
		FILE_LOG(logERROR, (firmware_message));
		firmware_compatibility = FAIL;
		firmware_check_done = 1;
		return;
    }
    firmware_check_done = 1;
    return;
#else

	defineGPIOpins();
	resetFPGA();
    if (mapCSP0() == FAIL) {
    	strcpy(firmware_message,
				"Could not map to memory. Dangerous to continue.\n");
		FILE_LOG(logERROR, ("%s\n\n", firmware_message));
		firmware_compatibility = FAIL;
		firmware_check_done = 1;
		return;
    }

    // does check only if flag is 0 (by default), set by command line
	if ((!debugflag) && ((checkType() == FAIL) || (testFpga() == FAIL) || (testBus() == FAIL))) {
		strcpy(firmware_message,
				"Could not pass basic tests of FPGA and bus. Dangerous to continue.\n");
		FILE_LOG(logERROR, ("%s\n\n", firmware_message));
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
	FILE_LOG(logINFOBLUE, ("************ Jungfrau Server *********************\n"
			"Hardware Version:\t\t 0x%x\n"
			"Hardware Serial Nr:\t\t 0x%x\n"

			"Detector IP Addr:\t\t 0x%x\n"
			"Detector MAC Addr:\t\t 0x%llx\n\n"

			"Firmware Version:\t\t 0x%llx\n"
			"Software Version:\t\t 0x%llx\n"
			"F/w-S/w API Version:\t\t 0x%llx\n"
			"Required Firmware Version:\t 0x%x\n"
			"Client-Software API Version:\t 0x%llx\n"
			"********************************************************\n",
			hversion, hsnumber,
			ipadd,
			(long  long unsigned int)macadd,
			(long  long int)fwversion,
			(long  long int)swversion,
			(long  long int)sw_fw_apiversion,
			REQRD_FRMWR_VRSN,
			(long long int)client_sw_apiversion
	));

	// return if flag is not zero, debug mode
	if (debugflag) {
		firmware_check_done = 1;
		return;
	}


	//cant read versions
    FILE_LOG(logINFO, ("Testing Firmware-software compatibility:\n"));
	if(!fwversion || !sw_fw_apiversion){
		strcpy(firmware_message,
				"Cant read versions from FPGA. Please update firmware.\n");
		FILE_LOG(logERROR, (firmware_message));
		firmware_compatibility = FAIL;
		firmware_check_done = 1;
		return;
	}

	//check for API compatibility - old server
	if(sw_fw_apiversion > REQRD_FRMWR_VRSN){
		sprintf(firmware_message,
				"This detector software software version (0x%llx) is incompatible.\n"
				"Please update detector software (min. 0x%llx) to be compatible with this firmware.\n",
				(long long int)sw_fw_apiversion,
				(long long int)REQRD_FRMWR_VRSN);
		FILE_LOG(logERROR, (firmware_message));
		firmware_compatibility = FAIL;
		firmware_check_done = 1;
		return;
	}

	//check for firmware compatibility - old firmware
	if( REQRD_FRMWR_VRSN > fwversion) {
		sprintf(firmware_message,
				"This firmware version (0x%llx) is incompatible.\n"
				"Please update firmware (min. 0x%llx) to be compatible with this server.\n",
				(long long int)fwversion,
				(long long int)REQRD_FRMWR_VRSN);
		FILE_LOG(logERROR, (firmware_message));
		firmware_compatibility = FAIL;
		firmware_check_done = 1;
		return;
	}
	FILE_LOG(logINFO, ("Compatibility - success\n"));
	firmware_check_done = 1;
#endif
}


int checkType() {
#ifdef VIRTUAL
    return OK;
#endif
	volatile u_int32_t type = ((bus_r(FPGA_VERSION_REG) & DETECTOR_TYPE_MSK) >> DETECTOR_TYPE_OFST);
	if (type != BOARD_JUNGFRAU_TYPE){
			FILE_LOG(logERROR, ("This is not a Jungfrau Server (read %d, expected %d)\n", type, BOARD_JUNGFRAU_TYPE));
			return FAIL;
		}

	return OK;
}



int testFpga() {
#ifdef VIRTUAL
    return OK;
#endif
	FILE_LOG(logINFO, ("Testing FPGA:\n"));

	//fixed pattern
	int ret = OK;
	volatile u_int32_t val = bus_r(FIX_PATT_REG);
	if (val == FIX_PATT_VAL) {
		FILE_LOG(logINFO, ("Fixed pattern: successful match 0x%08x\n",val));
	} else {
		FILE_LOG(logERROR, ("Fixed pattern does not match! Read 0x%08x, expected 0x%08x\n", val, FIX_PATT_VAL));
		ret = FAIL;
	}
	return ret;
}


int testBus() {
#ifdef VIRTUAL
    return OK;
#endif
	FILE_LOG(logINFO, ("Testing Bus:\n"));

	int ret = OK;
	u_int32_t addr = SET_TRIGGER_DELAY_LSB_REG;
	int times = 1000 * 1000;
	int i = 0;

	for (i = 0; i < times; ++i) {
		bus_w(addr, i * 100);
		if (i * 100 != bus_r(addr)) {
			FILE_LOG(logERROR, ("Mismatch! Wrote 0x%x, read 0x%x\n",
					i * 100, bus_r(addr)));
			ret = FAIL;
		}
	}

	bus_w(addr, 0);

	if (ret == OK) {
		FILE_LOG(logINFO, ("Successfully tested bus %d times\n", times));
	}
	return ret;
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
		FILE_LOG(logERROR, ("Test %s not implemented for this detector\n", (int)arg));
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
	//FILE_LOG(logINFO, ("ip:%x\n",res);

	return res;
}








/* initialization */

void initControlServer(){
	setupDetector();
}



void initStopServer() {

	usleep(CTRL_SRVR_INIT_TIME_US);
	if (mapCSP0() == FAIL) {
		FILE_LOG(logERROR, ("Stop Server: Map Fail. Dangerous to continue. Goodbye!\n"));
		exit(EXIT_FAILURE);
	}
}






/* set up detector */




void setupDetector() {
    FILE_LOG(logINFO, ("This Server is for 1 Jungfrau module (500k)\n"));

    adcPhase = 0;
    ALTERA_PLL_ResetPLL();
	resetCore();
	resetPeripheral();
	cleanFifos();

	// hv
    MAX1932_SetDefines(SPI_REG, SPI_HV_SRL_CS_OTPT_MSK, SPI_HV_SRL_CLK_OTPT_MSK, SPI_HV_SRL_DGTL_OTPT_MSK, SPI_HV_SRL_DGTL_OTPT_OFST, HIGHVOLTAGE_MIN, HIGHVOLTAGE_MAX);
    MAX1932_Disable();
    setHighVoltage(DEFAULT_HIGH_VOLTAGE);

	// adc
    AD9257_SetDefines(ADC_SPI_REG, ADC_SPI_SRL_CS_OTPT_MSK, ADC_SPI_SRL_CLK_OTPT_MSK, ADC_SPI_SRL_DT_OTPT_MSK, ADC_SPI_SRL_DT_OTPT_OFST);
    AD9257_Disable();
    AD9257_Configure();

    //dac
    LTC2620_SetDefines(SPI_REG, SPI_DAC_SRL_CS_OTPT_MSK, SPI_DAC_SRL_CLK_OTPT_MSK, SPI_DAC_SRL_DGTL_OTPT_MSK, SPI_DAC_SRL_DGTL_OTPT_OFST, NDAC, DAC_MIN_MV, DAC_MAX_MV);
    LTC2620_Disable();
    LTC2620_Configure();
	setDefaultDacs();

    // altera pll
    ALTERA_PLL_SetDefines(PLL_CNTRL_REG, PLL_PARAM_REG, PLL_CNTRL_RCNFG_PRMTR_RST_MSK, PLL_CNTRL_WR_PRMTR_MSK, PLL_CNTRL_PLL_RST_MSK, PLL_CNTRL_ADDR_MSK, PLL_CNTRL_ADDR_OFST);

	bus_w(DAQ_REG, 0x0);         /* Only once at server startup */

	FILE_LOG(logINFOBLUE, ("Setting Default parameters\n"));
	setClockDivider(HALF_SPEED);
	cleanFifos();
	resetCore();

	alignDeserializer();


	configureASICTimer();
	bus_w(ADC_PORT_INVERT_REG, ADC_PORT_INVERT_VAL);

	//Initialization of acquistion parameters
	setSettings(DEFAULT_SETTINGS);

	setTimer(FRAME_NUMBER, DEFAULT_NUM_FRAMES);
	setTimer(CYCLES_NUMBER, DEFAULT_NUM_CYCLES);
	setTimer(ACQUISITION_TIME, DEFAULT_EXPTIME);
	setTimer(FRAME_PERIOD, DEFAULT_PERIOD);
	setTimer(DELAY_AFTER_TRIGGER, DEFAULT_DELAY);
	setTimer(STORAGE_CELL_NUMBER, DEFAULT_NUM_STRG_CLLS);
	setTimer(STORAGE_CELL_DELAY, DEFAULT_STRG_CLL_DLY);
	selectStoragecellStart(DEFAULT_STRG_CLL_STRT);
	/*setClockDivider(HALF_SPEED); depends if all the previous stuff works*/
	setTiming(DEFAULT_TIMING_MODE);


	// temp threshold and reset event
	setThresholdTemperature(DEFAULT_TMP_THRSHLD);
	setTemperatureEvent(0);
}


int setDefaultDacs() {
	int ret = OK;
	FILE_LOG(logINFOBLUE, ("Setting Default Dac values\n"));
	{
		int i = 0;
		const int defaultvals[NDAC] = DEFAULT_DAC_VALS;
		for(i = 0; i < NDAC; ++i) {
			// if not already default, set it to default
			if (dacValues[i] != defaultvals[i]) {
				setDAC((enum DACINDEX)i,defaultvals[i],0);
			}
		}
	}
	return ret;
}




/* firmware functions (resets) */


void cleanFifos() {
#ifdef VIRTUAL
    return;
#endif
	FILE_LOG(logINFO, ("Clearing Acquisition Fifos\n"));
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_ACQ_FIFO_CLR_MSK);
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_ACQ_FIFO_CLR_MSK);
}

void resetCore() {
#ifdef VIRTUAL
    return;
#endif
	FILE_LOG(logINFO, ("Resetting Core\n"));
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_CORE_RST_MSK);
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_CORE_RST_MSK);
	usleep(1000 * 1000);
}

void resetPeripheral() {
#ifdef VIRTUAL
    return;
#endif
	FILE_LOG(logINFO, ("Resetting Peripheral\n"));
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_PERIPHERAL_RST_MSK);
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_PERIPHERAL_RST_MSK);
}





/* set parameters -  dr, roi */



int setDynamicRange(int dr){
	return DYNAMIC_RANGE;
}




/* parameters - speed, readout */

void setSpeed(enum speedVariable ind, int val, int mode) {
    switch(ind) {
    case CLOCK_DIVIDER:
        setClockDivider(val);
    case ADC_PHASE:
        setAdcPhase(val, mode);
    default:
        return;
    }
}

int getSpeed(enum speedVariable ind, int mode) {
    switch(ind) {
    case CLOCK_DIVIDER:
        return getClockDivider();
    case ADC_PHASE:
        return getPhase(mode);
    case MAX_ADC_PHASE_SHIFT:
    	return getMaxPhaseShift();
    default:
        return -1;
    }
}




/* parameters - timer */
int selectStoragecellStart(int pos) {
    if (pos >= 0) {
        FILE_LOG(logINFO, ("Setting storage cell start: %d\n", pos));
        bus_w(DAQ_REG, bus_r(DAQ_REG) & ~DAQ_STRG_CELL_SLCT_MSK);
        bus_w(DAQ_REG, bus_r(DAQ_REG) | ((pos << DAQ_STRG_CELL_SLCT_OFST) & DAQ_STRG_CELL_SLCT_MSK));
    }
    return ((bus_r(DAQ_REG) & DAQ_STRG_CELL_SLCT_MSK) >> DAQ_STRG_CELL_SLCT_OFST);
}



int64_t setTimer(enum timerIndex ind, int64_t val) {

	int64_t retval = -1;
	switch(ind){

	case FRAME_NUMBER:
		if(val >= 0) {
			FILE_LOG(logINFO, ("Setting #frames: %lld\n",(long long int)val));
		}
		retval = set64BitReg(val,  SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG);
		FILE_LOG(logDEBUG1, ("Getting #frames: %lld\n", (long long int)retval));
		break;

	case ACQUISITION_TIME:
		if(val >= 0){
			FILE_LOG(logINFO, ("Setting exptime: %lldns\n", (long long int)val));
			val *= (1E-3 * CLK_RUN);
			val -= ACQ_TIME_MIN_CLOCK;
			if(val < 0) val = 0;
		}
		retval = (set64BitReg(val, SET_EXPTIME_LSB_REG, SET_EXPTIME_MSB_REG) + ACQ_TIME_MIN_CLOCK) / (1E-3 * CLK_RUN);
		FILE_LOG(logDEBUG1, ("Getting exptime: %lldns\n", (long long int)retval));
		break;

	case FRAME_PERIOD:
		if(val >= 0){
			FILE_LOG(logINFO, ("Setting period: %lldns\n",(long long int)val));
			val *= (1E-3 * CLK_SYNC);
		}
		retval = set64BitReg(val, SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG )/ (1E-3 * CLK_SYNC);
		FILE_LOG(logDEBUG1, ("Getting period: %lldns\n", (long long int)retval));
		break;

	case DELAY_AFTER_TRIGGER:
		if(val >= 0){
			FILE_LOG(logINFO, ("Setting delay: %lldns\n", (long long int)val));
			val *= (1E-3 * CLK_SYNC);
		}
		retval = set64BitReg(val, SET_TRIGGER_DELAY_LSB_REG, SET_TRIGGER_DELAY_MSB_REG) / (1E-3 * CLK_SYNC);
		FILE_LOG(logDEBUG1, ("Getting delay: %lldns\n", (long long int)retval));
		break;

	case CYCLES_NUMBER:
		if(val >= 0) {
			FILE_LOG(logINFO, ("Setting #cycles: %lld\n", (long long int)val));
		}
		retval = set64BitReg(val,  SET_CYCLES_LSB_REG, SET_CYCLES_MSB_REG);
		FILE_LOG(logDEBUG1, ("Getting #cycles: %lld\n", (long long int)retval));
		break;

	case STORAGE_CELL_NUMBER:
        if(val >= 0) {
            FILE_LOG(logINFO, ("Setting #storage cells: %lld\n", (long long int)val));
            bus_w(CONTROL_REG, (bus_r(CONTROL_REG) & ~CONTROL_STORAGE_CELL_NUM_MSK) |
                    ((val << CONTROL_STORAGE_CELL_NUM_OFST) & CONTROL_STORAGE_CELL_NUM_MSK));
        }
        retval = ((bus_r(CONTROL_REG) & CONTROL_STORAGE_CELL_NUM_MSK) >> CONTROL_STORAGE_CELL_NUM_OFST);
        FILE_LOG(logDEBUG1, ("Getting #storage cells: %lld\n", (long long int)retval));
        break;

	case STORAGE_CELL_DELAY:
		if(val >= 0){
			FILE_LOG(logINFO, ("Setting storage cell delay: %lldns\n", (long long int)val));
			val *= (1E-3 * CLK_RUN);
            bus_w(ASIC_CTRL_REG, (bus_r(ASIC_CTRL_REG) & ~ASIC_CTRL_EXPSRE_TMR_MSK) |
                    ((val << ASIC_CTRL_EXPSRE_TMR_OFST) & ASIC_CTRL_EXPSRE_TMR_MSK));
		}

		retval = ((bus_r(ASIC_CTRL_REG) & ASIC_CTRL_EXPSRE_TMR_MSK) >> ASIC_CTRL_EXPSRE_TMR_OFST);
		FILE_LOG(logDEBUG1, ("Getting storage cell delay: %lldns\n", (long long int)retval));
		break;

	default:
		FILE_LOG(logERROR, ("Timer Index not implemented for this detector: %d\n", ind));
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
		FILE_LOG(logINFO, ("Getting number of frames left: %lld\n",(long long int)retval));
		break;

	case FRAME_PERIOD:
		retval = get64BitReg(GET_PERIOD_LSB_REG, GET_PERIOD_MSB_REG) / (1E-3 * CLK_SYNC);
		FILE_LOG(logINFO, ("Getting period left: %lldns\n", (long long int)retval));
		break;
/*
	case DELAY_AFTER_TRIGGER:
		retval = get64BitReg(xxx) / (1E-3 * CLK_SYNC);
		FILE_LOG(logINFO, ("Getting delay left: %lldns\n", (long long int)retval));
		break;
*/
	case CYCLES_NUMBER:
		retval = get64BitReg(GET_CYCLES_LSB_REG, GET_CYCLES_MSB_REG);
		FILE_LOG(logINFO, ("Getting number of cycles left: %lld\n", (long long int)retval));
		break;

	case ACTUAL_TIME:
		retval = get64BitReg(TIME_FROM_START_LSB_REG, TIME_FROM_START_MSB_REG) / (1E-3 * CLK_SYNC);
		FILE_LOG(logINFO, ("Getting actual time (time from start): %lld\n", (long long int)retval));
		break;

	case MEASUREMENT_TIME:
		retval = get64BitReg(START_FRAME_TIME_LSB_REG, START_FRAME_TIME_MSB_REG) / (1E-3 * CLK_SYNC);
		FILE_LOG(logINFO, ("Getting measurement time (timestamp/ start frame time): %lld\n", (long long int)retval));
		break;

	case FRAMES_FROM_START:
	case FRAMES_FROM_START_PG:
		retval = get64BitReg(FRAMES_FROM_START_PG_LSB_REG, FRAMES_FROM_START_PG_MSB_REG);
		FILE_LOG(logINFO, ("Getting frames from start run control %lld\n", (long long int)retval));
		break;

	default:
		FILE_LOG(logERROR, ("Remaining Timer index not implemented for this detector: %d\n", ind));
		break;
	}

	return retval;
}


int validateTimer(enum timerIndex ind, int64_t val, int64_t retval) {
    if (val < 0)
        return OK;
    switch(ind) {
    case ACQUISITION_TIME:
        // convert to freq
        val *= (1E-3 * CLK_RUN);
        val -= ACQ_TIME_MIN_CLOCK;
        if(val < 0) val = 0;
        // convert back to timer
        val = (val + ACQ_TIME_MIN_CLOCK) / (1E-3 * CLK_RUN);
        if (val != retval)
            return FAIL;
        break;
    case STORAGE_CELL_DELAY:
        // convert to freq
        val *= (1E-3 * CLK_RUN);
        if(val < 0) val = 0;
        // convert back to timer
        val = val / (1E-3 * CLK_RUN);
        if (val != retval)
            return FAIL;
        break;
    case FRAME_PERIOD:
    case DELAY_AFTER_TRIGGER:
        // convert to freq
        val *= (1E-3 * CLK_SYNC);
        // convert back to timer
        val = (val) / (1E-3 * CLK_SYNC);
        if (val != retval)
            return FAIL;
        break;
    default:
        break;
    }
    return OK;
}



/* parameters - channel, chip, module, settings */


int setModule(sls_detector_module myMod, char* mess){

    FILE_LOG(logINFO, ("Setting module with settings %d\n",myMod.reg));

    // settings
	setSettings( (enum detectorSettings)myMod.reg);

    //set dac values
	{
	    int i = 0;
	    for(i = 0; i < NDAC; ++i)
	        setDAC((enum DACINDEX)i, myMod.dacs[i], 0);
	}
	return OK;
}


int getModule(sls_detector_module *myMod){
    int idac = 0;
    for (idac = 0; idac < NDAC; ++idac) {
        if (dacValues[idac] >= 0)
            *((myMod->dacs) + idac) = dacValues[idac];
    }
    // check if all of them are not initialized
    int initialized = 0;
    for (idac = 0; idac < NDAC; ++idac) {
        if (dacValues[idac] >= 0)
            initialized = 1;
    }
    if (initialized)
        return OK;
	return FAIL;
}



enum detectorSettings setSettings(enum detectorSettings sett){
	if(sett == UNINITIALIZED)
		return thisSettings;

	// set settings
	if(sett != GET_SETTINGS) {
	    switch (sett) {
	    case DYNAMICGAIN:
	        bus_w(DAQ_REG, bus_r(DAQ_REG) & ~DAQ_SETTINGS_MSK);
            FILE_LOG(logINFO, ("Set settings - Dyanmic Gain, DAQ Reg: 0x%x\n", bus_r(DAQ_REG)));
	        break;
	    case DYNAMICHG0:
            bus_w(DAQ_REG, bus_r(DAQ_REG) & ~DAQ_SETTINGS_MSK);
            bus_w(DAQ_REG, bus_r(DAQ_REG) | DAQ_FIX_GAIN_HIGHGAIN_VAL);
            FILE_LOG(logINFO, ("Set settings - Dyanmic High Gain 0, DAQ Reg: 0x%x\n", bus_r(DAQ_REG)));
            break;
	    case FIXGAIN1:
            bus_w(DAQ_REG, bus_r(DAQ_REG) & ~DAQ_SETTINGS_MSK);
            bus_w(DAQ_REG, bus_r(DAQ_REG) | DAQ_FIX_GAIN_STG_1_VAL);
            FILE_LOG(logINFO, ("Set settings - Fix Gain 1, DAQ Reg: 0x%x\n", bus_r(DAQ_REG)));
            break;
	    case FIXGAIN2:
            bus_w(DAQ_REG, bus_r(DAQ_REG) & ~DAQ_SETTINGS_MSK);
            bus_w(DAQ_REG, bus_r(DAQ_REG) | DAQ_FIX_GAIN_STG_2_VAL);
            FILE_LOG(logINFO, ("Set settings - Fix Gain 2, DAQ Reg: 0x%x\n", bus_r(DAQ_REG)));
            break;
	    case FORCESWITCHG1:
            bus_w(DAQ_REG, bus_r(DAQ_REG) & ~DAQ_SETTINGS_MSK);
            bus_w(DAQ_REG, bus_r(DAQ_REG) | DAQ_FRCE_GAIN_STG_1_VAL);
            FILE_LOG(logINFO, ("Set settings - Force Switch Gain 1, DAQ Reg: 0x%x\n", bus_r(DAQ_REG)));
            break;
	    case FORCESWITCHG2:
            bus_w(DAQ_REG, bus_r(DAQ_REG) & ~DAQ_SETTINGS_MSK);
            bus_w(DAQ_REG, bus_r(DAQ_REG) | DAQ_FRCE_GAIN_STG_2_VAL);
            FILE_LOG(logINFO, ("Set settings - Force Switch Gain 2, DAQ Reg: 0x%x\n", bus_r(DAQ_REG)));
            break;
	    default:
	        FILE_LOG(logERROR, ("This settings is not defined for this detector %d\n", (int)sett));
	        return -1;
	    }

		thisSettings = sett;
	}

	return getSettings();

}


enum detectorSettings getSettings(){

	uint32_t regval = bus_r(DAQ_REG);
	uint32_t val = regval & DAQ_SETTINGS_MSK;
	FILE_LOG(logDEBUG1, ("Getting Settings\n Reading DAQ Register :0x%x\n", val));

	switch(val) {
	case DAQ_FIX_GAIN_DYNMC_VAL:
        thisSettings = DYNAMICGAIN;
        FILE_LOG(logDEBUG1, ("Settings read: Dynamic Gain. DAQ Reg: 0x%x\n", regval));
        break;
    case DAQ_FIX_GAIN_HIGHGAIN_VAL:
        thisSettings = DYNAMICHG0;
        FILE_LOG(logDEBUG1, ("Settings read: Dynamig High Gain. DAQ Reg: 0x%x\n", regval));
        break;
    case DAQ_FIX_GAIN_STG_1_VAL:
        thisSettings = FIXGAIN1;
        FILE_LOG(logDEBUG1, ("Settings read: Fix Gain 1. DAQ Reg: 0x%x\n", regval));
        break;
    case DAQ_FIX_GAIN_STG_2_VAL:
        thisSettings = FIXGAIN2;
        FILE_LOG(logDEBUG1, ("Settings read: Fix Gain 2. DAQ Reg: 0x%x\n", regval));
        break;
    case DAQ_FRCE_GAIN_STG_1_VAL:
        thisSettings = FORCESWITCHG1;
        FILE_LOG(logDEBUG1, ("Settings read: Force Switch Gain 1. DAQ Reg: 0x%x\n", regval));
        break;
    case DAQ_FRCE_GAIN_STG_2_VAL:
        thisSettings = FORCESWITCHG2;
        FILE_LOG(logDEBUG1, ("Settings read: Force Switch Gain 2. DAQ Reg: 0x%x\n", regval));
        break;
    default:
        thisSettings = UNDEFINED;
        FILE_LOG(logERROR, ("Settings read: Undefined. DAQ Reg: 0x%x\n", regval));
	}

	return thisSettings;
}





/* parameters - dac, adc, hv */
void setDAC(enum DACINDEX ind, int val, int mV) {
    if (val < 0)
        return;

    FILE_LOG(logDEBUG1, ("Setting dac[%d]: %d %s \n", (int)ind, val, (mV ? "mV" : "dac units")));
    int dacval = val;
#ifdef VIRTUAL
    if (!mV) {
        dacValues[ind] = val;
    }
    // convert to dac units
    else if (LTC2620_VoltageToDac(val, &dacval) == OK) {
        dacValues[ind] = dacval;
    }
#else
    if (LTC2620_SetDACValue((int)ind, val, mV, &dacval) == OK) {
        dacValues[ind] = dacval;
        if (ind == VREF_COMP && (val >= 0)) {//FIXME: if val == pwr down value, write 0?
            bus_w (VREF_COMP_MOD_REG, (bus_r(VREF_COMP_MOD_REG) &~ (VREF_COMP_MOD_MSK))   // reset
                    | ((val << VREF_COMP_MOD_OFST) & VREF_COMP_MOD_MSK));   // or it with value
        }
    }
#endif
}

int getDAC(enum DACINDEX ind, int mV) {
    if (!mV) {
        FILE_LOG(logDEBUG1, ("Getting DAC %d : %d dac\n",ind, dacValues[ind]));
        return dacValues[ind];
    }
    int voltage = -1;
    LTC2620_DacToVoltage(dacValues[ind], &voltage);
    FILE_LOG(logDEBUG1, ("Getting DAC %d : %d dac (%d mV)\n",ind, dacValues[ind], voltage));
    return voltage;
}

int getMaxDacSteps() {
    return LTC2620_MAX_STEPS;
}

int getADC(enum ADCINDEX ind){
#ifdef VIRTUAL
    return 0;
#endif
	char tempnames[2][40]={"VRs/FPGAs Temperature", "ADCs/ASICs Temperature"};
	FILE_LOG(logDEBUG1, ("Getting Temperature for %s\n", tempnames[ind]));
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
	FILE_LOG(logINFO, ("Temperature %s: %f °C\n",tempnames[ind],retval/1000.00));
	return retval;
}



int setHighVoltage(int val){
#ifdef VIRTUAL
    if (val >= 0)
        highvoltage = val;
    return highvoltage;
#endif

	// setting hv
	if (val >= 0) {
	    FILE_LOG(logINFO, ("Setting High voltage: %d V", val));
	    MAX1932_Set(val);
	    highvoltage = val;
	}
	return highvoltage;
}






/* parameters - timing, extsig */


void setTiming( enum externalCommunicationMode arg){

	if(arg != GET_EXTERNAL_COMMUNICATION_MODE){
		switch((int)arg){
		case AUTO_TIMING:
		    FILE_LOG(logINFO, ("Set Timing: Auto\n"));
		    bus_w(EXT_SIGNAL_REG, bus_r(EXT_SIGNAL_REG) & ~EXT_SIGNAL_MSK);
		    break;
		case TRIGGER_EXPOSURE:
		    FILE_LOG(logINFO, ("Set Timing: Trigger\n"));
		    bus_w(EXT_SIGNAL_REG, bus_r(EXT_SIGNAL_REG) | EXT_SIGNAL_MSK);
		    break;
		default:
			FILE_LOG(logERROR, ("Unknown timing mode %d\n", arg));
			return;
		}
	}
}


enum externalCommunicationMode getTiming() {
    if (bus_r(EXT_SIGNAL_REG) == EXT_SIGNAL_MSK)
        return TRIGGER_EXPOSURE;
    return AUTO_TIMING;
}



/* configure mac */


long int calcChecksum(int sourceip, int destip) {
	ip_header ip;
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

	int count = sizeof(ip);

	unsigned short *addr;
    addr = (unsigned short*) &(ip); /* warning: assignment from incompatible pointer type */

	long int sum = 0;
    while( count > 1 )  {
		sum += *addr++;
		count -= 2;
	}
	if (count > 0)
	    sum += *addr;                     // Add left-over byte, if any
	while (sum>>16)
	    sum = (sum & 0xffff) + (sum >> 16);// Fold 32-bit sum to 16 bits
	long int checksum = (~sum) & 0xffff;
	FILE_LOG(logINFO, ("IP checksum is 0x%lx\n",checksum));
	return checksum;
}



int configureMAC(int numInterfaces, int selInterface,
		uint32_t destip, uint64_t destmac, uint64_t sourcemac, uint32_t sourceip, uint32_t udpport,
		uint32_t destip2, uint64_t destmac2, uint64_t sourcemac2, uint32_t sourceip2, uint32_t udpport2) {
#ifdef VIRTUAL
    return OK;
#endif
	FILE_LOG(logINFOBLUE, ("Configuring MAC\n"));

	uint32_t sourceport  =  DEFAULT_TX_UDP_PORT;

	FILE_LOG(logINFO, ("\t#Interfaces : %d\n", numInterfaces));
	FILE_LOG(logINFO, ("\tInterface   : %d\n\n", selInterface));

	FILE_LOG(logINFO, ("\tSource IP   : %d.%d.%d.%d \t\t(0x%08x)\n",
	        (sourceip>>24)&0xff,(sourceip>>16)&0xff,(sourceip>>8)&0xff,(sourceip)&0xff, sourceip));
	FILE_LOG(logINFO, ("\tSource MAC  : %02x:%02x:%02x:%02x:%02x:%02x \t(0x%010llx)\n",
			(unsigned int)((sourcemac>>40)&0xFF),
			(unsigned int)((sourcemac>>32)&0xFF),
			(unsigned int)((sourcemac>>24)&0xFF),
			(unsigned int)((sourcemac>>16)&0xFF),
			(unsigned int)((sourcemac>>8)&0xFF),
			(unsigned int)((sourcemac>>0)&0xFF),
			(long  long unsigned int)sourcemac));
	FILE_LOG(logINFO, ("\tSource Port : %d \t\t\t(0x%08x)\n",sourceport, sourceport));

	FILE_LOG(logINFO, ("\tDest. IP    : %d.%d.%d.%d \t\t\t(0x%08x)\n",
	        (destip>>24)&0xff,(destip>>16)&0xff,(destip>>8)&0xff,(destip)&0xff, destip));
	FILE_LOG(logINFO, ("\tDest. MAC   : %02x:%02x:%02x:%02x:%02x:%02x \t(0x%010llx)\n",
			(unsigned int)((destmac>>40)&0xFF),
			(unsigned int)((destmac>>32)&0xFF),
			(unsigned int)((destmac>>24)&0xFF),
			(unsigned int)((destmac>>16)&0xFF),
			(unsigned int)((destmac>>8)&0xFF),
			(unsigned int)((destmac>>0)&0xFF),
			(long  long unsigned int)destmac));
	FILE_LOG(logINFO, ("\tDest. Port  : %d \t\t\t(0x%08x)\n\n",udpport, udpport));

	uint32_t sourceport2  =  DEFAULT_TX_UDP_PORT + 1;
	FILE_LOG(logINFO, ("\tSource IP2  : %d.%d.%d.%d \t\t(0x%08x)\n",
	        (sourceip2>>24)&0xff,(sourceip2>>16)&0xff,(sourceip2>>8)&0xff,(sourceip2)&0xff, sourceip2));
	FILE_LOG(logINFO, ("\tSource MAC2 : %02x:%02x:%02x:%02x:%02x:%02x \t(0x%010llx)\n",
			(unsigned int)((sourcemac2>>40)&0xFF),
			(unsigned int)((sourcemac2>>32)&0xFF),
			(unsigned int)((sourcemac2>>24)&0xFF),
			(unsigned int)((sourcemac2>>16)&0xFF),
			(unsigned int)((sourcemac2>>8)&0xFF),
			(unsigned int)((sourcemac2>>0)&0xFF),
			(long  long unsigned int)sourcemac2));
	FILE_LOG(logINFO, ("\tSource Port2: %d \t\t\t(0x%08x)\n",sourceport2, sourceport2));

	FILE_LOG(logINFO, ("\tDest. IP2   : %d.%d.%d.%d \t\t\t(0x%08x)\n",
	        (destip2>>24)&0xff,(destip2>>16)&0xff,(destip2>>8)&0xff,(destip2)&0xff, destip2));
	FILE_LOG(logINFO, ("\tDest. MAC2  : %02x:%02x:%02x:%02x:%02x:%02x \t(0x%010llx)\n",
			(unsigned int)((destmac2>>40)&0xFF),
			(unsigned int)((destmac2>>32)&0xFF),
			(unsigned int)((destmac2>>24)&0xFF),
			(unsigned int)((destmac2>>16)&0xFF),
			(unsigned int)((destmac2>>8)&0xFF),
			(unsigned int)((destmac2>>0)&0xFF),
			(long  long unsigned int)destmac2));
	FILE_LOG(logINFO, ("\tDest. Port2 : %d \t\t\t(0x%08x)\n",udpport2, udpport2));

	long int checksum=calcChecksum(sourceip, destip);
	bus_w(TX_IP_REG, sourceip);
	bus_w(RX_IP_REG, destip);

	uint32_t val = 0;

	val = ((sourcemac >> LSB_OF_64_BIT_REG_OFST) & BIT_32_MSK);
	bus_w(TX_MAC_LSB_REG, val);
	FILE_LOG(logDEBUG1, ("Read from TX_MAC_LSB_REG: 0x%08x\n", bus_r(TX_MAC_LSB_REG)));

	val = ((sourcemac >> MSB_OF_64_BIT_REG_OFST) & BIT_32_MSK);
	bus_w(TX_MAC_MSB_REG,val);
	FILE_LOG(logDEBUG1, ("Read from TX_MAC_MSB_REG: 0x%08x\n", bus_r(TX_MAC_MSB_REG)));

	val = ((destmac >> LSB_OF_64_BIT_REG_OFST) & BIT_32_MSK);
	bus_w(RX_MAC_LSB_REG, val);
	FILE_LOG(logDEBUG1, ("Read from RX_MAC_LSB_REG: 0x%08x\n", bus_r(RX_MAC_LSB_REG)));

	val = ((destmac >> MSB_OF_64_BIT_REG_OFST) & BIT_32_MSK);
	bus_w(RX_MAC_MSB_REG, val);
	FILE_LOG(logDEBUG1, ("Read from RX_MAC_MSB_REG: 0x%08x\n", bus_r(RX_MAC_MSB_REG)));

	val = (((sourceport << UDP_PORT_TX_OFST) & UDP_PORT_TX_MSK) |
			((udpport << UDP_PORT_RX_OFST) & UDP_PORT_RX_MSK));
	bus_w(UDP_PORT_REG, val);
	FILE_LOG(logDEBUG1, ("Read from UDP_PORT_REG: 0x%08x\n", bus_r(UDP_PORT_REG)));

	bus_w(TX_IP_CHECKSUM_REG,(checksum << TX_IP_CHECKSUM_OFST) & TX_IP_CHECKSUM_MSK);
	FILE_LOG(logDEBUG1, ("Read from TX_IP_CHECKSUM_REG: 0x%08x\n", bus_r(TX_IP_CHECKSUM_REG)));
	cleanFifos();
	resetCore();
	alignDeserializer();
	return OK;
}


int setDetectorPosition(int pos[]) {
	int ret = OK;
	FILE_LOG(logDEBUG1, ("Setting detector position: (%d, %d)\n", pos[0], pos[1]));

	bus_w(COORD_0_REG, bus_r(COORD_0_REG) & (~(COORD_0_X_MSK)));
	bus_w(COORD_0_REG, bus_r(COORD_0_REG) | ((pos[0] << COORD_0_X_OFST) & COORD_0_X_MSK));
	if ((bus_r(COORD_0_REG) &  COORD_0_X_MSK) != ((pos[0] << COORD_0_X_OFST) & COORD_0_X_MSK))
		ret = FAIL;

	bus_w(COORD_0_REG, bus_r(COORD_0_REG) & (~(COORD_0_Y_MSK)));
	bus_w(COORD_0_REG, bus_r(COORD_0_REG) | ((pos[1] << COORD_0_Y_OFST) & COORD_0_Y_MSK));
	if ((bus_r(COORD_0_REG) &  COORD_0_Y_MSK) != ((pos[1] << COORD_0_Y_OFST) & COORD_0_Y_MSK))
		ret = FAIL;

	if (ret == OK) {
		FILE_LOG(logINFO, ("Position set to [%d, %d]\n", pos[0], pos[1]));
	}
	return ret;
}



/* jungfrau specific - powerchip, autocompdisable, asictimer, clockdiv, pll, flashing fpga */



int powerChip (int on){
    if(on != -1){
        if(on){
            FILE_LOG(logINFO, ("Powering chip: on\n"));
            bus_w(CHIP_POWER_REG, bus_r(CHIP_POWER_REG) | CHIP_POWER_ENABLE_MSK);
        }
        else{
            FILE_LOG(logINFO, ("Powering chip: off\n"));
            bus_w(CHIP_POWER_REG, bus_r(CHIP_POWER_REG) & ~CHIP_POWER_ENABLE_MSK);
        }
    }

    return ((bus_r(CHIP_POWER_REG) & CHIP_POWER_STATUS_MSK) >> CHIP_POWER_STATUS_OFST);
}



int autoCompDisable(int on) {
    if(on != -1){
        if(on){
            FILE_LOG(logINFO, ("Auto comp disable mode: on\n"));
            bus_w(VREF_COMP_MOD_REG, bus_r(VREF_COMP_MOD_REG) | VREF_COMP_MOD_ENABLE_MSK);
        }
        else{
            FILE_LOG(logINFO, ("Auto comp disable mode: off\n"));
            bus_w(VREF_COMP_MOD_REG, bus_r(VREF_COMP_MOD_REG) & ~VREF_COMP_MOD_ENABLE_MSK);
        }
    }

    return (bus_r(VREF_COMP_MOD_REG) & VREF_COMP_MOD_ENABLE_MSK);
}

void configureASICTimer() {
    FILE_LOG(logINFO, ("Configuring ASIC Timer\n"));
    bus_w(ASIC_CTRL_REG, (bus_r(ASIC_CTRL_REG) & ~ASIC_CTRL_PRCHRG_TMR_MSK) | ASIC_CTRL_PRCHRG_TMR_VAL);
    bus_w(ASIC_CTRL_REG, (bus_r(ASIC_CTRL_REG) & ~ASIC_CTRL_DS_TMR_MSK) | ASIC_CTRL_DS_TMR_VAL);
}

void setClockDivider(int val) {
    // setting
    if(val >= 0) {

        // stop state machine if running
        if(runBusy())
            stopStateMachine();

        uint32_t txndelay_msk = 0;

        switch(val){

        // todo in firmware, for now setting half speed
        case FULL_SPEED://40
            FILE_LOG(logINFO, ("Setting Half Speed (20 MHz):\n"));

            FILE_LOG(logINFO, ("\tSetting Sample Reg to 0x%x\n", SAMPLE_ADC_HALF_SPEED));
            bus_w(SAMPLE_REG, SAMPLE_ADC_HALF_SPEED);

            txndelay_msk = (bus_r(CONFIG_REG) & CONFIG_TDMA_TIMESLOT_MSK); // read config tdma timeslot value
            FILE_LOG(logINFO, ("\tSetting Config Reg to 0x%x\n", CONFIG_HALF_SPEED | txndelay_msk));
            bus_w(CONFIG_REG, CONFIG_HALF_SPEED | txndelay_msk);

            FILE_LOG(logINFO, ("\tSetting ADC Ofst Reg to 0x%x\n", ADC_OFST_HALF_SPEED_VAL));
            bus_w(ADC_OFST_REG, ADC_OFST_HALF_SPEED_VAL);

            FILE_LOG(logINFO, ("\tSetting ADC Phase Reg to 0x%x\n", ADC_PHASE_HALF_SPEED));
            setAdcPhase(ADC_PHASE_HALF_SPEED, 0);

            break;
        case HALF_SPEED:
            FILE_LOG(logINFO, ("Setting Half Speed (20 MHz):\n"));

            FILE_LOG(logINFO, ("\tSetting Sample Reg to 0x%x\n", SAMPLE_ADC_HALF_SPEED));
            bus_w(SAMPLE_REG, SAMPLE_ADC_HALF_SPEED);

            txndelay_msk = (bus_r(CONFIG_REG) & CONFIG_TDMA_TIMESLOT_MSK); // read config tdma timeslot value
            FILE_LOG(logINFO, ("\tSetting Config Reg to 0x%x\n", CONFIG_HALF_SPEED | txndelay_msk));
            bus_w(CONFIG_REG, CONFIG_HALF_SPEED | txndelay_msk);

            FILE_LOG(logINFO, ("\tSetting ADC Ofst Reg to 0x%x\n", ADC_OFST_HALF_SPEED_VAL));
            bus_w(ADC_OFST_REG, ADC_OFST_HALF_SPEED_VAL);

            FILE_LOG(logINFO, ("\tSetting ADC Phase Reg to 0x%x\n", ADC_PHASE_HALF_SPEED));
            setAdcPhase(ADC_PHASE_HALF_SPEED, 0);

            break;
        case QUARTER_SPEED:
            FILE_LOG(logINFO, ("Setting Half Speed (10 MHz):\n"));

            FILE_LOG(logINFO, ("\tSetting Sample Reg to 0x%x\n", SAMPLE_ADC_QUARTER_SPEED));
            bus_w(SAMPLE_REG, SAMPLE_ADC_QUARTER_SPEED);

            txndelay_msk = (bus_r(CONFIG_REG) & CONFIG_TDMA_TIMESLOT_MSK); // read config tdma timeslot value
            FILE_LOG(logINFO, ("\tSetting Config Reg to 0x%x\n", CONFIG_QUARTER_SPEED | txndelay_msk));
            bus_w(CONFIG_REG, CONFIG_QUARTER_SPEED | txndelay_msk);

            FILE_LOG(logINFO, ("\tSetting ADC Ofst Reg to 0x%x\n", ADC_OFST_QUARTER_SPEED_VAL));
            bus_w(ADC_OFST_REG, ADC_OFST_QUARTER_SPEED_VAL);

            FILE_LOG(logINFO, ("\tSetting ADC Phase Reg to 0x%x\n", ADC_PHASE_QUARTER_SPEED));
            setAdcPhase(ADC_PHASE_QUARTER_SPEED, 0);

            break;
        }
    }
}

int getClockDivider() {
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

void setAdcPhase(int val, int degrees){
	int maxShift = MAX_PHASE_SHIFTS;

	// validation
	if (degrees && (val < 0 || val > 359)) {
		 FILE_LOG(logERROR, ("\tPhase provided outside limits (0 - 359°C)\n"));
		 return;
	}
	if (!degrees && (val < 0 || val > MAX_PHASE_SHIFTS - 1)) {
		 FILE_LOG(logERROR, ("\tPhase provided outside limits (0 - %d phase shifts)\n", maxShift - 1));
		 return;
	}

    FILE_LOG(logINFO, ("Setting ADC Phase to %d (degree mode: %d)\n", val, degrees));
	int valShift = val;
	// convert to phase shift
	if (degrees) {
		ConvertToDifferentRange(0, 359, 0, maxShift - 1, val, &valShift);
	}
	FILE_LOG(logDEBUG1, ("phase shift: %d (degrees/shift: %d)\n", valShift, val));

	int relativePhase = valShift - adcPhase;
	FILE_LOG(logDEBUG1, ("relative phase shift: %d (Current phase: %d)\n", relativePhase, adcPhase));

    // same phase
    if (!relativePhase) {
    	FILE_LOG(logINFO, ("Nothing to do in Phase Shift\n"));
    	return;
    }

    int phase = 0;
    if (relativePhase > 0) {
        phase = (maxShift - relativePhase);
    } else {
    	phase = (-1) * relativePhase;
    }
    FILE_LOG(logDEBUG1, ("[Single Direction] Phase:%d (0x%x). Max Phase shifts:%d\n", phase, phase, maxShift));

    ALTERA_PLL_SetPhaseShift(phase, 1, 0);

    adcPhase = valShift;
}

int getPhase(degrees) {
	if (!degrees)
		return adcPhase;
	// convert back to degrees
	int val = 0;
	ConvertToDifferentRange(0, MAX_PHASE_SHIFTS - 1, 0, 359, adcPhase, &val);
	return val;
}

int getMaxPhaseShift() {
	return MAX_PHASE_SHIFTS;
}

int validatePhaseinDegrees(int val, int retval) {
	if (val == -1)
		return OK;
	FILE_LOG(logDEBUG1, ("validating phase in degrees\n"));
	int maxShift = MAX_PHASE_SHIFTS;
	// convert degrees to shift
	int valShift = 0;
	ConvertToDifferentRange(0, 359, 0, maxShift - 1, val, &valShift);
	// convert back to degrees
	ConvertToDifferentRange(0, maxShift - 1, 0, 359, valShift, &val);

	if (val == retval)
		return OK;
	return FAIL;
}


int setThresholdTemperature(int val) {

    if (val >= 0) {
        FILE_LOG(logINFO, ("Setting Threshold Temperature: %f °C\n", val/1000.00));
        val *= (10.0/625.0);
        FILE_LOG(logDEBUG1, ("Converted Threshold Temperature: %d\n", val));
        bus_w(TEMP_CTRL_REG, (bus_r(TEMP_CTRL_REG) &~(TEMP_CTRL_PROTCT_THRSHLD_MSK) &~(TEMP_CTRL_OVR_TMP_EVNT_MSK))
                | (((val  << TEMP_CTRL_PROTCT_THRSHLD_OFST) & TEMP_CTRL_PROTCT_THRSHLD_MSK)));
        FILE_LOG(logDEBUG1, ("Converted Threshold Temperature set to %d\n",
                ((bus_r(TEMP_CTRL_REG) & TEMP_CTRL_PROTCT_THRSHLD_MSK) >> TEMP_CTRL_PROTCT_THRSHLD_OFST)));
    }
    uint32_t temp = ((bus_r(TEMP_CTRL_REG) & TEMP_CTRL_PROTCT_THRSHLD_MSK) >> TEMP_CTRL_PROTCT_THRSHLD_OFST);

    // conversion
    temp = (temp * (625.0/10.0));

    float ftemp = (double)temp/1000.00;
    FILE_LOG(logDEBUG1, ("Threshold Temperature read %f °C\n",ftemp));

    return temp;

}


int setTemperatureControl(int val) {
    if (val >= 0) {
        // binary value
        if (val > 0 ) val = 1;
        FILE_LOG(logINFO, ("Setting Temperature control: %d\n", val));
        bus_w(TEMP_CTRL_REG, (bus_r(TEMP_CTRL_REG)  &~(TEMP_CTRL_PROTCT_ENABLE_MSK) &~(TEMP_CTRL_OVR_TMP_EVNT_MSK))
                | (((val  << TEMP_CTRL_PROTCT_ENABLE_OFST) & TEMP_CTRL_PROTCT_ENABLE_MSK)));
        FILE_LOG(logDEBUG1, ("Temperature control read: %d\n",
                ((bus_r(TEMP_CTRL_REG) & TEMP_CTRL_PROTCT_ENABLE_MSK) >> TEMP_CTRL_PROTCT_ENABLE_OFST)));
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
        FILE_LOG(logINFO, ("Setting Temperature Event (clearing): %d\n", val));
        bus_w(TEMP_CTRL_REG, (bus_r(TEMP_CTRL_REG)   &~TEMP_CTRL_OVR_TMP_EVNT_MSK)
                | (((val  << TEMP_CTRL_OVR_TMP_EVNT_OFST) & TEMP_CTRL_OVR_TMP_EVNT_MSK)));
        FILE_LOG(logDEBUG1, ("Temperature Event read %d\n",
                ((bus_r(TEMP_CTRL_REG) & TEMP_CTRL_OVR_TMP_EVNT_MSK) >> TEMP_CTRL_OVR_TMP_EVNT_OFST)));
    }
    return ((bus_r(TEMP_CTRL_REG) & TEMP_CTRL_OVR_TMP_EVNT_MSK) >> TEMP_CTRL_OVR_TMP_EVNT_OFST);
}

void alignDeserializer() {
	// refresh alignment
	bus_w(ADC_DSRLZR_0_REG, bus_r(ADC_DSRLZR_0_REG) | ADC_DSRLZR_0_RFRSH_ALGNMNT_MSK);
	bus_w(ADC_DSRLZR_1_REG, bus_r(ADC_DSRLZR_1_REG) | ADC_DSRLZR_1_RFRSH_ALGNMNT_MSK);
	bus_w(ADC_DSRLZR_2_REG, bus_r(ADC_DSRLZR_2_REG) | ADC_DSRLZR_2_RFRSH_ALGNMNT_MSK);
	bus_w(ADC_DSRLZR_3_REG, bus_r(ADC_DSRLZR_3_REG) | ADC_DSRLZR_3_RFRSH_ALGNMNT_MSK);

	usleep(1 * 1000 * 1000);

	// disable the refresh
	bus_w(ADC_DSRLZR_0_REG, bus_r(ADC_DSRLZR_0_REG) & (~(ADC_DSRLZR_0_RFRSH_ALGNMNT_MSK)));
	bus_w(ADC_DSRLZR_1_REG, bus_r(ADC_DSRLZR_1_REG) & (~(ADC_DSRLZR_1_RFRSH_ALGNMNT_MSK)));
	bus_w(ADC_DSRLZR_2_REG, bus_r(ADC_DSRLZR_2_REG) & (~(ADC_DSRLZR_2_RFRSH_ALGNMNT_MSK)));
	bus_w(ADC_DSRLZR_3_REG, bus_r(ADC_DSRLZR_3_REG) & (~(ADC_DSRLZR_3_RFRSH_ALGNMNT_MSK)));
}


int setNetworkParameter(enum NETWORKINDEX mode, int value) {
    if (mode != TXN_FRAME)
        return -1;

    if (value >= 0) {
        FILE_LOG(logINFO, ("Setting transmission delay: %d\n", value));
        bus_w(CONFIG_REG, (bus_r(CONFIG_REG) &~CONFIG_TDMA_TIMESLOT_MSK)
                | (((value  << CONFIG_TDMA_TIMESLOT_OFST) & CONFIG_TDMA_TIMESLOT_MSK)));
        if (value == 0)
            bus_w(CONFIG_REG, bus_r(CONFIG_REG) &~ CONFIG_TDMA_MSK);
        else
            bus_w(CONFIG_REG, bus_r(CONFIG_REG) | CONFIG_TDMA_MSK);
        FILE_LOG(logDEBUG1, ("Transmission delay read %d\n",
                ((bus_r(CONFIG_REG) & CONFIG_TDMA_TIMESLOT_MSK) >> CONFIG_TDMA_TIMESLOT_OFST)));
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
		FILE_LOG(logERROR, ("Could not start Virtual acquisition thread\n"));
		return FAIL;
	}
	FILE_LOG(logINFOGREEN, ("Virtual Acquisition started\n"));
	return OK;
#endif
	FILE_LOG(logINFOBLUE, ("Starting State Machine\n"));

	cleanFifos();

	//start state machine
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_START_ACQ_MSK);
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_START_ACQ_MSK);

	FILE_LOG(logINFO, ("Status Register: %08x\n",bus_r(STATUS_REG)));
	return OK;
}


#ifdef VIRTUAL
void* start_timer(void* arg) {
	int wait_in_s = 	(setTimer(FRAME_NUMBER, -1) *
						setTimer(CYCLES_NUMBER, -1) *
						(setTimer(STORAGE_CELL_NUMBER, -1) + 1) *
						(setTimer(FRAME_PERIOD, -1)/(1E9)));
	FILE_LOG(logDEBUG1, ("going to wait for %d s\n", wait_in_s));
	while(!virtual_stop && (wait_in_s >= 0)) {
		usleep(1000 * 1000);
		wait_in_s--;
	}
	FILE_LOG(logINFOGREEN, ("Virtual Timer Done\n"));

	virtual_status = 0;
	return NULL;
}
#endif

int stopStateMachine(){
	FILE_LOG(logINFORED, ("Stopping State Machine\n"));
#ifdef VIRTUAL
	virtual_stop = 0;
	return OK;
#endif
	//stop state machine
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_STOP_ACQ_MSK);
	usleep(100);
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_STOP_ACQ_MSK);

	FILE_LOG(logINFO, ("Status Register: %08x\n",bus_r(STATUS_REG)));
	return OK;
}





enum runStatus getRunStatus(){
#ifdef VIRTUAL
	if(virtual_status == 0){
		FILE_LOG(logINFOBLUE, ("Status: IDLE\n"));
		return IDLE;
	}else{
		FILE_LOG(logINFOBLUE, ("Status: RUNNING\n"));
		return RUNNING;
	}
#endif
	FILE_LOG(logDEBUG1, ("Getting status\n"));

	enum runStatus s;
	u_int32_t retval = bus_r(STATUS_REG);
	FILE_LOG(logINFO, ("Status Register: %08x\n",retval));

	//running
	if (retval & RUN_BUSY_MSK) {
		if (retval & WAITING_FOR_TRIGGER_MSK) {
			FILE_LOG(logINFOBLUE, ("Status: WAITING\n"));
			s = WAITING;
		}
		else{
			FILE_LOG(logINFOBLUE, ("Status: RUNNING\n"));
			s = RUNNING;
		}
	}

	//not running
	else {
	    // stopped or error
		if (retval & STOPPED_MSK) {
			FILE_LOG(logINFOBLUE, ("Status: STOPPED\n"));
			s = STOPPED;
		} else if (retval & RUNMACHINE_BUSY_MSK) {
			FILE_LOG(logINFOBLUE, ("Status: READ MACHINE BUSY\n"));
			s = TRANSMITTING;
		} else if (!retval) {
			FILE_LOG(logINFOBLUE, ("Status: IDLE\n"));
			s = IDLE;
		} else {
			FILE_LOG(logERROR, ("Status: Unknown status %08x\n", retval));
			s = ERROR;
		}
	}

	return s;
}



void readFrame(int *ret, char *mess){
#ifdef VIRTUAL
	while(virtual_status) {
		//FILE_LOG(logERROR, ("Waiting for finished flag\n");
		usleep(5000);
	}
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
		sprintf(mess,"No data and run stopped: %lld frames left\n",(long  long int)retval);
		FILE_LOG(logERROR, (mess));
	} else {
		*ret = (int)OK;
		FILE_LOG(logINFOGREEN, ("Acquisition successfully finished\n"));
	}
}



u_int32_t runBusy() {
#ifdef VIRTUAL
    return virtual_status;
#endif
	u_int32_t s = (bus_r(STATUS_REG) & RUN_BUSY_MSK);
	FILE_LOG(logDEBUG1, ("Status Register: %08x\n", s));
	return s;
}








/* common */

int calculateDataBytes(){
	return DATA_BYTES;
}

int getTotalNumberOfChannels(){return  ((int)getNumberOfChannelsPerChip() * (int)getNumberOfChips());}
int getNumberOfChips(){return  NCHIP;}
int getNumberOfDACs(){return  NDAC;}
int getNumberOfChannelsPerChip(){return  NCHAN;}


