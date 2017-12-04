#ifndef PICASSOD
#include "server_defs.h"
#else
#include "picasso_defs.h"
#endif

#include "firmware_funcs.h"
#include "mcb_funcs.h"
#include "registers.h"

#ifdef SHAREDMEMORY
#include "sharedmemory.h"
#endif

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include <stdlib.h>

//for memory mapping
u_int64_t CSP0BASE;

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

int64_t totalTime=1;
u_int32_t progressMask=0;



int ififostart, ififostop, ififostep, ififo;

int masterMode=NO_MASTER, syncMode=NO_SYNCHRONIZATION, timingMode=AUTO_TIMING;

enum externalSignalFlag signals[4]={GATE_IN_ACTIVE_HIGH, TRIGGER_IN_RISING_EDGE, SIGNAL_OFF, SIGNAL_OFF};


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
  CSP0BASE = malloc(MEM_SIZE);
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
#ifndef PICASSOD 
  val=bus_r(addr) & 0x3ff;   //anna (MYTHEN: reads direct input for 10 chips per module)
#else
  val=bus_r(addr) & 0xfff;  //frances. despite changed, errors still in the ShiftStSel, shiftIn and shiftOut. 
#endif
    

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
  //int d=d1-3;
  char cmd[100];
  if (d1<=0xf) {
    sprintf(cmd,"bus -a 0xb0000000 -w 0x%x0008",d1);
    c=bus_r(SPEED_REG);
    bus_w(SPEED_REG,(d<<WAIT_STATES_OFFSET)|(c&~(WAIT_STATES_MASK))); 
    system(cmd);
  }
  return ((bus_r(SPEED_REG)& WAIT_STATES_MASK)>>WAIT_STATES_OFFSET)+2;    
}

u_int32_t getWaitStates() {
  u_int32_t clk_div;
  clk_div=((bus_r(SPEED_REG)& WAIT_STATES_MASK)>>WAIT_STATES_OFFSET);
  return clk_div+2;    
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


u_int32_t setTotDutyCycle(int d) {
  u_int32_t c;
  c=bus_r(SPEED_REG);
  bus_w(SPEED_REG,(d<<TOTCLK_DUTYCYCLE_OFFSET)|(c&~(TOTCLK_DUTYCYCLE_MASK))); 
  return ((bus_r(SPEED_REG)& TOTCLK_DUTYCYCLE_MASK)>>TOTCLK_DUTYCYCLE_OFFSET);    
}

u_int32_t getTotDutyCycle() {
  u_int32_t clk_div;
  clk_div=((bus_r(SPEED_REG)&TOTCLK_DUTYCYCLE_MASK)>>TOTCLK_DUTYCYCLE_OFFSET);
  return clk_div;    
}


u_int32_t setExtSignal(int d, enum externalSignalFlag  mode) {
  

  //  int modes[]={EXT_SIG_OFF, EXT_GATE_IN_ACTIVEHIGH, EXT_GATE_IN_ACTIVELOW,EXT_TRIG_IN_RISING,EXT_TRIG_IN_FALLING,EXT_RO_TRIG_IN_RISING, EXT_RO_TRIG_IN_FALLING,EXT_GATE_OUT_ACTIVEHIGH, EXT_GATE_OUT_ACTIVELOW, EXT_TRIG_OUT_RISING, EXT_TRIG_OUT_FALLING, EXT_RO_TRIG_OUT_RISING, EXT_RO_TRIG_OUT_FALLING};

  u_int32_t c;
  //  int off=d*SIGNAL_OFFSET;
  c=bus_r(EXT_SIGNAL_REG);



  if (d>=0 && d<4) {
    signals[d]=mode;
#ifdef VERBOSE
    printf("settings signal variable number %d to value %04x\n", d, signals[d]);
#endif 

 // if output signal, set it!

    switch (mode) {
    case GATE_IN_ACTIVE_HIGH: 
    case GATE_IN_ACTIVE_LOW:
      if (timingMode==GATE_FIX_NUMBER || timingMode==GATE_WITH_START_TRIGGER)
	setFPGASignal(d,mode);	
      else 
	setFPGASignal(d,SIGNAL_OFF);
      break;
    case TRIGGER_IN_RISING_EDGE:
    case TRIGGER_IN_FALLING_EDGE:
      if (timingMode==TRIGGER_EXPOSURE || timingMode==GATE_WITH_START_TRIGGER)
	setFPGASignal(d,mode);	
      else 
	setFPGASignal(d,SIGNAL_OFF);
      break;
    case RO_TRIGGER_IN_RISING_EDGE:
    case RO_TRIGGER_IN_FALLING_EDGE:
      if (timingMode==TRIGGER_READOUT)
	setFPGASignal(d,mode);	
      else 
	setFPGASignal(d,SIGNAL_OFF);
      break;
    case MASTER_SLAVE_SYNCHRONIZATION:
      setSynchronization(syncMode);
      break;
    default:
      setFPGASignal(d,mode);
    }
    
    setTiming(GET_EXTERNAL_COMMUNICATION_MODE);
  }

  return getExtSignal(d);
}

u_int32_t setFPGASignal(int d, enum externalSignalFlag  mode) {
  

  int modes[]={SIGNAL_OFF, GATE_IN_ACTIVE_HIGH, GATE_IN_ACTIVE_LOW,TRIGGER_IN_RISING_EDGE, TRIGGER_IN_FALLING_EDGE,RO_TRIGGER_IN_RISING_EDGE, RO_TRIGGER_IN_FALLING_EDGE, GATE_OUT_ACTIVE_HIGH,   GATE_OUT_ACTIVE_LOW, TRIGGER_OUT_RISING_EDGE, TRIGGER_OUT_FALLING_EDGE, RO_TRIGGER_OUT_RISING_EDGE,RO_TRIGGER_OUT_FALLING_EDGE};
    
  // int modes[]={EXT_SIG_OFF, EXT_GATE_IN_ACTIVEHIGH, EXT_GATE_IN_ACTIVELOW,EXT_TRIG_IN_RISING,EXT_TRIG_IN_FALLING,EXT_RO_TRIG_IN_RISING, EXT_RO_TRIG_IN_FALLING,EXT_GATE_OUT_ACTIVEHIGH, EXT_GATE_OUT_ACTIVELOW, EXT_TRIG_OUT_RISING, EXT_TRIG_OUT_FALLING, EXT_RO_TRIG_OUT_RISING, EXT_RO_TRIG_OUT_FALLING};

  u_int32_t c;
  int off=d*SIGNAL_OFFSET;
  c=bus_r(EXT_SIGNAL_REG);


  if (mode<=RO_TRIGGER_OUT_FALLING_EDGE &&  mode>=0) {
#ifdef VERBOSE
    printf("writing signal register number %d mode %04x\n",d, modes[mode]);
#endif
    bus_w(EXT_SIGNAL_REG,((modes[mode])<<off)|(c&~(SIGNAL_MASK<<off)));
  }
  return getExtSignal(d);
}


int getExtSignal(int d) {

/*   int modes[]={SIGNAL_OFF, GATE_IN_ACTIVE_HIGH, GATE_IN_ACTIVE_LOW,TRIGGER_IN_RISING_EDGE, TRIGGER_IN_FALLING_EDGE,RO_TRIGGER_IN_RISING_EDGE, RO_TRIGGER_IN_FALLING_EDGE, GATE_OUT_ACTIVE_HIGH,   GATE_OUT_ACTIVE_LOW, TRIGGER_OUT_RISING_EDGE, TRIGGER_OUT_FALLING_EDGE, RO_TRIGGER_OUT_RISING_EDGE,RO_TRIGGER_OUT_FALLING_EDGE}; */
    
/*     int off=d*SIGNAL_OFFSET; */
/*     int mode=((bus_r(EXT_SIGNAL_REG)&(SIGNAL_MASK<<off))>>off); */

/*     if (mode<RO_TRIGGER_OUT_FALLING_EDGE) */
/*       return modes[mode]; */
/*     else  */
/*       return -1; */
    

  if (d>=0 && d<4) {
#ifdef VERBOSE
    printf("gettings signal variable number %d  value %04x\n", d, signals[d]);
#endif
    return signals[d];
  } else
    return -1;

}


int getFPGASignal(int d) {

  int modes[]={SIGNAL_OFF, GATE_IN_ACTIVE_HIGH, GATE_IN_ACTIVE_LOW,TRIGGER_IN_RISING_EDGE, TRIGGER_IN_FALLING_EDGE,RO_TRIGGER_IN_RISING_EDGE, RO_TRIGGER_IN_FALLING_EDGE, GATE_OUT_ACTIVE_HIGH,   GATE_OUT_ACTIVE_LOW, TRIGGER_OUT_RISING_EDGE, TRIGGER_OUT_FALLING_EDGE, RO_TRIGGER_OUT_RISING_EDGE,RO_TRIGGER_OUT_FALLING_EDGE};
    
    int off=d*SIGNAL_OFFSET;
    int mode=((bus_r(EXT_SIGNAL_REG)&(SIGNAL_MASK<<off))>>off);

    if (mode<=RO_TRIGGER_OUT_FALLING_EDGE) {
      if (modes[mode]!=SIGNAL_OFF && signals[d]!=MASTER_SLAVE_SYNCHRONIZATION)
	signals[d]=modes[mode];
#ifdef VERBOSE
      printf("gettings signal register number %d  value %04x\n", d, modes[mode]);
#endif
      return modes[mode];
    } else
      return -1;
    
}


/*
enum externalCommunicationMode{
  GET_EXTERNAL_COMMUNICATION_MODE,
  AUTO,
  TRIGGER_EXPOSURE_SERIES,
  TRIGGER_EXPOSURE_BURST,
  TRIGGER_READOUT,
  TRIGGER_COINCIDENCE_WITH_INTERNAL_ENABLE,
  GATE_FIX_NUMBER,
  GATE_FIX_DURATION,
  GATE_WITH_START_TRIGGER,
  GATE_COINCIDENCE_WITH_INTERNAL_ENABLE
};
*/


int setTiming(int ti) {


  int ret=GET_EXTERNAL_COMMUNICATION_MODE;

  int g=-1, t=-1, rot=-1;

  int i;
  printf("*********************************Setting timing mode %d!\n", ti);
  switch (ti) {
  case AUTO_TIMING:  
    timingMode=ti;
    // disable all gates/triggers in except if used for master/slave synchronization
    for (i=0; i<4; i++) {
      if (getFPGASignal(i)>0 && getFPGASignal(i)<GATE_OUT_ACTIVE_HIGH && signals[i]!=MASTER_SLAVE_SYNCHRONIZATION)
	setFPGASignal(i,SIGNAL_OFF);
    }
    break;
    
  case   TRIGGER_EXPOSURE: 
    timingMode=ti;
    // if one of the signals is configured to be trigger, set it and unset possible gates
    for (i=0; i<4; i++) {
      if (signals[i]==TRIGGER_IN_RISING_EDGE ||  signals[i]==TRIGGER_IN_FALLING_EDGE)
	setFPGASignal(i,signals[i]);
      else if (signals[i]==GATE_IN_ACTIVE_HIGH || signals[i]==GATE_IN_ACTIVE_LOW)
	setFPGASignal(i,SIGNAL_OFF);
      else if (signals[i]==RO_TRIGGER_IN_RISING_EDGE ||  signals[i]==RO_TRIGGER_IN_FALLING_EDGE)
	setFPGASignal(i,SIGNAL_OFF);
	
    }
    break;
    
    

  case  TRIGGER_READOUT: 
    timingMode=ti;
    // if one of the signals is configured to be trigger, set it and unset possible gates
    for (i=0; i<4; i++) {
      if (signals[i]==RO_TRIGGER_IN_RISING_EDGE ||  signals[i]==RO_TRIGGER_IN_FALLING_EDGE)
	setFPGASignal(i,signals[i]);
      else if (signals[i]==GATE_IN_ACTIVE_HIGH || signals[i]==GATE_IN_ACTIVE_LOW)
	setFPGASignal(i,SIGNAL_OFF);
      else if (signals[i]==TRIGGER_IN_RISING_EDGE ||  signals[i]==TRIGGER_IN_FALLING_EDGE)
	setFPGASignal(i,SIGNAL_OFF);
    }
    break;
    
  case GATE_FIX_NUMBER: 
    timingMode=ti;
    printf("*********************************Setting gating!\n");
    // if one of the signals is configured to be trigger, set it and unset possible gates
    for (i=0; i<4; i++) {
      if (signals[i]==RO_TRIGGER_IN_RISING_EDGE ||  signals[i]==RO_TRIGGER_IN_FALLING_EDGE)
	setFPGASignal(i,SIGNAL_OFF);
      else if (signals[i]==GATE_IN_ACTIVE_HIGH || signals[i]==GATE_IN_ACTIVE_LOW)
	setFPGASignal(i,signals[i]);
      else if (signals[i]==TRIGGER_IN_RISING_EDGE ||  signals[i]==TRIGGER_IN_FALLING_EDGE)
	setFPGASignal(i,SIGNAL_OFF);
    }
    break;
    


  case GATE_WITH_START_TRIGGER: 
    timingMode=ti;
    for (i=0; i<4; i++) {
      if (signals[i]==RO_TRIGGER_IN_RISING_EDGE ||  signals[i]==RO_TRIGGER_IN_FALLING_EDGE)
	setFPGASignal(i,SIGNAL_OFF);
      else if (signals[i]==GATE_IN_ACTIVE_HIGH || signals[i]==GATE_IN_ACTIVE_LOW)
	setFPGASignal(i,signals[i]);
      else if (signals[i]==TRIGGER_IN_RISING_EDGE ||  signals[i]==TRIGGER_IN_FALLING_EDGE)
	setFPGASignal(i,signals[i]);
    }
    break;
    
  default:
    ;

  }
  


  for (i=0; i<4; i++) {
    if (signals[i]!=MASTER_SLAVE_SYNCHRONIZATION) {
      if (getFPGASignal(i)==RO_TRIGGER_IN_RISING_EDGE ||  getFPGASignal(i)==RO_TRIGGER_IN_FALLING_EDGE)
	rot=i;
      else if (getFPGASignal(i)==GATE_IN_ACTIVE_HIGH || getFPGASignal(i)==GATE_IN_ACTIVE_LOW)
	g=i;
      else if (getFPGASignal(i)==TRIGGER_IN_RISING_EDGE ||  getFPGASignal(i)==TRIGGER_IN_FALLING_EDGE)
	t=i;
    }
  }


  if (g>=0 && t>=0 && rot<0) {
    ret=GATE_WITH_START_TRIGGER;
  } else if (g<0 && t>=0 && rot<0) {
    ret=TRIGGER_EXPOSURE;
  } else if (g>=0 && t<0 && rot<0) {
    ret=GATE_FIX_NUMBER;
  } else if (g<0 && t<0 && rot>0) {
    ret=TRIGGER_READOUT;
  } else if (g<0 && t<0 && rot<0) {
    ret=AUTO_TIMING;
  }

  // timingMode=ret;

  return ret;

}




int setConfigurationRegister(int d) {
#ifdef VERBOSE
  printf("Setting configuration register to %x",d);
#endif
  if (d>=0) {
    bus_w(CONFIG_REG,d);
  }
#ifdef VERBOSE
  printf("configuration register is %x", bus_r(CONFIG_REG));
#endif
  return bus_r(CONFIG_REG);
}

int setToT(int d) {
  // int ret=0;
 int reg;
#ifdef VERBOSE
  printf("Setting ToT to %d\n",d);
#endif
  reg=bus_r(CONFIG_REG);
#ifdef VERBOSE
  printf("Before: ToT is %x\n", reg);
#endif
  if (d>0) {
    bus_w(CONFIG_REG,reg|TOT_ENABLE_BIT);
  } else if (d==0) {
    bus_w(CONFIG_REG,reg&(~TOT_ENABLE_BIT));
  } 
  reg=bus_r(CONFIG_REG);
#ifdef VERBOSE
  printf("ToT is %x\n", reg);
#endif
  if (reg&TOT_ENABLE_BIT)
    return 1;
  else
    return 0;
}

int setContinousReadOut(int d) {
  // int ret=0;
 int reg;
#ifdef VERBOSE
  printf("Setting Continous readout to %d\n",d);
#endif
  reg=bus_r(CONFIG_REG);
#ifdef VERBOSE
  printf("Before: Continous readout is %x\n", reg);
#endif
  if (d>0) {
    bus_w(CONFIG_REG,reg|CONT_RO_ENABLE_BIT);
  } else if (d==0) {
    bus_w(CONFIG_REG,reg&(~CONT_RO_ENABLE_BIT));
  } 
  reg=bus_r(CONFIG_REG);
#ifdef VERBOSE
  printf("Continous readout is %x\n", reg);
#endif
  if (reg&CONT_RO_ENABLE_BIT)
    return 1;
  else
    return 0;
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
  //while (1) {
  //fixed pattern
  val=bus_r(FIX_PATT_REG);
  if (val==FIXED_PATT_VAL) {
    printf("fixed pattern ok!! %08x\n",val);
  } else {
    printf("fixed pattern wrong!! %08x\n",val);
    result=FAIL;
    //    return FAIL;
  }
  //FPGA code version
  val=bus_r(FPGA_VERSION_REG)&0x00ffffff;
  if (val>=(FPGA_VERSION_VAL&0x00ffffff)) {
    printf("FPGA version ok!! %06x\n",val);
  } else {
    printf("FPGA version too old! %06x\n",val);
    return FAIL;
  }
  //dummy register
  val=0xF0F0F0F0;
  bus_w(DUMMY_REG, val);
  val=bus_r(DUMMY_REG);
  if (val==0xF0F0F0F0) {
    printf("FPGA dummy register ok!! %x\n",val);
  } else {
    printf("FPGA dummy register wrong!! %x instead of 0xF0F0F0F0 \n",val);
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
    printf("FPGA dummy register wrong!! %x instead of 0x0F0F0F0F \n",val);
    result=FAIL;
    //    return FAIL;
  }
  //  }
  return result;
}


// for fpga test 
u_int32_t testRAM(void) {
  int result=OK;
  int i=0;
  allocateRAM();
  //  while(i<100000) {
    memcpy(ram_values, values, dataBytes);
    printf ("%d: copied fifo %x to memory %x size %d\n",i++, values, ram_values, dataBytes);
    // }
  return result;
}

int getNModBoard() {
  int nmodboard;
  u_int32_t val;
  val=bus_r(FPGA_VERSION_REG)&0xff000000;
  
  //  printf("version register %08x\n",val);
  nmodboard=val >> 24;
  #ifdef VERY_VERBOSE
  printf("The board hosts %d modules\n",nmodboard); 
  #endif
  nModBoard=nmodboard;
  //getNModBoard()=nmodboard;
  return nmodboard;
}

int setNMod(int n) {
  
  //  int fifo;
  // int ifsta, ifsto, ifste;
  int imod;
  int rval;
  int reg;
  int nf=0;
  int shiftfifo=SHIFTFIFO;
  int ntot=getNModBoard();
  if (getProbes()==0) {
    switch (dynamicRange) {
    case 16:
      shiftfifo=SHIFTFIFO-1;
      break;
    case 8:
      shiftfifo=SHIFTFIFO-2;
      break;
    case 4:
      shiftfifo=SHIFTFIFO-3;
      break;
    case 1:
      shiftfifo=SHIFTFIFO-5;
      break;
    default:
      shiftfifo=SHIFTFIFO;
    }
  } else
    shiftfifo=SHIFTFIFO;


#ifdef VERBOSE
  printf("SetNMod called arg %d -- dr %d shiftfifo %d\n",n,dynamicRange,shiftfifo);
#endif
  if (n>=0 && n<=ntot) {
    nModX=n;
 
  /*d isable the fifos relative to the unused modules */
      for (ififo=0; ififo<ntot*NCHIP; ififo++) {
	reg=bus_r(FIFO_COUNTR_REG_OFF+(ififo<<shiftfifo));
      if (ififo<n*NCHIP) {
	if (reg&FIFO_DISABLED_BIT) {
	  bus_w(FIFO_CNTRL_REG_OFF+(ififo<<shiftfifo), FIFO_DISABLE_TOGGLE_BIT); 
#ifdef VERBOSE
	  if (bus_r(FIFO_COUNTR_REG_OFF+(ififo<<shiftfifo))&FIFO_DISABLED_BIT) {
	    printf("Fifo %d is %x (nm %d nc %d addr %08x)",ififo,reg, (reg&FIFO_NM_MASK)>>FIFO_NM_OFF, (reg&FIFO_NC_MASK)>>FIFO_NC_OFF, FIFO_COUNTR_REG_OFF+(ififo<<shiftfifo));
	    printf(" enabling  %08x\n",bus_r(FIFO_COUNTR_REG_OFF+(ififo<<shiftfifo)));
	  }
#endif
	}
	//#ifdef VERBOSE
	//else printf(" unmodified ",ififo,reg);
	//#endif

      } else {
	if ((reg&FIFO_ENABLED_BIT)) {
	  bus_w(FIFO_CNTRL_REG_OFF+(ififo<<shiftfifo), FIFO_DISABLE_TOGGLE_BIT); 
#ifdef VERBOSE
	  if ((bus_r(FIFO_COUNTR_REG_OFF+(ififo<<shiftfifo))&FIFO_ENABLED_BIT))  { 
	    printf("Fifo %d is %x (nm %d nc %d addr %08x)",ififo,reg, (reg&FIFO_NM_MASK)>>FIFO_NM_OFF, (reg&FIFO_NC_MASK)>>FIFO_NC_OFF, FIFO_COUNTR_REG_OFF+(ififo<<shiftfifo));
	    printf(" disabling %08x\n",bus_r(FIFO_COUNTR_REG_OFF+(ififo<<shiftfifo)));
	  }
#endif
	}
	//#ifdef VERBOSE
	//else printf(" unmodified ",ififo,reg);
	//#endif   
      } 
      //#ifdef VERBOSE
      //printf(" done %x\n",bus_r(FIFO_COUNTR_REG_OFF+(ififo<<shiftfifo)));
      //#endif
    } 
  }
  // ifste=dynamicRange/32; 
  nModX=0;
  nf=0;
  for (imod=0; imod<ntot; imod++) {
    rval=0;
    for (ififo=imod*NCHIP; ififo<(imod+1)*NCHIP; ififo++) {
      bus_w(FIFO_CNTRL_REG_OFF+(ififo<<shiftfifo), FIFO_RESET_BIT); 
#ifdef VERBOSE
      printf("%08x ",(bus_r(FIFO_COUNTR_REG_OFF+(ififo<<shiftfifo))));
#endif
      if ((bus_r(FIFO_COUNTR_REG_OFF+(ififo<<shiftfifo))&FIFO_ENABLED_BIT)){
	rval=1; // checks if at least one fifo of the module is enabled
#ifdef VERBOSE
	printf("Fifo %d is enabled\n",ififo);
#endif
	nf++;
      }
#ifdef VERBOSE
	else printf("Fifo %d is disabled\n",ififo);
#endif
    } 
    if (rval) {
      nModX++;
#ifdef VERBOSE
      printf("Module %d is enabled --total %d (%d fifos)\n",imod,nModX,nf );
#endif
    }
#ifdef VERBOSE 
    else printf("Module %d is disabled --total %d (%d fifos)\n",imod,nModX,nf );
#endif
  }
#ifdef VERBOSE
  printf("There are %d modules enabled (%d fifos)\n",nModX, nf);
#endif
  getDynamicRange();

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
  vMSB=bus_r(aMSB);
  vLSB=bus_r(aLSB);
  v64=vMSB;
  v64=(v64<<32) | vLSB;
#ifdef VERBOSE
  printf("MSB %08x LSB %08x, %016llx\n", vMSB, vLSB, v64);
#endif

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
  if (value!=-1) 
    value*=(1E-9*CLK_FREQ);
  
  if (masterMode==IS_SLAVE && syncMode==MASTER_GATES)
    setGates(1);

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
  int nm=setNMod(-1);
  switch (getDynamicRange()) {
  case 32:
    ow=1;
    break;
  case 16:
    ow=2;
    break;
  case 8:
    ow=3;
    break;
  case 4:
    ow=4;
    break;
  case 1:
    ow=5;
    break;
  default:
    ow=1;
  }
  if (value>=0) {
    setCSregister(ALLMOD);
    initChipWithProbes(0, ow,value, ALLMOD);
    putout("0000000000000000",ALLMOD);
    setNMod(nm);
    getDynamicRange(); // needed to change dataBytes 
  }
  return getProbes();
}


int64_t setProgress() {

  //????? eventually call after setting the registers
  return 100;
}


int64_t getProgress() {


  //should be done in firmware!!!!
  return 0;

}


int64_t getActualTime(){
  return get64BitReg(GET_ACTUAL_TIME_LSB_REG, GET_ACTUAL_TIME_MSB_REG)/(1E-9*CLK_FREQ);  
}

int64_t getMeasurementTime(){
  int64_t v=get64BitReg(GET_MEASUREMENT_TIME_LSB_REG, GET_MEASUREMENT_TIME_MSB_REG);
  int64_t mask=0x8000000000000000;

  if (v & mask ) {
#ifdef VERBOSE
    printf("no measurement time left\n");
#endif
    return -1E+9;
  } else
    return v/(1E-9*CLK_FREQ);
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


int setDACRegister(int idac, int val, int imod) {
  u_int32_t addr, reg, mask;
  int off;
#ifdef VERBOSE
  printf("Settings dac %d module %d register to %d\n",idac,imod,val);
#endif
   if ((idac%2)==0) {
     addr=MOD_DACS1_REG;
   } else {
     addr=MOD_DACS2_REG;
   }
   off=(idac%3)*10;
   mask=~((0x3ff)<<off);

   if (val>=0 && val<DAC_DR) {
     reg=bus_r(addr+(imod<<SHIFTMOD));
     reg&=mask;
     reg|=(val<<off);
     bus_w(addr+(imod<<SHIFTMOD),reg);
   }
   val=(bus_r(addr+(imod<<SHIFTMOD))>>off)&0x3ff;
#ifdef VERBOSE
  printf("Dac %d module %d register is %d\n",idac,imod,val);
#endif
   return val;
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

  write_istatus_sm(s);

#endif
#ifdef VERBOSE
  printf("status %08x\n",s);
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
  int shiftfifo;
  switch (dynamicRange) {
  case 16:
    shiftfifo=SHIFTFIFO-1;
    break;
  case 8:
    shiftfifo=SHIFTFIFO-2;
    break;
  case 4:
    shiftfifo=SHIFTFIFO-3;
    break;
  case 1:
    shiftfifo=SHIFTFIFO-5;
    break;
  default:
    shiftfifo=SHIFTFIFO;
  }

  rval=bus_r(FIFO_COUNTR_REG_OFF+(fifonum<<shiftfifo));
#ifdef VERBOSE
  //printf("FIFO %d contains %x words\n",fifonum, rval);
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

  int ir=0;

#ifdef VERBOSE
  int ichip;
  int ichan;
#endif
#ifdef VIRTUAL
  return NULL;
#endif
#ifdef VERYVERBOSE
  printf("before looping\n");
  for (ichip=0; ichip<nModBoard*NCHIP; ichip++) {
    if ((fifoReadCounter(ichip)&FIFO_COUNTER_MASK)%128)
      printf("FIFO %d contains %d words\n",ichip,(fifoReadCounter(ichip)&FIFO_COUNTER_MASK)); 
  }
#endif 
  while(bus_r(LOOK_AT_ME_REG)==0) {
#ifdef VERYVERBOSE
    printf("Waiting for data status %x\n",runState());
#endif
    if (runBusy()==0) {
     /*  for (ir=0; ir<100; ir++) { */
/* 	//usleep(100); */
/* 	//printf("check %d\n", ir);  */
/* 	if (runBusy()==1)  */
/* 	  break; */
/*       } */
/*       if (ir==100) { */
	if (bus_r(LOOK_AT_ME_REG)==0) {
	 //#ifdef VERBOSE
	  printf("no frame found - exiting ");
	  
	  printf("%08x %08x\n", runState(), bus_r(LOOK_AT_ME_REG)); 
	  //  for (ichip=0; ichip<nModBoard*NCHIP; ichip++) {
	    //  if ((fifoReadCounter(ichip)&FIFO_COUNTER_MASK)%128)
	  // printf("FIFO %d contains %d words\n",ichip,(fifoReadCounter(ichip)&FIFO_COUNTER_MASK)); 
	  // }
	  
	  //#endif
	  return NULL;
	} else {
#ifdef VERYVERBOSE
	  printf("no frame found %x status %x\n", bus_r(LOOK_AT_ME_REG),runState());
#endif
	  break;
	}
    /*   } else */
/* 	printf("run busy error workaround %d \n", ir); */
    }
   }
#ifdef VERYVERBOSE
  printf("before readout %08x %08x\n", runState(), bus_r(LOOK_AT_ME_REG));
  for (ichip=0; ichip<nModBoard*NCHIP; ichip++) {
    if ((fifoReadCounter(ichip)&FIFO_COUNTER_MASK)%128)
      printf("FIFO %d contains %d words\n",ichip,(fifoReadCounter(ichip)&FIFO_COUNTER_MASK)); 
  }
#endif
  memcpy(now_ptr, values, dataBytes);
  /*
    #ifdef VERBOSE
  for (ichip=0;ichip<dataBytes/4; ichip++) {
    now_ptr[ichip*4]=values[ichip];
#ifdef VERBOSE 
    if (((fifoReadCounter(ichip/128)&FIFO_COUNTER_MASK)+(ichip%128))>128)
      printf("chip %d ch %d %d\n",ichip/128, ichip%128, (fifoReadCounter(ichip/128)&FIFO_COUNTER_MASK));
#endif   
  }
  //#endif
  */



#ifdef VERYVERBOSE
  printf("Copying to ptr %x %d\n",now_ptr, dataBytes);
  printf("after readout %08x %08x\n", runState(), bus_r(LOOK_AT_ME_REG)); 
  for (ichip=0; ichip<nModBoard*NCHIP; ichip++) {
     if ((fifoReadCounter(ichip)&FIFO_COUNTER_MASK)%128)
      printf("FIFO %d contains %d words\n",ichip,(fifoReadCounter(ichip)&FIFO_COUNTER_MASK)); 
  }
#endif

  if (storeInRAM>0) {
    now_ptr+=dataBytes;
  }
  return ram_values;
}



u_int32_t* decode_data(int *datain)
{
  u_int32_t *dataout;
  // const char one=1;
  const int bytesize=8;
  char *ptr=(char*)datain;
  //int nbits=dynamicRange;
  int  ipos=0, ichan=0;;
  //int nch, boff=0;
  int ibyte;//, ibit;
  char iptr;

#ifdef VERBOSE
  printf("Decoding data for DR %d\n",dynamicRange);
#endif
  dataout=malloc(nChans*nChips*nModX*4);
  ichan=0;
  switch (dynamicRange) {
  case 1:
    for (ibyte=0; ibyte<dataBytes; ibyte++) {
      iptr=ptr[ibyte];
      for (ipos=0; ipos<bytesize; ipos++) {
	dataout[ichan]=(iptr>>(ipos))&0x1;
	ichan++;
      }
    }
    break;
  case 4:
    for (ibyte=0; ibyte<dataBytes; ibyte++) {
      iptr=ptr[ibyte]&0xff;
      for (ipos=0; ipos<2; ipos++) {
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
      dataout[ichan]=datain[ichan]&0xffffff;
  }
  
#ifdef VERBOSE
  printf("decoded %d  channels\n",ichan);
#endif
  return dataout;
}



int setDynamicRange(int dr) {
  int ow;
  int nm;

  u_int32_t np=getProbes();
#ifdef VERYVERBOSE
  printf("probes==%02x\n",np);
#endif
  if (dr>0) {
    nm=setNMod(-1);
    if (dr==1) {
      dynamicRange=1;
      ow=5;
    } else if (dr<=4) {
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
    setNMod(nm);
  }
  

  return   getDynamicRange();
}






int getDynamicRange() {
  int dr;
  u_int32_t shiftin=bus_r(GET_SHIFT_IN_REG);
  u_int32_t outmux=(shiftin >> OUTMUX_OFF) & OUTMUX_MASK;
  u_int32_t probes=(shiftin >> PROBES_OFF) & PROBES_MASK;
#ifdef VERYVERBOSE
  printf("%08x ",shiftin);
  printf("outmux=%02x probes=%d\n",outmux,probes);
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
  if (probes==0) {
    dataBytes=nModX*nModY*NCHIP*NCHAN*dynamicRange/8;
  }  else {
    dataBytes=nModX*nModY*NCHIP*NCHAN*4;///
  }
#ifdef VERBOSE
  printf("Number of data bytes %d - probes %d dr %d\n", dataBytes, probes, dr);
#endif
  if (allocateRAM()==OK) {
      ;
  } else
    printf("ram not allocated\n");

  return dynamicRange;

}

int testBus() {
  u_int32_t j;
  u_int64_t i, n, nt;
  //char cmd[100];
  u_int32_t val=0x0;
  int ifail=OK;
  // printf("%s\n",cmd);
  // system(cmd);
  i=0;

  n=1000000;
  nt=n/100;
  printf("testing bus %d times\n",n);
  while (i<n) {
    // val=bus_r(FIX_PATT_REG);
    bus_w(DUMMY_REG,val);
    bus_w(FIX_PATT_REG,0);
    j=bus_r(DUMMY_REG);
    //if (i%10000==1)
    if (j!=val){
      printf("%d :  read wrong value %08x instead of %08x\n",i,j, val);
      ifail++;
      //return FAIL;
    }// else
     // printf("%d : value OK 0x%08x\n",i,j);
    if ((i%nt)==0)
      printf("%lld cycles OK\n",i);
    val+=0xbbbbb;
    i++;
  }
  return ifail;
}


int setStoreInRAM(int b) {
  if (b>0)
    storeInRAM=1;
  else
    storeInRAM=0;
  return  allocateRAM();
}


int allocateRAM() {
  size_t size;
  u_int32_t nt, nf;
  nt=setTrains(-1);
  nf=setFrames(-1);
  if (nt==0) nt=1;
  if (nf==0) nf=1;
  // ret=clearRAM();
  if (storeInRAM) {
    size=dataBytes*nf*nt;
    if (size<dataBytes)
      size=dataBytes;
  }  else 
    size=dataBytes;

#ifdef VERBOSE
  printf("nmodx=%d nmody=%d dynamicRange=%d dataBytes=%d nFrames=%d nTrains, size=%d\n",nModX,nModY,dynamicRange,dataBytes,nf,nt,size );
#endif

    if (size==ram_size) {
     
#ifdef VERBOSE
      printf("RAM of size %d already allocated: nothing to be done\n", size);
#endif
 
      return OK;
    }
    
  

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
	ram_size=size;
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

int setMaster(int f) {
 
  int i;
  switch(f) {
  case NO_MASTER:
    // switch of gates or triggers
    masterMode=NO_MASTER; 
    for (i=0; i<4; i++) {
      if (signals[i]==MASTER_SLAVE_SYNCHRONIZATION) {
	setFPGASignal(i,SIGNAL_OFF);
      }
    }
    break;
  case IS_MASTER:
    // configure gate or trigger out
    masterMode=IS_MASTER;
    for (i=0; i<4; i++) {
      if (signals[i]==MASTER_SLAVE_SYNCHRONIZATION) {
	switch (syncMode) {
	case NO_SYNCHRONIZATION:
	  setFPGASignal(i,SIGNAL_OFF);
	  break;
	case MASTER_GATES:
	  setFPGASignal(i,GATE_OUT_ACTIVE_HIGH);
	  break;
	case MASTER_TRIGGERS:
	  setFPGASignal(i,TRIGGER_OUT_RISING_EDGE);
	  break;
	case SLAVE_STARTS_WHEN_MASTER_STOPS:
	  setFPGASignal(i,RO_TRIGGER_OUT_RISING_EDGE);
	  break;
	default:
	  ;
	}
      }
    }
    break;
  case IS_SLAVE:
    // configure gate or trigger in
    masterMode=IS_SLAVE;
    for (i=0; i<4; i++) {
      if (signals[i]==MASTER_SLAVE_SYNCHRONIZATION) {
	switch (syncMode) {
	case NO_SYNCHRONIZATION:
	  setFPGASignal(i,SIGNAL_OFF);
	  break;
	case MASTER_GATES:
	  setFPGASignal(i,GATE_IN_ACTIVE_HIGH);
	  break;
	case MASTER_TRIGGERS:
	  setFPGASignal(i,TRIGGER_IN_RISING_EDGE);
	  break;
	case SLAVE_STARTS_WHEN_MASTER_STOPS:
	  setFPGASignal(i,TRIGGER_IN_RISING_EDGE);
	  break;
	default:
	  ;
	}
      }
    }
    break;
  default:   
    //do nothing 
    ;
  } 
  
  switch(masterMode) {
  case NO_MASTER:
    return NO_MASTER;


  case IS_MASTER:
    for (i=0; i<4; i++) {
      if (signals[i]==MASTER_SLAVE_SYNCHRONIZATION) {
	switch (syncMode) {
	case NO_SYNCHRONIZATION:
	  return IS_MASTER;
	case MASTER_GATES:
	  if (getFPGASignal(i)==GATE_OUT_ACTIVE_HIGH)
	    return IS_MASTER;
	  else
	    return NO_MASTER;
	case MASTER_TRIGGERS:
	  if (getFPGASignal(i)==TRIGGER_OUT_RISING_EDGE)
	    return IS_MASTER;
	  else
	    return NO_MASTER;
	case SLAVE_STARTS_WHEN_MASTER_STOPS:
	  if (getFPGASignal(i)==RO_TRIGGER_OUT_RISING_EDGE)
	    return IS_MASTER;
	  else
	    return NO_MASTER;
	default:
	  return NO_MASTER;
	}
	
      }
    }

  case IS_SLAVE:
    for (i=0; i<4; i++) {
      if (signals[i]==MASTER_SLAVE_SYNCHRONIZATION) {
	switch (syncMode) {
	case NO_SYNCHRONIZATION:
	  return IS_SLAVE;
	case MASTER_GATES:
	  if (getFPGASignal(i)==GATE_IN_ACTIVE_HIGH)
	    return IS_SLAVE;
	  else
	    return NO_MASTER;
	case MASTER_TRIGGERS:
	case SLAVE_STARTS_WHEN_MASTER_STOPS:
	  if (getFPGASignal(i)==TRIGGER_IN_RISING_EDGE)
	    return IS_SLAVE;
	  else
	    return NO_MASTER;
	default:
	  return NO_MASTER;
	}
	
      }
    }
    
  }

  return masterMode;
}

int setSynchronization(int s) {

  int i;

  switch(s) {
  case NO_SYNCHRONIZATION:
    syncMode=NO_SYNCHRONIZATION;
    for (i=0; i<4; i++) {
      if (signals[i]==MASTER_SLAVE_SYNCHRONIZATION) {
	setFPGASignal(i,SIGNAL_OFF);
      }
    }
    break;
    // disable external signals?
  case MASTER_GATES:
    // configure gate in or out
    syncMode=MASTER_GATES;
    for (i=0; i<4; i++) {
      if (signals[i]==MASTER_SLAVE_SYNCHRONIZATION) {
	if (masterMode==IS_MASTER)
	  setFPGASignal(i,GATE_OUT_ACTIVE_HIGH);
	else if (masterMode==IS_SLAVE)
	  setFPGASignal(i,GATE_IN_ACTIVE_HIGH);
      }
    }

    break;
  case MASTER_TRIGGERS:
    // configure trigger in or out 
    syncMode=MASTER_TRIGGERS;  
    for (i=0; i<4; i++) {
      if (signals[i]==MASTER_SLAVE_SYNCHRONIZATION) {
	if (masterMode==IS_MASTER)
	  setFPGASignal(i,TRIGGER_OUT_RISING_EDGE);
	else if (masterMode==IS_SLAVE)
	  setFPGASignal(i,TRIGGER_IN_RISING_EDGE);
      }
    }
    break;


  case SLAVE_STARTS_WHEN_MASTER_STOPS:
    // configure trigger in or out
    syncMode=SLAVE_STARTS_WHEN_MASTER_STOPS;
    for (i=0; i<4; i++) {
      if (signals[i]==MASTER_SLAVE_SYNCHRONIZATION) {
	if (masterMode==IS_MASTER)
	  setFPGASignal(i,RO_TRIGGER_OUT_RISING_EDGE);
	else if (masterMode==IS_SLAVE)
	  setFPGASignal(i,TRIGGER_IN_RISING_EDGE);
      }
    }
    break;

    
  default:
    //do nothing
    ;
  } 
  
  switch (syncMode) {

  case NO_SYNCHRONIZATION:
    return NO_SYNCHRONIZATION;
    
  case MASTER_GATES:

    for (i=0; i<4; i++) {
      if (signals[i]==MASTER_SLAVE_SYNCHRONIZATION) {
	if (masterMode==IS_MASTER && getFPGASignal(i)==GATE_OUT_ACTIVE_HIGH)
	  return MASTER_GATES;
	else if (masterMode==IS_SLAVE && getFPGASignal(i)==GATE_IN_ACTIVE_HIGH)
	  return MASTER_GATES;
      }
    }
    return NO_SYNCHRONIZATION;

  case MASTER_TRIGGERS:
    for (i=0; i<4; i++) {
      if (signals[i]==MASTER_SLAVE_SYNCHRONIZATION) {
	if (masterMode==IS_MASTER && getFPGASignal(i)==TRIGGER_OUT_RISING_EDGE)
	  return MASTER_TRIGGERS;
	else if (masterMode==IS_SLAVE && getFPGASignal(i)==TRIGGER_IN_RISING_EDGE)
	  return MASTER_TRIGGERS;
      }
    }
    return NO_SYNCHRONIZATION;
    
  case SLAVE_STARTS_WHEN_MASTER_STOPS:
    for (i=0; i<4; i++) {
      if (signals[i]==MASTER_SLAVE_SYNCHRONIZATION) {
	if (masterMode==IS_MASTER &&	getFPGASignal(i)==RO_TRIGGER_OUT_RISING_EDGE)
	  return SLAVE_STARTS_WHEN_MASTER_STOPS;
	else if (masterMode==IS_SLAVE && getFPGASignal(i)==TRIGGER_IN_RISING_EDGE)
	  return SLAVE_STARTS_WHEN_MASTER_STOPS;
      }
    }
    return NO_SYNCHRONIZATION;

  default:
      return NO_SYNCHRONIZATION;

  }
  


  return NO_SYNCHRONIZATION;
  


}
