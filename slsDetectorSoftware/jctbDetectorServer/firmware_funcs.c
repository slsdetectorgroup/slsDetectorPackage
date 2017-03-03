//#define TESTADC
#define TESTADC1


//#define TIMEDBG 
#include "server_defs.h"
#include "firmware_funcs.h"
#include "mcb_funcs.h"
#include "registers_m.h"

//#define VERBOSE
//#define VERYVERBOSE


#ifdef SHAREDMEMORY
#include "sharedmemory.h"
#endif

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <sys/time.h>
#include <stdlib.h>  /* exit() */
#include <string.h>  /* memset(), memcpy() */
#include <sys/utsname.h>   /* uname() */
#include <sys/types.h>
#include <sys/socket.h>   /* socket(), bind(),
                         listen(), accept() */
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>  /* fork(), write(), close() */
#include <time.h> 
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct ip_header_struct {
  u_int16_t     ip_len;
  u_int8_t      ip_tos;
  u_int8_t      ip_ihl:4 ,ip_ver:4;
  u_int16_t     ip_offset:13,ip_flag:3;
  u_int16_t     ip_ident;
  u_int16_t     ip_chksum;
  u_int8_t      ip_protocol;
  u_int8_t      ip_ttl;
  u_int32_t     ip_sourceip;
  u_int32_t     ip_destip;
} ip_header;


struct timeval tss,tse,tsss; //for timing


//for memory mapping
u_int32_t CSP0BASE;


FILE *debugfp, *datafp;

int fr;
int wait_time;
int *fifocntrl;

//int *statusreg; commented out by dhanya
const int nModY=1;
int nModBoard;
int nModX=NMAXMOD;
int dynamicRange=16;//32;
int nSamples=1;

int dataBytes=NMAXMOD*NCHIP*NCHAN*2;

int storeInRAM=0;
int ROI_flag=0;
int adcConfigured=-1;
u_int16_t *ram_values=NULL;
char volatile *now_ptr=NULL;
//u_int32_t volatile  *values;
u_int16_t volatile  *values;
int ram_size=0;

int64_t totalTime=1;
u_int32_t progressMask=0;

int phase_shift=0;//DEFAULT_PHASE_SHIFT;
int ipPacketSize=DEFAULT_IP_PACKETSIZE;
int udpPacketSize=DEFAULT_UDP_PACKETSIZE;


#ifndef NEW_PLL_RECONFIG
u_int32_t clkDivider[4]={32,16,16,16};
#else
u_int32_t clkDivider[4]={40,20,20,200};
#endif
int32_t clkPhase[4]={0,0,0,0};

u_int32_t adcDisableMask=0;

int ififostart, ififostop, ififostep, ififo;

int masterMode=NO_MASTER, syncMode=NO_SYNCHRONIZATION, timingMode=AUTO_TIMING;

enum externalSignalFlag  signals[4]={EXT_SIG_OFF, EXT_SIG_OFF, EXT_SIG_OFF, EXT_SIG_OFF};

int withGotthard = 0;

/**is not const because this value will change after initDetector, is removed from mcb_funcs.c cuz its not used anywhere
 * why is this used anywhere instead of macro*/
int nChans=NCHAN;
int nChips=NCHIP;
//int nDacs;//=NDAC;
//int nAdcs=NADC;

extern enum detectorType myDetectorType;
/** for jungfrau reinitializing macro later  in server_funcs.c in initDetector*/
 extern int N_CHAN; 
 extern int N_CHIP; 
 extern int N_DAC; 
 extern int N_ADC; 
 extern int N_CHANS; 


int analogEnable=1;
int digitalEnable=0;


int vLimit=-100;




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
  printf("CSPObase is 0x%08x \n",CSP0BASE);
  printf("CSPOBASE=from %08x to %08x\n",CSP0BASE,CSP0BASE+MEM_SIZE);

  u_int32_t address;
  address = FIFO_DATA_REG;//_OFF;
  //values=(u_int32_t*)(CSP0BASE+address*2);
  values=(u_int16_t*)(CSP0BASE+address*2);
  printf("statusreg=%08x\n",bus_r(STATUS_REG));
  printf("\n\n");
  return OK;
}

u_int16_t bus_r16(u_int32_t offset){
  volatile u_int16_t *ptr1;
  ptr1=(u_int16_t*)(CSP0BASE+offset*2);
  return *ptr1;
}

u_int16_t bus_w16(u_int32_t offset, u_int16_t data) {
  volatile u_int16_t  *ptr1;
  ptr1=(u_int16_t*)(CSP0BASE+offset*2);
  *ptr1=data;
  return OK;
}

/** ramType is DARK_IMAGE_REG or GAIN_IMAGE_REG */
u_int16_t ram_w16(u_int32_t ramType, int adc, int adcCh, int Ch, u_int16_t data) { 
  unsigned int adr = (ramType | adc << 8 | adcCh << 5 | Ch );
  // printf("Writing to addr:%x\n",adr);
  return bus_w16(adr,data);
}

/** ramType is DARK_IMAGE_REG or GAIN_IMAGE_REG */
u_int16_t ram_r16(u_int32_t ramType, int adc, int adcCh, int Ch){
  unsigned int adr = (ramType | adc << 8 | adcCh << 5 | Ch );
  //  printf("Reading from addr:%x\n",adr);
  return bus_r16(adr);
}

u_int32_t bus_w(u_int32_t offset, u_int32_t data) {
 volatile  u_int32_t  *ptr1;

  ptr1=(u_int32_t*)(CSP0BASE+offset*2);
  *ptr1=data;

  return OK;
}


u_int32_t bus_r(u_int32_t offset) {
  volatile u_int32_t  *ptr1;
  ptr1=(u_int32_t*)(CSP0BASE+offset*2);
  return *ptr1;
}




int cleanFifo(){
/* 	u_int32_t addr, reg, val, adc_sync; */
/* 	printf("Cleaning FIFO\n"); */
/* 	addr=ADC_SYNC_REG; */

/* 	if(withGotthard) */
/* 		adc_sync = GOTTHARD_ADCSYNC_VAL; */
/* 	else */
/* 		adc_sync = ADCSYNC_VAL; */


/* 	reg = bus_r(addr) &	CLEAN_FIFO_MASK; */

/* 	//only for start up */
/* 	if(!reg) reg = adc_sync; */

/* 	// 88 3 02111 */
/* 	if (ROI_flag==0) { */
/* 	val=reg | ADCSYNC_CLEAN_FIFO_BITS | TOKEN_RESTART_DELAY; */
/* 	bus_w(addr,val); */
/* 	// 88 0 02111 */
/* 	val=reg | TOKEN_RESTART_DELAY; */
/* 	bus_w(addr,val); */
/* 	} */
/* 	else { */
/* 		//1b332214 */
/* 	  val=reg | ADCSYNC_CLEAN_FIFO_BITS | TOKEN_RESTART_DELAY_ROI; */
/* 	  bus_w(addr,val); */
/* 	  //1b032214 */
/* 	  val=reg | TOKEN_RESTART_DELAY_ROI; */
/* 	  bus_w(addr,val); */

/* 	} */
/* 	reg=bus_r(addr); */
/* //#ifdef DDEBUG */
/* 	printf("ADC SYNC reg 0x19:%x\n",reg); */
/* //#endif */
	return OK;
}


int setDAQRegister()
{
/* 	u_int32_t addr, reg, val; */
/* 	addr=DAQ_REG; */

/* 	//depended on adcval */
/* 	int packetlength=0x7f; */
/* 	if(!ROI_flag) packetlength=0x13f; */

/* 	//depended on pcb rev */
/* 	int tokenTiming = TOKEN_TIMING_REV2; */
/* 	if((bus_r(PCB_REV_REG)&BOARD_REVISION_MASK)==1) */
/* 		tokenTiming= TOKEN_TIMING_REV1; */


/* 	val = (packetlength<<16) + tokenTiming; */
/* 	//val=34+(42<<8)+(packetlength<<16); */

/* 	reg=bus_r(addr); */
/* 	bus_w(addr,val); */
/* 	reg=bus_r(addr); */
/* //#ifdef VERBOSE */
/* 	printf("DAQ reg 0x15:%x\n",reg); */
/* //#endif */

	return OK;
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
    if (s[i]=='1') pat=pat+(1<<(15-i));
  }
  //addr=DAC_REG+(modnum<<4);
  addr=DAC_REG;//+(modnum<<SHIFTMOD); commented by dhanya
  bus_w(addr, pat);
  printf("ACHTUNG!!!!!!!!!!!!! Writing to DAc reg using putout!!!!!!!!!!!!!!!\n");
  return OK;
}


// read direct input 
u_int32_t readin(int modnum) {
	return 0;
}

u_int32_t setPllReconfigReg(u_int32_t reg, u_int32_t val, int trig) {
 
  u_int32_t vv;




  // printf("*********** pll busy: %08x\n",bus_r(STATUS_REG)&PLL_RECONFIG_BUSY);

  bus_w(PLL_PARAM_REG,val); 
  //  printf("param: %x\n",val);


  vv=reg<<PLL_CNTR_ADDR_OFF;
  bus_w(PLL_CNTRL_REG,vv); 
  usleep(10000);
  //  printf("wrote: %08x\n",vv);
  //  vv=(1<<PLL_CNTR_WRITE_BIT)|(reg<<PLL_CNTR_ADDR_OFF)|(trig<<15);
  bus_w(PLL_CNTRL_REG,vv|(1<<PLL_CNTR_WRITE_BIT) );//15 is trigger for the tap
  // printf("----------- pll busy: %08x\n",bus_r(STATUS_REG)&PLL_RECONFIG_BUSY);
 //  printf("wrote: %08x\n",vv); 
 //  usleep(10000);


 // vv=(reg<<PLL_CNTR_ADDR_OFF);
  //  printf("wrote: %08x\n",vv); 
  bus_w(PLL_CNTRL_REG,vv); 
  usleep(10000);
  // printf("+++++++++ pll busy: %08x\n",bus_r(STATUS_REG)&PLL_RECONFIG_BUSY);

  //  bus_w(PLL_CNTRL_REG,(1<<PLL_CNTR_READ_BIT)|(reg<<PLL_CNTR_ADDR_OFF));
  // usleep(1000);
  // val=bus_r(PLL_PARAM_OUT_REG);
  // printf("counter %x reg: %x\n",reg,val); 
  // bus_w(PLL_CNTRL_REG,(reg<<PLL_CNTR_ADDR_OFF)); 

/*   usleep(100); */
/*   bus_w(PLL_CNTRL_REG,PLL_CNTR_PLL_RESET_BIT);  */
/*   usleep(100); */
/*   bus_w(PLL_CNTRL_REG,0);  */
  return val;

}

u_int32_t getPllReconfigReg(u_int32_t reg, int trig) {
  
  u_int32_t val=reg<<PLL_CNTR_ADDR_OFF;
  u_int32_t vv;

  //  printf("cntrlreg: %08x\n",PLL_CNTRL_REG);
  // printf("wrote: %08x\n",val);
  bus_w(PLL_CNTRL_REG,val);
  //  printf("read: %08x\n",bus_r(PLL_CNTRL_REG));
  usleep(100);

  val=(1<<PLL_CNTR_READ_BIT)|(reg<<PLL_CNTR_ADDR_OFF)|(trig<<15);
  //  printf("wrote: %08x\n",val);
  bus_w(PLL_CNTRL_REG,val);//15 is trigger for the tap
  //  printf("read: %08x\n",bus_r(PLL_CNTRL_REG));
  //  usleep(100);
/*   for (i=0; i<10; i++) { */
  //   vv=bus_r(PLL_PARAM_OUT_REG); 
  //   printf("addr %x reg: %x\n",reg,vv); 
/*     usleep(100); */
/*   } */

  val=(reg<<PLL_CNTR_ADDR_OFF);
  //  printf("wrote: %08x\n",val);
  bus_w(PLL_CNTRL_REG,val);
  usleep(100);

  val=0;
  // printf("wrote: %08x\n",val);
  bus_w(PLL_CNTRL_REG,val);

  while(bus_r(STATUS_REG)&PLL_RECONFIG_BUSY) {
    printf("get: reconfig busy");
  }
  return vv;
}

void resetPLL() {
  bus_w(PLL_CNTRL_REG,(1<<PLL_CNTR_RECONFIG_RESET_BIT)|(1<<PLL_CNTR_PLL_RESET_BIT)); //reset PLL and pll reconfig
  usleep(100);
  bus_w(PLL_CNTRL_REG, 0);
}

int configurePhase(int val, int i) {
  


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

  //   printf("PLL reconfig reset\N");   bus_w(PLL_CNTRL_REG,(1<<PLL_CNTR_RECONFIG_RESET_BIT));  usleep(100);  bus_w(PLL_CNTRL_REG, 0);
   bus_w(PLL_CNTRL_REG,((1<<PLL_CNTR_PLL_RESET_BIT))); //reset PLL
   usleep(100);
   bus_w(PLL_CNTRL_REG, 0);
    
  setPllReconfigReg(PLL_MODE_REG,1,0);
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


int configureFrequency(int val, int i) {


  u_int32_t l=0x0c;
  u_int32_t h=0x0d;
  u_int32_t vv;
  int32_t phase=0, inv=0;

  u_int32_t tot;
  u_int32_t odd=1;//0;

  //   printf("PLL reconfig reset\N");   bus_w(PLL_CNTRL_REG,(1<<PLL_CNTR_RECONFIG_RESET_BIT));  usleep(100);  bus_w(PLL_CNTRL_REG, 0);
  if (i<0 || i>3)
    return -1;
  
  if (val<=0) 
    return clkDivider[i];

  if (i==1 || i==2){
    if (val>40) printf("Too high frequency %d MHz for these ADCs!\n", val);
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
  /* //  usleep(20); */
  /* //change sync at the same time as  */
  /* if (i>0) { */
  /*   val= (2<<18)| (odd<<17) | l | (h<<8);  */

  /*   printf("Counter %d, val: %08x\n", i,  val);   */
  /*   setPllReconfigReg(PLL_C_COUNTER_REG, val,0); */
    
  /* } */

 
  usleep(10000);
 
  printf("reset pll\n"); 
  bus_w(PLL_CNTRL_REG,((1<<PLL_CNTR_PLL_RESET_BIT))); //reset PLL
  usleep(100);
  bus_w(PLL_CNTRL_REG, 0);
    
  return clkDivider[i];   
}









/* u_int32_t setClockDivider(int d, int ic) { */


/*    //u_int32_t l=0x0c; */
/*    //u_int32_t h=0x0d; */

/*   u_int32_t tot= PLL_VCO_FREQ_MHZ/d; */

/*   //	int ic=0  is run clk; ic=1 is adc clk  */
/*   printf("set clk divider %d to %d\n", ic, d); */
/*   if (ic>2) */
/*     return -1; */

/*   if (ic==2) { */
/*     printf("dbit clock is the same as adc clk\n"); */
/*     ic=1; */

/*   } */

/*   if (ic==1 && d>40) */
/*     return -1; */

/*   if (d>160) */
/*     return -1; */

/*   if (tot>510) */
/*     return -1; */

/*   if (tot<1) */
/*     return -1; */



/*   clkDivider[ic]=d; */
/*   configurePll(ic); */

  
  
/*  return clkDivider[ic]; */
/* } */


/* int phaseStep(int st){ */
  
/*   if (st>65535 || st<-65535)  */
/*     return clkPhase[0]; */
/* #ifdef NEW_PLL_RECONFIG */
/*      printf("reset pll\n");  */
/*      bus_w(PLL_CNTRL_REG,((1<<PLL_CNTR_PLL_RESET_BIT))); //reset PLL */
/*      usleep(100); */
/*      bus_w(PLL_CNTRL_REG, 0); */
    
/*      clkPhase[1]=st; */
/* #else */
/*      clkPhase[1]=st-clkPhase[0]; */
/* #endif */
  
/*   printf("phase %d\n", clkPhase[1] ); */

/*   configurePll(2); */

/*   clkPhase[0]=st; */
 
/*   return clkPhase[0]; */
/* } */

/* int dbitPhaseStep(int st){ */
  
/*     printf("dbit clock is the same as adc clk\n"); */
/*     return phaseStep(st); */
/* } */







int getPhase(int i) {
  if (i>=0 && i<4)
    return clkPhase[i];
  else
    return -1;
    
};



/* int getDbitPhase() { */
  
/*   printf("dbit clock is the same as adc clk\n"); */
/*   return getPhase(); */
    
/* }; */


/* u_int32_t getClockDivider(int ic) { */

/*   if (ic>2) */
/*     return -1; */
  
/*   if (ic==2) { */
/*     printf("dbit clock is the same as adc clk\n"); */
/*     ic=1; */

/*   } */
/*   return clkDivider[ic]; */
  

/* /\*   int ic=0; *\/ */
/* /\*  u_int32_t val; *\/ */
/* /\*  u_int32_t l,h; *\/ */

/* /\*  printf("get clk divider\n"); *\/ */

  
/* /\*   setPllReconfigReg(PLL_MODE_REG,1,0); *\/ */
/* /\*   getPllReconfigReg(PLL_MODE_REG,0); *\/ */
  
/* /\*   u_int32_t addr=0xa; //c0 *\/ */
/* /\*   if (ic>0) *\/ */
/* /\*     addr=0xb; //c1 *\/ */

/* /\*   val=getPllReconfigReg(PLL_N_COUNTER_REG,0); *\/ */
/* /\*   printf("Getting N counter %08x\n",val); *\/ */

/* /\*   l=val&0xff; *\/ */
/* /\*   h=(val>>8)&0xff; *\/ */

/* /\*   //getPllReconfigReg(PLL_STATUS_REG,0); *\/ */
/* /\*   val=getPllReconfigReg(addr,0); *\/ */
/* /\*   printf("Getting C counter %08x\n",val); *\/ */

  

/* /\*   return 800/(l+h); *\/ */

/* } */


u_int32_t adcPipeline(int d) {
  u_int32_t v;
  if (d>=0) {
    v=bus_r(ADC_PIPELINE_REG)&0x00ff0000;
    bus_w(ADC_PIPELINE_REG, d|v);
  }
  return bus_r(ADC_PIPELINE_REG)&0xff;
}


u_int32_t dbitPipeline(int d) {
  u_int32_t v;
  if (d>=0) {
    v=bus_r(ADC_PIPELINE_REG)&0x000000ff;
    bus_w(ADC_PIPELINE_REG, v|(d<<16));

  }
  v=bus_r(ADC_PIPELINE_REG)>>16;
  return v&0xff;
}


u_int32_t setSetLength(int d) {
	 return 0;
}

u_int32_t getSetLength() {
	 return 0;
}

u_int32_t setOversampling(int d) {
  return 0;
  /* if (d>=0 && d<=255) */
  /*   bus_w(OVERSAMPLING_REG, d); */

  /* return bus_r(OVERSAMPLING_REG); */
}


u_int32_t setWaitStates(int d1) {
	 return 0;
}

u_int32_t getWaitStates() {
	 return 0;
}



u_int32_t setExtSignal(int d, enum externalSignalFlag  mode) {

	//int modes[]={EXT_SIG_OFF, EXT_GATE_IN_ACTIVEHIGH, EXT_GATE_IN_ACTIVELOW,EXT_TRIG_IN_RISING,EXT_TRIG_IN_FALLING,EXT_RO_TRIG_IN_RISING, EXT_RO_TRIG_IN_FALLING,EXT_GATE_OUT_ACTIVEHIGH, EXT_GATE_OUT_ACTIVELOW, EXT_TRIG_OUT_RISING, EXT_TRIG_OUT_FALLING, EXT_RO_TRIG_OUT_RISING, EXT_RO_TRIG_OUT_FALLING};
	// int off=d*SIGNAL_OFFSET;

	u_int32_t c;
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
			break;
		}

		setTiming(GET_EXTERNAL_COMMUNICATION_MODE);
	}


//  if (mode<=RO_TRIGGER_OUT_FALLING_EDGE && mode>=0)
//    bus_w(EXT_SIGNAL_REG,((modes[mode])<<off)|(c&~(SIGNAL_MASK<<off)));


  return getExtSignal(d);
}



u_int32_t setFPGASignal(int d, enum externalSignalFlag  mode) {


  int modes[]={EXT_SIG_OFF, EXT_GATE_IN_ACTIVEHIGH, EXT_GATE_IN_ACTIVELOW,EXT_TRIG_IN_RISING,EXT_TRIG_IN_FALLING,EXT_RO_TRIG_IN_RISING, EXT_RO_TRIG_IN_FALLING,EXT_GATE_OUT_ACTIVEHIGH, EXT_GATE_OUT_ACTIVELOW, EXT_TRIG_OUT_RISING, EXT_TRIG_OUT_FALLING, EXT_RO_TRIG_OUT_RISING, EXT_RO_TRIG_OUT_FALLING};

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

/*  int modes[]={SIGNAL_OFF, GATE_IN_ACTIVE_HIGH, GATE_IN_ACTIVE_LOW,TRIGGER_IN_RISING_EDGE, TRIGGER_IN_FALLING_EDGE,RO_TRIGGER_IN_RISING_EDGE, RO_TRIGGER_IN_FALLING_EDGE, GATE_OUT_ACTIVE_HIGH,   GATE_OUT_ACTIVE_LOW, TRIGGER_OUT_RISING_EDGE, TRIGGER_OUT_FALLING_EDGE, RO_TRIGGER_OUT_RISING_EDGE,RO_TRIGGER_OUT_FALLING_EDGE};

    int off=d*SIGNAL_OFFSET;
    int mode=((bus_r(EXT_SIGNAL_REG)&(SIGNAL_MASK<<off))>>off);

    if (mode<RO_TRIGGER_OUT_FALLING_EDGE)
      return modes[mode];
    else 
      return -1;*/

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
#ifdef VERYVERBOSE
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
	  break;

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
 //int ret=0;
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

/* int setOutputMode(int d) { */
/*  //int ret=0; */
/*  int reg; */
/*  int v; */
/*  //#ifdef VERBOSE */
/*   printf("Setting readout flags to to %d\n",d); */
/*   //#endif */
/*   reg=bus_r(CONFIG_REG); */
/*   //#ifdef VERBOSE */
/*   printf("Before: config reg is %x\n", reg); */
/*   //#endif */
/*   if (d>=0) { */
/*     reg=reg & ~(3<<8); */
/*     if (d==DIGITAL_ONLY) */
/*       reg=reg | (3<<8); */
/*     else if (d==ANALOG_AND_DIGITAL) */
/*       reg=reg | (2<<8); */
  
/*     bus_w(CONFIG_REG,reg); */
    
/*   } */

/*   reg=bus_r(CONFIG_REG); */
/*   //#ifdef VERBOSE */
/*   printf("After: config reg is %x\n", reg); */
/*   //#endif */
/*   if ((reg&(2<<8))) { */
/*     if (reg&(1<<8)) { */
/*       digitalEnable=1; */
/*       analogEnable=0; */
/*       return DIGITAL_ONLY; */
/*     }    else { */
/*       digitalEnable=1; */
/*       analogEnable=0; */
/*       return ANALOG_AND_DIGITAL; */
/*     } */
/*   } else */
/*     if (reg&(1<<8)) */
/*       return -1; */
/*     else */
/*       return NORMAL_READOUT; */


/* } */

int setContinousReadOut(int d) {
 //int ret=0;
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


int startReceiver(int start) {
	u_int32_t addr=CONFIG_REG;
	//#ifdef VERBOSE
	if(start)
		printf("Setting up detector to send to Receiver\n");
	else
		printf("Setting up detector to send to CPU\n");
	//#endif
	int reg=bus_r(addr);
	//for start recever, write 0 and for stop, write 1
	if (!start)
		bus_w(CONFIG_REG,reg&(~GB10_NOT_CPU_BIT));
	else
		bus_w(CONFIG_REG,reg|GB10_NOT_CPU_BIT);

	reg=bus_r(addr);
//#ifdef VERBOSE
	printf("Config Reg %x\n", reg);
//#endif
	int d =reg&GB10_NOT_CPU_BIT;
	if(d!=0) d=1;

	printf("Value is %d expected %d\n", d, start);

	if(d!=start)
		return FAIL;
	else
		return OK;
}


u_int64_t  getDetectorNumber() {
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

u_int32_t  getFirmwareVersion() {
  return bus_r(FPGA_VERSION_REG);
}

u_int32_t  getFirmwareSVNVersion(){
  return bus_r(FPGA_SVN_REG);
}


// for fpga test 
u_int32_t testFpga(void) {
	  printf("Testing FPGA:\n");
  volatile u_int32_t val,addr,val2;
  int result=OK,i;
  //fixed pattern
  val=bus_r(FIX_PATT_REG);
  if (val==FIXED_PATT_VAL) {
    printf("fixed pattern ok!! %08x\n",val);
  } else {
    printf("fixed pattern wrong!! %08x\n",val);
    result=FAIL;
  }

  //dummy register
  addr = DUMMY_REG;
  for(i=0;i<1000000;i++)
    {
      val=0x5A5A5A5A-i;
      bus_w(addr, val);
      val=bus_r(addr);
      if (val!=0x5A5A5A5A-i) {
	printf("ATTEMPT:%d:\tFPGA dummy register wrong!! %x instead of %x \n",i,val,0x5A5A5A5A-i);
	result=FAIL;
      }
      val=(i+(i<<10)+(i<<20));
      bus_w(addr, val);
      val2=bus_r(addr);
      if (val2!=val) {
	printf("ATTEMPT:%d:\tFPGA dummy register wrong!! read %x instead of %x.\n",i,val2,val);
	result=FAIL;
      }
      val=0x0F0F0F0F;
      bus_w(addr, val);
      val=bus_r(addr);
      if (val!=0x0F0F0F0F) {
	printf("ATTEMPT:%d:\tFPGA dummy register wrong!! %x instead of 0x0F0F0F0F \n",i,val);
	result=FAIL;
      }
      val=0xF0F0F0F0;
      bus_w(addr, val);
      val=bus_r(addr);
      if (val!=0xF0F0F0F0)  {
	printf("ATTEMPT:%d:\tFPGA dummy register wrong!! %x instead of 0xF0F0F0F0 \n\n",i,val);
	result=FAIL;
      }
    }
  if(result==OK)
    {
      printf("----------------------------------------------------------------------------------------------");
      printf("\nATTEMPT 1000000: FPGA DUMMY REGISTER OK!!!\n");
      printf("----------------------------------------------------------------------------------------------");
    }
  printf("\n");
  return result;
}


// for fpga test 
u_int32_t testRAM(void) {
  int result=OK;

  printf("TestRAM not implemented\n");

/*  int i=0;
  allocateRAM();
  //  while(i<100000) {
    memcpy(ram_values, values, dataBytes);
    printf ("Testing RAM:\t%d: copied fifo %x to memory %x size %d\n",i++, (unsigned int)(values), (unsigned int)(ram_values), dataBytes);
    // }
     *
*/
  return result;
}


int getNModBoard() {
  
  return 1;
}

int setNMod(int n) {

/*   printf("Writin ADC disable register %08x\n",n); */
/*   bus_w(ADC_LATCH_DISABLE_REG,n); */
  return getNMod();
}

int getNMod() {
/*   u_int32_t reg; */
/*   int i; */
/*   reg=bus_r(ADC_LATCH_DISABLE_REG); */

/*   printf("Read ADC disable register %08x\n",reg); */
/*   nModX=32; */
/*   for (i=0; i<32; i++) { */
/*     if (reg & (1<<i)) { */
/*       nModX--; */
/*       printf("ADC %d is disabled\n",i); */
/*     } */
/*   } */
  
  
  return nModX;
}


// fifo test 
int testFifos(void) {
  printf("Fifo test not implemented!\n");
  bus_w16(CONTROL_REG, START_FIFOTEST_BIT);
  bus_w16(CONTROL_REG, 0x0);
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
    //   printf("Wreg64(%x,%x) %08x %08x %016llx\n", aLSB>>11, aMSB>>11, vLSB, vMSB, value);
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

  // printf("reg64(%x,%x) %x %x %llx\n", aLSB, aMSB, vLSB, vMSB, v64);

  return v64;
}

int64_t setFrames(int64_t value){
  return set64BitReg(value,  SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG);
}

int64_t getFrames(){
  /*printf("gf");*/
  return get64BitReg(GET_FRAMES_LSB_REG, GET_FRAMES_MSB_REG);
}

int64_t setExposureTime(int64_t value){
  /* time is in ns */
  if (value!=-1)
    value*=(1E-3*clkDivider[0]);//(1E-9*CLK_FREQ);
    return set64BitReg(value,SET_EXPTIME_LSB_REG, SET_EXPTIME_MSB_REG)/(1E-3*clkDivider[0]);//(1E-9*CLK_FREQ);
}

int64_t getExposureTime(){
  return get64BitReg(GET_EXPTIME_LSB_REG, GET_EXPTIME_MSB_REG)/(1E-3*clkDivider[0]);//(1E-9*CLK_FREQ);
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
    // value*=(1E-9*CLK_FREQ);
    value*=(1E-3*clkDivider[1]);
  }
  if (value%2==0) {
    
    printf("Adding one to period: was %08llx ", value);
    value+=1;
    printf("now is %08llx\n ", value);

    
  } else
    printf("Period already even is %08llx\n ", value);


  return set64BitReg(value,SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG)/(1E-3*clkDivider[1]);//(1E-9*CLK_FREQ);
}




int64_t getPeriod(){
  return get64BitReg(GET_PERIOD_LSB_REG, GET_PERIOD_MSB_REG)/(1E-3*clkDivider[1]);//(1E-9*CLK_FREQ);
}
int64_t setSamples(int64_t value){
  if (value>=0) {
    nSamples=value;
    bus_w(NSAMPLES_REG,nSamples);
  }
  getDynamicRange();
  allocateRAM();
  //printf("Setting dataBytes to %d: dr %d; samples %d\n",dataBytes, dynamicRange, nSamples);
  return nSamples;
}

int64_t setDelay(int64_t value){
  /* time is in ns */
  if (value!=-1) {
    value*=(1E-3*clkDivider[1]);//(1E-9*CLK_FREQ);
  }
  return set64BitReg(value,SET_DELAY_LSB_REG, SET_DELAY_MSB_REG)/(1E-3*clkDivider[0]);//(1E-9*CLK_FREQ);
}

int64_t getDelay(){
  return get64BitReg(GET_DELAY_LSB_REG, GET_DELAY_MSB_REG)/(1E-3*clkDivider[0]);//(1E-9*CLK_FREQ);
}

int64_t setTrains(int64_t value){
  printf("Set cycles %lld\n",value);
  return set64BitReg(value,  SET_CYCLES_LSB_REG, SET_CYCLES_MSB_REG);
}

int64_t getTrains(){
  return get64BitReg(GET_CYCLES_LSB_REG, GET_CYCLES_MSB_REG);
}


int64_t setProbes(int64_t value){
  return 0;
}


int64_t setProgress() {

  //????? eventually call after setting the registers

return 0;

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
  // int64_t mask=0x8000000000000000;
  // if (v & mask ) {
  //#ifdef VERBOSE
  //  printf("no measurement time left\n");
  //#endif
  //  return -1E+9;
  // } else
    return v/(1E-9*CLK_FREQ);
}

int64_t getFramesFromStart(){
  int64_t v=get64BitReg(FRAMES_FROM_START_LSB_REG, FRAMES_FROM_START_MSB_REG);
  int64_t v1=get64BitReg(FRAMES_FROM_START_PG_LSB_REG, FRAMES_FROM_START_PG_MSB_REG);

  printf("Frames from start data streaming %lld\n",v);
  printf("Frames from start run control %lld\n",v1);

  // int64_t mask=0x8000000000000000;
  // if (v & mask ) {
  //#ifdef VERBOSE
  //  printf("no measurement time left\n");
  //#endif
  //  return -1E+9;
  // } else
    return v;
}


int setROI(int nroi,ROI* arg,int *retvalsize, int *ret) {

 

  // ROI retval[MAX_ROIS];
  int i, ich;
  adcDisableMask=0xffffffff; 

  printf("Setting ROI\n");
  if (nroi>=0) {
    if (nroi==0) {
      adcDisableMask=0;
    } else {
      for (i=0; i<nroi; i++) {
	printf("iroi: %d  - %d %d %d %d\n",i, arg[i].xmin, arg[i].xmax, arg[i].ymin, arg[i].ymax);
	for (ich=arg[i].xmin; ich<=arg[i].xmax; ich++) {
	  if (ich>=0 && ich<N_CHAN)
	    adcDisableMask&=~(1<<ich);
	  else
	    break;
	  printf("%d write adc disable mask %08x\n",ich , adcDisableMask);
	}
      }
    }
    printf("write adc disable mask %08x\n",adcDisableMask);
    bus_w(ADC_LATCH_DISABLE_REG,adcDisableMask);
  }
  *ret=OK;
  
   adcDisableMask=bus_r(ADC_LATCH_DISABLE_REG);

   printf("read adc disable mask %08x\n",adcDisableMask); 
   *retvalsize=0;
   if (adcDisableMask==0) return *retvalsize;
   arg[0].xmin=0;
   arg[0].xmax=0; 
   for (ich=0 ; ich<N_CHAN ; ich++) {
     
     if ((~adcDisableMask)&(1<<ich)) {
       if (ich==0) {
	 *retvalsize+=1;
	 if (*retvalsize>MAX_ROIS) {
	   *retvalsize-=1;
	   break;
	 }
	 arg[*retvalsize-1].xmin=ich;
	 arg[*retvalsize-1].xmax=ich; 
       } 
       else {
	 if  ((adcDisableMask)&(1<<(ich-1))) {
	   *retvalsize+=1;
	   if (*retvalsize>MAX_ROIS) {
	     *retvalsize-=1;
	     break;
	   }
	   arg[*retvalsize-1].xmin=ich;
	 }
	 arg[*retvalsize-1].xmax=ich; 	 
       }
     }
   }

   for (ich=0; ich<*retvalsize; ich++) {
     printf("%d xmin %d xmax %d\n", ich, arg[ich].xmin, arg[ich].xmax);
   }

   getDynamicRange();
   return *retvalsize;/*warning: function returns address of local variable*/

}


int loadImage(int index, short int ImageVals[]){

	printf("loadImage Not implemented yet\n");

	/*
	u_int32_t address;
	switch (index) {
	case DARK_IMAGE :
		address = DARK_IMAGE_REG;
		break;
	case GAIN_IMAGE :
		address = GAIN_IMAGE_REG;
		break;
	}
	volatile u_int16_t *ptr;
	ptr=(u_int16_t*)(CSP0BASE+address*2);
#ifdef VERBOSE
	int i;
	for(i=0;i<6;i++)
		printf("%d:%d\t",i,ImageVals[i]);
#endif
	memcpy(ptr,ImageVals ,dataBytes);
#ifdef VERBOSE
	printf("\nLoaded x%08x address with image of index %d\n",(unsigned int)(ptr),index);
#endif
	*/

	return OK;
}



int64_t getProbes(){
  return 0;
}


int setDACRegister(int idac, int val, int imod) {
/*   u_int32_t addr, reg, mask; */
/*   int off; */
/* #ifdef VERBOSE */
/*   if(val==-1) */
/*     printf("Getting dac register%d module %d\n",idac,imod); */
/*   else */
/*     printf("Setting dac register %d module %d to %d\n",idac,imod,val); */
/* #endif */

/*   switch(idac){ */
/*   case 0: */
/*   case 1: */
/*   case 2: */
/*     addr=MOD_DACS1_REG; */
/*     break; */
/*   case 3: */
/*   case 4: */
/*   case 5: */
/*     addr=MOD_DACS2_REG; */
/*     break; */
/*   case 6: */
/*   case 7: */
/*     addr=MOD_DACS3_REG; */
/*     break; */
/*   default: */
/*     printf("weird idac value %d\n",idac); */
/*     return -1; */
/*     break; */
/*   } */
/*   //saving only the msb */
/*   val=val>>2; */

/*   off=(idac%3)*10; */
/*   mask=~((0x3ff)<<off); */

/*   if (val>=0 && val<DAC_DR) { */
/*     reg=bus_r(addr+(imod<<SHIFTMOD)); */
/*       reg&=mask; */
/*       reg|=(val<<off); */
/*       bus_w(addr+(imod<<SHIFTMOD),reg); */
/*   } */
/*   val=(bus_r(addr+(imod<<SHIFTMOD))>>off)&0x3ff; */
/*   //since we saved only the msb */
/*   val=val<<2; */

/*   //val=(bus_r(addr)>>off)&0x3ff; */


/* #ifdef VERBOSE */
/*   printf("Dac %d module %d register is %d\n\n",idac,imod,val); */
/* #endif */
/*    return val; */
}


int getTemperature(int tempSensor, int imod){
  int val;
  imod=0;//ignoring more than 1 mod for now
  int i,j,repeats=6;
  u_int32_t tempVal=0;
#ifdef VERBOSE
  char cTempSensor[2][100]={"ADCs/ASICs","VRs/FPGAs"};
  printf("Getting Temperature of module:%d for the %s for tempsensor:%d\n",imod,cTempSensor[tempSensor],tempSensor);
#endif
  bus_w(TEMP_IN_REG,(T1_CLK_BIT)|(T1_CS_BIT)|(T2_CLK_BIT)|(T2_CS_BIT));//standby
  bus_w(TEMP_IN_REG,((T1_CLK_BIT)&~(T1_CS_BIT))|(T2_CLK_BIT));//high clk low cs

  for(i=0;i<20;i++) {
    //repeats is number of register writes for delay
    for(j=0;j<repeats;j++)
      bus_w(TEMP_IN_REG,~(T1_CLK_BIT)&~(T1_CS_BIT)&~(T2_CLK_BIT)&~(T2_CS_BIT));//low clk low cs
    for(j=0;j<repeats;j++)
      bus_w(TEMP_IN_REG,((T1_CLK_BIT)&~(T1_CS_BIT))|(T2_CLK_BIT));//high clk low cs

    if(i<=10){//only the first time
      if(!tempSensor)
	tempVal= (tempVal<<1) + (bus_r(TEMP_OUT_REG) & (1));//adc
      else
	tempVal= (tempVal<<1) + ((bus_r(TEMP_OUT_REG) & (2))>>1);//fpga
    }
  }

  bus_w(TEMP_IN_REG,(T1_CLK_BIT)|(T1_CS_BIT)|(T2_CLK_BIT)|(T2_CS_BIT));//standby
  val=((int)tempVal)/4.0;

#ifdef VERBOSE
   printf("Temperature of module:%d for the %s is %.2fC\n",imod,cTempSensor[tempSensor],val);
#endif
 return val;
}



int initHighVoltage(int val, int imod){

   
  u_int32_t offw,codata;
  u_int16_t valw, dacvalue;
  int i,ddx,csdx,cdx;//iru,
  float alpha=0.55;//, fval=val;

   if (val!=-1) {
   
     if (val<60) {
       dacvalue=0;
       val=60;
     } else if (val>=200) {
       dacvalue=0x1;
       val=200;
     } else {
       dacvalue=1.+(200.-val)/alpha;
       val=200.-(dacvalue-1)*alpha;
     }
     printf ("****************************** setting val %d, dacval %d\n",val, dacvalue);

     if (val>=0) {

       offw=DAC_REG;
     
       ddx=8; csdx=10; cdx=9;
       codata=((dacvalue)&0xff);
     
    
       valw=bus_r(offw)&0x7fff; //switch off HV 
       bus_w(offw,(valw)); // start point
       valw=((valw&(~(0x1<<csdx))));bus_w(offw,valw); //chip sel bar down
       for (i=0;i<8;i++) {
	valw=(valw&(~(0x1<<cdx)));bus_w(offw,valw); //cldwn
	valw=((valw&(~(0x1<<ddx)))+(((codata>>(7-i))&0x1)<<ddx));bus_w(offw,valw);//write data (i)
	valw=((valw&(~(0x1<<cdx)))+(0x1<<cdx));bus_w(offw,valw);//clkup
       }
       valw=((valw&(~(0x1<<csdx)))+(0x1<<csdx));bus_w(offw,valw); //csup
       
       valw=(valw&(~(0x1<<cdx)));bus_w(offw,valw); //cldwn
       
       
       valw=0xff00|bus_r(offw); //switch on HV 
       bus_w(offw,(valw)); // stop point =start point of course */
       
       printf("Writing %d in HVDAC  \n",dacvalue);
       
       bus_w(HV_REG,val);
       
     } else {
       valw=bus_r(offw)&0x7fff;
       bus_w(offw,(valw));
       bus_w(HV_REG,0);
     }
   } 



  return bus_r(HV_REG);



  //  return val;
}



int initConfGain(int isettings,int val,int imod){
  int retval;
  u_int32_t addr=CONFGAIN_REG;

  if(isettings!=-1){
#ifdef VERBOSE
    printf("Setting Gain of module:%d with val:%d\n",imod,val);
#endif
    bus_w(addr,val);
  }
  retval=(bus_r(addr));
#ifdef VERBOSE
  printf("Value read from Gain reg is %d\n",retval);
#endif 
    return retval;
}



int setADC(int adc){
/* 	int reg,nchips,mask,nchans; */

/* 	if(adc==-1)	ROI_flag=0; */
/* 	else		ROI_flag=1; */

/* 	//	setDAQRegister();//token timing */
/* 	cleanFifo();//adc sync */

/* 	//with gotthard module */
/* 	if(withGotthard){ */
/* 		//set packet size */
/* 		ipPacketSize= DEFAULT_IP_PACKETSIZE; */
/* 		udpPacketSize=DEFAULT_UDP_PACKETSIZE; */
/* 		//set channel mask */
/* 		nchips = GOTTHARDNCHIP; */
/* 		nchans = GOTTHARDNCHAN; */
/* 		mask = ACTIVE_ADC_MASK; */
/* 	} */

/* 	//with moench module all adc */
/* 	else{/\* if(adc==-1){*\/ */
/* 		//set packet size */
/* 		ipPacketSize= DEFAULT_IP_PACKETSIZE; */
/* 		udpPacketSize=DEFAULT_UDP_PACKETSIZE; */
/* 		//set channel mask */
/* 		nchips = N_CHIP; */
/* 		nchans = N_CHANS; */
/* 		mask = ACTIVE_ADC_MASK; */
/* 	}/\* */
/* 	//with moench module 1 adc -- NOT IMPLEMENTED */
/* 	else{ */
/* 		ipPacketSize= ADC1_IP_PACKETSIZE; */
/* 		udpPacketSize=ADC1_UDP_PACKETSIZE; */
/* 		//set channel mask */
/* 		nchips = NCHIPS_PER_ADC; */
/* 		nchans = GOTTHARDNCHAN; */
/* 		mask = 1<<adc; */
/* 	}*\/ */

/* 	//set channel mask */
/* 	reg = (nchans*nchips)<<CHANNEL_OFFSET; */
/* 	reg&=CHANNEL_MASK; */
/* 	reg|=(ACTIVE_ADC_MASK & mask); */
/* 	bus_w(CHIP_OF_INTRST_REG,reg); */

/* //#ifdef DDEBUG */
/* 	printf("Chip of Interest Reg:%x\n",bus_r(CHIP_OF_INTRST_REG)); */
/* //#endif */

/* 	adcConfigured = adc; */

	return adcConfigured;
}



long int calcChecksum(int sourceip, int destip) {


  ip_header ip;
 int count;
 unsigned short *addr;
 long int sum = 0;
 long int checksum;

ip.ip_ver            = 0x4;
ip.ip_ihl            = 0x5;
ip.ip_tos            = 0x0;
ip.ip_len            = 0x2032;//ipPacketSize;//fixed in firmware
ip.ip_ident          = 0x0000;
ip.ip_flag           = 0x2; //not nibble aligned (flag& offset
ip.ip_offset         = 0x000;
ip.ip_ttl            = 0x40;
ip.ip_protocol       = 0x11;
ip.ip_chksum         = 0x0000 ; // pseudo
ip.ip_sourceip       = sourceip;
ip.ip_destip         = destip;


 count=sizeof(ip);
 addr=(unsigned short*) (&(ip)); /* warning: assignment from incompatible pointer type */
  while( count > 1 )  {
    sum += *addr++;
    count -= 2;
  }
  if( count > 0 )  sum += *addr;                     // Add left-over byte, if any
  while (sum>>16) sum = (sum & 0xffff) + (sum >> 16);// Fold 32-bit sum to 16 bits
  checksum = (~sum)&0xffff;

  printf("IP checksum is 0x%lx\n",checksum);

  return checksum;
}



#ifdef NEW_GBE_INTERFACE
int  writeGbeReg(int ivar,  uint32_t val, int addr, int interface) {
/* #define GBE_CTRL_WSTROBE 0  */
/* #define GBE_CTRL_VAR_OFFSET 16 */
/* #define GBE_CTRL_VAR_MASK 0XF */
/* #define GBE_CTRL_RAMADDR_OFFSET 24 */
/* #define GBE_CTRL_RAMADDR_MASK 0X3F */
/* #define GBE_CTRL_INTERFACE 23 */
  uint32_t ctrl=((ivar&GBE_CTRL_VAR_MASK)<<GBE_CTRL_VAR_OFFSET)|((addr&GBE_CTRL_RAMADDR_MASK)<<GBE_CTRL_RAMADDR_OFFSET)| (interface<<GBE_CTRL_INTERFACE);
  bus_w(GBE_CNTRL_REG,ctrl);
  bus_w(GBE_PARAM_REG,val);
  bus_w(GBE_CNTRL_REG,ctrl|(1<<GBE_CTRL_WSTROBE));
  usleep(100);
  bus_w(GBE_CNTRL_REG,ctrl);


}
#endif


int configureInterface(uint32_t destip,uint64_t destmac,uint64_t  sourcemac,int sourceip,int ival,uint32_t destport,  uint32_t sourceport, int interface) {
//int configureMAC(int ipad,long long int macad,long long int detectormacad, int detipad, int ival, int udpport){

 volatile u_int32_t conf= bus_r(CONFIG_REG);
 long int checksum=calcChecksum(sourceip, destip);
 #ifdef NEW_GBE_INTERFACE

 printf("Configure interface %d\n",interface);
 const int nvar=12;
 uint32_t vals[nvar];
 int ivar;
 int addr=0;
 vals[RX_UDP_IP_ADDR]=destip;
 vals[RX_UDP_PORTS_ADDR]=destport;
 vals[RX_UDP_MAC_L_ADDR]=(destmac)&0xFFFFFFFF;
 vals[RX_UDP_MAC_H_ADDR]=(destmac>>32)&0xFFFFFFFF;
 vals[IPCHECKSUM_ADDR]=checksum;
 vals[GBE_DELAY_ADDR]=0;
 vals[GBE_RESERVED1_ADDR]=sourceport;
 vals[GBE_RESERVED2_ADDR]=interface;
 vals[DETECTOR_MAC_L_ADDR]=(sourcemac)&0xFFFFFFFF;
 vals[DETECTOR_MAC_H_ADDR]=(sourcemac>>32)&0xFFFFFFFF;
 vals[DETECTOR_IP_ADDR]=sourceip;

 for (ivar=0; ivar<nvar; ivar++) {
   writeGbeReg(ivar, vals[ivar], addr, interface);
 }
 

#else
  bus_w(DETECTORIP_AREG,sourceip);//detectorip_AReg_c
  bus_w(RX_UDP_AREG,destip);//rx_udpip_AReg_c

  bus_w(RX_UDPMACH_AREG,(destmac>>32)&0xFFFFFFFF);//rx_udpmacH_AReg_c
  bus_w(RX_UDPMACL_AREG,(destmac)&0xFFFFFFFF);//rx_udpmacL_AReg_c
  bus_w(DETECTORMACH_AREG,(sourcemac>>32)&0xFFFFFFFF);//detectormacH_AReg_c
  bus_w(DETECTORMACL_AREG,(sourcemac)&0xFFFFFFFF);//detectormacL_AReg_c
  bus_w(UDPPORTS_AREG,((sourceport&0xFFFF)<<16)+(destport&0xFFFF));//udpports_AReg_c
  bus_w(IPCHKSUM_AREG,(checksum&0xFFFF));//ipchksum_AReg_c


#endif

  bus_w(CONTROL_REG,GB10_RESET_BIT);
  sleep(1);
  bus_w(CONTROL_REG,0);
  usleep(10000);
  bus_w(CONFIG_REG,conf | GB10_NOT_CPU_BIT);
  printf("System status register is %08x\n",bus_r(SYSTEM_STATUS_REG));

return 0; //any value doesnt matter - dhanya

}











int configureMAC(uint32_t destip,uint64_t destmac,uint64_t  sourcemac,int sourceip,int ival,uint32_t destport) {
//int configureMAC(int ipad,long long int macad,long long int detectormacad, int detipad, int ival, int udpport){

  uint32_t sourceport  =  0x7e9a; // 0xE185;
  int interface=0;  
  int ngb;
volatile u_int32_t conf= bus_r(CONFIG_REG);





#ifdef NEW_GBE_INTERFACE
  ngb=2;
  printf("--------- New XGB interface\n");
#else
  ngb=1;
  printf("********* Old XGB interface\n");
#endif

  for (interface=0; interface <ngb; interface++) 
    configureInterface(destip, destmac, sourcemac+interface, sourceip+interface, ival, destport+interface, sourceport+interface, interface);
  

  bus_w(CONTROL_REG,GB10_RESET_BIT);
  bus_w(CONTROL_REG,0);
  usleep(10000);
  bus_w(CONFIG_REG,conf | GB10_NOT_CPU_BIT);
  printf("System status register is %08x\n",bus_r(SYSTEM_STATUS_REG));
  return OK;

}


int getAdcConfigured(){
	return adcConfigured;
}

u_int32_t runBusy(void) {
	u_int32_t s = bus_r(STATUS_REG) & 1;
#ifdef VERBOSE
	printf("status %04x\n",s);
#endif
  return s;
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
#ifdef VERBOSE
  printf("status %04x\n",s);
#endif

/*  if (s==0x62001)
    exit(-1);*/
  return s;
}


// State Machine 

int startStateMachine(){
  //int i;
//#ifdef VERBOSE
  printf("*******Starting State Machine*******\n");
//#endif

//NEEDED?
//	cleanFifo();



  // fifoReset();
  now_ptr=(char*)ram_values;
#ifdef SHAREDMEMORY
  write_stop_sm(0);
  write_status_sm("Started");
#endif


  // for(i=0;i<100;i++){
	  //start state machine
    bus_w16(CONTROL_REG, FIFO_RESET_BIT);
    bus_w16(CONTROL_REG, 0x0);
    bus_w16(CONTROL_REG, START_ACQ_BIT |  START_EXPOSURE_BIT);
    //  usleep(20);
    bus_w16(CONTROL_REG, 0x0);
	  //verify
  /*   if(bus_r(STATUS_REG) & RUN_BUSY_BIT) */
/*       break; */
/*     else { */
/*       printf("status: %08x\n",bus_r(STATUS_REG)); */
/*       usleep(5000); */
/*     } */
/*   } */

/*   if(i!=0) */
/*     printf("tried to start state machine %d times\n",i); */
 
/*   if(i==100){ */
/*      printf("\n***********COULD NOT START STATE MACHINE***************\n"); */
/*      return FAIL; */
/*    } */

   printf("statusreg=%08x\n",bus_r(STATUS_REG));
  return OK;
}




int stopStateMachine(){
	int i;
//#ifdef VERBOSE
  printf("*******Stopping State Machine*******\n");
//#endif
#ifdef SHAREDMEMORY
  write_stop_sm(1);
  write_status_sm("Stopped");
#endif
  // for(i=0;i<100;i++){
  	  //stop state machine
	  bus_w16(CONTROL_REG, STOP_ACQ_BIT);
	  usleep(100);
	  bus_w16(CONTROL_REG, 0x0);
/* 	  usleep(5000); */
/* 	  //verify */
/* 	  if(!(bus_r(STATUS_REG)&RUNMACHINE_BUSY_BIT)) */
/* 		break; */
/*   } */
  if(i!=0)
	  printf("tried to stop state machine %d times\n",i);
  if(i==100){
	  printf("\n***********COULD NOT STOP STATE MACHINE***************\n");
	  return FAIL;
  }

/*
 usleep(5000);
 // if (!runBusy())
  if(!(bus_r(STATUS_REG)&RUNMACHINE_BUSY_BIT))
    return OK;
  else
    return FAIL;
    */
  printf("statusreg=%08x\n",bus_r(STATUS_REG));
  return OK;
}


int startReadOut(){
  u_int32_t status;
#ifdef VERBOSE
  printf("Starting State Machine Readout\n");
#endif
  status=bus_r(STATUS_REG)&RUN_BUSY_BIT;
#ifdef DEBUG
  printf("State machine status is %08x\n",bus_r(STATUS_REG));
#endif
  bus_w16(CONTROL_REG,  START_ACQ_BIT |START_READOUT_BIT);   //  start readout
  usleep(100);
  bus_w16(CONTROL_REG,  0x0);
  return OK;
}


// fifo routines 

u_int32_t fifoReset(void) {
	return -1;
}


u_int32_t setNBits(u_int32_t n) {
	return -1;
}

u_int32_t getNBits(){
	return -1;
}


u_int32_t fifoReadCounter(int fifonum){
	return -1;
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


u_int16_t* fifo_read_event(int ns)
{
  int i=0,  mask=1;//j,
  volatile u_int16_t a;
  bus_w16(DUMMY_REG,0); 
  if (ns==0) {
    a=bus_r16(LOOK_AT_ME_REG);
    while(a==0) {
      if (runBusy()==0) {
	a = bus_r(LOOK_AT_ME_REG);
	if (a==0) {
	printf("no frame found and acquisition finished - exiting\n");
	printf("%08x %08x\n", runState(), bus_r(LOOK_AT_ME_REG));
	return NULL;
	} else {
	  break;
	}
      }
      a = bus_r(LOOK_AT_ME_REG);
      //#ifdef VERBOSE
      //printf(".");
      //#endif
    }
/* #ifdef TIMEDBG  */
/*     //    tsss=tss; */
/*     gettimeofday(&tss,NULL); */
/*     printf("look for data  = %ld usec\n", (tss.tv_usec) - (tse.tv_usec));  */

/*   #endif  */

    //   printf("LAM: %08x\n",a);
  }

  // printf(".");
  a = bus_r(LOOK_AT_ME_REG);

  if (analogEnable) {
  printf("*");
    bus_w16(DUMMY_REG,1<<8); // read strobe to all fifos
    bus_w16(DUMMY_REG,0);
    for (i=0; i<32; i++) {
      if (~(mask&adcDisableMask)) {
	*((u_int16_t*)now_ptr)=*values;//bus_r16(FIFO_DATA_REG);
	if (i!=0 || ns!=0) { 
	  a=0;
	      while (*((u_int16_t*)now_ptr)==*((u_int16_t*)(now_ptr)-1) && a++<10) {
	    	*((u_int16_t*)now_ptr)=*values;
	      }
	    }
	now_ptr+=2;
      } 
      mask=mask<<1;
      //   if (~(mask&adcDisableMask)
      bus_w16(DUMMY_REG,i+1);
    }
  }
  if (digitalEnable) {
    // printf("+");
    
     bus_w16(DUMMY_REG,1<<9); // read strobe to digital fifo
     bus_w16(DUMMY_REG,0<<9); // read strobe to digital fifo
     *((u_int64_t*)now_ptr)=get64BitReg(FIFO_DIGITAL_DATA_LSB_REG,FIFO_DIGITAL_DATA_MSB_REG);
     //bit17 is clkout 
     printf("%d",(*((u_int64_t*)now_ptr)>>17)&1);
     now_ptr+=8;
     
  } 
    //  bus_w16(DUMMY_REG,0); //
/* #ifdef TIMEDBG  */

/*   gettimeofday(&tss,NULL); */
/*   printf("read data loop  = %ld usec\n",(tss.tv_usec) - (tse.tv_usec));  */

/* #endif  */
//#ifdef VERBOSE
      // printf("*");
  //#endif
    //    printf("\n"); 
  return ram_values;
}



u_int16_t* fifo_read_frame()
{
#ifdef TIMEDBG 
  gettimeofday(&tsss,NULL);
#endif   

  // u_int16_t *dum;
  int ns=0;
  now_ptr=(char*)ram_values;
  while(ns<nSamples && fifo_read_event(ns)) {
    // now_ptr+=dataBytes;
      ns++;
  }
#ifdef TIMEDBG 
  // usleep(10);
  gettimeofday(&tss,NULL);
  printf("total read data loop  = %ld usec\n",(tss.tv_usec) - (tsss.tv_usec)); 
   
#endif 
/* #ifdef VERBOSE */
/*   printf("+\n"); */
/* #else */
/*   printf("+\n"); */
/* #endif */
  //  printf("%x %d\n",dum, ns);
  if (ns) return ram_values;
#ifdef VERBOSE
  printf("+\n");
#else
  printf("+");
#endif
  return NULL;
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
    break;
  }

#ifdef VERBOSE
  printf("decoded %d  channels\n",ichan);
#endif
  return dataout;
}



int setDynamicRange(int dr) {
  if (dr%16==0 && dr>0) {
    dynamicRange=16;
    nSamples=dr/16;
    bus_w(NSAMPLES_REG,nSamples);
  } 
  getDynamicRange();
  allocateRAM();
  printf("Setting dataBytes to %d: dr %d; samples %d\n",dataBytes, dynamicRange, nSamples);
  return   getDynamicRange();
}






int getDynamicRange() {
  if(myDetectorType == JUNGFRAU){
    dynamicRange=16;
    return dynamicRange;
  }

  nSamples=bus_r(NSAMPLES_REG);
  getChannels();
  dataBytes=nModX*N_CHIP*getChannels()*2*nSamples;
  printf("data bytes is:%d\n",dataBytes);
  return dynamicRange;//nSamples;
}

int testBus() {
  u_int32_t j;
  u_int64_t i, n, nt;
 // char cmd[100];
  u_int32_t val=0x0;
  int ifail=OK;
  // printf("%s\n",cmd);
  // system(cmd);
  i=0;

  n=1000000;
  nt=n/100;
  printf("testing bus %d times\n",(int)n);
  while (i<n) {
    // val=bus_r(FIX_PATT_REG);
    bus_w(DUMMY_REG,val);
    bus_w(FIX_PATT_REG,0);
    j=bus_r(DUMMY_REG);
    //if (i%10000==1)
    if (j!=val){
      printf("%d :  read wrong value %08x instead of %08x\n",(int)i,j, val);
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

int getChannels() {
  int nch=0;
  int i;
  
  if (analogEnable) {
    nch+=32;
    for (i=0; i<N_CHAN; i++) {
      if (adcDisableMask & (1<<i)) nch--;
    }
  }
  if (digitalEnable)
    nch+=4;
  return nch;
}

int allocateRAM() {
  size_t size;
  getDynamicRange();

  //adcDisableMask 
  size=dataBytes;//*nSamples
  
#ifdef VERBOSE
  printf("\nnmodx=%d nmody=%d dynamicRange=%d dataBytes=%d nFrames=%d nTrains=%d, size=%d\n",nModX,nModY,dynamicRange,dataBytes,nf,nt,(int)size );
#endif

    if (size==ram_size) {

      //#ifdef VERBOSE
      printf("RAM of size %d already allocated: nothing to be done\n",(int) size);
      //#endif
      return OK;
    }



    //#ifdef VERBOSE
      printf("reallocating ram %x, size %d\n",(unsigned int)ram_values, (int)size);
    //#endif
    //+2 was added since dma_memcpy would switch the 16 bit values and the mem is 32 bit   

      //  while (nSamples>1) {
 
      clearRAM();
     ram_values=malloc(size);
     // ram_values=realloc(ram_values,size)+2;
      // if (ram_values)
      //	break;
      // nSamples--;
      //}

  if (ram_values) {
    now_ptr=(char*)ram_values;

    //#ifdef VERBOSE
    printf("ram allocated 0x%x of size %d to %x\n",(int)now_ptr,(unsigned int) size,(unsigned int)(now_ptr+size));
    //#endif
    ram_size=size;
    return OK;
  }
  

  printf("Fatal error: there must be a memory leak somewhere! You can't allocate even one frame!\n");
  return FAIL;




}


int writeADC(int addr, int val) {


  u_int32_t valw,codata,csmask;              
  int i,cdx,ddx;
   cdx=0; ddx=1;
   csmask=0xfc; //  1111100
   
   codata=val + (addr<< 8);   
   printf("***** ADC SPI WRITE TO REGISTER %04X value %04X\n",addr,val);
	// start point
   valw=0xff;
   bus_w16(ADC_WRITE_REG,(valw));
   
   //chip sel bar down
   valw=((0xffffffff&(~csmask)));
   bus_w16(ADC_WRITE_REG,valw);

   for (i=0;i<24;i++) {
     //cldwn
     valw=valw&(~(0x1<<cdx));
     bus_w16(ADC_WRITE_REG,valw);
     // usleep(0);

     //write data (i)
     valw=(valw&(~(0x1<<ddx)))+(((codata>>(23-i))&0x1)<<ddx);
     bus_w16(ADC_WRITE_REG,valw);
     //  usleep(0);

     //clkup
     valw=valw+(0x1<<cdx);
     bus_w16(ADC_WRITE_REG,valw);
     // usleep(0);
   }
   
		 // stop point =start point
   valw=valw&(~(0x1<<cdx));
   // usleep(0);
   valw=0xff;
   bus_w16(ADC_WRITE_REG,(valw));
   
   //usleep in between
   //   usleep(50000);

   return OK;
}

int prepareSlowADCSeq() {

  //  u_int16_t vv=0x3c40;
  u_int16_t codata=( 1<<13) | (7<<10)  | (7<<7) | (1<<6) | (0<<3) | (2<<1) | 1;

  u_int32_t valw;              
  int  obit, ibit;

  int cnv_bit=16, sdi_bit=17, sck_bit=18;
  

  // int oval=0;


  printf("Codata is %04x\n",codata);
  
   /* //convert */
  valw=(1<<cnv_bit); 
  bus_w(ADC_WRITE_REG,valw); 
  
  usleep(20);
  
  valw=0; 
  bus_w(ADC_WRITE_REG,(valw)); 
  
  usleep(20);
    
  for (ibit=0; ibit<14; ibit++) {
    obit=((codata >> (13-ibit)) & 1);
    //   printf("%d",obit);
    valw = obit << sdi_bit;
    
    bus_w(ADC_WRITE_REG,valw);
    
    usleep(20);
    
    bus_w(ADC_WRITE_REG,valw|(1<<sck_bit));
    
    usleep(20);
    
    bus_w(ADC_WRITE_REG,valw);

  }
  //   printf("\n");
  


  bus_w(ADC_WRITE_REG,0);

   /* //convert */
  valw=(1<<cnv_bit); 
  bus_w(ADC_WRITE_REG,valw); 
  
  usleep(20);
  
  valw=0; 
  bus_w(ADC_WRITE_REG,(valw)); 
  
  usleep(20);
  return 0;

}

int prepareSlowADC(int ichan) {

  // u_int16_t vv=0x3c40;
  // u_int16_t codata=( 1<<13) | (7<<10)  | (7<<7) | (1<<6) | (0<<3) | (2<<1) | 1;

    u_int16_t codata=(1<<13) | (7<<10)  | (ichan<<7) | (1<<6) | (0<<3) | (0<<1) | 1; //read single channel
    if (ichan<0) codata=( 1<<13) | (3<<10)  | (7<7) | (1<<6) | (0<<3) | (0<<1) | 1;

    u_int32_t valw;              
  int obit, ibit;

  int cnv_bit=16, sdi_bit=17, sck_bit=18;
  

  //  int oval=0;


  printf("Codata is %04x\n",codata);
  
   /* //convert */
  valw=(1<<cnv_bit); 
  bus_w(ADC_WRITE_REG,valw); 
  
  usleep(20);
  
  valw=0; 
  bus_w(ADC_WRITE_REG,(valw)); 
  
  usleep(20);
    
  for (ibit=0; ibit<14; ibit++) {
    obit=((codata >> (13-ibit)) & 1);
    //   printf("%d",obit);
    valw = obit << sdi_bit;
    
    bus_w(ADC_WRITE_REG,valw);
    
    usleep(20);
    
    bus_w(ADC_WRITE_REG,valw|(1<<sck_bit));
    
    usleep(20);
    
    bus_w(ADC_WRITE_REG,valw);

  }
  //   printf("\n");
  


  bus_w(ADC_WRITE_REG,0);

   /* //convert */
  valw=(1<<cnv_bit); 
  bus_w(ADC_WRITE_REG,valw); 
  
  usleep(20);
  
  valw=0; 
  bus_w(ADC_WRITE_REG,(valw)); 
  
  usleep(20);
  return 0;

}




int readSlowADC(int ichan) {


  // u_int16_t vv=0x3c40;
  //  u_int16_t codata=( 1<<13) | (7<<10)  | (ichan<<7) | (1<<6) | (0<<3) | (0<<1) | 1; //read single channel

  u_int32_t valw;              
  int i, obit;

  int cnv_bit=16,  sck_bit=18;
  

  int oval=0;

  printf("DAC index is %d\n",ichan);

  if (ichan<-1 || ichan>7)
    return -1;


  prepareSlowADC(ichan);


  /* printf("Codata is %04x\n",codata); */
  
  /*  /\* //convert *\/ */
  /* valw=(1<<cnv_bit);  */
  /* bus_w(ADC_WRITE_REG,valw);  */
  
  /* usleep(20); */
  
  /* valw=0;  */
  /* bus_w(ADC_WRITE_REG,(valw));  */
  
  /* usleep(20); */
    
  /* for (ibit=0; ibit<14; ibit++) { */
  /*   obit=((codata >> (13-ibit)) & 1); */
  /*   //   printf("%d",obit); */
  /*   valw = obit << sdi_bit; */
    
  /*   bus_w(ADC_WRITE_REG,valw); */
    
  /*   usleep(20); */
    
  /*   bus_w(ADC_WRITE_REG,valw|(1<<sck_bit)); */
    
  /*   usleep(20); */
    
  /*   bus_w(ADC_WRITE_REG,valw); */

  /* } */
  /* //   printf("\n"); */
  


  /* bus_w(ADC_WRITE_REG,0); */


  


  for (ichan=0; ichan<9; ichan++) {
 
 
   /* //convert */
    valw=(1<<cnv_bit);
    bus_w(ADC_WRITE_REG,valw);
    
    usleep(20);
    
    valw=0;
    bus_w(ADC_WRITE_REG,(valw));
    

    usleep(20);
    //  printf("Channel %d ",ichan);
    //read
    oval=0;
    for (i=0;i<16;i++) {
      //cldwn
      valw=0;
      bus_w(ADC_WRITE_REG,valw);
      bus_w(ADC_WRITE_REG,valw|(1<<sck_bit));
      usleep(20);
      bus_w(ADC_WRITE_REG,valw);
      usleep(20);
      
      obit=bus_r16(SLOW_ADC_REG)&0x1;
      //  printf("%d",obit);
      //write data (i)
      // usleep(0);
      oval|=obit<<(15-i);
      
    }
    printf("\t");
    printf("Value %d  is %d\n",ichan, oval);
    
  }
 
 
  return 2500*oval/65535;
}


int prepareADC(){
  printf("Preparing ADC\n");
  u_int32_t codata,csmask;
  int cdx,ddx;
   cdx=0; ddx=1;
   csmask=0x7c; //  1111100
   
/* #define ADCREG1 			0x08   */
/* #define ADCREG2 			0x14//20  */
/* #define ADCREG3 			0x4   */
/* #define ADCREG4 			0x5   */
    codata=0;    
    writeADC(ADCREG1,0x3); 
writeADC(ADCREG1,0x0);
    

    	writeADC(ADCREG3,0xf);
    	writeADC(ADCREG4,0x3f);

	//   writeADC(0x16,0x01);//output clock phase


	  // writeADC(0x16,0x07);//output clock phase

     //   writeADC(0x16,0x4);//output clock phase

	  //  writeADC(0x18,0x0);// vref 1V
	    
     writeADC(ADCREG2,0x40);//lvds reduced range -- offset binary

     writeADC(0xD,0x0);//no test mode

#ifdef TESTADC


    ////////////TEST ADC!!!!!!!!!!
    
    printf("***************************************** *******\n");
    printf("******* PUTTING ADC IN TEST MODE!!!!!!!!! *******\n");
    printf("***************************************** *******\n");
       

    //    writeADC(0xD,0x4);//ALTERNATING CHECKERBOARD

    //  writeADC(0xD,0x7);//ONE/ZERO WORD TOGGLE

/*     writeADC(0x19,0xf0);//user input */
/*     writeADC(0x1A,0xf0);//user input */
/*     writeADC(0x1B,0x0f);//user input */
/*     writeADC(0x1C,0x0f);//user input */
/*      writeADC(0xD,0x48);//user input, alternate */

/*     //writeADC(0xD,0xA);//1xsync */
//    writeADC(0xD,0xB);//1xbit high 
      writeADC(0xD,0xC);//1xmixed frequqncy 



#endif







    bus_w(ADC_LATCH_DISABLE_REG,0x0); // enable all ADCs
    //  bus_w(DAQ_REG,0x12); //adc pipeline=18

    //bus_w(DAQ_REG,0xbbbbbbbb);
    //   bus_w(ADC_INVERSION_REG,0x1f6170c6);

    return OK;

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
		break;
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
		break;

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
		break;
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
		break;
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



int readCounterBlock(int startACQ, short int CounterVals[]){

/* 	//char *counterVals=NULL; */
/* 	//counterVals=realloc(counterVals,dataBytes); */

/* 	u_int32_t val; */
/* 	volatile u_int16_t *ptr; */

/* 	u_int32_t address = COUNTER_MEMORY_REG; */
/* 	ptr=(u_int16_t*)(CSP0BASE+address*2); */


/* 	if (runBusy()) { */
/* 		if(stopStateMachine()==FAIL) */
/* 			return FAIL; */
/* 		//waiting for the last frame read to be done */
/* 		while(runBusy())  usleep(500); */
/* #ifdef VERBOSE */
/* 		printf("State machine stopped\n"); */
/* #endif */
/* 	} */

/* 		val=bus_r(MULTI_PURPOSE_REG); */
/* #ifdef VERBOSE */
/* 		printf("Value of multipurpose reg:%d\n",bus_r(MULTI_PURPOSE_REG)); */
/* #endif */

/* 		memcpy(CounterVals,ptr,dataBytes); /\*warning: passing argument 2 of ‘memcpy’ discards qualifiers from pointer target type*\/ */
/* #ifdef VERBOSE */
/* 		int i; */
/* 		printf("Copied counter memory block with size of %d bytes..\n",dataBytes); */
/* 		for(i=0;i<6;i++) */
/* 			printf("%d: %d\t",i,CounterVals[i]); */
/* #endif */


/* 		bus_w(MULTI_PURPOSE_REG,(val&~RESET_COUNTER_BIT)); */
/* #ifdef VERBOSE */
/* 		printf("\nClearing bit 2 of multipurpose reg:%d\n",bus_r(MULTI_PURPOSE_REG)); */
/* #endif */

/* 		if(startACQ==1){ */
/* 			startStateMachine(); */
/* 			if(runBusy()) */
/* 				printf("State machine RUNNING\n"); */
/* 			else */
/* 				printf("State machine IDLE\n"); */
/* 		} */

/* /\*		if(sizeof(CounterVals)<=0){ */
/* 			printf("ERROR:size of counterVals=%d\n",(int)sizeof(CounterVals)); */
/* 			return FAIL; */
/* 		}*\/ */


	return OK;
}




int resetCounterBlock(int startACQ){
	int ret = OK; 

/* 	char *counterVals=NULL; */
/* 	counterVals=realloc(counterVals,dataBytes); */

/* 	u_int32_t val; */
/* 	volatile u_int16_t *ptr; */


/* 	u_int32_t address = COUNTER_MEMORY_REG; */
/* 	ptr=(u_int16_t*)(CSP0BASE+address*2); */


/* 	if (runBusy()) { */
/* 		if(stopStateMachine()==FAIL) */
/* 			return FAIL; */
/* 		//waiting for the last frame read to be done */
/* 		while(runBusy())  usleep(500); */
/* #ifdef VERBOSE */
/* 		printf("State machine stopped\n"); */
/* #endif */
/* 	} */

/* 		val=bus_r(MULTI_PURPOSE_REG); */
/* #ifdef VERBOSE */
/* 		printf("Value of multipurpose reg:%d\n",bus_r(MULTI_PURPOSE_REG)); */
/* #endif */


/* 		bus_w(MULTI_PURPOSE_REG,(val|RESET_COUNTER_BIT)); */
/* #ifdef VERBOSE */
/* 		printf("Setting bit 2 of multipurpose reg:%d\n",bus_r(MULTI_PURPOSE_REG)); */
/* #endif */


/* 		memcpy(counterVals,ptr,dataBytes);/\*warning: passing argument 2 of ‘memcpy’ discards qualifiers from pointer target type*\/ */
/* #ifdef VERBOSE */
/* 		int i; */
/* 		printf("Copied counter memory block with size of %d bytes..\n",(int)sizeof(counterVals)); */
/* 		for(i=0;i<6;i=i+2) */
/* 			printf("%d: %d\t",i,*(counterVals+i)); */
/* #endif */


/* 		bus_w(MULTI_PURPOSE_REG,(val&~RESET_COUNTER_BIT)); */
/* #ifdef VERBOSE */
/* 		printf("\nClearing bit 2 of multipurpose reg:%d\n",bus_r(MULTI_PURPOSE_REG)); */
/* #endif */

/* 		if(startACQ==1){ */
/* 			startStateMachine(); */
/* 			if(runBusy()) */
/* 				printf("State machine RUNNING\n"); */
/* 			else */
/* 				printf("State machine IDLE\n"); */
/* 		} */

/* 		if(sizeof(counterVals)<=0){ */
/* 			printf("ERROR:size of counterVals=%d\n",(int)sizeof(counterVals)); */
/* 			ret = FAIL; */
/* 		} */

		return ret;

	}



int calibratePedestal(int frames){
  printf("---------------------------\n");
  printf("In Calibrate Pedestal\n");
  int64_t framesBefore = getFrames();
  int64_t periodBefore = getPeriod();
  setFrames(frames);
  setPeriod(1000000);
  int dataret = OK;

  double avg[1280];
  int numberFrames = 0;

  int adc = 3;
  int adcCh = 3;
  int Ch = 3;
 

  int i = 0;
  for(i =0; i < 1280; i++){
    
    avg[i] = 0.0;
  }

  startReceiver(0);
	
  startStateMachine();

  while(dataret==OK){
    //got data
    if (fifo_read_event(0)) {
      dataret=OK;
      //sendDataOnly(file_des,&dataret,sizeof(dataret));
      //sendDataOnly(file_des,dataretval,dataBytes);
      printf("received frame\n");
	
      unsigned short *frame = (unsigned short *)now_ptr;

      int a;
      for (a=0;a<1280; a++){
	//unsigned short v = (frame[a] << 8) + (frame[a] >> 8);
	//	  printf("%i: %i %i\n",a, frame[a],v);
	avg[a] += ((double)frame[a])/(double)frames;
	//if(frame[a] == 8191)
	//  printf("ch %i: %u\n",a,frame[a]);
      }
      //      printf("********\n");
      numberFrames++;
    }  

    //no more data or no data
    else {
      if(getFrames()>-2) {
	dataret=FAIL;
	printf("no data and run stopped: %d frames left\n",(int)(getFrames()+2));
	     
      } else {
	dataret=FINISHED;
	printf("acquisition successfully finished\n");

      }
      printf("dataret %d\n",dataret);
    }
  }

  

  //double nf = (double)numberFrames;
  for(i =0; i < 1280; i++){
    adc = i / 256;
    adcCh = (i - adc * 256) / 32;
    Ch = i - adc * 256 - adcCh * 32;
    adc--;
    double v2 = avg[i];
    avg[i] = avg[i]/ ((double)numberFrames/(double)frames);
    unsigned short v = (unsigned short)avg[i];
    printf("setting avg for channel %i(%i,%i,%i): %i (double= %f (%f))\t", i,adc,adcCh,Ch, v,avg[i],v2);
    v=i*100;
    ram_w16(DARK_IMAGE_REG,adc,adcCh,Ch,v-4096);
    if(ram_r16(DARK_IMAGE_REG,adc,adcCh,Ch) !=  v-4096){
        printf("value is wrong (%i,%i,%i): %i \n",adc,adcCh,Ch,  ram_r16(DARK_IMAGE_REG,adc,adcCh,Ch));
    }
  }

      /*for(adc = 1; adc < 5; adc++){
    for(adcCh = 0; adcCh < 8; adcCh++){
      for(Ch=0 ; Ch < 32; Ch++){
	int channel = (adc+1) * 32 * 8  + adcCh * 32 + Ch;
	double v2 = avg[channel];
	avg[channel] = avg[channel]/ ((double)numberFrames/(double)frames);
	unsigned short v = (unsigned short)avg[channel];
	printf("setting avg for channel %i: %i (double= %f (%f))\t", channel, v,avg[channel],v2);
	ram_w16(DARK_IMAGE_REG,adc,adcCh,Ch,v-4096);
	if(ram_r16(DARK_IMAGE_REG,adc,adcCh,Ch) !=  v-4096){
	  printf("value is wrong (%i,%i,%i): %i \n",adc,adcCh,Ch,  ram_r16(DARK_IMAGE_REG,adc,adcCh,Ch));
	}
      }
    }
    }*/



  printf("frames: %i\n",numberFrames);	
  printf("corrected avg by: %f\n",(double)numberFrames/(double)frames);
  
  printf("restoring previous condition\n");
  setFrames(framesBefore);
  setPeriod(periodBefore); 
  
  printf("---------------------------\n");
  return 0;
}
uint64_t readPatternWord(int addr) {
  uint64_t word=0;
  int cntrl=0;

  if (addr>=MAX_PATTERN_LENGTH)
    return -1;
 
  
  printf("read %x\n",addr);
    cntrl= (addr&APATTERN_MASK) << PATTERN_CTRL_ADDR_OFFSET;
    bus_w(PATTERN_CNTRL_REG, cntrl);
    usleep(1000);
    bus_w(PATTERN_CNTRL_REG, cntrl | (1<< PATTERN_CTRL_READ_BIT)  );
    usleep(1000);
    printf("reading\n");
    word=get64BitReg(PATTERN_OUT_LSB_REG,PATTERN_OUT_MSB_REG);
    printf("read %llx\n", word);
    usleep(1000);
    bus_w(PATTERN_CNTRL_REG, cntrl);
    printf("done\n");
    
    return word;
}

uint64_t writePatternWord(int addr, uint64_t word) {
  

  int cntrl=0;
  if (addr>=MAX_PATTERN_LENGTH)
    return -1;

  printf("write %x %llx\n",addr, word);
  if (word!=-1){
    
    set64BitReg(word,PATTERN_IN_REG_LSB,PATTERN_IN_REG_MSB);

  
    cntrl= (addr&APATTERN_MASK) << PATTERN_CTRL_ADDR_OFFSET;
    bus_w(PATTERN_CNTRL_REG, cntrl);
    usleep(1000);
    bus_w(PATTERN_CNTRL_REG, cntrl | (1<< PATTERN_CTRL_WRITE_BIT)  );
    usleep(1000);
    bus_w(PATTERN_CNTRL_REG, cntrl);
    return word;
  } else
  return readPatternWord(addr);
}
uint64_t writePatternIOControl(uint64_t word) {
  uint64_t c=0xffffffffffffffffULL;
  if (word!=c) { /*warning: integer constant is too large for ‘long’ type*/
    //    printf("%llx %llx %lld",get64BitReg(PATTERN_IOCTRL_REG_LSB,PATTERN_IOCTRL_REG_MSB),word);
    set64BitReg(word,PATTERN_IOCTRL_REG_LSB,PATTERN_IOCTRL_REG_MSB);
    //  printf("************ write IOCTRL (%x)\n",PATTERN_IOCTRL_REG_MSB);
  }
  return get64BitReg(PATTERN_IOCTRL_REG_LSB,PATTERN_IOCTRL_REG_MSB);
    
}
uint64_t writePatternClkControl(uint64_t word) {
  uint64_t c=0xffffffffffffffffULL;
  if (word!=c) set64BitReg(word,PATTERN_IOCLKCTRL_REG_LSB,PATTERN_IOCLKCTRL_REG_MSB);/*warning: integer constant is too large for ‘long’ type*/
  return get64BitReg(PATTERN_IOCLKCTRL_REG_LSB,PATTERN_IOCLKCTRL_REG_MSB);
    
}

int setPatternLoop(int level, int *start, int *stop, int *n) {
  int ret=OK;
  int lval=0;

  int nreg;
  int areg;

  switch (level ) {
  case 0:
    nreg=PATTERN_N_LOOP0_REG;
    areg=PATTERN_LOOP0_AREG;
    break;
  case 1:
    nreg=PATTERN_N_LOOP1_REG;
    areg=PATTERN_LOOP1_AREG;
    break;
  case 2:
    nreg=PATTERN_N_LOOP2_REG;
    areg=PATTERN_LOOP2_AREG;
    break;
  case -1:
    nreg=-1;
    areg=PATTERN_LIMITS_AREG;
    break;
  default:
    return FAIL;
  }

  printf("level %d start %x stop %x nl %d\n",level, *start, *stop, *n);
  if (nreg>=0) {
    if ((*n)>=0) bus_w(nreg, *n);
    printf ("n %d\n",*n);
    *n=bus_r(nreg);
    printf ("n %d\n",*n);

  }

    printf("level %d start %x stop %x nl %d\n",level, *start, *stop, *n);
    lval=bus_r(areg);
/*     printf("l=%x\n",bus_r16(areg)); */
/*     printf("m=%x\n",bus_r16_m(areg)); */
    




    printf("lval %x\n",lval);
    if (*start==-1) *start=(lval>> ASTART_OFFSET)   & APATTERN_MASK;
    printf("start %x\n",*start);

    
    if (*stop==-1) *stop=(lval>> ASTOP_OFFSET)   & APATTERN_MASK;
    printf("stop %x\n",*stop);

    lval= ((*start & APATTERN_MASK) << ASTART_OFFSET) | ((*stop & APATTERN_MASK) << ASTOP_OFFSET);
    printf("lval %x\n",lval);

    bus_w(areg,lval);
    printf("lval %x\n",lval);


  return ret;
}


int setPatternWaitAddress(int level, int addr) { 
  int reg;

  switch (level) {
  case 0:
    reg=PATTERN_WAIT0_AREG;
    break;
  case 1:
    reg=PATTERN_WAIT1_AREG;
    break;
  case 2:
    reg=PATTERN_WAIT2_AREG;
    break;
  default:
    return -1;
  };
  //  printf("BEFORE *********PATTERN IOCTRL IS %llx (%x)\n",writePatternIOControl(-1), PATTERN_IOCTRL_REG_MSB);

  // printf ("%d addr %x (%x)\n",level,addr,reg);
  if (addr>=0) bus_w(reg, addr);
  // printf ("%d addr %x %x (%x) \n",level,addr, bus_r(reg), reg);

  // printf("AFTER *********PATTERN IOCTRL IS %llx (%x)\n",writePatternIOControl(-1), PATTERN_IOCTRL_REG_MSB);

  return bus_r(reg);
}


uint64_t setPatternWaitTime(int level, uint64_t t) { 
  int reglsb;
  int regmsb;


  switch (level) {
  case 0:
    reglsb=PATTERN_WAIT0_TIME_REG_LSB;
    regmsb=PATTERN_WAIT0_TIME_REG_MSB;
    break;
  case 1:
    reglsb=PATTERN_WAIT1_TIME_REG_LSB;
    regmsb=PATTERN_WAIT1_TIME_REG_MSB;
    break;
  case 2:
    reglsb=PATTERN_WAIT2_TIME_REG_LSB;
    regmsb=PATTERN_WAIT2_TIME_REG_MSB;
    break;
  default:
    return -1;
  }


    if (t>=0) set64BitReg(t,reglsb,regmsb);
    return get64BitReg(reglsb,regmsb);

}


void initDac(int dacnum) {


  u_int32_t offw,codata;
  u_int16_t valw; 
  int i,ddx,csdx,cdx;
 
    

  //setting int reference 
  offw=DAC_REG;
   
 
    ddx=0; cdx=1;
    csdx=dacnum/8+2; 


    printf("data bit=%d, clkbit=%d, csbit=%d",ddx,cdx,csdx);
    codata=(((0x6)<<4)|((0xf)<<16)|((0x0<<4)&0xfff0));  
  
    valw=0x00ff|(bus_r(offw)&0xff00); 
    bus_w(offw,(valw)); // start point
    valw=((valw&(~(0x1<<csdx))));bus_w(offw,valw); //chip sel bar down
    for (i=1;i<25;i++) {
      
      valw=(valw&(~(0x1<<cdx)));bus_w(offw,valw); //cldwn	
      valw=((valw&(~(0x1<<ddx)))+(((codata>>(24-i))&0x1)<<ddx));bus_w(offw,valw);//write data (i)
      //	printf("%d ", ((codata>>(24-i))&0x1));
      valw=((valw&(~(0x1<<cdx)))+(0x1<<cdx));bus_w(offw,valw);//clkup
    }
      // printf("\n ");
    
    
    valw=((valw&(~(0x1<<csdx)))+(0x1<<csdx));bus_w(offw,valw); //csup
    
    valw=(valw&(~(0x1<<cdx)));bus_w(offw,valw); //cldwn




    valw=0x00ff|(bus_r(offw)&0xff00);  
    bus_w(offw,(valw)); // stop point =start point of course */


 //end of setting int reference 







}

int setDacRegister(int dacnum,int dacvalue) {
  //  int val;
 /*    if (dacvalue==-100) */
/*       dacvalue=0xffff; */


/*   if (dacnum%2) { */
/*     val=((dacvalue & 0xffff)<<16) | getDacRegister(dacnum-1); */
/*   }  else { */
/*     val=(getDacRegister(dacnum+1)<<16) | (dacvalue & 0xffff); */
    
/*   } */
  
/*   printf("Dac register %x wrote %08x\n",(DAC_REG_OFF+dacnum/2)<<11,val); */
/*     bus_w((DAC_REG_OFF+dacnum/2)<<11, val); */
    

  bus_w(DAC_NUM_REG, dacnum);
  bus_w(DAC_VAL_REG, dacvalue);
  bus_w(DAC_NUM_REG, dacnum | (1<<16));
  bus_w(DAC_NUM_REG, dacnum);
  printf("Wrote dac register value %d address %d\n",bus_r(DAC_VAL_REG),bus_r(DAC_NUM_REG)) ;
  return getDacRegister(dacnum);
  

}
int getDacRegister(int dacnum) {

  
  bus_w(DAC_NUM_REG, dacnum);
  printf("READ dac register value %d address %d\n",bus_r(DAC_VAL_OUT_REG),bus_r(DAC_NUM_REG)) ;
  return bus_r(DAC_VAL_OUT_REG);
/* #define DAC_VAL_REG 121<<11 */
/* #define DAC_NUM_REG 122<<11 */
/* #define DAC_VAL_OUT_REG 42<<11 */
/*   retval=bus_r((DAC_REG_OFF+dacnum/2)<<11); */
/*   printf("Dac register %x read %08x\n",(DAC_REG_OFF+dacnum/2)<<11,retval); */
/*   if (dacnum%2)  */
/*     return (retval>>16)&0xffff; */
/*   else */
/*     return retval&0xffff; */
  
}




int setPower(int ind, int val) {
  int dacindex=-1;
  int dacval=-1;
  int pwrindex=-1;
  int retval=-1;
  int retval1=-1;

  u_int32_t preg;

  int vchip=2700-(getDacRegister(19)*1000)/4095;
  int vmax=vchip-200;
  int vmin=600;
  
  printf("---------------------------------------------Current V_Chip is %d mV\n",vchip);

  switch (ind) {

  case V_POWER_CHIP:
    dacindex=19;
    pwrindex=-1;
    break;
  case V_POWER_A:
    dacindex=22;
    pwrindex=1;
    break;
  case V_POWER_B:
    dacindex=21;
    pwrindex=2;
    break;
  case V_POWER_C:
    dacindex=20;
    pwrindex=3;
    break;
  case V_POWER_D:
    dacindex=18;
    pwrindex=4;
    break;
  case V_POWER_IO:
    dacindex=23;
    pwrindex=0;
    break;
  case V_LIMIT:
    dacindex=-1;
    pwrindex=-1;
    break;
  default:
    pwrindex=-1;
    dacindex=-1;
  }

  if (val==-1) {
    printf("get\n");
    dacval=-1;
  } else {
    if (pwrindex>=0) {
      printf("vpower\n");
	dacval=((vmax-val)*4095)/(vmax-vmin);
	if (dacval<0)
	  dacval=0;
	if (dacval>4095)
	  dacval=-100;
	if (val==-100)
	  dacval=-100;
     
    
    } else if (dacindex>=0) {
      printf("vchip\n");
	dacval=((2700-val)*4095)/1000;
	if (dacval<0)
	  dacval=0;
	if (dacval>4095)
	  dacval=4095;
      
    } else {
      vLimit=val;
      printf("vlimit %d\n",vLimit );
    }
      
  }

  if (pwrindex>=0 && val!=-1) {
    preg=bus_r(POWER_ON_REG);
    printf("power reg is %08x\n",bus_r(POWER_ON_REG));
    printf("Switching off power %d\n", pwrindex);
    bus_w(POWER_ON_REG,preg&(~(1<<(16+pwrindex))));
    setDac(dacindex,-100);
    printf("power reg is %08x\n",bus_r(POWER_ON_REG));
    retval=0;
  }
  
  if (dacindex>0 && dacval!=-100) {
    
    printf("Setting power %d to %d mV\n",ind,val);
    printf("Setting DAC %d to value %d\n",dacindex,dacval);
    retval=setDac(dacindex,dacval);
    if (pwrindex>=0 && dacval>=0 ) {
      preg=bus_r(POWER_ON_REG);
      printf("power reg is %08x\n",bus_r(POWER_ON_REG));
      printf("Switching on power %d\n", pwrindex);
      bus_w(POWER_ON_REG,preg|((1<<(16+pwrindex))));
      printf("power reg is %08x\n",bus_r(POWER_ON_REG));
    }
  }
  
  if (pwrindex>=0) {
    if (bus_r(POWER_ON_REG)&(1<<(16+pwrindex))){
      vmax=2700-(getDacRegister(19)*1000)/4095-200;
      printf("Vchip id %d mV\n",vmax+200);
      retval1=vmax-(retval*(vmax-vmin))/4095;
      printf("Vdac id %d mV\n",retval1);
	if (retval1>vmax)
	  retval1=vmax;
	if (retval1<vmin)
	  retval1=vmin;
	if (retval<0)
	  retval1=retval;
    } else
      retval1=0;
  } else if (dacindex>=0) {
    if (retval>=0) {
      retval1=2700-(retval*1000)/4095;
      printf("Vchip is %d mV\n",vmax);
    } else
      retval1=-1;
  } else {
    printf("Get vlimit %d\n",vLimit);
    retval=vLimit;
    retval1=vLimit;
  }

 /*  switch (ind) { */
/*   case V_POWER_A: */
/*     break; */
/*   case V_POWER_B: */
/*     break; */
/*   case V_POWER_C: */
/*     break; */
/*   case V_POWER_D: */
/*     break; */
/*   case V_POWER_IO: */
/*     break; */
/*   case V_POWER_CHIP: */
/*     break;	       */
/*   default: */
/*     retval1=retval; */
/*   } */


  return retval1;


}



int powerChip(int arg) {
  //#ifndef CTB
  
  u_int32_t preg=bus_r(POWER_ON_REG); 
  if (myDetectorType!=JUNGFRAUCTB) {
    if (arg>=0) {
      if (arg)
	bus_w(POWER_ON_REG,preg|0xffff0000);
      else
	bus_w(POWER_ON_REG,preg&0x0000ffff);
      preg=bus_r(POWER_ON_REG);
    }
  }
  printf("Power register is %08x\n",preg);
  if (preg&0xffff0000)
    return 1;
  else
    return 0;
}



int vLimitCompliant(int val_mV) {
  int ret=0;

  if (vLimit>0) {
    if (val_mV<=vLimit) ret=1;
  } else ret=1;
  
  return ret;
      

}


int dacSPI(int codata) {
  u_int32_t offw;
  int valw, vv; 
  int i, ddx,cdx;

  ddx=0; cdx=1;

  offw=DAC_REG;
  valw=bus_r(offw); 
  // codata=((cmd&0xf)<<DAC_CMD_OFF)|((val<<4)&0xfff0);
  printf("%08x\n",codata);
  valw=bus_r(offw);

  for (i=1;i<33;i++) {    

    if ((codata&(1<<(32-i)))) {
      vv=valw|(0x1<<ddx);
    }  else {
      vv=valw&(~(0x1<<ddx));
    }
    printf("%x",vv&0x1);
    bus_w16(offw,vv);//data
    bus_w16(offw,vv|(0x1<<cdx));//clkup
    bus_w16(offw,vv&(~(0x1<<cdx))); //cldwn

  } 
  printf("\n");
  return 1;
  

}





int nextDac(){ 


  return dacSPI(0xf<<DAC_CMD_OFF);

}





int setThisDac(int dacnum,  int dacvalue){ 

  u_int32_t codata, cmd; 
  /* u_int16_t valw=bus_r(offw);  */
  /* int i,ddx,csdx,cdx; */

  int dacch;
  /* //int val=dacvalue; */

  



  /* ddx=0; cdx=1; */


  /* if  (myDetectorType==JUNGFRAUCTB) */
  /*   csdx=2;  */
  /* else */
  /*   csdx=dacnum/8+2;  */

   dacch=dacnum%8; 
  /* //setting int reference  */
  /* offw=DAC_REG; */

  /* valw= bus_r(offw);//0xffff; */

  /* // if  (myDetectorType==JUNGFRAUCTB)  */
  /*   for (i=0;i<8;i++) {  */
  /*     valw=(valw&(~(0x1<<cdx))); */
  /*     bus_w(offw,valw); //cldwn  */
  /*     valw=(valw|(0x1<<cdx)); */
  /*     bus_w(offw,valw);//clkup  */
  /*   } */
  
  /* //  printf("**************************************************set dac\n"); */
  if (dacvalue>=0) {
    
    cmd=0x3;
   
  } else if (dacvalue==-100) {

    cmd=0x4;

  }
  codata=cmd<<DAC_CMD_OFF;
  codata|=(dacch&0xf)<<16;
  codata|=(dacvalue&0xfff)<<4;

  return dacSPI(codata);
  /* //  printf("%08x\n",codata); */
  /* codata=codata|((((dacch)&0xf))<<16)|((dacvalue<<4)&0xfff0);  */
  /* printf("%08x\n",codata); */


  /* for (i=1;i<25;i++) { */
  /*   //   printf("%d",((codata>>(24-i)))&0x1); */
  /*   valw=(valw&(~(0x1<<cdx)));bus_w(offw,valw); //cldwn	 */
    
  /*   valw=((valw&(~(0x1<<ddx)))+(((codata>>(24-i))&0x1)<<ddx)); */
    
  /*   bus_w(offw,valw);//write data (i) */
  /* 	//	printf("%d ", ((codata>>(24-i))&0x1)); */
    
    
  /*   valw=((valw)|(0x1<<cdx));bus_w(offw,valw);//clkup */
  /* } */
  
  /*     //      printf("\n "); */
    
  /*     //    printf("Writing %d in DAC(0-15) %d \n",dacvalue,dacnum); */
     
	

    
  /*     //   printf("Writing %d in DAC(0-15) %d \n",dacvalue,dacnum); */
  /* return 0; */
}










int setDac(int dacnum,int dacvalue){ 

  u_int32_t offw;
  u_int32_t ichip;
  u_int16_t valw; 
  int i,ddx,csdx,cdx;

  
  if  (myDetectorType==JUNGFRAUCTB)
    ichip=2-dacnum/8;
  else 
     ichip=dacnum/8;
  
  if  (myDetectorType==JUNGFRAUCTB) printf("This is a CTB\n");
  else printf("This is not a CTB\n");


  if (dacvalue!=-1) {
    printf("************** set dac %d chip %d value %d ------ %d\n", dacnum, ichip, dacvalue, DAC_CMD_OFF);
    ddx=0; cdx=1;
    if  (myDetectorType==JUNGFRAUCTB) 
      csdx=2; 
    else 
      csdx=ichip+2; 
  //setting int reference 
  offw=DAC_REG;


  valw=bus_r(offw)|0xff;
  
  bus_w(offw,(valw)); // start point
  valw=((valw&(~(0x1<<csdx))));bus_w(offw,valw); //chip sel bar down
  valw=(valw&(~(0x1<<cdx)));bus_w(offw,valw); //cldwn

    //#ifdef CTB  
    if  (myDetectorType==JUNGFRAUCTB) {
      for (i=0; i<ichip; i++) {
	nextDac();
	printf("next DAC\n");
      }
    }
    //#endif
    printf("--------thisdac %d \n",i);
    
    setThisDac(dacnum,dacvalue);
    //    printf("--------thisdac %d ",i);

    printf("--------done %d \n",i);
    
    if  (myDetectorType==JUNGFRAUCTB) {
      for (i=ichip+1; i<N_DAC/8; i++) {
	nextDac();
	printf("next DAC\n");
      }
    }
    valw=bus_r(offw);
    valw=(valw&(~(0x1<<cdx)));bus_w(offw,valw); //cldwn
    valw=(valw|(0x1<<csdx));
    bus_w(offw,valw); //csup
    
    valw=bus_r(offw)|0xff; 
    bus_w(offw,(valw)); // stop point =start point of course */
     

    setDacRegister(dacnum,dacvalue); 

  }

  return getDacRegister(dacnum); 


}

int setReadOutMode(int arg) {

  //#define ADC_OUTPUT_DISABLE_BIT 0x00100
  //#define DIGITAL_OUTPUT_ENABLE_BIT 0x00200
  int v=bus_r(CONFIG_REG)&(~ADC_OUTPUT_DISABLE_BIT)&(~DIGITAL_OUTPUT_ENABLE_BIT);
  int v1;
  printf("before: %x %x\n",bus_r(CONFIG_REG),v);
  switch (arg) {
  case NORMAL_READOUT:
    bus_w(CONFIG_REG, v);
    break;
  case DIGITAL_ONLY:
    bus_w(CONFIG_REG,v|ADC_OUTPUT_DISABLE_BIT|DIGITAL_OUTPUT_ENABLE_BIT);
    break;
  case ANALOG_AND_DIGITAL:
    bus_w(CONFIG_REG,v|DIGITAL_OUTPUT_ENABLE_BIT);
    break;
  default:
    ;
  }
  
  printf("after: %x\n",bus_r(CONFIG_REG));

  switch((bus_r(CONFIG_REG)>>8)&0x3) {
  case 0:
    analogEnable=1;
    digitalEnable=0;
    v1=NORMAL_READOUT;
    break;
  case 3: 
    analogEnable=0;
    digitalEnable=1;
    v1=DIGITAL_ONLY;
    break;
  case 2:
    analogEnable=1;
    digitalEnable=1;
    v1=ANALOG_AND_DIGITAL;
    break;
  default:
    printf("Unknown readout mode for analog and digital fifos %d\n",(bus_r(CONFIG_REG)>>8)&0x3);
    v1=GET_READOUT_FLAGS;
  }
  getDynamicRange();
  allocateRAM();
  printf("dataBytes is %d\n",dataBytes);
  return v1;

}
