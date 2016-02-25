#ifdef MCB_FUNCS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "registers_m.h"

#ifndef PICASSOD
#include "server_defs.h"
#else
#include "picasso_defs.h"
#endif
#include "firmware_funcs.h"
#include "mcb_funcs.h"


/* global variables */
#undef DEBUG
#undef DEBUGOUT

extern int nModX;
//extern int dataBytes;
extern int dynamicRange;
enum detectorSettings thisSettings;

int sChan, sChip, sMod, sDac, sAdc;
const int allSelected=-2;
const int noneSelected=-1;


sls_detector_module *detectorModules=NULL;
int *detectorChips=NULL;
int *detectorChans=NULL;
int *detectorDacs=NULL;
int *detectorAdcs=NULL;
//int numberOfProbes;

ROI rois[MAX_ROIS];
int nROI=0;
extern enum detectorType myDetectorType;

/** for jungfrau reinitializing macro later  in server_funcs.c in initDetector*/
extern int N_CHAN;
extern int N_CHIP;
extern int N_DAC;
extern int N_ADC;
extern int N_CHANS;

extern int nChans;
extern int nChips;
extern int nDacs;
extern int nAdcs;

int initDetector() {

  int imod;
  //  sls_detector_module *myModule;
  int n=getNModBoard();
  nModX=n;

#ifdef VERBOSE
  printf("Board is for %d modules\n",n);
#endif

  if(myDetectorType == JUNGFRAU){
	  N_CHAN=JUNGFRAU_NCHAN;
	  N_CHIP=JUNGFRAU_NCHIP;
	  N_DAC=JUNGFRAU_NDAC;
	  N_ADC=JUNGFRAU_NADC;
	  N_CHANS=JUNGFRAU_NCHANS;

	  nChans=N_CHAN;
	  nChips=N_CHIP;
	  nDacs=N_DAC;
	  nAdcs=N_ADC;
  }

  detectorModules=malloc(n*sizeof(sls_detector_module));
  detectorDacs=malloc(n*N_DAC*sizeof(int));
  detectorAdcs=malloc(n*N_ADC*sizeof(int));
  detectorChips=NULL;
  detectorChans=NULL;
  detectorAdcs=NULL;
  if(myDetectorType != JUNGFRAU){
	  detectorChips=malloc(n*N_CHIP*sizeof(int));
	  detectorChans=malloc(n*N_CHIP*N_CHAN*sizeof(int));
  }

#ifdef VERBOSE
  printf("modules from 0x%x to 0x%x\n",(unsigned int)(detectorModules), (unsigned int)(detectorModules+n));
  printf("dacs from 0x%x to 0x%x\n",(unsigned int)(detectorDacs), (unsigned int)(detectorDacs+n*N_DAC));
  printf("adcs from 0x%x to 0x%x\n",(unsigned int)(detectorAdcs), (unsigned int)(detectorAdcs+n*N_ADC));
  if(myDetectorType != JUNGFRAU){
	  printf("chips from 0x%x to 0x%x\n",(unsigned int)(detectorChips), (unsigned int)(detectorChips+n*N_CHIP));
	  printf("chans from 0x%x to 0x%x\n",(unsigned int)(detectorChans), (unsigned int)(detectorChans+n*N_CHIP*N_CHAN));
  }
#endif


  for (imod=0; imod<n; imod++) {
    (detectorModules+imod)->dacs=detectorDacs+imod*N_DAC;
	(detectorModules+imod)->adcs=detectorAdcs+imod*N_ADC;
    if(myDetectorType != JUNGFRAU){
    	(detectorModules+imod)->chipregs=detectorChips+imod*N_CHIP;
    	(detectorModules+imod)->chanregs=detectorChans+imod*N_CHIP*N_CHAN;
    }
    (detectorModules+imod)->ndac=N_DAC;
    (detectorModules+imod)->nadc=N_ADC;
    (detectorModules+imod)->nchip=N_CHIP;
    (detectorModules+imod)->nchan=N_CHIP*N_CHAN;
    (detectorModules+imod)->module=imod;
    (detectorModules+imod)->gain=0;
    (detectorModules+imod)->offset=0;
    (detectorModules+imod)->reg=0;
    /* initialize registers, dacs, retrieve sn, adc values etc */
  }
  thisSettings=UNINITIALIZED;
  sChan=noneSelected;
  sChip=noneSelected;
  sMod=noneSelected;
  sDac=noneSelected;
  sAdc=noneSelected;
  
  /*
  setCSregister(ALLMOD); //commented out by dhanya
  setSSregister(ALLMOD);
  counterClear(ALLMOD);
  clearSSregister(ALLMOD); 
  putout("0000000000000000",ALLMOD);
  */

  /* initialize dynamic range etc. */
  /* dynamicRange=getDynamicRange(); //always 16 not required commented out
     nModX=setNMod(-1);*/

  // dynamicRange=32;
  // initChip(0, 0,ALLMOD);
  //nModX=n; 
  //
  allocateRAM();

  return OK;
}




int copyChannel(sls_detector_channel *destChan, sls_detector_channel *srcChan) {
  destChan->chan=srcChan->chan;
  destChan->chip=srcChan->chip;
  destChan->module=srcChan->module;
  destChan->reg=srcChan->reg;
  return OK;
}


int copyChip(sls_detector_chip *destChip, sls_detector_chip *srcChip) {

  int ichan;
  int ret=OK;
  if ((srcChip->nchan)>(destChip->nchan)) {
    printf("Number of channels of source is larger than number of channels of destination\n");
    return FAIL;
  }

  destChip->nchan=srcChip->nchan;
  destChip->reg=srcChip->reg;
  destChip->chip=srcChip->chip;
  destChip->module=srcChip->module;
  for (ichan=0; ichan<(srcChip->nchan); ichan++) {
    *((destChip->chanregs)+ichan)=*((srcChip->chanregs)+ichan);
  }
  return ret;
}


int copyModule(sls_detector_module *destMod, sls_detector_module *srcMod) {

  int ichip, idac,  ichan, iadc;

  int ret=OK;

#ifdef VERBOSE
    printf("Copying module %x to module %x\n",(unsigned int)(srcMod),(unsigned int)(destMod));
#endif

  if (srcMod->module>=0) {
#ifdef VERBOSE
    printf("Copying module number %d to module number %d\n",srcMod->module,destMod->module);
#endif
    destMod->module=srcMod->module;
  }
  if (srcMod->serialnumber>=0){
/* #ifdef VERBOSE */
/*     printf("Copying module serial number %x to module serial number %x\n",srcMod->serialnumber,destMod->serialnumber); */
/* #endif */
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
 
  // printf("copying gain and offset %f %f to %f %f\n",srcMod->gain,srcMod->offset,destMod->gain,destMod->offset);
 
  if(myDetectorType != JUNGFRAU){
    for (ichip=0; ichip<(srcMod->nchip); ichip++) {
      if (*((srcMod->chipregs)+ichip)>=0)
	  *((destMod->chipregs)+ichip)=*((srcMod->chipregs)+ichip);
    }
    for (ichan=0; ichan<(srcMod->nchan); ichan++) {
      if (*((srcMod->chanregs)+ichan)>=0)
	  *((destMod->chanregs)+ichan)=*((srcMod->chanregs)+ichan);
    }
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



/* Register commands */


/* int clearDACSregister(int imod) { */

/*   putout("1111111111111111",imod);//reset */
/*   putout("1111111111111110",imod);//cs down */

/*   /\* commented out by dhanya */
/*      putout("0000000001000000",imod);   */
/*      putout("0000000101000000",imod);   */
/*      putout("0000000101000000",imod); */
/*      putout("0000000001000000",imod); */
/*   *\/ */
/* #ifdef DEBUG */
/*   fprintf(stdout, "Clearing DAC shiftregister\n"); */
/* #endif  */
/*   //  sDac=0; */
/*   sMod=imod; */
/*   if (imod==ALLMOD) */
/*     sMod=allSelected; */
/*   return OK; */
/* } */

/* int nextDAC(int imod) { */

/*   putout("1111111111111011",imod);//cs up */
/*   putout("1111111111111001",imod);//clk down */
/*   putout("1111111111111111",imod);//reset */

/*   /\*commented out by dhanya */
/*     putout("0000000001000000",imod);       */
/*     putout("0000000001001000",imod); */
/*     putout("0000000001000000",imod); */
/*   *\/ */
/* #ifdef DEBUG */
/*   fprintf(stdout, "Next DAC\n"); */
/* #endif  */
/*   //  sDac++;  */
/*   sMod=imod; */
/*   if (imod==ALLMOD) */
/*     sMod=allSelected; */
/*   return OK; */
/* } */


int clearCSregister(int imod) {

  putout("0000000001000000",imod);  
  putout("0000100001000000",imod);  
  putout("0000100001000000",imod);
  putout("0000000001000000",imod);  
#ifdef DEBUG
  fprintf(stdout, "Clearing CS shiftregister\n");
#endif
  /*  
      sChan=noneSelected;
      sMod=noneSelected;
      sDac=noneSelected;
      sAdc=noneSelected;
  */
  sChip=noneSelected;
  sMod=imod;
  if (imod==ALLMOD)
    sMod=allSelected;
  //putout("0000000000000000",imod);
  return 0;
}

int setCSregister(int imod){

  putout("0000000001000000",imod);  
  putout("0001000001000000",imod);  
  putout("0001000001000000",imod);
  putout("0000000001000000",imod);  
#ifdef DEBUG
  fprintf(stdout, "Setting CS shiftregister\n");
#endif
  putout("0000000000000000",imod);
  sChip=allSelected;
  sMod=imod;
  if (imod==ALLMOD)
    sMod=allSelected;
  return 0;
}

int nextChip(int imod){

  putout("0000000001000000",imod);  
  putout("0010000001000000",imod);
  putout("0000000001000000",imod); 
#ifdef DEBUG
  fprintf(stdout, "Next Chip\n");
#endif
  sChip++;
  sMod=imod;
  if (imod==ALLMOD)
    sMod=allSelected;
  return 0;
}

int firstChip(int imod){

  putout("0100000001000000",imod);  
  putout("0110000001000000",imod);
  putout("0100000001000000",imod); 
#ifdef DEBUG
  fprintf(stdout, "First Chip\n");
#endif
  sChip=0;
  sMod=imod;
  if (imod==ALLMOD)
    sMod=allSelected;
  return 0;
}

int clearSSregister(int imod){
  int i;
  putout("0000011000000000",imod);
  for (i=0; i<10; i++)   
    putout("0000111000000000",imod);
  putout("0000011000000000",imod); 
#ifdef DEBUG
  fprintf(stdout,"Clearing SS shiftregister\n");
#endif
  putout("0000000000000000",imod);
  sChan=noneSelected;
  sMod=imod;
  if (imod==ALLMOD)
    sMod=allSelected;
  return 0;
}

int setSSregister(int imod){
  int i;
  putout("0000011000000000",imod);
  for (i=0; i<10; i++) 
    putout("0001011000000000",imod);
  putout("0000011000000000",imod);  
#ifdef DEBUG
  fprintf(stdout,"Setting SS shiftregister\n");
#endif
  putout("0000000000000000",imod);
  sChan=allSelected;
  sMod=imod;
  if (imod==ALLMOD)
    sMod=allSelected;
  return 0;
}

int nextStrip(int imod){
  putout("0000011000000000",imod); 
  putout("0010011000000000",imod);
  putout("0000011000000000",imod); 
#ifdef DEBUG
  fprintf(stdout,"|-");
#endif
  sChan++;
  sMod=imod;
  if (imod==ALLMOD)
    sMod=allSelected;
  return 0;
}

int selChannel(const int strip,int imod) {
  int istrip;
  clearSSregister(imod);
  nextStrip(imod);
  for (istrip=0; istrip<strip; istrip++) {
    nextStrip(imod);
  }  
  sChan=strip;
  sMod=imod;
  if (imod==ALLMOD)
    sMod=allSelected;
  return 0;

}  

int selChip(const int chip,int imod) {
  int ichip;
  clearCSregister(imod);
  firstChip(imod);
  for (ichip=0; ichip<chip; ichip++) {
    nextChip(imod);
  }
  
  sChip=chip;
  sMod=imod;
  if (imod==ALLMOD)
    sMod=allSelected;
  return 0;
}  

/* DACs routines */

/* int program_one_dac(int addr, int value, int imod) { */

/* #ifdef VERBOSE */
/*   printf("programming dac %d value %d module %d\n",addr, value,imod); */
/* #endif */


/*   int i,im,idac,bit, control; */
/*   int v=value; */

/*   //  codata=((((0x2)<<4)+((addr)&0xf))<<16)+((value<<4)&0xfff0);   */
/*   control=32+addr;  */
/*   value=(value<<4) |  (control<< 16); */

/* #ifdef DEBUGOUT */
/*   fprintf(stdout,"value=%d\n",value); */
/* #endif */

/*   for (i=0;i<24;i++) { */
/*     bit=value & (1<<(23-i));  */
/*     if (bit) { */
/*       putout("1111111111111100",imod);//clk down */
/*       putout("1111111111111100",imod);//write data */
/*       putout("1111111111111110",imod);//clk up */
/* #ifdef DEBUGOUT */
/*       fprintf(stdout,"1"); */
/* #endif */
/*     } */
/*     else */
/*       { */
/* 	putout("1111111111111000",imod);//clk down */
/* 	putout("1111111111111000",imod);//write data */
/* 	putout("1111111111111010",imod);//clk up */
/* #ifdef DEBUGOUT */
/*       fprintf(stdout,"0"); */
/* #endif */
/*       } */
/*   }	 */

/* #ifdef DEBUGOUT */
/*   fprintf(stdout,"\n"); */
/* #endif */

/*   idac=addr;//sDac*2+addr; */


/*   if (detectorDacs) { */
/*     sMod=imod; */
/*     if (imod==ALLMOD) */
/*       sMod=allSelected; */
    
/*     if (imod>=0 && imod<nModX) { */
/*       detectorDacs[idac+N_DAC*imod]=v; */
/* #ifdef VERBOSE */
/*       printf("module=%d index=%d, val=%d addr=%x\n",imod, idac, v, (unsigned int)(detectorDacs+idac+N_DAC*imod)); */
/* #endif */

/*       setDACRegister(idac,v,imod); */
/*     } else if (imod==ALLMOD) { */
/*       for (im=0; im<nModX; im++) { */
/* 	detectorDacs[idac+N_DAC*im]=v; */
/* 	setDACRegister(idac,v,im); */
/*       } */
/*     } */
/*   } */
/*   return OK; */

/* } */





/* int set_one_dac(int imod) { */
/*   int control, ibit; */
/*   int bit, value; */
/*   control=13; */
/*   value=(DAC_REFOUT<<10) | (control<< 12); */
/* #ifdef DEBUGOUT */
/*       fprintf(stdout,"value=%d\n",value); */
/* #endif */
/*   for (ibit=0; ibit<6; ibit++) { */
/*     bit=value & (1<<(15-ibit));  */
/*     if (bit) { */
/*       putout("0000010001000000",imod); */
/*       putout("0000011001000000",imod); */
/*       putout("0000010001000000",imod); */
/* #ifdef DEBUGOUT */
/*       fprintf(stdout,"1"); */
/* #endif */
/*     } else { */
/*       putout("0000000001000000",imod);          */
/*       putout("0000001001000000",imod); */
/*       putout("0000000001000000",imod); */
/* #ifdef DEBUGOUT */
/*       fprintf(stdout,"0"); */
/* #endif */
/*     } */
/*   }  */
/*   for (ibit=0; ibit<10; ibit++) { */
/*     putout("0000000001000000",imod);          */
/*     putout("0000001001000000",imod); */
/*     putout("0000000001000000",imod); */
/* #ifdef DEBUGOUT */
/*     fprintf(stdout,"0"); */
/* #endif */
/*   }    */
  
/* #ifdef DEBUGOUT */
/*       fprintf(stdout,"\n"); */
/* #endif */
/*   return OK; */
/* } */

/* int initDACbyIndex(int ind,int val, int imod) { */
/*   int v; */
/*   const int partref[N_DAC]=PARTREF; */
/*   const int partr1[N_DAC]=PARTR1; */
/*   const int partr2[N_DAC]=PARTR2; */
 
/*   int ref=partref[ind]; */
/*   int r1=partr1[ind]; */
/*   int r2=partr2[ind]; */
  

/*   v=(val+(val-ref)*r1/r2)*DAC_DR/DAC_MAX; */
/*   v=initDACbyIndexDACU(ind,v,imod); */
  
/*   return (v*DAC_MAX/DAC_DR+ref*r1/r2)/(1+r1/r2); */
/* } */

/* int initDACbyIndexDACU(int ind, int val, int imod) { */
 
/*   //  const double daccs[N_DAC]=DACCS; */
/*   //  const double dacaddr[N_DAC]=DACADDR; */
 
/*   //  int cs=daccs[ind]; */
/*   //  int addr=dacaddr[ind]; */
/*  // int iv; */
/*   int im; */

/*   if (val>=0)  */
/*     initDAC(ind,val, imod); */
 
/*   if (imod>=0 && imod<nModX) { */
/*     // return detectorDacs[ind+imod*N_DAC]; */
/*     return setDACRegister(ind,  -1, imod); */
/*   } */
/*   else { */
/*     //iv=detectorDacs[ind]; */
/* #ifdef VERBOSE */
/*     printf("mod 0 of %d dac %d val %d\n",nModX,ind,setDACRegister(ind,  -1, 0)); */
/* #endif */
/*     for (im=1; im<nModX; im++) { */
/* #ifdef VERBOSE */
/*       printf("mod %d dac %d val %d\n",im,ind,setDACRegister(ind,  -1, im)); */
/* #endif */
/*       //if (detectorDacs[ind+im*N_DAC]!=detectorDacs[ind]) { */
	
/*       if (setDACRegister(ind,  -1, im)!=setDACRegister(ind,  -1, 0)) { */
/* #ifdef VERBOSE */

/*       printf("mod %d returning -1\n",im); */
/* #endif */
/* 	return -1; */
/*       } */
/*     } */
/* #ifdef VERBOSE */
/*       printf("returning %d\n",setDACRegister(ind,  -1, 0)); */
/* #endif */
/*     return setDACRegister(ind,  -1, 0); */
/*   } */
/* } */

/* int getThresholdEnergy() { */
/*   double g[3]=DEFAULTGAIN; */
/*   double o[3]=DEFAULTOFFSET; */
/*   double myg=-1, myo=-1; */
/*  // int dacu; */
/*   int imod; */
/*   int ethr=-1; */
/*   int ret=FAIL; */

/*   if (detectorModules) { */
/*     //  for (imod=0; imod<getNModBoard(); imod++) { */
/*     for (imod=0; imod<nModX; imod++) { */
/* #ifdef VERBOSE */
/*       printf("module=%d settings=%d, gain=%f, offset=%f\n",imod,thisSettings, (detectorModules+imod)->gain,(detectorModules+imod)->offset); */
/* #endif */
/*       if ((detectorModules+imod)->gain>0) */
/* 	myg=(detectorModules+imod)->gain; */
/*       else { */
/* 	if (thisSettings>=0 && thisSettings<3) */
/* 	  myg=g[thisSettings]; */
/* 	//	else */
/* 	//myg=-1; */
/*       } */

/*       if ((detectorModules+imod)->offset>0) */
/* 	myo=(detectorModules+imod)->offset; */
/*       else { */
/* 	if (thisSettings>=0 && thisSettings<3) */
/* 	  myo=o[thisSettings]; */
/* 	//	else */
/* 	//myo=-1; */
/*       } */

/*       if (myg>0 && myo>0) { */
/* 	//ethr=(myo-detectorDacs[VTHRESH+imod*N_DAC])*1000/myg; */
      
/* 	ethr=(myo-setDACRegister(VDAC0,-1,imod))*1000/myg;//edited by dhanya */
/* 	// else */
/*       //	ethr=-1; */
      
/*       } */
/* #ifdef VERBOSE */
/*       //printf("module=%d gain=%f, offset=%f, dacu=%f\n",imod, myg, myo, detectorDacs[VTHRESH+imod*N_DAC]); */
/*       printf("module=%d gain=%f, offset=%f, dacu=%d\n",imod, myg, myo,(int)(setDACRegister(VDAC0,-1,imod)));//edited by dhanya */
/*       printf("Threshold energy of module %d is %d eV\n", imod, ethr); */
/* #endif  */
      
/*       if (imod==0) */
/* 	ret=ethr; */
/*       else { */
/* 	if (ethr>(ret+100) || ethr<(ret-100)) */
/* 	  return FAIL; */
/*       } */
/*     } */
/*   } */
/*   return ret; */
/* } */

/* int setThresholdEnergy(int ethr) { */
/*   double g[3]=DEFAULTGAIN; */
/*   double o[3]=DEFAULTOFFSET; */
/*   double myg=-1, myo=-1; */
/*   int dacu; */
/*   int imod; */
/*   int ret=ethr; */

/*   setSettings(GET_SETTINGS,-1);//-1 added by dhanya */
/*   if (thisSettings>=0 || thisSettings<3){ */
/*     myg=g[thisSettings]; */
/*     myo=o[thisSettings]; */
/*   } */
/*   for (imod=0; imod<nModX; imod++) { */
/*     if (detectorModules) { */
/*       if ((detectorModules+imod)->gain>0) */
/* 	myg=(detectorModules+imod)->gain; */
/*       else   */
/* 	if (thisSettings>=0 && thisSettings<3) */
/* 	  myg=g[thisSettings]; */
/* 	else */
/* 	  myg=-1; */
/*       if ((detectorModules+imod)->offset>0) */
/* 	myo=(detectorModules+imod)->offset; */
/*       else */
/* 	if (thisSettings>=0 && thisSettings<3) */
/* 	  myo=o[thisSettings]; */
/* 	else */
/* 	  myo=-1; */
/*     } else { */
/* 	if (thisSettings>=0 && thisSettings<3) */
/* 	  myo=o[thisSettings]; */
/* 	else */
/* 	  myo=-1; */
/* 	if (thisSettings>=0 && thisSettings<3) */
/* 	  myg=g[thisSettings]; */
/* 	else */
/* 	  myg=-1; */
/*     } */
/*     if (myg>0 && myo>0) { */
/*       dacu=myo-myg*((double)ethr)/1000.; */
/* #ifdef VERBOSE     */
/*       printf("module %d (%x): gain %f, off %f, energy %d eV, dac %d\n",imod,(unsigned int)((detectorModules+imod)),(detectorModules+imod)->gain,(detectorModules+imod)->offset, ethr,dacu); */
/* #endif */
/*     } else { */
/*       dacu=ethr; */
/* #ifdef VERBOSE     */
/*       printf("could not set threshold energy for module %d, settings %d (offset is %f; gain is %f)\n",imod,thisSettings,myo,myg); */
/* #endif */
/*     } */
/*     initDACbyIndexDACU(VDAC0, dacu, imod); ///needs to be fixed dhanya */
/*   } */
/*   return ret; */
/* } */



/* int getDACbyIndexDACU(int ind,  int imod) { */
/*   /\* */
/*   if (detectorDacs) { */
/*     if (imod<getNModBoard()) */
/*       if (ind<(detectorModules+imod)->ndac) */
/* 	return (detectorDacs[ind+imod*N_DAC]); */
/*   }   */
/*   return FAIL; */
/*   *\/ */
/*   return setDACRegister(ind, -1, imod); */
/* } */


/* int initDAC(int dac_addr, int value, int imod) { */
/* //  int i; */
/* #ifdef VERBOSE */
/*   printf("Programming dac %d with value %d\n", dac_addr, value); */
/* #endif */
/*   clearDACSregister(imod); */
/*   program_one_dac(dac_addr,value,imod); */
/*   nextDAC(imod); */
/*   clearDACSregister(imod); */

/*   return 0; */
/* } */

int getTemperatureByModule(int tempSensor, int imod)
{
  int im;
  //for the particular module
  if (imod>=0 && imod<nModX) { 
    return getTemperature(tempSensor,imod);
  }
  else{
    //checks if all modules have the same value(ALLMOD)
    for (im=1; im<nModX; im++) {
      if (getTemperature(tempSensor,im)!=getTemperature(tempSensor,0)) {
	return -1;
      }
    }
    return getTemperature(tempSensor,0);
  }
}


int initHighVoltageByModule(int val, int imod)
{
  int im;
  //for the particular module
  if (imod>=0 && imod<nModX) { 
    return initHighVoltage(val,imod);
  }
  else{
    //checks if all modules have the same value(ALLMOD)
    for (im=1; im<nModX; im++) {
      if (initHighVoltage(val,im)!=initHighVoltage(val,0)) {
	return -1;
      }
    }
    return initHighVoltage(val,0);
  }
}


int initConfGainByModule(int isettings,int val, int imod)
{
  int im;
  //for the particular module
  if (imod>=0 && imod<nModX) { 
    return initConfGain(isettings,val,imod);
  }
  else{
    //checks if all modules have the same value(ALLMOD)
    for (im=1; im<nModX; im++) {
      if (initConfGain(isettings,val,im)!=initConfGain(isettings,val,0))
	return -1;
    }
    return initConfGain(isettings,val,0);
  }
}


void showbits(int h)
{
  if(h==1)
    printf("%d",h);
  else
    {
      showbits(h/2);
      printf("%d",h%2);
    }
}


/* int initDACs(int* v,int imod) */
/* { */
/* #ifdef VERBOSE */
/*   printf("\n..inside initdacs\n"); */
/* #endif */
/*   int iaddr; */
/*   //  sDac=0; */

/*   for (iaddr=0; iaddr<8; iaddr++) { */
/*     clearDACSregister(imod); */
/*     if (v[iaddr]>=0) {  */
/* #ifdef VERBOSE */
/*       fprintf(stdout, "voltage %d\n", *(v+iaddr)); */
/* #endif */
/*       program_one_dac(iaddr, *(v+iaddr),imod); */
/*     } */
/*     nextDAC(imod); */
/*   } */
    

/*   clearDACSregister(imod); */

/*   return 0; */

/* } */




int setSettings(int i, int imod) {
#ifdef VERBOSE
	if(i==-1)
		printf("\nReading settings of detector...\n");
	else
		printf("\ninside set settings wit settings=%d...\n",i);
#endif
	int isett=-1,val=-1,retval=-1;
	enum conf_gain {
		dynamic = 0x0f00,	//dynamic
		dynamichighgain0 =	0x0f01,	//dynamichighgain0
		fixgain1 = 0x0f02,	//fixgain1
		fixgain2 = 0x0f06,	//fixgain2
		forceswitchgain1 = 0x1f00,	//forceswitchgain1
		forceswitchgain2 = 0x3f00	//forceswitchgain2
	};

	//determine conf value to write
	if(i!=GET_SETTINGS){
		switch(i){
		case DYNAMICGAIN: 	val = dynamic;break;
		case DYNAMICHG0: 	val = dynamichighgain0;break;
		case FIXGAIN1: 		val = fixgain1;break;
		case FIXGAIN2: 		val = fixgain2;break;
		case FORCESWITCHG1: val = forceswitchgain1;break;
		case FORCESWITCHG2: val = forceswitchgain2;break;
		default:
			printf("Error: This settings is not defined for this detector %d\n",i);
			return GET_SETTINGS;
		}
	}

	retval=initConfGainByModule(i,val,imod);

	switch(retval){
	case dynamic: isett=DYNAMICGAIN;	break;
	case dynamichighgain0: isett=DYNAMICHG0;	break;
	case fixgain1: isett=FIXGAIN1;		break;
	case fixgain2: isett=FIXGAIN2;		break;
	case forceswitchgain1: isett=FORCESWITCHG1;	break;
	case forceswitchgain2: isett=FORCESWITCHG2;	break;
	default:
		isett=UNDEFINED;
		printf("Error:Wrong settings read out from Gain Reg 0x%x\n",retval);
		break;
	}

	thisSettings=isett;
//#ifdef VERBOSE
	printf("detector settings are %d\n",thisSettings);
//#endif
	return thisSettings;
}




/* Initialization*/

int initChannelbyNumber(sls_detector_channel myChan) {printf("in init channel by number\n");
  int reg=myChan.reg;
  int ft=reg & TRIM_DR;
  int cae=(reg>>(NTRIMBITS))&1;
  int ae=(reg>>(NTRIMBITS+1))&1;
  int coe=(reg>>(NTRIMBITS+2))&1;
  int ocoe=(reg>>(NTRIMBITS+3))&1;
  int counts=(reg>>(NTRIMBITS+4));
#ifdef VERBOSE
  printf("Initializing channel %d chip %d module %d reg %x\n",myChan.chan,myChan.chip,myChan.module, reg);
  printf("trim %d, cae %d, ae %d, coe %d, ocoe %d, counts %d\n",ft, cae, ae, coe, ocoe, counts);
#endif
  
  if (myChan.chip<0)
    setCSregister(myChan.module);
  else
    selChip(myChan.chip,myChan.module);

   if (myChan.chan<0)
    setSSregister(myChan.module);
  else
    selChannel(myChan.chan,myChan.module);
  
   initChannel(ft,cae,ae, coe, ocoe, counts,myChan.module);

   setDynamicRange(dynamicRange);

   setCSregister(ALLMOD);
   clearSSregister(ALLMOD); 
   putout("0000000000000000",ALLMOD);

   return myChan.reg;

}

int getChannelbyNumber(sls_detector_channel* myChan) {
  int imod, ichip, ichan;
  imod=myChan->module;
  ichip=myChan->chip;
  ichan=myChan->chan;
   
  if (detectorChans) {
    if (imod<nModX && imod>=0) {
      if (ichip<(detectorModules+imod)->nchip && ichan<(detectorModules+imod)->nchan/(detectorModules+imod)->nchip)
	myChan->reg=detectorChans[imod*N_CHAN*N_CHIP+ichip*N_CHAN+ichan];
      return OK;
    }
  } 
  return FAIL;

}

int getTrimbit(int imod, int ichip, int ichan) {
  if (detectorChans) {
    if (imod<getNModBoard() && imod>=0)
      if (ichip<(detectorModules+imod)->nchip && ichan<(detectorModules+imod)->nchan/(detectorModules+imod)->nchip)
	return (detectorChans[imod*N_CHAN*N_CHIP+ichip*N_CHAN+ichan] & TRIM_DR);
  }

  return -1;
}

int initChannel(int ft,int cae, int ae, int coe, int ocoe, int counts, int imod){
  
  int ibit, bit, i, im, ichip, ichan;
  int chanmi, chanma, chipmi, chipma, modmi, modma;



  sMod=imod;
  //  printf("initializing module %d\n",sMod);
  if (imod==ALLMOD) {
    sMod=allSelected;

    //    printf("initializing all modules\n");
  }

  if (sChan==allSelected) {
    //    printf("initializing all channels ft=%d coe=%d\n",ft,coe);
    chanmi=0;
    chanma=N_CHAN;
  } else if (sChan==noneSelected || sChan>N_CHAN || sChan<0) {
    //  printf("initializing no channels ft=%d coe=%d\n",ft,coe);
    chanmi=0;
    chanma=-1;
  } else {
    // printf("initializing channel %d ft=%d coe=%d\n",sChan, ft,coe);
    chanmi=sChan;
    chanma=sChan+1;
  }
  
  if (sChip==allSelected) {
    // printf("initializing all chips\n");
    chipmi=0;
    chipma=N_CHIP;
  } else if (sChip==noneSelected || sChip>N_CHIP || sChip<0) {
    // printf("initializing no chips\n");
    chipmi=0;
    chipma=-1;
  } else {
    // printf("initializing chip %d\n",sChip);
    chipmi=sChip;
    chipma=sChip+1;
  }
 
  
  if (sMod==allSelected) {
    modmi=0;
    modma=nModX;//getNModBoard();
  } else if (sMod==noneSelected || sMod>nModX || sMod<0) {//(sMod==noneSelected || sMod>getNModBoard() || sMod<0) {
    modmi=0;
    modma=-1;
    return 1;
  } else {
    modmi=sMod;
    modma=sMod+1;
  }
 
  if (detectorChans) {
    for (im=modmi; im<modma; im++) {
      for (ichip=chipmi; ichip<chipma; ichip++) {
	for (ichan=chanmi; ichan<chanma; ichan++) {
#ifdef VERBOSE
	  //  printf("im=%d ichi=%d icha=%d tot=%d reg=%x\n",im,ichip, ichan, im*N_CHAN*N_CHIP+ichip*N_CHAN+ichan,detectorChans[im*N_CHAN*N_CHIP+ichip*N_CHAN+ichan]);
#endif
	 detectorChans[im*N_CHAN*N_CHIP+ichip*N_CHAN+ichan]= ft | (cae<<(NTRIMBITS+1)) | (ae<<(NTRIMBITS+2)) | (coe<<(NTRIMBITS+3)) | (ocoe<<(NTRIMBITS+4)) | (counts<<(NTRIMBITS+5));
#ifdef VERBOSE
	 //printf("imod=%d ichip=%d ichan=%d addr=%x reg=%x\n",im,ichip,ichan,detectorChans+im*N_CHAN*N_CHIP+ichip*N_CHAN+ichan, detectorChans[im*N_CHAN*N_CHIP+ichip*N_CHAN+ichan]);
	  // printf("imod=%d ichip=%d ichan=%d addr=%x reg=%x\n",im,ichip,ichan,detectorChans+im*N_CHAN*N_CHIP+ichip*N_CHAN+ichan, detectorChans[im*N_CHAN*N_CHIP+ichip*N_CHAN+ichan]);
#endif
	}
      }
    }
  }
  /* remember to clear counters before programming!!!*/
  if (ft>63 || ft<0) {
    fprintf(stdout,"Fine Threshold is %d  while should be between 0 and 63!",ft);
    return 1;
  }
  /*cal_enable*/
  if (cae) {
    putout("0100000000000000",imod);  
    putout("0110000000000000",imod); 
  } else {
    putout("0000000000000000",imod);  
    putout("0010000000000000",imod); 
  }
  /*n_an_enable*/
  if (ae) {
    putout("0000000000000000",imod);  
    putout("0010000000000000",imod);
    putout("0000000000000000",imod);   
  } else {
    putout("0100000000000000",imod);  
    putout("0110000000000000",imod); 
    putout("0100000000000000",imod);  
  }
  /*trb5*/
  ibit=5;
  bit=ft & (1<<ibit);  
  if (bit) {
    putout("0100000000000000",imod);  
    putout("0110000000000000",imod);
    putout("0100000000000000",imod);  
  } else {
    putout("0000000000000000",imod);  
    putout("0010000000000000",imod); 
    putout("0000000000000000",imod);  
  }

  /*trb3*/
  ibit=3;
  bit=ft & (1<<ibit);  
  if (bit) {
    putout("0100000000000000",imod);  
    putout("0110000000000000",imod); 
    putout("0100000000000000",imod);  
  } else {
    putout("0000000000000000",imod);  
    putout("0010000000000000",imod);
    putout("0000000000000000",imod);  
  }

  /*trb2*/
  ibit=2;
  bit=ft & (1<<ibit);  
  if (bit) {
    putout("0100000000000000",imod);  
    putout("0110000000000000",imod);
    putout("0100000000000000",imod);   
  } else {
    putout("0000000000000000",imod);  
    putout("0010000000000000",imod); 
    putout("0000000000000000",imod);  
  }

  /*trb1*/
  ibit=1;
  bit=ft & (1<<ibit);  
  if (bit) {
    putout("0100000000000000",imod);  
    putout("0110000000000000",imod); 
    putout("0100000000000000",imod);  
  } else {
    putout("0000000000000000",imod);  
    putout("0010000000000000",imod); 
    putout("0000000000000000",imod);  
  }

  /*trb0*/
  ibit=0;
  bit=ft & (1<<ibit);  
  if (bit) {
    putout("0100000000000000",imod);  
    putout("0110000000000000",imod); 
    putout("0100000000000000",imod); 
  } else {
    putout("0000000000000000",imod);  
    putout("0010000000000000",imod); 
    putout("0000000000000000",imod);  
  }

  /*n_comp_enable*/
  if (coe) {
    putout("0000000000000000",imod);  
    putout("0010000000000000",imod); 
    putout("0000000000000000",imod); 
  } else {
    putout("0100000000000000",imod);  
    putout("0110000000000000",imod); 
    putout("0100000000000000",imod);  
  }

 
  /*trb4*/
  ibit=4;
  bit=ft & (1<<ibit);  
  if (bit) {
    putout("0100000000000000",imod);  
    putout("0110000000000000",imod);
    putout("0100000000000000",imod);  
  } else {
    putout("0000000000000000",imod);  
    putout("0010000000000000",imod);
    putout("0000000000000000",imod);   
  }
  
  /*out_comp_enable*/
  if (ocoe) {
    putout("0100000000000000",imod);  
    putout("0110000000000000",imod); 
    putout("0100000000000000",imod);  
  } else {
    putout("0000000000000000",imod);  
    putout("0010000000000000",imod); 
    putout("0000000000000000",imod);  
  }
  
  
  for (ibit=0; ibit<24; ibit++) {
    bit=counts & (1<<ibit);  
    if (bit) {
      putout("0100000000000000",imod);  
      putout("0110000000000000",imod); 
      putout("0100000000000000",imod);  
    } else {
      putout("0000000000000000",imod);  
      putout("0010000000000000",imod);
      putout("0000000000000000",imod);  
    }
  }
  counterClear(imod);
  counterSet(imod);
  putout("0000000000000000",imod);
  for (i=0; i<10; i++) 
    putout("0000100000000000",imod);
  putout("0000000000000000",imod);

  //getDynamicRange();


  return 0;

}


int initChipbyNumber(sls_detector_chip myChip){
  int imod, ichip;
  imod=myChip.module;
  ichip=myChip.chip;
  int obe=(myChip.reg)&1;
  int ow=(myChip.reg)>>1;
  int nchan, ichan;
  int ft,  cae, ae, coe, ocoe, counts, chanreg;
 
  
  
  nchan=myChip.nchan;
  if (ichip<0)
    setCSregister(imod);
  else
    selChip(ichip,imod);

  clearSSregister(imod);
  for (ichan=0; ichan<nchan; ichan++) {
    chanreg=(myChip.chanregs)[ichan];
    ft=chanreg&((int)TRIM_DR);
    cae=(chanreg>>(NTRIMBITS+1))&1;
    ae=(chanreg>>(NTRIMBITS+2))&1;
    coe=((chanreg)>>(NTRIMBITS+3))&1;
    ocoe=((chanreg)>>(NTRIMBITS+4))&1;
    counts=((chanreg)>>(NTRIMBITS+5));
    nextStrip(imod); 
    initChannel(ft,cae,ae, coe, ocoe, counts,imod);
  }
  initChip(obe,ow,imod);
  return myChip.reg;

}

int getChipbyNumber(sls_detector_chip* myChip){
  int imod, ichip;
  imod=myChip->module;
  ichip=myChip->chip;
  
  if (detectorChips) {
    if (imod<nModX)
      if (ichip<(detectorModules+imod)->nchip) {
	myChip->reg=detectorChips[ichip+imod*N_CHIP];
	myChip->nchan=N_CHAN;
	myChip->chanregs=detectorChans+imod*N_CHAN*N_CHIP+ichip*N_CHIP;
	return OK;
      }
  }
  return FAIL;

}



int initChip(int obe, int ow,int imod){
  int i;
  int im, ichip;
  int chipmi, chipma, modmi, modma;
  /* switch (ow) {
  case 0:;
  case 1:
    setDynamicRange(32);
    break;
  case 2:
    setDynamicRange(16);
    break;
  case 3:
    setDynamicRange(8);
    break;
  case 4:
    setDynamicRange(4);
    break;
  case 5:
    setDynamicRange(1);
    break;
  default:
    setDynamicRange(32);
    break;
  }
  */

#ifdef DEBUGOUT
  printf("Initializing chip\n");
#endif
  putout("0000000000000000",imod);
#ifdef DEBUGOUT
  printf("Output mode= %d\n", ow);
#endif

  /* clearing shift in register */
  for (i=0; i<10; i++) 
    putout("0000100000000000",imod);
  putout("0000000000000000",imod);

  if (ow>0) {
    putout("0100000000000000",imod);  
    putout("0110000000000000",imod); 
    putout("0100000000000000",imod);
    for (i=0; i<(OUTMUX_OFFSET-1); i++) {
      putout("0000000000000000",imod);  
      putout("0010000000000000",imod); 
      putout("0000000000000000",imod); 
    }
    if (ow>1) {
      putout("0000000000000000",imod);  
      putout("0010000000000000",imod);
      putout("0000000000000000",imod); 
    }
    if (ow>2) {
      putout("0000000000000000",imod);  
      putout("0010000000000000",imod);
      putout("0000000000000000",imod);
    }
    if (ow>3) {
      putout("0000000000000000",imod);  
      putout("0010000000000000",imod); 
      putout("0000000000000000",imod);
    }
    if (ow>4) {
      putout("0000000000000000",imod);  
      putout("0010000000000000",imod); 
      putout("0000000000000000",imod);
    }
  }  
#ifdef DEBUGOUT
  printf("Output buffer  enable= %d\n", obe);
#endif
  if (obe) {
    putout("0100000000000000",imod);  
    putout("0110000000000000",imod); 
    putout("0100000000000000",imod);  
  } else {
    putout("0000000000000000",imod);  
    putout("0010000000000000",imod); 
    putout("0000000000000000",imod);  
  }    
  /*}*/
  putout("0000000000000000",imod);  





  sMod=imod;
  if (imod==ALLMOD)
    sMod=allSelected;

 
  if (sChip==allSelected) {
    chipmi=0;
    chipma=N_CHIP;
  } else if (sChip==noneSelected || sChip>N_CHIP || sChip<0) {
    chipmi=0;
    chipma=-1;
  } else {
    chipmi=sChip;
    chipma=sChip+1;
  }
 
  
  if (sMod==allSelected) {
    modmi=0;
    modma=nModX;//getNModBoard();
  } else if (sMod==noneSelected || sMod>nModX || sMod<0) {//(sMod==noneSelected || sMod>getNModBoard() || sMod<0) {
    modmi=0;
    modma=-1;
  } else {
    modmi=sMod;
    modma=sMod+1;
  }
 
  if (detectorChips) {
    for (im=modmi; im<modma; im++) {
      for (ichip=chipmi; ichip<chipma; ichip++) {
	//	printf("imod %d ichip %d\n",im,ichip);
	detectorChips[im*N_CHIP+ichip]=obe | (ow<<1);
#ifdef VERBOSE
	//printf("imod=%d ichip=%d reg=%d (%x)\n",im,ichip,detectorChips[im*N_CHIP+ichip],detectorChips+im*N_CHIP+ichip);
#endif
      }
    }
  }
  getDynamicRange();
  return 0;
}


int initChipWithProbes(int obe, int ow,int nprobes, int imod){
  int i;
  int im, ichip;
  int chipmi, chipma, modmi, modma;

  int64_t regval=0, dum;
  int omask;
  switch (ow) {
  case 2:
    omask=2;
    break;
  case 3:
    omask=4;
    break;
  case 4:
    omask=8;
    break;
  case 5:
    omask=16;
    break;
  default:
    omask=0;//1;
    break;
  }

#ifdef VERBOSE 
 // printf("\n \n \n",regval);
  printf("initChip ow=%d omask=%d probes=%d\n",ow, omask,nprobes);
#endif
  regval|=(omask<<OUTMUX_OFFSET);
  regval|=(nprobes<<PROBES_OFFSET);
  regval|=(obe<<OUTBUF_OFFSET);
#ifdef VERBOSE
  printf("initChip : shift in will be %08x\n",(unsigned int)(regval));
#endif
  /* clearing shift in register */

  putout("0000000000000000",imod);
  for (i=0; i<10; i++) 
    putout("0000100000000000",imod);
  putout("0000000000000000",imod);

  for (i=0; i<35; i++) {
    dum= (1<<(34-i));
    if (regval & dum) {
      putout("0100000000000000",imod);  
      putout("0110000000000000",imod); 
      putout("0100000000000000",imod);
    } else {
      putout("0000000000000000",imod);  
      putout("0010000000000000",imod); 
      putout("0000000000000000",imod); 
    }
  }
  putout("0000000000000000",imod);  

  sMod=imod;
  if (imod==ALLMOD)
    sMod=allSelected;

 
  if (sChip==allSelected) {
    chipmi=0;
    chipma=N_CHIP;
  } else if (sChip==noneSelected || sChip>N_CHIP || sChip<0) {
    chipmi=0;
    chipma=-1;
  } else {
    chipmi=sChip;
    chipma=sChip+1;
  }
 
  
  if (sMod==allSelected) {
    modmi=0;
    modma=nModX;//getNModBoard();
  } else if (sMod==noneSelected || sMod>nModX || sMod<0) {//(sMod==noneSelected || sMod>getNModBoard() || sMod<0) {
    modmi=0;
    modma=-1;
  } else {
    modmi=sMod;
    modma=sMod+1;
  }
 
  if (detectorChips) {
    for (im=modmi; im<modma; im++) {
      for (ichip=chipmi; ichip<chipma; ichip++) {
	//	printf("imod %d ichip %d\n",im,ichip);
	detectorChips[im*N_CHIP+ichip]=obe | (ow<<1);
#ifdef VERBOSE
	//printf("imod=%d ichip=%d reg=%d (%x)\n",im,ichip,detectorChips[im*N_CHIP+ichip],detectorChips+im*N_CHIP+ichip);
#endif
      }
    }
  }
  getDynamicRange();

  return 0;
}
/*
int getNProbes() {
  return numberOfProbes;
}
*/


int initMCBregisters(int cm, int imod){

  int im, modmi, modma;
  if (cm<0)
    return 0;
    

  putout("0000000001000000",imod);    
  putout("0000000001100000",imod);   
  putout("0000000001100000",imod);     
  putout("0000000001000000",imod);    
  
 if (cm) { 
   putout("0000000001000001",imod); 
#ifdef DEBUGOUT   
   printf("enabling cal through sensor\n");
#endif
 }  else {
   putout("0000000001000001",imod);
   putout("0000000001000000",imod);
   putout("0000000001000001",imod); 
#ifdef DEBUGOUT   
   printf("disabling cal through sensor\n");
#endif 
 }
 putout("0000000001000000",imod);
 
  sMod=imod;
  if (imod==ALLMOD)
    sMod=allSelected;

 
  
  if (sMod==allSelected) {
    modmi=0;
    modma=nModX;
  } else if (sMod==noneSelected || sMod>nModX || sMod<0) {//(sMod==noneSelected || sMod>getNModBoard() || sMod<0) {
    modmi=0;
    modma=-1;
  } else {
    modmi=sMod;
    modma=sMod+1;
  }
 
  if (detectorModules) {
    for (im=modmi; im<modma; im++) {
      ((detectorModules+im)->reg)=cm;
#ifdef VERBOSE
      printf("imod=%d reg=%d (%x)\n",im,(detectorModules+im)->reg,(unsigned int)((detectorModules+im)));
#endif
    }
  }
 return 0;
}

int initModulebyNumber(sls_detector_module myMod) { 

  printf("\ninside initmoduleynumberrrr..\n");
  printf("000\n");
  int nchip,nchan;//int ichip, nchip, ichan, nchan;
  int im, modmi,modma;
 // int ft,  cae, ae, coe, ocoe, counts, chanreg;
  int imod;
 // int obe;
 // int ow;
 /* int v[N_DAC];*/
  int retval =-1, idac;


  nchip=myMod.nchip;
  nchan=(myMod.nchan)/nchip;

  imod=myMod.module;
  sMod=imod;

  if (sMod==ALLMOD)
    sMod=allSelected;
    
  if (sMod==allSelected) {
    modmi=0;
    modma=nModX;//getNModBoard();
  } else if (sMod==noneSelected || sMod>nModX || sMod<0) {// (sMod==noneSelected || sMod>getNModBoard() || sMod<0) {
    modmi=0;
    modma=-1;
  } else {
    modmi=sMod;
    modma=sMod+1;
  }

  printf("222\n");
  /*
  for (idac=0; idac<N_DAC; idac++)
    v[idac]=(myMod.dacs)[idac];

  v[VDAC0]=(myMod.dacs)[0];
  v[VDAC1]=(myMod.dacs)[1];
  v[VDAC2]=(myMod.dacs)[2];
  v[VDAC3]=(myMod.dacs)[3];
  v[VDAC4]=(myMod.dacs)[4];
  v[VDAC5]=(myMod.dacs)[5];
  v[VDAC6]=(myMod.dacs)[6];
  v[VDAC7]=(myMod.dacs)[7];

#ifdef VERBOSE
  printf("imod=%d\n",imod);
  printf("vdac0=%d\n",v[VDAC0]);
  printf("vdac1=%d\n",v[VDAC1]);
  printf("vdac2=%d\n",v[VDAC2]);
  printf("vdac3=%d\n",v[VDAC3]);
  printf("vdac4=%d\n",v[VDAC4]);
  printf("vdac5=%d\n",v[VDAC5]);
  printf("vdac6=%d\n",v[VDAC6]);
  printf("vdac7=%d\n",v[VDAC7]);
#endif
 */
  printf("ndac:%d\n",N_DAC);
  //  initDACs(v,imod);
  // initMCBregisters(myMod.reg,imod);
  for (idac=0; idac<N_DAC; idac++){
	  retval = setDac(idac,(myMod.dacs)[idac]);
	  if(retval ==(myMod.dacs)[idac])
		  printf("Setting dac %d to %d\n",idac,retval);
	  else
		  printf("Error: Could not set dac %d, wrote %d but read %d\n",idac,(myMod.dacs)[idac],retval);
  }

  if (detectorModules) {
    for (im=modmi; im<modma; im++) {
#ifdef VERBOSE
      printf("im=%d\n",im);
#endif
      copyModule(detectorModules+im,&myMod);
    }
  }
  //setting the conf gain and the settings register
  setSettings(myMod.reg,imod);
  return thisSettings;
}


int getModulebyNumber(sls_detector_module* myMod) {
  int imod=myMod->module;
#ifdef VERBOSE
  printf("Getting module %d\n",imod);
#endif
  if (detectorModules) {
    copyModule(myMod,detectorModules+imod);
    ;
  } else
    return FAIL;

  return OK;
}

/* To chips */
int clearCounter(int imod){
  int i;
#ifdef DEBUG
  printf("Clearing counter with contclear\n");
#endif
  putout("0000000000000000",imod);  
  for (i=0; i<10; i++)   
    putout("0000000000010000",imod);   
  putout("0000000000000000",imod);  

  return 0;    
}

int clearOutReg(int imod){
  int i;
#ifdef DEBUG
  printf("Clearing output register\n");
#endif
  putout("0000010000000000",imod); 
  for (i=0; i<10; i++)
    putout("0000110000000000",imod);   
  putout("0000010000000000",imod);  
  return 0;    
}
int setOutReg(int imod){
  int i;
#ifdef DEBUG
  printf("Setting output register\n");
#endif
  putout("0000010000000000",imod); 
  for (i=0; i<10; i++)
    putout("0001010000000000",imod);   
  putout("0000010000000000",imod);  
  return 0;    
}


int extPulse(int ncal, int imod) {
  int ical;
#ifdef DEBUG
  printf("Giving a clock pulse to the counter\n");
#endif
  for (ical=0; ical<ncal; ical++) {
    putout("0000001000000000",imod);
    putout("0010001000000000",imod);
    putout("0000001000000000",imod);
  }
  return 0;
}
int calPulse(int ncal, int imod) {
  int ical,i;
#ifdef DEBUG
  printf("Giving a cal pulse\n");
#endif
  for (ical=0; ical<ncal; ical++) { 
    //printf("%d\n",ical);
     for (i=0; i<10; i++)
       putout("0000000000100000",imod);
    for (i=0; i<20; i++)
      putout("0000000000100001",imod);
     for (i=0; i<10; i++)
       putout("0000000000100000",imod);
  }
  putout("0000000000000000",imod);
  return 0;
}

int countEnable(int imod) {
#ifdef DEBUG
  printf("Enabling counter\n");
#endif
  putout("0000000000100000",imod);
  return 0;
}


int counterClear(int imod) {
  int i;
#ifdef DEBUG
  printf("Clearing counter in counter mode\n");
#endif
  putout("0000001000000000",imod);
  for (i=0; i<10; i++) 
    putout("0000101000000000",imod);
  putout("0000001000000000",imod);
  return 0;
}

int counterSet(int imod) {
  int i;
#ifdef DEBUG
  printf("Setting counter\n");
#endif
  putout("0000001000000000",imod);
  for (i=0; i<20; i++)
    putout("0001001000000000",imod);
  putout("0000001000000000",imod);    

  return 0;
}
// Fifo continuous read 

int readOutChan(int *val) {
  int i, j, k, v;
  int nbit=24;

  setCSregister(ALLMOD); 
  clearOutReg(ALLMOD);
  
  setOutReg(ALLMOD);

#ifdef DEBUG
  printf("Reading out one channel\n");
#endif

  

  for (i=0; i<nbit; i++) {
    putout("0000010000000000", ALLMOD);
    k=0;
    for (k=0; k<nModX; k++) {
      v=readin(k);
      //v=bus_r(MCB_DOUT_REG_OFF+(k<<SHIFTMOD)) & 0x3ff;
      for (j=0; j<N_CHIP; j++) {
	if (i==0) 
	  *(val+j+k*N_CHIP)=0;
	if (v & (1<<j)) { 
	  *(val+j+k*N_CHIP)= *(val+j+k*N_CHIP) | (1 << i);
	} 
      }
    }
    putout("0000010000000000",ALLMOD);
    putout("0010010000000000", ALLMOD); 
    putout("0010010000000000", ALLMOD); 
  }
  putout("0000010000000000",ALLMOD);
  putout("0000010000000000",ALLMOD); 
  return 0;
}

int getModuleNumber(int modnum) {
  int val = 0xfff;
 // val=readin(modnum);
  return val;
}


int testShiftIn(int imod) {
  int val,i,j, k, result=OK;
  setCSregister(ALLMOD); 
  printf("testing shift in for module %d\n", imod);
  //for (j=0; j<10; j++) {
  //selChip(j);
  for (i=0; i<34; i++) {
    if (i%2) {
      putout("0100000000000000",ALLMOD);
      putout("0100000000000000",ALLMOD);  
      putout("0110000000000000",ALLMOD);  
      putout("0110000000000000",ALLMOD); 
      putout("0100000000000000",ALLMOD);
      putout("0100000000000000",ALLMOD); 
    } else {
      putout("0000000000000000",ALLMOD);
      putout("0000000000000000",ALLMOD);  
      putout("0010000000000000",ALLMOD);  
      putout("0010000000000000",ALLMOD);  
      putout("0000000000000000",ALLMOD);
      putout("0000000000000000",ALLMOD);
    } 
  }
    putout("0000000000000000",ALLMOD);    
    for (i=0; i<34; i++) {
      putout("0000000000000000",ALLMOD); 
      
      k=imod;
      //for (k=0; k<nModX; k++) {
      val=readin(k);
      //val=bus_r(MCB_DOUT_REG_OFF+(k<<SHIFTMOD)) & 0x3ff;
	for (j=0; j<10; j++) {	
	  if ( (val & 1<<j)>0 && i%2==0) {   
	    printf("Shift in: module %d chip %i bit %d read %d instead of %d \n",k,j,i,val & 1<< j, i%2);
	    result++;
	  }
	  if (i%2>0 && (val & 1<<j)==0) {   
	    printf("Shift in: module %d chip %i bit %d read %d instead of %d \n",k,j,i,val & 1<< j, i%2);
	    result++;
	  }
	}
	//}
      putout("0010000000000000",ALLMOD); 
      putout("0010000000000000",ALLMOD); 
      putout("0000000000000000",ALLMOD); 
      
    }
    putout("0000000000000000", ALLMOD); 
    printf("Shift in module %d : %d errors\n", imod,result);
    if (result)
      return 1;
    else
      return 0;
}

int testShiftOut(int imod) {  
  int i, j, val,k, result=OK; 
  int dum;
  dum=0xaaaaaa;

  printf("testing shift out for module %d\n", imod);

  setCSregister(ALLMOD);
  for (i=0; i<24; i++) {
    if (dum & 1<<i) {
      putout("0100010000000000",ALLMOD);  
      putout("0110010000000000",ALLMOD);  
      putout("0110010000000000",ALLMOD);  
      putout("0100010000000000",ALLMOD);  
    } else {
      putout("0000010000000000",ALLMOD);  
      putout("0010010000000000",ALLMOD);  
      putout("0010010000000000",ALLMOD);  
      putout("0000010000000000",ALLMOD); 
    }
  }
  putout("0000000100000000",ALLMOD); 

  for (i=0; i<24; i++) {
    putout("0000010000000000",ALLMOD);
    putout("0000010000000000",ALLMOD);  


    k=imod;
    //for (k=0; k<nModX; k++) {
    val=readin(k);
    //val=bus_r(MCB_DOUT_REG_OFF+(k<<SHIFTMOD)) & 0x3ff;
      //printf("%8x\n",val);
      for (j=0; j<10; j++) {	
	if ( (val & (1<<j))>0 && (dum & (1<<i))==0) {   
	  printf("Shift out: module %d chip %i bit %d read %d instead of %d \n",k,j,i,val & 1<< j, (dum &1<<j));
	  result++;
	}
	if ( (val & (1<<j))==0 && (dum & (1<<i))>0) {   
	  printf("Shift out: module %d chip %i bit %d read %d instead of %d \n",k,j,i,val & 1<< j, (dum &1<<j));
	  result++;
	}
      }
      //}
    putout("0010010000000000",ALLMOD);  
  }
  putout("0000000000000000", ALLMOD);  
  printf("Shift out module %d : %d errors\n", imod,result);
    if (result)
      return 1;
    else
      return 0;
}

int testShiftStSel(int imod) {
  int result=OK;
  int val,i,j,k;
  printf("testing shift stsel for module %d\n", imod);
  setCSregister(ALLMOD);  
  for (i=0; i<N_CHAN; i++) {
    if (i%2) {
      putout("0100011000000000",ALLMOD);  
      putout("0110011000000000",ALLMOD);  
      putout("0100011000000000",ALLMOD); 
    } else { 
      putout("0000011000000000",ALLMOD);  
      putout("0010011000000000",ALLMOD);  
      putout("0000011000000000",ALLMOD);
    }
  }
  putout("0010011000000000",ALLMOD); 
  for (i=0; i<N_CHAN; i++) {
    putout("0000011000000000",ALLMOD); 


    k=imod;
    //for (k=0; k<nModX; k++) {
    val=readin(k);
    //val=bus_r(MCB_DOUT_REG_OFF+(k<<SHIFTMOD)) & 0x3ff;
	for (j=0; j<N_CHIP; j++) {
	  if ( (val & 1<<j)>0 && i%2==0) {   
	    printf("Shift stsel: module %d chip %i bit %d read %d instead of %d \n",k,j,i,val & 1<< j, i%2);
	    result++;
	  }
	  if (i%2>0 && (val & 1<<j)==0) {   
	    printf("Shift stsel: module %d chip %i bit %d read %d instead of %d \n",k,j,i,val & 1<< j, i%2);
	    result++;
	  }
	}
	//}




    putout("0010011000000000",ALLMOD);  
  }  
  putout("0000011000000000",ALLMOD); 
  printf("Shift stsel module %d : %d errors\n", imod,result); 
    if (result)
      return 1;
    else
      return 0;
}




int testDataInOut(int num, int imod) {
  int val[N_CHIP*nModX], result=OK;
  int ich, ichip;
  setCSregister(ALLMOD);
  printf("Testing data in out for module %d pattern 0x%x\n", imod, num);
  setSSregister(ALLMOD);
  initChannel(0,0,0,0,0,num,ALLMOD);
  putout("0000000000000000",ALLMOD);
  setCSregister(ALLMOD);
  initChip(0, 0,ALLMOD);
  clearSSregister(ALLMOD);
  for (ich=0; ich<N_CHAN; ich++) {
    nextStrip(ALLMOD);
    readOutChan(val);
    //imod=0;
    //for (imod=0; imod<nModX; imod++) {
      for (ichip=0; ichip<N_CHIP; ichip++) {
	if (val[ichip+imod*N_CHIP]!=num) {
	  printf("Test datain out: Channel %d read %x instead of %x\n", (imod*N_CHIP+ichip)*N_CHAN+ich, val[ichip+N_CHIP*imod], num);
	result++;
	}
      }
      //}
  } 
  if (result)
    return 1;
  else
    return 0;
}




int testExtPulse(int imod) {
  int i, ichan, ichip, result=OK;
  int *val1;

  printf("Testing counter for module %d\n", imod);

  setCSregister(ALLMOD);
  setSSregister(ALLMOD);
  counterClear(ALLMOD);  
  putout("0000000000000000",ALLMOD);
  putout("0000100000000000",ALLMOD);
  putout("0000000000000000",ALLMOD);
  for (i=0; i<N_CHAN; i++) {
    putout("0000000000000000",ALLMOD);
    putout("0000000000001000",ALLMOD);
    putout("0000000000000000",ALLMOD);
    extPulse(1,ALLMOD);
  }
  clearSSregister(ALLMOD);
  putout("0000000000000000",ALLMOD);

 
  // Readout with SM 

  //startStateMachine();
  startReadOut();
  usleep(100);
  val1=(int*)(decode_data((int*)(fifo_read_event(0))));
  // val1=fifo_read_event();
  //imod=0;
  //for (imod=0; imod<nModX; imod++) {
  for (ichip=0; ichip<N_CHIP; ichip++) {
      for (ichan=0; ichan<N_CHAN; ichan++) {//
	if ((*(val1+ichan+(ichip+imod*N_CHIP)*N_CHAN))!=ichan) {
	  result++;
	  printf("Counter test: channel %d read %d instead of %d\n",ichan+(ichip+imod*N_CHIP)*N_CHAN, val1[ichan+(ichip+imod*N_CHIP)*N_CHAN], ichan);
	  }
      }
  }
  //}
  free(val1);
  if (result)
    return 1;
  else
    return 0;
}


int testExtPulseMux(int imod, int ow) {

  int i, ichan, ichip, result=0,   ind, chipr=0;
  int  *values, *v1;  
  int vright,v;
  int nbit_mask=0xffffff;

  printf("Testing counter for module %d, mux %d\n", imod, ow);
  setExposureTime(0);
  setFrames(1);
  setTrains(1);


  if (ow==2)
    nbit_mask=0xffff;
  else if (ow==3)
    nbit_mask=0xff;
  else if (ow==4)
    nbit_mask=0xf;
  else if (ow==5)
    nbit_mask=0x1;
  


  setCSregister(ALLMOD);
  setSSregister(ALLMOD);
  counterClear(ALLMOD); 
  initChipWithProbes(0, ow,0,ALLMOD); 
  //  initChip(0, ow,ALLMOD);
  for (ichip=0; ichip<N_CHIP; ichip++) {
  setSSregister(ALLMOD);
    for (i=0; i<N_CHAN; i++) {
      putout("0000000000000000",ALLMOD);
      putout("0000000000001000",ALLMOD);
      putout("0000000000000000",ALLMOD);
      extPulse(1,ALLMOD);
    }
    nextChip(ALLMOD);
  }
  setCSregister(ALLMOD);
  clearSSregister(ALLMOD);
  putout("0000000000000000",ALLMOD);

  // Readout with SM 


  startReadOut();
  usleep(100);
  v1=(int*)(fifo_read_event(0));
  if (v1)
    values=(int*)(decode_data(v1));
  else {
    printf("no data found in fifos\n");
    return 1;
  }
  for (ichip=0; ichip<N_CHIP; ichip++) {
    chipr=0;
      for (ichan=0; ichan<N_CHAN; ichan++) {
	ind=ichan+(ichip+imod*N_CHIP)*N_CHAN;
	v=values[ind];
	vright=(ichan*(ichip+1))&nbit_mask;
	if (v!=vright) {
	  result++;
	  chipr++;
	  printf("Counter test mux %d mode: channel %d chip %d read %d instead of %d\n",ow, ichan+(ichip+imod*N_CHIP)*N_CHAN, ichip, v, vright);
	  //break;
	}
	//printf("\n");
      }
      if (chipr)
	printf("Test Counter  module %d chip%d mux %d: %d errors\n", imod,ichip, ow,chipr);
  }
  free(values);
  if (result)
    printf("Test Counter  module %d mux %d: %d errors\n", imod,ow,result);

  if (result)
    return 1;
  else
    return 0;

}

int testDataInOutMux(int imod, int ow, int num) {

  int  ichan, ichip, result=0, chipr=0,  ind;
  int vright,v;
  int nbit_mask=0xffffff;
  int *values, *v1;

  printf("Testing data inout for module %d, mux %d, pattern 0x%x\n", imod, ow, num);
  setExposureTime(0);
  setFrames(1);
  setTrains(1);


  if (ow==2)
    nbit_mask=0xffff;
  else if (ow==3)
    nbit_mask=0xff;
  else if (ow==4)
    nbit_mask=0xf;
  else if (ow==5)
    nbit_mask=0x1;

  vright=num&nbit_mask;

  setCSregister(ALLMOD);
  //printf("Testin data in out\n");
  setSSregister(ALLMOD);
  counterClear(ALLMOD); 
  initChannel(0,0,0,0,0,num,ALLMOD);
  putout("0000000000000000",ALLMOD);
  clearSSregister(ALLMOD);
  initChipWithProbes(0, ow,0,ALLMOD);
  clearSSregister(ALLMOD);
  putout("0000000000000000",ALLMOD);

  // Readout with SM 

  printf("mux %d\n",ow);
  startReadOut();
  usleep(100);  
  v1=(int*)(fifo_read_event(0));
  if (v1)
    values=(int*)(decode_data(v1));
  else {
    printf("no data found in fifos\n");
    return 1;
  }
  for (ichip=0; ichip<N_CHIP; ichip++) {
    chipr=0;
      for (ichan=0; ichan<N_CHAN; ichan++) {
	ind=ichan+(ichip+imod*N_CHIP)*N_CHAN;
	v=values[ind];
	if (v!=vright) {
	  result++;
	  chipr++;
	  printf("DataInOut test mux %d mode: channel %d chip %d  read %d instead of %d\n",ow, ichan+(ichip+imod*N_CHIP)*N_CHAN, ichip, v, vright);
	  //break;
	}
	//printf("\n");
      }
      if (chipr)
	printf("Test DatInOut module %d chip %d mux %d: %d errors\n", imod,ichip, ow,chipr);
  }
  if (result)
    printf("Test DatInOut module %d mux %d: %d errors\n", imod,ow,result);
  free(values);
  if (result)
    return 1;
  else
    return 0;

}


int testOutMux(int imod) {
  int ibit, i, val, v, j, dist, k;
  int result=OK;
  long pat=0xf0f0f0;
  printf("testing outmux for module %d\n", imod);
  setCSregister(ALLMOD);
  // Clear outshift reg 
  putout("0000010000000000",ALLMOD);
  for (ibit=0; ibit<10;ibit++)
    putout("0000110000000000",ALLMOD);
  putout("0000010000000000",ALLMOD);

  // Clear inshift reg 
  putout("0000000000000000",ALLMOD);
  for (ibit=0; ibit<10;ibit++)
    putout("0000100000000000",ALLMOD);
  putout("0000000000000000",ALLMOD);

  // input pattern 0f0f0f 

  for (ibit=0; ibit<24;ibit++) {
    if (pat & (1 << ibit)) {
      putout("0100010000000000",ALLMOD);
      putout("0110010000000000",ALLMOD); //write in the data 1 
      putout("0100010000000000",ALLMOD);

    } else {
      putout("0000010000000000",ALLMOD);
      putout("0010010000000000",ALLMOD); //write in the data 0 
      putout("0000010000000000",ALLMOD);
    }
  }  
  
  putout("0100000000000000",ALLMOD);
  putout("0110000000000000",ALLMOD); //shift in 1 
  putout("0100000000000000",ALLMOD);

  for (ibit=0; ibit<20;ibit++) {
    putout("0000000000000000",ALLMOD);
    putout("0010000000000000",ALLMOD); //shift in 20 bits 
    putout("0000000000000000",ALLMOD);
  }  
      
  // check pattern, with different oumux settings 

    for (ibit=0; ibit<4;ibit++) { 
      dist=6-2*ibit;
      //dist=2*ibit;
      if (dist==0)
	dist=1;
#ifdef DEBUGOUT
      printf("Distance is %d\n",dist);
#endif


   
      putout("0000010001000000",ALLMOD);
      putout("0000010001000000",ALLMOD); //output is out0
      putout("0000010001000000",ALLMOD);

      i=0;
      k=imod;
      //for (k=0; k<nModX; k++) {
      val=readin(k);
      //val=bus_r(MCB_DOUT_REG_OFF+(k<<SHIFTMOD)) & 0x3ff; 
#ifdef DEBUGOUT
      printf("%d %x\n",i*dist,val);     
#endif
      for (j=0; j<N_CHIP; j++) {
	v=val & 1<< j;
	if (pat & (1<<(i*dist))) {
	  if (v==0) {
	    printf("Outmux: module %d chip %i bit %d  read %d instead of %ld\n",k, j,(i*dist), v,pat & (1<<(i*dist) ));
	    result++;
	  }
	  // should be 1
	} else { 
	  if (v) {
	    printf("Outmux: module %d chip %i bit %d  read %d instead of %ld\n",k, j,(i*dist), v,pat & (1<<(i*dist) ));
	    result++;
	  }
	}
      }
      //}
      putout("0000010001100000",ALLMOD);
      putout("0000010001100000",ALLMOD); //output is out1
      putout("0000010001100000",ALLMOD);

      i++;
      k=imod;
      //for (k=0; k<nModX; k++) {
      val=readin(k);
      //val=bus_r(MCB_DOUT_REG_OFF+(k<<SHIFTMOD)) & 0x3ff; 
#ifdef DEBUGOUT
      printf("%d %x\n",i*dist, val);
#endif
      for (j=0; j<N_CHIP; j++) {
	v=val & 1<< j;
	if (pat & (1<<(i*dist))) {
	  if (v==0) {
	    printf("Outmux: module %d chip %i bit %d  read %d instead of %ld\n",k, j,(i*dist), v,pat & (1<<(i*dist) ));
	    result++;
	  }
	  // should be 1
	} else { 
	  if (v) {
	    printf("Outmux: module %d chip %i bit %d  read %d instead of %ld\n",k, j,(i*dist), v,pat & (1<<(i*dist) ));
	    result++;
	  }
	}
      }
      //}




      putout("0000010001010000",ALLMOD);
      putout("0000010001010000",ALLMOD); //output is out2
      putout("0000010001010000",ALLMOD);


      i++;
      k=imod;
      //for (k=0; k<nModX; k++) {
      val=readin(k);
      //val=bus_r(MCB_DOUT_REG_OFF+(k<<SHIFTMOD)) & 0x3ff; 
#ifdef DEBUGOUT
      printf("%d %x\n",i*dist, val);
#endif
      for (j=0; j<N_CHIP; j++) {
	v=val & 1<< j;
	if (pat & (1<<(i*dist))) {
	  if (v==0) {
	    printf("Outmux: module %d chip %i bit %d  read %d instead of %ld\n",k, j,(i*dist), v,pat & (1<<(i*dist) ));
	    result++;
	  }
	  // should be 1
	} else { 
	  if (v) {
	    printf("Outmux: module %d chip %i bit %d  read %d instead of %ld\n",k, j,(i*dist), v,pat & (1<<(i*dist) ));
	    result++;
	  }
	}
      }
      //}

      putout("0000010001110000",ALLMOD);
      putout("0000010001110000",ALLMOD); //output is out3
      putout("0000010001110000",ALLMOD);
      i++;
      k=imod;
      //for (k=0; k<nModX; k++) {
      val=readin(k);
      //val=bus_r(MCB_DOUT_REG_OFF+(k<<SHIFTMOD)) & 0x3ff; 
#ifdef DEBUGOUT
      printf("%d %x\n",i*dist, val);
#endif
      for (j=0; j<N_CHIP; j++) {
	v=val & 1<< j;
	if (pat & (1<<(i*dist))) {
	  if (v==0) {
	    printf("Outmux: module %d chip %i bit %d  read %d instead of %ld\n",k, j,(i*dist), v,pat & (1<<(i*dist) ));
	    result++;
	  }
	  // should be 1
	} else { 
	  if (v) {
	    printf("Outmux: module %d chip %i bit %d  read %d instead of %ld\n",k, j,(i*dist), v,pat & (1<<(i*dist) ));
	    result++;
	  }
	}
      }
      //}

      putout("0000000000000000",ALLMOD);
      putout("0010000000000000",ALLMOD); //change mux setting 
      putout("0000000000000000",ALLMOD);
    }  
    
    printf("Test OutMux module %d : %d errors\n", imod,result);
  if (result)
    return 1;
  else
    return 0;
}






int testFpgaMux(int imod)  {
  int ibit, i, val, v, j, dist,k;
  int result=OK;
  long pat=0xf0f0af;
  printf("testing fpga mux of module %d\n",imod);

  // Clear outshift reg 
  putout("0000010000000000",ALLMOD);
  for (ibit=0; ibit<10;ibit++)
    putout("0000110000000000",ALLMOD);
  putout("0000010000000000",ALLMOD);

  // Clear inshift reg 
  putout("0000000000000000",ALLMOD);
  for (ibit=0; ibit<10;ibit++)
    putout("0000100000000000",ALLMOD);
  putout("0000000000000000",ALLMOD);

  // input pattern 0f0f0f 

  for (ibit=0; ibit<24;ibit++) {
    if (pat & (1 << ibit)) {
      putout("0100010000000000",ALLMOD);
      putout("0110010000000000",ALLMOD); //write in the data 1 
      putout("0100010000000000",ALLMOD);

    } else {
      putout("0000010000000000",ALLMOD);
      putout("0010010000000000",ALLMOD); //write in the data 0 
      putout("0000010000000000",ALLMOD);
    }
  }  
  
  putout("0100000000000000",ALLMOD);
  putout("0110000000000000",ALLMOD); //shift in 1 
  putout("0100000000000000",ALLMOD);

  for (ibit=0; ibit<20;ibit++) {
    putout("0000000000000000",ALLMOD);
    putout("0010000000000000",ALLMOD); //shift in 20 bits 
    putout("0000000000000000",ALLMOD);
  }  
      
  // check pattern, with different oumux settings 

    for (ibit=0; ibit<4;ibit++) { 
      dist=6-2*ibit;
      //dist=2*ibit;
      if (dist==0)
	dist=1;
#ifdef DEBUGOUT
      printf("Distance is %d\n",dist);
#endif


    
      //This should test the parallel to serial converter

      putout("0000010000000000",ALLMOD) ; 	
      putout("0000010100000000",ALLMOD) ; // reset readout		     
      putout("0000010100000000",ALLMOD) ; 		     
      putout("0000010000000000",ALLMOD) ; 


#ifdef DEBUGOUT
	  printf("testing FPGA Mux\n");
#endif  
		     
      for (i=0; i<4; i++) {
	k=imod;
	//for (k=0; k<nModX; k++) {
	val=readin(k);
	//val=bus_r(MCB_DOUT_REG_OFF+(k<<SHIFTMOD)) & 0x3ff; 
  

#ifdef DEBUGOUT
	  printf("%d %x\n",i*dist, val);
#endif    
	  for (j=0; j<10; j++) {
	    v=val & 1<< j;
	    if (pat & (1<<(i*dist))) {
	      if (v==0) {
		printf("Fpgamux: module %d chip %i bit %d  read %d instead of %ld\n",k,j,(i*dist), v,pat & (1<<(i*dist) ));
		result++;
	      }
	  // should be 1
	    } else { 
	      if (v) {
		printf("Fpgamux: module %d chip %i bit %d  read %d instead of %ld\n",k,j,(i*dist), v,pat & (1<<(i*dist) ));
		result++;
	      }
	    }
	  }
	  //}
	putout("0000010000000000",ALLMOD);
	putout("0010010000000000",ALLMOD); //output
	putout("0000010000000000",ALLMOD );

      }
      pat = pat >> 1;
     

      putout("0000000000000000",ALLMOD);
      putout("0010000000000000",ALLMOD); //change mux setting 
      putout("0000000000000000",ALLMOD);
    }  
    
    printf("Test FpgaMux module %d : %d errors\n", imod,result);
  if (result)
    return 1;
  else
    return 0;
}









int calibration_sensor(int num, int *v, int *dacs) {
  int ich, ichip, imod;
  int val[10];

 
    printf("calibrating sensor...");
  for (imod=0; imod<nModX; imod++) {
    //selMod(imod);   
    for (ichip=0; ichip<10; ichip++) { 
      selChip(ichip,imod);
      for (ich=0; ich<128; ich++){ 
	selChannel(ich,imod);
	initChannel(*(dacs+ichip*N_CHAN+ich),0,0,0,0,0,imod); //disable channel;
	clearCounter(imod);      
	//initChannel(*(dacs+ichip*N_CHAN+ich),FALSE,FALSE,TRUE,FALSE,0); //disable channel;
      }
    }
  }
 
  for (imod=0; imod<nModX; imod++) {
    //selMod(imod);
    initMCBregisters(1,imod);
    for (ich=0; ich<N_CHAN; ich++){
      for (ichip=0; ichip<N_CHIP; ichip++) {
	selChip(ichip,imod); // select channel 
	selChannel(ich,imod); // select channel 
	initChannel(*(dacs+imod*N_CHAN*N_CHIP+ichip*N_CHAN+ich),0,0,1,0,0,imod); // enable channel
	clearCounter(imod);      
      }
      setCSregister(imod); 
      clearSSregister(imod);
      countEnable(imod);
      usleep(20);
      calPulse(num,imod); // give pulses//	  
      usleep(20);
      //for (ich=0; ich<128; ich++){  
      selChannel(ich,imod); // select channel
    } 
    readOutChan(val); // readout channel
    for (imod=0; imod<nModX; imod++) {
      for (ichip=0; ichip<N_CHIP; ichip++) {
	*(v+(ichip+imod*N_CHIP)*N_CHAN+ich)=val[ichip+imod*N_CHIP];
	selChip(ichip,imod); // select channel 
	selChannel(ich,imod); // select channel 
	initChannel(*(dacs+ichip*N_CHAN+ich),0,0,0,0,0,imod); //disable channel;
      }
    }
  }
  printf("done\n");
  return OK; 
}

int calibration_chip(int num, int *v, int *dacs) {
  int ich, ichip, imod;
  int val[10];

 
  printf("calibrating chip...");
  for (imod=0; imod<nModX; imod++) {
    //selMod(imod);
    initMCBregisters(0,imod);
    for (ichip=0; ichip<10; ichip++) { 
      selChip(ichip,imod);
      for (ich=0; ich<128; ich++){ 
	selChannel(ich,imod);
	clearCounter(imod); 
	initChannel(*(dacs+ichip*N_CHAN+ich),0,0,0,0,0,imod); //disable channel;
      }
    }
  }
  for (ich=0; ich<N_CHAN; ich++){
    for (imod=0; imod<nModX; imod++) {
      //selMod(imod);
      for (ichip=0; ichip<N_CHIP; ichip++) {
	selChip(ichip,imod); // select channel 
	selChannel(ich,imod); // select channel 
	initChannel(*(dacs+imod*N_CHAN*N_CHIP+ichip*N_CHAN+ich),1,0,1,0,0,imod); // enable channel
	clearCounter(imod);      
      }
    }
    //setMSregister();
    setCSregister(ALLMOD); 
    clearSSregister(ALLMOD);
    countEnable(ALLMOD);
    usleep(20);
    calPulse(num,ALLMOD); // give pulses//	  
    usleep(20);  
    selChannel(ich,ALLMOD); // select channel 
    readOutChan(val); // readout channel
    for (imod=0; imod<nModX; imod++) {
      //selMod(imod);
      for (ichip=0; ichip<10; ichip++) {
	*(v+(ichip+imod*N_CHIP)*N_CHAN+ich)=val[ichip+imod*N_CHIP];
	selChip(ichip,imod); // select chip 
	selChannel(ich,imod); // select channel 
	initChannel(*(dacs+imod*N_CHAN*N_CHIP+ichip*N_CHAN+ich),0,0,0,0,0,imod); //disable channel;
      }
    }
  }
  printf("done\n");
  return OK;  
}



/* ROI* setROI(int n, ROI arg[], int *retvalsize, int *ret){ */

/* 	int i,adc; */
/* 	ROI temp; */

/* 	if(n>=0){ */

/* 		//clear rois */
/* 		for(i=0;i<MAX_ROIS;i++) */
/* 			rois[i]=temp; */

/* 		/\*ideal way to set roi */
/* 		for(i=0;i<n;i++) */
/* 			rois[i]=arg[i]; */
/* 		nROI = n; */
/* 		 *\/ */

/* 		if(n==0) */
/* 			adc=-1; */
/* 		else{ */
/* 			//if its for 1 adc or general */
/* 			if ((arg[0].xmin==0) && (arg[0].xmax==GOTTHARDNCHIP*GOTTHARDNCHAN)) */
/* 				adc=-1; */
/* 			else{ */
/* 				//adc = mid value/numchans also for only 1 roi */
/* 				adc = ((((arg[0].xmax)+(arg[0].xmin))/2)/(GOTTHARDNCHAN*NCHIPS_PER_ADC)); */
/* 				if((adc>=0) && (adc<=4)); */
/* 				else { */
/* 					printf("warning:adc value greater than 5. deleting roi\n"); */
/* 					adc=-1; */
/* 				} */
/* 			} */
/* 		} */


/* 		//set rois for just 1 adc - take only 1st roi */
/* 		if(adc!=-1){ */
/* 			rois[0].xmin=adc*(GOTTHARDNCHAN*NCHIPS_PER_ADC); */
/* 			rois[0].xmax=(adc+1)*(GOTTHARDNCHAN*NCHIPS_PER_ADC)-1; */
/* 			rois[0].ymin=-1; */
/* 			rois[0].ymax=-1; */
/* 			nROI = 1; */
/* 		}else */
/* 			nROI = 0; */

/* 		if((arg[0].xmin!=rois[0].xmin)||(arg[0].xmax!=rois[0].xmax)||(arg[0].ymin!=rois[0].ymin)||(arg[0].ymax!=rois[0].ymax)) */
/* 			*ret=FAIL; */
/* 		if(n!=nROI) */
/* 			*ret=FAIL; */

/* 		//set adc of interest */
/* 		setADC(adc); */
/* 	} */

/* //#ifdef VERBOSE */
/* 	printf("Rois:\n"); */
/* 	for( i=0;i<nROI;i++) */
/* 		printf("%d\t%d\t%d\t%d\n",rois[i].xmin,rois[i].xmax,rois[i].ymin,rois[i].ymax); */
/* //#endif */
/* 	*retvalsize = nROI; */
/* 	return rois; */
/* } */

#endif


