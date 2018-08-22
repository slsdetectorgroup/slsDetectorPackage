//#ifdef SLS_DETECTOR_FUNCTION_LIST


#include <stdio.h>
#include <unistd.h> //to gethostname
#include <string.h>
#ifdef VIRTUAL
#include <pthread.h>
#include <time.h>
#endif

#include "slsDetectorFunctionList.h"
#ifndef VIRTUAL
#include "gitInfoEiger.h"
#include "FebControl.h"
#include "Beb.h"
#include "versionAPI.h"
#endif

int default_tau_from_file= -1;

#define BEB_NUM 34

enum detectorSettings thisSettings;
const char* dac_names[16] = {"SvP","Vtr","Vrf","Vrs","SvN","Vtgstv","Vcmp_ll","Vcmp_lr","cal","Vcmp_rl","rxb_rb","rxb_lb","Vcmp_rr","Vcp","Vcn","Vis"};

enum{E_PARALLEL, E_NON_PARALLEL, E_SAFE};

sls_detector_module *detectorModules=NULL;
int *detectorChips=NULL;
int *detectorChans=NULL;
dacs_t *detectorDacs=NULL;
dacs_t *detectorAdcs=NULL;

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


int send_to_ten_gig = 0;
int  ndsts_in_use=32;
unsigned int nimages_per_request=1;
int  on_dst=0;
int dst_requested[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


int default_gain_values[3] = {517000,517000,517000};
int default_offset_values[3] = {3851000,3851000,3851000};


enum masterFlags  masterMode=IS_SLAVE;
int top = 0;
int master = 0;
int normal = 0;
#ifndef VIRTUAL
uint32_t detid = 0;
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

void checkFirmwareCompatibility(int flag){
	firmware_compatibility = OK;
	firmware_check_done = 0;
	memset(firmware_message, 0, MAX_STR_LENGTH);
#ifdef VIRTUAL
	cprintf(BLUE,"\n\n"
			"********************************************************\n"
			"***************** EIGER Virtual Server *****************\n"
			"********************************************************\n");
	firmware_check_done = 1;
	return;
#endif
	uint32_t ipadd	= getDetectorIP();
	uint64_t macadd	= getDetectorMAC();
	int64_t fwversion = getDetectorId(DETECTOR_FIRMWARE_VERSION);
	int64_t swversion = getDetectorId(DETECTOR_SOFTWARE_VERSION);
	int64_t sw_fw_apiversion = getDetectorId(SOFTWARE_FIRMWARE_API_VERSION);
	int64_t client_sw_apiversion = getDetectorId(CLIENT_SOFTWARE_API_VERSION);

	cprintf(BLUE,"\n\n"
			"********************************************************\n"
			"**********************EIGER Server**********************\n"
			"********************************************************\n");
	cprintf(BLUE,"\n"
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
			(long long int)client_sw_apiversion);

	// return if flag is not zero, debug mode
	if (flag) {
		firmware_check_done = 1;
	    return;
	}

	//cant read versions
	if(!fwversion || !sw_fw_apiversion){
		strcpy(firmware_message,
				"FATAL ERROR: Cant read versions from FPGA. Please update firmware.\n");
		cprintf(RED,"%s\n\n", firmware_message);
		firmware_compatibility = FAIL;
		firmware_check_done = 1;
		return;
	}

	//check for API compatibility - old server
	if(sw_fw_apiversion > REQUIRED_FIRMWARE_VERSION){
		sprintf(firmware_message,
				"FATAL ERROR: This detector software software version (%lld) is incompatible.\n"
				"Please update detector software (min. %lld) to be compatible with this firmware.\n",
				(long long int)sw_fw_apiversion,
				(long long int)REQUIRED_FIRMWARE_VERSION);
		cprintf(RED,"%s\n\n", firmware_message);
		firmware_compatibility = FAIL;
		firmware_check_done = 1;
		return;
	}

	//check for firmware compatibility - old firmware
	if( REQUIRED_FIRMWARE_VERSION > fwversion){
		sprintf(firmware_message,
				"FATAL ERROR: This firmware version (%lld) is incompatible.\n"
				"Please update firmware (min. %lld) to be compatible with this server.\n",
				(long long int)fwversion,
				(long long int)REQUIRED_FIRMWARE_VERSION);
		cprintf(RED,"%s\n\n", firmware_message);
		firmware_compatibility = FAIL;
		firmware_check_done = 1;
		return;
	}
	printf("Compatibility - success\n");
	firmware_check_done = 1;
}





/* Ids */

int64_t getDetectorId(enum idMode arg){
#ifdef VIRTUAL
	return 0;
#else
	int64_t retval = -1;

	switch(arg){
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



u_int32_t getDetectorNumber(){
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
	while (pch != NULL){
		strcat(mac,pch);
		pch = strtok (NULL, ":");
	}
	sscanf(mac,"%llx",&res);
	//increment by 1 for 10g
	if(send_to_ten_gig)
		res++;
	//printf("mac:%llx\n",res);

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
#ifdef VIRTUAL
	getModuleConfiguration();
	setupDetector();
	printf("\n");
	return;
#else
	//Feb and Beb Initializations
	getModuleConfiguration();
	Feb_Interface_FebInterface();
	Feb_Control_FebControl();
	Feb_Control_Init(master,top,normal, getDetectorNumber());
	//master of 9M, check high voltage serial communication to blackfin
	if(master && !normal){
		if(Feb_Control_OpenSerialCommunication())
		;//	Feb_Control_CloseSerialCommunication();
	}
	printf("FEB Initialization done\n");
	Beb_Beb();
	Beb_SetDetectorNumber(getDetectorNumber());
	printf("BEB Initialization done\n");


	setupDetector();
#endif
	printf("\n");
}

void initStopServer(){
#ifdef VIRTUAL
	getModuleConfiguration();
	printf("\n");
	return;
#else
	getModuleConfiguration();
	Feb_Interface_FebInterface();
	Feb_Control_FebControl();
	Feb_Control_Init(master,top,normal,getDetectorNumber());
	printf("FEB Initialization done\n");
#endif
	printf("\n");
}


void getModuleConfiguration(){
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
	if(top)	printf("*************** TOP ***************\n");
	else	printf("*************** BOTTOM ***************\n");
	if(master)	printf("*************** MASTER ***************\n");
	else		printf("*************** SLAVE ***************\n");
	if(normal)	printf("*************** NORMAL ***************\n");
	else		printf("*************** SPECIAL ***************\n");
	return;
#else
	int *m=&master;
	int *t=&top;
	int *n=&normal;
	Beb_GetModuleConfiguration(m,t,n);
	if(top)	printf("*************** TOP ***************\n");
	else	printf("*************** BOTTOM ***************\n");
	if(master)	printf("*************** MASTER ***************\n");
	else		printf("*************** SLAVE ***************\n");
	if(normal)	printf("*************** NORMAL ***************\n");
	else		printf("*************** SPECIAL ***************\n");

	// read detector id
	char output[255];
	FILE* sysFile = popen(IDFILECOMMAND, "r");
	fgets(output, sizeof(output), sysFile);
	pclose(sysFile);
	sscanf(output,"%u",&detid);
	printf("detector id: %u\n",detid);
#endif
}



/* set up detector */

void allocateDetectorStructureMemory(){
	printf("This Server is for 1 Eiger half module (250k)\n");

	//Allocation of memory
	detectorModules=malloc(sizeof(sls_detector_module));
	detectorChips=malloc(NCHIP*sizeof(int));
	detectorChans=malloc(NCHIP*NCHAN*sizeof(int));
	detectorDacs=malloc(NDAC*sizeof(dacs_t));
	detectorAdcs=malloc(NADC*sizeof(dacs_t));
#ifdef VERBOSE
	printf("modules from 0x%x to 0x%x\n",detectorModules, detectorModules+n);
	printf("chips from 0x%x to 0x%x\n",detectorChips, detectorChips+n*NCHIP);
	printf("chans from 0x%x to 0x%x\n",detectorChans, detectorChans+n*NCHIP*NCHAN);
	printf("dacs from 0x%x to 0x%x\n",detectorDacs, detectorDacs+n*NDAC);
	printf("adcs from 0x%x to 0x%x\n",detectorAdcs, detectorAdcs+n*NADC);
#endif
	(detectorModules)->dacs=detectorDacs;
	(detectorModules)->adcs=detectorAdcs;
	(detectorModules)->chipregs=detectorChips;
	(detectorModules)->chanregs=detectorChans;
	(detectorModules)->ndac=NDAC;
	(detectorModules)->nadc=NADC;
	(detectorModules)->nchip=NCHIP;
	(detectorModules)->nchan=NCHIP*NCHAN;
	(detectorModules)->module=0;
	(detectorModules)->gain=0;
	(detectorModules)->offset=0;
	(detectorModules)->reg=0;
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
	printf("Setting Default Dac values\n");
	{
		int i = 0;
		int retval[2]={-1,-1};
		const int defaultvals[NDAC] = DEFAULT_DAC_VALS;
		for(i = 0; i < NDAC; ++i) {
			setDAC((enum DACINDEX)i,defaultvals[i],0,0,retval);
			if (retval[0] != defaultvals[i])
				cprintf(RED, "Warning: Setting dac %d failed, wrote %d, read %d\n",i ,defaultvals[i], retval[0]);
		}
	}

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
	setSpeed(CLOCK_DIVIDER, DEFAULT_CLK_SPEED);//clk_devider,half speed
	setIODelay(DEFAULT_IO_DELAY, DEFAULT_MOD_INDEX);
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


/* set parameters - nmod, dr, roi */

int setNMod(int nm, enum dimension dim){
	return NMOD;
}


int getNModBoard(enum dimension arg){
	return NMAXMOD;
}

int setDynamicRange(int dr){
#ifdef VIRTUAL
	if(dr > 0){
		printf(" Setting dynamic range: %d\n",dr);
		eiger_dynamicrange = dr;
	}
	return eiger_dynamicrange;
#else
	if(dr > 0){
		printf(" Setting dynamic range: %d\n",dr);
		if(Feb_Control_SetDynamicRange(dr)){

			//EigerSetBitMode(dr);
			on_dst = 0;
			int i;
			for(i=0;i<32;i++) dst_requested[i] = 0; //clear dst requested
			if(Beb_SetUpTransferParameters(dr))
				eiger_dynamicrange = dr;
			else printf("ERROR:Could not set bit mode in the back end\n");
		}
	}
	//make sure back end and front end have the same bit mode
	dr= Feb_Control_GetDynamicRange();

	return dr;
#endif
}




/* parameters - readout */

int setSpeed(enum speedVariable arg, int val){

	if (arg != CLOCK_DIVIDER)
		return -1;

	if(val != -1){
		printf(" Setting Read out Speed: %d\n",val);
#ifndef VIRTUAL
		if(Feb_Control_SetReadoutSpeed(val))
#endif
			eiger_readoutspeed = val;
	}
	return 	eiger_readoutspeed;
}


enum readOutFlags setReadOutFlags(enum readOutFlags val){

	enum readOutFlags retval = GET_READOUT_FLAGS;
	if(val!=GET_READOUT_FLAGS){


		if(val&0xF0000){
			switch(val){
			case PARALLEL:  	val=E_PARALLEL; 	printf(" Setting Read out Flag: Parallel\n"); 			break;
			case NONPARALLEL:	val=E_NON_PARALLEL; printf(" Setting Read out Flag: Non Parallel\n");		break;
			case SAFE:			val=E_SAFE; 		printf(" Setting Read out Flag: Safe\n"); 				break;

			default:
				cprintf(RED,"Cannot set unknown readout flag. 0x%x\n", val);
				return -1;
			}
			printf(" Setting Read out Flag: %d\n",val);
#ifndef VIRTUAL
			if(Feb_Control_SetReadoutMode(val))
#endif
				eiger_readoutmode = val;
#ifndef VIRTUAL
			else return -1;
#endif

		}

		else if (val&0xF00000) {
			switch(val){
			case SHOW_OVERFLOW:  	val=1; 	printf(" Setting Read out Flag: Overflow in 32 bit mode\n"); 	break;
			case NOOVERFLOW:	val=0; 	printf(" Setting Read out Flag: No overflow in 32 bit mode\n");	break;
			default:
				cprintf(RED,"Cannot set unknown readout flag. 0x%x\n", val);
				return -1;
			}
			printf(" Setting Read out Flag: %d\n",val);
#ifndef VIRTUAL
			if(Beb_Set32bitOverflow(val) != -1)
#endif
				eiger_overflow32 = val;
#ifndef VIRTUAL
			else return -1;
#endif
		}


		else{
			switch(val){
			case STORE_IN_RAM:  	val=1; 		printf(" Setting Read out Flag: Store in Ram\n"); 		break;
			case CONTINOUS_RO:		val=0; 		printf(" Setting Read out Flag: Continuous Readout\n");		break;

			default:
				cprintf(RED,"Cannot set unknown readout flag. 0x%x\n", val);
				return -1;
			}
			printf(" Setting store in ram variable: %d\n",val);
			eiger_storeinmem = val;

		}
	}

	switch(eiger_readoutmode){
	case E_PARALLEL: 		retval=PARALLEL; 		break;
	case E_NON_PARALLEL:	retval=NONPARALLEL; 	break;
	case E_SAFE:			retval=SAFE; 			break;
	}

	switch(eiger_overflow32){
	case 1: 		retval|=SHOW_OVERFLOW; 		break;
	case 0:			retval|=NOOVERFLOW; 	break;
	}


	switch(eiger_storeinmem){
	case 0: 		retval|=CONTINOUS_RO; 	break;
	case 1:			retval|=STORE_IN_RAM; 	break;
	}



	printf("Read out Flag: 0x%x\n",retval);
	return retval;
}








/* parameters - timer */

int64_t setTimer(enum timerIndex ind, int64_t val){
#ifndef VIRTUAL
	int64_t subdeadtime = 0;
#endif
	int64_t subexptime = 0;
	switch(ind){
	case FRAME_NUMBER:
		if(val >= 0){
			printf(" Setting number of frames: %d * %d\n",(unsigned int)val,eiger_ncycles);
#ifndef VIRTUAL
			if(Feb_Control_SetNExposures((unsigned int)val*eiger_ncycles)){
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
		if(val >= 0){
			printf(" Setting exp time: %fs\n",val/(1E9));
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
		if(val >= 0){
			printf(" Setting sub exp time: %lldns\n",(long long int)val);
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
		if(val >= 0){
			printf(" Setting sub period: %lldns = subexptime(%lld) + subdeadtime(%lld)\n",
					(long long int)(val + subexptime),
					(long long int)subexptime,
					(long long int)val);
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
		if(val >= 0){
			printf(" Setting acq period: %fs\n",val/(1E9));
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
		if(val >= 0){
			printf(" Setting number of triggers: %d * %d\n",(unsigned int)val,eiger_nexposures);
#ifndef VIRTUAL
			if(Feb_Control_SetNExposures((unsigned int)val*eiger_nexposures)){
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
		cprintf(RED,"Warning: Timer Index not implemented for this detector: %d\n", ind);
		break;
	}

	return -1;
}


int64_t getTimeLeft(enum timerIndex ind) {
#ifdef VIRTUAL
	return 0;
#else
	switch(ind){
	case MEASURED_PERIOD: return Feb_Control_GetMeasuredPeriod();
	case MEASURED_SUBPERIOD: return Feb_Control_GetSubMeasuredPeriod();
	return 0;
	default:
		cprintf(RED,"This timer left index (%d) not defined for Eiger\n",ind);
		return -1;
	}
#endif
}




/* parameters - channel, chip, module, settings */


int setModule(sls_detector_module myMod, int delay){
	int retval[2];
	int i;

	//#ifdef VERBOSE
	printf("Setting module with settings %d\n",myMod.reg);
	//#endif

	//copy module locally (module number, serial number, gain offset,
	//dacs (pointless), trimbit values(if needed)
	if (detectorModules)
		if (copyModule(detectorModules,&myMod) == FAIL)
			return FAIL;

	// settings
	setSettings( (enum detectorSettings)myMod.reg,-1);

	// iodelay
	if(setIODelay(delay, -1)!= delay){
		cprintf(RED,"could not set iodelay %d\n",delay);
		return FAIL;
	}

	// dacs
	for(i=0;i<myMod.ndac;i++)
		setDAC((enum DACINDEX)i,myMod.dacs[i],myMod.module,0,retval);

	// trimbits
#ifndef VIRTUAL
	if(myMod.nchan==0 && myMod.nchip == 0)
		cprintf(BLUE,"Setting module without trimbits\n");
	else{
		printf("Setting module with trimbits\n");
		//includ gap pixels
		unsigned int tt[263680];
		int iy,ichip,ix,ip=0,ich=0;
		for(iy=0;iy<256;iy++) {
			for (ichip=0; ichip<4; ichip++) {
				for(ix=0;ix<256;ix++) {
					tt[ip++]=myMod.chanregs[ich++];
				}
				if (ichip<3) {
					tt[ip++]=0;
					tt[ip++]=0;
				}
			}
		}

		//set trimbits
		if(!Feb_Control_SetTrimbits(Feb_Control_GetModuleNumber(),tt)){
			cprintf(BG_RED,"Could not set trimbits\n");
			return FAIL;
		}
	}
#endif
	return thisSettings;
}


int getModule(sls_detector_module *myMod){

#ifndef VIRTUAL
	int i;
	int retval[2];

	//dacs
	for(i=0;i<NDAC;i++) {
		setDAC((enum DACINDEX)i,-1,-1,0,retval);
		//cprintf(BLUE,"dac%d:%d\n",i, *((detectorModules->dacs)+i));
	}

	//trimbits
	unsigned int* tt;
	tt = Feb_Control_GetTrimbits();

	//exclude gap pixels
	int iy,ichip,ix,ip=0,ich=0;
	for(iy=0;iy<256;iy++) {
		for (ichip=0; ichip<4; ichip++) {
			for(ix=0;ix<256;ix++) {
				myMod->chanregs[ich++]=tt[ip++];
			}
			if (ichip<3) {
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



enum detectorSettings setSettings(enum detectorSettings sett, int imod){
	if(sett == UNINITIALIZED){
		return thisSettings;
	}if(sett != GET_SETTINGS)
		thisSettings = sett;
	printf(" Settings: %d\n", thisSettings);
	return thisSettings;
}

enum detectorSettings getSettings(){
	return thisSettings;
}






/* parameters - threshold */

int getThresholdEnergy(int imod){
	printf(" Getting Threshold energy\n");
	return eiger_photonenergy;
}


int setThresholdEnergy(int ev, int imod){
	printf(" Setting threshold energy:%d\n",ev);
	if(ev >= 0)
		eiger_photonenergy = ev;
	return  getThresholdEnergy(imod);
}





/* parameters - dac, adc, hv */

void setDAC(enum DACINDEX ind, int val, int imod, int mV, int retval[]){
	printf("Going to set dac %d to %d of imod %d with mv mode %d \n", (int)ind, val, imod, mV);
	if(ind == VTHRESHOLD){
		int ret[5];
		setDAC(VCMP_LL,val,imod,mV,retval);
			ret[0] = retval[mV];
		setDAC(VCMP_LR,val,imod,mV,retval);
			ret[1] = retval[mV];
		setDAC(VCMP_RL,val,imod,mV,retval);
			ret[2] = retval[mV];
		setDAC(VCMP_RR,val,imod,mV,retval);
			ret[3] = retval[mV];
		setDAC(VCP,val,imod,mV,retval);
			ret[4] = retval[mV];


		if((ret[0]== ret[1])&&
				(ret[1]==ret[2])&&
				(ret[2]==ret[3]) && 
		   (ret[3]==ret[4]))
		  cprintf(GREEN,"vthreshold match\n");
		else{
		  retval[0] = -1;retval[1] = -1;
		  cprintf(RED,"vthreshold mismatch 0:%d 1:%d 2:%d 3:%d\n",
			  ret[0],ret[1],ret[2],ret[3]);
		}
		return;
	}
	char iname[10];

	if(((int)ind>=0)&&((int)ind<NDAC))
		strcpy(iname,dac_names[(int)ind]);
	else{
		printf("dac value outside range:%d\n",(int)ind);
		strcpy(iname,dac_names[0]);
	}
#ifdef VERBOSE
	if(val >= 0)
		printf("Setting dac %d: %s to %d ",ind, iname,val);
	else
		printf("Getting dac %d: %s ",ind, iname);
	if(mV)
		printf("in mV\n");
	else
		printf("in dac units\n");
#endif
#ifdef VIRTUAL
	if (mV){
		retval[0] = (int)(((val-0)/(2048-0))*(4096-1) + 0.5);
		retval[1] = val;
	}else
		retval[0] = val;

#else
	if(val >= 0)
		Feb_Control_SetDAC(iname,val,mV);
	int k;
	Feb_Control_GetDAC(iname, &k,0);
	retval[0] = k;
	Feb_Control_GetDAC(iname,&k,1);
	retval[1] = k;
#endif
	(detectorModules)->dacs[ind] = retval[0];

}



int getADC(enum ADCINDEX ind,  int imod){
#ifdef VIRTUAL
	return 0;
#else
	int retval = -1;
	char tempnames[6][20]={"FPGA EXT", "10GE","DCDC", "SODL", "SODR", "FPGA"};
	char cstore[255];

	switch(ind){
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

	printf("Temperature %s: %f°C\n",tempnames[ind],(double)retval/1000.00);

	return retval;
#endif
}


int setHighVoltage(int val){
#ifdef VIRTUAL
	if (master) {
		// set
		if(val!=-1){
			eiger_theo_highvoltage = val;
		}
		return eiger_theo_highvoltage;
	}

	return SLAVE_HIGH_VOLTAGE_READ_VAL;
#else

	if (master) {

		// set
		if(val!=-1){
			eiger_theo_highvoltage = val;
			int ret = Feb_Control_SetHighVoltage(val);
			if(!ret)			//could not set
				return -2;
			else if (ret == -1) //outside range
				return -1;
		}

		// get
		if (!Feb_Control_GetHighVoltage(&eiger_highvoltage)) {
			cprintf(RED,"Warning: Could not read high voltage\n");
			return -3;
		}

		// tolerance of 5
		if (abs(eiger_theo_highvoltage-eiger_highvoltage) > HIGH_VOLTAGE_TOLERANCE) {
			cprintf(BLUE, "High voltage still ramping: %d\n", eiger_highvoltage);
			return eiger_highvoltage;
		}
		return eiger_theo_highvoltage;
	}

	return SLAVE_HIGH_VOLTAGE_READ_VAL;
#endif
}







/* parameters - timing, extsig */

enum externalCommunicationMode setTiming( enum externalCommunicationMode arg){
	enum externalCommunicationMode ret=GET_EXTERNAL_COMMUNICATION_MODE;
	if(arg != GET_EXTERNAL_COMMUNICATION_MODE){
		switch((int)arg){
		case AUTO_TIMING:			ret = 0;	break;
		case TRIGGER_EXPOSURE:		ret = 2;	break;
		case BURST_TRIGGER:			ret = 1;	break;
		case GATE_FIX_NUMBER:		ret = 3;	break;
		}
		printf(" Setting Triggering Mode: %d\n",(int)ret);
#ifndef VIRTUAL
		if(Feb_Control_SetTriggerMode(ret,1))
#endif
			eiger_triggermode = ret;
	}

	ret = eiger_triggermode;
	switch((int)ret){
	case 0:		ret = AUTO_TIMING;		break;
	case 2:		ret = TRIGGER_EXPOSURE; break;
	case 1:		ret = BURST_TRIGGER;	break;
	case 3:		ret = GATE_FIX_NUMBER;	break;
	default:
		printf("Unknown trigger mode found %d\n",ret);
		ret = 0;
	}
	return ret;
}






/* configure mac */

int configureMAC(uint32_t destip, uint64_t destmac, uint64_t sourcemac, uint32_t sourceip, uint32_t udpport, uint32_t udpport2, int ival) {
#ifndef VIRTUAL
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

	printf("src_port:%d\n",src_port);
	printf("src_ip:%s\n",src_ip);
	printf("dst_ip:%s\n",dst_ip);
	printf("src_mac:%s\n",src_mac);
	printf("dst_mac:%s\n",dst_mac);


	int beb_num = BEB_NUM;//Feb_Control_GetModuleNumber();
	int header_number = 0;
	int dst_port = udpport;
	if(!top)
		dst_port = udpport2;

	printf("dst_port:%d\n\n",dst_port);

	int i=0;
	/* for(i=0;i<32;i++){ modified for Aldo*/
	if(Beb_SetBebSrcHeaderInfos(beb_num,send_to_ten_gig,src_mac,src_ip,src_port) &&
			Beb_SetUpUDPHeader(beb_num,send_to_ten_gig,header_number+i,dst_mac,dst_ip, dst_port))
		printf("set up left ok\n");
	else return -1;
	/*}*/

	header_number = 32;
	dst_port = udpport2;
	if(!top)
		dst_port = udpport;
	printf("dst_port:%d\n\n",dst_port);

	/*for(i=0;i<32;i++){*//** modified for Aldo*/
	if(Beb_SetBebSrcHeaderInfos(beb_num,send_to_ten_gig,src_mac,src_ip,src_port) &&
			Beb_SetUpUDPHeader(beb_num,send_to_ten_gig,header_number+i,dst_mac,dst_ip, dst_port))
		printf("set up right ok\n\n");
	else return -1;
	/*}*/

	on_dst = 0;

	for(i=0;i<32;i++) dst_requested[i] = 0; //clear dst requested
	nimages_per_request=eiger_nexposures * eiger_ncycles;
#endif
	return 0;
}



int	setDetectorPosition(int pos[]) {
#ifdef VIRTUAL
	return OK;
#else
	return Beb_SetDetectorPosition(pos);
#endif
}





/* eiger specific - iodelay, 10g, pulse, rate, temp, activate, delay nw parameter */

int setIODelay(int val, int imod){
	if(val!=-1){
		printf(" Setting IO Delay: %d\n",val);
#ifndef VIRTUAL
		if(Feb_Control_SetIDelays(Feb_Control_GetModuleNumber(),val))
#endif
			eiger_iodelay = val;
	}
	return eiger_iodelay;
}


int enableTenGigabitEthernet(int val){
	if(val!=-1){
		if(val>0)
			send_to_ten_gig = 1;
		else
			send_to_ten_gig = 0;
		//configuremac called from client
	}
#ifdef VERBOSE
	printf("10Gbe:%d\n",send_to_ten_gig);
#endif
	return send_to_ten_gig;
}


int setCounterBit(int val){
	if(val!=-1){
#ifdef VIRTUAL
		eiger_virtual_counter_bit = val;
#else
		Feb_Control_Set_Counter_Bit(val);
#endif
#ifdef VERBOSE
	printf("Counter Bit:%d\n",val);
#endif
	}
#ifdef VIRTUAL
	return eiger_virtual_counter_bit;
#else
	return Feb_Control_Get_Counter_Bit();
#endif
}


int pulsePixel(int n, int x, int y){
#ifndef VIRTUAL
	if(!Feb_Control_Pulse_Pixel(n,x,y))
		return FAIL;
#endif
	return OK;
}

int pulsePixelNMove(int n, int x, int y){
#ifndef VIRTUAL
	if(!Feb_Control_PulsePixelNMove(n,x,y))
		return FAIL;
#endif
	return OK;
}

int pulseChip(int n){
#ifndef VIRTUAL
	if(!Feb_Control_PulseChip(n))
		return FAIL;
#endif
	return OK;
}

int64_t setRateCorrection(int64_t custom_tau_in_nsec){//in nanosec (will never be -1)
#ifdef VIRTUAL
	//deactivating rate correction
	if(custom_tau_in_nsec==0){
		eiger_virtual_ratecorrection_variable = 0;
		return 0;
	}

	//when dynamic range changes, use old tau
	else if(custom_tau_in_nsec == -1)
		custom_tau_in_nsec = eiger_virtual_ratetable_tau_in_ns;

	//get period = subexptime if 32bit , else period = exptime if 16 bit
	int64_t actual_period = eiger_virtual_subexptime*10; //already in nsec
	if(eiger_dynamicrange == 16)
		actual_period = eiger_virtual_exptime;

	int64_t ratetable_period_in_nsec = eiger_virtual_ratetable_period_in_ns;
	int64_t tau_in_nsec = eiger_virtual_ratetable_tau_in_ns;



	//same setting
	if((tau_in_nsec == custom_tau_in_nsec) && (ratetable_period_in_nsec == actual_period)){
		if(eiger_dynamicrange == 32)
			printf("Rate Table already created before: Same Tau %lldns, Same subexptime %lldns\n",
					(long long int)tau_in_nsec,(long long int)ratetable_period_in_nsec);
		else
			printf("Rate Table already created before: Same Tau %lldns, Same exptime %lldns\n",
					(long long int)tau_in_nsec,(long long int)ratetable_period_in_nsec);
	}
	//different setting, calculate table
	else{
		eiger_virtual_ratetable_tau_in_ns = custom_tau_in_nsec;
		double period_in_sec = (double)(eiger_virtual_subexptime*10)/(double)1e9;
		if(eiger_dynamicrange == 16)
			period_in_sec = eiger_virtual_exptime;
		eiger_virtual_ratetable_period_in_ns = period_in_sec*1e9;
	}
	//activating rate correction
	eiger_virtual_ratecorrection_variable = 1;
	printf("Rate Correction Value set to %lld ns\n",(long long int)eiger_virtual_ratetable_tau_in_ns);

	return eiger_virtual_ratetable_tau_in_ns;
#else

	//deactivating rate correction
	if(custom_tau_in_nsec==0){
		Feb_Control_SetRateCorrectionVariable(0);
		return 0;
	}

	//when dynamic range changes, use old tau
	else if(custom_tau_in_nsec == -1)
		custom_tau_in_nsec = Feb_Control_Get_RateTable_Tau_in_nsec();


	int dr = Feb_Control_GetDynamicRange();
	//get period = subexptime if 32bit , else period = exptime if 16 bit
	int64_t actual_period = Feb_Control_GetSubFrameExposureTime(); //already in nsec
	if(dr == 16)
		actual_period = Feb_Control_GetExposureTime_in_nsec();

	int64_t ratetable_period_in_nsec = Feb_Control_Get_RateTable_Period_in_nsec();
	int64_t tau_in_nsec = Feb_Control_Get_RateTable_Tau_in_nsec();


	//same setting
	if((tau_in_nsec == custom_tau_in_nsec) && (ratetable_period_in_nsec == actual_period)){
		if(dr == 32)
			printf("Rate Table already created before: Same Tau %lldns, Same subexptime %lldns\n",
					tau_in_nsec,ratetable_period_in_nsec);
		else
			printf("Rate Table already created before: Same Tau %lldns, Same exptime %lldns\n",
					tau_in_nsec,ratetable_period_in_nsec);
	}
	//different setting, calculate table
	else{
		int ret = Feb_Control_SetRateCorrectionTau(custom_tau_in_nsec);
		if(ret<=0){
			cprintf(RED,"Rate correction failed. Deactivating rate correction\n");
			Feb_Control_SetRateCorrectionVariable(0);
			return ret;
		}
	}
	//activating rate correction
	Feb_Control_SetRateCorrectionVariable(1);
	printf("Rate Correction Value set to %lld ns\n",(long long int)Feb_Control_Get_RateTable_Tau_in_nsec());
#ifdef VERBOSE
	Feb_Control_PrintCorrectedValues();
#endif

	return Feb_Control_Get_RateTable_Tau_in_nsec();
#endif
}

int getRateCorrectionEnable(){
#ifdef VIRTUAL
	return eiger_virtual_ratecorrection_variable;
#else
	return Feb_Control_GetRateCorrectionVariable();
#endif
}

int getDefaultSettingsTau_in_nsec(){
	return default_tau_from_file;
}

void setDefaultSettingsTau_in_nsec(int t){
	default_tau_from_file = t;
	printf("Default tau set to %d\n",default_tau_from_file);
}

int64_t getCurrentTau(){
	if(!getRateCorrectionEnable())
		return 0;
	else
#ifndef VIRTUAL
		return Feb_Control_Get_RateTable_Tau_in_nsec();
#else
		return eiger_virtual_ratetable_tau_in_ns;
#endif
}

void setExternalGating(int enable[]){
	if(enable>=0){
#ifndef VIRTUAL
		Feb_Control_SetExternalEnableMode(enable[0], enable[1]);//enable = 0 or 1, polarity = 0 or 1 , where 1 is positive
#endif
		eiger_extgating = enable[0];
		eiger_extgatingpolarity = enable[1];
	}
	enable[0] = eiger_extgating;
	enable[1] = eiger_extgatingpolarity;
}

int setAllTrimbits(int val){
#ifndef VIRTUAL
	if(!Feb_Control_SaveAllTrimbitsTo(val)){
		cprintf(RED,"error in setting all trimbits to value\n");
		return FAIL;
	}
#endif
#ifdef VERBOSE
	printf("Copying register %x value %d\n",destMod->reg,val);
#endif
	if (detectorModules){
		int ichan;
		for (ichan=0; ichan<(detectorModules->nchan); ichan++) {
			*((detectorModules->chanregs)+ichan)=val;
		}
	}

	cprintf(GREEN, "All trimbits have been set to %d\n", val);
	return OK;
}

int getAllTrimbits(){
	int ichan=0;
	int value = *((detectorModules->chanregs));
	if (detectorModules){
		for (ichan=0; ichan<(detectorModules->nchan); ichan++) {
			if(*((detectorModules->chanregs)+ichan) != value) {
				value= -1;
				break;
			}

		}
	}
	printf("Value of all Trimbits: %d\n", value);
	return value;
}

int getBebFPGATemp(){
#ifdef VIRTUAL
	return 0;
#else
	return Beb_GetBebFPGATemp();
#endif
}

int activate(int enable){
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

int setNetworkParameter(enum NETWORKINDEX mode, int value){
#ifndef VIRTUAL
	return Beb_SetNetworkParameter(mode, value);
#else
	if (value>-1) {
		switch(mode){
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
			if(value>0) value = 1;
			break;
		default: cprintf(BG_RED,"Unrecognized mode in network parameter: %d\n",mode);
			return -1;
		}
	}
	switch(mode){
	case TXN_LEFT:
		return eiger_virtual_transmission_delay_left;
	case TXN_RIGHT:
		return eiger_virtual_transmission_delay_right;
	case TXN_FRAME:
		return eiger_virtual_transmission_delay_frame;
	case FLOWCTRL_10G:
		return eiger_virtual_transmission_flowcontrol_10g;
	default: cprintf(BG_RED,"Unrecognized mode in network parameter: %d\n",mode);
		return -1;
	}
#endif
}







/* aquisition */


int prepareAcquisition(){
#ifndef VIRTUAL
	printf("Going to prepare for acquisition with counter_bit:%d\n",Feb_Control_Get_Counter_Bit());
	Feb_Control_PrepareForAcquisition();
	printf("Going to reset Frame Number\n");
	Beb_ResetFrameNumber();
#endif
	return OK;

}


int startStateMachine(){
#ifdef VIRTUAL
	eiger_virtual_status = 1;
	eiger_virtual_stop = 0;
	if(pthread_create(&eiger_virtual_tid, NULL, &start_timer, NULL)) {
		cprintf(RED,"Could not start Virtual acquisition thread\n");
		eiger_virtual_status = 0;
		return FAIL;
	}
	cprintf(GREEN,"***Virtual Acquisition started\n");
	return OK;
#else

	int ret = OK,prev_flag;
	//get the DAQ toggle bit
	prev_flag = Feb_Control_AcquisitionStartedBit();

	printf("Going to start acquisition\n");
	Feb_Control_StartAcquisition();

	if(!eiger_storeinmem){
		printf("requesting images right after start\n");
		ret =  startReadOut();
	}

	//wait for acquisition start
	if(ret == OK){
		if(!Feb_Control_WaitForStartedFlag(5000, prev_flag)){
			cprintf(RED,"Error: Acquisition did not start or trouble reading register\n");
			return FAIL;
		}
		cprintf(GREEN,"***Acquisition started\n");
	}

	/*while(getRunStatus() == IDLE){printf("waiting for being not idle anymore\n");}*/

	return ret;
#endif
}

#ifdef VIRTUAL
void* start_timer(void* arg) {
	eiger_virtual_status = 1;
	int wait_in_s = nimages_per_request * eiger_virtual_period;
	cprintf(GREEN,"going to wait for %d s\n", wait_in_s);
	while(!eiger_virtual_stop && (wait_in_s >= 0)) {
		usleep(1000 * 1000);
		wait_in_s--;
	}
	cprintf(GREEN,"Virtual Timer Done***\n");

	eiger_virtual_status = 0;
	return NULL;
}
#endif



int stopStateMachine(){
	cprintf(BG_RED,"Going to stop acquisition\n");
#ifdef VIRTUAL
	eiger_virtual_stop = 0;
	return OK;
#else

	if((Feb_Control_StopAcquisition() == STATUS_IDLE) & Beb_StopAcquisition())
		return OK;
	cprintf(BG_RED,"failed to stop acquisition\n");
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


int startReadOut(){

	printf("Requesting images...\n");
#ifdef VIRTUAL
	return OK;
#else
	//RequestImages();
	int ret_val = 0;
	dst_requested[0] = 1;
	while(dst_requested[on_dst]){
		//waits on data
		int beb_num = BEB_NUM;//Feb_Control_GetModuleNumber();
		if  ((ret_val = (!Beb_RequestNImages(beb_num,send_to_ten_gig,on_dst,nimages_per_request,0))))
			break;
//		for(i=0;i<nimages_per_request;i++)
//			if  ((ret_val = (!Beb_RequestNImages(beb_num,send_to_ten_gig,on_dst,1,0))))
//				break;

		dst_requested[on_dst++]=0;
		on_dst%=ndsts_in_use;
	}

	if(ret_val)
		return FAIL;
	else
		return OK;
#endif
}


enum runStatus getRunStatus(){
#ifdef VIRTUAL
	if(eiger_virtual_status == 0){
		printf("Status: IDLE\n");
		return IDLE;
	}else{
		printf("Status: RUNNING...\n");
		return RUNNING;
	}
#else

	int i = Feb_Control_AcquisitionInProgress();
	switch (i) {
	case STATUS_ERROR:
		printf("Status: ERROR reading status register\n");
		return ERROR;
	case STATUS_IDLE:
		printf("Status: IDLE\n");
		return IDLE;
	default:
		printf("Status: RUNNING...\n");
		return RUNNING;
	}

	return IDLE;
#endif
}



void readFrame(int *ret, char *mess){
#ifdef VIRTUAL
	while(eiger_virtual_status) {
		//cprintf(RED,"Waiting for finished flag\n");
		usleep(5000);
	}
	*ret = (int)FINISHED;
	strcpy(mess,"acquisition successfully finished\n");
	return;
#else

	if(Feb_Control_WaitForFinishedFlag(5000) == STATUS_ERROR) {
		cprintf(RED,"Error: Waiting for finished flag\n");
		*ret = FAIL;
		return;
	}
	cprintf(GREEN,"Acquisition finished***\n");

	if(eiger_storeinmem){
		printf("requesting images after storing in memory\n");
		if(startReadOut() == FAIL){
			strcpy(mess,"Could not execute read image requests\n");
			*ret = (int)FAIL;
			return;
		}
	}

	//wait for detector to send
	Beb_EndofDataSend(send_to_ten_gig);


	printf("*****Done Waiting...\n");
	*ret = (int)FINISHED;
	strcpy(mess,"acquisition successfully finished\n");
#endif
}








/* common */


int copyModule(sls_detector_module *destMod, sls_detector_module *srcMod){

	int ichip, idac,  ichan, iadc;
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
	//no trimbit feature
	if (destMod->nchip && ((srcMod->nchip)>(destMod->nchip))) {
		printf("Number of chip of source is larger than number of chips of destination\n");
		return FAIL;
	}
	//no trimbit feature
	if (destMod->nchan && ((srcMod->nchan)>(destMod->nchan))) {
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

	if((destMod->nchip!=0) || (destMod->nchan!=0)) {
		for (ichip=0; ichip<(srcMod->nchip); ichip++) {
			if (*((srcMod->chipregs)+ichip)>=0)
				*((destMod->chipregs)+ichip)=*((srcMod->chipregs)+ichip);
		}
		for (ichan=0; ichan<(srcMod->nchan); ichan++) {
			if (*((srcMod->chanregs)+ichan)>=0)
				*((destMod->chanregs)+ichan)=*((srcMod->chanregs)+ichan);
		}
	}
#ifdef VERBOSE
	else printf("Not Copying trimbits\n");
#endif
	for (idac=0; idac<(srcMod->ndac); idac++) {
		if (*((srcMod->dacs)+idac)>=0) {
			*((destMod->dacs)+idac)=*((srcMod->dacs)+idac);
		}
	}
	for (iadc=0; iadc<(srcMod->nadc); iadc++) {
		if (*((srcMod->adcs)+iadc)>=0)
			*((destMod->adcs)+iadc)=*((srcMod->adcs)+iadc);
	}
	return ret;
}


int calculateDataBytes(){
	if(send_to_ten_gig)
		return setDynamicRange(-1) * ONE_GIGA_CONSTANT * TEN_GIGA_BUFFER_SIZE;
	else
		return setDynamicRange(-1) * TEN_GIGA_CONSTANT * ONE_GIGA_BUFFER_SIZE;
}


int getTotalNumberOfChannels(){return ((int)getNumberOfChannelsPerModule() * (int)getTotalNumberOfModules());}
int getTotalNumberOfChips(){return ((int)getNumberOfChipsPerModule() * (int)getTotalNumberOfModules());}
int getTotalNumberOfModules(){return NMOD;}
int getNumberOfChannelsPerModule(){return  ((int)getNumberOfChannelsPerChip() * (int)getTotalNumberOfChips());}
int getNumberOfChipsPerModule(){return  NCHIP;}
int getNumberOfDACsPerModule(){return  NDAC;}
int getNumberOfADCsPerModule(){return  NADC;}
int getNumberOfChannelsPerChip(){return  NCHAN;}
int getNumberOfGainsPerModule(){return  NGAIN;}
int getNumberOfOffsetsPerModule(){return  NOFFSET;}






/* sync */

enum masterFlags setMaster(enum masterFlags arg){
	return NO_MASTER;
}



enum synchronizationMode setSynchronization(enum synchronizationMode arg){
	return NO_SYNCHRONIZATION;
}







//#endif
