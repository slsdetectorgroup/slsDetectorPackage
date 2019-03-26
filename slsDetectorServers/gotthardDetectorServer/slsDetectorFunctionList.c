#include "slsDetectorFunctionList.h"
#include "gitInfoGotthard.h"
#include "versionAPI.h"
#include "logger.h"
#include "RegisterDefs.h"

#ifndef VIRTUAL
#include "AD9257.h"		// commonServerFunctions.h, blackfin.h, ansi.h
#include "AD9252.h"     // old board compatibility
#include "LTC2620.h"    // dacs
#else
#include "blackfin.h"
#include <pthread.h>
#include <time.h>
#endif

#include "string.h"

// Variables that will be exported
int phaseShift = DEFAULT_PHASE_SHIFT;

int firmware_compatibility = OK;
int firmware_check_done = 0;
char firmware_message[MAX_STR_LENGTH];

#ifdef VIRTUAL
pthread_t pthread_virtual_tid;
int virtual_status = 0;
int virtual_stop = 0;
int highvoltage = 0;
#endif

int detectorFirstServer = 1;
int dacValues[NDAC] = {0};
enum detectorSettings thisSettings = UNINITIALIZED;
enum externalSignalFlag signalMode = 0;
int digitalTestBit = 0;

// roi configuration
int adcConfigured = -1;
ROI rois[MAX_ROIS];
int nROI = 0;
int ipPacketSize = 0;
int udpPacketSize = 0;

// master slave configuration (for 25um)
int masterflags = NO_MASTER;
int masterdefaultdelay = 62;
int patternphase = 0;
int adcphase = 0;
int slavepatternphase = 0;
int slaveadcphase = 0;
int rsttosw1delay = 2;
int startacqdelay = 1;

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
    FILE_LOG(logINFOBLUE, ("******** Gotthard Virtual Server *****************\n"));
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
    if (mapCSP0() == FAIL) {
    	strcpy(firmware_message,
				"Could not map to memory. Dangerous to continue.\n");
		FILE_LOG(logERROR, ("%s\n\n", firmware_message));
		firmware_compatibility = FAIL;
		firmware_check_done = 1;
		return;
    }

    // does check only if flag is 0 (by default), set by command line
	if (((checkType() == FAIL) || (testFpga() == FAIL) || (testBus() == FAIL))) {
		strcpy(firmware_message,
				"Could not pass basic tests of FPGA and bus. Dangerous to continue.\n");
		FILE_LOG(logERROR, ("%s\n\n", firmware_message));
		firmware_compatibility = FAIL;
		firmware_check_done = 1;
		return;
	}

	uint32_t boardrev           = getBoardRevision();
	uint32_t ipadd				= getDetectorIP();
	uint64_t macadd				= getDetectorMAC();
    int64_t fwversion           = getDetectorId(DETECTOR_FIRMWARE_VERSION);
	int64_t swversion 			= getDetectorId(DETECTOR_SOFTWARE_VERSION);
	int64_t client_sw_apiversion = getDetectorId(CLIENT_SOFTWARE_API_VERSION);

	FILE_LOG(logINFOBLUE, ("************ Gotthard Server *********************\n"
	        "Board Revision         : 0x%x\n"

			"Detector IP Addr       : 0x%x\n"
			"Detector MAC Addr      : 0x%llx\n\n"

			"Firmware Version       : 0x%llx\n"
			"Software Version       : 0x%llx\n"
			"Client-S/w API Version : 0x%llx\n"
			"********************************************************\n",
			boardrev,

			ipadd,
			(long  long unsigned int)macadd,

			(long  long int)fwversion,
			(long  long int)swversion,
			(long long int)client_sw_apiversion
	));

	FILE_LOG(logINFO, ("Basic Tests - success\n"));
	firmware_check_done = 1;
#endif
}

int checkType() {
#ifdef VIRTUAL
    return OK;
#endif
	volatile u_int32_t type = ((bus_r(BOARD_REVISION_REG) & DETECTOR_TYPE_MSK) >> DETECTOR_TYPE_OFST);
	if (type == DETECTOR_TYPE_MOENCH_VAL){
			FILE_LOG(logERROR, ("This is not a Gotthard Server (read %d, expected ?)\n", type));
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
	u_int32_t val = bus_r(FIX_PATT_REG);
	if (val == FIX_PATT_VAL) {
		FILE_LOG(logINFO, ("Fixed pattern: successful match (0x%08x)\n",val));
	} else {
		FILE_LOG(logERROR, ("Fixed pattern does not match! Read 0x%08x, expected 0x%08x\n", val, FIX_PATT_VAL));
		ret = FAIL;
	}

	if (ret == OK) {
	    // dummy reg
	    FILE_LOG(logINFO, ("\tTesting Dummy Register:\n"));
	    u_int32_t addr = DUMMY_REG;
	    volatile u_int32_t val = 0, readval = 0;
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
	    bus_w(addr, 0);
	    if (ret == OK) {
	        FILE_LOG(logINFO, ("Successfully tested FPGA Dummy Register %d times\n", times));
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
	u_int32_t addr = DUMMY_REG;
	volatile u_int32_t val = 0, readval = 0;
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

	bus_w(addr, 0);

	if (ret == OK) {
		FILE_LOG(logINFO, ("Successfully tested bus %d times\n", times));
	}
	return ret;
}

int detectorTest( enum digitalTestMode arg, int ival) {
#ifdef VIRTUAL
    return OK;
#endif
	switch(arg){
	case DIGITAL_BIT_TEST:
	    if (ival > -1) {
	        digitalTestBit = (ival == 0) ? 0 : 1;
	        FILE_LOG(logINFO, ("Digital Test bit set: %d\n", digitalTestBit));
	    }
	    return digitalTestBit;
	case DETECTOR_FIRMWARE_TEST:	return testFpga();
	case DETECTOR_BUS_TEST: 		return testBus();
	default:
		FILE_LOG(logERROR, ("Test not implemented for this detector %d\n", (int)arg));
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
	case DETECTOR_SOFTWARE_VERSION:
		return  (GITDATE & 0xFFFFFF);
	case CLIENT_SOFTWARE_API_VERSION:
		return APIGOTTHARD;
	default:
		return retval;
	}
}

u_int64_t getFirmwareVersion() {
#ifdef VIRTUAL
    return 0;
#endif
    return ((bus_r(FPGA_VERSION_REG) & FPGA_VERSION_MSK) >> FPGA_VERSION_OFST);
}

u_int32_t getDetectorNumber(){
#ifdef VIRTUAL
    return 0;
#endif
	return 0;
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

u_int32_t getBoardRevision() {
#ifdef VIRTUAL
    return 0;
#endif
    return ((bus_r(BOARD_REVISION_REG) & BOARD_REVISION_MSK) >> BOARD_REVISION_OFST);
}


/* initialization */

void initControlServer(){
	setupDetector();
}

void initStopServer() {
	if (mapCSP0() == FAIL) {
		FILE_LOG(logERROR, ("Stop Server: Map Fail. Dangerous to continue. Goodbye!\n"));
		exit(EXIT_FAILURE);
	}
}


/* set up detector */

void setupDetector() {
    FILE_LOG(logINFO, ("This Server is for 1 Gotthard module (1280 channels)\n"));

    // Initialization
    setPhaseShiftOnce();

    // hv
    setHighVoltage(DEFAULT_HIGH_VOLTAGE);

    // adc
    if (getBoardRevision() == 1) {
        AD9252_SetDefines(ADC_SPI_REG, ADC_SPI_SRL_CS_OTPT_MSK, ADC_SPI_SRL_CLK_OTPT_MSK, ADC_SPI_SRL_DT_OTPT_MSK, ADC_SPI_SRL_DT_OTPT_OFST);
        AD9252_Disable();
        AD9252_Configure();
    } else {
        AD9257_SetDefines(ADC_SPI_REG, ADC_SPI_SRL_CS_OTPT_MSK, ADC_SPI_SRL_CLK_OTPT_MSK, ADC_SPI_SRL_DT_OTPT_MSK, ADC_SPI_SRL_DT_OTPT_OFST);
        AD9257_Disable();
        AD9257_Configure();
    }

    // dac
    LTC2620_SetDefines(SPI_REG, SPI_DAC_SRL_CS_OTPT_MSK, SPI_DAC_SRL_CLK_OTPT_MSK, SPI_DAC_SRL_DGTL_OTPT_MSK, SPI_DAC_SRL_DGTL_OTPT_OFST, NDAC, DAC_MIN_MV, DAC_MAX_MV);
    LTC2620_Disable();
    LTC2620_Configure();
    setDefaultDacs();

    // temp
    bus_w(TEMP_SPI_IN_REG, TEMP_SPI_IN_IDLE_MSK);
    bus_w(TEMP_SPI_OUT_REG, 0x0);

    // roi, gbit readout
    setROIADC(-1); // set adcsyncreg, daqreg, chipofinterestreg, cleanfifos,
    setGbitReadout();

    // master, slave (25um)
    setMasterSlaveConfiguration();

    // Default Parameters
    FILE_LOG(logINFOBLUE, ("Setting Default parameters\n"));

    setSettings(DEFAULT_SETTINGS);
    setExtSignal(DEFAULT_TRIGGER_MODE);
    setTiming(DEFAULT_TIMING_MODE);
	setTimer(FRAME_NUMBER, DEFAULT_NUM_FRAMES);
	setTimer(CYCLES_NUMBER, DEFAULT_NUM_CYCLES);
	setTimer(ACQUISITION_TIME, DEFAULT_EXPTIME);
	setTimer(FRAME_PERIOD, DEFAULT_PERIOD);
	setTimer(DELAY_AFTER_TRIGGER, DEFAULT_DELAY);

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
                setDAC((enum DACINDEX)i, defaultvals[i], 0);
            }
        }
    }
    return ret;
}

uint32_t writeRegister16And32(uint32_t offset, uint32_t data) {
   if (((offset << MEM_MAP_SHIFT) == CONTROL_REG) ||
           ((offset << MEM_MAP_SHIFT) == FIFO_DATA_REG)) {
       return writeRegister16(offset, data);
   } else
       return writeRegister(offset, data);

}

uint32_t readRegister16And32(uint32_t offset) {
    if (((offset << MEM_MAP_SHIFT) == CONTROL_REG) ||
            ((offset << MEM_MAP_SHIFT) == FIFO_DATA_REG)) {
        return readRegister16(offset);
    } else
        return readRegister(offset);
}

/* firmware functions (resets) */

void setPhaseShiftOnce() {
    u_int32_t addr = MULTI_PURPOSE_REG;
    volatile u_int32_t val = bus_r(addr);
    FILE_LOG(logDEBUG1, ("Multipurpose reg: 0x%x\n", val));

    // first time detector has switched on
    if (!val) {
        detectorFirstServer = 1;
        FILE_LOG(logINFO, ("Implementing the first phase shift of %d\n", phaseShift));
        int times = 0;
        for (times = 1; times < phaseShift; ++times) {
            bus_w(addr,(INT_RSTN_MSK | ENT_RSTN_MSK | SW1_MSK | PHS_STP_MSK));      //0x1821
            bus_w(addr,(INT_RSTN_MSK | ENT_RSTN_MSK | (SW1_MSK &~ PHS_STP_MSK)));   //0x1820
        }
        FILE_LOG(logDEBUG1, ("Multipurpose reg: 0x%x\n", val));
    } else
        detectorFirstServer = 0;
}

void setPhaseShift(int numphaseshift) {
    FILE_LOG(logINFO, ("Implementing phase shift of %d\n", numphaseshift));
    u_int32_t addr = MULTI_PURPOSE_REG;

    volatile u_int32_t val = bus_r(addr);
    FILE_LOG(logDEBUG1, ("Multipurpose reg: 0x%x\n", val));
    int times = 0;
    for (times = 1; times < numphaseshift; ++times) {
        bus_w(addr, val | PHS_STP_MSK);
        bus_w(addr, val & (~PHS_STP_MSK));
    }
    FILE_LOG(logDEBUG1, ("Multipurpose reg: 0x%x\n", val));
}

void cleanFifos() {
    FILE_LOG(logINFO, ("Cleaning FIFOs\n"));
    bus_w(ADC_SYNC_REG, bus_r(ADC_SYNC_REG) | ADC_SYNC_CLEAN_FIFOS_MSK);
    bus_w(ADC_SYNC_REG, bus_r(ADC_SYNC_REG) & ~ADC_SYNC_CLEAN_FIFOS_MSK);
}

void setADCSyncRegister() {
    FILE_LOG(logINFO, ("\tSetting ADC Sync and Token Delays:\n"));
    u_int32_t addr = ADC_SYNC_REG;

    // 0x88(no roi), 0x1b(roi) (MSB)
    u_int32_t tokenDelay =  ((adcConfigured == -1) ?
            ADC_SYNC_ENET_DELAY_NO_ROI_VAL : ADC_SYNC_ENET_DELAY_ROI_VAL);

    // 0x88032214(no roi), 0x1b032214(with roi)
    u_int32_t val = (ADC_SYNC_TKN_VAL | tokenDelay);

    bus_w(addr, val);
    FILE_LOG(logINFO, ("\tADC Sync Reg: 0x%x\n", bus_r(addr)));
}

void setDAQRegister() {
    FILE_LOG(logINFO, ("\tSetting Packet Length and DAQ Token Timing:\n"));
    u_int32_t addr = DAQ_REG;

    // 0x1f16(board rev 1) 0x1f0f(board rev 2)
    u_int32_t tokenTiming = ((getBoardRevision() == 1) ?
            DAQ_TKN_TMNG_BRD_RVSN_1_VAL : DAQ_TKN_TMNG_BRD_RVSN_2_VAL);

    // 0x13f(no roi), 0x7f(roi)
    u_int32_t packetLength = ((adcConfigured == -1) ?
            DAQ_PCKT_LNGTH_NO_ROI_VAL : DAQ_PCKT_LNGTH_ROI_VAL);

    // MSB: packetLength LSB: tokenTiming
    u_int32_t val =  (tokenTiming | packetLength);

    bus_w(addr, val);
    FILE_LOG(logINFO, ("\tDAQ Reg: 0x%x\n", bus_r(addr)));
}

void setChipOfInterestRegister(int adc) {
    FILE_LOG(logINFO, ("\tSelecting Chips of Interst:\n"));
    u_int32_t addr = CHIP_OF_INTRST_REG;

    // 0x1f(no roi), 0xXX(roi)
    u_int32_t adcSelect = ((adcConfigured == -1) ?
            CHIP_OF_INTRST_ADC_SEL_MSK :
            (((1 << adc) << CHIP_OF_INTRST_ADC_SEL_OFST) & CHIP_OF_INTRST_ADC_SEL_MSK));

    // 0x0500(no roi), 0x0100(roi)
    u_int32_t numChannels = (adcConfigured == -1) ? (NCHIP * NCHAN) : (NCHIPS_PER_ADC * NCHAN);
    numChannels = ((numChannels << CHIP_OF_INTRST_NUM_CHNNLS_OFST) & CHIP_OF_INTRST_NUM_CHNNLS_MSK);

    // 0x500001f(no roi), 0x10000xx(roi) MSB:num channels, LSB: selected ADC
    u_int32_t val =  (numChannels | adcSelect);

    bus_w(addr, val);
    FILE_LOG(logINFO, ("\tChip Of Interest Reg: 0x%x\n", bus_r(addr)));
}

void setROIADC(int adc) {
    FILE_LOG(logINFO, ("\tSetting ROI ADC: %d\n", adc));
    adcConfigured = adc;

    setADCSyncRegister();           // adc sync & token delays
    setDAQRegister();               // packet length & token timing
    cleanFifos();                   // clean fifos
    setChipOfInterestRegister(adc); // num channels & select adc

    ipPacketSize = ((adcConfigured == -1) ? IP_PACKET_SIZE_NO_ROI : IP_PACKET_SIZE_ROI);
    udpPacketSize = ((adcConfigured == -1) ? UDP_PACKETSIZE_NO_ROI : UDP_PACKETSIZE_ROI);
}

void setGbitReadout() {
    FILE_LOG(logINFO, ("Setting Gbit Readout\n"));
    u_int32_t addr = CONFIG_REG;
    bus_w(addr, bus_r(addr) & ~CONFIG_CPU_RDT_MSK);
    FILE_LOG(logINFO, ("\tConfig Reg 0x%x\n", bus_r(addr)));
}

int readConfigFile() {
    // open config file
    FILE* fd = fopen(CONFIG_FILE, "r");
    if(fd == NULL) {
        FILE_LOG(logWARNING, ("\tCould not find config file %s\n", CONFIG_FILE));
        return FAIL;
    }
    FILE_LOG(logINFO, ("\tConfig file %s opened\n", CONFIG_FILE));

    // Initialization
    const size_t lineSize = 256;
    char line[lineSize];
    memset(line, 0, lineSize);
    const size_t keySize = lineSize / 2;
    char key[keySize];
    memset(key, 0, keySize);
    char value[keySize];
    memset(value, 0, keySize);
    int scan = OK;

    // keep reading a line
    while (fgets(line, lineSize, fd)) {
        // ignore comments
        if (line[0] == '#')
            continue;
        // read key & value
        sscanf(line, "%s %s\n", key, value);

        // key is master/ slave flag
        if (!strcasecmp(key,"masterflags")) {
            if  (!strcasecmp(value,"is_master")) {
                masterflags = IS_MASTER;
                FILE_LOG(logINFOBLUE, ("\tMaster\n"));
            } else if (!strcasecmp(value,"is_slave")) {
                masterflags = IS_SLAVE;
                FILE_LOG(logINFOBLUE, ("\tSlave\n"));
            } else  if (!strcasecmp(value,"no_master")){
                masterflags = NO_MASTER;
                FILE_LOG(logINFOBLUE, ("\tNo Master\n"));
            } else {
                FILE_LOG(logERROR, ("\tCould not scan masterflags %s value from config file\n", value));
                scan = FAIL;
                break;
            }

            // not first server since detector power on
            if (!detectorFirstServer) {
                FILE_LOG(logINFOBLUE, ("\tServer has been started up before. Ignoring rest of config file\n"));
                fclose(fd);
                return FAIL;
            }
        }

        // key is not master/ slave flag
        else {
            // convert value to int
            int ival = 0;
            if(sscanf(value, "%d", &ival) <= 0) {
                FILE_LOG(logERROR, ("\tCould not scan parameter %s value %s from config file\n", key, value));
                scan = FAIL;
                break;
            }
            // set value
            if (!strcasecmp(key, "masterdefaultdelay"))
                masterdefaultdelay = ival;
            else if (!strcasecmp(key, "patternphase"))
                patternphase = ival;
            else if (!strcasecmp(key, "adcphase"))
                adcphase = ival;
            else if (!strcasecmp(key, "slavepatternphase"))
                slavepatternphase = ival;
            else if (!strcasecmp(key, "slaveadcphase"))
                slaveadcphase = ival;
            else if (!strcasecmp(key, "rsttosw1delay"))
                rsttosw1delay = ival;
            else if (!strcasecmp(key, "startacqdelay"))
                startacqdelay = ival;
            else {
                FILE_LOG(logERROR, ("\tCould not scan parameter %s from config file\n", key));
                scan = FAIL;
                break;
            }
        }
    }
    fclose(fd);
    if (scan == FAIL)
        exit(EXIT_FAILURE);

    FILE_LOG(logINFOBLUE, (
            "\tmasterdefaultdelay:%d\n"
            "\tpatternphase:%d\n"
            "\tadcphase:%d\n"
            "\tslavepatternphase:%d\n"
            "\tslaveadcphase:%d\n"
            "\trsttosw1delay:%d\n"
            "\tstartacqdelay:%d\n",
            masterdefaultdelay,
            patternphase,
            adcphase,
            slavepatternphase,
            slaveadcphase,
            rsttosw1delay,
            startacqdelay));
    return OK;
}

void setMasterSlaveConfiguration() {
    FILE_LOG(logINFO, ("Reading Master Slave Configuration\n"));

    // no config file or not first time server
    if (readConfigFile() == FAIL)
        return;

    // master configuration
    if (masterflags == IS_MASTER) {
        // master default delay set, so reset delay
        setTimer(DELAY_AFTER_TRIGGER, 0);

        // Set pattern phase for the master module
        u_int32_t val = (bus_r(MULTI_PURPOSE_REG) & (~(PLL_CLK_SL_MSK))); // unset mask
        bus_w(MULTI_PURPOSE_REG, val | PLL_CLK_SL_MSTR_VAL);
        setPhaseShift(patternphase);

        // Set adc phase for the master module
        val = (bus_r(MULTI_PURPOSE_REG) & (~(PLL_CLK_SL_MSK))); // unset mask
        bus_w(MULTI_PURPOSE_REG, val | PLL_CLK_SL_MSTR_ADC_VAL);
        setPhaseShift(adcphase);

        // Set pattern phase for the slave module
        val = (bus_r(MULTI_PURPOSE_REG) & (~(PLL_CLK_SL_MSK))); // unset mask
        bus_w(MULTI_PURPOSE_REG, val | PLL_CLK_SL_SLV_VAL);
        setPhaseShift(slavepatternphase);

        // Set adc phase for the slave module
        val = (bus_r(MULTI_PURPOSE_REG) & (~(PLL_CLK_SL_MSK))); // unset mask
        bus_w(MULTI_PURPOSE_REG, val | PLL_CLK_SL_SLV_ADC_VAL);
        setPhaseShift(slaveadcphase);

        // Set start acq delay
        val = (bus_r(MULTI_PURPOSE_REG) & (~(STRT_ACQ_DLY_MSK))); // unset mask
        val = val | ((startacqdelay << STRT_ACQ_DLY_OFST) & STRT_ACQ_DLY_MSK); // set val
        bus_w(MULTI_PURPOSE_REG, val);
        FILE_LOG(logDEBUG1, ("\tMultipurpose reg: 0x%x\n", val));
    }

    // all configuration - Set RST to SW1 delay
    u_int32_t val = (bus_r(MULTI_PURPOSE_REG) & (~(RST_TO_SW1_DLY_MSK))); // unset mask
    val = val | ((rsttosw1delay << RST_TO_SW1_DLY_OFST) & RST_TO_SW1_DLY_MSK); // set val
    bus_w(MULTI_PURPOSE_REG, val);
    FILE_LOG(logDEBUG1, ("\tMultipurpose reg: 0x%x\n", val));

    FILE_LOG(logINFO, ("\tMaster Slave Configuration has been set up\n"));
}


/* set parameters -  dr, roi */

int setDynamicRange(int dr){
	return DYNAMIC_RANGE;
}

ROI* setROI(int n, ROI arg[], int *retvalsize, int *ret) {

    // set ROI
    if(n >= 0){
        // print
        if (!n) {
            FILE_LOG(logINFO, ("Clearing ROI\n"));
        } else {
            FILE_LOG(logINFO, ("Setting ROI:\n"));
            int i = 0;
            for (i = 0; i < n; ++i) {
                FILE_LOG(logINFO, ("\t(%d, %d)\n", arg[i].xmin, arg[i].xmax));
            }
        }
        // only one ROI allowed per module
        if (n > 1) {
            FILE_LOG(logERROR, ("\tCannot set more than 1 ROI per module\n"));
            *ret = FAIL;
            *retvalsize = nROI;
            return rois;
        }

        //clear all rois
        nROI = 0;

        // find adc number and recorrect channel limits
        int adc = -1;
        if (n) {
            // all channels
            if ((arg[0].xmin <= 0) && (arg[0].xmax >= NCHIP * NCHAN))
                adc = -1;
            // single adc
            else {
                //adc = mid value/numchans
                adc = ((((arg[0].xmax) + (arg[0].xmin))/2) / (NCHAN * NCHIPS_PER_ADC));
                // incorrect adc
                if((adc < 0) || (adc > 4)) {
                    FILE_LOG(logERROR, ("\tadc value greater than 5. deleting roi\n"));
                    adc = -1;
                }
                // recorrect roi values
                else {
                    rois[0].xmin = adc * (NCHAN * NCHIPS_PER_ADC);
                    rois[0].xmax = (adc + 1) * (NCHAN * NCHIPS_PER_ADC) - 1;
                    rois[0].ymin = -1;
                    rois[0].ymax = -1;
                    nROI = 1;
                }
            }
        }

        if (adc == -1)
            nROI = 0;

        FILE_LOG(logINFO, ("\tAdc to be configured: %d\n", adc));
        FILE_LOG(logINFO, ("\tROI to be configured: (%d, %d)\n",
                (adc == -1) ? 0 :  (rois[0].xmin),
                        (adc == -1) ? (NCHIP * NCHAN - 1) :  (rois[0].xmax)));

        // could not set roi
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

        //set adc of interest
        setROIADC(adc);
    } else FILE_LOG(logINFO, ("Getting ROI:\n"));

    // print
    if (!nROI) {
        FILE_LOG(logINFO, ("\tROI: None\n"));
    } else {
        FILE_LOG(logINFO, ("ROI:\n"));
        int i = 0;
        for (i = 0; i < nROI; ++i) {
            FILE_LOG(logINFO, ("\t(%d, %d)\n", rois[i].xmin, rois[i].xmax));

        }
    }

    *retvalsize = nROI;
    return rois;
}


/* parameters - timer */

int64_t setTimer(enum timerIndex ind, int64_t val) {

	int64_t retval = -1;
	switch(ind){

	case FRAME_NUMBER:
		if(val >= 0) {
			FILE_LOG(logINFO, ("Setting Frames: %lld\n",(long long int)val));
		}
		retval = set64BitReg(val,  SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG);
		FILE_LOG(logDEBUG1, ("\tGetting Frames: %lld\n", (long long int)retval));
		break;

	case ACQUISITION_TIME:
		if(val >= 0){
			FILE_LOG(logINFO, ("Setting Exptime: %lld ns\n", (long long int)val));
			val = (val * 1E-3 * CLK_FREQ) + 0.5;
		}
		retval = (set64BitReg(val, SET_EXPTIME_LSB_REG, SET_EXPTIME_MSB_REG) /
		        (1E-3 * CLK_FREQ)) + 0.5;
		FILE_LOG(logINFO, ("\tGetting Exptime: %lld ns\n", (long long int)retval));
		break;

	case FRAME_PERIOD:
		if(val >= 0){
			FILE_LOG(logINFO, ("Setting Period: %lld ns\n",(long long int)val));
			val = (val * 1E-3 * CLK_FREQ) + 0.5;
		}
		retval = (set64BitReg(val, SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG) /
		        (1E-3 * CLK_FREQ)) + 0.5;
		FILE_LOG(logINFO, ("\tGetting Period: %lld ns\n", (long long int)retval));
		break;

	case DELAY_AFTER_TRIGGER:
		if(val >= 0){
			FILE_LOG(logINFO, ("Setting Delay: %lld ns\n", (long long int)val));
			if (masterflags == IS_MASTER) {
			    val += masterdefaultdelay;
			    FILE_LOG(logINFO, ("\tActual Delay (master): %lld\n", (long long int) val));
			}
			val = (val * 1E-3 * CLK_FREQ) + 0.5;
		}
		retval = (set64BitReg(val, SET_DELAY_LSB_REG, SET_DELAY_MSB_REG) /
		        (1E-3 * CLK_FREQ)) + 0.5;
		FILE_LOG(logINFO, ("\tGetting Delay: %lld ns\n", (long long int)retval));
	    if (masterflags == IS_MASTER) {
	        FILE_LOG(logDEBUG1, ("\tActual Delay read (master): %lld\n", (long long int) retval));
	        retval -= masterdefaultdelay;
	    }
		break;

	case CYCLES_NUMBER:
		if(val >= 0) {
			FILE_LOG(logINFO, ("Setting Cycles: %lld\n", (long long int)val));
		}
		retval = set64BitReg(val,  SET_TRAINS_LSB_REG, SET_TRAINS_MSB_REG);
		FILE_LOG(logDEBUG1, ("\tGetting Cycles: %lld\n", (long long int)retval));
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

    case ACQUISITION_TIME:
        retval = (get64BitReg(GET_EXPTIME_LSB_REG, GET_EXPTIME_MSB_REG) /
                (1E-3 * CLK_FREQ)) + 0.5;
        FILE_LOG(logINFO, ("Getting exptime left: %lldns\n", (long long int)retval));
        break;

	case FRAME_PERIOD:
		retval = (get64BitReg(GET_PERIOD_LSB_REG, GET_PERIOD_MSB_REG) /
		        (1E-3 * CLK_FREQ)) + 0.5;
		FILE_LOG(logINFO, ("Getting period left: %lldns\n", (long long int)retval));
		break;

	case DELAY_AFTER_TRIGGER:
        retval = (get64BitReg(GET_DELAY_LSB_REG, GET_DELAY_MSB_REG) /
                (1E-3 * CLK_FREQ)) + 0.5;
        FILE_LOG(logINFO, ("Getting delay left: %lldns\n", (long long int)retval));
        if (masterflags == IS_MASTER) {
            FILE_LOG(logDEBUG1, ("\tGetting Actual delay (master): %lld\n", (long long int) retval));
            retval -= masterdefaultdelay;
        }
        break;

	case CYCLES_NUMBER:
		retval = get64BitReg(GET_TRAINS_LSB_REG, GET_TRAINS_MSB_REG);
		FILE_LOG(logINFO, ("Getting number of cycles left: %lld\n", (long long int)retval));
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
    case FRAME_PERIOD:
        val = (val * 1E-3 * CLK_FREQ) + 0.5;    // convert to freq
        val = (val / (1E-3 * CLK_FREQ)) + 0.5;  // convert back to timer
        if (val != retval)
            return FAIL;
        break;
    case DELAY_AFTER_TRIGGER:
        if (masterflags == IS_MASTER) {
            val += masterdefaultdelay;
        }
        val = (val * 1E-3 * CLK_FREQ) + 0.5;    // convert to freq
        val = (val / (1E-3 * CLK_FREQ)) + 0.5;  // convert back to timer
        if (masterflags == IS_MASTER) {
            val -= masterdefaultdelay;
        }
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
	    u_int32_t addr = GAIN_REG;

	    // find conf gain value
	    uint32_t confgain = 0x0;
	    switch (sett) {
        case DYNAMICGAIN:
            FILE_LOG(logINFO, ("Set settings - Dyanmic Gain\n"));
            confgain = GAIN_CONFGAIN_DYNMC_GAIN_VAL;
            break;
        case HIGHGAIN:
            FILE_LOG(logINFO, ("Set settings - High Gain\n"));
            confgain = GAIN_CONFGAIN_HGH_GAIN_VAL;
            break;
        case LOWGAIN:
            FILE_LOG(logINFO, ("Set settings - Low Gain\n"));
            confgain = GAIN_CONFGAIN_LW_GAIN_VAL;
            break;
        case MEDIUMGAIN:
            FILE_LOG(logINFO, ("Set settings - Medium Gain\n"));
            confgain = GAIN_CONFGAIN_MDM_GAIN_VAL;
            break;
        case VERYHIGHGAIN:
            FILE_LOG(logINFO, ("Set settings - Very High Gain\n"));
            confgain = GAIN_CONFGAIN_VRY_HGH_GAIN_VAL;
            break;
        default:
            FILE_LOG(logERROR, ("This settings is not defined for this detector %d\n", (int)sett));
            return -1;
	    }
	    // set conf gain
        bus_w(addr, bus_r(addr) & ~GAIN_CONFGAIN_MSK);
        bus_w(addr, bus_r(addr) | confgain);
        FILE_LOG(logINFO, ("\tGain Reg: 0x%x\n", bus_r(addr)));
		thisSettings = sett;
	}

	return getSettings();
}

enum detectorSettings getSettings(){
	uint32_t regval = bus_r(GAIN_REG);
	uint32_t val = regval & GAIN_CONFGAIN_MSK;
	switch(val) {
    case GAIN_CONFGAIN_DYNMC_GAIN_VAL:
        FILE_LOG(logDEBUG1, ("Settings read: Dynamic Gain. Gain Reg: 0x%x\n", regval));
        thisSettings = DYNAMICGAIN;
        break;
    case GAIN_CONFGAIN_HGH_GAIN_VAL:
        FILE_LOG(logDEBUG1, ("Settings read: High Gain. Gain Reg: 0x%x\n", regval));
        thisSettings = HIGHGAIN;
        break;
    case GAIN_CONFGAIN_LW_GAIN_VAL:
        FILE_LOG(logDEBUG1, ("Settings read: Low Gain. Gain Reg: 0x%x\n", regval));
        thisSettings = LOWGAIN;
        break;
    case GAIN_CONFGAIN_MDM_GAIN_VAL:
        FILE_LOG(logDEBUG1, ("Settings read: Medium Gain. Gain Reg: 0x%x\n", regval));
        thisSettings = MEDIUMGAIN;
        break;
    case GAIN_CONFGAIN_VRY_HGH_GAIN_VAL:
        FILE_LOG(logDEBUG1, ("Settings read: Very High Gain. Gain Reg: 0x%x\n", regval));
        thisSettings = VERYHIGHGAIN;
        break;
    default:
        thisSettings = UNDEFINED;
        FILE_LOG(logERROR, ("Settings read: Undefined. Gain Reg: 0x%x\n", regval));
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


int getADC(enum ADCINDEX ind){
#ifdef VIRTUAL
    return 0;
#endif
	char tempnames[2][40]={"VRs/FPGAs Temperature", "ADCs/ASICs Temperature"};
	FILE_LOG(logDEBUG1, ("Getting Temperature for %s\n", tempnames[ind]));

	u_int32_t addr = TEMP_SPI_IN_REG;
	uint32_t addrout = TEMP_SPI_OUT_REG;
	const int repeats = 6; // number of register writes for delay
	const int reads = 20;
	u_int32_t value = 0;

	// standby, high clk, high cs
	bus_w(addr, (TEMP_SPI_IN_T1_CLK_MSK | TEMP_SPI_IN_T1_CS_MSK | TEMP_SPI_IN_T2_CLK_MSK | TEMP_SPI_IN_T2_CS_MSK));

	// high clk low cs
    bus_w(addr, (TEMP_SPI_IN_T1_CLK_MSK | TEMP_SPI_IN_T2_CLK_MSK));

    {
        int i = 0;
        for (i = 0; i < reads; ++i) {

            int j = 0;
            // low clk low cs
            for (j = 0; j < repeats; ++j)
                bus_w(addr, 0x0);
            //high clk low cs
            for (j = 0; j < repeats; ++j)
                bus_w(addr, (TEMP_SPI_IN_T1_CLK_MSK | TEMP_SPI_IN_T2_CLK_MSK));

            // only the first time
            if (i <= 10) {
                if (ind == TEMP_ADC)
                    value = (value << 1) + (bus_r(addrout) & TEMP_SPI_OUT_T1_DT_MSK);
                else
                    value = (value << 1) + (bus_r(addrout) & TEMP_SPI_OUT_T2_DT_MSK);
            }
        }
    }

    // standby high clk, high cs
    bus_w(addr, (TEMP_SPI_IN_T1_CLK_MSK | TEMP_SPI_IN_T1_CS_MSK | TEMP_SPI_IN_T2_CLK_MSK | TEMP_SPI_IN_T2_CS_MSK));

    FILE_LOG(logDEBUG1, ("\tInitial Temperature value: %u\n", value));
    // conversion
    value = value/4.0;
	FILE_LOG(logINFO, ("\tTemperature %s: %f °C\n",tempnames[ind], value));
	return value;
}

int setHighVoltage(int val){
#ifdef VIRTUAL
    if (val >= 0)
        highvoltage = val;
    return highvoltage;
#endif
    u_int32_t addr = HV_REG;
    u_int32_t sel = 0x0;

    // set
    if (val >= 0) {
        FILE_LOG(logINFO, ("Setting High Voltage to %d\n", val));
        switch (val) {
        case 0:
            break;
        case 90:
            sel = HV_SEL_90_VAL;
            break;
        case 110:
            sel = HV_SEL_110_VAL;
            break;
        case 120:
            sel = HV_SEL_120_VAL;
            break;
        case 150:
            sel = HV_SEL_150_VAL;
            break;
        case 180:
            sel = HV_SEL_180_VAL;
            break;
        case 200:
            sel = HV_SEL_200_VAL;
            break;
        default:
            FILE_LOG(logERROR, ("%d high voltage is not defined for this detector\n", val));
            return setHighVoltage(-1);
        }
        FILE_LOG(logDEBUG1, ("\tHigh voltage value to be sent: 0x%x\n", sel));

        // switch off high voltage
        bus_w(addr, (bus_r(addr) & ~HV_ENBL_MSK));

        // unset mask and set value
        bus_w(addr, (bus_r(addr) & ~HV_SEL_MSK) | sel);

        // switch on high voltage
        if (val > 0)
            bus_w(addr, bus_r(addr) | HV_ENBL_MSK);
    }

    // get
    u_int32_t retval = 0;
    u_int32_t regval = bus_r(addr);
    FILE_LOG(logDEBUG1, ("\tHigh voltage value read: 0x%x\n", regval));

    // if high voltage was enabled, find value
    if (regval & HV_ENBL_MSK) {
        switch (regval & HV_SEL_MSK) {
        case HV_SEL_90_VAL:
            retval = 90;
            break;
        case HV_SEL_110_VAL:
            retval = 110;
            break;
        case HV_SEL_120_VAL:
            retval = 120;
            break;
        case HV_SEL_150_VAL:
            retval = 150;
            break;
        case HV_SEL_180_VAL:
            retval = 180;
            break;
        default:
            retval = 200;
            break;
        }
    }
    FILE_LOG(logDEBUG1, ("\tHigh Voltage: %d\n", retval));
    return retval;
}


/* parameters - timing, extsig */


void setTiming( enum externalCommunicationMode arg){
    u_int32_t addr = EXT_SIGNAL_REG;

	if (arg != GET_EXTERNAL_COMMUNICATION_MODE){
		switch((int)arg){
		case AUTO_TIMING:
		    FILE_LOG(logINFO, ("Set Timing: Auto\n"));
		    bus_w(addr, EXT_SIGNAL_OFF_VAL);
		    break;
		case TRIGGER_EXPOSURE:
		    if (signalMode == TRIGGER_IN_FALLING_EDGE) {
                FILE_LOG(logINFO, ("Set Timing: Trigger (Falling Edge)\n"));
                bus_w(addr, EXT_SIGNAL_TRGGR_IN_FLLNG_VAL);
		    } else {
                FILE_LOG(logINFO, ("Set Timing: Trigger (Rising Edge)\n"));
                bus_w(addr, EXT_SIGNAL_TRGGR_IN_RSNG_VAL);
		    }
		    break;
		default:
			FILE_LOG(logERROR, ("Unknown timing mode %d for this detector\n", (int)arg));
			return;
		}
	}
}

enum externalCommunicationMode getTiming() {
    u_int32_t regval = bus_r(EXT_SIGNAL_REG);
    switch (regval) {
    case EXT_SIGNAL_TRGGR_IN_RSNG_VAL:
    case EXT_SIGNAL_TRGGR_IN_FLLNG_VAL:
        return TRIGGER_EXPOSURE;
    default:
       return AUTO_TIMING;
    }
}

void setExtSignal(enum externalSignalFlag  mode) {
    switch (mode) {
    case TRIGGER_IN_RISING_EDGE:
        FILE_LOG(logINFO, ("Setting External Signal flag: Trigger in Rising Edge\n"));
        break;
    case TRIGGER_IN_FALLING_EDGE:
        FILE_LOG(logINFO, ("Setting External Signal flag: Trigger in Falling Edge\n"));
        break;
    default:
        FILE_LOG(logERROR, ("Extsig (signal mode) %d not defined for this detector\n", mode));
        return;
    }
    signalMode = mode;
    setTiming(getTiming());
}

int getExtSignal() {
    return signalMode;
}


/* configure mac */

void calcChecksum(mac_conf* mac, int sourceip, int destip) {
    mac->ip.ip_ver       = 0x4;
    mac->ip.ip_ihl       = 0x5;
    mac->ip.ip_tos       = 0x0;
    mac->ip.ip_len       = ipPacketSize;
    mac->ip.ip_ident     = 0x0000;
    mac->ip.ip_flag      = 0x2; 	//not nibble aligned (flag& offset
    mac->ip.ip_offset    = 0x00;
    mac->ip.ip_ttl       = 0x70;
    mac->ip.ip_protocol  = 0x11;
    mac->ip.ip_chksum    = 0x0000 ; // pseudo
    mac->ip.ip_sourceip  = sourceip;
    mac->ip.ip_destip    = destip;
    FILE_LOG(logDEBUG1, ("\tIP TTL: 0x%x\n", mac->ip.ip_ttl));

	int count = sizeof(mac->ip);
	unsigned short *addr;
	addr = (unsigned short*)(&(mac->ip)); /* warning: assignment from incompatible pointer type */

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
	FILE_LOG(logINFO, ("\tIP checksum : 0x%lx\n", checksum));
	mac->ip.ip_chksum   =  checksum;
}

int configureMAC(uint32_t destip, uint64_t destmac, uint64_t sourcemac, uint32_t sourceip, uint32_t udpport) {
#ifdef VIRTUAL
    return OK;
#endif
	FILE_LOG(logINFOBLUE, ("Configuring MAC\n"));
    u_int32_t addr = MULTI_PURPOSE_REG;

	FILE_LOG(logDEBUG1, ("\tRoi: %d, Ip Packet size: %d UDP Packet size: %d\n",
	        adcConfigured, ipPacketSize, udpPacketSize));

	uint32_t sourceport  =  DEFAULT_TX_UDP_PORT;
	FILE_LOG(logINFO, ("\tSource IP   : %d.%d.%d.%d (0x%08x)\n",
	        (sourceip>>24)&0xff,(sourceip>>16)&0xff,(sourceip>>8)&0xff,(sourceip)&0xff, sourceip));
	FILE_LOG(logINFO, ("\tSource MAC  : %02x:%02x:%02x:%02x:%02x:%02x (0x%010llx)\n",
			(unsigned int)((sourcemac>>40)&0xFF),
			(unsigned int)((sourcemac>>32)&0xFF),
			(unsigned int)((sourcemac>>24)&0xFF),
			(unsigned int)((sourcemac>>16)&0xFF),
			(unsigned int)((sourcemac>>8)&0xFF),
			(unsigned int)((sourcemac>>0)&0xFF),
			(long  long unsigned int)sourcemac));
	FILE_LOG(logINFO, ("\tSource Port : %d (0x%08x)\n",sourceport, sourceport));
	FILE_LOG(logINFO, ("\tDest. IP    : %d.%d.%d.%d (0x%08x)\n",
	        (destip>>24)&0xff,(destip>>16)&0xff,(destip>>8)&0xff,(destip)&0xff, destip));
	FILE_LOG(logINFO, ("\tDest. MAC   : %02x:%02x:%02x:%02x:%02x:%02x (0x%010llx)\n",
			(unsigned int)((destmac>>40)&0xFF),
			(unsigned int)((destmac>>32)&0xFF),
			(unsigned int)((destmac>>24)&0xFF),
			(unsigned int)((destmac>>16)&0xFF),
			(unsigned int)((destmac>>8)&0xFF),
			(unsigned int)((destmac>>0)&0xFF),
			(long  long unsigned int)destmac));
	FILE_LOG(logINFO, ("\tDest. Port  : %d (0x%08x)\n",udpport, udpport));

	// set/ unset the digital test bit
	if (digitalTestBit)
	    bus_w (addr, bus_r(addr) | DGTL_TST_MSK);
	else
	    bus_w (addr, bus_r(addr) & ~DGTL_TST_MSK);
    FILE_LOG(logDEBUG1, ("\tDigital Test Bit. MultiPurpose reg: 0x%x\n", bus_r(addr)));

	//reset mac
	bus_w (addr, bus_r(addr) | RST_MSK);
	FILE_LOG(logDEBUG1, ("\tReset Mac. MultiPurpose reg: 0x%x\n", bus_r(addr)));

	usleep(500000);

	// release reset
	bus_w(addr, bus_r(addr) &(~ RST_MSK));
	FILE_LOG(logDEBUG1, ("\tReset released. MultiPurpose reg: 0x%x\n", bus_r(addr)));

	// write shadow regs
    bus_w(addr, bus_r(addr) | (ENT_RSTN_MSK | WRT_BCK_MSK));
    FILE_LOG(logDEBUG1, ("\tWrite shadow regs. MultiPurpose reg: 0x%x\n", bus_r(addr)));

    // release write back
    bus_w(addr, bus_r(addr) &(~WRT_BCK_MSK));
    FILE_LOG(logDEBUG1, ("\tWrite back released. MultiPurpose reg: 0x%x\n", bus_r(addr)));

    FILE_LOG(logDEBUG1, ("\tConfiguring MAC CONF\n"));
    mac_conf *mac_conf_regs = (mac_conf*)(CSP0BASE + ENET_CONF_REG * 2);    // direct write
    mac_conf_regs->mac.mac_dest_mac1  = ((destmac >> (8 * 5)) & 0xFF);
    mac_conf_regs->mac.mac_dest_mac2  = ((destmac >> (8 * 4)) & 0xFF);
    mac_conf_regs->mac.mac_dest_mac3  = ((destmac >> (8 * 3)) & 0xFF);
    mac_conf_regs->mac.mac_dest_mac4  = ((destmac >> (8 * 2)) & 0xFF);
    mac_conf_regs->mac.mac_dest_mac5  = ((destmac >> (8 * 1)) & 0xFF);
    mac_conf_regs->mac.mac_dest_mac6  = ((destmac >> (8 * 0)) & 0xFF);
    FILE_LOG(logDEBUG1, ("\tDestination Mac: %llx %x:%x:%x:%x:%x:%x\n",
            destmac,
            mac_conf_regs->mac.mac_dest_mac1,
            mac_conf_regs->mac.mac_dest_mac2,
            mac_conf_regs->mac.mac_dest_mac3,
            mac_conf_regs->mac.mac_dest_mac4,
            mac_conf_regs->mac.mac_dest_mac5,
            mac_conf_regs->mac.mac_dest_mac6));
    mac_conf_regs->mac.mac_src_mac1  = ((sourcemac >> (8 * 5)) & 0xFF);
    mac_conf_regs->mac.mac_src_mac2  = ((sourcemac >> (8 * 4)) & 0xFF);
    mac_conf_regs->mac.mac_src_mac3  = ((sourcemac >> (8 * 3)) & 0xFF);
    mac_conf_regs->mac.mac_src_mac4  = ((sourcemac >> (8 * 2)) & 0xFF);
    mac_conf_regs->mac.mac_src_mac5  = ((sourcemac >> (8 * 1)) & 0xFF);
    mac_conf_regs->mac.mac_src_mac6  = ((sourcemac >> (8 * 0)) & 0xFF);
    FILE_LOG(logDEBUG1, ("\tSource Mac: %llx %x:%x:%x:%x:%x:%x\n",
            sourcemac,
            mac_conf_regs->mac.mac_src_mac1,
            mac_conf_regs->mac.mac_src_mac2,
            mac_conf_regs->mac.mac_src_mac3,
            mac_conf_regs->mac.mac_src_mac4,
            mac_conf_regs->mac.mac_src_mac5,
            mac_conf_regs->mac.mac_src_mac6));
    mac_conf_regs->mac.mac_ether_type   = 0x0800;   //ipv4

    calcChecksum(mac_conf_regs, sourceip, destip);
    mac_conf_regs->udp.udp_srcport      = sourceport;
    mac_conf_regs->udp.udp_destport     = udpport;
    mac_conf_regs->udp.udp_len          = udpPacketSize;
    mac_conf_regs->udp.udp_chksum       = 0x0000;

    FILE_LOG(logDEBUG1, ("\tConfiguring TSE\n"));
    tse_conf *tse_conf_regs = (tse_conf*)(CSP0BASE + TSE_CONF_REG * 2);     // direct write
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

    bus_w(addr, bus_r(addr) | (INT_RSTN_MSK | WRT_BCK_MSK));
    FILE_LOG(logDEBUG1, ("\tWrite shadow regs with int reset. MultiPurpose reg: 0x%x\n", bus_r(addr)));

    usleep(100000);

    // release write back
    bus_w(addr, bus_r(addr) &(~WRT_BCK_MSK));
    FILE_LOG(logDEBUG1, ("\tWrite back released. MultiPurpose reg: 0x%x\n", bus_r(addr)));

    bus_w(addr, bus_r(addr) | SW1_MSK);
    FILE_LOG(logDEBUG1, ("\tSw1. MultiPurpose reg: 0x%x\n", bus_r(addr)));

    usleep(1000 * 1000);
    FILE_LOG(logDEBUG1, ("\tConfigure Mac Done\n"));
    {
        /** send out first image as first packet does not give 0xcacacaca (needed to know if first image
         * when switching back and forth between roi and no roi
         */
        FILE_LOG(logINFOBLUE, ("Sending an image to counter the packet numbers\n"));
        // remember old parameters
        enum externalCommunicationMode oldtiming = getTiming();
        uint64_t oldframes = setTimer(FRAME_NUMBER, -1);
        uint64_t oldcycles = setTimer(CYCLES_NUMBER, -1);
        uint64_t oldPeriod = setTimer(FRAME_PERIOD, -1);
        uint64_t oldExptime = setTimer(ACQUISITION_TIME, -1);

        // set to basic parameters
        FILE_LOG(logINFO, ("\tSetting basic parameters\n"
                "\tTiming: auto\n"
                "\tframes: 1\n"
                "\tcycles: 1\n"
                "\tperiod: 1s\n"
                "\texptime: 900ms\n"));
        setTiming(AUTO_TIMING);
        setTimer(FRAME_NUMBER, 1);
        setTimer(CYCLES_NUMBER, 1);
        setTimer(FRAME_PERIOD, 1e9); // important to keep this until we have to wait for acquisition to start
        setTimer(ACQUISITION_TIME, 900 * 1000);

        // take an image
        if (masterflags == IS_MASTER)
            usleep(1 * 1000 * 1000); // required to ensure master starts acquisition only after slave has changed to basic parameters and is waiting

        int loop = 0;
        startStateMachine();
        // wait for acquisition to start (trigger from master)
        FILE_LOG(logINFO, ("\tWaiting for acquisition to start\n"));
        while(!runBusy()) {
            usleep(0);
            ++loop;
        }

        FILE_LOG(logINFO, ("\twaited %d loops to start\n", loop));
        FILE_LOG(logINFO, ("\tWaiting for acquisition to end (frames left: %lld)\n", (long long int)getTimeLeft(FRAME_NUMBER)));
    	// wait for status to be done
    	while(runBusy()){
    		usleep(500);
    	}

        // set to previous parameters
        FILE_LOG(logINFO, ("\tSetting previous parameters:\n"
                "\tTiming: %d\n"
                "\tframes: %lld\n"
                "\tcycles: %lld\n"
                "\tperiod: %lld ns\n"
                "\texptime:%lld ns\n",
                (int)oldtiming, (long long int)oldframes, (long long int)oldcycles,
                (long long int)oldPeriod, (long long int)oldExptime));
        setTiming(oldtiming);
        setTimer(FRAME_NUMBER, oldframes);
        setTimer(CYCLES_NUMBER, oldcycles);
        setTimer(FRAME_PERIOD, oldPeriod);
        setTimer(ACQUISITION_TIME, oldExptime);
        FILE_LOG(logINFOBLUE, ("Done sending a frame at configuration\n"));
    }
    return OK;
}

int getAdcConfigured(){
    return adcConfigured;
}


/* gotthard specific - loadimage, read/reset counter block */

void loadImage(enum imageType index, short int imageVals[]){
    u_int32_t addr = DARK_IMAGE_REG;
    if (index == GAIN_IMAGE)
        addr = GAIN_IMAGE_REG;
    int dataBytes = calculateDataBytes();

    volatile u_int16_t *ptr = (u_int16_t*)(CSP0BASE + addr * 2);
    memcpy((char*)ptr, (char*)imageVals, dataBytes);

    FILE_LOG(logINFO, ("Loaded %s image at 0x%p\n",
            (index == GAIN_IMAGE) ? "Gain" : "Dark", (void*) ptr));
}

int readCounterBlock(int startACQ, short int counterVals[]){
    FILE_LOG(logINFO, ("Reading Counter Block with start Acq :%d\n", startACQ));

    // stop any current acquisition
    if (runBusy()) {
        if (stopStateMachine() == FAIL)
            return FAIL;
        // waiting for the last frame read to be done
        while(runBusy())
            usleep(500);
        FILE_LOG(logDEBUG1, ("State machine stopped\n"));
    }

    // copy memory
    u_int32_t addr = COUNTER_MEMORY_REG;
    volatile u_int16_t *ptr = (u_int16_t*)(CSP0BASE + addr * 2);
    int dataBytes = calculateDataBytes();
    memcpy((char*)counterVals, (char*)ptr, dataBytes);

    // unreset counter
    addr = MULTI_PURPOSE_REG;
    bus_w(addr, (bus_r(addr) &~ RST_CNTR_MSK));
    FILE_LOG(logDEBUG1, ("\tUnsetting reset Counter. Multi Purpose Reg: 0x%x\n", bus_r(addr)));

    // start state machine
    if (startACQ == 1){
        startStateMachine();
        if (runBusy()) {
            FILE_LOG(logINFO, ("State machine RUNNING\n"));
        } else {
            FILE_LOG(logINFO, ("State machine IDLE\n"));
        }
    }
    return OK;
}

int resetCounterBlock(int startACQ){
    FILE_LOG(logINFO, ("Resetting Counter Block with start Acq :%d\n", startACQ));

    // stop any current acquisition
    if (runBusy()) {
        if (stopStateMachine() == FAIL)
            return FAIL;
        // waiting for the last frame read to be done
        while(runBusy())
            usleep(500);
        FILE_LOG(logDEBUG1, ("State machine stopped\n"));
    }

    // reset counter
    u_int32_t addr = MULTI_PURPOSE_REG;
    bus_w(addr, (bus_r(addr) | RST_CNTR_MSK));
    FILE_LOG(logDEBUG1, ("\tResetting Counter. Multi Purpose Reg: 0x%x\n", bus_r(addr)));

    // copy memory
    addr = COUNTER_MEMORY_REG;
    volatile u_int16_t *ptr = (u_int16_t*)(CSP0BASE + addr * 2);
    int dataBytes = calculateDataBytes();
    char *counterVals = NULL;
    counterVals = realloc(counterVals, dataBytes);
    memcpy((char*)counterVals, (char*)ptr, dataBytes);

    // unreset counter
    addr = MULTI_PURPOSE_REG;
    bus_w(addr, (bus_r(addr) &~ RST_CNTR_MSK));
    FILE_LOG(logDEBUG1, ("\tUnsetting reset Counter. Multi Purpose Reg: 0x%x\n", bus_r(addr)));

    // start state machine
    if (startACQ == 1){
        startStateMachine();
        if (runBusy()) {
            FILE_LOG(logINFO, ("State machine RUNNING\n"));
        } else {
            FILE_LOG(logINFO, ("State machine IDLE\n"));
        }
    }

    if (sizeof(counterVals) <= 0){
        FILE_LOG(logERROR, ("\tSize of counterVals: %d\n", (int)sizeof(counterVals)));
        return FAIL;
    }

    return OK;
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
	FILE_LOG(logINFO, ("#frames to acquire:%lld\n", (long long int)setTimer(FRAME_NUMBER, -1)));

	cleanFifos();

	//start state machine
	bus_w16(CONTROL_REG, CONTROL_STRT_ACQ_MSK | CONTROL_STRT_EXPSR_MSK);
	bus_w16(CONTROL_REG, 0x0);
	runState(logINFO);
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
	bus_w16(CONTROL_REG, CONTROL_STP_ACQ_MSK);
	usleep(100);
	bus_w16(CONTROL_REG, 0x0);

	// check
	usleep(500);
    if ((runState(logDEBUG1) & STATUS_RN_MSHN_BSY_MSK)) {
        FILE_LOG(logERROR, ("\tFailed to stop state machine.\n"));
        runState(logINFORED);
        return FAIL;
    }

    runState(logINFO);
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

	enum runStatus s = IDLE;
	u_int32_t retval = runState(logINFO);

	// finished (external stop or fifo full)
	if (retval & STATUS_RN_FNSHD_MSK) {
	    FILE_LOG(logINFORED, ("Status: Stopped\n"));
	    s = STOPPED;

	    FILE_LOG(logINFO, ("\t Reading status reg again\n"));
        retval = runState(logINFO);
        // fifo full
        if (runState(logDEBUG1) & STATUS_RN_FNSHD_MSK) {
            FILE_LOG(logINFORED, ("Status: Error\n"));
            runState(logINFORED);
            s = ERROR;
        }
	}

	// error (fifo full)
	else if (retval & STATUS_SM_FF_FLL_MSK) {
        FILE_LOG(logINFORED, ("Status: Error\n"));
        s = ERROR;
	}

	// not running
	else if (!(retval & STATUS_RN_BSY_MSK)) {
	    // read last frames
	    if (retval & STATUS_RD_MSHN_BSY_MSK) {
	        FILE_LOG(logINFOBLUE, ("Status: Read Machine Busy\n"));
	        s = TRANSMITTING;
	    }
	    // ???
	    else if (retval & STATUS_ALL_FF_EMPTY_MSK) {
            FILE_LOG(logINFOBLUE, ("Status: Data in Fifo\n"));
            s = TRANSMITTING;
	    }
	    // idle, unknown
	    else if (!(retval & STATUS_IDLE_MSK)) {
            FILE_LOG(logINFOBLUE, ("Status: IDLE\n"));
            s = IDLE;
	    } else {
            FILE_LOG(logINFORED, ("Status: Unknown Status: 0x%x. Trying again.\n", retval));
            int iloop = 0;
            for (iloop = 0; iloop < 10; ++iloop) {
                usleep(1000 * 1000);
                if (runState(logDEBUG1) != retval)
                    return getRunStatus();
            }
           s = ERROR;
	    }
	}

	// running
	else {
	    if (retval & STATUS_WTNG_FR_TRGGR_MSK){
	        FILE_LOG(logINFOBLUE, ("Status: Waiting\n"));
	        s = WAITING;
	    }
	    else{
	        FILE_LOG(logINFOBLUE, ("Status: Running\n"));
	        s = RUNNING;
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
	if ( retval > -1) {
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
	return runState(logDEBUG1) & STATUS_RN_BSY_MSK;
}

u_int32_t runState(enum TLogLevel lev) {
#ifdef VIRTUAL
    return virtual_status;
#endif
    u_int32_t s = bus_r(STATUS_REG);
    FILE_LOG(lev, ("Status Register: 0x%08x\n", s));
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


