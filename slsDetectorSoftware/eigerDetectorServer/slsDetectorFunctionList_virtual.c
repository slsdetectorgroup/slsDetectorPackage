#ifdef SLS_DETECTOR_FUNCTION_LIST


#include <stdio.h>
#include <string.h>
#include <unistd.h>	//usleep
#include <pthread.h>
#include <time.h>

#include "slsDetectorFunctionList.h"


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
int eiger_readoutspeed = 0;
int eiger_triggermode = 0;
int eiger_extgating = 0;
int eiger_extgatingpolarity = 0;


int eiger_nexposures = 1;
int eiger_ncycles = 1;

//values for virtual server
double eiger_virtual_exptime = 0;
int64_t eiger_virtual_subexptime = 0;
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
pthread_t eiger_virtual_tid;

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





/* basic tests */

void checkFirmwareCompatibility(int flag){
	cprintf(BLUE,"\n\n"
			"********************************************************\n"
			"***************** EIGER Virtual Server *****************\n"
			"********************************************************\n");
}





/* Ids */

int64_t getDetectorId(enum idMode arg){
	return 0;
}

u_int64_t getFirmwareVersion() {
	return 0;
}



u_int32_t getDetectorNumber(){
	return 0;
}


u_int64_t  getDetectorMAC() {
	return 0;
}


u_int32_t  getDetectorIP(){
	return 0;
}





/* initialization */

void initControlServer(){
	getModuleConfiguration();
	setupDetector();
	printf("\n");
}

void initStopServer(){
	getModuleConfiguration();
	printf("\n");
}


void getModuleConfiguration(){
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
	setTimer(SUBFRAME_PERIOD, DEFAULT_SUBFRAME_PERIOD);
	setTimer(FRAME_PERIOD, DEFAULT_PERIOD);
	setDynamicRange(DEFAULT_DYNAMIC_RANGE);
	eiger_photonenergy = DEFAULT_PHOTON_ENERGY;
	setReadOutFlags(DEFAULT_READOUT_FLAG);
	setSpeed(CLOCK_DIVIDER, DEFAULT_CLK_SPEED);//clk_devider,half speed
	setIODelay(DEFAULT_IO_DELAY, DEFAULT_MOD_INDEX);
	setTiming(DEFAULT_TIMING_MODE);
	//SetPhotonEnergyCalibrationParameters(-5.8381e-5,1.838515,5.09948e-7,-4.32390e-11,1.32527e-15);
	setRateCorrection(DEFAULT_RATE_CORRECTION);
	int enable[2] = {DEFAULT_EXT_GATING_ENABLE, DEFAULT_EXT_GATING_POLARITY};
	setExternalGating(enable);//disable external gating
	setHighVoltage(DEFAULT_HIGH_VOLTAGE);
}




/* advanced read/write reg */
uint32_t writeRegister(uint32_t offset, uint32_t data) {
	return 0;
}

uint32_t readRegister(uint32_t offset) {
	return 0;
}


/* set parameters - nmod, dr, roi */

int setNMod(int nm, enum dimension dim){
	return NMOD;
}


int getNModBoard(enum dimension arg){
	return NMAXMOD;
}

int setDynamicRange(int dr){
	if(dr > 0){
		printf(" Setting dynamic range: %d\n",dr);
		eiger_dynamicrange = dr;
	}
	return eiger_dynamicrange;
}




/* parameters - readout */

int setSpeed(enum speedVariable arg, int val){

	if (arg != CLOCK_DIVIDER)
		return -1;

	if(val != -1){
		printf(" Setting Read out Speed: %d\n",val);
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
			eiger_readoutmode = val;
		}else{
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

	switch(eiger_storeinmem){
	case 0: 		retval|=CONTINOUS_RO; 	break;
	case 1:			retval|=STORE_IN_RAM; 	break;
	}
	printf("Read out Flag: 0x%x\n",retval);
	return retval;
}








/* parameters - timer */

int64_t setTimer(enum timerIndex ind, int64_t val){

	switch(ind){
	case FRAME_NUMBER:
		if(val >= 0){
			printf(" Setting number of frames: %d * %d\n",(unsigned int)val,eiger_ncycles);
			eiger_nexposures = val;
			nimages_per_request = eiger_nexposures * eiger_ncycles;
		}return eiger_nexposures;

	case ACQUISITION_TIME:
		if(val >= 0){
			printf(" Setting exp time: %fs\n",val/(1E9));
			eiger_virtual_exptime = (val/(1E9));
		}
		return eiger_virtual_exptime*1e9;

	case SUBFRAME_ACQUISITION_TIME:
		if(val >= 0){
			printf(" Setting sub exp time: %lldns\n",(long long int)val/10);
			eiger_virtual_subexptime = (val/(10));
		}
		return eiger_virtual_subexptime*10;

	case FRAME_PERIOD:
		if(val >= 0){
			printf(" Setting acq period: %fs\n",val/(1E9));
			eiger_virtual_period = (val/(1E9));
		}
		return eiger_virtual_period*1e9;

	case CYCLES_NUMBER:
		if(val >= 0){
			printf(" Setting number of triggers: %d * %d\n",(unsigned int)val,eiger_nexposures);
			eiger_ncycles = (val/(1E9));
			nimages_per_request = eiger_nexposures * eiger_ncycles;
		}
		return eiger_ncycles;
	default:
		cprintf(RED,"Warning: Timer Index not implemented for this detector: %d\n", ind);
		break;
	}

	return -1;
}





/* parameters - channel, chip, module, settings */


int setModule(sls_detector_module myMod, int delay){
	int retval[2];
	int i;

	//#ifdef VERBOSE
	printf("Setting module with settings %d\n",myMod.reg);
	//#endif

	setSettings( (enum detectorSettings)myMod.reg,-1);

	if(setIODelay(delay, -1)!= delay){
		cprintf(RED,"could not set iodelay %d\n",delay);
		return FAIL;
	}

	//copy module locally
	if (detectorModules)
		copyModule(detectorModules,&myMod);

	//set dac values
	for(i=0;i<myMod.ndac;i++)
		setDAC((enum DACINDEX)i,myMod.dacs[i],myMod.module,0,retval);

	return thisSettings;
}


int getModule(sls_detector_module *myMod){
	//copy from local copy
	if (detectorModules) {
		copyModule(myMod, detectorModules);
		return OK;
	}
	return FAIL;
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

	if (mV){
		retval[0] = (int)(((val-0)/(2048-0))*(4096-1) + 0.5);
		retval[1] = val;
	}else
		retval[0] = val;

	(detectorModules)->dacs[ind] = retval[0];
}



int getADC(enum ADCINDEX ind,  int imod){
	return 0;
}


int setHighVoltage(int val){
	if (master) {
		// set
		if(val!=-1){
			eiger_theo_highvoltage = val;
		}
		return eiger_theo_highvoltage;
	}

	return SLAVE_HIGH_VOLTAGE_READ_VAL;
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
	return 0;
}



int	setDetectorPosition(int pos[]) {
	return OK;
}



/* eiger specific - iodelay, 10g, pulse, rate, temp, activate, delay nw parameter */

int setIODelay(int val, int imod){
	if(val!=-1){
		printf(" Setting IO Delay: %d\n",val);
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
		eiger_virtual_counter_bit = val;
#ifdef VERBOSE
	printf("Counter Bit:%d\n",val);
#endif
	}
	return eiger_virtual_counter_bit;
}


int pulsePixel(int n, int x, int y){
	return OK;
}

int pulsePixelNMove(int n, int x, int y){
	return OK;
}

int pulseChip(int n){
	return OK;
}

int64_t setRateCorrection(int64_t custom_tau_in_nsec){//in nanosec (will never be -1)
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
}

int getRateCorrectionEnable(){
	return eiger_virtual_ratecorrection_variable;
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
		return eiger_virtual_ratetable_tau_in_ns;
}

void setExternalGating(int enable[]){
	if(enable>=0){
		eiger_extgating = enable[0];
		eiger_extgatingpolarity = enable[1];
	}
	enable[0] = eiger_extgating;
	enable[1] = eiger_extgatingpolarity;
}

int setAllTrimbits(int val){
	int ichan;
#ifdef VERBOSE
		printf("Copying register %x value %d\n",destMod->reg,val);
#endif
		if (detectorModules){
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
	return 0;
}

int activate(int enable){
	return enable;
}

int setNetworkParameter(enum NETWORKINDEX mode, int value){
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
}







/* aquisition */


int prepareAcquisition(){
	return OK;
}


int startStateMachine(){

	if(pthread_create(&eiger_virtual_tid, NULL, &start_timer, NULL)) {
		cprintf(RED,"Could not start Virtual acquisition thread\n");
		return FAIL;
	}
	eiger_virtual_status = 1;
	cprintf(GREEN,"***Virtual Acquisition started\n");
	return OK;
}

void* start_timer(void* arg) {
	double wait_in_s = nimages_per_request * eiger_virtual_period;
	cprintf(GREEN,"going to wait for %f s\n", wait_in_s);
	usleep(wait_in_s * 1000 * 1000);
	cprintf(GREEN,"Virtual Timer Done***\n");

	eiger_virtual_status = 0;
	return NULL;
}

int stopStateMachine(){
	cprintf(BG_RED,"Going to stop acquisition\n");
		return OK;
}


int startReadOut(){
	printf("Requesting images...\n");
	return OK;
}


enum runStatus getRunStatus(){
	if(eiger_virtual_status== 0){
		printf("Status: IDLE\n");
		return IDLE;
	}else{
		printf("Status: RUNNING...\n");
		return RUNNING;
	}
	//}else printf("***** not master*** \n");

	return IDLE;
}



void readFrame(int *ret, char *mess){
	while(eiger_virtual_status) {
		//cprintf(RED,"Waiting for finished flag\n");
		usleep(5000);
	}
	*ret = (int)FINISHED;
	strcpy(mess,"acquisition successfully finished\n");
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







#endif
