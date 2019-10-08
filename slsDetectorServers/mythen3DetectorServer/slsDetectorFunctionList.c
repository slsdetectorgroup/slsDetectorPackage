#include "slsDetectorFunctionList.h"
#include "versionAPI.h"
#include "clogger.h"
#include "nios.h"
#include "DAC6571.h"
#include "LTC2620_Driver.h"
#include "common.h"
#include "RegisterDefs.h"

#ifdef VIRTUAL
#include "communication_funcs_UDP.h"
#endif

#include <string.h>
#include <unistd.h>     // usleep
#ifdef VIRTUAL
#include <pthread.h>
#include <time.h>
#endif

extern udpStruct udpDetails;

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

int32_t clkPhase[NUM_CLOCKS] = {0, 0, 0};
uint32_t clkDivider[NUM_CLOCKS] = {125, 20, 80};

int highvoltage = 0;
int dacValues[NDAC] = {0};
int detPos[2] = {0, 0};

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
    FILE_LOG(logINFOBLUE, ("******** Mythen3 Virtual Server *****************\n"));
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
	FILE_LOG(logINFOBLUE, ("******** Mythen3 Server: do the checks *****************\n"));
	if (mapCSP0() == FAIL) {
    	strcpy(firmware_message,
				"Could not map to memory. Dangerous to continue.\n");
		FILE_LOG(logERROR, ("%s\n\n", firmware_message));
		firmware_compatibility = FAIL;
		firmware_check_done = 1;
		return;
    }
	// does check only if flag is 0 (by default), set by command line
	if ((!debugflag) && ((testFpga() == FAIL)|| (testBus() == FAIL))) {
		strcpy(firmware_message,
				"Could not pass basic tests of FPGA and bus. Dangerous to continue.\n");
		FILE_LOG(logERROR, ("%s\n\n", firmware_message));
		firmware_compatibility = FAIL;
		firmware_check_done = 1;
		return;
	}
	uint16_t hversion			= getHardwareVersionNumber();
	uint32_t ipadd				= getDetectorIP();
	uint64_t macadd				= getDetectorMAC();
	int64_t fwversion 			= getDetectorId(DETECTOR_FIRMWARE_VERSION);
	int64_t swversion 			= getDetectorId(DETECTOR_SOFTWARE_VERSION);
	int64_t sw_fw_apiversion    = 0;
	int64_t client_sw_apiversion = getDetectorId(CLIENT_SOFTWARE_API_VERSION);
	uint32_t requiredFirmwareVersion = REQRD_FRMWRE_VRSN;


	if (fwversion >= MIN_REQRD_VRSN_T_RD_API)
	    sw_fw_apiversion 	    = getDetectorId(SOFTWARE_FIRMWARE_API_VERSION);
	FILE_LOG(logINFOBLUE, ("************ Mythen3 Server *********************\n"
			"Hardware Version:\t\t 0x%x\n"

			"Detector IP Addr:\t\t 0x%x\n"
			"Detector MAC Addr:\t\t 0x%llx\n\n"

			"Firmware Version:\t\t 0x%llx\n"
			"Software Version:\t\t 0x%llx\n"
			"F/w-S/w API Version:\t\t 0x%llx\n"
			"Required Firmware Version:\t 0x%x\n"
			"Client-Software API Version:\t 0x%llx\n"
			"********************************************************\n",
			hversion, 
			ipadd,
			(long  long unsigned int)macadd,
			(long  long int)fwversion,
			(long  long int)swversion,
			(long  long int)sw_fw_apiversion,
			requiredFirmwareVersion,
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
	if(sw_fw_apiversion > requiredFirmwareVersion){
		sprintf(firmware_message,
				"This detector software software version (0x%llx) is incompatible.\n"
				"Please update detector software (min. 0x%llx) to be compatible with this firmware.\n",
				(long long int)sw_fw_apiversion,
				(long long int)requiredFirmwareVersion);
		FILE_LOG(logERROR, (firmware_message));
		firmware_compatibility = FAIL;
		firmware_check_done = 1;
		return;
	}

	//check for firmware compatibility - old firmware
	if( requiredFirmwareVersion > fwversion) {
		sprintf(firmware_message,
				"This firmware version (0x%llx) is incompatible.\n"
				"Please update firmware (min. 0x%llx) to be compatible with this server.\n",
				(long long int)fwversion,
				(long long int)requiredFirmwareVersion);
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
	if (type != MYTHEN3){
		FILE_LOG(logERROR, ("This is not a Mythen3 Server (read %d, expected %d)\n", type, MYTHEN3));
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
	u_int32_t addr = DTA_OFFSET_REG;
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
		return APIMYTHEN3;
	default:
		return retval;
	}
}

u_int64_t getFirmwareVersion() {
#ifdef VIRTUAL
    return 0;
#endif
	return ((bus_r(FPGA_VERSION_REG) & FPGA_COMPILATION_DATE_MSK) >> FPGA_COMPILATION_DATE_OFST);
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
	return bus_r(MCB_SERIAL_NO_REG);
}

u_int32_t getDetectorNumber(){
#ifdef VIRTUAL
    return 0;
#endif
	return bus_r(MCB_SERIAL_NO_REG);
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
    FILE_LOG(logINFO, ("This Server is for 1 Mythen3 module \n")); 

	clkDivider[RUN_CLK] = DEFAULT_RUN_CLK;
	clkDivider[TICK_CLK] = DEFAULT_TICK_CLK;
	clkDivider[SAMPLING_CLK] = DEFAULT_SAMPLING_CLK;

	highvoltage = 0;

#ifndef VIRTUAL
	// hv
   	DAC6571_SetDefines(HV_HARD_MAX_VOLTAGE, HV_DRIVER_FILE_NAME);
	//dac
	LTC2620_D_SetDefines(DAC_MAX_MV, DAC_DRIVER_FILE_NAME, NDAC);
#endif
    setHighVoltage(DEFAULT_HIGH_VOLTAGE);
	setDefaultDacs();
	
	// Initialization of acquistion parameters
	setTimer(FRAME_NUMBER, DEFAULT_NUM_FRAMES);
	setTimer(TRIGGER_NUMBER, DEFAULT_NUM_CYCLES);

	setTimer(ACQUISITION_TIME, DEFAULT_EXPTIME);
	setTimer(FRAME_PERIOD, DEFAULT_PERIOD);
	setTimer(DELAY_AFTER_TRIGGER, DEFAULT_DELAY_AFTER_TRIGGER);
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

/* set parameters -  dr, roi */

int setDynamicRange(int dr){
	return -1;
}


/* parameters - speed, readout */

void setSpeed(enum speedVariable ind, int val) {

}

int getSpeed(enum speedVariable ind) {
    return -1;
}

int64_t setTimer(enum timerIndex ind, int64_t val) {

	int64_t retval = -1;

	switch(ind){

	case FRAME_NUMBER: // defined in sls_detector_defs.h (general)
		if(val >= 0) {
			FILE_LOG(logINFO, ("Setting #frames: %lld\n",(long long int)val));
		}
		retval = set64BitReg(val,  SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG); // defined in my RegisterDefs.h
		FILE_LOG(logDEBUG1, ("Getting #frames: %lld\n", (long long int)retval));
		break;
	
	case ACQUISITION_TIME:
		if(val >= 0){
			FILE_LOG(logINFO, ("Setting exptime (pattern wait time level 0): %lldns\n",(long long int)val));
			val *= (1E-3 * clkDivider[RUN_CLK]);
			setPatternWaitTime(0, val);
		}
		retval = setPatternWaitTime(0, -1) / (1E-3 * clkDivider[RUN_CLK]);
		FILE_LOG(logINFO, ("\tGetting exptime (pattern wait time level 0): %lldns\n", (long long int)retval));
		FILE_LOG(logDEBUG1, ("Getting exptime (pattern wait time level 0): %lldns\n", (long long int)retval));
		break;

	case FRAME_PERIOD:
		if(val >= 0){
			FILE_LOG(logINFO, ("Setting period: %lldns\n",(long long int)val));
			val *= (1E-3 * TICK_CLK);
		}
		retval = set64BitReg(val, SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG )/ (1E-3 * TICK_CLK);
		FILE_LOG(logDEBUG1, ("Getting period: %lldns\n", (long long int)retval));
		break;

	case DELAY_AFTER_TRIGGER:
		if(val >= 0){
			FILE_LOG(logINFO, ("Setting delay: %lldns\n", (long long int)val));
			val *= (1E-3 * clkDivider[TICK_CLK]);
		}
		retval = set64BitReg(val, GET_DELAY_LSB_REG, GET_DELAY_MSB_REG) / (1E-3 * clkDivider[TICK_CLK]);
		FILE_LOG(logINFO, ("\tGetting delay: %lldns\n", (long long int)retval));
		break;

	case TRIGGER_NUMBER:
		if(val >= 0) {
			FILE_LOG(logINFO, ("Setting #triggers: %lld\n", (long long int)val));
		}
		retval = set64BitReg(val,  SET_CYCLES_LSB_REG, SET_CYCLES_MSB_REG);
		FILE_LOG(logDEBUG1, ("Getting #triggers: %lld\n", (long long int)retval));
		break;

	default:
		FILE_LOG(logERROR, ("Timer Index not implemented for this detector: %d\n", ind));
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
        val *= (1E-3 * RUN_CLK);
        // convert back to timer
        val = (val) / (1E-3 * RUN_CLK);
        if (val != retval)
            return FAIL;
        break;
    case FRAME_PERIOD:
	    case DELAY_AFTER_TRIGGER:
		// convert to freq
        val *= (1E-3 * TICK_CLK);
        // convert back to timer
        val = (val) / (1E-3 * TICK_CLK);
        if (val != retval)
            return FAIL;
        break;
    default:
        break;
    }
    return OK; 

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

	case TRIGGER_NUMBER:
		retval = get64BitReg(GET_CYCLES_LSB_REG, GET_CYCLES_MSB_REG);
		FILE_LOG(logINFO, ("Getting number of triggers left: %lld\n", (long long int)retval));
		break;

	default:
		FILE_LOG(logERROR, ("Remaining Timer index not implemented for this detector: %d\n", ind));
		break;
	}
	return retval;
}


/* parameters - dac, hv */
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
    else if (LTC2620_D_VoltageToDac(val, &dacval) == OK) {
        dacValues[ind] = dacval;
    }
#else
    if (LTC2620_D_SetDACValue((int)ind, val, mV, &dacval) == OK) {
        dacValues[ind] = dacval;
    }
#endif
}

int getDAC(enum DACINDEX ind, int mV) {
    if (!mV) {
        FILE_LOG(logDEBUG1, ("Getting DAC %d : %d dac\n",ind, dacValues[ind]));
        return dacValues[ind];
    }
    int voltage = -1;
    LTC2620_D_DacToVoltage(dacValues[ind], &voltage);
    FILE_LOG(logDEBUG1, ("Getting DAC %d : %d dac (%d mV)\n",ind, dacValues[ind], voltage));
    return voltage;
}

int getMaxDacSteps() {
    return LTC2620_D_GetMaxNumSteps();
}

int setHighVoltage(int val){
	// limit values 
    if (val > HV_SOFT_MAX_VOLTAGE ) {
        val = HV_SOFT_MAX_VOLTAGE ;
    }
#ifdef VIRTUAL
    if (val >= 0)
        highvoltage = val;
    return highvoltage;
#endif

	// setting hv
	if (val >= 0) {
	    FILE_LOG(logINFO, ("Setting High voltage: %d V", val));
	    DAC6571_Set(val);
	    highvoltage = val;
	}
	return highvoltage;
}


int configureMAC() {

	uint32_t srcip = udpDetails.srcip;
	uint32_t dstip = udpDetails.dstip;
	uint64_t srcmac = udpDetails.srcmac;
	uint64_t dstmac = udpDetails.dstmac;
	int srcport = udpDetails.srcport;
	int dstport = udpDetails.dstport;

#ifdef VIRTUAL
	char cDestIp[MAX_STR_LENGTH];
	memset(cDestIp, 0, MAX_STR_LENGTH);
	sprintf(cDestIp, "%d.%d.%d.%d", (dstip>>24)&0xff,(dstip>>16)&0xff,(dstip>>8)&0xff,(dstip)&0xff);
	FILE_LOG(logINFO, ("1G UDP: Destination (IP: %s, port:%d)\n", cDestIp, dstport));
	if (setUDPDestinationDetails(0, cDestIp, dstport) == FAIL) {
		FILE_LOG(logERROR, ("could not set udp destination IP and port\n"));
		return FAIL;
	}
#endif
	FILE_LOG(logINFOBLUE, ("Configuring MAC\n"));
	
	FILE_LOG(logINFO, ("\tSource IP   : %d.%d.%d.%d \t\t(0x%08x)\n",
	        (srcip>>24)&0xff,(srcip>>16)&0xff,(srcip>>8)&0xff,(srcip)&0xff, srcip));
	FILE_LOG(logINFO, ("\tSource MAC  : %02x:%02x:%02x:%02x:%02x:%02x \t(0x%010llx)\n",
			(unsigned int)((srcmac>>40)&0xFF),
			(unsigned int)((srcmac>>32)&0xFF),
			(unsigned int)((srcmac>>24)&0xFF),
			(unsigned int)((srcmac>>16)&0xFF),
			(unsigned int)((srcmac>>8)&0xFF),
			(unsigned int)((srcmac>>0)&0xFF),
			(long  long unsigned int)srcmac));
	FILE_LOG(logINFO, ("\tSource Port : %d \t\t\t(0x%08x)\n", srcport, srcport));

	FILE_LOG(logINFO, ("\tDest. IP    : %d.%d.%d.%d \t\t(0x%08x)\n",
	        (dstip>>24)&0xff,(dstip>>16)&0xff,(dstip>>8)&0xff,(dstip)&0xff, dstip));
	FILE_LOG(logINFO, ("\tDest. MAC   : %02x:%02x:%02x:%02x:%02x:%02x \t(0x%010llx)\n",
			(unsigned int)((dstmac>>40)&0xFF),
			(unsigned int)((dstmac>>32)&0xFF),
			(unsigned int)((dstmac>>24)&0xFF),
			(unsigned int)((dstmac>>16)&0xFF),
			(unsigned int)((dstmac>>8)&0xFF),
			(unsigned int)((dstmac>>0)&0xFF),
			(long  long unsigned int)dstmac));
	FILE_LOG(logINFO, ("\tDest. Port  : %d \t\t\t(0x%08x)\n\n",dstport, dstport));

	// start addr
	uint32_t addr = BASE_UDP_RAM;
	// calculate rxr endpoint offset
	//addr += (iRxEntry * RXR_ENDPOINT_OFST);//TODO: is there round robin already implemented?
	// get struct memory
	udp_header *udp = (udp_header*) (Nios_getBaseAddress() + addr/(sizeof(u_int32_t)));
	memset(udp, 0, sizeof(udp_header));

	//  mac addresses	
	// msb (32) + lsb (16)
	udp->udp_destmac_msb	= ((dstmac >> 16) & BIT32_MASK);
	udp->udp_destmac_lsb	= ((dstmac >> 0) & BIT16_MASK);
	// msb (16) + lsb (32)
	udp->udp_srcmac_msb		= ((srcmac >> 32) & BIT16_MASK);
	udp->udp_srcmac_lsb		= ((srcmac >> 0) & BIT32_MASK);

	// ip addresses
	udp->ip_srcip_msb		= ((srcip >> 16) & BIT16_MASK);
	udp->ip_srcip_lsb		= ((srcip >> 0) & BIT16_MASK);	
	udp->ip_destip_msb		= ((dstip >> 16) & BIT16_MASK);
	udp->ip_destip_lsb		= ((dstip >> 0) & BIT16_MASK);	

	// source port
	udp->udp_srcport 		= srcport;
	udp->udp_destport		= dstport;

	// other defines
	udp->udp_ethertype		= 0x800;
	udp->ip_ver				= 0x4;
	udp->ip_ihl				= 0x5;
	udp->ip_flags			= 0x2; //FIXME
	udp->ip_ttl           	= 0x40;
	udp->ip_protocol      	= 0x11;
	// total length is redefined in firmware

	// calcChecksum(udp);

	//TODO?
	//cleanFifos();
	//resetCore();
	//alignDeserializer();
	return OK;
}

void calcChecksum(udp_header* udp) {
	int count = IP_HEADER_SIZE;
	long int sum = 0;
	
	// start at ip_tos as the memory is not continous for ip header
	uint16_t *addr = (uint16_t*) (&(udp->ip_tos)); 

	sum += *addr++;
	count -= 2;

	// ignore ethertype (from udp header)
	addr++;

	// from identification to srcip_lsb
    while( count > 2 )  {
		sum += *addr++;
		count -= 2;
	}

	// ignore src udp port (from udp header)
	addr++;
	
	if (count > 0)
	    sum += *addr;                     // Add left-over byte, if any
	while (sum >> 16)
	    sum = (sum & 0xffff) + (sum >> 16);// Fold 32-bit sum to 16 bits
	long int checksum = sum & 0xffff;
	checksum += UDP_IP_HEADER_LENGTH_BYTES;
	FILE_LOG(logINFO, ("\tIP checksum is 0x%lx\n",checksum));
	udp->ip_checksum = checksum;
}

/* pattern */

uint64_t readPatternWord(int addr) {
    // error (handled in tcp)
    if (addr < 0 || addr >= MAX_PATTERN_LENGTH) {
        FILE_LOG(logERROR, ("Cannot get Pattern - Word. Invalid addr 0x%x. "
                "Should be between 0 and 0x%x\n", addr, MAX_PATTERN_LENGTH));
        return -1;
    }

    FILE_LOG(logINFORED, ("  Reading Pattern Word (addr:0x%x)\n", addr));
    uint32_t reg_lsb = PATTERN_STEP0_LSB_REG + addr * REG_OFFSET * 2; // the first word in RAM as base plus the offset of the word to write (addr)
	uint32_t reg_msb = PATTERN_STEP0_MSB_REG + addr * REG_OFFSET * 2;

    // read value
    uint64_t retval = get64BitReg(reg_lsb, reg_msb);
    FILE_LOG(logDEBUG1, ("  Word(addr:0x%x) retval: 0x%llx\n", addr, (long long int) retval));

    return retval;
}

uint64_t writePatternWord(int addr, uint64_t word) {
    // get
    if (word == -1)
        return readPatternWord(addr);

    // error (handled in tcp)
    if (addr < 0 || addr >= MAX_PATTERN_LENGTH) {
        FILE_LOG(logERROR, ("Cannot set Pattern - Word. Invalid addr 0x%x. "
                "Should be between 0 and 0x%x\n", addr, MAX_PATTERN_LENGTH));
        return -1;
    }

    FILE_LOG(logINFO, ("Setting Pattern Word (addr:0x%x, word:0x%llx)\n", addr, (long long int) word));
    uint32_t reg_lsb = PATTERN_STEP0_LSB_REG + addr * REG_OFFSET * 2; // the first word in RAM as base plus the offset of the word to write (addr)
	uint32_t reg_msb = PATTERN_STEP0_MSB_REG + addr * REG_OFFSET * 2;

    // write word
    set64BitReg(word, reg_lsb, reg_msb);
    FILE_LOG(logDEBUG1, ("  Wrote word. PatternIn Reg: 0x%llx\n", get64BitReg(reg_lsb, reg_msb)));

    return readPatternWord(addr);
}

int setPatternWaitAddress(int level, int addr) {

    // error (handled in tcp)
    if (addr >= MAX_PATTERN_LENGTH) {
        FILE_LOG(logERROR, ("Cannot set Pattern Wait Address. Invalid addr 0x%x. "
                "Should be between 0 and 0x%x\n", addr, MAX_PATTERN_LENGTH));
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
        FILE_LOG(logERROR, ("Cannot set Pattern Wait Address. Invalid level 0x%x. "
                "Should be between 0 and 2.\n", level));
        return -1;
    }

    // set
    if (addr >= 0) {
        FILE_LOG(logINFO, ("Setting Pattern Wait Address (level:%d, addr:0x%x)\n", level, addr));
        bus_w(reg, ((addr << offset) & mask));
    }

    // get
    uint32_t regval = ((bus_r(reg) & mask) >> offset);
    FILE_LOG(logDEBUG1, ("  Wait Address retval (level:%d, addr:0x%x)\n", level, regval));
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
        FILE_LOG(logERROR, ("Cannot set Pattern Wait Time. Invalid level %d. "
                "Should be between 0 and 2.\n", level));
        return -1;
    }

    // set
    if (t >= 0) {
        FILE_LOG(logINFO, ("Setting Pattern Wait Time (level:%d, t:%lld)\n", level, (long long int)t));
        set64BitReg(t, regl, regm);
    }

    // get
    uint64_t regval = get64BitReg(regl, regm);
    FILE_LOG(logDEBUG1, ("  Wait Time retval (level:%d, t:%lld)\n", level, (long long int)regval));
    return regval;
}

void setPatternLoop(int level, int *startAddr, int *stopAddr, int *nLoop) {

    // (checked at tcp)
     if (*startAddr >= MAX_PATTERN_LENGTH || *stopAddr >= MAX_PATTERN_LENGTH) {
        FILE_LOG(logERROR, ("Cannot set Pattern Loop, Address (startaddr:0x%x, stopaddr:0x%x) must be "
                "less than 0x%x\n",
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
        FILE_LOG(logERROR, ("Cannot set Pattern loop. Invalid level %d. "
                "Should be between -1 and 2.\n", level));
        *startAddr = 0;
        *stopAddr = 0;
        *nLoop = 0;
    }

    // set iterations
    if (level >= 0) {
        // set iteration
        if (*nLoop >= 0) {
            FILE_LOG(logINFO, ("Setting Pattern Loop (level:%d, nLoop:%d)\n",
                      level, *nLoop));
            bus_w(nLoopReg, *nLoop);
        }
        *nLoop = bus_r(nLoopReg);
    }

    // set
    if (*startAddr >= 0 && *stopAddr >= 0) {
    	// writing start and stop addr
    	FILE_LOG(logINFO, ("Setting Pattern Loop (level:%d, startaddr:0x%x, stopaddr:0x%x)\n",
    			level, *startAddr, *stopAddr));
    	bus_w(addr, ((*startAddr << startOffset) & startMask) | ((*stopAddr << stopOffset) & stopMask));
    	FILE_LOG(logDEBUG1, ("Addr:0x%x, val:0x%x\n", addr, bus_r(addr)));
    }

    // get
    else {
    	*startAddr = ((bus_r(addr)  & startMask) >> startOffset);
    	FILE_LOG(logDEBUG1, ("Getting Pattern Loop Start Address (level:%d, Read startAddr:0x%x)\n",
    			level, *startAddr));

    	*stopAddr = ((bus_r(addr) & stopMask) >> stopOffset);
    	FILE_LOG(logDEBUG1, ("Getting Pattern Loop Stop Address (level:%d, Read stopAddr:0x%x)\n",
    			level, *stopAddr));
    }
}



/* aquisition */

int setDetectorPosition(int pos[]) {
    memcpy(detPos, pos, sizeof(detPos));
	return OK;
}

int* getDetectorPosition() {
    return detPos;
}

int startStateMachine(){
#ifdef VIRTUAL
	// create udp socket
	if(createUDPSocket(0) != OK) {
		return FAIL;
	}
	FILE_LOG(logINFOBLUE, ("starting state machine\n"));
	// set status to running
	virtual_status = 1;
	virtual_stop = 0;
	if(pthread_create(&pthread_virtual_tid, NULL, &start_timer, NULL)) {
		FILE_LOG(logERROR, ("Could not start Virtual acquisition thread\n"));
		virtual_status = 0;
		return FAIL;
	}
	FILE_LOG(logINFOGREEN, ("Virtual Acquisition started\n"));
	return OK;
#endif
    return OK;
}




#ifdef VIRTUAL
void* start_timer(void* arg) {
	int64_t periodns = setTimer(FRAME_PERIOD, -1);
	int numFrames = (setTimer(FRAME_NUMBER, -1) *
						setTimer(TRIGGER_NUMBER, -1) );
	int64_t exp_ns = 	setTimer(ACQUISITION_TIME, -1);


    int frameNr = 0;
	// loop over number of frames
    for(frameNr=0; frameNr!= numFrames; ++frameNr ) {

		//check if virtual_stop is high
		if(virtual_stop == 1){
			break;
		}
		// sleep for exposure time
        struct timespec begin, end;
        clock_gettime(CLOCK_REALTIME, &begin);
        usleep(exp_ns / 1000);
        clock_gettime(CLOCK_REALTIME, &end);

		// calculate time left in period
        int64_t time_ns = ((end.tv_sec - begin.tv_sec) * 1E9 +
                (end.tv_nsec - begin.tv_nsec));

		// sleep for (period - exptime)
        if (periodns > time_ns) {
            usleep((periodns - time_ns)/ 1000);
        }

		// set register frames left
    }

	closeUDPSocket(0);
	// set status to idle
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
    return IDLE;
}

void readFrame(int *ret, char *mess){
	// wait for status to be done
	while(runBusy()){
		usleep(500);
	}
#ifdef VIRTUAL
	FILE_LOG(logINFOGREEN, ("acquisition successfully finished\n"));
	return;
#endif
}

u_int32_t runBusy() {
#ifdef VIRTUAL
    return virtual_status;
#endif
#ifdef VIRTUAL
	u_int32_t s = (bus_r(STATUS_REG) & RUN_BUSY_MSK);
	FILE_LOG(logDEBUG1, ("Status Register: %08x\n", s));
	return s;
#endif
	return -1;
}

/* common */

int calculateDataBytes(){
	return 0;
}

int getTotalNumberOfChannels(){return  ((int)getNumberOfChannelsPerChip() * (int)getNumberOfChips());}
int getNumberOfChips(){return  NCHIP;}
int getNumberOfDACs(){return  NDAC;}
int getNumberOfChannelsPerChip(){return  NCHAN;}
