#include "slsDetectorFunctionList.h"
#include "gitInfoCtb.h"
#include "versionAPI.h"
#include "logger.h"

#include "communication_funcs_UDP.h"
#include "UDPPacketHeaderGenerator.h"
#include "AD9257.h"		// commonServerFunctions.h, blackfin.h, ansi.h
#include "AD7689.h"     // slow adcs
#include "LTC2620.h"    // dacs
#include "MAX1932.h"    // hv
#include "INA226.h"     // i2c
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

// Global variable from UDPPacketHeaderGenerator
extern uint64_t udpFrameNumber;
extern uint32_t udpPacketNumber;

int firmware_compatibility = OK;
int firmware_check_done = 0;
char firmware_message[MAX_STR_LENGTH];

#ifdef VIRTUAL
pthread_t pthread_virtual_tid;
int virtual_status = 0;
int virtual_stop = 0;
#endif

int dataBytes = 0;
char* ramValues = 0;
char udpPacketData[UDP_PACKET_DATA_BYTES + sizeof(sls_detector_header)];

int32_t clkPhase[NUM_CLOCKS] = {0, 0, 0, 0};
uint32_t clkDivider[NUM_CLOCKS] = {40, 20, 20, 200};

int dacValues[NDAC] = {0};
// software limit that depends on the current chip on the ctb
int vLimit = 0;

int highvoltage = 0;
ROI rois[MAX_ROIS];
int nROI = 0;
uint32_t adcDisableMask = 0;
int analogEnable = 1;
int digitalEnable = 0;
int nSamples = 1;
char volatile *now_ptr = 0;

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
    FILE_LOG(logINFOBLUE, ("******** Chip Test Board Virtual Server *****************\n"));
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
		cprintf(RED,"exiting for now!\n");exit(-1); return;
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
	FILE_LOG(logINFOBLUE, ("************ Chip Test Board Server *********************\n"
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
	FILE_LOG(logINFO, ("\tCompatibility - success\n"));
	firmware_check_done = 1;
#endif
}

int checkType() {
#ifdef VIRTUAL
    return OK;
#endif
    uint32_t type = ((bus_r(FPGA_VERSION_REG) & FPGA_VERSION_DTCTR_TYP_MSK) >> FPGA_VERSION_DTCTR_TYP_OFST);
	uint32_t expectedType = (((FPGA_VERSION_DTCTR_TYP_CTB_VAL) & FPGA_VERSION_DTCTR_TYP_MSK) >> FPGA_VERSION_DTCTR_TYP_OFST);

	if (type != expectedType) {
        FILE_LOG(logERROR, ("(Type Fail) - This is not a Chip Test Board Server (read %d, expected %d)\n",
                type, expectedType));
        return FAIL;
	}
	return OK;
}

uint32_t testFpga(void) {
#ifdef VIRTUAL
    return OK;
#endif
    FILE_LOG(logINFO, ("Testing FPGA:\n"));

    //fixed pattern
    int ret = OK;
    uint32_t val = bus_r(FIX_PATT_REG);
    if (val == FIX_PATT_VAL) {
        FILE_LOG(logINFO, ("\tFixed pattern: successful match (0x%08x)\n",val));
    } else {
        FILE_LOG(logERROR, ("Fixed pattern does not match! Read 0x%08x, expected 0x%08x\n", val, FIX_PATT_VAL));
        ret = FAIL;
    }

    if (ret == OK) {
        // Delay LSB reg
        FILE_LOG(logINFO, ("\tTesting Delay LSB Register:\n"));
        uint32_t addr = DELAY_LSB_REG;

        // store previous delay value
        uint32_t previousValue = bus_r(addr);

        volatile uint32_t val = 0, readval = 0;
        int times = 1000 * 1000;
        int i = 0;
        for (i = 0; i < times; ++i) {
            val = 0x5A5A5A5A - i;
            bus_w(addr, val);
            readval = bus_r(addr);
            if (readval != val) {
                FILE_LOG(logERROR, ("1:Mismatch! Loop(%d): Wrote 0x%x, read 0x%x\n",
                        i, val, readval));
                ret = FAIL;
                break;
            }
            val = (i + (i << 10) + (i << 20));
            bus_w(addr, val);
            readval = bus_r(addr);
            if (readval != val) {
                FILE_LOG(logERROR, ("2:Mismatch! Loop(%d): Wrote 0x%x, read 0x%x\n",
                        i, val, readval));
                ret = FAIL;
                break;
            }
            val = 0x0F0F0F0F;
            bus_w(addr, val);
            readval = bus_r(addr);
            if (readval != val) {
                FILE_LOG(logERROR, ("3:Mismatch! Loop(%d): Wrote 0x%x, read 0x%x\n",
                        i, val, readval));
                ret = FAIL;
                break;
            }
            val = 0xF0F0F0F0;
            bus_w(addr, val);
            readval = bus_r(addr);
            if (readval != val) {
                FILE_LOG(logERROR, ("4:Mismatch! Loop(%d): Wrote 0x%x, read 0x%x\n",
                        i, val, readval));
                ret = FAIL;
                break;
            }
        }
        // write back previous value
        bus_w(addr, previousValue);
        if (ret == OK) {
            FILE_LOG(logINFO, ("\tSuccessfully tested FPGA Delay LSB Register %d times\n", times));
        }
    }

    return ret;
}

int testBus() {
#ifdef VIRTUAL
    return OK;
#endif
    FILE_LOG(logINFO, ("Testing Bus:\n"));

    int ret = OK;
    uint32_t addr = DELAY_LSB_REG;

    // store previous delay value
    uint32_t previousValue = bus_r(addr);

    volatile uint32_t val = 0, readval = 0;
    int times = 1000 * 1000;
    int i = 0;

    for (i = 0; i < times; ++i) {
        val += 0xbbbbb;
        bus_w(addr, val);
         readval = bus_r(addr);
        if (readval != val) {
            FILE_LOG(logERROR, ("Mismatch! Loop(%d): Wrote 0x%x, read 0x%x\n",
                    i, val, readval));
            ret = FAIL;
        }
    }

    // write back previous value
    bus_w(addr, previousValue);

    if (ret == OK) {
        FILE_LOG(logINFO, ("\tSuccessfully tested bus %d times\n", times));
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
		return getDetectorNumber();
	case DETECTOR_FIRMWARE_VERSION:
		return getFirmwareVersion();
	case SOFTWARE_FIRMWARE_API_VERSION:
	    return getFirmwareAPIVersion();
	case DETECTOR_SOFTWARE_VERSION:
		return  (GITDATE & 0xFFFFFF);
	case CLIENT_SOFTWARE_API_VERSION:
		return APICTB;
	default:
		return retval;
	}
}

uint64_t getFirmwareVersion() {
#ifdef VIRTUAL
    return 0;
#endif
	return ((bus_r(FPGA_VERSION_REG) & FPGA_VERSION_BRD_RVSN_MSK) >> FPGA_VERSION_BRD_RVSN_OFST);
}

uint64_t getFirmwareAPIVersion() {
#ifdef VIRTUAL
    return 0;
#endif
    return ((bus_r(API_VERSION_REG) & API_VERSION_MSK) >> API_VERSION_OFST);
}

uint16_t getHardwareVersionNumber() {
#ifdef VIRTUAL
    return 0;
#endif
	return ((bus_r(MOD_SERIAL_NUMBER_REG) & MOD_SERIAL_NUMBER_VRSN_MSK) >> MOD_SERIAL_NUMBER_VRSN_OFST);
}

uint16_t getHardwareSerialNumber() {
#ifdef VIRTUAL
    return 0;
#endif
	return ((bus_r(MOD_SERIAL_NUMBER_REG) & MOD_SERIAL_NUMBER_MSK) >> MOD_SERIAL_NUMBER_OFST);
}

uint32_t getDetectorNumber(){
#ifdef VIRTUAL
    return 0;
#endif
	return bus_r(MOD_SERIAL_NUMBER_REG);
}

uint64_t  getDetectorMAC() {
#ifdef VIRTUAL
    return 0;
#else
	char output[255],mac[255]="";
	uint64_t res=0;
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

uint32_t  getDetectorIP(){
#ifdef VIRTUAL
    return 0;
#endif
	char temp[50]="";
	uint32_t res=0;
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
    FILE_LOG(logINFO, ("This Server is for 1 Chip Test Board module\n"));

    // default variables
    dataBytes = 0;
    if (ramValues) {
        free(ramValues);
        ramValues = 0;
    }
    {
        int i = 0;
        for (i = 0; i < NUM_CLOCKS; ++i) {
            clkPhase[i] = 0;
        }
        clkDivider[RUN_CLK] = DEFAULT_RUN_CLK;
        clkDivider[ADC_CLK] = DEFAULT_ADC_CLK;
        clkDivider[SYNC_CLK] = DEFAULT_SYNC_CLK;
        clkDivider[DBIT_CLK] = DEFAULT_DBIT_CLK;
        for (i = 0; i < NDAC; ++i)
            dacValues[i] = -1;
    }
    vLimit = DEFAULT_VLIMIT;
    highvoltage = 0;
    nROI = 0;
    adcDisableMask = 0;
    analogEnable = 1;
    digitalEnable = 0;
    nSamples = 1;
    now_ptr = 0;


    ALTERA_PLL_ResetPLLAndReconfiguration();
    resetCore();
    resetPeripheral();
    cleanFifos();

    // hv
    MAX1932_SetDefines(SPI_REG, SPI_HV_SRL_CS_OTPT_MSK, SPI_HV_SRL_CLK_OTPT_MSK, SPI_HV_SRL_DGTL_OTPT_MSK, SPI_HV_SRL_DGTL_OTPT_OFST, HIGHVOLTAGE_MIN, HIGHVOLTAGE_MAX);
    MAX1932_Disable();
    setHighVoltage(DEFAULT_HIGH_VOLTAGE);

    // power off voltage regulators
    powerOff();

    // adcs
    AD9257_SetDefines(ADC_SPI_REG, ADC_SPI_SRL_CS_OTPT_MSK, ADC_SPI_SRL_CLK_OTPT_MSK, ADC_SPI_SRL_DT_OTPT_MSK, ADC_SPI_SRL_DT_OTPT_OFST);
    AD9257_Disable();
    AD9257_Configure();

    // slow adcs
    AD7689_SetDefines(ADC_SPI_REG, ADC_SPI_SLOW_VAL_REG, ADC_SPI_SLOW_SRL_CNV_MSK, ADC_SPI_SLOW_SRL_CLK_MSK, ADC_SPI_SLOW_SRL_DT_MSK, ADC_SPI_SLOW_SRL_DT_OFST);
    AD7689_Disable();
    AD7689_Configure();

    // dacs
    LTC2620_SetDefines(SPI_REG, SPI_DAC_SRL_CS_OTPT_MSK, SPI_DAC_SRL_CLK_OTPT_MSK, SPI_DAC_SRL_DGTL_OTPT_MSK, SPI_DAC_SRL_DGTL_OTPT_OFST, NDAC, DAC_MIN_MV, DAC_MAX_MV);  //has to be before setvchip
    LTC2620_Disable();
    LTC2620_Configure();
	// switch off dacs (power regulators most likely only sets to minimum (if power enable on))
    FILE_LOG(logINFOBLUE, ("Powering down all dacs\n"));
	{
	    int idac = 0;
	    for (idac = 0; idac < NDAC; ++idac) {
	        setDAC(idac, LTC2620_PWR_DOWN_VAL, 0); //has to be before setvchip
	    }
	}

    // power regulators
    // I2C
    INA226_ConfigureI2CCore(I2C_SHUNT_RESISTER_OHMS, I2C_CONTROL_REG, I2C_STATUS_REG, I2C_RX_DATA_FIFO_REG, I2C_RX_DATA_FIFO_LEVEL_REG, I2C_SCL_LOW_COUNT_REG, I2C_SCL_HIGH_COUNT_REG, I2C_SDA_HOLD_REG, I2C_TRANSFER_COMMAND_FIFO_REG);
    INA226_CalibrateCurrentRegister(I2C_POWER_VIO_DEVICE_ID);
    INA226_CalibrateCurrentRegister(I2C_POWER_VA_DEVICE_ID);
    INA226_CalibrateCurrentRegister(I2C_POWER_VB_DEVICE_ID);
    INA226_CalibrateCurrentRegister(I2C_POWER_VC_DEVICE_ID);
    INA226_CalibrateCurrentRegister(I2C_POWER_VD_DEVICE_ID);
    setVchip(VCHIP_MIN_MV);

	// altera pll
	ALTERA_PLL_SetDefines(PLL_CNTRL_REG, PLL_PARAM_REG, PLL_CNTRL_RCNFG_PRMTR_RST_MSK, PLL_CNTRL_WR_PRMTR_MSK, PLL_CNTRL_PLL_RST_MSK, PLL_CNTRL_ADDR_MSK, PLL_CNTRL_ADDR_OFST);

    bus_w(ADC_PORT_INVERT_REG, 0);// depends on chip

	FILE_LOG(logINFOBLUE, ("Setting Default parameters\n"));
	cleanFifos(); // FIXME: why twice?
	resetCore();

	//Initialization of acquistion parameters
    setTimer(SAMPLES, DEFAULT_NUM_SAMPLES); // update databytes and allocate ram
	setTimer(FRAME_NUMBER, DEFAULT_NUM_FRAMES);
	setTimer(CYCLES_NUMBER, DEFAULT_NUM_CYCLES);
	setTimer(FRAME_PERIOD, DEFAULT_PERIOD);
	setTimer(DELAY_AFTER_TRIGGER, DEFAULT_DELAY);
	setTiming(DEFAULT_TIMING_MODE);

	// 1G UDP
	enableTenGigabitEthernet(0);
    // clear roi
    {
        int ret = OK, retvalsize = 0;
        setROI(0, rois, &ret, &retvalsize);
    }
}

int allocateRAM() {
	int oldDataBytes = dataBytes;
	updateDataBytes();

	// only allcoate RAM for 1 giga udp (if 10G, return)
	if (enableTenGigabitEthernet(-1))
		return OK;


	// update only if change in databytes
	if (dataBytes == oldDataBytes) {
		FILE_LOG(logDEBUG1, ("RAM of size %d already allocated. Nothing to be done.\n", dataBytes));
		return OK;
	}
	// Zero databytes
	if (dataBytes <= 0) {
		FILE_LOG(logERROR, ("Can not allocate RAM for 0 bytes (databytes: 0).\n"));
		return FAIL;
	}
	// clear RAM
	if (ramValues) {
		free(ramValues);
		ramValues = 0;
	}
	// allocate RAM
	ramValues = malloc(dataBytes);
	// cannot malloc
	if (ramValues == NULL) {
		FILE_LOG(logERROR, ("Can not allocate RAM for even 1 frame. "
				"Probably cause: Memory Leak.\n"));
		return FAIL;
	}

	FILE_LOG(logINFO, ("\tRAM allocated to %d bytes\n", dataBytes));
	return OK;
}

void updateDataBytes() {
    int oldDataBytes = dataBytes;
    dataBytes = NCHIP * getChannels() * NUM_BYTES_PER_PIXEL * nSamples;
    if (dataBytes != oldDataBytes) {
        FILE_LOG(logINFO, ("\tUpdating Databytes: %d\n", dataBytes));
    }
}

int getChannels() {
    int nchans = 0;

    if (analogEnable) {
        nchans += NCHAN_ANALOG;
        // remove the channels disabled
        int ichan = 0;
        for (ichan = 0; ichan < NCHAN_ANALOG; ++ichan) {
            if (adcDisableMask & (1 << ichan))
                --nchans;
        }
    }
    if (digitalEnable)
        nchans += NCHAN_DIGITAL;
    return nchans;
}


/* firmware functions (resets) */

void cleanFifos() {
#ifdef VIRTUAL
    return;
#endif
	FILE_LOG(logINFO, ("Clearing Acquisition Fifos\n"));
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_CLR_ACQSTN_FIFO_MSK);
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_CLR_ACQSTN_FIFO_MSK);
}

void resetCore() {
#ifdef VIRTUAL
    return;
#endif
	FILE_LOG(logINFO, ("Resetting Core\n"));
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_CRE_RST_MSK);
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_CRE_RST_MSK);
}

void resetPeripheral() {
#ifdef VIRTUAL
    return;
#endif
	FILE_LOG(logINFO, ("Resetting Peripheral\n"));
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_PRPHRL_RST_MSK);
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_PRPHRL_RST_MSK);
}


/* set parameters -  dr, roi */

int setDynamicRange(int dr){
	return DYNAMIC_RANGE;
}

ROI* setROI(int n, ROI arg[], int *retvalsize, int *ret) {
    uint32_t addr = ADC_DISABLE_REG;

    // set ROI
    if(n >= 0) {
        // clear roi
        if (!n) {
            FILE_LOG(logINFO, ("Clearing ROI\n"));
            adcDisableMask = 0;
        }
        // set roi
        else {
            FILE_LOG(logINFO, ("Setting ROI:\n"));
            adcDisableMask = 0xffffffff;
            int iroi = 0;
            // for every roi
            for (iroi = 0; iroi < n; ++iroi) {
                FILE_LOG(logINFO, ("\t%d: (%d, %d)\n", arg[iroi].xmin, arg[iroi].xmax));
                // swap if xmin > xmax
                if (arg[iroi].xmin > arg[iroi].xmax) {
                    int temp = arg[iroi].xmin;
                    arg[iroi].xmin = arg[iroi].xmax;
                    arg[iroi].xmax = temp;
                    FILE_LOG(logINFORED, ("\tCorrected %d: (%d, %d)\n", arg[iroi].xmin, arg[iroi].xmax));
                }
                int ich = 0;
                // for the roi specified
                for (ich = arg[iroi].xmin; ich <= arg[iroi].xmax; ++ich) {
                    // valid channel (disable)
                    if (ich >= 0 && ich < NCHAN_ANALOG)
                        adcDisableMask &= ~(1 << ich);

                    FILE_LOG(logDEBUG1, ("%d: ich:%d adcDisableMask:0x%08x\n",
                            iroi, ich, adcDisableMask));
                }
            }
        }
        FILE_LOG(logINFO, ("\tSetting adcDisableMask to 0x%08x\n", adcDisableMask));
        bus_w(addr, adcDisableMask);
    }

    // get roi
    adcDisableMask = bus_r(addr);
    FILE_LOG(logDEBUG1, ("Getting adcDisableMask: 0x%08x\n", adcDisableMask));

    nROI = 0;
    if (adcDisableMask) {
        int ich = 0;
        // loop through channels
        for (ich = 0; ich < NCHAN_ANALOG; ++ich) {
            // channel disabled
            if ((~adcDisableMask) & (1 << ich)) {
                // first channel
                if (ich == 0) {
                    ++nROI;
                    rois[nROI - 1].xmin = ich;
                    rois[nROI - 1].xmax = ich;
                    rois[nROI - 1].ymin = -1;
                    rois[nROI - 1].ymax = -1;
                }
                // not first channel
                else {
                    // previous channel enabled (so increase roi)
                    if  ((adcDisableMask) & (1 << (ich - 1))) {
                        ++nROI;
                        // max roi level
                        if (nROI > MAX_ROIS) {
                            nROI = -1;
                            *ret = FAIL;
                            FILE_LOG(logERROR, ("Max ROI reached!\n"));
                            break;
                        }
                        rois[nROI - 1].xmin = ich;
                        rois[nROI - 1].ymin = -1;
                        rois[nROI - 1].ymax = -1;
                    }
                    // set max as current one each time
                    rois[nROI - 1].xmax = ich;
                }
            }
        }
    }

    // print
    if (!nROI) {
        FILE_LOG(logINFO, ("\tROI: None\n"));
    } else {
        FILE_LOG(logINFO, ("ROI:\n"));
        int i = 0;
        for (i = 0; i < nROI; ++i) {
            FILE_LOG(logINFO, ("\t%d: (%d, %d)\n", i, rois[i].xmin, rois[i].xmax));

        }
    }

    // validate and update databytes
    if (n >= 0) {
        // validate
        if((n != 0) && ((arg[0].xmin != rois[0].xmin)||
                (arg[0].xmax != rois[0].xmax)||
                (arg[0].ymin != rois[0].ymin)||
                (arg[0].ymax != rois[0].ymax))) {
            *ret = FAIL;
            FILE_LOG(logERROR, ("\tCould not set given ROI\n"));
        }
        if(n != nROI) {
            *ret = FAIL;
            FILE_LOG(logERROR, ("\tCould not set or clear ROIs\n"));
        }
        // update databytes (now that mask is up to date from fpga) and allocate ram
        if (allocateRAM() == FAIL) {
            *ret = FAIL;
            nROI = -2;
        }
    }

    *retvalsize = nROI;
    return rois;
}


/* parameters - speed, readout */

void setSpeed(enum speedVariable ind, int val) {
    switch(ind) {
    case ADC_PHASE:
    case PHASE_SHIFT:
        FILE_LOG(logINFO, ("Configuring ADC Phase\n"));
        configurePhase(RUN_CLK, val);
        break;
    case DBIT_PHASE:
        FILE_LOG(logINFO, ("Configuring Dbit Phase\n"));
        configurePhase(DBIT_CLK, val);
        break;
    case ADC_CLOCK:
        FILE_LOG(logINFO, ("Configuring ADC Clock\n"));
        configureFrequency(ADC_CLK, val);
        configureSyncFrequency(ADC_CLK);
        break;
    case DBIT_CLOCK:
        FILE_LOG(logINFO, ("Configuring Dbit Clock\n"));
        configureFrequency(DBIT_CLK, val);
        configureSyncFrequency(DBIT_CLK);
        break;
    case ADC_PIPELINE:
        setAdcOffsetRegister(1, val);
        break;
    case DBIT_PIPELINE:
        setAdcOffsetRegister(0, val);
        break;
    case CLOCK_DIVIDER:
        FILE_LOG(logINFO, ("Configuring Run Clock\n"));
        configureFrequency(RUN_CLK, val);
        configureSyncFrequency(RUN_CLK);
        break;
    default:
        return;
    }
}

int getSpeed(enum speedVariable ind) {
    switch(ind) {
    case ADC_PHASE:
    case PHASE_SHIFT:
        return getPhase(RUN_CLK);
    case DBIT_PHASE:
        return clkPhase[DBIT_CLK];
    case ADC_CLOCK:
        return getFrequency(ADC_CLK);
    case DBIT_CLOCK:
        return getFrequency(DBIT_CLK);
    case ADC_PIPELINE:
        return getAdcOffsetRegister(1);
    case DBIT_PIPELINE:
        return getAdcOffsetRegister(0);
    case CLOCK_DIVIDER:
        return getFrequency(RUN_CLK);
    default:
        return -1;
    }
}

enum  readOutFlags setReadOutFlags(enum readOutFlags val) {
    enum readOutFlags retval = GET_READOUT_FLAGS;
    uint32_t addr = CONFIG_REG;

    // set
    if (val != GET_READOUT_FLAGS) {
        switch(val) {
        case NORMAL_READOUT:
            FILE_LOG(logINFO, ("Setting Normal Readout\n"));
            bus_w(addr, bus_r(addr) & (~CONFIG_DSBL_ANLG_OTPT_MSK) & (~CONFIG_ENBLE_DGTL_OTPT_MSK));
            break;
        case DIGITAL_ONLY:
            FILE_LOG(logINFO, ("Setting Digital Only Readout\n"));
            bus_w(addr, bus_r(addr) | CONFIG_DSBL_ANLG_OTPT_MSK | CONFIG_ENBLE_DGTL_OTPT_MSK);
            break;
        case ANALOG_AND_DIGITAL:
            FILE_LOG(logINFO, ("Setting Analog & Digital Readout\n"));
            bus_w(addr, (bus_r(addr) & (~CONFIG_DSBL_ANLG_OTPT_MSK)) | CONFIG_ENBLE_DGTL_OTPT_MSK);
            break;
        default:
            FILE_LOG(logERROR, ("Cannot set unknown readout flag. 0x%x\n", val));
            return retval;
        }
    }

    // get
    uint32_t regval = bus_r(addr);
    FILE_LOG(logDEBUG1, ("Config Reg: 0x%08x\n", regval));
    // this bit reads analog disable, so inverse
    analogEnable = (((regval & CONFIG_DSBL_ANLG_OTPT_MSK) >> CONFIG_DSBL_ANLG_OTPT_OFST) ? 0 : 1);
    digitalEnable = ((regval & CONFIG_ENBLE_DGTL_OTPT_MSK) >> CONFIG_ENBLE_DGTL_OTPT_OFST);

    if (analogEnable && digitalEnable) {
        FILE_LOG(logDEBUG1, ("Getting readout: Analog & Digital\n"));
        retval = ANALOG_AND_DIGITAL;
    } else if (analogEnable && !digitalEnable) {
        FILE_LOG(logDEBUG1, ("Getting readout: Normal\n"));
        retval = NORMAL_READOUT;
    } else if (!analogEnable && digitalEnable) {
        FILE_LOG(logDEBUG1, ("Getting readout: Digital Only\n"));
        retval = DIGITAL_ONLY;
    } else {
        FILE_LOG(logERROR, ("Read unknown readout (Both digital and analog are disabled). "
                "Config reg: 0x%x\n", regval));
        return retval;
    }

    // update databytes and allocate ram
    if (allocateRAM() == FAIL) {
        return -2;
    }

    return retval;
}



/* parameters - timer */
int64_t setTimer(enum timerIndex ind, int64_t val) {

	int64_t retval = -1;
	switch(ind){

	case FRAME_NUMBER:
		if(val >= 0) {
			FILE_LOG(logINFO, ("Setting #frames: %lld\n",(long long int)val));
		}
		retval = set64BitReg(val,  FRAMES_LSB_REG, FRAMES_MSB_REG);
		FILE_LOG(logINFO, ("\tGetting #frames: %lld\n", (long long int)retval));
		break;

	case FRAME_PERIOD:
		if(val >= 0){
			FILE_LOG(logINFO, ("Setting period: %lldns\n",(long long int)val));
			val *= (1E-3 * clkDivider[ADC_CLK]);
			// make period odd
			//FIXME to be tested
			/*if (val % 2 == 0) { //fIXME: period is even here, not other way round?
			    FILE_LOG(logINFO, ("\tPeriod %lld not even, adding 1\n", (long long int)val));
			    ++val;
			    FILE_LOG(logINFO, ("\tNew Period:%lld\n", (long long int)val))
			} else {
			    FILE_LOG(logINFO, ("\tPeriod already even:%lld\n", (long long int)val))
			}*/
		}
		retval = set64BitReg(val, PERIOD_LSB_REG, PERIOD_MSB_REG )/ (1E-3 * clkDivider[ADC_CLK]);
		FILE_LOG(logINFO, ("\tGetting period: %lldns\n", (long long int)retval));
		break;

	case DELAY_AFTER_TRIGGER:
		if(val >= 0){
			FILE_LOG(logINFO, ("Setting delay: %lldns\n", (long long int)val));
			val *= (1E-3 * clkDivider[ADC_CLK]);
		}
		retval = set64BitReg(val, DELAY_LSB_REG, DELAY_MSB_REG) / (1E-3 * clkDivider[ADC_CLK]);
		FILE_LOG(logINFO, ("\tGetting delay: %lldns\n", (long long int)retval));
		break;

	case CYCLES_NUMBER:
		if(val >= 0) {
			FILE_LOG(logINFO, ("Setting #cycles: %lld\n", (long long int)val));
		}
		retval = set64BitReg(val,  CYCLES_LSB_REG, CYCLES_MSB_REG);
		FILE_LOG(logINFO, ("\tGetting #cycles: %lld\n", (long long int)retval));
		break;

	case SAMPLES:
	    if(val >= 0) {
	        FILE_LOG(logINFO, ("Setting #samples: %lld\n", (long long int)val));
	        nSamples = val;
	        bus_w(SAMPLES_REG, val);
	        if (allocateRAM() == FAIL) {
	            return -1;
	        }
	    }
        retval = nSamples;
        FILE_LOG(logINFO, ("\tGetting #samples: %lld\n", (long long int)retval));

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
		retval = get64BitReg(FRAMES_LEFT_LSB_REG, FRAMES_LEFT_MSB_REG);
		FILE_LOG(logINFO, ("Getting number of frames left: %lld\n",(long long int)retval));
		break;

    case FRAME_PERIOD:
        retval = get64BitReg(PERIOD_LEFT_LSB_REG, PERIOD_LEFT_MSB_REG) / (1E-3 * clkDivider[ADC_CLK]);
        FILE_LOG(logINFO, ("Getting period left: %lldns\n", (long long int)retval));
        break;

	case DELAY_AFTER_TRIGGER:
		retval = get64BitReg(DELAY_LEFT_LSB_REG, DELAY_LEFT_MSB_REG) / (1E-3 * clkDivider[ADC_CLK]);
		FILE_LOG(logINFO, ("Getting delay left: %lldns\n", (long long int)retval));
		break;

	case CYCLES_NUMBER:
		retval = get64BitReg(CYCLES_LEFT_LSB_REG, CYCLES_LEFT_MSB_REG);
		FILE_LOG(logINFO, ("Getting number of cycles left: %lld\n", (long long int)retval));
		break;

	case ACTUAL_TIME:
		retval = get64BitReg(TIME_FROM_START_LSB_REG, TIME_FROM_START_MSB_REG) / (1E-3 * CLK_FREQ);
		FILE_LOG(logINFO, ("Getting actual time (time from start): %lld\n", (long long int)retval));
		break;

	case MEASUREMENT_TIME:
		retval = get64BitReg(START_FRAME_TIME_LSB_REG, START_FRAME_TIME_MSB_REG) / (1E-3 * CLK_FREQ);
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
    case FRAME_PERIOD:
    case DELAY_AFTER_TRIGGER:
        // convert to freq
        val *= (1E-3 * ADC_CLK);
        // convert back to timer
        val = (val) / (1E-3 * ADC_CLK);
        if (val != retval)
            return FAIL;
    default:
        break;
    }
    return OK;
}


/* parameters - settings */
enum detectorSettings getSettings() {
    return UNDEFINED;
}

/* parameters - dac, adc, hv */


void setDAC(enum DACINDEX ind, int val, int mV) {
    if (val < 0 && val != LTC2620_PWR_DOWN_VAL)
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
    if (LTC2620_SetDACValue((int)ind, val, mV, &dacval) == OK)
        dacValues[ind] = dacval;
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

int dacToVoltage(int dac) {
    int val;
    LTC2620_DacToVoltage(dac, &val);
    return val;
}

int checkVLimitCompliant(int mV) {
    if (vLimit > 0 && mV > vLimit)
        return FAIL;
    return OK;
}

int checkVLimitDacCompliant(int dac) {
    if (vLimit > 0) {
        int mv = 0;
        // could not convert
        if (LTC2620_DacToVoltage(dac, &mv) == FAIL)
            return FAIL;
        if (mv > vLimit)
            return FAIL;
    }
    return OK;
}

int getVLimit() {
    return vLimit;
}

void setVLimit(int l) {
    if (l >= 0)
        vLimit = l;
}

int isVchipValid(int val) {
    if (val < VCHIP_MIN_MV || val > VCHIP_MAX_MV) {
        return 0;
    }
    return 1;
}

int getVchip() {
    // not set yet
    if (dacValues[D_PWR_CHIP] == -1 || dacValues[D_PWR_CHIP] == LTC2620_PWR_DOWN_VAL)
        return dacValues[D_PWR_CHIP];
    int voltage = -1;
    // dac to voltage
    ConvertToDifferentRange(LTC2620_MAX_VAL, LTC2620_MIN_VAL, VCHIP_MIN_MV, VCHIP_MAX_MV,
            dacValues[D_PWR_CHIP], &voltage);
    return voltage;
}

void setVchip(int val) {
    // set vchip
    if (val != -1) {
        FILE_LOG(logINFOBLUE, ("Setting Vchip to %d mV\n", val));

        int dacval = LTC2620_PWR_DOWN_VAL;

        // validate & convert it to dac
        if (val != LTC2620_PWR_DOWN_VAL) {
            // convert voltage to dac
            if (ConvertToDifferentRange(VCHIP_MIN_MV, VCHIP_MAX_MV, LTC2620_MAX_VAL, LTC2620_MIN_VAL, //min val is max V
                    val, &dacval) == FAIL) {
                FILE_LOG(logERROR, ("\tVChip %d mV invalid. Is not between %d and %d mV\n", val, VCHIP_MIN_MV, VCHIP_MAX_MV));
                return;
            }
        }
        FILE_LOG(logINFO, ("Setting Vchip (DAC %d): %d dac (%d mV)\n",D_PWR_CHIP, dacval, val));
        // set
        setDAC(D_PWR_CHIP, dacval, 0);
    }
}

int getVChipToSet(enum DACINDEX ind, int val) {
    FILE_LOG(logDEBUG1, ("Calculating vchip to set\n"));
    // validate index & get adc index
    int adcIndex = getADCIndexFromDACIndex(ind);
    if (adcIndex == -1) {
        return -1;
    }

    // get maximum value of the adc values (minimum is 0)
    int max = 0;

    int ipwr = 0;
    // loop through the adcs
    for (ipwr = 0; ipwr < NPWR -1; ++ipwr) {

        // get the dac values for each adc
        int dacmV = getPower(getDACIndexFromADCIndex(ipwr));

        // if current index, replace with value to be set
        if (ipwr == adcIndex) {
            dacmV = val;
        }

        // update max
        max = (dacmV > max) ? dacmV : max;
    }

    // increment to get vchip value
    max += VCHIP_POWER_INCRMNT;

    // validate with vchip minimum value
    if (max < VCHIP_MIN_MV)
        max = VCHIP_MIN_MV;
    // with correct calulations, vchip val should never be greater than vchipmax
    if (max > VCHIP_MAX_MV) {
        FILE_LOG(logERROR, ("Vchip value to set %d is beyond its maximum (WEIRD)\n",  max));
        return -1;
    }
    return max;
}

int getDACIndexFromADCIndex(enum ADCINDEX ind) {
    switch (ind) {
    case V_PWR_IO:
        return D_PWR_IO;
    case V_PWR_A:
        return D_PWR_A;
    case V_PWR_B:
        return D_PWR_B;
    case V_PWR_C:
        return D_PWR_C;
    case V_PWR_D:
        return D_PWR_D;
    default:
        FILE_LOG(logERROR, ("ADC index %d is not defined to get DAC index\n", ind));
        return -1;
    }
}

int getADCIndexFromDACIndex(enum DACINDEX ind) {
    switch (ind) {
    case D_PWR_IO:
        return V_PWR_IO;
    case D_PWR_A:
        return V_PWR_A;
    case D_PWR_B:
        return V_PWR_B;
    case D_PWR_C:
        return V_PWR_C;
    case D_PWR_D:
        return V_PWR_D;
    default:
        FILE_LOG(logERROR, ("DAC index %d is not defined to get ADC index\n", ind));
        return -1;
    }
}

int isPowerValid(enum DACINDEX ind, int val) {
    int min = (ind == D_PWR_IO) ? VIO_MIN_MV : POWER_RGLTR_MIN;

    // not power_rgltr_max because it is allowed only upto vchip max - 200
    if (val != 0 && (val != LTC2620_PWR_DOWN_VAL) && (val < min || val > (VCHIP_MAX_MV - VCHIP_POWER_INCRMNT))) {
        return 0;
    }
    return 1;
}

int getPower(enum DACINDEX ind) {
    // validate index & get adc index
    int adcIndex = getADCIndexFromDACIndex(ind);
    if (adcIndex == -1) {
        return -1;
    }

    // powered enable off
    {
        uint32_t addr = POWER_REG;
        uint32_t offset = POWER_ENBL_VLTG_RGLTR_OFST + adcIndex;
        uint32_t mask = (1 << offset);
        if (!(bus_r(addr) & mask))
            return 0;
    }

    // not set yet
    if (dacValues[ind] == -1) {
        FILE_LOG(logERROR, ("Power enabled, but unknown dac value for power index %d!", ind));
        return -1;
    }

    // dac powered off
    if (dacValues[ind] == LTC2620_PWR_DOWN_VAL) {
        FILE_LOG(logWARNING, ("Power %d enabled, dac value %d, voltage at minimum or 0\n", ind, LTC2620_PWR_DOWN_VAL));
        return LTC2620_PWR_DOWN_VAL;
    }

    // vchip not set, weird error, should not happen (as vchip set to max in the beginning)
    // unless user set vchip to LTC2620_PWR_DOWN_VAL  and then tried to get a power regulator value
    if (dacValues[D_PWR_CHIP] == -1 || dacValues[D_PWR_CHIP] == LTC2620_PWR_DOWN_VAL) {
        FILE_LOG(logERROR, ("Cannot read power regulator %d (vchip not set)."
                "Set a power regulator, which will also set vchip.\n"));
        return -1;
    }

    // convert dac to voltage
    int retval = -1;
    ConvertToDifferentRange(LTC2620_MAX_VAL, LTC2620_MIN_VAL, POWER_RGLTR_MIN, POWER_RGLTR_MAX,
            dacValues[ind], &retval);
    return retval;
}

void setPower(enum DACINDEX ind, int val) {
    // validate index & get adc index
    int adcIndex = getADCIndexFromDACIndex(ind);
    if (adcIndex == -1) {
        return;
    }

    uint32_t addr = POWER_REG;
    uint32_t offset = POWER_ENBL_VLTG_RGLTR_OFST + adcIndex;
    uint32_t mask = (1 << offset);

    // set power
    if (val != -1) {
        FILE_LOG(logINFO, ("Setting Power to %d mV\n", val));

        // validate value (already checked at tcp)
        if (!isPowerValid(ind, val)) {
            FILE_LOG(logERROR, ("Invalid value of %d mV for Power %d. Is not between %d and %d mV\n",
                    val, ind, (ind == D_PWR_IO ? VIO_MIN_MV : POWER_RGLTR_MIN), POWER_RGLTR_MAX));
            return;
        }

        // get vchip to set vchip (calculated now before switching off power enable)
        int vchip = getVChipToSet(ind, val);
        FILE_LOG(logDEBUG1, ("Vchip to set: %d\n", vchip));
        // index problem of vchip calculation problem
        if (vchip == -1)
            return;

        // Switch off power enable
        FILE_LOG(logDEBUG1, ("Switching off power enable\n"));
        bus_w(addr, bus_r(addr) & ~(mask));

        // power down dac
        FILE_LOG(logDEBUG1, ("Powering off P%d (DAC %d)\n", adcIndex, ind));
        setDAC(ind, LTC2620_PWR_DOWN_VAL, 0);

        // set vchip
        setVchip(vchip);
        if (getVchip() != vchip) {
            FILE_LOG(logERROR, ("Weird, Could not set vchip. Set %d, read %d\n.", vchip, getVchip()));
            return;
        }

        //(power off is anyway done with power enable)
        if (val == 0)
            val = LTC2620_PWR_DOWN_VAL;

        // convert it to dac (power off is anyway done with power enable)
        if (val != LTC2620_PWR_DOWN_VAL) {
            FILE_LOG(logDEBUG1, ("Convert Power of %d mV to dac units\n", val));

            int dacval = -1;
            // convert voltage to dac
            if (ConvertToDifferentRange(POWER_RGLTR_MIN, POWER_RGLTR_MAX, LTC2620_MAX_VAL, LTC2620_MIN_VAL,
                    val, &dacval) == FAIL) {
                FILE_LOG(logERROR, ("\tPower index %d of value %d mV invalid. Is not between %d and %d mV\n",
                        ind, val, POWER_RGLTR_MIN, vchip - VCHIP_POWER_INCRMNT));
                return;
            }

            // set and power on/ update dac
            FILE_LOG(logINFO, ("Setting P%d (DAC %d): %d dac (%d mV)\n", adcIndex, ind, dacval, val));
            setDAC(ind, dacval, 0);

            // to be sure of valid conversion
            if (dacval >= 0) {
                FILE_LOG(logDEBUG1, ("Switching on power enable\n"));
                bus_w(addr, bus_r(addr) | mask);
            }
        }
    }
}

void powerOff() {
    uint32_t addr = POWER_REG;
    FILE_LOG(logINFO, ("Powering off all voltage regulators\n"));
    bus_w(addr, bus_r(addr) & (~POWER_ENBL_VLTG_RGLTR_MSK));
    FILE_LOG(logDEBUG1, ("Power Register: 0x%08x\n", bus_r(addr)));
}


int getADC(enum ADCINDEX ind){
#ifdef VIRTUAL
    return 0;
#endif
    switch(ind) {
    case V_PWR_IO:
    case V_PWR_A:
    case V_PWR_B:
    case V_PWR_C:
    case V_PWR_D:
        FILE_LOG(logDEBUG1, ("Reading I2C Voltage for device Id: %d\n", (int)ind));
        return INA226_ReadVoltage(I2C_POWER_VIO_DEVICE_ID + (int)ind);
    case I_PWR_IO:
    case I_PWR_A:
    case I_PWR_B:
    case I_PWR_C:
    case I_PWR_D:
        FILE_LOG(logDEBUG1, ("Reading I2C Current for device Id: %d\n", (int)ind));
        return INA226_ReadCurrent(I2C_POWER_VIO_DEVICE_ID + (int)(ind - I_PWR_IO));

        // slow adcs
    case SLOW_ADC_TEMP:
        FILE_LOG(logDEBUG1, ("Reading Slow ADC Temperature\n"));
        return AD7689_GetTemperature();
    case SLOW_ADC0:
    case SLOW_ADC1:
    case SLOW_ADC2:
    case SLOW_ADC3:
    case SLOW_ADC4:
    case SLOW_ADC5:
    case SLOW_ADC6:
    case SLOW_ADC7:
        FILE_LOG(logDEBUG1, ("Reading Slow ADC Channel %d\n", (int)ind - SLOW_ADC0));
        return AD7689_GetChannel((int)ind - SLOW_ADC0);
    default:
        FILE_LOG(logERROR, ("Adc Index %d not defined \n", (int)ind));
        return -1;
    }
}


int setHighVoltage(int val){
#ifdef VIRTUAL
    if (val >= 0)
        highvoltage = val;
    return highvoltage;
#endif

	// setting hv
	if (val >= 0) {
	    FILE_LOG(logINFO, ("Setting High voltage: %d V\n", val));
		uint32_t addr = POWER_REG;

		// switch off high voltage
		bus_w(addr, bus_r(addr) & (~POWER_HV_SLCT_MSK));

		MAX1932_Set(val);

		// switch on high voltage if val > 0
		if (val > 0)
		    bus_w(addr, bus_r(addr) | POWER_HV_SLCT_MSK);

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



int configureMAC(uint32_t destip, uint64_t destmac, uint64_t sourcemac, uint32_t sourceip, uint32_t udpport, uint32_t udpport2){
#ifdef VIRTUAL
	return OK;
#endif
	FILE_LOG(logINFOBLUE, ("Configuring MAC\n"));
	// 1 giga udp
	if (!enableTenGigabitEthernet(-1)) {
		// if it was in 10G mode, it was not allocating RAM
		if (allocateRAM() == FAIL)
			return -1;
		char cDestIp[MAX_STR_LENGTH];
		memset(cDestIp, 0, MAX_STR_LENGTH);
		sprintf(cDestIp, "%d.%d.%d.%d", (destip>>24)&0xff,(destip>>16)&0xff,(destip>>8)&0xff,(destip)&0xff);
		FILE_LOG(logINFO, ("1G UDP: Destination (IP: %s, port:%d)\n", cDestIp, udpport));
		if (setUDPDestinationDetails(cDestIp, udpport) == FAIL) {
			FILE_LOG(logERROR, ("could not set udp 1G destination IP and port\n"));
			return FAIL;
		}
		return OK;
	}

	// 10 G
	else {
		uint32_t sourceport  =  DEFAULT_TX_UDP_PORT;

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

		FILE_LOG(logINFO, ("\tDest. IP    : %d.%d.%d.%d \t\t(0x%08x)\n",
				(destip>>24)&0xff,(destip>>16)&0xff,(destip>>8)&0xff,(destip)&0xff, destip));
		FILE_LOG(logINFO, ("\tDest. MAC   : %02x:%02x:%02x:%02x:%02x:%02x \t(0x%010llx)\n",
				(unsigned int)((destmac>>40)&0xFF),
				(unsigned int)((destmac>>32)&0xFF),
				(unsigned int)((destmac>>24)&0xFF),
				(unsigned int)((destmac>>16)&0xFF),
				(unsigned int)((destmac>>8)&0xFF),
				(unsigned int)((destmac>>0)&0xFF),
				(long  long unsigned int)destmac));
		FILE_LOG(logINFO, ("\tDest. Port  : %d \t\t\t(0x%08x)\n",udpport, udpport));

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

		cleanFifos();//FIXME: resetPerpheral() for ctb?
		resetPeripheral();
		usleep(WAIT_TIME_CONFIGURE_MAC); // todo maybe without
	}

	return OK;
}

int enableTenGigabitEthernet(int val) {
	uint32_t addr = CONFIG_REG;

	// set
	if (val != -1) {
		FILE_LOG(logINFO, ("Setting 10Gbe: %d\n", (val > 0) ? 1 : 0));
		if (val > 0) {
			bus_w(addr, bus_r(addr) | CONFIG_GB10_SND_UDP_MSK);
		} else {
			bus_w(addr, bus_r(addr) & (~CONFIG_GB10_SND_UDP_MSK));
		}
		//configuremac called from client
	}
	return  ((bus_r(addr) & CONFIG_GB10_SND_UDP_MSK) >> CONFIG_GB10_SND_UDP_OFST);
}




/* ctb specific - pll, flashing fpga */


// ind can only be ADC_CLK or DBIT_CLK
void configurePhase(enum CLKINDEX ind, int val) {
    if (val > 65535 || val < -65535) {
        FILE_LOG(logERROR, ("\tPhase provided outside limits\n"));
        return;
    }

    FILE_LOG(logINFO, ("Configuring Phase of C%d to %d\n", ind, val));

    // reset only pll
    ALTERA_PLL_ResetPLL();

    // set mode register to polling mode
    ALTERA_PLL_SetModePolling();

    int phase = 0, inv = 0;
    if (val > 0) {
        inv = 0;
        phase = val;
    } else {
        inv = 1;
        val = -1 * val;
        phase = (~val);
    }
    FILE_LOG(logINFO, ("\tphase out %d (0x%08x), inv:%d\n", phase, phase, inv));

    ALTERA_PLL_SetPhaseShift(phase, (int)ind, 0);

    clkPhase[ind] = val;
}

int getPhase(enum CLKINDEX ind) {
    return clkPhase[ind];
}

void configureFrequency(enum CLKINDEX ind, int val) {
    if (val < 0)
        return;

    FILE_LOG(logINFO, ("\tConfiguring Frequency of C%d to %d MHz\n", ind, val));

    // check adc clk too high
    if (ind == ADC_CLK && val > MAXIMUM_ADC_CLK) {
        FILE_LOG(logERROR, ("Frequency %d MHz too high for ADC\n", val));
        return;
    }

    // Calculate and set output frequency
    clkDivider[ind] = ALTERA_PLL_SetOuputFrequency (ind, PLL_VCO_FREQ_MHZ, val);
    FILE_LOG(logINFO, ("\tC%d: Frequency set to %d MHz\n", ind, clkDivider[ind]));
}

int getFrequency(enum CLKINDEX ind) {
    return clkDivider[ind];
}

void configureSyncFrequency(enum CLKINDEX ind) {
    int clka = 0, clkb = 0;
    switch(ind) {
    case ADC_CLOCK:
        clka = DBIT_CLK;
        clkb = RUN_CLK;
        break;
    case DBIT_CLOCK:
        clka = ADC_CLK;
        clkb = RUN_CLK;
        break;
    case CLOCK_DIVIDER:
        clka = DBIT_CLK;
        clkb = ADC_CLK;
        break;
    default:
        return;
    }

    int clksync = getFrequency(SYNC_CLK);
    int retval = getFrequency(ind);
    int configure = 0;

    // sync is greater than current
    if (clksync > retval)  {
        FILE_LOG(logINFO, ("\t--Configuring Sync Clock\n"));
        configure = 1;
    }

    // the others are both greater than current
    else if ((clka > retval && clkb > retval)) {
        FILE_LOG(logINFO, ("\t++Configuring Sync Clock\n"));
        configure = 1;
    }

    // configure sync to current
    if (configure)
        configureFrequency(SYNC_CLK, retval);
}

void setAdcOffsetRegister(int adc, int val) {
    if (val < 0)
        return;

    FILE_LOG(logINFO, ("Setting %s Pipeline to %d\n", (adc ? "ADC" : "Dbit"), val));
    uint32_t offset = ADC_OFFSET_ADC_PPLN_OFST;
    uint32_t mask = ADC_OFFSET_ADC_PPLN_MSK;
    if (!adc) {
        offset = ADC_OFFSET_DBT_PPLN_OFST;
        mask = ADC_OFFSET_DBT_PPLN_MSK;
    }

    uint32_t addr = ADC_OFFSET_REG;
    // reset value
    bus_w(addr, bus_r(addr) & ~ mask);
    // set value
    bus_w(addr, bus_r(addr) | ((val << offset) & mask));
    FILE_LOG(logDEBUG1, (" %s Offset: 0x%8x\n", (adc ? "ADC" : "Dbit"), bus_r(addr)));
}

int getAdcOffsetRegister(int adc) {
    if (adc)
        return ((bus_r(ADC_OFFSET_REG) & ADC_OFFSET_ADC_PPLN_MSK) >> ADC_OFFSET_ADC_PPLN_OFST);
    return ((bus_r(ADC_OFFSET_REG) & ADC_OFFSET_DBT_PPLN_MSK) >> ADC_OFFSET_DBT_PPLN_OFST);
}

uint64_t writePatternIOControl(uint64_t word) {
    if (word != -1) {
        FILE_LOG(logINFO, ("Setting Pattern - I/O Control: 0x%llx\n", (long long int) word));
        set64BitReg(word, PATTERN_IO_CNTRL_LSB_REG, PATTERN_IO_CNTRL_MSB_REG);
    }
    uint64_t retval = get64BitReg(PATTERN_IO_CNTRL_LSB_REG, PATTERN_IO_CNTRL_MSB_REG);
    FILE_LOG(logDEBUG1, ("  I/O Control: 0x%llx\n", (long long int) retval));
    return retval;
}

uint64_t writePatternClkControl(uint64_t word) {
    if (word != -1) {
        FILE_LOG(logINFO, ("Setting Pattern - Clock Control: 0x%llx\n", (long long int) word));
        set64BitReg(word, PATTERN_IO_CLK_CNTRL_LSB_REG, PATTERN_IO_CLK_CNTRL_MSB_REG);
    }
    uint64_t retval = get64BitReg(PATTERN_IO_CLK_CNTRL_LSB_REG, PATTERN_IO_CLK_CNTRL_MSB_REG);
    FILE_LOG(logDEBUG1, ("  Clock Control: 0x%llx\n", (long long int) retval));
    return retval;
}

uint64_t readPatternWord(int addr) {
    // error (handled in tcp)
    if (addr < 0 || addr >= MAX_PATTERN_LENGTH) {
        FILE_LOG(logERROR, ("Cannot get Pattern - Word. Invalid addr 0x%x. "
                "Should be within 0x%x\n", addr, MAX_PATTERN_LENGTH));
        return -1;
    }

    FILE_LOG(logDEBUG1, ("  Reading Pattern - Word (addr:0x%x)\n", addr));
    uint32_t reg = PATTERN_CNTRL_REG;

    // overwrite with  only addr
    bus_w(reg, ((addr << PATTERN_CNTRL_ADDR_OFST) & PATTERN_CNTRL_ADDR_MSK));

    // set read strobe
    bus_w(reg, bus_r(reg) | PATTERN_CNTRL_RD_MSK);

    // unset read strobe
    bus_w(reg, bus_r(reg) & (~PATTERN_CNTRL_RD_MSK));

    // read value
    uint64_t retval = get64BitReg(PATTERN_OUT_LSB_REG, PATTERN_OUT_MSB_REG);
    FILE_LOG(logDEBUG1, ("  Word(addr:0x%x): 0x%llx\n", addr, (long long int) retval));

    return retval;
}

uint64_t writePatternWord(int addr, uint64_t word) {
    // get
    if (word == -1)
        return readPatternWord(addr);

    // error (handled in tcp)
    if (addr < 0 || addr >= MAX_PATTERN_LENGTH) {
        FILE_LOG(logERROR, ("Cannot set Pattern - Word. Invalid addr 0x%x. "
                "Should be within 0x%x\n", addr, MAX_PATTERN_LENGTH));
        return -1;
    }

    FILE_LOG(logINFO, ("Setting Pattern - Word (addr:0x%x, word:0x%llx)\n", addr, (long long int) word));
    uint32_t reg = PATTERN_CNTRL_REG;

    // write word
    set64BitReg(word, PATTERN_IN_LSB_REG, PATTERN_IN_MSB_REG);
    FILE_LOG(logDEBUG1, ("  Wrote word. PatternIn Reg: 0x%llx\n", get64BitReg(PATTERN_IN_LSB_REG, PATTERN_IN_MSB_REG)));

    // overwrite with  only addr
    bus_w(reg, ((addr << PATTERN_CNTRL_ADDR_OFST) & PATTERN_CNTRL_ADDR_MSK));

    // set write strobe
    bus_w(reg, bus_r(reg) | PATTERN_CNTRL_WR_MSK);

    // unset write strobe
    bus_w(reg, bus_r(reg) & (~PATTERN_CNTRL_WR_MSK));

    return readPatternWord(addr);
}

int setPatternWaitAddress(int level, int addr) {

    // error (handled in tcp)
    if (addr >= MAX_PATTERN_LENGTH) {
        FILE_LOG(logERROR, ("Cannot set Pattern - Wait Address. Invalid addr 0x%x. "
                "Should be within 0x%x\n", addr, MAX_PATTERN_LENGTH));
        return -1;
    }

    uint32_t reg = 0;
    uint32_t offset = 0;
    uint32_t mask = 0;

    switch (level) {
    case 0:
        reg = PATTERN_WAIT_0_ADDR_REG;
        offset = PATTERN_WAIT_0_ADDR_OFST;
        mask = PATTERN_WAIT_0_ADDR_MSK;
        break;
    case 1:
        reg = PATTERN_WAIT_1_ADDR_REG;
        offset = PATTERN_WAIT_1_ADDR_OFST;
        mask = PATTERN_WAIT_1_ADDR_MSK;
        break;
    case 2:
        reg = PATTERN_WAIT_2_ADDR_REG;
        offset = PATTERN_WAIT_2_ADDR_OFST;
        mask = PATTERN_WAIT_2_ADDR_MSK;
        break;
    default:
        FILE_LOG(logERROR, ("Cannot set Pattern - Wait Address. Invalid level 0x%x. "
                "Should be between 0 and 2.\n", level));
        return -1;
    }

    // set
    if (addr >= 0) {
        FILE_LOG(logINFO, ("Setting Pattern - Wait Address (level:%d, addr:0x%x)\n", level, addr));
        bus_w(reg, ((addr << offset) & mask));
    }

    // get
    uint32_t regval = bus_r((reg & mask) >> offset);
    FILE_LOG(logDEBUG1, ("  Wait Address (level:%d, addr:0x%x)\n", level, regval));
    return regval;
}

uint64_t setPatternWaitTime(int level, uint64_t t) {
    uint32_t regl = 0;
    uint32_t regm = 0;

    switch (level) {
    case 0:
        regl = PATTERN_WAIT_TIMER_0_LSB_REG;
        regm = PATTERN_WAIT_TIMER_0_MSB_REG;
        break;
    case 1:
        regl = PATTERN_WAIT_TIMER_1_LSB_REG;
        regm = PATTERN_WAIT_TIMER_1_MSB_REG;
        break;
    case 2:
        regl = PATTERN_WAIT_TIMER_2_LSB_REG;
        regm = PATTERN_WAIT_TIMER_2_MSB_REG;
        break;
    default:
        FILE_LOG(logERROR, ("Cannot set Pattern - Wait Time. Invalid level %d. "
                "Should be between 0 and 2.\n", level));
        return -1;
    }

    // set
    if (t >= 0) {
        FILE_LOG(logINFO, ("Setting Pattern - Wait Time (level:%d, t:%lld)\n", level, (long long int)t));
        set64BitReg(t, regl, regm);
    }

    // get
    uint64_t regval = get64BitReg(regl, regm);
    FILE_LOG(logDEBUG1, ("  Wait Time (level:%d, t:%lld)\n", level, (long long int)regval));
    return regval;
}

void setPatternLoop(int level, int *startAddr, int *stopAddr, int *nLoop) {

    // level 0-2, addr upto patternlength + 1 (checked at tcp)
    if ((level != -1) &&
            (*startAddr > MAX_PATTERN_LENGTH || *stopAddr > MAX_PATTERN_LENGTH)) {
        FILE_LOG(logERROR, ("Cannot set Pattern (Pattern Loop, level:%d, startaddr:0x%x, stopaddr:0x%x). "
                "Addr must be less than 0x%x\n",
                level, *startAddr, *stopAddr, MAX_PATTERN_LENGTH));
    }

    //level -1, addr upto patternlength (checked at tcp)
    else if ((level == -1) &&
            (*startAddr > MAX_PATTERN_LENGTH || *stopAddr > MAX_PATTERN_LENGTH)) {
        FILE_LOG(logERROR, ("Cannot set Pattern (Pattern Loop, complete pattern, startaddr:0x%x, stopaddr:0x%x). "
                "Addr must be less than 0x%x\n",
                *startAddr, *stopAddr, MAX_PATTERN_LENGTH));
    }

    uint32_t addr = 0;
    uint32_t nLoopReg = 0;
    uint32_t startOffset = 0;
    uint32_t startMask = 0;
    uint32_t stopOffset = 0;
    uint32_t stopMask = 0;

    switch (level) {
    case 0:
        addr = PATTERN_LOOP_0_ADDR_REG;
        nLoopReg = PATTERN_LOOP_0_ITERATION_REG;
        startOffset = PATTERN_LOOP_0_ADDR_STRT_OFST;
        startMask = PATTERN_LOOP_0_ADDR_STRT_MSK;
        stopOffset = PATTERN_LOOP_0_ADDR_STP_OFST;
        stopMask = PATTERN_LOOP_0_ADDR_STP_MSK;
        break;
    case 1:
        addr = PATTERN_LOOP_1_ADDR_REG;
        nLoopReg = PATTERN_LOOP_1_ITERATION_REG;
        startOffset = PATTERN_LOOP_1_ADDR_STRT_OFST;
        startMask = PATTERN_LOOP_1_ADDR_STRT_MSK;
        stopOffset = PATTERN_LOOP_1_ADDR_STP_OFST;
        stopMask = PATTERN_LOOP_1_ADDR_STP_MSK;
        break;
    case 2:
        addr = PATTERN_LOOP_2_ADDR_REG;
        nLoopReg = PATTERN_LOOP_2_ITERATION_REG;
        startOffset = PATTERN_LOOP_2_ADDR_STRT_OFST;
        startMask = PATTERN_LOOP_2_ADDR_STRT_MSK;
        stopOffset = PATTERN_LOOP_2_ADDR_STP_OFST;
        stopMask = PATTERN_LOOP_2_ADDR_STP_MSK;
        break;
    case -1:
        // complete pattern
        addr = PATTERN_LIMIT_REG;
        nLoopReg = -1;
        startOffset = PATTERN_LIMIT_STRT_OFST;
        startMask = PATTERN_LIMIT_STRT_MSK;
        stopOffset = PATTERN_LIMIT_STP_OFST;
        stopMask = PATTERN_LIMIT_STP_MSK;
        break;
    default:
        // already checked at tcp interface
        FILE_LOG(logERROR, ("Cannot set Pattern - Pattern loop. Invalid level %d. "
                "Should be between -1 and 2.\n", level));
        *startAddr = 0;
        *stopAddr = 0;
        *nLoop = 0;
    }

    // set iterations
    if (level >= 0) {
        // set iteration
        if (*nLoop >= 0) {
            FILE_LOG(logINFO, ("Setting Pattern - Pattern Loop (level:%d, nLoop:%d)\n",
                      level, *nLoop));
            bus_w(nLoopReg, *nLoop);
        }
        *nLoop = bus_r(nLoopReg);
    }

    // set
    if (*startAddr != -1 && *stopAddr != -1) {
    	// writing start and stop addr
    	FILE_LOG(logINFO, ("Setting Pattern - Pattern Loop (level:%d, startaddr:0x%x, stopaddr:0x%x)\n",
    			level, *startAddr, *stopAddr));
    	bus_w(addr, ((*startAddr << startOffset) & startMask) | ((*stopAddr << stopOffset) & stopMask));
    	FILE_LOG(logDEBUG1, ("Addr:0x%x, val:0x%x\n", addr, bus_r(addr)));
    }

    // get
    else {
    	*startAddr = ((bus_r(addr)  & startMask) >> startOffset);
    	FILE_LOG(logDEBUG1, ("Getting Pattern - Pattern Loop Start Address (level:%d, Read startAddr:0x%x)\n",
    			level, *startAddr));

    	*stopAddr = ((bus_r(addr) & stopMask) >> stopOffset);
    	FILE_LOG(logDEBUG1, ("Getting Pattern - Pattern Loop Stop Address (level:%d, Read stopAddr:0x%x)\n",
    			level, *stopAddr));
    }
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
	// 1 giga udp
	if (!enableTenGigabitEthernet(-1)) {
		// create udp socket
		if(createUDPSocket() != OK) {
			return FAIL;
		}
		// update header with modId, detType and version. Reset offset and fnum
		createUDPPacketHeader(udpPacketData, getHardwareSerialNumber());
	}

	FILE_LOG(logINFOBLUE, ("Starting State Machine\n"));
	cleanFifos();
	unsetFifoReadStrobes(); // FIXME: unnecessary to write bus_w(dumm, 0) as it is 0 in the beginnig and the strobes are always unset if set

	//start state machine
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_STRT_ACQSTN_MSK | CONTROL_STRT_EXPSR_MSK);
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_STRT_ACQSTN_MSK & ~CONTROL_STRT_EXPSR_MSK);

	FILE_LOG(logINFO, ("Status Register: %08x\n",bus_r(STATUS_REG)));
	return OK;
}


#ifdef VIRTUAL
void* start_timer(void* arg) {
	int wait_in_s = 	(setTimer(FRAME_NUMBER, -1) *
						setTimer(CYCLES_NUMBER, -1) *
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
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_STP_ACQSTN_MSK);
	usleep(WAIT_TIME_US_STP_ACQ);
	bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_STP_ACQSTN_MSK);

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

	uint32_t retval = bus_r(STATUS_REG);
	FILE_LOG(logINFO, ("Status Register: %08x\n",retval));

	// error
	if (retval & STATUS_SM_FF_FLL_MSK) {
	    FILE_LOG(logINFORED, ("Status: Error (Some fifo full)\n"));
	    return ERROR;
	}

	// running
	if(retval & STATUS_RN_BSY_MSK) {
	    if (retval & STATUS_WTNG_FR_TRGGR_MSK) {
	        FILE_LOG(logINFOBLUE, ("Status: Waiting for Trigger\n"));
	        return WAITING;
	    }

	    FILE_LOG(logINFOBLUE, ("Status: Running\n"));
	    return RUNNING;

	}

	// not running
	else {
	    if (retval & STATUS_STPPD_MSK) {
	        FILE_LOG(logINFOBLUE, ("Status: Stopped\n"));
	        return STOPPED;
	    }

	    if (retval & STATUS_FRM_RN_BSY_MSK) {
	        FILE_LOG(logINFOBLUE, ("Status: Transmitting (Read machine busy)\n"));
	        return TRANSMITTING;
	    }

	    /*if (retval & STATUS_ALL_FF_EMPTY_MSK) {
	        FILE_LOG(logINFOBLUE, ("Status: Transmitting (All fifo empty)\n"));
	        return TRANSMITTING;
	    }*/

	    if (! (retval & STATUS_IDLE_MSK)) {
	        FILE_LOG(logINFOBLUE, ("Status: Idle\n"));
	        return IDLE;
	    }

	    FILE_LOG(logERROR, ("Status: Unknown status %08x\n", retval));
	    return ERROR;
	}
}


void readandSendUDPFrames(int *ret, char *mess) {
	FILE_LOG(logDEBUG1, ("Reading from 1G UDP\n"));

	// validate udp socket
	if (getUdPSocketDescriptor() <= 0) {
		*ret = FAIL;
		sprintf(mess,"UDP Socket not created. sockfd:%d\n", getUdPSocketDescriptor());
		FILE_LOG(logERROR, (mess));
		return;
	}

	// every frame read
	while(readFrameFromFifo() == OK) {
		int bytesToSend = 0, n = 0;
		while((bytesToSend = fillUDPPacket(udpPacketData))) {
			n += sendUDPPacket(udpPacketData, bytesToSend);
		}
		if (n >= dataBytes) {
			FILE_LOG(logINFO, (" Frame %lld sent (%d packets, %d databytes, n:%d bytes sent)\n",
					udpFrameNumber, udpPacketNumber + 1, dataBytes, n));
		}
	}
	closeUDPSocket();
}


void readFrame(int *ret, char *mess) {
#ifdef VIRTUAL
	while(virtual_status) {
		//FILE_LOG(logERROR, ("Waiting for finished flag\n");
		usleep(5000);
	}
	return;
#endif
	// 1G
	if (!enableTenGigabitEthernet(-1)) {
		readandSendUDPFrames(ret, mess);
	}
	 // 10G
	else {
		// wait for acquisition to be done
		while(runBusy()){
			usleep(500); // random
		}
	}

	// ret could be fail in 1gudp for not creating udp sockets
	if (*ret != FAIL) {
		// frames left to give status
		int64_t retval = getTimeLeft(FRAME_NUMBER) + 2;
		if ( retval > 1) {
			*ret = (int)FAIL;
			sprintf(mess,"No data and run stopped: %lld frames left\n",(long  long int)retval);
			FILE_LOG(logERROR, (mess));
		} else {
			*ret = (int)OK;
			FILE_LOG(logINFOGREEN, ("Acquisition successfully finished\n"));
		}
	}
}

void unsetFifoReadStrobes() {
    bus_w(DUMMY_REG, bus_r(DUMMY_REG) & (~DUMMY_ANLG_FIFO_RD_STRBE_MSK) & (~DUMMY_DGTL_FIFO_RD_STRBE_MSK));
}

void readSample(int ns) {
	if (!(ns%1000)) {
		FILE_LOG(logDEBUG2, ("Reading sample ns:%d (out of %d), fifodinstatus:0x%x\n",
				ns, nSamples,
				bus_r(FIFO_DIN_STATUS_REG)));
	}
    uint32_t addr = DUMMY_REG;
    uint32_t fifoAddr = FIFO_DATA_REG;

    // read adcs
    if (analogEnable) {

        // read strobe to all analog fifos
        bus_w(addr, bus_r(addr) | DUMMY_ANLG_FIFO_RD_STRBE_MSK);
        bus_w(addr, bus_r(addr) & (~DUMMY_ANLG_FIFO_RD_STRBE_MSK));

        // loop through all channels
        int ich = 0;
        for (ich = 0; ich < NCHAN_ANALOG; ++ich) {

            // if channel is in ROI
            if ((1 << ich) & ~(adcDisableMask)) {

                // unselect channel
                bus_w(addr, bus_r(addr) & ~(DUMMY_FIFO_CHNNL_SLCT_MSK));

                // select channel
                bus_w(addr, bus_r(addr) | ((ich << DUMMY_FIFO_CHNNL_SLCT_OFST) & DUMMY_FIFO_CHNNL_SLCT_MSK));

                // read fifo and write it to current position of data pointer
                *((uint16_t*)now_ptr) = bus_r16(fifoAddr);

                // keep reading till the value is the same
                while (*((uint16_t*)now_ptr) != bus_r16(fifoAddr)) {
                    FILE_LOG(logDEBUG1, ("%d ", ich));
                    *((uint16_t*)now_ptr) = bus_r16(fifoAddr);
                }

                // increment pointer to data out destination
                now_ptr += 2;
            }
        }
    }

    // read digital output
    if (digitalEnable) {
        // read strobe to digital fifo
        bus_w(addr, bus_r(addr) | DUMMY_DGTL_FIFO_RD_STRBE_MSK);
        bus_w(addr, bus_r(addr) & (~DUMMY_DGTL_FIFO_RD_STRBE_MSK));

        // read fifo and write it to current position of data pointer
        *((uint64_t*)now_ptr) = get64BitReg(FIFO_DIN_LSB_REG, FIFO_DIN_MSB_REG);
        now_ptr += 8;
    }
}

// only called for first sample
int checkDataPresent() {
    uint32_t dataPresent = bus_r(LOOK_AT_ME_REG);
    FILE_LOG(logDEBUG2, ("LookatMe:0x%x, status:0x%x\t, fifodinstatus:0x%x\n",
			dataPresent,
			bus_r(STATUS_REG),
			bus_r(FIFO_DIN_STATUS_REG)));
    // as long as fifo empty (keep checking)
    while (!dataPresent) {
        // acquisition done
        if (!runBusy()) {
            usleep(WAIT_TME_US_FR_LK_AT_ME_REG);
            dataPresent = bus_r(LOOK_AT_ME_REG);
            // still no data
            if (!dataPresent) {
                FILE_LOG(logINFO, ("Acquisition Finished (State: 0x%08x), "
                        "no frame found .\n", bus_r(STATUS_REG)));
                return FAIL;
            }
            // got data, exit
            else  {
                break;
            }
        }
        // check if fifo empty again
        dataPresent = bus_r(LOOK_AT_ME_REG);
    }
    FILE_LOG(logDEBUG2, ("Got data, Lookatme:0x%x\n", dataPresent));
    return OK;
}

int readFrameFromFifo() {
	int ns = 0;
	// point the data pointer to the starting position of data
	now_ptr = ramValues;

    // no data for this frame
    if (checkDataPresent() == FAIL) {
        return FAIL;
    }

    // read Sample
    while(ns < nSamples) {
    	// chceck if no data in fifo, return ns?//FIXME: ask Anna
        readSample(ns);
        ns++;
    }

    // got frame
    return OK;
}

uint32_t runBusy() {
#ifdef VIRTUAL
    return virtual_status;
#endif
	uint32_t s = (bus_r(STATUS_REG) & STATUS_RN_BSY_MSK);
	//FILE_LOG(logDEBUG1, ("Status Register: %08x\n", s));
	return s;
}








/* common */

int calculateDataBytes(){
	return dataBytes;
}

int getTotalNumberOfChannels(){return  ((int)getNumberOfChannelsPerChip() * (int)getNumberOfChips());}
int getNumberOfChips(){return  NCHIP;}
int getNumberOfDACs(){return  NDAC;}
int getNumberOfChannelsPerChip(){return  NCHAN;}


