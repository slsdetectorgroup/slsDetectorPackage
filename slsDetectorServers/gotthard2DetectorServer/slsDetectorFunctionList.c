#include "slsDetectorFunctionList.h"
#include "RegisterDefs.h"
#include "versionAPI.h"
#include "clogger.h"
#include "nios.h"
#include "DAC6571.h"
#include "LTC2620_Driver.h"
#include "common.h"
#include "ALTERA_PLL_CYCLONE10.h" // pll
#ifdef VIRTUAL
#include "communication_funcs_UDP.h"
#endif

#include <string.h>
#include <unistd.h>     // usleep
#ifdef VIRTUAL
#include <pthread.h>
#include <time.h>
#endif

enum {READOUT_PLL, SYSTEM_PLL};


// Global variable from slsDetectorServer_funcs
extern int debugflag;
extern udpStruct udpDetails;

int firmware_compatibility = OK;
int firmware_check_done = 0;
char firmware_message[MAX_STR_LENGTH];

#ifdef VIRTUAL
pthread_t pthread_virtual_tid;
int virtual_status = 0;
int virtual_stop = 0;
#endif

int32_t clkPhase[NUM_CLOCKS] = {0, 0, 0, 0, 0, 0};
uint32_t clkDivider[NUM_CLOCKS] = {0, 0, 0, 0, 0, 0};
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
    FILE_LOG(logINFOBLUE, ("******** Gotthard2 Virtual Server *****************\n"));
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
	if ((!debugflag) && ((testFpga() == FAIL) || (testBus() == FAIL))) {
		sprintf(firmware_message,
				"Could not pass basic tests of FPGA and bus. Dangerous to continue. (Firmware version:0x%llx) \n", getDetectorId(DETECTOR_FIRMWARE_VERSION));
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
	int64_t sw_fw_apiversion    = getDetectorId(SOFTWARE_FIRMWARE_API_VERSION);
	int64_t client_sw_apiversion = getDetectorId(CLIENT_SOFTWARE_API_VERSION);
	uint32_t requiredFirmwareVersion = REQRD_FRMWRE_VRSN;

	FILE_LOG(logINFOBLUE, ("************ Gotthard2 Server *********************\n"
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
	if (type != GOTTHARD2){
			FILE_LOG(logERROR, ("This is not a Gotthard2 Server (read %d, expected %d)\n", type, GOTTHARD2));
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
		return APIGOTTHARD2;
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
	return ((bus_r(MCB_SERIAL_NO_REG)));// & HARDWARE_VERSION_NUM_MSK) >> HARDWARE_VERSION_NUM_OFST);
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
    FILE_LOG(logINFO, ("This Server is for 1 Gotthard2 module \n")); 

	clkDivider[READOUT_C0] = DEFAULT_READOUT_C0;
	clkDivider[READOUT_C1] = DEFAULT_READOUT_C1;
	clkDivider[SYSTEM_C0] = DEFAULT_SYSTEM_C0;
	clkDivider[SYSTEM_C1] = DEFAULT_SYSTEM_C1;
	clkDivider[SYSTEM_C2] = DEFAULT_SYSTEM_C2;
	clkDivider[SYSTEM_C3] = DEFAULT_SYSTEM_C3;


	highvoltage = 0;
	{
		int i;
		for (i = 0; i < NUM_CLOCKS; ++i) {
            clkPhase[i] = 0;
        }
		for (i = 0; i < NDAC; ++i) {
			dacValues[i] = 0;
		}
	}


#ifndef VIRTUAL
	// pll defines
	ALTERA_PLL_C10_SetDefines(REG_OFFSET, BASE_READOUT_PLL, BASE_SYSTEM_PLL, READOUT_PLL_RESET_REG, SYSTEM_PLL_RESET_REG, READOUT_PLL_RESET_MSK, SYSTEM_PLL_RESET_MSK, READOUT_PLL_WAIT_REG, SYSTEM_PLL_WAIT_REG, READOUT_PLL_WAIT_MSK, SYSTEM_PLL_WAIT_MSK, READOUT_PLL_VCO_FREQ_HZ, SYSTEM_PLL_VCO_FREQ_HZ);
	ALTERA_PLL_C10_ResetPLL(READOUT_PLL);
	ALTERA_PLL_C10_ResetPLL(SYSTEM_PLL);
	// hv
    DAC6571_SetDefines(HV_HARD_MAX_VOLTAGE, HV_DRIVER_FILE_NAME);
	// dacs
	LTC2620_D_SetDefines(DAC_MAX_MV, DAC_DRIVER_FILE_NAME, NDAC);
#endif

	// Default values
    setHighVoltage(DEFAULT_HIGH_VOLTAGE);
	setDefaultDacs();
	setTimer(FRAME_NUMBER, DEFAULT_NUM_FRAMES);
	setTimer(TRIGGER_NUMBER, DEFAULT_NUM_CYCLES);
	setTimer(ACQUISITION_TIME, DEFAULT_EXPTIME);
	setTimer(ACQUISITION_TIME, DEFAULT_PERIOD);
}

int setDefaultDacs() {
	int ret = OK;
	FILE_LOG(logINFOBLUE, ("Setting Default Dac values\n"));
	{
		int i = 0;
		const int defaultvals[NDAC] = DEFAULT_DAC_VALS;
		for(i = 0; i < NDAC; ++i) {
			// if not already default, set it to default
			//if (dacValues[i] != defaultvals[i]) {
				setDAC((enum DACINDEX)i,defaultvals[i],0);
			//}
		}
	}
	return ret;
}

/* set parameters -  dr, roi */

int setDynamicRange(int dr){
	return DYNAMIC_RANGE;
}


/* parameters  */


int64_t setTimer(enum timerIndex ind, int64_t val) {

	int64_t retval = -1;

	switch(ind) {

	case FRAME_NUMBER: // defined in sls_detector_defs.h (general)
		if(val >= 0) {
			FILE_LOG(logINFO, ("Setting #frames: %lld\n",(long long int)val));
		}
		retval = set64BitReg(val,  SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG); // defined in my RegisterDefs.h
		FILE_LOG(logDEBUG1, ("Getting #frames: %lld\n", (long long int)retval));
		break;

	case ACQUISITION_TIME:
		if(val >= 0) {
			FILE_LOG(logINFO, ("Setting exptime: %lldns\n", (long long int)val));
			val *= (1E-9 * READOUT_C0); //TODO
		}
		retval = set64BitReg(val, SET_EXPTIME_LSB_REG, SET_EXPTIME_MSB_REG) / (1E-9 * READOUT_C0); //TODO
		FILE_LOG(logDEBUG1, ("Getting exptime: %lldns\n", (long long int)retval));
		break;

	case FRAME_PERIOD:
		if(val >= 0){
			FILE_LOG(logINFO, ("Setting period: %lldns\n",(long long int)val));
			val *= (1E-9 * SYSTEM_C0);//TODO
		}
		retval = set64BitReg(val, SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG )/ (1E-9 * SYSTEM_C0); //TODO
		FILE_LOG(logDEBUG1, ("Getting period: %lldns\n", (long long int)retval));
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
        val *= (1E-9 * READOUT_C0);//TODO
        // convert back to timer
        val = (val) / (1E-9 * READOUT_C0);//TODO
        if (val != retval)
            return FAIL;
        break;
    case FRAME_PERIOD:
		// convert to freq
        val *= (1E-9 * SYSTEM_C0);//TODO
        // convert back to timer
        val = (val) / (1E-9 * SYSTEM_C0);//TODO
        if (val != retval)
            return FAIL;
        break;
    default:
        break;
    }
    return OK;
}


int64_t getTimeLeft(enum timerIndex ind){
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
    if (val < 0) {
        return;
	}

	char* dac_names[] = {DAC_NAMES};
    FILE_LOG(logDEBUG1, ("Setting dac[%d - %s]: %d %s \n", (int)ind, dac_names[ind], val, (mV ? "mV" : "dac units")));
    int dacval = val;
#ifdef VIRTUAL
    FILE_LOG(logINFO, ("Setting dac[%d - %s]: %d %s \n", (int)ind, dac_names[ind], val, (mV ? "mV" : "dac units")));
    if (!mV) {
        dacValues[ind] = val;
    }
    // convert to dac units
    else if (LTC2620_D_VoltageToDac(val, &dacval) == OK) {
        dacValues[ind] = dacval;
    }
#else
    if (LTC2620_D_SetDACValue((int)ind, val, mV, dac_names[ind], &dacval) == OK) {
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
	if (val > HV_SOFT_MAX_VOLTAGE) {
		val = HV_SOFT_MAX_VOLTAGE;
	}
	
#ifdef VIRTUAL
    if (val >= 0)
        highvoltage = val;
    return highvoltage;
#endif

	// setting hv
	if (val >= 0) {
	    FILE_LOG(logINFO, ("Setting High voltage: %d V\n", val));
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
    return OK;
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

	calcChecksum(udp);

	//TODO?
	//cleanFifos();
	//resetCore();

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

// Detector Specific
int setPhase(enum CLKINDEX ind, int val, int degrees) {
	char clock_names[6][15]={"Readout_c0", "Readout_c1", "System_c0", "System_c1", "System_c2", "System_c3"};
    FILE_LOG(logDEBUG1, ("\tConfiguring Phase of C%d(%s) to %d (degree mode: %d)\n", ind, clock_names[ind], val, degrees));
	
	int maxShift = getMaxPhase(ind);
	int valShift = val;
	// convert to phase shift
	if (degrees) {
		ConvertToDifferentRange(0, 359, 0, maxShift - 1, val, &valShift);
	}
	FILE_LOG(logDEBUG1, ("\tphase shift: %d (degrees/shift: %d)\n", valShift, val));

	int relativePhase = valShift - clkPhase[ind];
	FILE_LOG(logDEBUG1, ("\trelative phase shift: %d (Current phase: %d)\n", relativePhase, clkPhase[ind]));

    // same phase
    if (!relativePhase) {
    	FILE_LOG(logINFO, ("\tNothing to do in Phase Shift\n"));
    	return OK;
    }
    FILE_LOG(logINFOBLUE, ("\tConfiguring Phase of C%d(%s) to %d (degree mode: %d)\n", ind, clock_names[ind], val, degrees));

    int phase = 0;
    if (relativePhase > 0) {
        phase = (maxShift - relativePhase);
    } else {
    	phase = (-1) * relativePhase;
    }
    FILE_LOG(logDEBUG1, ("\t[Single Direction] Phase:%d (0x%x). Max Phase shifts:%d\n", phase, phase, maxShift));

	int pllIndex = ind >= SYSTEM_C0 ? SYSTEM_PLL : READOUT_PLL;
	int clkIndex = ind >= SYSTEM_C0 ? ind - SYSTEM_C0 : ind;
    int ret = ALTERA_PLL_C10_SetPhaseShift(pllIndex, clkIndex, phase, 0);

    clkPhase[ind] = valShift;
	return ret;
}

int getPhase(enum CLKINDEX ind, int degrees) {
	if (!degrees)
		return clkPhase[ind];
	// convert back to degrees
	int val = 0;
	ConvertToDifferentRange(0, getMaxPhase(ind) - 1, 0, 359, clkPhase[ind], &val);
	return val;
}

int getMaxPhase(enum CLKINDEX ind) {
	int vcofreq = getVCOFrequency(ind);
	int maxshiftstep = ALTERA_PLL_C10_GetMaxPhaseShiftStepsofVCO();
	int ret = ((double)vcofreq / (double)clkDivider[ind]) * maxshiftstep;

	char clock_names[6][15]={"Readout_c0", "Readout_c1", "System_c0", "System_c1", "System_c2", "System_c3"};
	FILE_LOG(logDEBUG1, ("\tMax Phase Shift (%s): %d (Clock: %d Hz, VCO:%d Hz)\n",
			clock_names[ind], ret, clkDivider[ind], vcofreq));

	return ret;
}

int validatePhaseinDegrees(enum CLKINDEX ind, int val, int retval) {
	if (val == -1)
		return OK;
	FILE_LOG(logDEBUG1, ("validating phase in degrees for clk %d\n", (int)ind));
	int maxShift = getMaxPhase(ind);
	// convert degrees to shift
	// convert degrees to shift
	int valShift = 0;
	ConvertToDifferentRange(0, 359, 0, maxShift - 1, val, &valShift);
	// convert back to degrees
	ConvertToDifferentRange(0, maxShift - 1, 0, 359, valShift, &val);

	if (val == retval)
		return OK;
	return FAIL;
}



int getFrequency(enum CLKINDEX ind) {
    return clkDivider[ind];
}

int getVCOFrequency(enum CLKINDEX ind) {
	int pllIndex = ind >= SYSTEM_C0 ? SYSTEM_PLL : READOUT_PLL;
	return ALTERA_PLL_C10_GetVCOFrequency(pllIndex);
}

int getMaxClockDivider() {
	return ALTERA_PLL_C10_GetMaxClockDivider();
}

int setClockDivider(enum CLKINDEX ind, int val) {
	char clock_names[6][15]={"Readout_c0", "Readout_c1", "System_c0", "System_c1", "System_c2", "System_c3"};

	int vcofreq = getVCOFrequency(ind);
	int currentdiv = vcofreq / clkDivider[ind];
	int newfreq = vcofreq / val;

    FILE_LOG(logINFO, ("\tConfiguring Click Divider of C%d(%s) from %d (%d Hz) to %d (%d Hz). \n\t(Vcofreq: %d Hz)\n", ind, clock_names[ind], currentdiv, clkDivider[ind], val, newfreq, vcofreq));

    // Remembering old phases in degrees
    int oldPhases[NUM_CLOCKS];
	{ 
		int i = 0;
		for (i = 0; i < NUM_CLOCKS; ++i) {
			oldPhases	[i] = getPhase(i, 1);
			FILE_LOG(logDEBUG1, ("\tRemembering C%d (%s) phase: %d degrees\n", ind, clock_names, oldPhases[i]));
		}
	}

    // Calculate and set output frequency
	int pllIndex = ind >= SYSTEM_C0 ? SYSTEM_PLL : READOUT_PLL;
	int clkIndex = ind >= SYSTEM_C0 ? ind - SYSTEM_C0 : ind;
    int ret = ALTERA_PLL_C10_SetOuputFrequency (pllIndex, clkIndex, newfreq);
	clkDivider[ind] = newfreq;
    FILE_LOG(logINFO, ("\tC%d(%s): Clock Divider set to %d (%d Hz)\n", ind, clock_names[ind], val, clkDivider[ind]));
   
    // phase is reset by pll (when setting output frequency)
	if (ind >= READOUT_C0) {
    	clkPhase[READOUT_C0] = 0;
    	clkPhase[READOUT_C1] = 0;		
	} else {
    	clkPhase[SYSTEM_C0] = 0;
    	clkPhase[SYSTEM_C1] = 0;
    	clkPhase[SYSTEM_C2] = 0;
    	clkPhase[SYSTEM_C3] = 0;
	}

    // set the phase in degreesif custom set
	{ 
		int i = 0;
		for (i = 0; i < NUM_CLOCKS; ++i) {
			FILE_LOG(logINFO, ("\tPhase reset by PLL\n\tCorrecting C%d(%s) to %d degrees\n", i, clock_names[i], oldPhases[i]));
			setPhase(i, oldPhases[i], 1);
		}
	}
	return ret;
}

int getClockDivider(enum CLKINDEX ind) {
	return (getVCOFrequency(ind) / clkDivider[ind]);
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
	return getTotalNumberOfChannels() * DYNAMIC_RANGE;
}

int getTotalNumberOfChannels(){return  ((int)getNumberOfChannelsPerChip() * (int)getNumberOfChips());}
int getNumberOfChips(){return  NCHIP;}
int getNumberOfDACs(){return  NDAC;}
int getNumberOfChannelsPerChip(){return  NCHAN;}