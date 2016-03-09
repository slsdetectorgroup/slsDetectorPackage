#ifdef SLS_DETECTOR_FUNCTION_LIST


#include <stdio.h>
#include <unistd.h> //to gethostname
#include <string.h>



#include "slsDetectorFunctionList.h"
#include "gitInfoEiger.h"
#include "FebControl.h"
#include "Beb.h"

#define BEB_NUM 34

enum detectorSettings thisSettings;
const char* dac_names[16] = {"SvP","Vtr","Vrf","Vrs","SvN","Vtgstv","Vcmp_ll","Vcmp_lr","cal","Vcmp_rl","rxb_rb","rxb_lb","Vcmp_rr","Vcp","Vcn","Vis"};

enum{E_PARALLEL, E_NON_PARALLEL, E_SAFE};

//static const string dacNames[16] = {"Svp","Svn","Vtr","Vrf","Vrs","Vtgstv","Vcmp_ll","Vcmp_lr","Cal","Vcmp_rl","Vcmp_rr","Rxb_rb","Rxb_lb","Vcp","Vcn","Vis"};

sls_detector_module *detectorModules=NULL;
int *detectorChips=NULL;
int *detectorChans=NULL;
dacs_t *detectorDacs=NULL;
dacs_t *detectorAdcs=NULL;
int* detectorGain = NULL;
int* detectorOffset = NULL;

int eiger_highvoltage = 0;
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


int send_to_ten_gig = 0;
int  ndsts_in_use=32;
unsigned int nimages_per_request=1;
int  on_dst=0;
int dst_requested[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

int default_dac_values[16] = {
		0, 		//SvP
		2480, 	//Vtr
		3300, 	//Vrf
		1400, 	//Vrs
		4000, 	//SvN
		2556, 	//Vtgstv
		1000, 	//Vcmp_ll
		1000, 	//Vcmp_lr
		4000, 	//cal
		1000, 	//Vcmp_rl
		1100, 	//rxb_rb
		1100, 	//rxb_lb
		1000, 	//Vcmp_rr
		200, 	//Vcp
		2000, 	//Vcn
		1550 	//Vis
};
int default_gain_values[3] = {517000,517000,517000};
int default_offset_values[3] = {3851000,3851000,3851000};


enum masterFlags  masterMode=IS_SLAVE;
int top = 0;
int master = 0;


#define TEN_GIGA_BUFFER_SIZE 4112
#define ONE_GIGA_BUFFER_SIZE 1040


int initDetector(){
	int imod,i,n;
	n = getNModBoard(1);

	//#ifdef VERBOSE
	printf("This Server is for 1 Eiger half module\n");
	//#endif


	//Allocation of memory
	detectorModules=malloc(n*sizeof(sls_detector_module));
	detectorChips=malloc(n*NCHIP*sizeof(int));

	detectorChans=malloc(n*NCHIP*NCHAN*sizeof(int));
	detectorDacs=malloc(n*NDAC*sizeof(dacs_t));
	detectorAdcs=malloc(n*NADC*sizeof(dacs_t));
	detectorGain=malloc(n*NGAIN*sizeof(int));
	detectorOffset=malloc(n*NOFFSET*sizeof(int));
#ifdef VERBOSE
	printf("modules from 0x%x to 0x%x\n",detectorModules, detectorModules+n);
	printf("chips from 0x%x to 0x%x\n",detectorChips, detectorChips+n*NCHIP);
	printf("chans from 0x%x to 0x%x\n",detectorChans, detectorChans+n*NCHIP*NCHAN);
	printf("dacs from 0x%x to 0x%x\n",detectorDacs, detectorDacs+n*NDAC);
	printf("adcs from 0x%x to 0x%x\n",detectorAdcs, detectorAdcs+n*NADC);
	printf("gains from 0x%x to 0x%x\n",detectorGain, detectorGain+n*NGAIN);
	printf("offsets from 0x%x to 0x%x\n",detectorOffset, detectorOffset+n*NOFFSET);
#endif
	for (imod=0; imod<n; imod++) {
		(detectorModules+imod)->dacs=detectorDacs+imod*NDAC;
		(detectorModules+imod)->adcs=detectorAdcs+imod*NADC;
		(detectorModules+imod)->chipregs=detectorChips+imod*NCHIP;
		(detectorModules+imod)->chanregs=detectorChans+imod*NCHIP*NCHAN;
		(detectorModules+imod)->ndac=NDAC;
		(detectorModules+imod)->nadc=NADC;
		(detectorModules+imod)->nchip=NCHIP;
		(detectorModules+imod)->nchan=NCHIP*NCHAN;
		(detectorModules+imod)->module=imod;
		(detectorModules+imod)->gain=0;
		(detectorModules+imod)->offset=0;
		(detectorModules+imod)->reg=0;
		/* initialize registers, dacs, retrieve sn, adc values etc */
	}
	for(i=0;i<NGAIN;i++)
		detectorGain[i] = default_gain_values[(int)STANDARD];
	for(i=0;i<NOFFSET;i++)
		detectorOffset[i] = default_offset_values[(int)STANDARD];
	thisSettings = STANDARD;/**UNITIALIZED*/
	/*sChan=noneSelected;
  sChip=noneSelected;
  sMod=noneSelected;
  sDac=noneSelected;
  sAdc=noneSelected;
	 */

	//Feb and Beb Initializations
	getModuleConfiguration();
	Feb_Interface_FebInterface();
	Feb_Control_FebControl();
	Feb_Control_Init(master,top,getDetectorNumber());
	printf("FEB Initialization done\n");
	Beb_Beb();
	printf("BEB Initialization done\n");

	//Get dac values
	int retval[2];
	for(i=0;i<(detectorModules)->ndac;i++)
		setDAC((enum detDacIndex)i,default_dac_values[i],(detectorModules)->module,0,retval);

	//setting default measurement parameters
	setTimer(FRAME_NUMBER,1);
	setTimer(ACQUISITION_TIME,1E9);
	setTimer(SUBFRAME_ACQUISITION_TIME,DEFAULT_SUBFRAME_EXPOSURE_VAL);
	setTimer(FRAME_PERIOD,1E9);
	setDynamicRange(16);
	eiger_photonenergy = -1;
	setReadOutFlags(NONPARALLEL);
	setSpeed(0,1);//clk_devider,half speed
	setHighVoltage(0,0);
	setIODelay(650,0);
	setTiming(AUTO_TIMING);
	//SetPhotonEnergyCalibrationParameters(-5.8381e-5,1.838515,5.09948e-7,-4.32390e-11,1.32527e-15);
	setRateCorrection(0); //deactivate rate correction
	int enable[2] = {0,1};
	setExternalGating(enable);//disable external gating
	Feb_Control_SetInTestModeVariable(0);
	Feb_Control_CheckSetup();

	//print detector mac and ip
	printf("mac read from detector: %llx\n",getDetectorMAC());
	printf("ip read from detector: %x\n",getDetectorIP());

	printf("\n");
	return 1;
}

int initDetectorStop(){
	getModuleConfiguration();
	Feb_Interface_FebInterface();
	Feb_Control_FebControl();
	Feb_Control_Init(master,top,getDetectorNumber());
	printf("FEB Initialization done\n");
	/* Beb_Beb(-1);
    printf("BEB constructor done\n");*/

	printf("\n");
	return 1;
}



void getModuleConfiguration(){
	int *m=&master;
	int *t=&top;
	/*if(getDetectorNumber() == 0xbeb015){
		master = 1;
		top = 1;
	}*/
	Beb_GetModuleCopnfiguration(m,t);
	if(top)	printf("*************** TOP ***************\n");
	else	printf("*************** BOTTOM ***************\n");
	if(master)	printf("*************** MASTER ***************\n");
	else		printf("*************** SLAVE ***************\n");
}



int setNMod(int nm, enum dimension dim){
	return 1;
}



int getNModBoard(enum dimension arg){
	return 1;
}



int64_t getModuleId(enum idMode arg, int imod){

	/**/
	return -1;
}




int64_t getDetectorId(enum idMode arg){
	int64_t retval = -1;

	switch(arg){
	case DETECTOR_SERIAL_NUMBER:
		retval =  getDetectorNumber();/** to be implemented with mac? */
		break;
	case DETECTOR_FIRMWARE_VERSION:
		return (int64_t)Beb_GetFirmwareRevision();
	case DETECTOR_SOFTWARE_VERSION:
		retval= SVNREV;
		retval= (retval <<32) | SVNDATE;
		break;
	default:
		break;
	}

	return retval;
}



int getDetectorNumber(){
	int res=0;

	//execute and get address
	char output[255];
	FILE* sysFile = popen("more /home/root/executables/detid.txt", "r");
	fgets(output, sizeof(output), sysFile);
	pclose(sysFile);
	sscanf(output,"%d",&res);
	printf("detector id: %d\n",res);

/*
	int res=0;
	char hostname[100];
	if (gethostname(hostname, sizeof hostname) == 0)
		puts(hostname);
	else
		perror("gethostname");
	sscanf(hostname,"%x",&res);
*/
	return res;
}


u_int64_t  getDetectorMAC() {
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
}



int  getDetectorIP(){
	char temp[50]="";
	int res=0;
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



int moduleTest( enum digitalTestMode arg, int imod){
	//template testShiftIn from mcb_funcs.c

	//CHIP_TEST
	//testShiftIn
	//testShiftOut
	//testShiftStSel
	//testDataInOutMux
	//testExtPulseMux
	//testOutMux
	//testFpgaMux

	return OK;
}





int detectorTest( enum digitalTestMode arg){
	//templates from firmware_funcs.c

	//DETECTOR_FIRMWARE_TEST:testFpga()
	//DETECTOR_MEMORY_TEST:testRAM()
	//DETECTOR_BUS_TEST:testBus()
	//DETECTOR_SOFTWARE_TEST:testFpga()
	return OK;
}






void setDAC(enum detDacIndex ind, int val, int imod, int mV, int retval[]){

	if(ind == VTHRESHOLD){
		int ret[4];
		setDAC(VCMP_LL,val,imod,mV,retval);
			ret[0] = retval[mV];
		setDAC(VCMP_LR,val,imod,mV,retval);
			ret[1] = retval[mV];
		setDAC(VCMP_RL,val,imod,mV,retval);
			ret[2] = retval[mV];
		setDAC(VCMP_RR,val,imod,mV,retval);
			ret[3] = retval[mV];

		if((ret[0]== ret[1])&&
				(ret[1]==ret[2])&&
				(ret[2]==ret[3]))
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
	if(val >= 0)
		Feb_Control_SetDAC(iname,val,mV);
	int k;
	Feb_Control_GetDAC(iname, &k,0);
	retval[0] = k;
	Feb_Control_GetDAC(iname,&k,1);
	retval[1] = k;

	(detectorModules)->dacs[ind] = retval[0];

}


int setHighVoltage(int val, int imod){
	if(val!=-1){
		printf(" Setting High Voltage: %d\n",val);
		if(!top)
			eiger_highvoltage = val;
		else if(Feb_Control_SetHighVoltage(val))
			eiger_highvoltage = val;
	}
	return eiger_highvoltage;
}


int getADC(enum detDacIndex ind,  int imod){
	//get adc value
	return 0;
}


int setIODelay(int val, int imod){
	if(val!=-1){
		printf(" Setting IO Delay: %d\n",val);
		if(Feb_Control_SetIDelays(Feb_Control_GetModuleNumber(),val))
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
		Feb_Control_Set_Counter_Bit(val);
#ifdef VERBOSE
	printf("Counter Bit:%d\n",val);
#endif
	}

	return Feb_Control_Get_Counter_Bit();
}


int pulsePixel(int n, int x, int y){
	if(!Feb_Control_Pulse_Pixel(n,x,y))
		return FAIL;
	return OK;
}

int pulsePixelNMove(int n, int x, int y){
	if(!Feb_Control_PulsePixelNMove(n,x,y))
		return FAIL;
	return OK;
}

int pulseChip(int n){
	if(!Feb_Control_PulseChip(n))
		return FAIL;
	return OK;
}

int setRateCorrection(int64_t custom_tau_in_nsec){//in nanosec (will never be -1)

	//deactivating rate correction
	if(custom_tau_in_nsec==0){
		Feb_Control_SetRateCorrectionVariable(0);
		return 0;
	}

	int64_t tau_in_nsec = Feb_Control_Get_RateTable_Tau_in_nsec();
	int64_t subexp_in_nsec = Feb_Control_Get_RateTable_Subexptime_in_nsec();
	//same setting
	if((tau_in_nsec == custom_tau_in_nsec) && (subexp_in_nsec == Feb_Control_GetSubFrameExposureTime())){
		printf("Rate Table already created before: Same Tau %lldns, Same subexptime %lldns\n",
				tau_in_nsec,subexp_in_nsec);
	}
	//different setting, calculate table
	else{
		int ret = Feb_Control_SetRateCorrectionTau(custom_tau_in_nsec);
		if(ret<=0){
			cprintf(RED,"Rate correction failed. Deactivating rate correction\n");
			Feb_Control_SetRateCorrectionVariable(0);
			return ret;//-1 is for tau/subexptime error, 0 for all other errors
		}
	}
	//activating rate correction
	Feb_Control_SetRateCorrectionVariable(1);
#ifdef VERBOSE
	Feb_Control_PrintCorrectedValues();
#endif

	return Feb_Control_Get_RateTable_Tau_in_nsec();
}

int getRateCorrectionEnable(){
	return Feb_Control_GetRateCorrectionVariable();
}

int getDefaultSettingsTau_in_nsec(){
	switch(thisSettings){
	case STANDARD:	return STANDARD_TAU;
	case HIGHGAIN:	return HIGHGAIN_TAU;
	case LOWGAIN:	return LOWGAIN_TAU;
	default:		return -1;
	}
}


int setModule(sls_detector_module myMod, int* gain, int* offset,int* delay){
	int retval[2];
	int i;

	//#ifdef VERBOSE
	printf("Setting module with settings %d\n",myMod.reg);
	//#endif

	//set the settings variable
	setSettings( (enum detectorSettings)myMod.reg,-1);

	//set the gains and offset variables locally
	for(i=0;i<NGAIN;i++){
		if(gain[i]>=0){
			detectorGain[i] = gain[i];
			printf("gain[%d]:%d\n",i,detectorGain[i]);
		}else cprintf(RED,"gain not set\n");
	}
	for(i=0;i<NOFFSET;i++){
		if(offset[i]>=0){
			detectorOffset[i] = offset[i];
			printf("offset[%d]:%d\n",i,detectorOffset[i]);
		}else cprintf(RED,"offset not set\n");
	}

	if(setIODelay(*delay, -1)!= (*delay)){
		cprintf(RED,"could not set iodelay %d\n",*delay);
		return FAIL;
	}

	//copy module locally
	if (detectorModules)
		copyModule(detectorModules,&myMod);

	//set dac values
	for(i=0;i<myMod.ndac;i++)
		setDAC((enum detDacIndex)i,myMod.dacs[i],myMod.module,0,retval);

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

	return 0;
}


int getModule(sls_detector_module *myMod, int* gain, int* offset){
	int i;
	int retval[2];
	//printf("get gainval[0]:%d\n",detectorGain[0]);

	//dacs
	for(i=0;i<NDAC;i++)
		setDAC((enum detDacIndex)i,-1,-1,0,retval);

	//gains, offsets
	for(i=0;i<NGAIN;i++)
		gain[i] = detectorGain[i];
	for(i=0;i<NOFFSET;i++)
		offset[i] = detectorOffset[i];

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

	//copy to local copy as well
	if (detectorModules)
		copyModule(myMod,detectorModules);
	else
		return FAIL;
	return OK;
}






int getThresholdEnergy(int imod){
	printf(" Getting Threshold energy\n");
	return eiger_photonenergy;
}


int setThresholdEnergy(int ev, int imod){
	printf(" Setting threshold energy:%d\n",ev);
	int retval[2],i;
	int thrvalue[NGAIN];

	if(ev >= 0){
		eiger_photonenergy = ev;
		//calculate thrvalues for dacs
		for(i=0;i<NGAIN;i++){
			thrvalue[i] = (int) (( ((double)detectorGain[i]/1000) * (-1) * ((double)ev/1000)) + ((double)detectorOffset[i]/1000));
			printf("detectorGain[i]:%d detectorOffset[i]:%d thrvalue[i]:%d\n",detectorGain[i],detectorOffset[i],thrvalue[i]);
		}

		//setdacs
		setDAC(VCMP_LL,thrvalue[0],-1,0,retval);if(retval[0] != thrvalue[0]) cprintf(BG_RED,"Failed to set vcmp_ll to %d, got %d\n",retval[0],thrvalue[0]);
		setDAC(VCMP_LR,thrvalue[1],-1,0,retval);if(retval[0] != thrvalue[1]) cprintf(BG_RED,"Failed to set vcmp_lr to %d, got %d\n",retval[0],thrvalue[1]);
		setDAC(VCMP_RL,thrvalue[2],-1,0,retval);if(retval[0] != thrvalue[2]) cprintf(BG_RED,"Failed to set vcmp_rl to %d, got %d\n",retval[0],thrvalue[2]);
		setDAC(VCMP_RR,thrvalue[3],-1,0,retval);if(retval[0] != thrvalue[3]) cprintf(BG_RED,"Failed to set vcmp_rr to %d, got %d\n",retval[0],thrvalue[3]);
	}
	return  getThresholdEnergy(imod);
}



enum detectorSettings setSettings(enum detectorSettings sett, int imod){
	if(sett != GET_SETTINGS)
		thisSettings = sett;
	return thisSettings;
}


int startReceiver(int d){
	printf("Going to prepare for acquisition with counter_bit:%d\n",Feb_Control_Get_Counter_Bit());
	Feb_Control_PrepareForAcquisition();
	printf("Going to reset Frame Number\n");
	Beb_ResetFrameNumber();

	return OK;
}


int startStateMachine(){
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
			cprintf(RED,"Error: Acquisition did no start or trouble reading register\n");
			ret = FAIL;
		}
		cprintf(GREEN,"***Acquisition started\n");
	}

	/*while(getRunStatus() == IDLE){printf("waiting for being not idle anymore\n");}*/

	return ret;
}


int stopStateMachine(){
	cprintf(BG_RED,"Going to stop acquisition\n");
	if(Feb_Control_StopAcquisition() & Beb_StopAcquisition())
		return OK;
	cprintf(BG_RED,"failed to stop acquisition\n");
	return FAIL;
}


int startReadOut(){

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
}


enum runStatus getRunStatus(){
	//if(trialMasterMode == IS_MASTER){
	int i = Feb_Control_AcquisitionInProgress();
	if(i== 0){
		//printf("IDLE\n");
		return IDLE;
	}else{
		printf("RUNNING\n");
		return RUNNING;
	}
	//}else printf("***** not master*** \n");

	return IDLE;
}



char *readFrame(int *ret, char *mess){
	//if(master){
		if(!Feb_Control_WaitForFinishedFlag(5000))
			cprintf(RED,"Error: Waiting for finished flag\n");
		cprintf(GREEN,"Acquisition finished***\n");

		if(eiger_storeinmem){
			printf("requesting images after storing in memory\n");
			if(startReadOut() == FAIL){
				cprintf(RED, "Could not read out images\n");
				*ret = (int)FAIL;
				return NULL;
			}
		}
		//usleep(1000000);
		printf("*****Done Waiting...\n");
	//}
		*ret = (int)FINISHED;
	return NULL;
}







int64_t setTimer(enum timerIndex ind, int64_t val){

	switch(ind){
	case FRAME_NUMBER:
		if(val >= 0){
			printf(" Setting number of frames: %d * %d\n",(unsigned int)val,eiger_ncycles);
			if(Feb_Control_SetNExposures((unsigned int)val*eiger_ncycles)){
				eiger_nexposures = val;
				//SetDestinationParameters(EigerGetNumberOfExposures()*EigerGetNumberOfCycles());
				on_dst = 0;
				int i;
				for(i=0;i<32;i++) dst_requested[i] = 0; //clear dst requested
				ndsts_in_use = 1;
				nimages_per_request = eiger_nexposures * eiger_ncycles;
			}
		}return eiger_nexposures;

	case ACQUISITION_TIME:
		if(val >= 0){
			printf(" Setting exp time: %fs\n",val/(1E9));
			Feb_Control_SetExposureTime(val/(1E9));
		}
		return (Feb_Control_GetExposureTime()*(1E9));

	case SUBFRAME_ACQUISITION_TIME:
		if(val >= 0){
			printf(" Setting sub exp time: %dns\n",(int)val/10);
			Feb_Control_SetSubFrameExposureTime(val/10);
		}
		return (Feb_Control_GetSubFrameExposureTime());


	case FRAME_PERIOD:
		if(val >= 0){
			printf(" Setting acq period: %fs\n",val/(1E9));
			Feb_Control_SetExposurePeriod(val/(1E9));
		}return (Feb_Control_GetExposurePeriod()*(1E9));
		/*	case DELAY_AFTER_TRIGGER:
		if(val >= 0)
			EigerSetNumberOfExposures((unsigned int)val);
		return EigerGetNumberOfExposures();

	case GATES_NUMBER:
		if(val >= 0)
			EigerSetNumberOfGates((unsigned int)val);
		return EigerGetNumberOfGates();

	case PROBES_NUMBER:
		if(val >= 0)
			EigerSetNumberOfExposures((unsigned int)val);
		return EigerGetNumberOfExposures();*/
	case CYCLES_NUMBER:
		if(val >= 0){
			printf(" Setting number of triggers: %d * %d\n",(unsigned int)val,eiger_nexposures);
			if(Feb_Control_SetNExposures((unsigned int)val*eiger_nexposures)){
				eiger_ncycles = val;
				//SetDestinationParameters(EigerGetNumberOfExposures()*EigerGetNumberOfCycles());
				on_dst = 0;
				int i;
				for(i=0;i<32;i++) dst_requested[i] = 0; //clear dst requested
				nimages_per_request = eiger_nexposures * eiger_ncycles;
			}
		}return eiger_ncycles;
	default:
		printf("unknown timer index: %d\n",ind);
		break;
	}

	return -1;
}




int64_t getTimeLeft(enum timerIndex ind){

	return -1;
}



int setDynamicRange(int dr){
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
			if(Feb_Control_SetReadoutMode(val))
				eiger_readoutmode = val;
			else return -1;

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




int setROI(int n, ROI arg[], int *retvalsize, int *ret){
	return FAIL;
}



int setSpeed(enum speedVariable arg, int val){
	if(val != -1){
		printf(" Setting Read out Speed: %d\n",val);
		if(Feb_Control_SetReadoutSpeed(val))
			eiger_readoutspeed = val;
	}
	return 	eiger_readoutspeed;
}



int executeTrimming(enum trimMode mode, int par1, int par2, int imod){
	return FAIL;
}


int configureMAC(int ipad, long long int macad, long long int detectormacadd, int detipad, int udpport, int udpport2, int ival){
	if (detectormacadd != getDetectorMAC()){
		printf("*************************************************\n");
		printf("WARNING: actual detector mac address %llx does not match the one from client %llx\n",getDetectorMAC(),detectormacadd);
		detectormacadd = getDetectorMAC();
		printf("WARNING: Matched detectormac to the hardware mac now\n");
		printf("*************************************************\n");
	}
	//only for 1Gbe
	if(!send_to_ten_gig){
		if (detipad != getDetectorIP()){
			printf("*************************************************\n");
			printf("WARNING: actual detector ip address %x does not match the one from client %x\n",getDetectorIP(),detipad);
			detipad = getDetectorIP();
			printf("WARNING: Matched detector ip to the hardware ip now\n");
			printf("*************************************************\n");
		}
	}

	char src_mac[50], src_ip[50],dst_mac[50], dst_ip[50];
	int src_port = 0xE185;
	sprintf(src_ip,"%d.%d.%d.%d",(detipad>>24)&0xff,(detipad>>16)&0xff,(detipad>>8)&0xff,(detipad)&0xff);
	sprintf(dst_ip,"%d.%d.%d.%d",(ipad>>24)&0xff,(ipad>>16)&0xff,(ipad>>8)&0xff,(ipad)&0xff);
	sprintf(src_mac,"%02x:%02x:%02x:%02x:%02x:%02x",(unsigned int)((detectormacadd>>40)&0xFF),
			(unsigned int)((detectormacadd>>32)&0xFF),
			(unsigned int)((detectormacadd>>24)&0xFF),
			(unsigned int)((detectormacadd>>16)&0xFF),
			(unsigned int)((detectormacadd>>8)&0xFF),
			(unsigned int)((detectormacadd>>0)&0xFF));
	sprintf(dst_mac,"%02x:%02x:%02x:%02x:%02x:%02x",(unsigned int)((macad>>40)&0xFF),
			(unsigned int)((macad>>32)&0xFF),
			(unsigned int)((macad>>24)&0xFF),
			(unsigned int)((macad>>16)&0xFF),
			(unsigned int)((macad>>8)&0xFF),
			(unsigned int)((macad>>0)&0xFF));

	printf("src_port:%d\n",src_port);
	printf("src_ip:%s\n",src_ip);
	printf("dst_ip:%s\n",dst_ip);
	printf("src_mac:%s\n",src_mac);
	printf("dst_mac:%s\n",dst_mac);


	int beb_num = BEB_NUM;//Feb_Control_GetModuleNumber();
	int header_number = 0;
	int dst_port = udpport;

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

	return 0;
}


int calculateDataBytes(){
	if(send_to_ten_gig)
		return setDynamicRange(-1)*16*TEN_GIGA_BUFFER_SIZE;
	else
		return setDynamicRange(-1)*16*ONE_GIGA_BUFFER_SIZE;
}


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

	for (ichip=0; ichip<(srcMod->nchip); ichip++) {
		if (*((srcMod->chipregs)+ichip)>=0)
			*((destMod->chipregs)+ichip)=*((srcMod->chipregs)+ichip);
	}
	for (ichan=0; ichan<(srcMod->nchan); ichan++) {
		if (*((srcMod->chanregs)+ichan)>=0)
			*((destMod->chanregs)+ichan)=*((srcMod->chanregs)+ichan);
	}
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



int getTotalNumberOfChannels(){return getNumberOfChannelsPerModule();}
int getTotalNumberOfChips(){return NCHIP;}
int getTotalNumberOfModules(){return 1;}
int getNumberOfChannelsPerChip(){return  NCHAN;}
int getNumberOfChannelsPerModule(){return  getNumberOfChannelsPerChip() * getTotalNumberOfChips();}
int getNumberOfChipsPerModule(){return  NCHIP;}
int getNumberOfDACsPerModule(){return  NDAC;}
int getNumberOfADCsPerModule(){return  NADC;}
int getNumberOfGainsPerModule(){return  NGAIN;}
int getNumberOfOffsetsPerModule(){return  NOFFSET;}





enum externalSignalFlag getExtSignal(int signalindex){
	return GET_EXTERNAL_SIGNAL_FLAG;
}





enum externalSignalFlag setExtSignal(int signalindex,  enum externalSignalFlag flag){
	return getExtSignal(signalindex);
}






enum externalCommunicationMode setTiming( enum externalCommunicationMode arg){
	enum externalCommunicationMode ret=GET_EXTERNAL_COMMUNICATION_MODE;
	if(arg != GET_EXTERNAL_COMMUNICATION_MODE){
		switch((int)arg){
		case AUTO_TIMING:			ret = 0;	break;
		case TRIGGER_EXPOSURE:		ret = 2;	break;
		case TRIGGER_READOUT:		ret = 1;	break;
		case GATE_FIX_NUMBER:		ret = 3;	break;
		}
		printf(" Setting Triggering Mode: %d\n",(int)ret);
		if(Feb_Control_SetTriggerMode(ret,1))
			eiger_triggermode = ret;
	}

	ret = eiger_triggermode;
	switch((int)ret){
	case 0:		ret = AUTO_TIMING;		break;
	case 2:		ret = TRIGGER_EXPOSURE; break;
	case 1:		ret = TRIGGER_READOUT;	break;
	case 3:		ret = GATE_FIX_NUMBER;	break;
	default:
		printf("Unknown trigger mode found %d\n",ret);
		ret = 0;
	}
	return ret;
}


void setExternalGating(int enable[]){
	if(enable>=0){
		Feb_Control_SetExternalEnableMode(enable[0], enable[1]);//enable = 0 or 1, polarity = 0 or 1 , where 1 is positive
		eiger_extgating = enable[0];
		eiger_extgatingpolarity = enable[1];
	}
	enable[0] = eiger_extgating;
	enable[1] = eiger_extgatingpolarity;
}


enum masterFlags setMaster(enum masterFlags arg){
	//if(arg != GET_MASTER)
	//	masterMode = arg;

	return NO_MASTER;
}



enum synchronizationMode setSynchronization(enum synchronizationMode arg){
	return NO_SYNCHRONIZATION;
}

void setAllTrimbits(int val){
	int ichan;
	if(Feb_Control_SaveAllTrimbitsTo(val)){
#ifdef VERBOSE
		printf("Copying register %x value %d\n",destMod->reg,val);
#endif
		if (detectorModules){
			for (ichan=0; ichan<(detectorModules->nchan); ichan++) {
				*((detectorModules->chanregs)+ichan)=val;
			}
		}
	}else printf("error in setting all trimbits to value\n");
}

int getAllTrimbits(){
	return *((detectorModules->chanregs));
}

int getBebFPGATemp()
{
	return Beb_GetBebFPGATemp();
}

#endif
