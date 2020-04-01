#include "slsDetectorFunctionList.h"
#include "versionAPI.h"
#include "clogger.h"
#include "common.h"

#ifndef VIRTUAL
#include "FebControl.h"
#include "Beb.h"
#endif

#include <unistd.h> //to gethostname
#include <string.h>
#ifdef VIRTUAL
#include <netinet/in.h>
#include "communication_funcs_UDP.h"
#include <pthread.h>
#include <time.h>
#endif

// Global variable from slsDetectorServer_funcs
extern int debugflag;
extern udpStruct udpDetails;
extern const enum detectorType myDetectorType;

// Global variable from communication_funcs.c
extern int isControlServer;
extern void getMacAddressinString(char* cmac, int size, uint64_t mac);
extern void getIpAddressinString(char* cip, uint32_t ip);

int initError = OK;
int initCheckDone = 0;
char initErrorMessage[MAX_STR_LENGTH];


const char* dac_names[16] = {"SvP","Vtr","Vrf","Vrs","SvN","Vtgstv","Vcmp_ll","Vcmp_lr","cal","Vcmp_rl","rxb_rb","rxb_lb","Vcmp_rr","Vcp","Vcn","Vis"};
int default_tau_from_file= -1;
enum detectorSettings thisSettings;
sls_detector_module *detectorModules=NULL;
int *detectorChans=NULL;
int *detectorDacs=NULL;

int send_to_ten_gig = 0;
int  ndsts_in_use=32;
unsigned int nimages_per_request=1;
int  on_dst=0;
int dst_requested[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

enum masterFlags  masterMode=IS_SLAVE;
int top = 0;
int master = 0;
int normal = 0;
#ifndef VIRTUAL
uint32_t detid = 0;
#endif

int eiger_highvoltage = 0;
int eiger_theo_highvoltage = 0;
int eiger_iodelay = 0;
int eiger_photonenergy = 0;
int eiger_dynamicrange = 0;
int eiger_parallelmode = 0;
int eiger_storeinmem = 0;
int eiger_overflow32 = 0;
int eiger_readoutspeed = 0;
int eiger_triggermode = 0;
int eiger_extgating = 0;
int eiger_extgatingpolarity = 0;
int eiger_nexposures = 1;
int eiger_ntriggers = 1;
int eiger_tau_ns = 0;


#ifdef VIRTUAL
//values for virtual server
int64_t eiger_virtual_exptime = 0;
int64_t eiger_virtual_subexptime = 0;
int64_t eiger_virtual_subperiod = 0;
int64_t eiger_virtual_period = 0;
int eiger_virtual_counter_bit=1;
int eiger_virtual_ratecorrection_variable=0;
int64_t eiger_virtual_ratetable_tau_in_ns=-1;
int64_t eiger_virtual_ratetable_period_in_ns=-1;
int eiger_virtual_transmission_delay_left=0;
int eiger_virtual_transmission_delay_right=0;
int eiger_virtual_transmission_delay_frame=0;
int eiger_virtual_transmission_flowcontrol_10g=0;
int eiger_virtual_status=0;
int eiger_virtual_activate=1;
pthread_t eiger_virtual_tid;
int eiger_virtual_stop = 0;
uint64_t eiger_virtual_startingframenumber = 0;
int eiger_virtual_detPos[2] = {0, 0};
int eiger_virtual_test_mode = 0;
int eiger_virtual_quad_mode = 0;
#endif




int isInitCheckDone() {
	return initCheckDone;
}

int getInitResult(char** mess) {
	*mess = initErrorMessage;
	return initError;
}

void basictests() {
	initError = OK;
	initCheckDone = 0;
	memset(initErrorMessage, 0, MAX_STR_LENGTH);
#ifdef VIRTUAL
	LOG(logINFOBLUE, ("************ EIGER Virtual Server *****************\n\n"));
#endif
	uint32_t ipadd	= getDetectorIP();
	uint64_t macadd	= getDetectorMAC();
	int64_t fwversion = getFirmwareVersion();
	int64_t swversion = getServerVersion();
	int64_t sw_fw_apiversion = getFirmwareAPIVersion();
	int64_t client_sw_apiversion = getClientServerAPIVersion();

	LOG(logINFOBLUE, ("**************** EIGER Server *********************\n\n"
			"Detector IP Addr:\t\t 0x%x\n"
			"Detector MAC Addr:\t\t 0x%llx\n"

			"Firmware Version:\t\t %lld\n"
			"Software Version:\t\t 0x%llx\n"
			"F/w-S/w API Version:\t\t %lld\n"
			"Required Firmware Version:\t %d\n"
			"Client-Software API Version:\t 0x%llx\n"
			"\n"
			"********************************************************\n",
			(unsigned int)ipadd,
			(long long unsigned int)macadd,
			(long long int)fwversion,
			(long long int)swversion,
			(long long int)sw_fw_apiversion,
			REQUIRED_FIRMWARE_VERSION,
			(long long int)client_sw_apiversion));
	
	// update default udpdstip and udpdstmac (1g is hardware ip and hardware mac)
	udpDetails.srcip = ipadd;
	udpDetails.srcmac = macadd;

#ifdef VIRTUAL
	return;	
#endif
	// return if debugflag is not zero, debug mode
	if (debugflag) {
		return;
	}

	//cant read versions
	if (!fwversion || !sw_fw_apiversion) {
		strcpy(initErrorMessage, "Cant read versions from FPGA. Please update firmware.\n");
		LOG(logERROR, (initErrorMessage));
		initError = FAIL;
		return;
	}

	//check for API compatibility - old server
	if (sw_fw_apiversion > REQUIRED_FIRMWARE_VERSION) {
		sprintf(initErrorMessage, "This detector software software version (%lld) is incompatible.\n"
				"Please update detector software (min. %lld) to be compatible with this firmware.\n",
				(long long int)sw_fw_apiversion,
				(long long int)REQUIRED_FIRMWARE_VERSION);
		LOG(logERROR, (initErrorMessage));
		initError = FAIL;
		return;
	}

	//check for firmware compatibility - old firmware
	if ( REQUIRED_FIRMWARE_VERSION > fwversion) {
		sprintf(initErrorMessage, "This firmware version (%lld) is incompatible.\n"
				"Please update firmware (min. %lld) to be compatible with this server.\n",
				(long long int)fwversion,
				(long long int)REQUIRED_FIRMWARE_VERSION);
		LOG(logERROR, (initErrorMessage));
		initError = FAIL;
		return;
	}
	LOG(logINFO, ("Compatibility - success\n"));
}

#ifdef VIRTUAL
void setTestImageMode(int ival) {
    if (ival >= 0) {
        if (ival == 0) {
            LOG(logINFO, ("Switching off Image Test Mode\n"));
            eiger_virtual_test_mode = 0;
        } else {
            LOG(logINFO, ("Switching on Image Test Mode\n"));
            eiger_virtual_test_mode = 1;
        }
    }
}

int getTestImageMode() {
    return eiger_virtual_test_mode;
}
#endif


/* Ids */

uint64_t getServerVersion() {
    return APIEIGER;
}

uint64_t getClientServerAPIVersion() {
    return APIEIGER;
}

u_int64_t getFirmwareVersion() {
#ifdef VIRTUAL
	return 0;
#else
	return Beb_GetFirmwareRevision();
#endif
}

u_int64_t   getFirmwareAPIVersion() {
#ifdef VIRTUAL
	return 0;
#else
	return (u_int64_t)Beb_GetFirmwareSoftwareAPIVersion();
#endif
}



u_int32_t getDetectorNumber() {
#ifdef VIRTUAL
	return 0;
#else
	return detid;
#endif
}


u_int64_t  getDetectorMAC() {
	char mac[255]="";
	u_int64_t res=0;

	//execute and get address
	char output[255];
#ifdef VIRTUAL
	FILE* sysFile = popen("cat /sys/class/net/$(ip route show default | awk '/default/ {print $5}')/address", "r");
#else
	FILE* sysFile = popen("more /sys/class/net/eth0/address", "r");
#endif
	//FILE* sysFile = popen("ifconfig eth0 | grep HWaddr | cut -d \" \" -f 11", "r");
	fgets(output, sizeof(output), sysFile);
	pclose(sysFile);

	//getting rid of ":"
	char * pch;
	pch = strtok (output,":");
	while (pch != NULL) {
		strcat(mac,pch);
		pch = strtok (NULL, ":");
	}
#ifdef VIRTUAL
	sscanf(mac,"%lx",&res);
#else
	sscanf(mac, "%llx", &res);
#endif
	//increment by 1 for 10g
	if (send_to_ten_gig)
		res++;
	//LOG(logINFO, ("mac:%llx\n",res));

	return res;
}


u_int32_t  getDetectorIP() {
	char temp[50]="";
	u_int32_t res=0;
	//execute and get address
	char output[255];
#ifdef VIRTUAL
	FILE* sysFile = popen("ifconfig $(ip route show default | awk '/default/ {print $5}') | grep 'inet ' | cut -d ' ' -f10", "r");
#else
	FILE* sysFile = popen("ifconfig  | grep 'inet addr:'| grep -v '127.0.0.1' | cut -d: -f2", "r");
#endif
	fgets(output, sizeof(output), sysFile);
	pclose(sysFile);
	if (strlen(output) <= 1) {
		return 0;
	}

	//converting IPaddress to hex.
	char* pcword = strtok (output,".");
	while (pcword != NULL) {
		sprintf(output,"%02x",atoi(pcword));
		strcat(temp,output);
		pcword = strtok (NULL, ".");
	}
	strcpy(output,temp);
	sscanf(output, "%x", 	&res);
	//LOG(logINFO, ("ip:%x\n",res));

	return res;
}





/* initialization */

void initControlServer() {
#ifdef VIRTUAL
	if (initError == OK) {
		getModuleConfiguration();
		setupDetector();
	}
	initCheckDone = 1;
	return;
#else	
	if (initError == OK) {
		//Feb and Beb Initializations
		getModuleConfiguration();
		Feb_Interface_FebInterface();
		Feb_Control_FebControl();
		Feb_Control_Init(master,top,normal, getDetectorNumber());
		//master of 9M, check high voltage serial communication to blackfin
		if (master && !normal) {
			if (Feb_Control_OpenSerialCommunication())
				;//	Feb_Control_CloseSerialCommunication();
		}
		LOG(logDEBUG1, ("Control server: FEB Initialization done\n"));
		Beb_Beb(detid);
		Beb_SetDetectorNumber(getDetectorNumber());
		LOG(logDEBUG1, ("Control server: BEB Initialization done\n"));

		setupDetector();
		// activate (if it gets ip) (later FW will deactivate at startup)
		if (getDetectorIP() != 0) {
			Beb_Activate(1);
			Feb_Control_activate(1);
		} else {
			Beb_Activate(0);
			Feb_Control_activate(0);
		}
	}
	initCheckDone = 1;
#endif
}

void initStopServer() {
#ifdef VIRTUAL
	getModuleConfiguration();
	return;
#else
	getModuleConfiguration();
	Feb_Interface_FebInterface();
	Feb_Control_FebControl();
	Feb_Control_Init(master,top,normal,getDetectorNumber());
	LOG(logDEBUG1, ("Stop server: FEB Initialization done\n"));
	// activate (if it gets ip) (later FW will deactivate at startup)
	// also needed for stop server for status
	if (getDetectorIP() != 0) {
		Beb_Activate(1);
		Feb_Control_activate(1);
	} else {
		Beb_Activate(0);
		Feb_Control_activate(0);
	}
#endif
}


void getModuleConfiguration() {
#ifdef VIRTUAL
#ifdef VIRTUAL_MASTER
	master = 1;
	top = 1;
#else
	master = 0;
#ifdef VIRTUAL_TOP
	top = 1;
#else
	top = 0;
#endif
#endif
#ifdef VIRTUAL_9M
	normal = 0;
#else
	normal = 1;
#endif
	LOG(logINFOBLUE, ("Module: %s %s %s\n",
			(top ? "TOP" : "BOTTOM"),
			(master ? "MASTER" : "SLAVE"),
			(normal ? "NORMAL" : "SPECIAL")));
	return;
#else
	int *m=&master;
	int *t=&top;
	int *n=&normal;
	Beb_GetModuleConfiguration(m,t,n);
	if (isControlServer) {
		LOG(logINFOBLUE, ("Module: %s %s %s\n",
			(top ? "TOP" : "BOTTOM"),
			(master ? "MASTER" : "SLAVE"),
			(normal ? "NORMAL" : "SPECIAL")));
	}

	// read detector id
	char output[255];
	FILE* sysFile = popen(IDFILECOMMAND, "r");
	fgets(output, sizeof(output), sysFile);
	pclose(sysFile);
	sscanf(output,"%u",&detid);
	if (isControlServer) {
		LOG(logINFOBLUE, ("Detector ID: %u\n\n", detid));
	}
#endif
}



/* set up detector */

void allocateDetectorStructureMemory() {
	LOG(logINFO, ("This Server is for 1 Eiger half module (250k)\n\n"));

	//Allocation of memory
	detectorModules = malloc(sizeof(sls_detector_module));
	detectorChans = malloc(NCHIP*NCHAN*sizeof(int));
	detectorDacs = malloc(NDAC*sizeof(int));
	LOG(logDEBUG1, ("modules from 0x%x to 0x%x\n",detectorModules, detectorModules));
	LOG(logDEBUG1, ("chans from 0x%x to 0x%x\n",detectorChans, detectorChans));
	LOG(logDEBUG1, ("dacs from 0x%x to 0x%x\n",detectorDacs, detectorDacs));
	(detectorModules)->dacs = detectorDacs;
	(detectorModules)->chanregs = detectorChans;
	(detectorModules)->ndac = NDAC;
	(detectorModules)->nchip = NCHIP;
	(detectorModules)->nchan = NCHIP * NCHAN;
	(detectorModules)->reg = 0;
	(detectorModules)->iodelay = 0;
	(detectorModules)->tau = 0;
	(detectorModules)->eV = 0;
	thisSettings = UNINITIALIZED;

	// if trimval requested, should return -1 to acknowledge unknown
	int ichan=0;
	for (ichan=0; ichan<(detectorModules->nchan); ichan++) {
		*((detectorModules->chanregs)+ichan) = -1;
	}
}



void setupDetector() {

	allocateDetectorStructureMemory();
	//set dacs
	LOG(logINFOBLUE, ("Setting Default Dac values\n"));
	{
		int i = 0;
		const int defaultvals[NDAC] = DEFAULT_DAC_VALS;
		for(i = 0; i < NDAC; ++i) {
			setDAC((enum DACINDEX)i,defaultvals[i],0);
			if ((detectorModules)->dacs[i] != defaultvals[i]) {
				LOG(logERROR, ("Setting dac %d failed, wrote %d, read %d\n",i ,defaultvals[i], (detectorModules)->dacs[i]));
			}
		}
	}

	LOG(logINFOBLUE, ("Setting Default Parameters\n"));
	//setting default measurement parameters
	setNumFrames(DEFAULT_NUM_FRAMES);
	setExpTime(DEFAULT_EXPTIME);
	setSubExpTime(DEFAULT_SUBFRAME_EXPOSURE);
	getSubExpTime(DEFAULT_SUBFRAME_DEADTIME);
	setPeriod(DEFAULT_PERIOD);
	setNumTriggers(DEFAULT_NUM_CYCLES);
	eiger_dynamicrange = DEFAULT_DYNAMIC_RANGE;
	setDynamicRange(DEFAULT_DYNAMIC_RANGE);
	eiger_photonenergy = DEFAULT_PHOTON_ENERGY;
	setParallelMode(DEFAULT_PARALLEL_MODE);
	setOverFlowMode(DEFAULT_READOUT_STOREINRAM_MODE);
	setStoreInRamMode(DEFAULT_READOUT_OVERFLOW32_MODE);
	setClockDivider(RUN_CLK, DEFAULT_CLK_SPEED);//clk_devider,half speed
	setIODelay(DEFAULT_IO_DELAY);
	setTiming(DEFAULT_TIMING_MODE);
	setStartingFrameNumber(DEFAULT_STARTING_FRAME_NUMBER);
	setReadNLines(MAX_ROWS_PER_READOUT);
	//SetPhotonEnergyCalibrationParameters(-5.8381e-5,1.838515,5.09948e-7,-4.32390e-11,1.32527e-15);
	eiger_tau_ns = DEFAULT_RATE_CORRECTION;
	setRateCorrection(DEFAULT_RATE_CORRECTION);
	int enable[2] = {DEFAULT_EXT_GATING_ENABLE, DEFAULT_EXT_GATING_POLARITY};
	setExternalGating(enable);//disable external gating
#ifndef VIRTUAL
	Feb_Control_SetInTestModeVariable(DEFAULT_TEST_MODE);
#endif
	setHighVoltage(DEFAULT_HIGH_VOLTAGE);
#ifndef VIRTUAL
	Feb_Control_CheckSetup();
#endif
	LOG(logDEBUG1, ("Setup detector done\n\n"));
}




/* advanced read/write reg */
int writeRegister(uint32_t offset, uint32_t data) {
#ifdef VIRTUAL
	return OK;
#else
	if(!Feb_Control_WriteRegister(offset, data)) {
		return FAIL;
	}
	return OK;
#endif
}

int readRegister(uint32_t offset, uint32_t* retval) {
#ifdef VIRTUAL
	return OK;
#else
	if(!Feb_Control_ReadRegister(offset, retval)) {
		return FAIL;
	}
	return OK;
#endif
}


/* set parameters -  dr, roi */


int setDynamicRange(int dr) {
	// setting dr
	if (dr > 0) {
		LOG(logDEBUG1, ("Setting dynamic range: %d\n", dr));
#ifndef VIRTUAL
		if (Feb_Control_SetDynamicRange(dr)) {
			on_dst = 0;
			int i;
			for(i=0;i<32;i++) dst_requested[i] = 0; //clear dst requested
			if (!Beb_SetUpTransferParameters(dr)) {
				LOG(logERROR, ("Could not set bit mode in the back end\n"));
				return eiger_dynamicrange;
			}
		}
#endif
		eiger_dynamicrange = dr;
	}
	// getting dr
#ifndef VIRTUAL
	eiger_dynamicrange = Feb_Control_GetDynamicRange();
#endif
	return eiger_dynamicrange;
}




/* parameters - readout */

int	setParallelMode(int mode) {
	mode = (mode == 0 ? E_NON_PARALLEL : E_PARALLEL);
#ifndef VIRTUAL
	if (!Feb_Control_SetReadoutMode(mode)) {
		return FAIL;
	}
#endif
	eiger_parallelmode = mode;
	return OK;
}

int getParallelMode() {
	return (eiger_parallelmode == E_PARALLEL ? 1 : 0);
}

int	setOverFlowMode(int mode) {
	mode = (mode == 0 ? 0 : 1);
#ifndef VIRTUAL
	if (Beb_Set32bitOverflow(mode == 0 ? 0 : 1) == -1) {
		return FAIL;
	}
#endif
	eiger_overflow32 = mode;
	return OK;
}

int getOverFlowMode() {
	return eiger_overflow32;
}

void setStoreInRamMode(int mode) {
	mode = (mode == 0 ? 0 : 1);
	LOG(logINFO, ("Setting Store in Ram mode to %d\n", mode));
	eiger_storeinmem = mode;
}

int getStoreInRamMode() {
	return eiger_storeinmem;
}


/* parameters - timer */

int setStartingFrameNumber(uint64_t value) {
#ifdef VIRTUAL
	eiger_virtual_startingframenumber =  value;
	return OK;
#else
	return Beb_SetStartingFrameNumber(value);
#endif
}

int getStartingFrameNumber(uint64_t* retval) {
#ifdef VIRTUAL
	*retval = eiger_virtual_startingframenumber;
	return OK;
#else
	return Beb_GetStartingFrameNumber(retval, send_to_ten_gig);
#endif
}




void setNumFrames(int64_t val) {
    if (val > 0) {
		LOG(logINFO, ("Setting number of frames %lld\n", (long long int)val));
#ifndef VIRTUAL
		if (Feb_Control_SetNExposures((unsigned int)val * eiger_ntriggers)) {
			eiger_nexposures = val;
			on_dst = 0;
			int i;
			for(i=0;i<32;i++) dst_requested[i] = 0; //clear dst requested
			ndsts_in_use = 1;
			nimages_per_request = eiger_nexposures * eiger_ntriggers;
		}
#else
		eiger_nexposures = val;
		nimages_per_request = eiger_nexposures * eiger_ntriggers;
#endif		
    }
}

int64_t getNumFrames() {
    return eiger_nexposures;
}

void setNumTriggers(int64_t val) {
    if (val > 0) {
		LOG(logINFO, ("Setting number of triggers %lld\n", (long long int)val));
#ifndef VIRTUAL
		if (Feb_Control_SetNExposures((unsigned int)val * eiger_nexposures)) {
			eiger_ntriggers = val;
			on_dst = 0;
			int i;
			for(i=0;i<32;i++) dst_requested[i] = 0; //clear dst requested
			nimages_per_request = eiger_nexposures * eiger_ntriggers;
		}
#else
		eiger_ntriggers = val;
		nimages_per_request = eiger_nexposures * eiger_ntriggers;
#endif
    } 
}

int64_t getNumTriggers() {
    return eiger_ntriggers;
}

int setExpTime(int64_t val) {
	LOG(logINFO, ("Setting exptime %lld ns\n", (long long int)val));
#ifndef VIRTUAL
	Feb_Control_SetExposureTime(val/(1E9));
#else
	eiger_virtual_exptime = val;
#endif
    return OK;
}

int64_t getExpTime() {
#ifndef VIRTUAL
	return (Feb_Control_GetExposureTime()*(1E9));
#else
	return eiger_virtual_exptime;
#endif
}

int setPeriod(int64_t val) {
	LOG(logINFO, ("Setting period %lld ns\n", (long long int)val));
#ifndef VIRTUAL
	Feb_Control_SetExposurePeriod(val/(1E9));
#else
	eiger_virtual_period = val;
#endif
    return OK;
}

int64_t getPeriod() {
#ifndef VIRTUAL
	return (Feb_Control_GetExposurePeriod()*(1E9));
#else
	return eiger_virtual_period;
#endif
}

int setSubExpTime(int64_t val) {
	LOG(logINFO, ("Setting subexptime %lld ns\n", (long long int)val));
#ifndef VIRTUAL
	// calculate subdeadtime before settings subexptime
	int64_t subdeadtime = Feb_Control_GetSubFramePeriod() - Feb_Control_GetSubFrameExposureTime();
	Feb_Control_SetSubFrameExposureTime(val / 10);
	// set subperiod
	Feb_Control_SetSubFramePeriod((val+subdeadtime) / 10);
#else
	int64_t subdeadtime = eiger_virtual_subperiod * 10 -
	eiger_virtual_subexptime * 10;
	eiger_virtual_subexptime = (val / (10));
	eiger_virtual_subperiod = (val + subdeadtime) /10;
#endif
	return OK;
}

int64_t getSubExpTime() {
#ifndef VIRTUAL
	return (Feb_Control_GetSubFrameExposureTime());
#else
	return eiger_virtual_subexptime*10;
#endif
}

int setDeadTime(int64_t val) {
	LOG(logINFO, ("Setting subdeadtime %lld ns\n", (long long int)val));
#ifndef VIRTUAL
	// get subexptime
	int64_t subexptime = Feb_Control_GetSubFrameExposureTime();
#else
	int64_t subexptime = eiger_virtual_subexptime * 10;
#endif
	LOG(logINFO, ("Setting sub period (subdeadtime(%lld)): %lldns\n",
					(long long int)subexptime,
					(long long int)val),
					(long long int)(val + subexptime));
	//calculate subperiod
	val += subexptime;
#ifndef VIRTUAL
	Feb_Control_SetSubFramePeriod(val/10);
#else
	eiger_virtual_subperiod = (val/10);
#endif
	return OK;
}

int64_t getDeadTime() {
#ifndef VIRTUAL
	// get subexptime
	int64_t subexptime = Feb_Control_GetSubFrameExposureTime();
#else
	int64_t subexptime = eiger_virtual_subexptime * 10;
#endif
#ifndef VIRTUAL
	return (Feb_Control_GetSubFramePeriod() - subexptime);
#else
	return (eiger_virtual_subperiod*10 - subexptime);
#endif
}

int64_t getMeasuredPeriod() {
#ifdef VIRTUAL
	return 0;
#else
	return Feb_Control_GetMeasuredPeriod();
#endif
}

int64_t getMeasuredSubPeriod() {
#ifdef VIRTUAL
	return 0;
#else
	return Feb_Control_GetSubMeasuredPeriod();
#endif	
}


/* parameters - channel, module, settings */


int setModule(sls_detector_module myMod, char* mess) {


	LOG(logINFO, ("Setting module with settings %d\n",myMod.reg));

	// settings
	setSettings( (enum detectorSettings)myMod.reg);

	//copy module locally (module number, serial number
	//dacs (pointless), trimbit values(if needed)
	if (detectorModules) {
		if (copyModule(detectorModules,&myMod) == FAIL) {
			sprintf(mess, "Could not copy module\n");
			LOG(logERROR, (mess));
			setSettings(UNDEFINED);
			LOG(logERROR, ("Settings has been changed to undefined\n"));
			return FAIL;
		}
	}

	// iodelay
	if (setIODelay(myMod.iodelay)!= myMod.iodelay) {
		sprintf(mess, "Could not set module. Could not set iodelay %d\n", myMod.iodelay);
		LOG(logERROR, (mess));
		setSettings(UNDEFINED);
		LOG(logERROR, ("Settings has been changed to undefined\n"));
		return FAIL;
	}

	// threshold
	if (myMod.eV >= 0)
		setThresholdEnergy(myMod.eV);
	else {
		// (loading a random trim file) (dont return fail)
		setSettings(UNDEFINED);
		LOG(logERROR, ("Settings has been changed to undefined (random trim file)\n"));
	}

	// dacs
	{
		int i = 0;
		for(i = 0; i < NDAC; ++i) {
			setDAC((enum DACINDEX)i, myMod.dacs[i] , 0);
			if (myMod.dacs[i] != (detectorModules)->dacs[i]) {
				sprintf(mess, "Could not set module. Could not set dac %d\n", i);
				LOG(logERROR, (mess));
				setSettings(UNDEFINED);
				LOG(logERROR, ("Settings has been changed to undefined\n"));
				return FAIL;
			}
		}
	}

#ifndef VIRTUAL
	// trimbits
	if (myMod.nchan == 0) {
		LOG(logINFO, ("Setting module without trimbits\n"));
	} else {
		LOG(logINFO, ("Setting module with trimbits\n"));
		//includ gap pixels
		unsigned int tt[263680];
		int iy, ichip, ix, ip = 0, ich = 0;
		for (iy = 0; iy < 256; ++iy) {
			for (ichip = 0; ichip < 4; ++ichip) {
				for (ix = 0; ix < 256; ++ix) {
					tt[ip++] = myMod.chanregs[ich++];
				}
				if (ichip < 3) {
					tt[ip++] = 0;
					tt[ip++] = 0;
				}
			}
		}

		//set trimbits
		if (!Feb_Control_SetTrimbits(Feb_Control_GetModuleNumber(), tt)) {
			sprintf(mess, "Could not set module. Could not set trimbits\n");
			LOG(logERROR, (mess));
			setSettings(UNDEFINED);
			LOG(logERROR, ("Settings has been changed to undefined (random trim file)\n"));
			return FAIL;
		}
	}
#endif


	//rate correction
	//switch off rate correction: no value read from load settings)
	if (myMod.tau == -1) {
		if (getRateCorrectionEnable()) {
			setRateCorrection(0);
			sprintf(mess,"Cannot set module. Cannot set Rate correction. "
					"No default tau provided. Deactivating Rate Correction\n");
			LOG(logERROR, (mess));
			setSettings(UNDEFINED);
			LOG(logERROR, ("Settings has been changed to undefined (random trim file)\n"));
			return FAIL;
		}
	}
	//normal tau value (only if enabled)
	else {
		setDefaultSettingsTau_in_nsec(myMod.tau);
		if (getRateCorrectionEnable()) {
			if (setRateCorrection(myMod.tau) == FAIL) {
				sprintf(mess, "Cannot set module. Rate correction failed.\n");
				LOG(logERROR, (mess));
				setSettings(UNDEFINED);
				LOG(logERROR, ("Settings has been changed to undefined (random trim file)\n"));
				return FAIL;				
			} else {
				int64_t retvalTau = getCurrentTau();
				if (myMod.tau != retvalTau) {
					sprintf(mess, "Cannot set module. Could not set rate correction\n");
					LOG(logERROR, (mess));
					setSettings(UNDEFINED);
					LOG(logERROR, ("Settings has been changed to undefined (random trim file)\n"));
					return FAIL;
				}
			}
		}
	}
	return OK;
}


int getModule(sls_detector_module *myMod) {

#ifndef VIRTUAL
	//trimbits
	unsigned int* tt;
	tt = Feb_Control_GetTrimbits();

	//exclude gap pixels
	int iy, ichip, ix, ip = 0, ich = 0;
	for (iy = 0; iy < 256; ++iy) {
		for (ichip = 0; ichip < 4; ++ichip) {
			for (ix = 0; ix < 256; ++iy) {
				myMod->chanregs[ich++] = tt[ip++];
			}
			if (ichip < 3) {
				ip++;
				ip++;
			}
		}
	}
#endif

	//copy local module to myMod
	if (detectorModules) {
		if (copyModule(myMod, detectorModules) == FAIL)
			return FAIL;
	}
	else
		return FAIL;
	return OK;
}



enum detectorSettings setSettings(enum detectorSettings sett) {
	if (sett == UNINITIALIZED) {
		return thisSettings;
	}if (sett != GET_SETTINGS)
		thisSettings = sett;
	LOG(logINFO, ("Settings: %d\n", thisSettings));
	return thisSettings;
}

enum detectorSettings getSettings() {
	return thisSettings;
}






/* parameters - threshold */

int getThresholdEnergy() {
	LOG(logDEBUG1, ("Getting Threshold energy\n"));
	return eiger_photonenergy;
}


int setThresholdEnergy(int ev) {
	LOG(logINFO, ("Setting threshold energy:%d\n",ev));
	if (ev >= 0)
		eiger_photonenergy = ev;
	return  getThresholdEnergy();
}





/* parameters - dac, adc, hv */

// uses LTC2620 with 2.048V (implementation different to others not bit banging)
void setDAC(enum DACINDEX ind, int val, int mV) {
    if (val < 0)
        return;

    LOG(logDEBUG1, ("Setting dac[%d]: %d %s \n", (int)ind, val, (mV ? "mV" : "dac units")));

	if (ind == E_VTHRESHOLD) {
		setDAC(E_VCMP_LL, val, mV);
        setDAC(E_VCMP_LR, val, mV);
        setDAC(E_VCMP_RL, val, mV);
        setDAC(E_VCMP_RR, val, mV);
        setDAC(E_VCP, val, mV);
		return;
	}

    // validate index
    if (ind < 0 || ind >= NDAC) {
        LOG(logERROR, ("\tDac index %d is out of bounds (0 to %d)\n", ind, NDAC - 1));
        return;
    }

#ifdef VIRTUAL
    int dacval = 0;
    if (!mV) {
        (detectorModules)->dacs[ind] = val;
    }
    // convert to dac units
    else if (ConvertToDifferentRange(DAC_MIN_MV, DAC_MAX_MV, LTC2620_MIN_VAL, LTC2620_MAX_VAL,
            val, &dacval) == OK) {
        (detectorModules)->dacs[ind] = dacval;
    }
#else
    char iname[10];
    strcpy(iname,dac_names[(int)ind]);
    if (Feb_Control_SetDAC(iname, val, mV)) {
        int dacval = 0;
        Feb_Control_GetDAC(iname, &dacval, 0);
        (detectorModules)->dacs[ind] = dacval;
    }
#endif
}

int getDAC(enum DACINDEX ind, int mV) {
    if (ind == E_VTHRESHOLD) {
        int ret[5] = {0};
        ret[0] = getDAC(E_VCMP_LL, mV);
        ret[1] = getDAC(E_VCMP_LR, mV);
        ret[2] = getDAC(E_VCMP_RL, mV);
        ret[3] = getDAC(E_VCMP_RR, mV);
        ret[4] = getDAC(E_VCP, mV);

        if ((ret[0]== ret[1])&&
                (ret[1]==ret[2])&&
                (ret[2]==ret[3]) &&
                (ret[3]==ret[4])) {
            LOG(logINFO, ("\tvthreshold match\n"));
            return ret[0];
        } else {
            LOG(logERROR, ("\tvthreshold mismatch vcmp_ll:%d vcmp_lr:%d vcmp_rl:%d vcmp_rr:%d vcp:%d\n",
                    ret[0],ret[1],ret[2],ret[3], ret[4]));
            return -1;
        }
    }

    if (!mV) {
        LOG(logDEBUG1, ("Getting DAC %d : %d dac\n",ind, (detectorModules)->dacs[ind]));
        return (detectorModules)->dacs[ind];
    }
    int voltage = -1;
    // dac units to voltage
    ConvertToDifferentRange(LTC2620_MIN_VAL, LTC2620_MAX_VAL, DAC_MIN_MV, DAC_MAX_MV,
            (detectorModules)->dacs[ind], &voltage);
    LOG(logDEBUG1, ("Getting DAC %d : %d dac (%d mV)\n",ind, (detectorModules)->dacs[ind], voltage));
    return voltage;
}

int getMaxDacSteps() {
    return DAC_MAX_STEPS;
}


int getADC(enum ADCINDEX ind) {
#ifdef VIRTUAL
	return 0;
#else
	int retval = -1;
	char tempnames[6][20]={"FPGA EXT", "10GE","DCDC", "SODL", "SODR", "FPGA"};
	char cstore[255];

	switch(ind) {
	case TEMP_FPGA:
		retval=getBebFPGATemp();
		break;
	case TEMP_FPGAFEBL:
		retval=Feb_Control_GetLeftFPGATemp();
		break;
	case TEMP_FPGAFEBR:
		retval=Feb_Control_GetRightFPGATemp();
		break;
	case TEMP_FPGAEXT:
	case TEMP_10GE:
	case TEMP_DCDC:
	case TEMP_SODL:
	case TEMP_SODR:
		sprintf(cstore,"more  /sys/class/hwmon/hwmon%d/device/temp1_input",ind);
		FILE* sysFile = popen(cstore, "r");
		fgets(cstore, sizeof(cstore), sysFile);
		pclose(sysFile);
		sscanf(cstore,"%d",&retval);
		break;
	default:
		return -1;
	}

	LOG(logINFO, ("Temperature %s: %f°C\n", tempnames[ind], (double)retval/1000.00));

	return retval;
#endif
}


int setHighVoltage(int val) {
#ifdef VIRTUAL
	if (master) {
		// set
		if (val!=-1) {
			eiger_theo_highvoltage = val;
		}
		return eiger_theo_highvoltage;
	}

	return SLAVE_HIGH_VOLTAGE_READ_VAL;
#else

	if (master) {

		// set
		if (val!=-1) {
			eiger_theo_highvoltage = val;
			int ret = Feb_Control_SetHighVoltage(val);
			if (!ret)			//could not set
				return -2;
			else if (ret == -1) //outside range
				return -1;
		}

		// get
		if (!Feb_Control_GetHighVoltage(&eiger_highvoltage)) {
			LOG(logERROR, ("Could not read high voltage\n"));
			return -3;
		}

		// tolerance of 5
		if (abs(eiger_theo_highvoltage-eiger_highvoltage) > HIGH_VOLTAGE_TOLERANCE) {
			LOG(logINFO, ("High voltage still ramping: %d\n", eiger_highvoltage));
			return eiger_highvoltage;
		}
		return eiger_theo_highvoltage;
	}

	return SLAVE_HIGH_VOLTAGE_READ_VAL;
#endif
}







/* parameters - timing, extsig */

void setTiming( enum timingMode arg) {
	int ret = 0;
	switch(arg) {
	case AUTO_TIMING:			
		ret = 0;	
		break;
	case TRIGGER_EXPOSURE:		
		ret = 2;	
		break;
	case BURST_TRIGGER:			
		ret = 1;	
		break;
	case GATED:					
		ret = 3;	
		break;
	default:
		LOG(logERROR, ("Unknown timing mode %d\n", arg));
		return;
	}
	LOG(logDEBUG1, ("Setting Triggering Mode: %d\n", (int)ret));
#ifndef VIRTUAL
	if (Feb_Control_SetTriggerMode(ret,1))
#endif
		eiger_triggermode = ret;
}


enum timingMode getTiming() {
	switch(eiger_triggermode) {
	case 0:		
		return AUTO_TIMING;		
	case 2:		
		return TRIGGER_EXPOSURE; 
	case 1:		
		return BURST_TRIGGER;	
	case 3:		
		return GATED;			
	default:
		LOG(logERROR, ("Unknown trigger mode found %d\n", eiger_triggermode));
		return GET_TIMING_MODE;
	}
}



/* configure mac */

int configureMAC() {
    uint32_t srcip = udpDetails.srcip;
	uint32_t dstip = udpDetails.dstip;
	uint64_t srcmac = udpDetails.srcmac;
	uint64_t dstmac = udpDetails.dstmac;
	int srcport = udpDetails.srcport;
	int dstport = udpDetails.dstport;		
	int dstport2 = udpDetails.dstport2;			

	LOG(logINFOBLUE, ("Configuring MAC\n"));
	char src_mac[50], src_ip[INET_ADDRSTRLEN],dst_mac[50], dst_ip[INET_ADDRSTRLEN];
	getMacAddressinString(src_mac, 50, srcmac);
	getMacAddressinString(dst_mac, 50, dstmac);
	getIpAddressinString(src_ip, srcip);
	getIpAddressinString(dst_ip, dstip);

	LOG(logINFO, (
	        "\tSource IP   : %s\n"
	        "\tSource MAC  : %s\n"
	        "\tSource Port : %d\n"
	        "\tDest IP     : %s\n"
	        "\tDest MAC    : %s\n"
			"\tDest Port   : %d\n"
			"\tDest Port2  : %d\n",
	        src_ip, src_mac, srcport,
	        dst_ip, dst_mac, dstport, dstport2));

#ifdef VIRTUAL
	if (setUDPDestinationDetails(0, dst_ip, dstport) == FAIL) {
		LOG(logERROR, ("could not set udp destination IP and port\n"));
		return FAIL;
	}
	if (setUDPDestinationDetails(1, dst_ip, dstport2) == FAIL) {
		LOG(logERROR, ("could not set udp destination IP and port2\n"));
		return FAIL;
	}
    return OK;
#else

	int beb_num =  detid;
	int header_number = 0;
	int dst_port = dstport;
	if (!top)
		dst_port = dstport2;

	int i=0;
	/* for(i=0;i<32;i++) { modified for Aldo*/
	if (Beb_SetBebSrcHeaderInfos(beb_num,send_to_ten_gig,src_mac,src_ip,srcport) &&
			Beb_SetUpUDPHeader(beb_num,send_to_ten_gig,header_number+i,dst_mac,dst_ip, dst_port)) {
		LOG(logDEBUG1, ("\tset up left ok\n"));
	} else {
		return FAIL;
	}
	/*}*/

	header_number = 32;
	dst_port = dstport2;
	if (!top)
		dst_port = dstport;

	/*for(i=0;i<32;i++) {*//** modified for Aldo*/
	if (Beb_SetBebSrcHeaderInfos(beb_num,send_to_ten_gig,src_mac,src_ip,srcport) &&
			Beb_SetUpUDPHeader(beb_num,send_to_ten_gig,header_number+i,dst_mac,dst_ip, dst_port)) {
		LOG(logDEBUG1, (" set up right ok\n"));
	} else {
		return FAIL;
	}
	/*}*/

	on_dst = 0;

	for(i=0;i<32;i++) dst_requested[i] = 0; //clear dst requested
	nimages_per_request=eiger_nexposures * eiger_ntriggers;
#endif
	return OK;
}


int	setDetectorPosition(int pos[]) {
#ifdef VIRTUAL
	memcpy(eiger_virtual_detPos, pos, sizeof(eiger_virtual_detPos));
	return OK;
#else
	return Beb_SetDetectorPosition(pos);
#endif
}

int* getDetectorPosition() {
#ifdef VIRTUAL
	return eiger_virtual_detPos;
#else
	return Beb_GetDetectorPosition();
#endif	
}

int setQuad(int value) {
	if (value < 0) {
		return OK;
	}
#ifndef VIRTUAL
	if (Beb_SetQuad(value) == FAIL) {
		return FAIL;
	}
	if (!Feb_Control_SetQuad(value)) {
		return FAIL;
	}
#else
	eiger_virtual_quad_mode = value;
#endif
	return OK;
}

int	getQuad() {
#ifdef VIRTUAL
	return eiger_virtual_quad_mode;
#else
	return Beb_GetQuad();
#endif
}

int setInterruptSubframe(int value) {
	if(value < 0)
		return FAIL;
#ifndef VIRTUAL
	if(!Feb_Control_SetInterruptSubframe(value)) {
		return FAIL;
	}
#endif
	return OK;
}

int	getInterruptSubframe() {
#ifdef VIRTUAL
	return 0;
#else
	return Feb_Control_GetInterruptSubframe();
#endif
}

int setReadNLines(int value) {
	if(value < 0)
		return FAIL;
#ifndef VIRTUAL
	if(!Feb_Control_SetReadNLines(value)) {
		return FAIL;
	}
	Beb_SetReadNLines(value);
#endif
	return OK;
}

int	getReadNLines() {
#ifdef VIRTUAL
	return 0;
#else
	return Feb_Control_GetReadNLines();
#endif
}

int enableTenGigabitEthernet(int val) {
	if (val!=-1) {
		LOG(logINFO, ("Setting 10Gbe: %d\n", (val > 0) ? 1 : 0));
		if (val>0)
			send_to_ten_gig = 1;
		else
			send_to_ten_gig = 0;
		//configuremac called from client
	}
	return send_to_ten_gig;
}



/* eiger specific - iodelay, pulse, rate, temp, activate, delay nw parameter */
int setClockDivider(enum CLKINDEX ind, int val) {
    if (ind != RUN_CLK) {
		LOG(logERROR, ("Unknown clock index: %d\n", ind));
	    return FAIL;
	}
	if (val >= 0) {
		LOG(logINFO, ("Setting Read out Speed: %d\n", val));
#ifndef VIRTUAL
		if (Feb_Control_SetReadoutSpeed(val))
#endif
			eiger_readoutspeed = val;
	}
	return OK;
}

int getClockDivider(enum CLKINDEX ind) {
    if (ind != RUN_CLK) {
		LOG(logERROR, ("Unknown clock index: %d\n", ind));
	    return FAIL;
	}
	return  eiger_readoutspeed;
}

int setIODelay(int val) {
	if (val!=-1) {
		LOG(logDEBUG1, ("Setting IO Delay: %d\n",val));
#ifndef VIRTUAL
		if (Feb_Control_SetIDelays(Feb_Control_GetModuleNumber(),val))
#endif
			eiger_iodelay = val;
	}
	return eiger_iodelay;
}


int setCounterBit(int val) {
	if (val!=-1) {
		LOG(logINFO, ("Setting Counter Bit: %d\n",val));
#ifdef VIRTUAL
		eiger_virtual_counter_bit = val;
#else
		Feb_Control_Set_Counter_Bit(val);
#endif
	}
#ifdef VIRTUAL
	return eiger_virtual_counter_bit;
#else
	return Feb_Control_Get_Counter_Bit();
#endif
}


int pulsePixel(int n, int x, int y) {
#ifndef VIRTUAL
	if (!Feb_Control_Pulse_Pixel(n,x,y))
		return FAIL;
#endif
	return OK;
}

int pulsePixelNMove(int n, int x, int y) {
#ifndef VIRTUAL
	if (!Feb_Control_PulsePixelNMove(n,x,y))
		return FAIL;
#endif
	return OK;
}

int pulseChip(int n) {
#ifndef VIRTUAL
	if (!Feb_Control_PulseChip(n))
		return FAIL;
#endif
	return OK;
}

int updateRateCorrection(char* mess) {
	int ret = OK;
	// recalculates rate correction table, or switches off in wrong bit mode
	if (eiger_tau_ns != 0) {
		switch (eiger_dynamicrange) {
			case 16:
			case 32:
				ret = setRateCorrection(eiger_tau_ns);
				break;
			default:
				setRateCorrection(0);
				strcpy(mess, "Rate correction Deactivated, must be in 32 or 16 bit mode");
				ret = FAIL;
				break;
		}
	}
	getCurrentTau(); // update eiger_tau_ns
	return ret;
}

int validateAndSetRateCorrection(int64_t tau_ns, char* mess) {
	// switching on in wrong bit mode
	if ((tau_ns != 0) && 
		(eiger_dynamicrange != 32) && (eiger_dynamicrange != 16)) {
		strcpy(mess,"Rate correction Deactivated, must be in 32 or 16 bit mode\n");
		LOG(logERROR,(mess));
		return FAIL;
	}
	// default tau (-1, get proper value)
	if (tau_ns < 0) {
		tau_ns = getDefaultSettingsTau_in_nsec();
		if (tau_ns < 0) {
			strcpy(mess,"Default settings file not loaded. No default tau yet\n");
			LOG(logERROR,(mess));
			return FAIL;
		}
		eiger_tau_ns = -1;
	}
	// user defined value (settings become undefined)
	else if (tau_ns > 0) {
		setSettings(UNDEFINED);
		LOG(logERROR, ("Settings has been changed to undefined (tau changed)\n"));
		eiger_tau_ns = tau_ns;
	}	
	return setRateCorrection(tau_ns);
}

int setRateCorrection(int64_t custom_tau_in_nsec) {//in nanosec (will never be -1)
#ifdef VIRTUAL
	//deactivating rate correction
	if (custom_tau_in_nsec==0) {
		eiger_virtual_ratecorrection_variable = 0;
		return OK;
	}

	//when dynamic range changes, use old tau
	else if (custom_tau_in_nsec == -1)
		custom_tau_in_nsec = eiger_virtual_ratetable_tau_in_ns;

	//get period = subexptime if 32bit , else period = exptime if 16 bit
	int64_t actual_period = eiger_virtual_subexptime*10; //already in nsec
	if (eiger_dynamicrange == 16)
		actual_period = eiger_virtual_exptime;

	int64_t ratetable_period_in_nsec = eiger_virtual_ratetable_period_in_ns;
	int64_t tau_in_nsec = eiger_virtual_ratetable_tau_in_ns;



	//same setting
	if ((tau_in_nsec == custom_tau_in_nsec) && (ratetable_period_in_nsec == actual_period)) {
		if (eiger_dynamicrange == 32) {
			LOG(logINFO, ("Rate Table already created before: Same Tau %lldns, Same subexptime %lldns\n",
					(long long int)tau_in_nsec,(long long int)ratetable_period_in_nsec));
		} else {
			LOG(logINFO, ("Rate Table already created before: Same Tau %lldns, Same exptime %lldns\n",
					(long long int)tau_in_nsec,(long long int)ratetable_period_in_nsec));
		}
	}
	//different setting, calculate table
	else {
		eiger_virtual_ratetable_tau_in_ns = custom_tau_in_nsec;
		eiger_virtual_ratetable_period_in_ns = eiger_virtual_subexptime*10;
		if (eiger_dynamicrange == 16)
			eiger_virtual_ratetable_period_in_ns = eiger_virtual_exptime;
	}
	//activating rate correction
	eiger_virtual_ratecorrection_variable = 1;
	LOG(logINFO, ("Rate Correction Value set to %lld ns\n",(long long int)eiger_virtual_ratetable_tau_in_ns));

	return OK;
#else

	//deactivating rate correction
	if (custom_tau_in_nsec==0) {
		Feb_Control_SetRateCorrectionVariable(0);
		return OK;
	}

	//when dynamic range changes, use old tau
	else if (custom_tau_in_nsec == -1)
		custom_tau_in_nsec = Feb_Control_Get_RateTable_Tau_in_nsec();


	int dr = Feb_Control_GetDynamicRange();
	//get period = subexptime if 32bit , else period = exptime if 16 bit
	int64_t actual_period = Feb_Control_GetSubFrameExposureTime(); //already in nsec
	if (dr == 16)
		actual_period = Feb_Control_GetExposureTime_in_nsec();

	int64_t ratetable_period_in_nsec = Feb_Control_Get_RateTable_Period_in_nsec();
	int64_t tau_in_nsec = Feb_Control_Get_RateTable_Tau_in_nsec();


	//same setting
	if ((tau_in_nsec == custom_tau_in_nsec) && (ratetable_period_in_nsec == actual_period)) {
		if (dr == 32) {
			LOG(logINFO, ("Rate Table already created before: Same Tau %lldns, Same subexptime %lldns\n",
					tau_in_nsec,ratetable_period_in_nsec));
		} else {
			LOG(logINFO, ("Rate Table already created before: Same Tau %lldns, Same exptime %lldns\n",
					tau_in_nsec,ratetable_period_in_nsec));
		}
	}
	//different setting, calculate table
	else {
		int ret = Feb_Control_SetRateCorrectionTau(custom_tau_in_nsec);
		if (ret<=0) {
			LOG(logERROR, ("Rate correction failed. Deactivating rate correction\n"));
			Feb_Control_SetRateCorrectionVariable(0);
			return FAIL;
		}
	}
	//activating rate correction
	Feb_Control_SetRateCorrectionVariable(1);
	LOG(logINFO, ("Rate Correction Value set to %lld ns\n", (long long int)Feb_Control_Get_RateTable_Tau_in_nsec()));
	Feb_Control_PrintCorrectedValues();

	return OK;
#endif
}

int getRateCorrectionEnable() {
#ifdef VIRTUAL
	return eiger_virtual_ratecorrection_variable;
#else
	return Feb_Control_GetRateCorrectionVariable();
#endif
}

int getDefaultSettingsTau_in_nsec() {
	return default_tau_from_file;
}

void setDefaultSettingsTau_in_nsec(int t) {
	default_tau_from_file = t;
	LOG(logINFOBLUE, ("Default tau set to %d\n", default_tau_from_file));
}

int64_t getCurrentTau() {
	if (!getRateCorrectionEnable()) {
		eiger_tau_ns = 0;
		return 0;
	}
	else {
#ifndef VIRTUAL
		eiger_tau_ns = Feb_Control_Get_RateTable_Tau_in_nsec()
		return eiger_tau_ns;
#else
		eiger_tau_ns = eiger_virtual_ratetable_tau_in_ns;
		return eiger_tau_ns;
#endif
	}
}

void setExternalGating(int enable[]) {
	if (enable[0]>=0 && enable[1]>=0) {
#ifndef VIRTUAL
		Feb_Control_SetExternalEnableMode(enable[0], enable[1]);//enable = 0 or 1, polarity = 0 or 1 , where 1 is positive
#endif
		eiger_extgating = enable[0];
		eiger_extgatingpolarity = enable[1];
	}
	enable[0] = eiger_extgating;
	enable[1] = eiger_extgatingpolarity;
}

int setAllTrimbits(int val) {
#ifndef VIRTUAL
	if (!Feb_Control_SaveAllTrimbitsTo(val)) {
		LOG(logERROR, ("Could not set all trimbits\n"));
		return FAIL;
	}
#endif
	if (detectorModules) {
		int ichan;
		for (ichan=0; ichan<(detectorModules->nchan); ichan++) {
			*((detectorModules->chanregs)+ichan)=val;
		}
	}

	LOG(logINFO, ("All trimbits have been set to %d\n", val));
	return OK;
}

int getAllTrimbits() {
	int ichan=0;
	int value = *((detectorModules->chanregs));
	if (detectorModules) {
		for (ichan=0; ichan<(detectorModules->nchan); ichan++) {
			if (*((detectorModules->chanregs)+ichan) != value) {
				value= -1;
				break;
			}

		}
	}
	LOG(logINFO, ("Value of all Trimbits: %d\n", value));
	return value;
}

int getBebFPGATemp() {
#ifdef VIRTUAL
	return 0;
#else
	return Beb_GetBebFPGATemp();
#endif
}

int activate(int enable) {
#ifdef VIRTUAL
	if (enable >=0)
		eiger_virtual_activate = enable;
	return eiger_virtual_activate;
#else
	int ret = Beb_Activate(enable);
	Feb_Control_activate(ret);
	return ret;
#endif
}

int getTenGigaFlowControl() {
#ifdef VIRTUAL
	return eiger_virtual_transmission_flowcontrol_10g;
#else
	return Beb_GetTenGigaFlowControl();
#endif
}

int setTenGigaFlowControl(int value) {
#ifdef VIRTUAL
	eiger_virtual_transmission_flowcontrol_10g = (value == 0? 0 : 1);
#else
	if (!Beb_SetTenGigaFlowControl(value)) {
		return FAIL;
	}
#endif
	return OK;
}

int getTransmissionDelayFrame() {
#ifdef VIRTUAL
	return eiger_virtual_transmission_delay_frame;
#else
	return Beb_GetTransmissionDelayFrame();
#endif
}

int setTransmissionDelayFrame(int value) {
#ifdef VIRTUAL
	eiger_virtual_transmission_delay_frame = value;
#else
	if (!Beb_SetTransmissionDelayFrame(value)) {
		return FAIL;
	}
#endif
	return OK;
}

int getTransmissionDelayLeft() {
#ifdef VIRTUAL
	return eiger_virtual_transmission_delay_left;
#else
	return Beb_GetTransmissionDelayLeft();
#endif
}

int setTransmissionDelayLeft(int value) {
#ifdef VIRTUAL
	eiger_virtual_transmission_delay_left = value;	
#else
	if (!Beb_SetTransmissionDelayLeft(value)) {
		return FAIL;
	}
#endif
	return OK;
}

int getTransmissionDelayRight() {
#ifdef VIRTUAL
	return eiger_virtual_transmission_delay_right;
#else
	return Beb_GetTransmissionDelayRight();
#endif
}

int setTransmissionDelayRight(int value) {
#ifdef VIRTUAL
	eiger_virtual_transmission_delay_right = value;
#else
	if (!Beb_SetTransmissionDelayRight(value)) {
		return FAIL;
	}
#endif
	return OK;
}








/* aquisition */


int prepareAcquisition() {
#ifndef VIRTUAL
	LOG(logINFO, ("Going to prepare for acquisition with counter_bit:%d\n",Feb_Control_Get_Counter_Bit()));
	Feb_Control_PrepareForAcquisition();
#endif
	return OK;

}


int startStateMachine() {
#ifdef VIRTUAL
	// create udp socket
	if(createUDPSocket(0) != OK) {
		return FAIL;
	}
	if(createUDPSocket(1) != OK) {
		return FAIL;
	}
	LOG(logINFOBLUE, ("Starting State Machine\n"));
	eiger_virtual_status = 1;
	eiger_virtual_stop = 0;
	if (pthread_create(&eiger_virtual_tid, NULL, &start_timer, NULL)) {
		LOG(logERROR, ("Could not start Virtual acquisition thread\n"));
		eiger_virtual_status = 0;
		return FAIL;
	}
	LOG(logINFO ,("Virtual Acquisition started\n"));
	return OK;
#else
	LOG(logINFOBLUE, ("Starting State Machine\n"));
	int ret = OK,prev_flag;
	//get the DAQ toggle bit
	prev_flag = Feb_Control_AcquisitionStartedBit();

	LOG(logINFO, ("Going to start acquisition\n"));
	Feb_Control_StartAcquisition();

	if (!eiger_storeinmem) {
		LOG(logINFO, ("requesting images right after start\n"));
		ret =  startReadOut();
	}

	//wait for acquisition start
	if (ret == OK) {
		if (!Feb_Control_WaitForStartedFlag(5000, prev_flag)) {
			LOG(logERROR, ("Acquisition did not LOG(logERROR ouble reading register\n"));
			return FAIL;
		}
		LOG(logINFOGREEN, ("Acquisition started\n"));
	}

	return ret;
#endif
}

#ifdef VIRTUAL
void* start_timer(void* arg) {
	int64_t periodNs = eiger_virtual_period;
	int numFrames = nimages_per_request;
	int64_t expUs = eiger_virtual_exptime / 1000;

	int dr = eiger_dynamicrange;
	double bytesPerPixel  = (double)dr/8.00;
	int tgEnable = send_to_ten_gig;
	int datasize = (tgEnable ? 4096 : 1024);
	int packetsize = datasize + sizeof(sls_detector_header);
	int numPacketsPerFrame =  (tgEnable ? 4 : 16) * dr;
	int npixelsx = 256 * 2 * bytesPerPixel; 
	int databytes = 256 * 256 * 2 * bytesPerPixel;
	int row = eiger_virtual_detPos[0];
	int colLeft = top ? eiger_virtual_detPos[1] : eiger_virtual_detPos[1] + 1;
	int colRight = top ? eiger_virtual_detPos[1] + 1 : eiger_virtual_detPos[1];
	int ntotpixels = 256 * 256 * 4;

	LOG(logINFO, (" dr:%d\n bytesperpixel:%f\n tgenable:%d\n datasize:%d\n packetsize:%d\n numpackes:%d\n npixelsx:%d\n databytes:%d\n ntotpixels:%d\n",
	dr, bytesPerPixel, tgEnable, datasize, packetsize, numPacketsPerFrame, npixelsx, databytes, ntotpixels));

	// Generate data
	char imageData[databytes * 2];
	memset(imageData, 0, databytes * 2);
	{
		int i = 0;
		switch (dr) {
			case 4:
				for (i = 0; i < ntotpixels/2; ++i) {
					*((uint8_t*)(imageData + i)) = eiger_virtual_test_mode ? 0xEE : (uint8_t)(((2 * i & 0xF) << 4) | ((2 * i + 1) & 0xF));
				}
				break;				
			case 8:
				for (i = 0; i < ntotpixels; ++i) {
					*((uint8_t*)(imageData + i)) = eiger_virtual_test_mode ? 0xFE : (uint8_t)i;
				} 
				break;
			case 16:
				for (i = 0; i < ntotpixels; ++i) {
					*((uint16_t*)(imageData + i * sizeof(uint16_t))) = eiger_virtual_test_mode ? 0xFFE : (uint16_t)i;
				}
				break;
			case 32:
				for (i = 0; i < ntotpixels; ++i) {
					*((uint32_t*)(imageData + i * sizeof(uint32_t))) = eiger_virtual_test_mode ? 0xFFFFFE : (uint32_t)i;
				}	
				break;
			default:
				break;
		}
	}
	
	// Send data
	{
		int frameNr = 1;
        // loop over number of frames
		for(frameNr = 1; frameNr <= numFrames; ++frameNr ) {

			usleep(eiger_virtual_transmission_delay_frame);

			//check if virtual_stop is high
			if(eiger_virtual_stop == 1){
				break;
			}

            // sleep for exposure time
			struct timespec begin, end;
			clock_gettime(CLOCK_REALTIME, &begin);
			usleep(expUs);

			int srcOffset = 0;
			int srcOffset2 = npixelsx;
			
			// loop packet
			{
				int i = 0;
				for(i = 0; i != numPacketsPerFrame; ++i) {
					// set header
					char packetData[packetsize];
					memset(packetData, 0, packetsize);
					sls_detector_header* header = (sls_detector_header*)(packetData);
					header->detType = 3;//(uint16_t)myDetectorType; updated when firmware updates
					header->version = SLS_DETECTOR_HEADER_VERSION - 1;								
					header->frameNumber = frameNr;
					header->packetNumber = i;
					header->row = row;
					header->column = colLeft;

					char packetData2[packetsize];
					memset(packetData2, 0, packetsize);
					header = (sls_detector_header*)(packetData2);
					header->detType = 3;//(uint16_t)myDetectorType; updated when firmware updates
					header->version = SLS_DETECTOR_HEADER_VERSION - 1;								
					header->frameNumber = frameNr;
					header->packetNumber = i;
					header->row = row;
					header->column = colRight;
					if (eiger_virtual_quad_mode) {
						header->row = 1; // right is next row
						header->column = 0;	// right same first column						
					}

					// fill data	
					int dstOffset = sizeof(sls_detector_header);
					int dstOffset2 = sizeof(sls_detector_header);
					{		
						int psize = 0;	
						for (psize = 0; psize < datasize; psize += npixelsx) {

							if (dr == 32 && tgEnable == 0) {
								memcpy(packetData + dstOffset, imageData + srcOffset, npixelsx/2);
								memcpy(packetData2 + dstOffset2, imageData + srcOffset2, npixelsx/2);
								if (srcOffset % npixelsx == 0) {
									srcOffset += npixelsx/2;
									srcOffset2 += npixelsx/2;
								} 
								// skip the other half (2 packets in 1 line for 32 bit)
								else {
									srcOffset += npixelsx;
									srcOffset2 += npixelsx;										
								}
								dstOffset += npixelsx/2;
								dstOffset2 += npixelsx/2;
							} else {
								memcpy(packetData + dstOffset, imageData + srcOffset, npixelsx);
								memcpy(packetData2 + dstOffset2, imageData + srcOffset2, npixelsx);
								srcOffset += 2 * npixelsx;
								srcOffset2 += 2 * npixelsx;
								dstOffset += npixelsx;
								dstOffset2 += npixelsx;
							}
						}
					}
					usleep(eiger_virtual_transmission_delay_left);
					sendUDPPacket(0, packetData, packetsize);
					usleep(eiger_virtual_transmission_delay_right);
					sendUDPPacket(1, packetData2, packetsize);
				}
			}
			LOG(logINFO, ("Sent frame: %d\n", frameNr));
			clock_gettime(CLOCK_REALTIME, &end);
			int64_t timeNs = ((end.tv_sec - begin.tv_sec) * 1E9 +
					(end.tv_nsec - begin.tv_nsec));

			// sleep for (period - exptime)
			if (frameNr < numFrames) { // if there is a next frame
				if (periodNs > timeNs) {
					usleep((periodNs - timeNs)/ 1000);
				}
			}
		}
	}
	
	
	closeUDPSocket(0);
	closeUDPSocket(1);
	
	eiger_virtual_status = 0;
	LOG(logINFOBLUE, ("Finished Acquiring\n"));
	return NULL;
}
#endif




int stopStateMachine() {
	LOG(logINFORED, ("Going to stop acquisition\n"));
#ifdef VIRTUAL
	eiger_virtual_stop = 0;
	return OK;
#else
	if ((Feb_Control_StopAcquisition() != STATUS_IDLE) || (!Beb_StopAcquisition()) ) {
		LOG(logERROR, ("failed to stop acquisition\n"));
		return FAIL;
	}

	// ensure all have same starting frame numbers
	uint64_t retval = 0;
	if(Beb_GetStartingFrameNumber(&retval, send_to_ten_gig) == -2) {
		Beb_SetStartingFrameNumber(retval + 1);
	}
	return OK;
#endif
}

int	softwareTrigger() {
#ifdef VIRTUAL
	return OK;
#else
	if (!Feb_Control_SoftwareTrigger())
		return FAIL;
	return OK;
#endif
}


int startReadOut() {

	LOG(logINFO, ("Requesting images...\n"));
#ifdef VIRTUAL
	return OK;
#else
	//RequestImages();
	int ret_val = 0;
	dst_requested[0] = 1;
	while(dst_requested[on_dst]) {
		//waits on data
		int beb_num =  detid;
		if  ((ret_val = (!Beb_RequestNImages(beb_num,send_to_ten_gig,on_dst,nimages_per_request,0))))
			break;
		//		for(i=0;i<nimages_per_request;i++)
		//			if  ((ret_val = (!Beb_RequestNImages(beb_num,send_to_ten_gig,on_dst,1,0))))
		//				break;

		dst_requested[on_dst++]=0;
		on_dst%=ndsts_in_use;
	}

	if (ret_val)
		return FAIL;
	else
		return OK;
#endif
}


enum runStatus getRunStatus() {
#ifdef VIRTUAL
	if (eiger_virtual_status == 0) {
		LOG(logINFO, ("Status: IDLE\n"));
		return IDLE;
	} else {
		LOG(logINFO, ("Status: RUNNING...\n"));
		return RUNNING;
	}
#else

	int i = Feb_Control_AcquisitionInProgress();
	switch (i) {
	case STATUS_ERROR:
		LOG(logERROR, ("Status: ERROR reading status register\n"));
		return ERROR;
	case STATUS_IDLE:
		LOG(logINFOBLUE, ("Status: IDLE\n"));
		return IDLE;
	default:
		LOG(logINFOBLUE, ("Status: RUNNING...\n"));
		return RUNNING;
	}

	return IDLE;
#endif
}



void readFrame(int *ret, char *mess) {
#ifdef VIRTUAL
	// wait for status to be done
	while(eiger_virtual_status == 1){
		usleep(500);
	}
	LOG(logINFOGREEN, ("acquisition successfully finished\n"));
	return;
#else

	if (Feb_Control_WaitForFinishedFlag(5000) == STATUS_ERROR) {
		LOG(logERROR, ("Waiting for finished flag\n"));
		*ret = FAIL;
		return;
	}
	LOG(logINFOGREEN, ("Acquisition finished\n"));

	if (eiger_storeinmem) {
		LOG(logINFO, ("requesting images after storing in memory\n"));
		if (startReadOut() == FAIL) {
			strcpy(mess,"Could not execute read image requests\n");
			LOG(logERROR, (mess));
			*ret = (int)FAIL;
			return;
		}
	}

	//wait for detector to send
	Beb_EndofDataSend(send_to_ten_gig);
	LOG(logINFOGREEN, ("Acquisition successfully finished\n"));
#endif
}








/* common */


int copyModule(sls_detector_module *destMod, sls_detector_module *srcMod) {

	int idac,  ichan;
	int ret=OK;

	LOG(logDEBUG1, ("Copying module\n"));

	if (srcMod->serialnumber>=0) {

		destMod->serialnumber=srcMod->serialnumber;
	}
	//no trimbit feature
	if (destMod->nchan && ((srcMod->nchan)>(destMod->nchan))) {
		LOG(logINFO, ("Number of channels of source is larger than number of channels of destination\n"));
		return FAIL;
	}
	if ((srcMod->ndac)>(destMod->ndac)) {
		LOG(logINFO, ("Number of dacs of source is larger than number of dacs of destination\n"));
		return FAIL;
	}

	LOG(logDEBUG1, ("DACs: src %d, dest %d\n",srcMod->ndac,destMod->ndac));
	LOG(logDEBUG1, ("Chans: src %d, dest %d\n",srcMod->nchan,destMod->nchan));
	destMod->ndac=srcMod->ndac;
	destMod->nchip=srcMod->nchip;
	destMod->nchan=srcMod->nchan;
	if (srcMod->reg>=0)
		destMod->reg=srcMod->reg;
	if (srcMod->iodelay>=0)
		destMod->iodelay=srcMod->iodelay;
	if (srcMod->tau>=0)
		destMod->tau=srcMod->tau;
	if (srcMod->eV>=0)
		destMod->eV=srcMod->eV;
	LOG(logDEBUG1, ("Copying register %x (%x)\n",destMod->reg,srcMod->reg ));

	if (destMod->nchan!=0) {
		for (ichan=0; ichan<(srcMod->nchan); ichan++) {
			if (*((srcMod->chanregs)+ichan)>=0)
				*((destMod->chanregs)+ichan)=*((srcMod->chanregs)+ichan);
		}
	}
	else LOG(logINFO, ("Not Copying trimbits\n"));

	for (idac=0; idac<(srcMod->ndac); idac++) {
		if (*((srcMod->dacs)+idac)>=0) {
			*((destMod->dacs)+idac)=*((srcMod->dacs)+idac);
		}
	}
	return ret;
}


int calculateDataBytes() {
	if (send_to_ten_gig)
		return setDynamicRange(-1) * ONE_GIGA_CONSTANT * TEN_GIGA_BUFFER_SIZE;
	else
		return setDynamicRange(-1) * TEN_GIGA_CONSTANT * ONE_GIGA_BUFFER_SIZE;
}




int getTotalNumberOfChannels() {return  (getNumberOfChannelsPerChip() * getNumberOfChips());}
int getNumberOfChips() {return  NCHIP;}
int getNumberOfDACs() {return  NDAC;}
int getNumberOfChannelsPerChip() {return  NCHAN;}
