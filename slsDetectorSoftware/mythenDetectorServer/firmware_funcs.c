
#include "server_defs.h"
#include "firmware_funcs.h"
#include "mcb_funcs.h"
#include "registers.h"

#ifdef SHAREDMEMORY
#include "sharedmemory.h"
#endif

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>


//for memory mapping
u_int32_t CSP0BASE;

FILE *debugfp, *datafp;

int fr;
int wait_time;
int *fifocntrl;
int *values;
int *statusreg;
const int nModY=1;
int nModBoard;
int nModX=NMAXMOD;
int dynamicRange=32;
int dataBytes=NMAXMOD*NCHIP*NCHAN*4;
int storeInRAM=0;
int *ram_values=NULL;
char *now_ptr=NULL;
int ram_size=0;


int ififostart, ififostop, ififostep, ififo;


#ifdef MCB_FUNCS
extern const int nChans;
extern const int nChips;
extern const int nDacs;
extern const int nAdcs;
#endif
#ifndef MCB_FUNCS

const int nChans=NCHAN;
const int nChips=NCHIP;
const int nDacs=NDAC;
const int nAdcs=NADC;
#endif

//int mybyte;
//int mysize=dataBytes/8;

int mapCSP0(void) {
  printf("Mapping memory\n");
#ifndef VIRTUAL
  int fd;
  fd = open("/dev/mem", O_RDWR | O_SYNC, 0);
  if (fd == -1) {
    printf("\nCan't find /dev/mem!\n");
    return FAIL;
  }
  printf("/dev/mem opened\n");
  CSP0BASE = (u_int32_t)mmap(0, MEM_SIZE, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, CSP0);
  if (CSP0BASE == (u_int32_t)MAP_FAILED) {
    printf("\nCan't map memmory area!!\n");
    return FAIL;
  } 
  printf("CSP0 mapped\n");

#endif
#ifdef VIRTUAL
  CSP0BASE = (u_int32_t)malloc(MEM_SIZE);
  printf("memory allocated\n");
#endif
#ifdef SHAREDMEMORY 
  if ( (res=inism(SMSV))<0) {
    printf("error attaching shared memory! %i",res);
    return FAIL;
  }
#endif
  printf("CSPOBASE=from %08x to %x\n",CSP0BASE,CSP0BASE+MEM_SIZE);
  
  values=(u_int32_t*)(CSP0BASE+FIFO_DATA_REG_OFF);
  printf("values=%08x\n",values);
  fifocntrl=(u_int32_t*)(CSP0BASE+FIFO_CNTRL_REG_OFF);
  printf("fifcntrl=%08x\n",fifocntrl);
  statusreg=(u_int32_t*)(CSP0BASE+STATUS_REG);
  printf("statusreg=%08x\n",statusreg);

  return OK;
}


u_int32_t bus_w(u_int32_t offset, u_int32_t data) {
  u_int32_t *ptr1;


  ptr1=(u_int32_t*)(CSP0BASE+offset);
  *ptr1=data;

  return OK;
}


u_int32_t bus_r(u_int32_t offset) {
  u_int32_t *ptr1;
  
  ptr1=(u_int32_t*)(CSP0BASE+offset);
  return *ptr1;
}

// direct pattern output 
u_int32_t putout(char *s, int modnum) {
  int i;
  u_int32_t pat;
  int addr;

  if (strlen(s)<16) {
    fprintf(stdout," *** putout error: incorrect pattern length ***\n");
    fprintf(stdout," %s \n",s);
    return FAIL;
  }

  pat=0;
  for (i=0;i<16;i++) {
    if (s[i]=='1') pat=pat+(1<<i);
  }

  //addr=MCB_CNTRL_REG_OFF+(modnum<<4);
  addr=MCB_CNTRL_REG_OFF+(modnum<<SHIFTMOD);
  bus_w(addr, pat);
  return OK;
}


// read direct input 
u_int32_t readin(int modnum) {
  int addr;
  u_int32_t val;
  //addr=MCB_DOUT_REG_OFF+(modnum<<4);
  addr=MCB_DOUT_REG_OFF+(modnum<<SHIFTMOD);
  val=bus_r(addr) & 0x3ff;
  //  printf("reading 0x%08x, value 0x%08x\n",addr,val);
  return val;
}

u_int32_t setClockDivider(int d) {
  u_int32_t c;
  c=bus_r(SPEED_REG);
  bus_w(SPEED_REG,(d<<CLK_DIVIDER_OFFSET)|(c&~(CLK_DIVIDER_MASK))); 
  return ((bus_r(SPEED_REG)& CLK_DIVIDER_MASK)>>CLK_DIVIDER_OFFSET);    
}

u_int32_t getClockDivider() {
  u_int32_t clk_div;
  clk_div=((bus_r(SPEED_REG)&CLK_DIVIDER_MASK)>>CLK_DIVIDER_OFFSET);
  return clk_div;    
}

u_int32_t setSetLength(int d) {
  u_int32_t c;
  c=bus_r(SPEED_REG);
  bus_w(SPEED_REG,(d<<SET_LENGTH_OFFSET)|(c&~(SET_LENGTH_MASK))); 
  return ((bus_r(SPEED_REG)& SET_LENGTH_MASK)>>SET_LENGTH_OFFSET);    
}

u_int32_t getSetLength() {
  u_int32_t clk_div;
  clk_div=((bus_r(SPEED_REG)& SET_LENGTH_MASK)>>SET_LENGTH_OFFSET);   
  return clk_div;    
}


u_int32_t setWaitStates(int d1) {
  u_int32_t c;
  int d=d1-2;
  char cmd[100];
  sprintf(cmd,"bus -a 0xb0000000 -w 0xd000%1x",d1);
  c=bus_r(SPEED_REG);
  bus_w(SPEED_REG,(d<<WAIT_STATES_OFFSET)|(c&~(WAIT_STATES_MASK))); 
  system(cmd);
  return ((bus_r(SPEED_REG)& WAIT_STATES_MASK)>>WAIT_STATES_OFFSET);    
}

u_int32_t getWaitStates() {
  u_int32_t clk_div;
  clk_div=((bus_r(SPEED_REG)& WAIT_STATES_MASK)>>WAIT_STATES_OFFSET);
  return clk_div;    
}


u_int32_t setTotClockDivider(int d) {
  u_int32_t c;
  c=bus_r(SPEED_REG);
  bus_w(SPEED_REG,(d<<TOTCLK_DIVIDER_OFFSET)|(c&~(TOTCLK_DIVIDER_MASK))); 
  return ((bus_r(SPEED_REG)& TOTCLK_DIVIDER_MASK)>>TOTCLK_DIVIDER_OFFSET);    
}

u_int32_t getTotClockDivider() {
  u_int32_t clk_div;
  clk_div=((bus_r(SPEED_REG)&TOTCLK_DIVIDER_MASK)>>TOTCLK_DIVIDER_OFFSET);
  return clk_div;    
}


u_int32_t setExtSignal(int d, enum externalSignalFlag  mode) {
  
  int modes[]={-1,EXT_SIG_OFF, EXT_GATE_IN_ACTIVEHIGH, EXT_GATE_IN_ACTIVELOW,EXT_TRIG_IN_RISING,EXT_TRIG_IN_FALLING,EXT_RO_TRIG_IN_RISING, EXT_RO_TRIG_IN_FALLING,EXT_GATE_OUT_ACTIVEHIGH, EXT_GATE_OUT_ACTIVELOW, EXT_TRIG_OUT_RISING, EXT_TRIG_OUT_FALLING, EXT_RO_TRIG_OUT_RISING, EXT_RO_TRIG_OUT_FALLING};

  u_int32_t c;
  int off=d*SIGNAL_OFFSET;
  c=bus_r(EXT_SIGNAL_REG);
  if (mode<=RO_TRIGGER_OUT_FALLING_EDGE)
    bus_w(EXT_SIGNAL_REG,((modes[mode])<<off)|(c&~(SIGNAL_MASK<<off))); 


  return getExtSignal(d);
}


int getExtSignal(int d) {

  int modes[]={SIGNAL_OFF, GATE_IN_ACTIVE_HIGH, GATE_IN_ACTIVE_LOW,TRIGGER_IN_RISING_EDGE, TRIGGER_IN_FALLING_EDGE,RO_TRIGGER_IN_RISING_EDGE, RO_TRIGGER_IN_FALLING_EDGE, GATE_OUT_ACTIVE_HIGH,   GATE_OUT_ACTIVE_LOW, TRIGGER_OUT_RISING_EDGE, TRIGGER_OUT_FALLING_EDGE, RO_TRIGGER_OUT_RISING_EDGE,RO_TRIGGER_OUT_FALLING_EDGE};
    
    int off=d*SIGNAL_OFFSET;
    int mode=((bus_r(EXT_SIGNAL_REG)&(SIGNAL_MASK<<off))>>off);



    if (mode<RO_TRIGGER_OUT_FALLING_EDGE)
      return modes[mode];
    else 
      return -1;
    
}


u_int64_t  getMcsNumber() {
  
  FILE *fp=NULL;
  u_int64_t res;
  char line[150];
  int a[6];
  
  int n=0, i;
  //u_int64_t a0,a1,a2,a3,a4,a5,n=0;
  fp=fopen("/etc/conf.d/mac","r");
  if (fp==NULL) {
    printf("could not ope MAC file\n");;
    return -1;
  }
  while (fgets(line,150,fp)) {
    //MAC="00:40:8C:CD:00:00"
    printf(line);
    if (strstr(line,"MAC="))
      n=sscanf(line,"MAC=\"%x:%x:%x:%x:%x:%x\"",a+5,a+4,a+3,a+2,a+1,a);
  }
  fclose(fp);
  if (n!=6){ 
    printf("could not scan MAC address\n");;
    return -1;
  } 
  res=0;
  for (i=0; i<n; i++) {
    res=(res<<8)+a[n-1-i];
  }
  return res;
}

u_int32_t  getMcsVersion() {
  return bus_r(FPGA_VERSION_REG);
  //return MCSVERSION;
}

// for fpga test 
u_int32_t testFpga(void) {
  u_int32_t val;
  int result=OK;

  //fixed pattern
  val=bus_r(FIX_PATT_REG);
  if (val==FIXED_PATT_VAL) {
    printf("fixed pattern ok!! %x\n",val);
  } else {
    printf("fixed pattern wrong!! %x\n",val);
    result=FAIL;
    //    return FAIL;
  }
  //FPGA code version
  val=bus_r(FPGA_VERSION_REG)&0x00ffffff;
  if (val>=(FPGA_VERSION_VAL&0x00ffffff)) {
    printf("FPGA version ok!! %x\n",val);
  } else {
    printf("FPGA version too old! %x\n",val);
    return FAIL;
  }
  //dummy register
  val=0xF0F0F0F0;
  bus_w(DUMMY_REG, val);
  val=bus_r(DUMMY_REG);
  if (val==0xF0F0F0F0) {
    printf("FPGA dummy register ok!! %x\n",val);
  } else {
    printf("FPGA dummy register wrong!! %x\n",val);
    result=FAIL;
    //    return FAIL;
  }
  //dummy register
  val=0x0F0F0F0F;
  bus_w(DUMMY_REG, val);
  val=bus_r(DUMMY_REG);
  if (val==0x0F0F0F0F) {
    printf("FPGA dummy register ok!! %x\n",val);
  } else {
    printf("FPGA dummy register wrong!! %x\n",val);
    result=FAIL;
    //    return FAIL;
  }
  return result;
}

int getNModBoard() {
  int nmodboard;
  u_int32_t val;
  val=bus_r(FPGA_VERSION_REG)&0xff000000;
  nmodboard=val >> 24;
#ifdef VERY_VERBOSE
  printf("The board hosts %d modules\n",nmodboard); 
#endif
  nModBoard=nmodboard;
  //getNModBoard()=nmodboard;
  return nmodboard;
}

int setNMod(int n) {
  
  int fifo;
  int ifsta, ifsto, ifste;
  if (n>0 && n<=getNModBoard()) {
    nModX=n;
    dataBytes=nModX*nModY*NCHIP*NCHAN*dynamicRange/8;
    allocateRAM();
  }


  /* should enable all fifos*/
  bus_w(FIFO_CNTRL_REG_OFF+(ALLFIFO<<SHIFTMOD), FIFO_RESET_BIT | FIFO_DISABLE_TOGGLE_BIT); 

  /*disable the fifos relative to the unused modules */
  
  ifste=NCHAN*dynamicRange/32; 
  ifsta=nModX*NCHIP*ifste;
  ifsto=nModBoard*NCHIP*ifste;
 
  for (ififo=ifsta; ififo<ifsto; ififo+=ifste) {
    fifocntrl[ififo]=FIFO_DISABLE_TOGGLE_BIT;
  } 


  return nModX;
}


// fifo test 
u_int32_t  testFifos(void) {
  printf("Fifo test not implemented!\n");
  bus_w(CONTROL_REG, START_FIFOTEST_BIT); 
  return OK;
}



// program dacq settings 

int64_t set64BitReg(int64_t value, int aLSB, int aMSB){
  int64_t v64;
  u_int32_t vLSB,vMSB;
  if (value!=-1) {
    vLSB=value&(0xffffffff);
    bus_w(aLSB,vLSB);
    v64=value>> 32;
    vMSB=v64&(0xffffffff);
    bus_w(aMSB,vMSB);
  }
  return get64BitReg(aLSB, aMSB);

}

int64_t get64BitReg(int aLSB, int aMSB){
  int64_t v64;
  u_int32_t vLSB,vMSB;
  vLSB=bus_r(aLSB);
  vMSB=bus_r(aMSB);
  v64=vMSB;
  v64=(v64<<32) | vLSB;
  return v64;
}
  
int64_t setFrames(int64_t value){
  return set64BitReg(value,  SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG);
}

int64_t getFrames(){
  return get64BitReg(GET_FRAMES_LSB_REG, GET_FRAMES_MSB_REG);
}

int64_t setExposureTime(int64_t value){
  /* time is in ns */
  if (value!=-1) {
    value*=(1E-9*CLK_FREQ);
  }
  return set64BitReg(value,SET_EXPTIME_LSB_REG, SET_EXPTIME_MSB_REG)/(1E-9*CLK_FREQ);
}

int64_t getExposureTime(){
  return get64BitReg(GET_EXPTIME_LSB_REG, GET_EXPTIME_MSB_REG)/(1E-9*CLK_FREQ);   
}

int64_t setGates(int64_t value){
  return set64BitReg(value, SET_GATES_LSB_REG, SET_GATES_MSB_REG);
}

int64_t getGates(){
  return get64BitReg(GET_GATES_LSB_REG, GET_GATES_MSB_REG);
}

int64_t setPeriod(int64_t value){
  /* time is in ns */
  if (value!=-1) {
    value*=(1E-9*CLK_FREQ);
  }
  return set64BitReg(value,SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG)/(1E-9*CLK_FREQ);
}

int64_t getPeriod(){
  return get64BitReg(GET_PERIOD_LSB_REG, GET_PERIOD_MSB_REG)/(1E-9*CLK_FREQ);   
}

int64_t setDelay(int64_t value){
  /* time is in ns */
  if (value!=-1) {
    value*=(1E-9*CLK_FREQ);
  }
  return set64BitReg(value,SET_DELAY_LSB_REG, SET_DELAY_MSB_REG)/(1E-9*CLK_FREQ);
}

int64_t getDelay(){
  return get64BitReg(GET_DELAY_LSB_REG, GET_DELAY_MSB_REG)/(1E-9*CLK_FREQ);   
}

int64_t setTrains(int64_t value){
  return set64BitReg(value,  SET_TRAINS_LSB_REG, SET_TRAINS_MSB_REG);
}

int64_t getTrains(){
  return get64BitReg(GET_TRAINS_LSB_REG, GET_TRAINS_MSB_REG);
}


int64_t setProbes(int64_t value){
  int ow;
  switch (getDynamicRange()) {
  case 32:
    ow=1;
    break;
  case 16:
    ow=2;
    break;
  case 8:
    ow=4;
    break;
  case 4:
    ow=8;
    break;
  case 1:
    ow=5;
  default:
    ow=1;
  }
  
  if (value>=0) {
    setCSregister(ALLMOD);
    initChipWithProbes(0, ow,value, ALLMOD);
    putout("0000000000000000",ALLMOD);
  }
  return getProbes();
}

int64_t getProbes(){
  u_int32_t shiftin=bus_r(GET_SHIFT_IN_REG);
  u_int32_t np=(shiftin >>PROBES_OFF) & PROBES_MASK;
#ifdef VERYVERBOSE
  printf("%08x ",shiftin);
  printf("probes==%01x\n",np);
#endif
  
  return np;
    
}






u_int32_t runBusy(void) {
  return bus_r(STATUS_REG)&RUN_BUSY_BIT;
}

u_int32_t dataPresent(void) {
  return bus_r(LOOK_AT_ME_REG);
}

u_int32_t runState(void) {
  int s=bus_r(STATUS_REG);
#ifdef SHAREDMEMORY
  if (s&RUN_BUSY_BIT)
    write_status_sm("Running");
  else
    write_status_sm("Stopped");  
#endif
  return s;
}


// State Machine 

u_int32_t  startStateMachine(){
#ifdef VERBOSE
  printf("Starting State Machine\n");
#endif
  fifoReset(); 
  now_ptr=(char*)ram_values;
#ifdef SHAREDMEMORY
  write_stop_sm(0);
  write_status_sm("Started");
#endif
#ifdef MCB_FUNCS
  setCSregister(ALLMOD);
  clearSSregister(ALLMOD);
#endif
  putout("0000000000000000",ALLMOD);
  bus_w(CONTROL_REG, START_ACQ_BIT |  START_EXPOSURE_BIT); 
  return OK;
}




u_int32_t  stopStateMachine(){ 

#ifdef VERBOSE
  printf("Stopping State Machine\n");
#endif
#ifdef SHAREDMEMORY
  write_stop_sm(1);
  write_status_sm("Stopped");
#endif
  bus_w(CONTROL_REG, STOP_ACQ_BIT); 
  usleep(500);
  if (!runBusy())
    return OK;
  else
    return FAIL;    
}


u_int32_t startReadOut(){ 
  u_int32_t status;
#ifdef VERBOSE
  printf("Starting State Machine Readout\n");
#endif
  status=bus_r(STATUS_REG)&RUN_BUSY_BIT;
#ifdef DEBUG
  printf("State machine status is %08x\n",bus_r(STATUS_REG));
#endif
  bus_w(CONTROL_REG,  START_ACQ_BIT |START_READOUT_BIT);   //  start readout
  return OK;
}


// fifo routines 

u_int32_t fifoReset(void) {
#ifdef DEBUG
  printf("resetting fifo\n");
#endif
  bus_w(FIFO_CNTRL_REG_OFF+(ALLFIFO<<SHIFTMOD), FIFO_RESET_BIT); 
  return OK;
}


u_int32_t setNBits(u_int32_t n) {
  u_int32_t rval=0;
  rval=bus_w(SET_NBITS_REG, n);
  return bus_r(SET_NBITS_REG);
}

u_int32_t getNBits()
{
  return bus_r(SET_NBITS_REG);
}


u_int32_t fifoReadCounter(int fifonum)
{
  int rval=0;
  rval=bus_r(FIFO_COUNTR_REG_OFF+(fifonum<<SHIFTFIFO));
#ifdef VERBOSE
  printf("FIFO %d countains %x words\n",fifonum, rval);
#endif
  return rval;
}

u_int32_t  fifoReadStatus()
{
  // reads from the global status register
  
   return bus_r(STATUS_REG)&(SOME_FIFO_FULL_BIT | ALL_FIFO_EMPTY_BIT);
}

u_int32_t  fifo_full(void)
{
  // checks fifo empty flag returns 1 if fifo is empty
  //   otherwise 0
  return bus_r(STATUS_REG)&SOME_FIFO_FULL_BIT;
}


u_int32_t* fifo_read_event()
{

#ifdef VERBOSE
  int ichip;
#endif
#ifdef VIRTUAL
  return NULL;
#endif
  while (bus_r(LOOK_AT_ME_REG)==0) {
#ifdef VERYVERBOSE
    printf("Waiting for data\n");
#endif
    if (runBusy()==0) {
       if (bus_r(LOOK_AT_ME_REG)==0) {
#ifdef VERYVERBOSE
	 printf("no frame found\n");
	 for (ichip=0; ichip<nModBoard*NCHIP; ichip++) {
	   fifoReadCounter(ichip); 
	 }
#endif
	 return NULL;
       } else {
	 break;
       }
    }
   }
#ifdef VERYVERBOSE
  printf("before readout\n");
  for (ichip=0; ichip<nModBoard*NCHIP; ichip++) {
    fifoReadCounter(ichip); 
  }
#endif
  memcpy(now_ptr, values, dataBytes);
#ifdef VERYVERBOSE
  printf("after readout\n");
  for (ichip=0; ichip<nModBoard*NCHIP; ichip++) {
    fifoReadCounter(ichip); 
  }
#endif
  if (storeInRAM>0) {
    now_ptr+=dataBytes;
  }
  return ram_values;
  
}



u_int32_t* decode_data(int *datain)
{
  int *dataout;
  const char one=1;
  const int bytesize=8;
  char *ptr=(char*)datain;
  //int nbits=dynamicRange;
  int  ipos=0, ichan=0;;
  //int nch, boff=0;
  int ibyte, ibit;
  char iptr;

  dataout=malloc(nChans*nChips*nModX*4);
  ichan=0;
  switch (dynamicRange) {
  case 1:
    for (ibyte=0; ibyte<dataBytes; ibyte++) {
      iptr=ptr[ibyte];
      for (ipos=0; ipos<bytesize; ipos++) {
	//	dataout[ibyte*2+ichan]=((iptr&((0xf)<<ichan))>>ichan)&0xf;
	dataout[ichan]=(iptr>>(ipos))&0x1;
	ichan++;
      }
    }
    break;
  case 4:
    for (ibyte=0; ibyte<dataBytes; ibyte++) {
      iptr=ptr[ibyte]&0xff;
      for (ipos=0; ipos<2; ipos++) {
	//	dataout[ibyte*2+ichan]=((iptr&((0xf)<<ichan))>>ichan)&0xf;
	dataout[ichan]=(iptr>>(ipos*4))&0xf;
	ichan++;
      }
    }
    break;
  case 8:
    for (ichan=0; ichan<dataBytes; ichan++) {
      dataout[ichan]=ptr[ichan]&0xff;
    }
    break;
  case 16:
    for (ichan=0; ichan<nChans*nChips*nModX; ichan++) {
      dataout[ichan]=0;
      for (ibyte=0; ibyte<2; ibyte++) {
	iptr=ptr[ichan*2+ibyte];
	dataout[ichan]|=((iptr<<(ibyte*bytesize))&(0xff<<(ibyte*bytesize)));
      }
    }
    break;
  default:    
    for (ichan=0; ichan<nChans*nChips*nModX; ichan++)
      dataout[ichan]=datain[ichan];//&0xffffff;
  }
  
#ifdef VERBOSE
  printf("decoded %d  channels\n",ichan);
#endif
  return dataout;
}



int setDynamicRange(int dr) {




  int ow;
  u_int32_t np=getProbes();
#ifdef VERYVERBOSE
  printf("probes==%02x\n",np);
#endif
  
  
  if (dr>0) {
    if (dr<=1) {
      dynamicRange=1;
      ow=5;
    }    else if (dr<=4) {
      dynamicRange=4;
      ow=4;
    }    else if (dr<=8) {
      dynamicRange=8;
      ow=3;
    } else if (dr<=16) {
      dynamicRange=16;
      ow=2;
    }    else {
      dynamicRange=32;
      ow=0; //or 1?
    }
    setCSregister(ALLMOD);
    initChipWithProbes(0, ow,np, ALLMOD);
    putout("0000000000000000",ALLMOD);
    
  }
  

  return   getDynamicRange();
}






int getDynamicRange() {
  int dr;
  u_int32_t shiftin=bus_r(GET_SHIFT_IN_REG);
  u_int32_t outmux=(shiftin >> OUTMUX_OFF) & OUTMUX_MASK;
#ifdef VERYVERBOSE
  printf("%08x ",shiftin);
  printf("outmux==%02x\n",outmux);
#endif
  
  switch (outmux) {
  case 2:
    dr=16;
    break;
  case 4:
    dr=8;
    break;
  case 8:
    dr=4;
    break;
  case 16:
    dr=1;
    break;
  default:
    dr=32;
  }
  dynamicRange=dr;
  dataBytes=nModX*nModY*NCHIP*NCHAN*dynamicRange/8;
  if (allocateRAM()==OK) {
      ;
  } else
    printf("ram not allocated\n");

  return dynamicRange;

}

int testBus() {
  int j;
  char cmd[100];
  u_int32_t val;
  printf("%s\n",cmd);
  system(cmd);
  for (j=0; j<1000000; j++) {
    val=bus_r(FIX_PATT_REG);
    if (val!=0xacdc1980){
      printf("%d %x\n",j, val);
      return FAIL;
    }
  }
  return OK;
}


int setStoreInRAM(int b) {
  if (b)
    storeInRAM=1;
  else
    storeInRAM=0;
  return  allocateRAM();
}


int allocateRAM() {
  size_t size;

  // ret=clearRAM();
  if (storeInRAM) {
    size=dataBytes*setFrames(-1);
#ifdef VERBOSE
    //   printf("nmodx=%d nmody=%d dynamicRange=%d dataBytes=%d nFrames=%d size=%d\n",nModX,nModY,dynamicRange,dataBytes,setFrameNumber(-1),size );
#endif
    if (size<dataBytes)
      size=dataBytes;
  }  else 
    size=dataBytes;

  if (size==ram_size)
    return OK;
    
  

#ifdef VERBOSE
    printf("reallocating ram %x\n",ram_values);
#endif
    //  clearRAM();
    // ram_values=malloc(size);
    ram_values=realloc(ram_values,size);

  if (ram_values) {
    now_ptr=(char*)ram_values;
#ifdef VERBOSE
    printf("ram allocated 0x%x of size %d to %x\n",now_ptr, size, now_ptr+size);
#endif
    ram_size=size;
    return OK;
  } else {
    printf("could not allocate %d bytes\n",size);
    if (storeInRAM==1) {
      printf("retrying\n");
      storeInRAM=0;
      size=dataBytes;
      ram_values=realloc(ram_values,size);
      if (ram_values==NULL)
	printf("Fatal error: there must be a memory leak somewhere! You can't allocate even one frame!\n");
      else {
	now_ptr=(char*)ram_values;
#ifdef VERBOSE
	printf("ram allocated 0x%x of size %d to %x\n",now_ptr, size, now_ptr+size);
#endif
      }
    } else {
      printf("Fatal error: there must be a memory leak somewhere! You can't allocate even one frame!\n");
    }
    return FAIL;
  }


    
}



int clearRAM() {
  if (ram_values) {
    //#ifdef VERBOSE
    //printf("clearing RAM 0x%x\n", ram_values);
    //#endif
    free(ram_values);
    ram_values=NULL;
    now_ptr=NULL;
  }
  //#ifdef VERBOSE
  //printf("done 0x%x\n", ram_values);
  //#endif
  return OK;
}
