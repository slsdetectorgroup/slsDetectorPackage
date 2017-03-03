#ifdef MCB_FUNCS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "registers.h"

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
extern int nModBoard;
extern int dataBytes;
extern int dynamicRange;
const int nChans=NCHAN;
const int nChips=NCHIP;
const int nDacs=NDAC;
const int nAdcs=NADC;
const enum detectorType myDetectorType=MYTHEN;
enum detectorSettings thisSettings;

int sChan, sChip, sMod, sDac, sAdc;
const int allSelected=-2;
const int noneSelected=-1;


sls_detector_module *detectorModules=NULL;
int *detectorChips=NULL;
int *detectorChans=NULL;
dacs_t *detectorDacs=NULL;
dacs_t *detectorAdcs=NULL;
//int numberOfProbes;








int initDetector() {
  int imod;
  //  sls_detector_module *myModule;
  int n=getNModBoard();
  nModX=n;
  //#ifdef VERBOSE
  printf("Board is for %d modules\n",n);
  //#endif
  detectorModules=malloc(n*sizeof(sls_detector_module));
  detectorChips=malloc(n*NCHIP*sizeof(int));

  detectorChans=malloc(n*NCHIP*NCHAN*sizeof(int));
  detectorDacs=malloc(n*NDAC*sizeof(dacs_t));
  detectorAdcs=malloc(n*NADC*sizeof(dacs_t));
  //#ifdef VERBOSE
  printf("modules from 0x%x to 0x%x\n",detectorModules, detectorModules+n);
  printf("chips from 0x%x to 0x%x\n",detectorChips, detectorChips+n*NCHIP);
  printf("chans from 0x%x to 0x%x\n",detectorChans, detectorChans+n*NCHIP*NCHAN);
  printf("dacs from 0x%x to 0x%x\n",detectorDacs, detectorDacs+n*NDAC);
  printf("adcs from 0x%x to 0x%x\n",detectorAdcs, detectorAdcs+n*NADC);
  //#endif
  for (imod=0; imod<n; imod++) {
 printf("module %d\n",imod);
 
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
  printf("modules done\n");
  thisSettings=UNINITIALIZED;
  sChan=noneSelected;
  sChip=noneSelected;
  sMod=noneSelected;
  sDac=noneSelected;
  sAdc=noneSelected;
  setCSregister(ALLMOD);
  setSSregister(ALLMOD);
  counterClear(ALLMOD);
  clearSSregister(ALLMOD); 
  putout("0000000000000000",ALLMOD);

  printf("dr\n");
  /* initialize dynamic range etc. */
  dynamicRange=getDynamicRange();
  printf("nmod\n");
  nModX=setNMod(-1);
  printf("done\n");

  



  //dataBytes=nModX*NCHIP*NCHAN*4;
  // dynamicRange=32;
  // initChip(0, 0,ALLMOD);
  //nModX=n; 
  //
  // allocateRAM();


  

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
    printf("Copying module %x to module %x\n",srcMod,destMod);
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



/* Register commands */


int clearDACSregister(int imod) {
  putout("0000000001000000",imod);  
  putout("0000000101000000",imod);  
  putout("0000000101000000",imod);
  putout("0000000001000000",imod);
#ifdef DEBUG
  fprintf(stdout, "Clearing DAC shiftregister\n");
#endif 
  /*  
      sChan=noneSelected;
      sChip=noneSelected;
      sMod=noneSelected;
      sDac=noneSelected;
      sAdc=noneSelected;
  */
  sDac=0;
  sMod=imod;
  if (imod==ALLMOD)
    sMod=allSelected;
  return OK;
}

int nextDAC(int imod) {
  putout("0000000001000000",imod);      
  putout("0000000001001000",imod);
  putout("0000000001000000",imod);
#ifdef DEBUG
  fprintf(stdout, "Next DAC\n");
#endif 
  sDac++; 
  sMod=imod;
  if (imod==ALLMOD)
    sMod=allSelected;
  return OK;
}


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

int program_one_dac(int addr, int value, int imod) {
  
  int control, ibit;
  int bit;
  int im;
  int idac;
  int v=value;
  //  int reg, mask;
  
  //  sls_detector_module *myMod;
  control=9+addr; 

  int ireg;



  printf("programming dac %d value %d module %d\n",addr, value,imod);
#ifdef MAX5533
  value=value | (control<< 12);
#endif
#ifndef MAX5533
  value=(value<<2) |  (control<< 12);
#endif
#ifdef DEBUGOUT
      fprintf(stdout,"value=%d\n",value);
#endif
  for (ibit=0; ibit<16; ibit++) {
    bit=value & (1<<(15-ibit)); 
    if (bit) {
      putout("0000010001000000",imod);
      putout("0000011001000000",imod);
      putout("0000010001000000",imod);
#ifdef DEBUGOUT
      fprintf(stdout,"1");
#endif
    } else {
      putout("0000000001000000",imod);         
      putout("0000001001000000",imod);
      putout("0000000001000000",imod);
#ifdef DEBUGOUT
      fprintf(stdout,"0");
#endif
    }
  } 
#ifdef DEBUGOUT
  fprintf(stdout,"\n");
#endif
  idac=sDac*2+addr;


  ireg=idac;

  if (idac==VCAL) {
    idac=RGPR;
  }  else if (idac==RGPR){
    idac=VCAL;
  }


  if (detectorDacs) {
    sMod=imod;
    if (imod==ALLMOD)
      sMod=allSelected;
    
    if (imod>=0 && imod<nModX) {
      //  myMod=detectorModules+imod;
      //(detectorModules+imod)->dacs[idac]=v;
      detectorDacs[ireg+NDAC*imod]=v;
      //#ifdef VERBOSE  
#ifdef VERBOSE
      printf("module=%d index=%d, val=%d addr=%x\n",imod, idac, v, detectorDacs+idac+NDAC*imod);
#endif

      setDACRegister(ireg,v,imod);

      /*

      reg=bus_r(MOD_DACS1_REG+(imod<<SHIFTMOD));
#ifdef VERBOSE
      printf("DACS register 1 is %x\n",reg);
#endif
      if ((idac%2)==0) {
	
#ifdef VERYVERBOSE
	printf("DACS %d div2 should be 0 is %d\n",idac, idac%2);
#endif
	mask=~((0x3ff)<<((idac%3)*10));
	reg&=mask;
	reg|=(v<<((idac%3)*10));
#ifdef VERYVERBOSE
	printf("DACS %d div3  is %d\n",idac, idac%3);
#endif
	bus_w(MOD_DACS1_REG+(imod<<SHIFTMOD), reg);
      }
#ifdef VERBOSE
      printf("after setting dac %d module %d -- %x -- %x\n",idac, imod, reg, bus_r(MOD_DACS1_REG+(imod<<SHIFTMOD)));
#endif
      reg=bus_r(MOD_DACS2_REG+(imod<<SHIFTMOD));
#ifdef VERBOSE
      printf("DACS register 2 is %x\n",reg);
#endif
      if ((idac%2)==1) {
#ifdef VERYVERBOSE
	printf("DACS %d div2 should be 1 is %d\n",idac, idac%2);
#endif
	mask=~((0x3ff)<<((idac%3)*10));
	reg&=mask;
	reg|=(v<<((idac%3)*10));
#ifdef VERYVERBOSE
	printf("DACS %d div3  is %d\n",idac, idac%3);
#endif
	bus_w(MOD_DACS2_REG+(imod<<SHIFTMOD), reg);
      }

#ifdef VERBOSE
      printf("after setting dac %d module %d -- %x -- %x\n",idac, imod, reg, bus_r(MOD_DACS2_REG+(imod<<SHIFTMOD)));
#endif
      */
      //#endif
    } else if (imod==ALLMOD) {
      for (im=0; im<nModX; im++) {
	detectorDacs[ireg+NDAC*im]=v;

	/*
#ifdef VERBOSE
	printf("all: module=%d index=%d, val=%d addr=%x\n",im, idac, v,detectorDacs+idac+NDAC*im);
#endif
	//	(detectorModules+im)->dacs[idac]=v;



      reg=bus_r(MOD_DACS1_REG+(imod<<SHIFTMOD));
#ifdef VERBOSE
      printf("DACS register 1 is %x\n",reg);
#endif
      if ((idac%2)==0) {
#ifdef VERYVERBOSE
	printf("DACS %d div2 should be 0 is %d\n",idac, idac%2);
#endif
	mask=~((0x3ff)<<((idac%3)*10));
	reg&=mask;
	reg|=(v<<((idac%3)*10));
#ifdef VERYVERBOSE
	printf("DACS %d div3  is %d\n",idac, idac%3);
#endif
	bus_w(MOD_DACS1_REG+(imod<<SHIFTMOD), reg);
      }
#ifdef VERBOSE
      printf("after setting dac %d module %d -- %x -- %x\n",idac, im, reg, bus_r(MOD_DACS1_REG+(im<<SHIFTMOD)));
#endif
      reg=bus_r(MOD_DACS2_REG+(im<<SHIFTMOD));
#ifdef VERBOSE
      printf("DACS register 2 is %x\n",reg);
#endif
      if ((idac%2)==1) {
#ifdef VERYVERBOSE
	printf("DACS %d div2 should be 1 is %d\n",idac, idac%2);
#endif
	mask=~((0x3ff)<<((idac%3)*10));
	reg&=mask;
	reg|=(v<<((idac%3)*10));
#ifdef VERYVERBOSE
	printf("DACS %d div3  is %d\n",idac, idac%3);
#endif
	bus_w(MOD_DACS2_REG+(im<<SHIFTMOD), reg);
      }

#ifdef VERBOSE
      printf("after setting dac %d module %d -- %x -- %x addr %d\n",idac, im, reg, bus_r(MOD_DACS2_REG+(im<<SHIFTMOD)));
#endif
	*/
	setDACRegister(ireg,v,im);
      }
    }
  }
  return OK;
}

int set_one_dac(int imod) {
  int control, ibit;
  int bit, value;
  control=13;
  value=(DAC_REFOUT<<10) | (control<< 12);
#ifdef DEBUGOUT
      fprintf(stdout,"value=%d\n",value);
#endif
  for (ibit=0; ibit<6; ibit++) {
    bit=value & (1<<(15-ibit)); 
    if (bit) {
      putout("0000010001000000",imod);
      putout("0000011001000000",imod);
      putout("0000010001000000",imod);
#ifdef DEBUGOUT
      fprintf(stdout,"1");
#endif
    } else {
      putout("0000000001000000",imod);         
      putout("0000001001000000",imod);
      putout("0000000001000000",imod);
#ifdef DEBUGOUT
      fprintf(stdout,"0");
#endif
    }
  } 
  for (ibit=0; ibit<10; ibit++) {
    putout("0000000001000000",imod);         
    putout("0000001001000000",imod);
    putout("0000000001000000",imod);
#ifdef DEBUGOUT
    fprintf(stdout,"0");
#endif
  }   
  
#ifdef DEBUGOUT
      fprintf(stdout,"\n");
#endif
  return OK;
}

dacs_t initDACbyIndex(int ind,dacs_t ival, int imod) {
  int v;
  const float partref[NDAC]=PARTREF;
  const float partr1[NDAC]=PARTR1;
  const float partr2[NDAC]=PARTR2;
 
  float val;
#ifdef DACS_INT  
  val=((float)ival)*1E-3;
#else
  val=ival;
#endif

  float ref=partref[ind];
  float r1=partr1[ind];
  float r2=partr2[ind];
  

  v=(val+(val-ref)*r1/r2)*DAC_DR/DAC_MAX;
  v=initDACbyIndexDACU(ind,v,imod);
  
  return (v*DAC_MAX/DAC_DR+ref*r1/r2)/(1+r1/r2);
}

dacs_t initDACbyIndexDACU(int ind, dacs_t val, int imod) {
 
  const int daccs[NDAC]=DACCS;
  const int dacaddr[NDAC]=DACADDR;
 
  int cs=daccs[ind];
  int addr=dacaddr[ind];
  //  int iv;
  int im;

  if (val>=0) {
    initDAC(cs, addr,val, imod);
  /*#ifdef VERBOSE
  iv=detectorDacs[ind+imod*NDAC];
  printf("module=%d index=%d, cs=%d, addr=%d, dacu=%d, set to %d",imod, ind,cs,addr,val,iv);
#endif  
  */
    //return val;
  }
  if (imod>=0 && imod<nModX) {
    // return detectorDacs[ind+imod*NDAC];
    return setDACRegister(ind,  -1, imod);
  }
  else {
    //iv=detectorDacs[ind];
#ifdef VERBOSE
    printf("mod 0 of %d dac %d val %d\n",nModX,ind,setDACRegister(ind,  -1, 0));
#endif
    for (im=1; im<nModX; im++) {
#ifdef VERBOSE
      printf("mod %d dac %d val %d\n",im,ind,setDACRegister(ind,  -1, im));
#endif
      //if (detectorDacs[ind+im*NDAC]!=detectorDacs[ind]) {
	
      if (setDACRegister(ind,  -1, im)!=setDACRegister(ind,  -1, 0)) {
#ifdef VERBOSE
      printf("mod %d returning -1\n",im);
#endif
	return -1;
      }
    }
#ifdef VERBOSE
      printf("returning %d\n",setDACRegister(ind,  -1, 0));
#endif
    return setDACRegister(ind,  -1, 0);
  }
}

int getThresholdEnergy() {
  float g[3]=DEFAULTGAIN;
  float o[3]=DEFAULTOFFSET;
  float myg=-1, myo=-1;
  //  int dacu;
  int imod;
  int ethr=-1;
  int ret=FAIL;

  if (detectorModules) {
    //  for (imod=0; imod<getNModBoard(); imod++) {
    for (imod=0; imod<nModX; imod++) {
#ifdef VERBOSE
      printf("module=%d settings=%d, gain=%f, offset=%f\n",imod,thisSettings, (detectorModules+imod)->gain,(detectorModules+imod)->offset);
#endif
      if ((detectorModules+imod)->gain>0)
	myg=(detectorModules+imod)->gain;
      else {
	if (thisSettings>=0 && thisSettings<3)
	  myg=g[thisSettings];
	//	else
	//myg=-1;
      }

      if ((detectorModules+imod)->offset>0)
	myo=(detectorModules+imod)->offset;
      else {
	if (thisSettings>=0 && thisSettings<3)
	  myo=o[thisSettings];
	//	else
	//myo=-1;
      }

      if (myg>0 && myo>0) {
	//ethr=(myo-detectorDacs[VTHRESH+imod*NDAC])*1000/myg;
      
	ethr=(myo-setDACRegister(VTHRESH,-1,imod))*1000/myg;
	// else
      //	ethr=-1;
      
      }
#ifdef VERBOSE
#ifdef DACS_INT
      printf("module=%d gain=%f, offset=%f, dacu=%d\n",imod, myg, myo,setDACRegister(VTHRESH,-1,imod));
#else
      printf("module=%d gain=%f, offset=%f, dacu=%f\n",imod, myg, myo,setDACRegister(VTHRESH,-1,imod));
#endif
      printf("Threshold energy of module %d is %d eV\n", imod, ethr);
#endif 
      
      if (imod==0)
	ret=ethr;
      else {
	if (ethr>(ret+100) || ethr<(ret-100))
	  return FAIL;
      }
    }
  }
  return ret;
}

int setThresholdEnergy(int ethr) {
  float g[3]=DEFAULTGAIN;
  float o[3]=DEFAULTOFFSET;
  float myg=-1, myo=-1;
  int dacu;
  int imod;
  int ret=ethr;

  setSettings(GET_SETTINGS);
  if (thisSettings>=0 || thisSettings<3){
    myg=g[thisSettings];
    myo=o[thisSettings];
  }
  for (imod=0; imod<nModX; imod++) {
    if (detectorModules) {
      if ((detectorModules+imod)->gain>0)
	myg=(detectorModules+imod)->gain;
      else  
	if (thisSettings>=0 && thisSettings<3)
	  myg=g[thisSettings];
	else
	  myg=-1;
      if ((detectorModules+imod)->offset>0)
	myo=(detectorModules+imod)->offset;
      else
	if (thisSettings>=0 && thisSettings<3)
	  myo=o[thisSettings];
	else
	  myo=-1;
    } else {
	if (thisSettings>=0 && thisSettings<3)
	  myo=o[thisSettings];
	else
	  myo=-1;
	if (thisSettings>=0 && thisSettings<3)
	  myg=g[thisSettings];
	else
	  myg=-1;
    }
    if (myg>0 && myo>0) {
      dacu=myo-myg*((float)ethr)/1000.;
#ifdef VERBOSE    
      printf("module %d (%x): gain %f, off %f, energy %d eV, dac %d\n",imod,(detectorModules+imod),(detectorModules+imod)->gain,(detectorModules+imod)->offset, ethr,dacu);
#endif
    } else {
      dacu=ethr;
#ifdef VERBOSE    
      printf("could not set threshold energy for module %d, settings %d (offset is %f; gain is %f)\n",imod,thisSettings,myo,myg);
#endif
    }
    initDACbyIndexDACU(VTHRESH, dacu, imod);
  }
  return ret;
}



dacs_t getDACbyIndexDACU(int ind,  int imod) {
  /*
  if (detectorDacs) {
    if (imod<getNModBoard())
      if (ind<(detectorModules+imod)->ndac)
	return (detectorDacs[ind+imod*NDAC]);
  }  
  return FAIL;
  */
  return setDACRegister(ind, -1, imod);
}



int initDAC(int dac_cs, int dac_addr, int value, int imod) {
  int i;
#ifdef VERBOSE
  printf("Programming dac %d %d with value %d\n", dac_cs, dac_addr, value);
#endif
  clearDACSregister(imod);
  /*if (dac_cs>0) {*/
  for (i=0; i<dac_cs; i++) {
    nextDAC(imod);
  }
  /*}*/
  program_one_dac(dac_addr,value,imod);
  nextDAC(imod);
  clearDACSregister(imod);
  /*if (dac_cs>0) {*/
  for (i=0; i<dac_cs; i++) {
    nextDAC(imod);
  }
  /*}*/
  set_one_dac(imod);
  nextDAC(imod);
  return 0;
}





int initDACs(int* v,int imod)
{
  int ichip, iaddr, i;
  for (iaddr=0; iaddr<2; iaddr++) {
    clearDACSregister(imod);
    for (ichip=0; ichip<3; ichip++) {
      i=ichip*2+iaddr;
      if (v[i]>=0) { 
#ifdef VERBOSE
	fprintf(stdout, "voltage %d\n", *(v+i));
#endif
	program_one_dac(iaddr, *(v+i),imod);
      }
      nextDAC(imod);
    }
  }
  clearDACSregister(imod);
  for (ichip=0; ichip<3; ichip++) {
    set_one_dac(imod);
    nextDAC(imod);
  }

  return 0;
}




int setSettings(int i) {
  int imod, isett, is;
  int rgpr[]=RGPRVALS;
  int rgsh1[]=RGSH1VALS;
  int rgsh2[]=RGSH2VALS;
  int irgpr, irgsh1, irgsh2;

  int v[NDAC];
  int ind;
  for (ind=0; ind<NDAC; ind++)
    v[ind]=-1;
 
  // printf("vind\n");
  if (i!=GET_SETTINGS) {
   switch (i) {
   case STANDARD:; case FAST:; case HIGHGAIN:
     v[RGPR]=rgpr[i];
     v[RGSH1]=rgsh1[i];
     v[RGSH2]=rgsh2[i];
     initDACs(v,ALLMOD);
     thisSettings=i;
   break;
   default:
     break;
   }
  } //else {
  imod=0;
  /* check settings for module 0 */
  isett=UNDEFINED;/*
  irgpr=detectorDacs[4+imod*NDAC];
  irgsh1=detectorDacs[imod*NDAC+RGSH1];
  irgsh2=detectorDacs[imod*NDAC+RGSH2];
	
  	  */
  // printf("regs\n");
  irgpr=setDACRegister(RGPR,-1,imod);
  irgsh1=setDACRegister(RGSH1,-1,imod);
  irgsh2=setDACRegister(RGSH2,-1,imod);
  for (is=STANDARD; is<sizeof(rgpr); is++){//;UNDEFINED; is++) {
    if (irgpr==rgpr[is] && irgsh1==rgsh1[is] &&  irgsh2==rgsh2[is]) {
      isett=is;
    }
  }
  //#ifdef VERBOSE
    // printf("Settings of module 0 are %d\n",isett);
  //#endif
  for (imod=1; imod<nModX; imod++) {
    if (isett!=UNDEFINED) {
      irgpr=setDACRegister(RGPR,-1,imod);
      irgsh1=setDACRegister(RGSH1,-1,imod);
      irgsh2=setDACRegister(RGSH2,-1,imod);
      /*
      irgpr=detectorDacs[4+imod*NDAC];
      irgsh1=detectorDacs[imod*NDAC+RGSH1];
      irgsh2=detectorDacs[imod*NDAC+RGSH2];
      */
      if (irgpr!=rgpr[isett] || irgsh1!=rgsh1[isett] ||  irgsh2!=rgsh2[isett]) {
	isett=UNDEFINED; 
#ifdef VERBOSE
	printf("Settings of module %d are undefined\n",imod);
	printf("%d %d %d \n", irgpr, irgsh1, irgsh2);
	printf("dacs 1: %08x dacs2: %08x\n",bus_r(MOD_DACS1_REG+(imod<<SHIFTMOD)), bus_r(MOD_DACS2_REG+(imod<<SHIFTMOD)));

#endif



      }
    }
  }
#ifdef VERBOSE
  printf("detector settings are %d\n",isett);
#endif
  if (isett==UNDEFINED && thisSettings==UNINITIALIZED) 
    ;
  else
    thisSettings=isett;

  return thisSettings;
}






/* Initialization*/
int initChannelbyNumber(sls_detector_channel myChan) {
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
	myChan->reg=detectorChans[imod*NCHAN*NCHIP+ichip*NCHAN+ichan];
      return OK;
    }
  } 
  return FAIL;

}

int getTrimbit(int imod, int ichip, int ichan) {
  if (detectorChans) {
    if (imod<nModBoard && imod>=0)
      if (ichip<(detectorModules+imod)->nchip && ichan<(detectorModules+imod)->nchan/(detectorModules+imod)->nchip)
	return (detectorChans[imod*NCHAN*NCHIP+ichip*NCHAN+ichan] & TRIM_DR);
  } //else
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
    chanma=NCHAN;
  } else if (sChan==noneSelected || sChan>NCHAN || sChan<0) {
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
    chipma=NCHIP;
  } else if (sChip==noneSelected || sChip>NCHIP || sChip<0) {
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
	  //  printf("im=%d ichi=%d icha=%d tot=%d reg=%x\n",im,ichip, ichan, im*NCHAN*NCHIP+ichip*NCHAN+ichan,detectorChans[im*NCHAN*NCHIP+ichip*NCHAN+ichan]);
#endif
	 detectorChans[im*NCHAN*NCHIP+ichip*NCHAN+ichan]= ft | (cae<<(NTRIMBITS+1)) | (ae<<(NTRIMBITS+2)) | (coe<<(NTRIMBITS+3)) | (ocoe<<(NTRIMBITS+4)) | (counts<<(NTRIMBITS+5));
#ifdef VERBOSE
	 //printf("imod=%d ichip=%d ichan=%d addr=%x reg=%x\n",im,ichip,ichan,detectorChans+im*NCHAN*NCHIP+ichip*NCHAN+ichan, detectorChans[im*NCHAN*NCHIP+ichip*NCHAN+ichan]);
	  // printf("imod=%d ichip=%d ichan=%d addr=%x reg=%x\n",im,ichip,ichan,detectorChans+im*NCHAN*NCHIP+ichip*NCHAN+ichan, detectorChans[im*NCHAN*NCHIP+ichip*NCHAN+ichan]);
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
	myChip->reg=detectorChips[ichip+imod*NCHIP];
	myChip->nchan=NCHAN;
	myChip->chanregs=detectorChans+imod*NCHAN*NCHIP+ichip*NCHIP;
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
    chipma=NCHIP;
  } else if (sChip==noneSelected || sChip>NCHIP || sChip<0) {
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
	detectorChips[im*NCHIP+ichip]=obe | (ow<<1);
#ifdef VERBOSE
	//printf("imod=%d ichip=%d reg=%d (%x)\n",im,ichip,detectorChips[im*NCHIP+ichip],detectorChips+im*NCHIP+ichip);
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
  printf("\n \n \n",regval);
  printf("initChip ow=%d omask=%d probes=%d\n",ow, omask,nprobes);
#endif
  regval|=(omask<<OUTMUX_OFFSET);
  regval|=(nprobes<<PROBES_OFFSET);
  regval|=(obe<<OUTBUF_OFFSET);
#ifdef VERBOSE
  printf("initChip : shift in will be %08x\n",regval);
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
    chipma=NCHIP;
  } else if (sChip==noneSelected || sChip>NCHIP || sChip<0) {
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
	detectorChips[im*NCHIP+ichip]=obe | (ow<<1);
#ifdef VERBOSE
	//printf("imod=%d ichip=%d reg=%d (%x)\n",im,ichip,detectorChips[im*NCHIP+ichip],detectorChips+im*NCHIP+ichip);
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
      printf("imod=%d reg=%d (%x)\n",im,(detectorModules+im)->reg,(detectorModules+im));
#endif
    }
  }
 return 0;
}

int initModulebyNumber(sls_detector_module myMod) { 

 
  int ichip, nchip, ichan, nchan;
  int im, modmi,modma;
  int ft,  cae, ae, coe, ocoe, counts, chanreg;
  int imod;
  int obe;
  int ow;
  int v[NDAC];


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


  /*
  for (idac=0; idac<NDAC; idac++)
    v[idac]=(myMod.dacs)[idac];
  */


  v[VTRIM]=(myMod.dacs)[0];
  v[VTHRESH]=(myMod.dacs)[1];
  v[RGSH1]=(myMod.dacs)[2];
  v[RGSH2]=(myMod.dacs)[3];
  v[VCAL]=(myMod.dacs)[5];
  v[RGPR]=(myMod.dacs)[4];
 
  /*#ifdef VERBOSE
  printf("imod=%d\n",imod);
  printf("Vtrim=%d\n",v[VTRIM]);
  printf("Vthresh=%d\n",v[VTHRESH]);
  printf("Rgsh1=%d\n",v[RGSH1]);
  printf("Rgsh2=%d\n",v[RGSH2]);
  printf("Vcal=%d\n",v[VCAL]);
  printf("Rgpr=%d\n",v[RGPR]);
#endif
  */

  initDACs(v,imod);
  clearCSregister(imod);
  firstChip(imod);
  for (ichip=0; ichip<nchip; ichip++) {


    clearSSregister(imod);
    for (ichan=0; ichan<nchan; ichan++) {

      chanreg=(myMod.chanregs)[ichan+ichip*nchan];
      ft=chanreg&(TRIM_DR);
      cae=(chanreg>>(NTRIMBITS+1))&1;
      ae=(chanreg>>(NTRIMBITS+2))&1;
      coe=((chanreg)>>(NTRIMBITS+3))&1;
      ocoe=((chanreg)>>(NTRIMBITS+4))&1;
      counts=((chanreg)>>(NTRIMBITS+5));
      nextStrip(imod); 
      initChannel(ft,cae,ae, coe, ocoe, counts,imod);
    }    
    obe=((myMod.chipregs)[ichip])&1;
    ow=1;//((myMod.chipregs)[ichip])>>1;
    initChip(obe,ow,imod);
    nextChip(imod);
  }

 
  initMCBregisters(myMod.reg,imod);
  
  if (detectorModules) {
    for (im=modmi; im<modma; im++) {  
#ifdef VERBOSE
      printf("im=%d\n",im);
#endif
      copyModule(detectorModules+im,&myMod);
    }
  } 
  thisSettings=UNDEFINED;
  setSettings(GET_SETTINGS);
  return myMod.reg;
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
      for (j=0; j<NCHIP; j++) {
	if (i==0) 
	  *(val+j+k*NCHIP)=0;
	if (v & (1<<j)) { 
	  *(val+j+k*NCHIP)= *(val+j+k*NCHIP) | (1 << i);
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
  int val;
  putout("0000000000000000", modnum);
  putout("0000000001000000", modnum);
  val=readin(modnum);
  putout("0000000000000000", modnum);
  return val;
}


int testShiftIn(int imod) {
  int val,i,j, k, result=OK;
  setCSregister(imod); 
  printf("testing shift in for module %d\n", imod);
  //for (j=0; j<10; j++) {
  //selChip(j);
  for (i=0; i<34; i++) {
    if (i%2) {
      putout("0100000000000000",imod);
      putout("0100000000000000",imod);  
      putout("0110000000000000",imod);  
      putout("0110000000000000",imod); 
      putout("0100000000000000",imod);
      putout("0100000000000000",imod); 
    } else {
      putout("0000000000000000",imod);
      putout("0000000000000000",imod);  
      putout("0010000000000000",imod);  
      putout("0010000000000000",imod);  
      putout("0000000000000000",imod);
      putout("0000000000000000",imod);
    } 
  }
    putout("0000000000000000",imod);    
    for (i=0; i<34; i++) {
      putout("0000000000000000",imod); 
      
      k=imod;
      //for (k=0; k<nModX; k++) {
      val=readin(k);
      //val=bus_r(MCB_DOUT_REG_OFF+(k<<SHIFTMOD)) & 0x3ff;
	for (j=0; j<NCHIP; j++) {	
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
      putout("0010000000000000",imod); 
      putout("0010000000000000",imod); 
      putout("0000000000000000",imod); 
      
    }
    putout("0000000000000000", imod); 
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

  setCSregister(imod);
  for (i=0; i<24; i++) {
    if (dum & 1<<i) {
      putout("0100010000000000",imod);  
      putout("0110010000000000",imod);  
      putout("0110010000000000",imod);  
      putout("0100010000000000",imod);  
    } else {
      putout("0000010000000000",imod);  
      putout("0010010000000000",imod);  
      putout("0010010000000000",imod);  
      putout("0000010000000000",imod); 
    }
  }
  putout("0000000100000000",imod); 

  for (i=0; i<24; i++) {
    putout("0000010000000000",imod);
    putout("0000010000000000",imod);  


    k=imod;
    //for (k=0; k<nModX; k++) {
    val=readin(k);
    //val=bus_r(MCB_DOUT_REG_OFF+(k<<SHIFTMOD)) & 0x3ff;
      //printf("%8x\n",val);
      for (j=0; j<NCHIP; j++) {	
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
    putout("0010010000000000",imod);  
  }
  putout("0000000000000000", imod);  
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
  setCSregister(imod);  
  for (i=0; i<NCHAN; i++) {
    if (i%2) {
      putout("0100011000000000",imod);  
      putout("0110011000000000",imod);  
      putout("0100011000000000",imod); 
    } else { 
      putout("0000011000000000",imod);  
      putout("0010011000000000",imod);  
      putout("0000011000000000",imod);
    }
  }
  putout("0010011000000000",imod); 
  for (i=0; i<NCHAN; i++) {
    putout("0000011000000000",imod); 


    k=imod;
    //for (k=0; k<nModX; k++) {
    val=readin(k);
    //val=bus_r(MCB_DOUT_REG_OFF+(k<<SHIFTMOD)) & 0x3ff;
	for (j=0; j<NCHIP; j++) {	
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




    putout("0010011000000000",imod);  
  }  
  putout("0000011000000000",imod); 
  printf("Shift stsel module %d : %d errors\n", imod,result); 
    if (result)
      return 1;
    else
      return 0;
}




int testDataInOut(int num, int imod) {
  int val[NCHIP*nModX], result=OK;
  int ich, ichip;
  setCSregister(imod);
  printf("Testing data in out for module %d pattern 0x%x\n", imod, num);
  setSSregister(imod);
  initChannel(0,0,0,0,0,num,imod);
  putout("0000000000000000",imod);
  setCSregister(imod);
  initChip(0, 0,imod);
  clearSSregister(imod);
  for (ich=0; ich<NCHAN; ich++) {
    nextStrip(imod);
    readOutChan(val);
    //imod=0;
    //for (imod=0; imod<nModX; imod++) {
      for (ichip=0; ichip<NCHIP; ichip++) {
	if (val[ichip+imod*NCHIP]!=num) {
	  printf("Test datain out: Channel %d read %x instead of %x\n", (imod*NCHIP+ichip)*NCHAN+ich, val[ichip+NCHIP*imod], num);
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

  setCSregister(imod);
  setSSregister(imod);
  counterClear(imod);  
  putout("0000000000000000",imod);
  putout("0000100000000000",imod);
  putout("0000000000000000",imod);
  for (i=0; i<NCHAN; i++) {
    putout("0000000000000000",imod);
    putout("0000000000001000",imod);
    putout("0000000000000000",imod);
    extPulse(1,imod);
  }
  clearSSregister(imod);
  putout("0000000000000000",imod);

 
  // Readout with SM 

  //startStateMachine();
  startReadOut();
  usleep(100);
  val1=decode_data(fifo_read_event());
  // val1=fifo_read_event();
  //imod=0;
  //for (imod=0; imod<nModX; imod++) {
  for (ichip=0; ichip<NCHIP; ichip++) {
      for (ichan=0; ichan<NCHAN; ichan++) {//
	if ((*(val1+ichan+(ichip+imod*NCHIP)*NCHAN))!=ichan) {
	  result++;
	  printf("Counter test: channel %d read %d instead of %d\n",ichan+(ichip+imod*NCHIP)*NCHAN, val1[ichan+(ichip+imod*NCHIP)*NCHAN], ichan);
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
  //  initChip(0, ow,imod);
  for (ichip=0; ichip<NCHIP; ichip++) {
  setSSregister(imod);
    for (i=0; i<NCHAN; i++) {
      putout("0000000000000000",imod);
      putout("0000000000001000",imod);
      putout("0000000000000000",imod);
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
  v1=fifo_read_event();
  if (v1)
    values=decode_data(v1);
  else {
    printf("no data found in fifos\n");
    return 1;
  }
  for (ichip=0; ichip<NCHIP; ichip++) {
    chipr=0;
      for (ichan=0; ichan<NCHAN; ichan++) { 
	ind=ichan+(ichip+imod*NCHIP)*NCHAN;
	v=values[ind];
	vright=(ichan*(ichip+1))&nbit_mask;
	if (v!=vright) {
	  result++;
	  chipr++;
	  //	  printf("Counter test mux %d mode: channel %d chip %d read %d instead of %d\n",ow, ichan+(ichip+imod*NCHIP)*NCHAN, ichip, v, vright);
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
  initChannel(0,0,0,0,0,num,imod);
  putout("0000000000000000",imod);
  clearSSregister(ALLMOD);
  initChipWithProbes(0, ow,0,ALLMOD);
  clearSSregister(ALLMOD);
  putout("0000000000000000",ALLMOD);

  // Readout with SM 

  printf("mux %d\n",ow);
  startReadOut();
  usleep(100);  
  v1=fifo_read_event();
  if (v1)
    values=decode_data(v1);
  else {
    printf("no data found in fifos\n");
    return 1;
  }
  for (ichip=0; ichip<NCHIP; ichip++) {
    chipr=0;
      for (ichan=0; ichan<NCHAN; ichan++) { 
	ind=ichan+(ichip+imod*NCHIP)*NCHAN;
	v=values[ind];
	if (v!=vright) {
	  result++;
	  chipr++;
	  printf("DataInOut test mux %d mode: channel %d chip %d  read %08x instead of %08x\n",ow, ichan+(ichip+imod*NCHIP)*NCHAN, ichip, v, vright);
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
  setCSregister(imod);
  // Clear outshift reg 
  putout("0000010000000000",imod);
  for (ibit=0; ibit<10;ibit++)
    putout("0000110000000000",imod);
  putout("0000010000000000",imod);

  // Clear inshift reg 
  putout("0000000000000000",imod);
  for (ibit=0; ibit<10;ibit++)
    putout("0000100000000000",imod);
  putout("0000000000000000",imod);

  // input pattern 0f0f0f 

  for (ibit=0; ibit<24;ibit++) {
    if (pat & (1 << ibit)) {
      putout("0100010000000000",imod);
      putout("0110010000000000",imod); //write in the data 1 
      putout("0100010000000000",imod);

    } else {
      putout("0000010000000000",imod);
      putout("0010010000000000",imod); //write in the data 0 
      putout("0000010000000000",imod);
    }
  }  
  
  putout("0100000000000000",imod);
  putout("0110000000000000",imod); //shift in 1 
  putout("0100000000000000",imod);

  for (ibit=0; ibit<20;ibit++) {
    putout("0000000000000000",imod);
    putout("0010000000000000",imod); //shift in 20 bits 
    putout("0000000000000000",imod);
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


   
      putout("0000010001000000",imod);
      putout("0000010001000000",imod); //output is out0
      putout("0000010001000000",imod);

      i=0;
      k=imod;
      //for (k=0; k<nModX; k++) {
      val=readin(k);
      //val=bus_r(MCB_DOUT_REG_OFF+(k<<SHIFTMOD)) & 0x3ff; 
#ifdef DEBUGOUT
      printf("%d %x\n",i*dist,val);     
#endif
      for (j=0; j<NCHIP; j++) {
	v=val & 1<< j;
	if (pat & (1<<(i*dist))) {
	  if (v==0) {
	    // printf("Outmux: module %d chip %i bit %d  read %d instead of %ld\n",k, j,(i*dist), v,pat & (1<<(i*dist) ));
	    result++;
	  }
	  // should be 1
	} else { 
	  if (v) {
	    //  printf("Outmux: module %d chip %i bit %d  read %d instead of %ld\n",k, j,(i*dist), v,pat & (1<<(i*dist) ));
	    result++;
	  }
	}
      }
      //}
      putout("0000010001100000",imod);
      putout("0000010001100000",imod); //output is out1
      putout("0000010001100000",imod);

      i++;
      k=imod;
      //for (k=0; k<nModX; k++) {
      val=readin(k);
      //val=bus_r(MCB_DOUT_REG_OFF+(k<<SHIFTMOD)) & 0x3ff; 
#ifdef DEBUGOUT
      printf("%d %x\n",i*dist, val);
#endif
      for (j=0; j<NCHIP; j++) {
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




      putout("0000010001010000",imod);
      putout("0000010001010000",imod); //output is out2
      putout("0000010001010000",imod);


      i++;
      k=imod;
      //for (k=0; k<nModX; k++) {
      val=readin(k);
      //val=bus_r(MCB_DOUT_REG_OFF+(k<<SHIFTMOD)) & 0x3ff; 
#ifdef DEBUGOUT
      printf("%d %x\n",i*dist, val);
#endif
      for (j=0; j<NCHIP; j++) {
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

      putout("0000010001110000",imod);
      putout("0000010001110000",imod); //output is out3
      putout("0000010001110000",imod);
      i++;
      k=imod;
      //for (k=0; k<nModX; k++) {
      val=readin(k);
      //val=bus_r(MCB_DOUT_REG_OFF+(k<<SHIFTMOD)) & 0x3ff; 
#ifdef DEBUGOUT
      printf("%d %x\n",i*dist, val);
#endif
      for (j=0; j<NCHIP; j++) {
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

      putout("0000000000000000",imod);
      putout("0010000000000000",imod); //change mux setting 
      putout("0000000000000000",imod);
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
  putout("0000010000000000",imod);
  for (ibit=0; ibit<10;ibit++)
    putout("0000110000000000",imod);
  putout("0000010000000000",imod);

  // Clear inshift reg 
  putout("0000000000000000",imod);
  for (ibit=0; ibit<10;ibit++)
    putout("0000100000000000",imod);
  putout("0000000000000000",imod);

  // input pattern 0f0f0f 

  for (ibit=0; ibit<24;ibit++) {
    if (pat & (1 << ibit)) {
      putout("0100010000000000",imod);
      putout("0110010000000000",imod); //write in the data 1 
      putout("0100010000000000",imod);

    } else {
      putout("0000010000000000",imod);
      putout("0010010000000000",imod); //write in the data 0 
      putout("0000010000000000",imod);
    }
  }  
  
  putout("0100000000000000",imod);
  putout("0110000000000000",imod); //shift in 1 
  putout("0100000000000000",imod);

  for (ibit=0; ibit<20;ibit++) {
    putout("0000000000000000",imod);
    putout("0010000000000000",imod); //shift in 20 bits 
    putout("0000000000000000",imod);
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

      putout("0000010000000000",imod) ; 	
      putout("0000010100000000",imod) ; // reset readout		     
      putout("0000010100000000",imod) ; 		     
      putout("0000010000000000",imod) ; 


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
	  for (j=0; j<NCHIP; j++) {
	    v=val & 1<< j;
	    if (pat & (1<<(i*dist))) {
	      if (v==0) {
		//	printf("Fpgamux: module %d chip %i bit %d  read %d instead of %ld\n",k,j,(i*dist), v,pat & (1<<(i*dist) ));
		result++;
	      }
	  // should be 1
	    } else { 
	      if (v) {
		//	printf("Fpgamux: module %d chip %i bit %d  read %d instead of %ld\n",k,j,(i*dist), v,pat & (1<<(i*dist) ));
		result++;
	      }
	    }
	  }
	  //}
	putout("0000010000000000",imod);
	putout("0010010000000000",imod); //output
	putout("0000010000000000",imod );

      }
      pat = pat >> 1;
     

      putout("0000000000000000",imod);
      putout("0010000000000000",imod); //change mux setting 
      putout("0000000000000000",imod);
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
    for (ichip=0; ichip<NCHIP; ichip++) { 
      selChip(ichip,imod);
      for (ich=0; ich<128; ich++){ 
	selChannel(ich,imod);
	initChannel(*(dacs+ichip*NCHAN+ich),0,0,0,0,0,imod); //disable channel; 
	clearCounter(imod);      
	//initChannel(*(dacs+ichip*NCHAN+ich),FALSE,FALSE,TRUE,FALSE,0); //disable channel;  
      }
    }
  }
 
  for (imod=0; imod<nModX; imod++) {
    //selMod(imod);
    initMCBregisters(1,imod);
    for (ich=0; ich<NCHAN; ich++){  
      for (ichip=0; ichip<NCHIP; ichip++) {
	selChip(ichip,imod); // select channel 
	selChannel(ich,imod); // select channel 
	initChannel(*(dacs+imod*NCHAN*NCHIP+ichip*NCHAN+ich),0,0,1,0,0,imod); // enable channel
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
      for (ichip=0; ichip<NCHIP; ichip++) { 
	*(v+(ichip+imod*NCHIP)*NCHAN+ich)=val[ichip+imod*NCHIP];
	selChip(ichip,imod); // select channel 
	selChannel(ich,imod); // select channel 
	initChannel(*(dacs+ichip*NCHAN+ich),0,0,0,0,0,imod); //disable channel;  
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
    for (ichip=0; ichip<NCHIP; ichip++) { 
      selChip(ichip,imod);
      for (ich=0; ich<128; ich++){ 
	selChannel(ich,imod);
	clearCounter(imod); 
	initChannel(*(dacs+ichip*NCHAN+ich),0,0,0,0,0,imod); //disable channel;  
      }
    }
  }
  for (ich=0; ich<NCHAN; ich++){  
    for (imod=0; imod<nModX; imod++) {
      //selMod(imod);
      for (ichip=0; ichip<NCHIP; ichip++) {
	selChip(ichip,imod); // select channel 
	selChannel(ich,imod); // select channel 
	initChannel(*(dacs+imod*NCHAN*NCHIP+ichip*NCHAN+ich),1,0,1,0,0,imod); // enable channel
	clearCounter(imod);      
      }
    }
    //setMSregister();
    setCSregister(imod); 
    clearSSregister(imod);
    countEnable(imod);
    usleep(20);
    calPulse(num,imod); // give pulses//	  
    usleep(20);  
    selChannel(ich,imod); // select channel 
    readOutChan(val); // readout channel
    for (imod=0; imod<nModX; imod++) {
      //selMod(imod);
      for (ichip=0; ichip<NCHIP; ichip++) {
	*(v+(ichip+imod*NCHIP)*NCHAN+ich)=val[ichip+imod*NCHIP]; 
	selChip(ichip,imod); // select chip 
	selChannel(ich,imod); // select channel 
	initChannel(*(dacs+imod*NCHAN*NCHIP+ichip*NCHAN+ich),0,0,0,0,0,imod); //disable channel;
      }
    }
  }
  printf("done\n");
  return OK;  
}

#endif
