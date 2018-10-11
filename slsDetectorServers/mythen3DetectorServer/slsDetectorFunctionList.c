//#ifdef SLS_DETECTOR_FUNCTION_LIST


#include "slsDetectorFunctionList.h"
#include "gitInfoMythen3.h"


#include "AD9257.h"     // include "commonServerFunctions.h", which in turn includes "blackfin.h"
#include "programfpga.h"
#include "RegisterDefs.h"

/* global variables */
sls_detector_module *detectorModules=NULL;
int *detectorChips=NULL;
int *detectorChans=NULL;
dacs_t *detectorDacs=NULL;
dacs_t *detectorAdcs=NULL;

enum detectorSettings thisSettings;
enum masterFlags masterMode = NO_MASTER;
int32_t clkPhase[2] = {0, 0};
int vlimit = -1;



/* basic tests */

void checkFirmwareCompatibility(int flag) {

    defineGPIOpins();
    resetFPGA();
    if (mapCSP0() == FAIL) {
        cprintf(BG_RED, "Dangerous to continue. Goodbye!\n");
        exit(EXIT_FAILURE);
    }

    // does check only if flag is 0 (by default), set by command line
    if ((!flag) && ((checkType() == FAIL) || (testFpga() == FAIL) || (testBus() == FAIL))) {
        cprintf(BG_RED, "Dangerous to continue. Goodbye!\n");
        exit(EXIT_FAILURE);
    }

    uint32_t ipadd              = getDetectorIP();
    uint64_t macadd             = getDetectorMAC();
    int64_t fwversion           = getDetectorId(DETECTOR_FIRMWARE_VERSION);
    int64_t swversion           = getDetectorId(DETECTOR_SOFTWARE_VERSION);
    //int64_t sw_fw_apiversion  = getDetectorId(SOFTWARE_FIRMWARE_API_VERSION);
    cprintf(BLUE,"\n\n"
            "********************************************************\n"
            "****************** Mythen3 Server *********************\n"
            "********************************************************\n\n"

            "Detector IP Addr:\t\t 0x%x\n"
            "Detector MAC Addr:\t\t 0x%llx\n"

            "Firmware Version:\t\t 0x%llx\n"
            "Software Version:\t\t 0x%llx\n"
            //"F/w-S/w API Version:\t\t 0x%llx\n"
            //"Required Firmware Version:\t 0x%x\n"
            "\n"
            "********************************************************\n",
            hversion, hsnumber,
            ipadd, macadd,
            fwversion, swversion
            //, sw_fw_apiversion, REQUIRED_FIRMWARE_VERSION
    );


/*
 *  printf("Testing firmware capability... ");
    //cant read versions
    if(!fwversion || !sw_fw_apiversion){
        cprintf(RED,"FATAL ERROR: Cant read versions from FPGA. Please update firmware\n");
        cprintf(RED,"Exiting Server. Goodbye!\n\n");
        exit(-1);
    }

    //check for API compatibility - old server
    if(sw_fw_apiversion > REQUIRED_FIRMWARE_VERSION){
        cprintf(RED,"FATAL ERROR: This software version is incompatible.\n"
                "Please update it to be compatible with this firmware\n\n");
        cprintf(RED,"Exiting Server. Goodbye!\n\n");
        exit(-1);
    }

    //check for firmware compatibility - old firmware
    if( REQUIRED_FIRMWARE_VERSION > fwversion){
        cprintf(RED,"FATAL ERROR: This firmware version is incompatible.\n"
                "Please update it to v%d to be compatible with this server\n\n", REQUIRED_FIRMWARE_VERSION);
        cprintf(RED,"Exiting Server. Goodbye!\n\n");
        exit(-1);
    }
*/
}


int checkType() {
    volatile u_int32_t type = ((bus_r(FPGA_VERSION_REG) & DETECTOR_TYPE_MSK) >> DETECTOR_TYPE_OFST);
    if (type != JUNGFRAUCTB){
            cprintf(BG_RED,"This is not a Mythen 3 Server (read %d, expected %d)\n",type, JUNGFRAUCTB);
            return FAIL;
        }

    return OK;
}



u_int32_t testFpga(void) {
    printf("\nTesting FPGA...\n");

    //fixed pattern
    int ret = OK;
    volatile u_int32_t val = bus_r(FIX_PATT_REG);
    if (val == FIX_PATT_VAL) {
        printf("Fixed pattern: successful match 0x%08x\n",val);
    } else {
        cprintf(RED,"Fixed pattern does not match! Read 0x%08x, expected 0x%08x\n", val, FIX_PATT_VAL);
        ret = FAIL;
    }
    return ret;
}


int testBus() {
    printf("\nTesting Bus...\n");

    int ret = OK;
    u_int32_t addr = SET_DELAY_LSB_REG;
    int times = 1000 * 1000;
    int i = 0;

    for (i = 0; i < times; ++i) {
        bus_w(addr, i * 100);
        if (i * 100 != bus_r(SET_DELAY_LSB_REG)) {
            cprintf(RED,"ERROR: Mismatch! Wrote 0x%x, read 0x%x\n", i * 100, bus_r(SET_DELAY_LSB_REG));
            ret = FAIL;
        }
    }

    if (ret == OK)
        printf("Successfully tested bus %d times\n", times);
    return ret;
}




int detectorTest( enum digitalTestMode arg){
    switch(arg){
    case DETECTOR_FIRMWARE_TEST:    return testFpga();
    case DETECTOR_BUS_TEST:         return testBus();
    default:
        cprintf(RED,"Warning: Test not implemented for this detector %d\n", (int)arg);
        break;
    }
    return OK;
}





/* Ids */

int64_t getDetectorId(enum idMode arg){
    int64_t retval = -1;

    switch(arg){
    case DETECTOR_SERIAL_NUMBER:
        retval = getDetectorMAC();
        break;
    case DETECTOR_FIRMWARE_VERSION:
        retval = getFirmwareVersion();
        break;
    //case SOFTWARE_FIRMWARE_API_VERSION:
    //return GetFirmwareSoftwareAPIVersion();
    case DETECTOR_SOFTWARE_VERSION:
        retval= GITREV;
        retval= (retval <<32) | GITDATE;
        break;
    default:
        break;
    }

    return retval;
}

u_int64_t getFirmwareVersion() {
    return ((bus_r(FPGA_VERSION_REG) & BOARD_REVISION_MSK) >> BOARD_REVISION_OFST);
}



u_int64_t  getDetectorMAC() {
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
}

u_int32_t  getDetectorIP(){
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
    sscanf(output, "%x",    &res);
    //printf("ip:%x\n",res);

    return res;
}








/* initialization */

void initControlServer(){
    clkPhase[0] = 0; clkPhase[1] = 0;
    setupDetector();
    printf("\n");
}



void initStopServer() {

    usleep(CTRL_SRVR_INIT_TIME_US);
    if (mapCSP0() == FAIL) {
        cprintf(BG_RED, "Stop Server: Map Fail. Dangerous to continue. Goodbye!\n");
        exit(EXIT_FAILURE);
    }
}






/* set up detector */

void allocateDetectorStructureMemory(){
    printf("This Server is for 1 Jungfrau module (500k)\n");

    //Allocation of memory
    if (detectorModules!=NULL) free(detectorModules);
    if (detectorChans!=NULL) free(detectorChans);
    if (detectorChips!=NULL) free(detectorChips);
    if (detectorDacs!=NULL) free(detectorDacs);
    if (detectorAdcs!=NULL) free(detectorAdcs);
    detectorModules=malloc(sizeof(sls_detector_module));
    detectorChips=malloc(NCHIP*sizeof(int));
    detectorChans=malloc(NCHIP*NCHAN*sizeof(int));
    detectorDacs=malloc(NDAC*sizeof(dacs_t));
    detectorAdcs=malloc(NADC*sizeof(dacs_t));
#ifdef VERBOSE
    printf("modules from 0x%x to 0x%x\n",detectorModules, detectorModules+n);
    printf("dacs from 0x%x to 0x%x\n",detectorDacs, detectorDacs+n*NDAC);
    printf("adcs from 0x%x to 0x%x\n",detectorAdcs, detectorAdcs+n*NADC);
#endif
    (detectorModules)->dacs=detectorDacs;
    (detectorModules)->adcs=detectorAdcs;
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

    resetPLL();
    resetCore();
    resetPeripheral();
    cleanFifos();

    //initialize dac series
    initDac(0);
    initDac(8);
    initDac(16);

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

    /*setSamples(1);
    bus_w(DAC_REG,0xffff);
    setSpeed
    cleanFifos();   /* todo might work without
    resetCore();    /* todo might work without */

    //Initialization of acquistion parameters
     setTimer(FRAME_NUMBER, DEFAULT_NUM_FRAMES);
    setTimer(CYCLES_NUMBER, DEFAULT_NUM_CYCLES);
    setTimer(ACQUISITION_TIME, DEFAULT_EXPTIME);
    setTimer(FRAME_PERIOD, DEFAULT_PERIOD);
    setTimer(DELAY_AFTER_TRIGGER, DEFAULT_DELAY);
}







/* firmware functions (resets) */

int powerChip (int on){

    /* set all the required voltages */
    return 0;
}


void cleanFifos() { printf("\nClearing Acquisition Fifos - Not doing anything\n");
   /* printf("\nClearing Acquisition Fifos\n");
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_ACQ_FIFO_CLR_MSK);
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_ACQ_FIFO_CLR_MSK);*/
}

void resetCore() {printf("\nResetting Core - Not doing anything\n");
    /*printf("\nResetting Core\n");
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_CORE_RST_MSK);
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_CORE_RST_MSK);*/
}

void resetPeripheral() {printf("\nResetting Peripheral - Not doing anything\n");
   /* printf("\nResetting Peripheral\n");
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_PERIPHERAL_RST_MSK);
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_PERIPHERAL_RST_MSK);*/
}

int getPhase(int i) {
    if (i>=0 && i<4)
        return clkPhase[i];
    else
        return -1;
}


int configurePhase(int val, int i) { /** compare with old jungfrau software and add in */

  u_int32_t l=0x0c;
  u_int32_t h=0x0d;
  u_int32_t vv;
  int32_t phase=0, inv=0;

  u_int32_t tot;
  u_int32_t odd=1;//0;

  if (i<0 || i>3)
    return -1;

  if (val>65535 || val<-65535)
    return clkPhase[i];

  resetPLL();

  setPllReconfigReg(PLL_MODE_REG, 1, 0);
  printf("phase in %d\n",clkPhase[1]);

  if (val>0) {
    inv=0;
    phase=val&0xffff;
  }  else {
    inv=0;
    val=-1*val;
    phase=(~val)&0xffff;
  }


  vv=phase | (i<<16);// | (inv<<21);

  setPllReconfigReg(PLL_PHASE_SHIFT_REG,vv,0);

  clkPhase[i]=val;
  return clkPhase[i];
}



int configureFrequency(int val, enum CLKINDEX i) { /** compare with old jungfrau software and add in */


    u_int32_t l=0x0c;
    u_int32_t h=0x0d;
    u_int32_t vv;
    int32_t phase=0, inv=0;

    u_int32_t tot;
    u_int32_t odd=1;//0;
    printf("Want to configure frequency of counter %d to %d\n",i,val);
    //   printf("PLL reconfig reset\N");   bus_w(PLL_CNTRL_REG,(1<<PLL_CNTR_RECONFIG_RESET_BIT));  usleep(100);  bus_w(PLL_CNTRL_REG, 0);
    if (i<0 || i>3) {
        printf("wrong counter number %d\n",i);
        return -1;
    }

    if (val<=0) {

        printf("get value %d %d \n",i,clkDivider[i]);
        return clkDivider[i];
    }
    if (i==adc_clk_c){
        if (val>40)
        {
            printf("Too high frequency %d MHz for these ADCs!\n", val);
            return clkDivider[i];
        }
    }

    tot= PLL_VCO_FREQ_MHZ/val;
    l=tot/2;
    h=l;
    if (tot>2*l) {
        h=l+1;
        odd=1;
    }
    else
    {
        odd=0;
    }

    printf("Counter %d: Low is %d, High is %d\n",i, l,h);


    vv= (i<<18)| (odd<<17) | l | (h<<8);

    printf("Counter %d, val: %08x\n", i,  vv);
    setPllReconfigReg(PLL_C_COUNTER_REG, vv,0);


    usleep(10000);

    resetPLL();

    clkDivider[i]=PLL_VCO_FREQ_MHZ/(l+h);

    printf("Frequency of clock %d is %d\n", i, clkDivider[i]);

    return clkDivider[i];
}








/* set parameters - nmod, dr, roi */

int setNMod(int nm, enum dimension dim){
    return NMOD;
}


int getNModBoard(enum dimension arg){
    return NMAXMOD;
}


int setDynamicRange(int dr){
    /* edit functionality */
    return 16;
}




/* parameters - readout */

int setSpeed(enum speedVariable arg, int val) {
    int retval = -1;

    switch (arg) {
    case DBIT_PHASE:
        if (val==-1)
            retval=getPhase(DBIT_CLK_C);
        else
            retval=configurePhase(val,DBIT_CLK_C);
        break;
    case DBIT_CLOCK:
        retval=configureFrequency(val,DBIT_CLK_C);
        if (configureFrequency(-1,SYNC_CLK_C)>retval){
            configureFrequency(retval,SYNC_CLK_C);
            printf("--Configuring sync clk to %d MHz\n",val);
        } else if (configureFrequency(-1,ADC_CLK_C)>retval && configureFrequency(-1,RUN_CLK_C)>retval) {
            printf("++Configuring sync clk to %d MHz\n",val);
            configureFrequency(retval,SYNC_CLK_C);
        }
        break;
    default:
        sprintf(mess,"Unknown speed parameter %d",arg);
        break;
    }
    return retval;
}






/* parameters - timer */

int64_t setTimer(enum timerIndex ind, int64_t val) {

    int64_t retval = -1;
    switch(ind){

    case FRAME_NUMBER:
        if(val >= 0)
            printf("\nSetting #frames: %lld\n",(long long int)val);
        retval = set64BitReg(val,  SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG);
        printf("Getting #frames: %lld\n",(long long int)retval);
        break;

    case ACQUISITION_TIME:
        if(val >= 0){
            printf("\nSetting exptime: %lldns\n", (long long int)val);
            val *= (1E-3 * CLK_RUN); /* ?? */
        }
        retval = set64BitReg(val, SET_EXPTIME_LSB_REG, SET_EXPTIME_MSB_REG) / (1E-3 * CLK_RUN);
        printf("Getting exptime: %lldns\n", (long long int)retval);
        break;

    case FRAME_PERIOD:
        if(val >= 0){
            printf("\nSetting period to %lldns\n",(long long int)val);
            val *= (1E-3 * CLK_SYNC); /* ?? */
        }
        retval = set64BitReg(val, SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG )/ (1E-3 * CLK_SYNC);
        printf("Getting period: %lldns\n", (long long int)retval);
        break;

    case DELAY_AFTER_TRIGGER:
        if(val >= 0){
            printf("\nSetting delay to %lldns\n", (long long int)val);
            val *= (1E-3 * CLK_SYNC); /* ?? */
        }
        retval = set64BitReg(val, SET_DELAY_LSB_REG, SET_DELAY_MSB_REG) / (1E-3 * CLK_SYNC);
        printf("Getting delay: %lldns\n", (long long int)retval);
        break;

    case CYCLES_NUMBER:
        if(val >= 0)
            printf("\nSetting #cycles to %lld\n", (long long int)val);
        retval = set64BitReg(val,  SET_CYCLES_LSB_REG, SET_CYCLES_MSB_REG);
        printf("Getting #cycles: %lld\n", (long long int)retval);
        break;

    case GATES_NUMBER:
        if(val >= 0)
            printf("\nSetting #gates to %lld\n", (long long int)val);
        retval = set64BitReg(val,  SET_GATES_LSB_REG, SET_GATES_MSB_REG);
        printf("Getting #gates: %lld\n", (long long int)retval);
        break;

    case PROBES_NUMBER: /* does not exist in firmware_funcs.c*/
        /*if(val >= 0)
            printf("\nSetting #probes to %lld\n", (long long int)val);
        retval = set64BitReg(val,  SET_PROBES_LSB_REG, SET_PROBES_MSB_REG);
        printf("Getting #probes: %lld\n", (long long int)retval);
        */
        break;

    case SAMPLES_JCTB:
        if(val >= 0)
            printf("\nSetting #samples to %lld\n", (long long int)val);
        retval = bus_w(NSAMPLES_REG,  val);
        printf("Getting #samples: %lld\n", (long long int)retval);

        break;

    default:
        cprintf(RED,"Warning: Timer Index not implemented for this detector: %d\n", ind);
        break;
    }

    return retval;

}



int64_t getTimeLeft(enum timerIndex ind){
    int64_t retval = -1;
    switch(ind){

    case FRAME_NUMBER:
        retval = get64BitReg(GET_FRAMES_LSB_REG, GET_FRAMES_MSB_REG);
        printf("Getting number of frames left: %lld\n",(long long int)retval);
        break;

    case FRAME_PERIOD:
        retval = get64BitReg(GET_PERIOD_LSB_REG, GET_PERIOD_MSB_REG) / (1E-3 * CLK_SYNC);
        printf("Getting period left: %lldns\n", (long long int)retval);
        break;

    case DELAY_AFTER_TRIGGER:
        retval = get64BitReg(GET_DELAY_LSB_REG, GET_DELAY_MSB_REG) / (1E-3 * CLK_SYNC);
        printf("Getting delay left: %lldns\n", (long long int)retval);
        break;

 /** acquisition time and period gives in time left in func.c, pls check with anna */

    case CYCLES_NUMBER:
        retval = get64BitReg(GET_CYCLES_LSB_REG, GET_CYCLES_MSB_REG);
        printf("Getting number of cycles left: %lld\n", (long long int)retval);
        break;

    case ACTUAL_TIME:
        retval = get64BitReg(GET_ACTUAL_TIME_LSB_REG, GET_ACTUAL_TIME_MSB_REG) / (1E-9 * CLK_SYNC);
        printf("Getting actual time (time from start): %lld\n", (long long int)retval);
        break;

    case MEASUREMENT_TIME:
        retval = get64BitReg(GET_MEASUREMENT_TIME_LSB_REG, GET_MEASUREMENT_TIME_MSB_REG) / (1E-9 * CLK_SYNC);
        printf("Getting measurement time (timestamp/ start frame time): %lld\n", (long long int)retval);
        break;

    case FRAMES_FROM_START:
        retval = get64BitReg(FRAMES_FROM_START_LSB_REG, FRAMES_FROM_START_MSB_REG);
        printf("Getting frames from start run control %lld\n", (long long int)retval);
        break;

    case FRAMES_FROM_START_PG: /** ask anna, seems to be calling previous function (frames_from_start) */
        retval = get64BitReg(FRAMES_FROM_START_PG_LSB_REG, FRAMES_FROM_START_PG_MSB_REG);
        printf("Getting frames from start run control %lld\n", (long long int)retval);
        break;

    default:
        cprintf(RED, "Warning: Remaining Timer index not implemented for this detector: %d\n", ind);
        break;
    }

    return retval;
}






/* parameters - channel, chip, module, settings */


int setModule(sls_detector_module myMod){
    return thisSettings;
}


int getModule(sls_detector_module *myMod){
    return OK;
}



enum detectorSettings setSettings(enum detectorSettings sett, int imod){
    return getSettings();

}


enum detectorSettings getSettings(){
    return thisSettings;
}





/* parameters - dac, adc, hv */






void initDac(int dacnum) {
    printf("\nInitializing dac for %d to \n",dacnum);

    u_int32_t codata;
    int csdx        = dacnum / NDAC_PER_SET + DAC_SERIAL_CS_OUT_OFST;   //,so can be DAC_SERIAL_CS_OUT_OFST or +1
    int dacchannel  = 0xf;                                      // all channels
    int dacvalue    = 0x6;                                      // can be any random value (just writing to power up)
    printf(" Write to Input Register\n"
            " Chip select bit:%d\n"
            " Dac Channel:0x%x\n"
            " Dac Value:0x%x\n",
            csdx, dacchannel, dacvalue);

    codata = LTC2620_DAC_CMD_WRITE +                                            // command to write to input register
            ((dacchannel << LTC2620_DAC_ADDR_OFST) & LTC2620_DAC_ADDR_MSK) +    // all channels
            ((dacvalue << LTC2620_DAC_DATA_OFST) & LTC2620_DAC_DATA_MSK);       // any random value
    serializeToSPI(SPI_REG, codata, (0x1 << csdx), LTC2620_DAC_NUMBITS,
            DAC_SERIAL_CLK_OUT_MSK, DAC_SERIAL_DIGITAL_OUT_MSK, DAC_SERIAL_DIGITAL_OUT_OFST);
}









int voltageToDac(int value){
    int vmin = 0;
    int vmax = MAX_DACVOLTVAL;
    int nsteps = MAX_DACVAL;
    if ((value < vmin) || (value > vmax)) {
        cprintf(RED,"Voltage value (to convert to dac value) is outside bounds: %d\n", value);
        return -1;
    }
    return (int)(((value - vmin) / (vmax - vmin)) * (nsteps - 1) + 0.5);
}

int dacToVoltage(unsigned int digital){
    int vmin = 0;
    int vmax = MAX_DACVOLTVAL;
    int nsteps = MAX_DACVAL;
    int v = vmin + (vmax - vmin) * digital / (nsteps - 1);
    if((v < 0) || (v > nsteps - 1)) {
        cprintf(RED,"Voltage value (converted from dac value) is outside bounds: %d\n", v);
        return -1;
    }
    return v;
}

int powerToDac(int value, int chip) {
    int nsteps = MAX_DACVAL;
    int vchip = MAX_VCHIPVAL - (getDacRegister(V_CHIP)*1000)/(nsteps -1);
    printf("Current V_Chip: %d mV\n",vchip);

    int vmax = vchip - MIN_VCHIP_OFSTVAL;
    int vmin = MIN_VCHIP_VAL;

    // recalculating only for v_chip
    if (chip) {
        printf("vchip\n");
        vmax = MAX_VCHIPVAL;
        vmin = MAX_VCHIPVAL - 1000;
    }
    else
        printf("vpower\n");

    int v = (int)(((vmax - value) * (nsteps - 1) / (vmax - vmin)) ); /***+0.5 removed is this correct? seems different from voltageToDac maybe get rid of 0.5*/


    if (v < 0) v = 0;
    if (v > nsteps - 1) v = nsteps - 1;
    if (value == -100) v = -100;

   return v;
}

int dacToPower(int value, int chip) {
    int retval1 = -1;
    int nsteps = MAX_DACVAL;
    int vchip = MAX_VCHIPVAL - (getDacRegister(V_CHIP)*1000)/(nsteps -1);
    printf("Vchip id %d mV\n",vmax);
    int vmax = vchip - MIN_VCHIP_OFSTVAL;
    int vmin = MIN_VCHIP_VAL;

    // recalculating only for v_chip
    if (chip) {
        printf("vchip\n");
        vmax = MAX_VCHIPVAL;
        vmin = MAX_VCHIPVAL - 1000;
    }
    else
        printf("vpower\n");

    retval1 = vmax - (value * (vmax - vmin)) / (nsteps - 1);
    if (retval1 > vmax) retval1 = vmax;
    if (retval1 < vmin) retval1 = vmin;
    if (value < 0) retval1 = value;
    return retval1;
}



void setDAC(enum DACINDEX ind, int val, int imod, int mV, int retval[]){

    int dacval = val;

    // dacs: if set and mv, convert to dac
    if (ind < val > 0 && mV) {
        val = voltageToDac(val); //gives -1 on error
    }


    if ( (val >= 0) || (val == -100)) {


        u_int32_t ichip = 2 - ind / NDAC_PER_SET;
        u_int32_t valw = 0;
        int i;

        // start and chip select bar down -----------------
        SPIChipSelect (&valw, SPI_REG, (0x1 << csdx));


        // next dac --------------------------------------
        for (i = 0; i < ichip; ++i) {
            printf("%d next DAC\n", i);
            sendDataToSPI (&valw, SPI_REG, LTC2620_DAC_CMD_MSK, LTC2620_DAC_NUMBITS,
                    DAC_SERIAL_CLK_OUT_MSK, DAC_SERIAL_DIGITAL_OUT_MSK, DAC_SERIAL_DIGITAL_OUT_OFST);
        }


        // this dac ----------------------------------------
        u_int32_t codata;
        int csdx        = DAC_SERIAL_CS_OUT_OFST;
        int dacchannel  = ind % NDAC_PER_SET;

        printf("\nSetting of DAC %d : %d dac units (%d mV)\n",ind, dacval, val);
        // command
        if (val >= 0) {
            printf(" Write to Input Register and Update\n");
            codata = LTC2620_DAC_CMD_SET;

        } else if (val == -100) {
            printf(" POWER DOWN\n");
            codata = LTC2620_DAC_CMD_POWER_DOWN;
        }
        // address
        printf(" Chip select bit:%d\n"
                " Dac Channel:0x%x\n"
                " Dac Value:0x%x\n",
                csdx, dacchannel, val);
        codata += ((dacchannel << LTC2620_DAC_ADDR_OFST) & LTC2620_DAC_ADDR_MSK) +
                ((val << LTC2620_DAC_DATA_OFST) & LTC2620_DAC_DATA_MSK);
        // to spi
        serializeToSPI(SPI_REG, codata, (0x1 << csdx), LTC2620_DAC_NUMBITS,
                DAC_SERIAL_CLK_OUT_MSK, DAC_SERIAL_DIGITAL_OUT_MSK, DAC_SERIAL_DIGITAL_OUT_OFST);


        printf("--------done setting dac set %d \n",i);

        // next dac -----------------------------------------------------------
        for (i = ichip+1; i < (N_DAC / NDAC_PER_SET); ++i) {
            printf("%d next DAC\n", i);
            sendDataToSPI (&valw, SPI_REG, LTC2620_DAC_CMD_MSK, LTC2620_DAC_NUMBITS,
                    DAC_SERIAL_CLK_OUT_MSK, DAC_SERIAL_DIGITAL_OUT_MSK, DAC_SERIAL_DIGITAL_OUT_OFST);
        }


        //chip select bar up, clk down and stop --------------------------------
        SPIChipDeselect (&valw, SPI_REG, (0x1 << csdx), DAC_SERIAL_CLK_OUT_MSK);

        // writes to register
        setDacRegister(ind, dacval);
    }

    // reading dac value from register
    printf("Getting DAC %d : ",ind);
    retval[0] = getDacRegister(ind);    printf("%d dac units ", retval[0]);
    retval[1] = dacToVoltage(retval[0]);printf("(%d mV)\n", retval[1]);
}


int setPower(enum DACINDEX ind, int val) {

    // not implemented yet
    if ((ind == V_D) || (ind == V_C))
        return -1;

    // vlimit software limit
    else if (ind == V_LIMIT) {
        if (val >= 0)
            vlimit = val;
        return vlimit;
    }

    int pwrindex = -1;
    int dacval = val;
    int retval=-1, retval1=-1;
    u_int32_t preg = 0;
    int temp[2] = {-1,-1};

    // get
    if (val == -1)
        dacval = -1;

    // vchip
    else if (ind == V_CHIP)
        dacval = powerToDac(val, 1);

    // power a, b, c , d, io
    else {
        dacval = powerToDac(val, 0);

        switch (ind) {
        case V_IO: pwrindex = 0;break;
        case V_A: pwrindex = 1; break;
        case V_B: pwrindex = 2; break;
        default:break;
        }

        // shut down
        preg = bus_r(POWER_ON_REG);
        printf("power reg is %08x\n",bus_r(POWER_ON_REG));
        printf("Switching off power %d\n", ind);
        bus_w(POWER_ON_REG,preg &(~(1 << (POWER_ENABLE_OFST + pwrindex))));
        setDac(ind,-100, 0, 1, temp);
        printf("power reg is %08x\n",bus_r(POWER_ON_REG));
        retval=0;
    }

    // actual setting
    if (val != -100) {
        printf("Setting power %d to %d mV (%d dac val)\n",ind, val, dacval);
        retval = setDac(ind, dacval);
        // setting power a, b, c, d, io
        if (pwrindex >= 0 && dacval >= 0 ) {
            preg = bus_r(POWER_ON_REG);
            printf("power reg is %08x\n", bus_r(POWER_ON_REG));
            printf("Switching on power %d\n", pwrindex);
            bus_w(POWER_ON_REG, preg | ((1 << (POWER_ENABLE_OFST + pwrindex))));
            printf("power reg is %08x\n",bus_r(POWER_ON_REG));
        }
    }

    if (pwrindex >= 0) {
        if (bus_r(POWER_ON_REG) & (1 << (POWER_ENABLE_OFST + pwrindex))) {
            retval1 = dacToPower(retval, 0);
            printf("Vdac id %d mV\n",retval1);
        } else
            retval1 = 0;
    } else {
        if (retval >= 0) {
            retval1 = dacToPower(retval, 1);
            /*printf("Vchip is %d mV\n",vmax); makes no sense.. should be printing retval1??? */
        } else
            retval1=-1;
    }

    return retval1;
}

int getVLimit() {
    return vlimit;
}

void setDacRegister(int dacnum,int dacvalue) {
    bus_w(DAC_NUM_REG, dacnum);
    bus_w(DAC_VAL_REG, dacvalue);
    bus_w(DAC_NUM_REG, dacnum | (1<<16));/** super strange writing dac num in weird ways */
    bus_w(DAC_NUM_REG, dacnum);
    printf("Wrote dac register value %d address %d\n",bus_r(DAC_VAL_REG),bus_r(DAC_NUM_REG)) ;
}

int getDacRegister(int dacnum) {/** super strange, reading out in some other register than written */
    bus_w(DAC_NUM_REG, dacnum);
    printf("READ dac register value %d address %d\n",(int16_t)bus_r(DAC_VAL_OUT_REG),bus_r(DAC_NUM_REG)) ;
    return (int16_t)bus_r(DAC_VAL_OUT_REG);
}





/* parameters - timing, extsig */


enum externalCommunicationMode setTiming( enum externalCommunicationMode arg){
    return AUTO_TIMING;
}




/* jungfrau specific - pll, flashing fpga */



void resetPLL() {
    // reset PLL Reconfiguration and PLL
    bus_w(PLL_CONTROL_REG, bus_r(PLL_CONTROL_REG) | PLL_CTRL_RECONFIG_RST_MSK | PLL_CTRL_RST_MSK);
    usleep(100);
    bus_w(PLL_CONTROL_REG, bus_r(PLL_CONTROL_REG) & ~PLL_CTRL_RECONFIG_RST_MSK & ~PLL_CTRL_RST_MSK);
}


u_int32_t setPllReconfigReg(u_int32_t reg, u_int32_t val) {

    // set parameter
    bus_w(PLL_PARAM_REG, val);

    // set address
    bus_w(PLL_CONTROL_REG, (reg << PLL_CTRL_ADDR_OFST) & PLL_CTRL_ADDR_MSK);
    usleep(10*1000);

    //write parameter
    bus_w(PLL_CONTROL_REG, bus_r(PLL_CONTROL_REG) | PLL_CTRL_WR_PARAMETER_MSK);
    bus_w(PLL_CONTROL_REG, bus_r(PLL_CONTROL_REG) & ~PLL_CTRL_WR_PARAMETER_MSK);
    usleep(10*1000);

    return val;
}




void configurePll() {
    u_int32_t val;
    int32_t phase=0, inv=0;

    printf(" phase in %d\n", clkPhase[1]);
    if (clkPhase[1]>0) {
        inv=0;
        phase=clkPhase[1];
    }  else {
        inv=1;
        phase=-1*clkPhase[1];
    }
    printf(" phase out %d (0x%08x)\n", phase, phase);

    if (inv) {
        val = ((phase << PLL_SHIFT_NUM_SHIFTS_OFST) & PLL_SHIFT_NUM_SHIFTS_MSK) + PLL_SHIFT_CNT_SLCT_C1_VAL + PLL_SHIFT_UP_DOWN_NEG_VAL;
        printf(" phase word 0x%08x\n", val);
        setPllReconfigReg(PLL_PHASE_SHIFT_REG, val);
    } else {
        val = ((phase << PLL_SHIFT_NUM_SHIFTS_OFST) & PLL_SHIFT_NUM_SHIFTS_MSK) + PLL_SHIFT_CNT_SLCT_C0_VAL + PLL_SHIFT_UP_DOWN_NEG_VAL;
        printf(" phase word 0x%08x\n", val);
        setPllReconfigReg(PLL_PHASE_SHIFT_REG, val);

        printf(" phase word 0x%08x\n", val);
        val = ((phase << PLL_SHIFT_NUM_SHIFTS_OFST) & PLL_SHIFT_NUM_SHIFTS_MSK) + PLL_SHIFT_CNT_SLCT_C2_VAL;
        setPllReconfigReg(PLL_PHASE_SHIFT_REG, val);
    }
    usleep(10000);
}




/* aquisition */

int startStateMachine(){
    printf("*******Starting State Machine*******\n");

    cleanFifos();

    //start state machine
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_START_ACQ_MSK);
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_START_ACQ_MSK);

    printf("Status Register: %08x\n",bus_r(STATUS_REG));
    return OK;
}


int stopStateMachine(){
    cprintf(BG_RED,"*******Stopping State Machine*******\n");

    //stop state machine
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) | CONTROL_STOP_ACQ_MSK);
    usleep(100);
    bus_w(CONTROL_REG, bus_r(CONTROL_REG) & ~CONTROL_STOP_ACQ_MSK);

    printf("Status Register: %08x\n",bus_r(STATUS_REG));
    return OK;
}





enum runStatus getRunStatus(){
#ifdef VERBOSE
    printf("Getting status\n");
#endif

    enum runStatus s;
    u_int32_t retval = bus_r(STATUS_REG);
    printf("Status Register: %08x\n",retval);

    //running
    if(((retval & RUN_BUSY_MSK) >> RUN_BUSY_OFST)) {
        if ((retval & WAITING_FOR_TRIGGER_MSK) >> WAITING_FOR_TRIGGER_OFST) {
            printf("-----------------------------------WAITING-----------------------------------\n");
            s=WAITING;
        }
        else{
            printf("-----------------------------------RUNNING-----------------------------------\n");
            s=RUNNING;
        }
    }

    //not running
    else {
        if ((retval & STOPPED_MSK) >> STOPPED_OFST) {
            printf("-----------------------------------STOPPED--------------------------\n");
            s=STOPPED;
        } else if ((retval & RUNMACHINE_BUSY_MSK) >> RUNMACHINE_BUSY_OFST) {
            printf("-----------------------------------READ MACHINE BUSY--------------------------\n");
            s=TRANSMITTING;
        } else if (!retval) {
            printf("-----------------------------------IDLE--------------------------------------\n");
            s=IDLE;
        } else {
            printf("-----------------------------------Unknown status %08x--------------------------------------\n", retval);
            s=ERROR;
        }
    }

    return s;
}



void readFrame(int *ret, char *mess){

    // wait for status to be done
    while(runBusy()){
        usleep(500);
    }

    // frames left to give status
    int64_t retval = getTimeLeft(FRAME_NUMBER) + 1;
    if ( retval > 0) {
        *ret = (int)FAIL;
        sprintf(mess,"no data and run stopped: %lld frames left\n",retval);
        cprintf(RED,"%s\n",mess);
    } else {
        *ret = (int)FINISHED;
        sprintf(mess,"acquisition successfully finished\n");
        printf("%s",mess);
    }
}



u_int32_t runBusy(void) {
    u_int32_t s = ((bus_r(STATUS_REG) & RUN_BUSY_MSK) >> RUN_BUSY_OFST);
#ifdef VERBOSE
    printf("Status Register: %08x\n", s);
#endif
    return s;
}








/* common */

//jungfrau doesnt require chips and chans (save memory)
int copyModule(sls_detector_module *destMod, sls_detector_module *srcMod){

    int idac, iadc;
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
    return DATA_BYTES;
}

int getTotalNumberOfChannels(){return ((int)getNumberOfChannelsPerModule() * (int)getTotalNumberOfModules());}
int getTotalNumberOfChips(){return ((int)getNumberOfChipsPerModule() * (int)getTotalNumberOfModules());}
int getTotalNumberOfModules(){return NMOD;}
int getNumberOfChannelsPerModule(){return  ((int)getNumberOfChannelsPerChip() * (int)getTotalNumberOfChips());}
int getNumberOfChipsPerModule(){return  NCHIP;}
int getNumberOfDACsPerModule(){return  NDAC;}
int getNumberOfADCsPerModule(){return  NADC;}
int getNumberOfChannelsPerChip(){return  NCHAN;}



/* sync */

enum masterFlags setMaster(enum masterFlags arg){
    return NO_MASTER;
}

enum synchronizationMode setSynchronization(enum synchronizationMode arg){
    return NO_SYNCHRONIZATION;
}






//#endif
