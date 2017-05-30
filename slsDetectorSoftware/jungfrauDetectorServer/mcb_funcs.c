#ifdef MCB_FUNCS

#include "registers_m.h"
#include "server_defs.h"
#include "firmware_funcs.h"
#include "mcb_funcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

/* global variables */



enum detectorSettings thisSettings;

int sChan, sChip, sMod, sDac, sAdc;
const int allSelected=-2;
const int noneSelected=-1;

sls_detector_module *detectorModules=NULL;
int *detectorChips=NULL;
int *detectorChans=NULL;
int *detectorDacs=NULL;
int *detectorAdcs=NULL;



int initDetector() {
  int imod;
  int n=getNModBoard();
#ifdef VERBOSE
  printf("Board is for %d modules\n",n);
#endif
  detectorModules=malloc(n*sizeof(sls_detector_module));
  detectorDacs=malloc(n*NDAC*sizeof(int));
  detectorAdcs=malloc(n*NADC*sizeof(int));
  detectorChips=NULL;
  detectorChans=NULL;
  detectorAdcs=NULL;
#ifdef VERBOSE
  printf("modules from 0x%x to 0x%x\n",(unsigned int)(detectorModules), (unsigned int)(detectorModules+n));
  printf("dacs from 0x%x to 0x%x\n",(unsigned int)(detectorDacs), (unsigned int)(detectorDacs+n*NDAC));
  printf("adcs from 0x%x to 0x%x\n",(unsigned int)(detectorAdcs), (unsigned int)(detectorAdcs+n*NADC));
#endif


  for (imod=0; imod<n; imod++) {
    (detectorModules+imod)->dacs=detectorDacs+imod*NDAC;
	(detectorModules+imod)->adcs=detectorAdcs+imod*NADC;
    (detectorModules+imod)->ndac=NDAC;
    (detectorModules+imod)->nadc=NADC;
    (detectorModules+imod)->nchip=NCHIP;
    (detectorModules+imod)->nchan=NCHIP*NCHAN;
    (detectorModules+imod)->module=imod;
    (detectorModules+imod)->gain=0;
    (detectorModules+imod)->offset=0;
    (detectorModules+imod)->reg=0;
  }
  thisSettings=UNINITIALIZED;
  sChan=noneSelected;
  sChip=noneSelected;
  sMod=noneSelected;
  sDac=noneSelected;
  sAdc=noneSelected;

  return OK;
}





int copyModule(sls_detector_module *destMod, sls_detector_module *srcMod) {

  int idac, iadc;

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

  /*
  if ((srcMod->nchip)>(destMod->nchip)) {
    printf("Number of chip of source is larger than number of chips of destination\n");
    return FAIL;
  }
  if ((srcMod->nchan)>(destMod->nchan)) {
    printf("Number of channels of source is larger than number of channels of destination\n");
    return FAIL;
  }
  */

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
  //printf("Chips: src %d, dest %d\n",srcMod->nchip,destMod->nchip);
  //printf("Chans: src %d, dest %d\n",srcMod->nchan,destMod->nchan);

#endif



  destMod->ndac=srcMod->ndac;
  destMod->nadc=srcMod->nadc;
 // destMod->nchip=srcMod->nchip;
  //destMod->nchan=srcMod->nchan;
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







int setSettings(int i, int imod) {
//#ifdef VERBOSE
	if(i!=-1)
		printf("\nSetting settings wit value %d\n",i);
//#endif
	int isett=-1,val=-1,retval=-1;
	enum conf_gain {
		dynamic = 0x0f00,	//dynamic
		dynamichighgain0 =	v,	//dynamichighgain0
		fixgain1 = 0x0f02,	//fixgain1
		fixgain2 = 0x0f06,	//fixgain2
		forceswitchgain1 = 0x1f00,	//forceswitchgain1
		forceswitchgain2 = 0x3f00	//forceswitchgain2
	};

	//determine conf value to write
	if(i!=GET_SETTINGS){
		switch(i){
		case DYNAMICGAIN: 	val = dynamic;			break;
		case DYNAMICHG0: 	val = dynamichighgain0;	break;
		case FIXGAIN1: 		val = fixgain1;			break;
		case FIXGAIN2: 		val = fixgain2;			break;
		case FORCESWITCHG1: val = forceswitchgain1;	break;
		case FORCESWITCHG2: val = forceswitchgain2;	break;
		default:
			printf("Error: This settings is not defined for this detector %d\n",i);
			return GET_SETTINGS;
		}
	}

	retval = initConfGain(i,val,imod);

	switch(retval){
	case dynamic: 			isett=DYNAMICGAIN;		break;
	case dynamichighgain0: 	isett=DYNAMICHG0;		break;
	case fixgain1: 			isett=FIXGAIN1;			break;
	case fixgain2: 			isett=FIXGAIN2;			break;
	case forceswitchgain1: 	isett=FORCESWITCHG1;	break;
	case forceswitchgain2: 	isett=FORCESWITCHG2;	break;
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



int initModulebyNumber(sls_detector_module myMod) { 
 printf("\nInitializing Module\n");
  int nchip,nchan;//int ichip, nchip, ichan, nchan;
  int im, modmi,modma;
 // int ft,  cae, ae, coe, ocoe, counts, chanreg;
  int imod;
 // int obe;
 // int ow;
 /* int v[NDAC];*/
  int retval =-1, idac;


  nchip=myMod.nchip;
  nchan=(myMod.nchan)/nchip;
  imod=myMod.module;
  sMod=imod;
  if (sMod==ALLMOD)
    sMod=allSelected;
  if (sMod==allSelected) {
    modmi=0;
    modma=NMODX;//getNModBoard();
  } else if (sMod==noneSelected || sMod>NMODX || sMod<0) {// (sMod==noneSelected || sMod>getNModBoard() || sMod<0) {
    modmi=0;
    modma=-1;
  } else {
    modmi=sMod;
    modma=sMod+1;
  }

  //printf("ndac:%d\n",NDAC);

  for (idac=0; idac<NDAC; idac++){
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

  printf("Done Initializing Module\n");
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







// Fifo continuous read 


int getModuleNumber(int modnum) {
  int val = 0xfff;
 // val=readin(modnum);
  return val;
}










#endif


