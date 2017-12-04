#ifdef MCB_FUNCS

#ifndef MCB_FUNCS_H
#define MCB_FUNCS_H

#include "sls_detector_defs.h"

#define RGPRVALS {100,50,200}
#define RGSH1VALS {300,200,400}
#define RGSH2VALS {260,300,260}

#define DEFAULTGAIN {11.66,9.32,14.99}
#define DEFAULTOFFSET {817.5,828.6,804.2}



// DAC definitions

enum {VTRIM,VTHRESH,RGSH1,RGSH2,VCAL,RGPR};

/* DAC adresses */
#define DACCS {0,0,1,1,2,2}
#define DACADDR {0,1,0,1,0,1}

//dynamic range
#define MAX5523
#ifndef MAX5523
#define MAX5533
#endif
#ifdef MAX5533
#define DAC_DR 4096
#endif
#ifdef MAX5523
#define DAC_DR 1024
#endif


//reference voltage
#define DAC_REFOUT1
#ifdef DAC_REFOUT2 
#define DAC_MAX 2.425
#define DAC_REFOUT 2 
#define DAC_REFOUT1
#endif
#ifdef DAC_REFOUT3 
#define DAC_MAX 3.885
#define DAC_REFOUT 3
#define DAC_REFOUT1
#endif
#ifdef DAC_REFOUT0
#define DAC_MAX 1.214
#define DAC_REFOUT 0
#endif
#ifdef DAC_REFOUT1
#define DAC_MAX 1.940
#define DAC_REFOUT 1
#endif

/* dac calibration constants */

#define VA 1.11

#define CVTRIM  52.430851
#define BVTRIM  -0.102022
#define AVTRIM  0.000050

#define PARTREF {100,1.55,-2.5,-2.5,0,-2.5}
#define PARTR1 {78,10,10,10,10,10}
#define PARTR2 {0,4.7,27,47,22,47}


//chip shiftin register meaning
#define OUTMUX_OFFSET 20
#define PROBES_OFFSET 4
#define OUTBUF_OFFSET 0


int initDetector();
int copyChannel(sls_detector_channel *destChan, sls_detector_channel *srcChan);
int copyChip(sls_detector_chip *destChip, sls_detector_chip *srcChip);
int copyModule(sls_detector_module *destMod, sls_detector_module *srcMod);

/* Register commands */
int clearDACSregister(int imod );
int nextDAC(int imod );
int clearCSregister(int imod );
int setCSregister(int imod );
int nextChip(int imod );
int firstChip(int imod );
int clearSSregister(int imod );
int setSSregister(int imod );
int nextStrip(int imod );
int selChannel(int strip,int imod );
int selChip(int chip,int imod );
int selMod(int mod,int imod );

/* DACs routines */
int program_one_dac(int addr, int value,int imod );
int set_one_dac(int imod);
int initDAC(int dac_cs, int dac_addr, int value,int imod );
int initDACs(int* v,int imod );
int setSettings(int i);
dacs_t initDACbyIndex(int ind,dacs_t val, int imod);
dacs_t initDACbyIndexDACU(int ind,dacs_t val, int imod);
dacs_t getDACbyIndexDACU(int ind,  int imod);
int getThresholdEnergy();
int setThresholdEnergy(int ethr);
/* Initialization*/
int initChannel(int ft,int cae, int ae, int coe, int ocoe, int counts,int imod );
int initChannelbyNumber(sls_detector_channel myChan);
int getChannelbyNumber(sls_detector_channel*);

int getTrimbit(int imod, int ichip, int ichan);

int initChip(int obe, int ow,int imod );

int initChipWithProbes(int obe, int ow,int nprobes, int imod);
//int getNProbes();

int initChipbyNumber(sls_detector_chip myChip);
int getChipbyNumber(sls_detector_chip*);
int initMCBregisters(int cm,int imod );
int initModulebyNumber(sls_detector_module);
int getModulebyNumber(sls_detector_module*);

/* To chips */
int clearCounter(int imod );
int clearOutReg(int imod);
int setOutReg(int imod );
int extPulse(int ncal,int imod );
int calPulse(int ncal,int imod );
int counterClear(int imod );
int countEnable(int imod );
int counterSet(int imod );


/* moved from firmware_funcs */

int readOutChan(int *val);

int getModuleNumber(int modnum);
int testShiftIn(int imod);
int testShiftOut(int imod);
int testShiftStSel(int imod);
int testDataInOut(int num, int imod);
int testExtPulse(int imod);
int testExtPulseMux(int imod, int ow);
int testDataInOutMux(int imod, int ow, int num);
int testOutMux(int imod);
int testFpgaMux(int imod);
int calibration_sensor(int num, int *values, int *dacs) ;
int calibration_chip(int num, int *values, int *dacs);


#endif

#endif
