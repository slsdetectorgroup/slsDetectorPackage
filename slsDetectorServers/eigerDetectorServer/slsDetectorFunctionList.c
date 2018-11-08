#include "slsDetectorFunctionList.h"
#include "gitInfoEiger.h"
#include "versionAPI.h"
#include "logger.h"

#ifndef VIRTUAL
#include "FebControl.h"
#include "Beb.h"
#endif

#include <unistd.h> //to gethostname
#include <string.h>
#ifdef VIRTUAL
#include <pthread.h>
#include <time.h>
#endif

// Global variable from slsDetectorServer_funcs
extern int debugflag;


// Global variable from communication_funcs.c
extern int isControlServer;

int firmware_compatibility = OK;
int firmware_check_done = 0;
char firmware_message[MAX_STR_LENGTH];


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
int eiger_readoutmode = 0;
int eiger_storeinmem = 0;
int eiger_overflow32 = 0;
int eiger_readoutspeed = 0;
int eiger_triggermode = 0;
int eiger_extgating = 0;
int eiger_extgatingpolarity = 0;
int eiger_nexposures = 1;
int eiger_ncycles = 1;


#ifdef VIRTUAL
//values for virtual server
double eiger_virtual_exptime = 0;
int64_t eiger_virtual_subexptime = 0;
int64_t eiger_virtual_subperiod = 0;
double eiger_virtual_period = 0;
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
#endif




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
	FILE_LOG(logINFOBLUE, ("************ EIGER Virtual Server *****************\n\n"));
	firmware_check_done = 1;
	return;
#endif
	uint32_t ipadd	= getDetectorIP();
	uint64_t macadd	= getDetectorMAC();
	int64_t fwversion = getDetectorId(DETECTOR_FIRMWARE_VERSION);
	int64_t swversion = getDetectorId(DETECTOR_SOFTWARE_VERSION);
	int64_t sw_fw_apiversion = getDetectorId(SOFTWARE_FIRMWARE_API_VERSION);
	int64_t client_sw_apiversion = getDetectorId(CLIENT_SOFTWARE_API_VERSION);

	FILE_LOG(logINFOBLUE, ("**************** EIGER Server *********************\n\n"
			"Detector IP Addr:\t\t 0x%x\n"
			"Detector MAC Addr:\t\t 0x%llx\n"

			"Firmware Version:\t\t %lld\n"
			"Software Version:\t\t %llx\n"
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

	// return if debugflag is not zero, debug mode
	if (debugflag) {
		firmware_check_done = 1;
		return;
	}

	//cant read versions
	if (!fwversion || !sw_fw_apiversion) {
		strcpy(firmware_message, "Cant read versions from FPGA. Please update firmware.\n");
		FILE_LOG(logERROR, (firmware_message));
		firmware_compatibility = FAIL;
		firmware_check_done = 1;
		return;
	}

	//check for API compatibility - old server
	if (sw_fw_apiversion > REQUIRED_FIRMWARE_VERSION) {
		sprintf(firmware_message, "This detector software software version (%lld) is incompatible.\n"
				"Please update detector software (min. %lld) to be compatible with this firmware.\n",
				(long long int)sw_fw_apiversion,
				(long long int)REQUIRED_FIRMWARE_VERSION);
		FILE_LOG(logERROR, (firmware_message));
		firmware_compatibility = FAIL;
		firmware_check_done = 1;
		return;
	}

	//check for firmware compatibility - old firmware
	if ( REQUIRED_FIRMWARE_VERSION > fwversion) {
		sprintf(firmware_message, "This firmware version (%lld) is incompatible.\n"
				"Please update firmware (min. %lld) to be compatible with this server.\n",
				(long long int)fwversion,
				(long long int)REQUIRED_FIRMWARE_VERSION);
		FILE_LOG(logERROR, (firmware_message));
		firmware_compatibility = FAIL;
		firmware_check_done = 1;
		return;
	}
	FILE_LOG(logINFO, ("Compatibility - success\n"));
	firmware_check_done = 1;
}





/* Ids */

int64_t getDetectorId(enum idMode arg) {
#ifdef VIRTUAL
	return 0;
#else
	int64_t retval = -1;

	switch(arg) {
	case DETECTOR_SERIAL_NUMBER:
		retval =  getDetectorNumber();/** to be implemented with mac? */
		break;
	case DETECTOR_FIRMWARE_VERSION:
		return (int64_t)getFirmwareVersion();
	case SOFTWARE_FIRMWARE_API_VERSION:
		return (int64_t)Beb_GetFirmwareSoftwareAPIVersion();
	case DETECTOR_SOFTWARE_VERSION:
		return  (GITDATE & 0xFFFFFF);
	case CLIENT_SOFTWARE_API_VERSION:
		return APIEIGER;
	default:
		break;
	}

	return retval;
#endif
}

u_int64_t getFirmwareVersion() {
#ifdef VIRTUAL
	return 0;
#else
	return Beb_GetFirmwareRevision();
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
#ifdef VIRTUAL
	return 0;
#else
	char mac[255]="";
	u_int64_t res=0;

	//execute and get address
	char output[255];
	FILE* sysFile = popen("more /sys/class/net/eth0/address", "r");
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
	sscanf(mac,"%llx",&res);
	//increment by 1 for 10g
	if (send_to_ten_gig)
		res++;
	//FILE_LOG(logINFO, ("mac:%llx\n",res));

	return res;
#endif
}


u_int32_t  getDetectorIP() {
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
	//FILE_LOG(logINFO, ("ip:%x\n",res));

	return res;
}





/* initialization */

void initControlServer() {
#ifdef VIRTUAL
	getModuleConfiguration();
	setupDetector();
	return;
#else
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
	FILE_LOG(logDEBUG1, ("Control server: FEB Initialization done\n"));
	Beb_Beb(detid);
	Beb_SetDetectorNumber(getDetectorNumber());
	FILE_LOG(logDEBUG1, ("Control server: BEB Initialization done\n"));


	setupDetector();
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
	FILE_LOG(logDEBUG1, ("Stop server: FEB Initialization done\n"));
#endif
}


void getModuleConfiguration() {
#ifdef VIRTUAL
#ifdef VIRTUAL_MASTER
	master = 1;
	top = 1;
#else
	master = 0;
	top = 1;
#endif
#ifdef VIRTUAL_9M
	normal = 0;
#else
	normal = 1;
#endif
	FILE_LOG(logINFOBLUE, ("Module: %s %s %s\n",
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
		FILE_LOG(logINFOBLUE, ("Module: %s %s %s\n",
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
		FILE_LOG(logINFOBLUE, ("Detector ID: %u\n\n", detid));
	}
#endif
}



/* set up detector */

void allocateDetectorStructureMemory() {
	FILE_LOG(logINFO, ("This Server is for 1 Eiger half module (250k)\n\n"));

	//Allocation of memory
	detectorModules = malloc(sizeof(sls_detector_module));
	detectorChans = malloc(NCHIP*NCHAN*sizeof(int));
	detectorDacs = malloc(NDAC*sizeof(int));
	FILE_LOG(logDEBUG1, ("modules from 0x%x to 0x%x\n",detectorModules, detectorModules));
	FILE_LOG(logDEBUG1, ("chans from 0x%x to 0x%x\n",detectorChans, detectorChans));
	FILE_LOG(logDEBUG1, ("dacs from 0x%x to 0x%x\n",detectorDacs, detectorDacs));
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
	FILE_LOG(logINFOBLUE, ("Setting Default Dac values\n"));
	{
		int i = 0;
		int retval[2]={-1,-1};
		const int defaultvals[NDAC] = DEFAULT_DAC_VALS;
		for(i = 0; i < NDAC; ++i) {
			setDAC((enum DACINDEX)i,defaultvals[i],0,retval);
			if (retval[0] != defaultvals[i]) {
				FILE_LOG(logERROR, ("Setting dac %d failed, wrote %d, read %d\n",i ,defaultvals[i], retval[0]));
			}
		}
	}

	FILE_LOG(logINFOBLUE, ("Setting Default Parameters\n"));
	//setting default measurement parameters
	setTimer(FRAME_NUMBER, DEFAULT_NUM_FRAMES);
	setTimer(ACQUISITION_TIME, DEFAULT_EXPTIME);
	setTimer(SUBFRAME_ACQUISITION_TIME, DEFAULT_SUBFRAME_EXPOSURE);
	setTimer(SUBFRAME_DEADTIME, DEFAULT_SUBFRAME_DEADTIME);
	setTimer(FRAME_PERIOD, DEFAULT_PERIOD);
	setTimer(CYCLES_NUMBER, DEFAULT_NUM_CYCLES);
	setDynamicRange(DEFAULT_DYNAMIC_RANGE);
	eiger_photonenergy = DEFAULT_PHOTON_ENERGY;
	setReadOutFlags(DEFAULT_READOUT_MODE);
	setReadOutFlags(DEFAULT_READOUT_STOREINRAM_MODE);
	setReadOutFlags(DEFAULT_READOUT_OVERFLOW32_MODE);
	setSpeed(DEFAULT_CLK_SPEED);//clk_devider,half speed
	setIODelay(DEFAULT_IO_DELAY);
	setTiming(DEFAULT_TIMING_MODE);
	//SetPhotonEnergyCalibrationParameters(-5.8381e-5,1.838515,5.09948e-7,-4.32390e-11,1.32527e-15);
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
	FILE_LOG(logDEBUG1, ("Setup detector done\n\n"));
}




/* advanced read/write reg */
uint32_t writeRegister(uint32_t offset, uint32_t data) {
#ifdef VIRTUAL
	return 0;
#else
	return Feb_Control_WriteRegister(offset, data);
#endif
}

uint32_t readRegister(uint32_t offset) {
#ifdef VIRTUAL
	return 0;
#else
	return Feb_Control_ReadRegister(offset);
#endif
}


/* set parameters -  dr, roi */


int setDynamicRange(int dr) {
#ifdef VIRTUAL
	if (dr > 0) {
		FILE_LOG(logINFO, ("Setting dynamic range: %d\n", dr));
		eiger_dynamicrange = dr;
	}
	return eiger_dynamicrange;
#else
	if (dr > 0) {
		FILE_LOG(logDEBUG1, ("Setting dynamic range: %d\n", dr));
		if (Feb_Control_SetDynamicRange(dr)) {

			//EigerSetBitMode(dr);
			on_dst = 0;
			int i;
			for(i=0;i<32;i++) dst_requested[i] = 0; //clear dst requested
			if (Beb_SetUpTransferParameters(dr))
				eiger_dynamicrange = dr;
			else FILE_LOG(logERROR, ("Could not set bit mode in the back end\n"));
		}
	}
	//make sure back end and front end have the same bit mode
	dr= Feb_Control_GetDynamicRange();

	return dr;
#endif
}




/* parameters - readout */

enum speedVariable setSpeed(int val) {

	if (val != -1) {
		FILE_LOG(logDEBUG1, ("Setting Read out Speed: %d\n", val));
#ifndef VIRTUAL
		if (Feb_Control_SetReadoutSpeed(val))
#endif
			eiger_readoutspeed = val;
	}
	return 	eiger_readoutspeed;
}


enum readOutFlags setReadOutFlags(enum readOutFlags val) {

	enum readOutFlags retval = GET_READOUT_FLAGS;
	if (val!=GET_READOUT_FLAGS) {


		if (val&0xF0000) {
			switch(val) {
			case PARALLEL:
				val=E_PARALLEL;
				FILE_LOG(logDEBUG1, ("Setting Read out Flag: Parallel\n"));
				break;
			case NONPARALLEL:
				val=E_NON_PARALLEL;
				FILE_LOG(logDEBUG1, ("Setting Read out Flag: Non Parallel\n"));
				break;
			case SAFE:
				val=E_SAFE;
				FILE_LOG(logDEBUG1, ("Setting Read out Flag: Safe\n"));
				break;

			default:
				FILE_LOG(logERROR, ("Cannot set unknown readout flag. 0x%x\n", val));
				return -1;
			}
#ifndef VIRTUAL
			if (Feb_Control_SetReadoutMode(val))
#endif
				eiger_readoutmode = val;
#ifndef VIRTUAL
			else return -1;
#endif

		}

		else if (val&0xF00000) {
			switch(val) {
			case SHOW_OVERFLOW:
				val=1;
				FILE_LOG(logDEBUG1, ("Setting Read out Flag: Overflow in 32 bit mode\n"));
				break;
			case NOOVERFLOW:
				val=0;
				FILE_LOG(logDEBUG1, ("Setting Read out Flag: No overflow in 32 bit mode\n"));
				break;
			default:
				FILE_LOG(logERROR, ("Cannot set unknown readout flag. 0x%x\n", val));
				return -1;
			}
#ifndef VIRTUAL
			if (Beb_Set32bitOverflow(val) != -1)
#endif
				eiger_overflow32 = val;
#ifndef VIRTUAL
			else return -1;
#endif
		}


		else {
			switch(val) {
			case STORE_IN_RAM:
				val=1;
				FILE_LOG(logDEBUG1, ("Setting Read out Flag: Store in Ram\n"));
				break;
			case CONTINOUS_RO:
				val=0;
				FILE_LOG(logDEBUG1, ("Setting Read out Flag: Continuous Readout\n"));
				break;

			default:
				FILE_LOG(logERROR, ("Cannot set unknown readout flag. 0x%x\n", val));
				return -1;
			}
			eiger_storeinmem = val;

		}
	}

	switch(eiger_readoutmode) {
	case E_PARALLEL:
		retval=PARALLEL;
		break;
	case E_NON_PARALLEL:
		retval=NONPARALLEL;
		break;
	case E_SAFE:
		retval=SAFE;
		break;
	}

	switch(eiger_overflow32) {
	case 1:
		retval|=SHOW_OVERFLOW;
		break;
	case 0:
		retval|=NOOVERFLOW;
		break;
	}


	switch(eiger_storeinmem) {
	case 0:
		retval|=CONTINOUS_RO;
		break;
	case 1:
		retval|=STORE_IN_RAM;
		break;
	}

	FILE_LOG(logDEBUG1, ("Read out Flag: 0x%x\n", retval));
	return retval;
}








/* parameters - timer */

int64_t setTimer(enum timerIndex ind, int64_t val) {
#ifndef VIRTUAL
	int64_t subdeadtime = 0;
#endif
	int64_t subexptime = 0;
	switch(ind) {
	case FRAME_NUMBER:
		if (val >= 0) {
			FILE_LOG(logDEBUG1, ("Setting number of frames: %d * %d\n", (unsigned int)val, eiger_ncycles));
#ifndef VIRTUAL
			if (Feb_Control_SetNExposures((unsigned int)val*eiger_ncycles)) {
				eiger_nexposures = val;
				//SetDestinationParameters(EigerGetNumberOfExposures()*EigerGetNumberOfCycles());
				on_dst = 0;
				int i;
				for(i=0;i<32;i++) dst_requested[i] = 0; //clear dst requested
				ndsts_in_use = 1;
				nimages_per_request = eiger_nexposures * eiger_ncycles;
			}
#else
			eiger_nexposures = val;
			nimages_per_request = eiger_nexposures * eiger_ncycles;
#endif
		}return eiger_nexposures;

	case ACQUISITION_TIME:
		if (val >= 0) {
			FILE_LOG(logDEBUG1, ("Setting exp time: %fs\n", val/(1E9)));
#ifndef VIRTUAL
			Feb_Control_SetExposureTime(val/(1E9));
#else
			eiger_virtual_exptime = (val/(1E9));
#endif
		}
#ifndef VIRTUAL
		return (Feb_Control_GetExposureTime()*(1E9));
#else
		return eiger_virtual_exptime*1e9;
#endif

	case SUBFRAME_ACQUISITION_TIME:
		if (val >= 0) {
			FILE_LOG(logDEBUG1, ("Setting sub exp time: %lldns\n", (long long int)val));
#ifndef VIRTUAL
			// calculate subdeadtime before settings subexptime
			subdeadtime = Feb_Control_GetSubFramePeriod() -
					Feb_Control_GetSubFrameExposureTime();

			Feb_Control_SetSubFrameExposureTime(val/10);
			// set subperiod
			Feb_Control_SetSubFramePeriod((val+subdeadtime)/10);
#else
			int64_t subdeadtime = eiger_virtual_subperiod*10 -
					eiger_virtual_subexptime*10;
			eiger_virtual_subexptime = (val/(10));
			eiger_virtual_subperiod = (val+subdeadtime/10);
#endif
		}
#ifndef VIRTUAL
		return (Feb_Control_GetSubFrameExposureTime());
#else
		return eiger_virtual_subexptime*10;
#endif

	case SUBFRAME_DEADTIME:
#ifndef VIRTUAL
		// get subexptime
		subexptime = Feb_Control_GetSubFrameExposureTime();
#else
		subexptime = eiger_virtual_subexptime*10;
#endif
		if (val >= 0) {
			FILE_LOG(logINFO, ("Setting sub period (subdeadtime(%lld)): %lldns\n",
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
		}
#ifndef VIRTUAL
		return (Feb_Control_GetSubFramePeriod() - subexptime);
#else
		return (eiger_virtual_subperiod*10 - subexptime);
#endif

	case FRAME_PERIOD:
		if (val >= 0) {
			FILE_LOG(logDEBUG1, ("Setting acq period: %fs\n", val/(1E9)));
#ifndef VIRTUAL
			Feb_Control_SetExposurePeriod(val/(1E9));
#else
			eiger_virtual_period = (val/(1E9));
#endif
		}
#ifndef VIRTUAL
		return (Feb_Control_GetExposurePeriod()*(1E9));
#else
		return eiger_virtual_period*1e9;
#endif

	case CYCLES_NUMBER:
		if (val >= 0) {
			FILE_LOG(logDEBUG1, ("Setting number of triggers: %d * %d\n",
					(unsigned int)val,eiger_nexposures));
#ifndef VIRTUAL
			if (Feb_Control_SetNExposures((unsigned int)val*eiger_nexposures)) {
				eiger_ncycles = val;
				//SetDestinationParameters(EigerGetNumberOfExposures()*EigerGetNumberOfCycles());
				on_dst = 0;
				int i;
				for(i=0;i<32;i++) dst_requested[i] = 0; //clear dst requested
				nimages_per_request = eiger_nexposures * eiger_ncycles;
			}
#else
			eiger_ncycles = val;
			nimages_per_request = eiger_nexposures * eiger_ncycles;
#endif
		}
		return eiger_ncycles;
	default:
		FILE_LOG(logERROR, ("Timer Index not implemented for this detector: %d\n", ind));
		break;
	}

	return -1;
}


int64_t getTimeLeft(enum timerIndex ind) {
#ifdef VIRTUAL
	return 0;
#else
	switch(ind) {
	case MEASURED_PERIOD: return Feb_Control_GetMeasuredPeriod();
	case MEASURED_SUBPERIOD: return Feb_Control_GetSubMeasuredPeriod();
	return 0;
	default:
		FILE_LOG(logERROR, ("This timer left index (%d) not defined for Eiger\n", ind));
		return -1;
	}
#endif
}




/* parameters - channel, module, settings */


int setModule(sls_detector_module myMod, char* mess) {


	FILE_LOG(logINFO, ("Setting module with settings %d\n",myMod.reg));

	// settings
	setSettings( (enum detectorSettings)myMod.reg);

	//copy module locally (module number, serial number
	//dacs (pointless), trimbit values(if needed)
	if (detectorModules) {
		if (copyModule(detectorModules,&myMod) == FAIL) {
			sprintf(mess, "Could not copy module\n");
			FILE_LOG(logERROR, (mess));
			setSettings(UNDEFINED);
			FILE_LOG(logERROR, ("Settings has been changed to undefined\n"));
			return FAIL;
		}
	}

	// iodelay
	if (setIODelay(myMod.iodelay)!= myMod.iodelay) {
		sprintf(mess, "Could not set module. Could not set iodelay %d\n", myMod.iodelay);
		FILE_LOG(logERROR, (mess));
		setSettings(UNDEFINED);
		FILE_LOG(logERROR, ("Settings has been changed to undefined\n"));
		return FAIL;
	}

	// threshold
	if (myMod.eV >= 0)
		setThresholdEnergy(myMod.eV);
	else {
		// (loading a random trim file) (dont return fail)
		setSettings(UNDEFINED);
		FILE_LOG(logERROR, ("Settings has been changed to undefined (random trim file)\n"));
	}

	// dacs
	{
		int i = 0;
		int retval[2] = {0, 0};
		for(i = 0; i < myMod.ndac; ++i) {
			setDAC((enum DACINDEX)i, myMod.dacs[i] , 0, retval);
			if (myMod.dacs[i] != retval[0]) {
				sprintf(mess, "Could not set module. Could not set dac %d\n", i);
				FILE_LOG(logERROR, (mess));
				setSettings(UNDEFINED);
				FILE_LOG(logERROR, ("Settings has been changed to undefined\n"));
				return FAIL;
			}
		}
	}
	// trimbits
#ifndef VIRTUAL
	if (myMod.nchan == 0) {
		FILE_LOG(logINFO, ("Setting module without trimbits\n"));
	} else {
		FILE_LOG(logINFO, ("Setting module with trimbits\n"));
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
			FILE_LOG(logERROR, (mess));
			setSettings(UNDEFINED);
			FILE_LOG(logERROR, ("Settings has been changed to undefined (random trim file)\n"));
			return FAIL;
		}
	}


	//rate correction
	//switch off rate correction: no value read from load settings)
	if (myMod.tau == -1) {
		if (getRateCorrectionEnable()) {
			setRateCorrection(0);
			sprintf(mess,"Cannot set module. Cannot set Rate correction. "
					"No default tau provided. Deactivating Rate Correction\n");
			FILE_LOG(logERROR, (mess));
			setSettings(UNDEFINED);
			FILE_LOG(logERROR, ("Settings has been changed to undefined (random trim file)\n"));
			return FAIL;
		}
	}
	//normal tau value (only if enabled)
	else {
		setDefaultSettingsTau_in_nsec(myMod.tau);
		if (getRateCorrectionEnable()) {
			int64_t retvalTau = setRateCorrection(myMod.tau);
			if (myMod.tau != retvalTau) {
				sprintf(mess, "Cannot set module. Could not set rate correction\n");
				FILE_LOG(logERROR, (mess));
				setSettings(UNDEFINED);
				FILE_LOG(logERROR, ("Settings has been changed to undefined (random trim file)\n"));
				return FAIL;
			}
		}
	}
#endif
	return OK;
}


int getModule(sls_detector_module *myMod) {

#ifndef VIRTUAL
	int i;
	int retval[2];

	//dacs
	for(i=0;i<NDAC;i++) {
		setDAC((enum DACINDEX)i,-1,0,retval);
		//FILE_LOG(logINFO,"dac%d:%d\n",i, *((detectorModules->dacs)+i));
	}

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
	FILE_LOG(logINFO, ("Settings: %d\n", thisSettings));
	return thisSettings;
}

enum detectorSettings getSettings() {
	return thisSettings;
}






/* parameters - threshold */

int getThresholdEnergy() {
	FILE_LOG(logINFO, ("Getting Threshold energy\n"));
	return eiger_photonenergy;
}


int setThresholdEnergy(int ev) {
	FILE_LOG(logINFO, ("Setting threshold energy:%d\n",ev));
	if (ev >= 0)
		eiger_photonenergy = ev;
	return  getThresholdEnergy();
}





/* parameters - dac, adc, hv */

void setDAC(enum DACINDEX ind, int val, int mV, int retval[]) {
	FILE_LOG(logDEBUG1, ("Going to set dac %d to %d with mv mode %d \n", (int)ind, val, mV));
	if (ind == VTHRESHOLD) {
		int ret[5];
		setDAC(VCMP_LL,val,mV,retval);
		ret[0] = retval[mV];
		setDAC(VCMP_LR,val,mV,retval);
		ret[1] = retval[mV];
		setDAC(VCMP_RL,val,mV,retval);
		ret[2] = retval[mV];
		setDAC(VCMP_RR,val,mV,retval);
		ret[3] = retval[mV];
		setDAC(VCP,val,mV,retval);
		ret[4] = retval[mV];


		if ((ret[0]== ret[1])&&
				(ret[1]==ret[2])&&
				(ret[2]==ret[3]) && 
				(ret[3]==ret[4])) {
			FILE_LOG(logINFO, ("vthreshold match\n"));
		} else {
			retval[0] = -1;retval[1] = -1;
			FILE_LOG(logERROR, ("vthreshold mismatch 0:%d 1:%d 2:%d 3:%d\n",
					ret[0],ret[1],ret[2],ret[3]));
		}
		return;
	}
	char iname[10];

	if (((int)ind>=0)&&((int)ind<NDAC)) {
		strcpy(iname,dac_names[(int)ind]);
	} else {
		FILE_LOG(logINFO, ("dac value outside range:%d\n",(int)ind));
		strcpy(iname,dac_names[0]);
	}
	FILE_LOG(logDEBUG1, ("%s dac %d: %s to %d %s\n",
			(val >= 0) ? "Setting" : "Getting",
					ind, iname, val,
					mV ? "mV" : "dac units"));
#ifdef VIRTUAL
	if (mV) {
		retval[0] = (int)(((val-0)/(2048-0))*(4096-1) + 0.5);
		retval[1] = val;
	} else
		retval[0] = val;

#else
	if (val >= 0)
		Feb_Control_SetDAC(iname,val,mV);
	int k;
	Feb_Control_GetDAC(iname, &k,0);
	retval[0] = k;
	Feb_Control_GetDAC(iname,&k,1);
	retval[1] = k;
#endif
	(detectorModules)->dacs[ind] = retval[0];

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

	FILE_LOG(logINFO, ("Temperature %s: %f°C\n", tempnames[ind], (double)retval/1000.00));

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
			FILE_LOG(logERROR, ("Could not read high voltage\n"));
			return -3;
		}

		// tolerance of 5
		if (abs(eiger_theo_highvoltage-eiger_highvoltage) > HIGH_VOLTAGE_TOLERANCE) {
			FILE_LOG(logINFO, ("High voltage still ramping: %d\n", eiger_highvoltage));
			return eiger_highvoltage;
		}
		return eiger_theo_highvoltage;
	}

	return SLAVE_HIGH_VOLTAGE_READ_VAL;
#endif
}







/* parameters - timing, extsig */

void setTiming( enum externalCommunicationMode arg) {
	enum externalCommunicationMode ret=GET_EXTERNAL_COMMUNICATION_MODE;
	if (arg != GET_EXTERNAL_COMMUNICATION_MODE) {
		switch((int)arg) {
		case AUTO_TIMING:			ret = 0;	break;
		case TRIGGER_EXPOSURE:		ret = 2;	break;
		case BURST_TRIGGER:			ret = 1;	break;
		case GATE_FIX_NUMBER:		ret = 3;	break;
		}
		FILE_LOG(logDEBUG1, ("Setting Triggering Mode: %d\n", (int)ret));
#ifndef VIRTUAL
		if (Feb_Control_SetTriggerMode(ret,1))
#endif
			eiger_triggermode = ret;
	}
}


enum externalCommunicationMode getTiming() {
	enum externalCommunicationMode ret = GET_EXTERNAL_COMMUNICATION_MODE;
	ret = eiger_triggermode;
	switch((int)ret) {
	case 0:		ret = AUTO_TIMING;		break;
	case 2:		ret = TRIGGER_EXPOSURE; break;
	case 1:		ret = BURST_TRIGGER;	break;
	case 3:		ret = GATE_FIX_NUMBER;	break;
	default:
		FILE_LOG(logERROR, ("Unknown trigger mode found %d\n", ret));
		ret = 0;
	}
	return ret;
}



/* configure mac */

int configureMAC(uint32_t destip, uint64_t destmac, uint64_t sourcemac, uint32_t sourceip, uint32_t udpport, uint32_t udpport2) {
#ifndef VIRTUAL
    FILE_LOG(logINFO, ("Configuring MAC\n"));
	char src_mac[50], src_ip[50],dst_mac[50], dst_ip[50];
	int src_port = 0xE185;
	sprintf(src_ip,"%d.%d.%d.%d",(sourceip>>24)&0xff,(sourceip>>16)&0xff,(sourceip>>8)&0xff,(sourceip)&0xff);
	sprintf(dst_ip,"%d.%d.%d.%d",(destip>>24)&0xff,(destip>>16)&0xff,(destip>>8)&0xff,(destip)&0xff);
	sprintf(src_mac,"%02x:%02x:%02x:%02x:%02x:%02x",(unsigned int)((sourcemac>>40)&0xFF),
			(unsigned int)((sourcemac>>32)&0xFF),
			(unsigned int)((sourcemac>>24)&0xFF),
			(unsigned int)((sourcemac>>16)&0xFF),
			(unsigned int)((sourcemac>>8)&0xFF),
			(unsigned int)((sourcemac>>0)&0xFF));
	sprintf(dst_mac,"%02x:%02x:%02x:%02x:%02x:%02x",(unsigned int)((destmac>>40)&0xFF),
			(unsigned int)((destmac>>32)&0xFF),
			(unsigned int)((destmac>>24)&0xFF),
			(unsigned int)((destmac>>16)&0xFF),
			(unsigned int)((destmac>>8)&0xFF),
			(unsigned int)((destmac>>0)&0xFF));



	FILE_LOG(logINFO, (
	        "\tSource IP   : %s\n"
	        "\tSource MAC  : %s\n"
	        "\tSource Port : %d\n"
	        "\tDest IP     : %s\n"
	        "\tDest MAC    : %s\n",
	        src_ip, src_mac, src_port,
	        dst_ip, dst_mac));


	int beb_num =  detid;
	int header_number = 0;
	int dst_port = udpport;
	if (!top)
		dst_port = udpport2;

	FILE_LOG(logINFO, ("\tDest Port   : %d\n", dst_port));

	int i=0;
	/* for(i=0;i<32;i++) { modified for Aldo*/
	if (Beb_SetBebSrcHeaderInfos(beb_num,send_to_ten_gig,src_mac,src_ip,src_port) &&
			Beb_SetUpUDPHeader(beb_num,send_to_ten_gig,header_number+i,dst_mac,dst_ip, dst_port)) {
		FILE_LOG(logDEBUG1, ("\tset up left ok\n"));
	} else {
		return FAIL;
	}
	/*}*/

	header_number = 32;
	dst_port = udpport2;
	if (!top)
		dst_port = udpport;
	FILE_LOG(logINFO, ("\tDest Port   : %d\n",dst_port));

	/*for(i=0;i<32;i++) {*//** modified for Aldo*/
	if (Beb_SetBebSrcHeaderInfos(beb_num,send_to_ten_gig,src_mac,src_ip,src_port) &&
			Beb_SetUpUDPHeader(beb_num,send_to_ten_gig,header_number+i,dst_mac,dst_ip, dst_port)) {
		FILE_LOG(logDEBUG1, (" set up right ok\n"));
	} else {
		return FAIL;
	}
	/*}*/

	on_dst = 0;

	for(i=0;i<32;i++) dst_requested[i] = 0; //clear dst requested
	nimages_per_request=eiger_nexposures * eiger_ncycles;
#endif
	return OK;
}



int	setDetectorPosition(int pos[]) {
#ifdef VIRTUAL
	return OK;
#else
	return Beb_SetDetectorPosition(pos);
#endif
}





/* eiger specific - iodelay, 10g, pulse, rate, temp, activate, delay nw parameter */

int setIODelay(int val) {
	if (val!=-1) {
		FILE_LOG(logDEBUG1, ("Setting IO Delay: %d\n",val));
#ifndef VIRTUAL
		if (Feb_Control_SetIDelays(Feb_Control_GetModuleNumber(),val))
#endif
			eiger_iodelay = val;
	}
	return eiger_iodelay;
}


int enableTenGigabitEthernet(int val) {
	if (val!=-1) {
		FILE_LOG(logINFO, ("Setting 10Gbe: %d\n", (val > 0) ? 1 : 0));
		if (val>0)
			send_to_ten_gig = 1;
		else
			send_to_ten_gig = 0;
		//configuremac called from client
	}
	return send_to_ten_gig;
}


int setCounterBit(int val) {
	if (val!=-1) {
		FILE_LOG(logINFO, ("Setting Counter Bit: %d\n",val));
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

int64_t setRateCorrection(int64_t custom_tau_in_nsec) {//in nanosec (will never be -1)
#ifdef VIRTUAL
	//deactivating rate correction
	if (custom_tau_in_nsec==0) {
		eiger_virtual_ratecorrection_variable = 0;
		return 0;
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
			FILE_LOG(logINFO, ("Rate Table already created before: Same Tau %lldns, Same subexptime %lldns\n",
					(long long int)tau_in_nsec,(long long int)ratetable_period_in_nsec));
		} else {
			FILE_LOG(logINFO, ("Rate Table already created before: Same Tau %lldns, Same exptime %lldns\n",
					(long long int)tau_in_nsec,(long long int)ratetable_period_in_nsec));
		}
	}
	//different setting, calculate table
	else {
		eiger_virtual_ratetable_tau_in_ns = custom_tau_in_nsec;
		double period_in_sec = (double)(eiger_virtual_subexptime*10)/(double)1e9;
		if (eiger_dynamicrange == 16)
			period_in_sec = eiger_virtual_exptime;
		eiger_virtual_ratetable_period_in_ns = period_in_sec*1e9;
	}
	//activating rate correction
	eiger_virtual_ratecorrection_variable = 1;
	FILE_LOG(logINFO, ("Rate Correction Value set to %lld ns\n",(long long int)eiger_virtual_ratetable_tau_in_ns));

	return eiger_virtual_ratetable_tau_in_ns;
#else

	//deactivating rate correction
	if (custom_tau_in_nsec==0) {
		Feb_Control_SetRateCorrectionVariable(0);
		return 0;
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
			FILE_LOG(logINFO, ("Rate Table already created before: Same Tau %lldns, Same subexptime %lldns\n",
					tau_in_nsec,ratetable_period_in_nsec));
		} else {
			FILE_LOG(logINFO, ("Rate Table already created before: Same Tau %lldns, Same exptime %lldns\n",
					tau_in_nsec,ratetable_period_in_nsec));
		}
	}
	//different setting, calculate table
	else {
		int ret = Feb_Control_SetRateCorrectionTau(custom_tau_in_nsec);
		if (ret<=0) {
			FILE_LOG(logERROR, ("Rate correction failed. Deactivating rate correction\n"));
			Feb_Control_SetRateCorrectionVariable(0);
			return ret;
		}
	}
	//activating rate correction
	Feb_Control_SetRateCorrectionVariable(1);
	FILE_LOG(logINFO, ("Rate Correction Value set to %lld ns\n", (long long int)Feb_Control_Get_RateTable_Tau_in_nsec()));
	Feb_Control_PrintCorrectedValues();

	return Feb_Control_Get_RateTable_Tau_in_nsec();
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
	FILE_LOG(logINFO, ("Default tau set to %d\n", default_tau_from_file));
}

int64_t getCurrentTau() {
	if (!getRateCorrectionEnable())
		return 0;
	else
#ifndef VIRTUAL
		return Feb_Control_Get_RateTable_Tau_in_nsec();
#else
	return eiger_virtual_ratetable_tau_in_ns;
#endif
}

void setExternalGating(int enable[]) {
	if (enable>=0) {
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
		FILE_LOG(logERROR, ("Could not set all trimbits\n"));
		return FAIL;
	}
#endif
	if (detectorModules) {
		int ichan;
		for (ichan=0; ichan<(detectorModules->nchan); ichan++) {
			*((detectorModules->chanregs)+ichan)=val;
		}
	}

	FILE_LOG(logINFO, ("All trimbits have been set to %d\n", val));
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
	FILE_LOG(logINFO, ("Value of all Trimbits: %d\n", value));
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

int setNetworkParameter(enum NETWORKINDEX mode, int value) {
#ifndef VIRTUAL
	return Beb_SetNetworkParameter(mode, value);
#else
	if (value>-1) {
		switch(mode) {
		case TXN_LEFT:
			eiger_virtual_transmission_delay_left = value;
			break;
		case TXN_RIGHT:
			eiger_virtual_transmission_delay_right = value;
			break;
		case TXN_FRAME:
			eiger_virtual_transmission_delay_frame = value;
			break;
		case FLOWCTRL_10G:
			eiger_virtual_transmission_flowcontrol_10g = value;
			if (value>0) value = 1;
			break;
		default: FILE_LOG(logERROR, ("Unrecognized mode in network parameter: %d\n",mode));
		return -1;
		}
	}
	switch(mode) {
	case TXN_LEFT:
		return eiger_virtual_transmission_delay_left;
	case TXN_RIGHT:
		return eiger_virtual_transmission_delay_right;
	case TXN_FRAME:
		return eiger_virtual_transmission_delay_frame;
	case FLOWCTRL_10G:
		return eiger_virtual_transmission_flowcontrol_10g;
	default: FILE_LOG(logERROR, ("Unrecognized mode in network parameter: %d\n",mode));
	return -1;
	}
#endif
}







/* aquisition */


int prepareAcquisition() {
#ifndef VIRTUAL
	FILE_LOG(logINFO, ("Going to prepare for acquisition with counter_bit:%d\n",Feb_Control_Get_Counter_Bit()));
	Feb_Control_PrepareForAcquisition();
	FILE_LOG(logINFO, ("Going to reset Frame Number\n"));
	Beb_ResetFrameNumber();
#endif
	return OK;

}


int startStateMachine() {
#ifdef VIRTUAL
	eiger_virtual_status = 1;
	eiger_virtual_stop = 0;
	if (pthread_create(&eiger_virtual_tid, NULL, &start_timer, NULL)) {
		FILE_LOG(logERROR, ("Could not start Virtual acquisition thread\n"));
		eiger_virtual_status = 0;
		return FAIL;
	}
	FILE_LOG(logINFO ,("Virtual Acquisition started\n"));
	return OK;
#else

	int ret = OK,prev_flag;
	//get the DAQ toggle bit
	prev_flag = Feb_Control_AcquisitionStartedBit();

	FILE_LOG(logINFO, ("Going to start acquisition\n"));
	Feb_Control_StartAcquisition();

	if (!eiger_storeinmem) {
		FILE_LOG(logINFO, ("requesting images right after start\n"));
		ret =  startReadOut();
	}

	//wait for acquisition start
	if (ret == OK) {
		if (!Feb_Control_WaitForStartedFlag(5000, prev_flag)) {
			FILE_LOG(logERROR, ("Acquisition did not FILE_LOG(logERROR ouble reading register\n"));
			return FAIL;
		}
		FILE_LOG(logINFOGREEN, ("Acquisition started\n"));
	}

	/*while(getRunStatus() == IDLE) {FILE_LOG(logINFO, ("waiting for being not idle anymore\n"));}*/

	return ret;
#endif
}

#ifdef VIRTUAL
void* start_timer(void* arg) {
	eiger_virtual_status = 1;
	int wait_in_s = nimages_per_request * eiger_virtual_period;
	FILE_LOG(logINFO, ("going to wait for %d s\n", wait_in_s));
	while(!eiger_virtual_stop && (wait_in_s >= 0)) {
		usleep(1000 * 1000);
		wait_in_s--;
	}
	FILE_LOG(logINFO, ("Virtual Timer Done***\n"));

	eiger_virtual_status = 0;
	return NULL;
}
#endif



int stopStateMachine() {
	FILE_LOG(logINFORED, ("Going to stop acquisition\n"));
#ifdef VIRTUAL
	eiger_virtual_stop = 0;
	return OK;
#else

	if ((Feb_Control_StopAcquisition() == STATUS_IDLE) & Beb_StopAcquisition())
		return OK;
	FILE_LOG(logERROR, ("failed to stop acquisition\n"));
	return FAIL;
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

	FILE_LOG(logINFO, ("Requesting images...\n"));
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
		FILE_LOG(logINFO, ("Status: IDLE\n"));
		return IDLE;
	} else {
		FILE_LOG(logINFO, ("Status: RUNNING...\n"));
		return RUNNING;
	}
#else

	int i = Feb_Control_AcquisitionInProgress();
	switch (i) {
	case STATUS_ERROR:
		FILE_LOG(logERROR, ("Status: ERROR reading status register\n"));
		return ERROR;
	case STATUS_IDLE:
		FILE_LOG(logINFOBLUE, ("Status: IDLE\n"));
		return IDLE;
	default:
		FILE_LOG(logINFOBLUE, ("Status: RUNNING...\n"));
		return RUNNING;
	}

	return IDLE;
#endif
}



void readFrame(int *ret, char *mess) {
#ifdef VIRTUAL
	while(eiger_virtual_status) {
		//FILE_LOG(logERROR ,"Waiting for finished flag\n"));
		usleep(5000);
	}
	FILE_LOG(logINFOGREEN, ("acquisition successfully finished\n"));
	return;
#else

	if (Feb_Control_WaitForFinishedFlag(5000) == STATUS_ERROR) {
		FILE_LOG(logERROR, ("Waiting for finished flag\n"));
		*ret = FAIL;
		return;
	}
	FILE_LOG(logINFOGREEN, ("Acquisition finished\n"));

	if (eiger_storeinmem) {
		FILE_LOG(logINFO, ("requesting images after storing in memory\n"));
		if (startReadOut() == FAIL) {
			strcpy(mess,"Could not execute read image requests\n");
			FILE_LOG(logERROR, (mess));
			*ret = (int)FAIL;
			return;
		}
	}

	//wait for detector to send
	Beb_EndofDataSend(send_to_ten_gig);
	FILE_LOG(logINFOGREEN, ("Acquisition successfully finished\n"));
#endif
}








/* common */


int copyModule(sls_detector_module *destMod, sls_detector_module *srcMod) {

	int idac,  ichan;
	int ret=OK;

	FILE_LOG(logDEBUG1, ("Copying module\n"));

	if (srcMod->serialnumber>=0) {

		destMod->serialnumber=srcMod->serialnumber;
	}
	//no trimbit feature
	if (destMod->nchan && ((srcMod->nchan)>(destMod->nchan))) {
		FILE_LOG(logINFO, ("Number of channels of source is larger than number of channels of destination\n"));
		return FAIL;
	}
	if ((srcMod->ndac)>(destMod->ndac)) {
		FILE_LOG(logINFO, ("Number of dacs of source is larger than number of dacs of destination\n"));
		return FAIL;
	}

	FILE_LOG(logDEBUG1, ("DACs: src %d, dest %d\n",srcMod->ndac,destMod->ndac));
	FILE_LOG(logDEBUG1, ("Chans: src %d, dest %d\n",srcMod->nchan,destMod->nchan));
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
	FILE_LOG(logDEBUG1, ("Copying register %x (%x)\n",destMod->reg,srcMod->reg ));

	if (destMod->nchan!=0) {
		for (ichan=0; ichan<(srcMod->nchan); ichan++) {
			if (*((srcMod->chanregs)+ichan)>=0)
				*((destMod->chanregs)+ichan)=*((srcMod->chanregs)+ichan);
		}
	}
	else FILE_LOG(logINFO, ("Not Copying trimbits\n"));

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




int getTotalNumberOfChannels() {return  ((int)getNumberOfChannelsPerChip() * (int)getNumberOfChips());}
int getNumberOfChips() {return  NCHIP;}
int getNumberOfDACs() {return  NDAC;}
int getNumberOfChannelsPerChip() {return  NCHAN;}









