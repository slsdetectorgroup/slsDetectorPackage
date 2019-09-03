#include "slsDetectorFunctionList.h"
#include "versionAPI.h"
#include "clogger.h"
#include "nios.h"
#include "DAC6571.h"
#include "common.h"
#ifdef VIRTUAL
#include "communication_funcs_UDP.h"
#endif

#include <string.h>
#include <unistd.h>     // usleep
#ifdef VIRTUAL
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

int highvoltage = 0;


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
	// faking it
    firmware_check_done = 1;

	
#endif
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
	return 0;
}

u_int64_t getFirmwareAPIVersion() {
#ifdef VIRTUAL
    return 0;
#endif
    return 0;
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

	// hv
    DAC6571_SetDefines(HV_HARD_MAX_VOLTAGE, HV_DRIVER_FILE_NAME);
    setHighVoltage(DEFAULT_HIGH_VOLTAGE);

	//Initialization of acquistion parameters
	setTimer(FRAME_NUMBER, DEFAULT_NUM_FRAMES);
	setTimer(CYCLES_NUMBER, DEFAULT_NUM_CYCLES);
	setTimer(ACQUISITION_TIME, DEFAULT_EXPTIME);
	setTimer(ACQUISITION_TIME, DEFAULT_PERIOD);
}


/* set parameters -  dr, roi */

int setDynamicRange(int dr){
	return -1;
}


/* parameters  */


int64_t setTimer(enum timerIndex ind, int64_t val) {

	int64_t retval = -1;
#ifdef VIRTUAL
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
			FILE_LOG(logINFO, ("Setting exptime: %lldns\n", (long long int)val));
			val *= (1E-3 * TEMP_CLK);
		}
		retval = set64BitReg(val, SET_EXPTIME_LSB_REG, SET_EXPTIME_MSB_REG) / (1E-3 * TEMP_CLK); // CLK defined in slsDetectorServer_defs.h
		FILE_LOG(logDEBUG1, ("Getting exptime: %lldns\n", (long long int)retval));
		break;

	case FRAME_PERIOD:
		if(val >= 0){
			FILE_LOG(logINFO, ("Setting period: %lldns\n",(long long int)val));
			val *= (1E-3 * TEMP_CLK);
		}
		retval = set64BitReg(val, SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG )/ (1E-3 * TEMP_CLK);
		FILE_LOG(logDEBUG1, ("Getting period: %lldns\n", (long long int)retval));
		break;
	case CYCLES_NUMBER:
		if(val >= 0) {
			FILE_LOG(logINFO, ("Setting #cycles: %lld\n", (long long int)val));
		}
		retval = set64BitReg(val,  SET_CYCLES_LSB_REG, SET_CYCLES_MSB_REG);
		FILE_LOG(logDEBUG1, ("Getting #cycles: %lld\n", (long long int)retval));
		break;

	default:
		FILE_LOG(logERROR, ("Timer Index not implemented for this detector: %d\n", ind));
		break;
	}
#endif
	return retval;

}

int validateTimer(enum timerIndex ind, int64_t val, int64_t retval) {
    if (val < 0)
        return OK;
    switch(ind) {
    case ACQUISITION_TIME:
    case FRAME_PERIOD:
		// convert to freq
        val *= (1E-3 * TEMP_CLK);
        // convert back to timer
        val = (val) / (1E-3 * TEMP_CLK);
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
#ifdef VIRTUAL
	int64_t retval = -1;
	switch(ind){

	case FRAME_NUMBER:
		retval = get64BitReg(GET_FRAMES_LSB_REG, GET_FRAMES_MSB_REG);
		FILE_LOG(logINFO, ("Getting number of frames left: %lld\n",(long long int)retval));
		break;

	case CYCLES_NUMBER:
		retval = get64BitReg(GET_CYCLES_LSB_REG, GET_CYCLES_MSB_REG);
		FILE_LOG(logINFO, ("Getting number of cycles left: %lld\n", (long long int)retval));
		break;

	default:
		FILE_LOG(logERROR, ("Remaining Timer index not implemented for this detector: %d\n", ind));
		break;
	}
#endif
	return -1;
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
	    FILE_LOG(logINFO, ("Setting High voltage: %d V", val));
	    DAC6571_Set(val);
	    highvoltage = val;
	}
	return highvoltage;
}


int configureMAC(uint32_t destip, uint64_t destmac, uint64_t sourcemac, uint32_t sourceip, uint32_t udpport) {
#ifdef VIRTUAL
	char cDestIp[MAX_STR_LENGTH];
	memset(cDestIp, 0, MAX_STR_LENGTH);
	sprintf(cDestIp, "%d.%d.%d.%d", (destip>>24)&0xff,(destip>>16)&0xff,(destip>>8)&0xff,(destip)&0xff);
	FILE_LOG(logINFO, ("1G UDP: Destination (IP: %s, port:%d)\n", cDestIp, udpport));
	if (setUDPDestinationDetails(0, cDestIp, udpport) == FAIL) {
		FILE_LOG(logERROR, ("could not set udp destination IP and port\n"));
		return FAIL;
	}
    return OK;
#endif
	return OK;
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
						setTimer(CYCLES_NUMBER, -1) );
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
	return 0;
}

int getTotalNumberOfChannels(){return  ((int)getNumberOfChannelsPerChip() * (int)getNumberOfChips());}
int getNumberOfChips(){return  NCHIP;}
int getNumberOfDACs(){return  NDAC;}
int getNumberOfChannelsPerChip(){return  NCHAN;}