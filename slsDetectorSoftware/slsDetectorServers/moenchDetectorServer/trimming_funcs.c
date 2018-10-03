#ifndef PICASSOD
#include "server_defs.h"
#else
#include "picasso_defs.h"
#endif
#include "trimming_funcs.h"
#include "mcb_funcs.h"
#include "firmware_funcs.h"
#include <math.h>



extern int nModX;
//extern int *values;

extern const int nChans;
extern const int nChips;
extern const int nDacs;
extern const int nAdcs;


int trim_fixed_settings(int countlim, int par2, int im)
{

  int retval=OK;
#ifdef VERBOSE
  printf("Trimming with fixed settings\n");
#endif
#ifdef VIRTUAL
  return OK;
#endif

  if (par2<=0)
    retval=trim_with_level(countlim, im);
  else
    retval=trim_with_median(countlim,im);


  return retval;
}


int trim_with_noise(int countlim, int nsigma, int im)
{


  int retval=OK, retval1=OK, retval2=OK;
#ifdef VERBOSE
  printf("Trimming using noise\n");
#endif
#ifdef VIRTUAL
  return OK;
#endif

  /* threshold scan */

#ifdef VERBOSE
  printf("chosing vthresh and vtrim.....");
#endif
  retval1=choose_vthresh_and_vtrim(countlim,nsigma, im);
 
#ifdef VERBOSE
  printf("trimming with noise.....\n");
#endif
  retval2=trim_with_level(countlim, im);
  
#ifdef DEBUGOUT
  printf("done\n");
#endif
  if (retval1==OK && retval2==OK)
    retval=OK;
  else
    retval=FAIL;

  return retval;

}

int trim_with_beam(int countlim, int nsigma, int im) //rpc
{


  int retval=OK, retval1=OK, retval2=OK;

  printf("Trimming using beam\n");
  //return OK;
#ifdef VIRTUAL
  printf("Trimming using beam\n");
  return OK;
#endif
  /* threshold scan */
#ifdef DEBUGOUT
  printf("chosing vthresh and vtrim.....");
#endif
  
  retval1=choose_vthresh_and_vtrim(countlim,nsigma,im);
  retval2=trim_with_median(TRIM_DR, im);

#ifdef DEBUGOUT
    printf("done\n");
#endif

  if (retval1==OK && retval2==OK)
    retval=OK;
  else
    retval=FAIL;

  return retval;

}


int  trim_improve(int maxit, int par2, int im) //rpc
{

  int retval=OK, retval1=OK, retval2=OK;


#ifdef VERBOSE
  printf("Improve the trimming\n");
#endif
#ifdef VIRTUAL
  return OK;
#endif
 

  if (par2!=0 && im==ALLMOD)
    retval1=choose_vthresh();

  retval2=trim_with_median(2*maxit+1, im);
#ifdef DEBUGOUT
    printf("done\n");
#endif
  if (retval1==OK && retval2==OK)
    retval=OK;
  else
    retval=FAIL;

  return retval;

}

int calcthr_from_vcal(int vcal) {
  int thrmin;
  //thrmin=140+3*vcal/5;
  thrmin=180+3*vcal/5;
  return thrmin;
}

int calccal_from_vthr(int vthr) {
  int vcal;
  vcal=5*(vthr-140)/3;
  return vcal;
}

int choose_vthresh_and_vtrim(int countlim,  int nsigma, int im) {
  int retval=OK;
#ifdef MCB_FUNCS
  int modma, modmi, nm;
  int thr, thrstep=5, nthr=31;

  int *fifodata;

  double vthreshmean, vthreshSTDev;
  int *thrmi, *thrma;
  double c;
  double b=BVTRIM;
  double a=AVTRIM;
  int *trim;
  int ich, imod, ichan;
  int nvalid=0;
  u_int32_t *scan;
  int ithr;
  sls_detector_channel myChan;



  setFrames(1);
  // setNMod(getNModBoard());

  if (im==ALLMOD){
    modmi=0;
    modma=nModX;
  } else {
    modmi=im;
    modma=im+1;
  } 
  nm=modma-modmi;
  
  trim=malloc(sizeof(int)*nChans*nChips*nModX);
  thrmi=malloc(sizeof(int)*nModX);
  thrma=malloc(sizeof(int)*nModX);


  for (ich=0; ich<nChans*nChips*nm; ich++)
    trim[ich]=-1;
  /*
  setCSregister(im);
  setSSregister(im);
  initChannel(0,0,0,1,0,0,im);
  counterClear(im);
  clearSSregister(im); 
  usleep(500); 
  */
  myChan.chan=-1;
  myChan.chip=-1;
  myChan.module=ALLMOD;
  myChan.reg=COMPARATOR_ENABLE;
  initChannelbyNumber(myChan);


  for (ithr=0; ithr<nthr; ithr++) {
    fifoReset();
    /* scanning threshold */
    for (imod=modmi; imod<modma; imod++) {
      //commented out by dhanya      thr=getDACbyIndexDACU(VTHRESH,imod);
      if (ithr==0) {
	thrmi[imod]=thr;
//commented out by dhanya  	initDACbyIndexDACU(VTHRESH,thr,imod);
      } else
	;//commented out by dhanya  	initDACbyIndexDACU(VTHRESH,thr+thrstep,imod);
    } 

    /*    setCSregister(ALLMOD);
    setSSregister(ALLMOD);
    initChannel(0,0,0,1,0,0,im);
    setDynamicRange(32);
    */



    counterClear(ALLMOD);
    clearSSregister(ALLMOD); 
    usleep(500); 
    startStateMachine();
    while (runBusy()) {
    }
    usleep(500);
    fifodata=fifo_read_event();
    scan=decode_data(fifodata);
    for (imod=modmi; imod<modma; imod++) {
      for (ichan=0; ichan<nChans*nChips; ichan++){
	ich=imod*nChips*nChans+ichan;
	if (scan[ich]>countlim && trim[ich]==-1) {
//commented out by dhanya  	  trim[ich]=getDACbyIndexDACU(VTHRESH,imod);
#ifdef VERBOSE
	  //	  printf("yes: %d %d %d\n",ich,ithr,scan[ich]);
#endif
	} 
#ifdef VERBOSE
	/*	else {
	  printf("no: %d %d %d\n",ich,ithr,scan[ich]);
	  }*/
#endif
      }
    }
    free(scan);
  }
  
  for (imod=modmi; imod<modma; imod++) {
    vthreshmean=0;
    vthreshSTDev=0;
    nvalid=0;
//commented out by dhanya      thrma[imod]=getDACbyIndexDACU(VTHRESH,imod);
    
    for (ichan=0; ichan<nChans*nChips; ichan++){
      ich=imod*nChans*nChips+ichan;
      if(trim[ich]>thrmi[imod] && trim[ich]<thrma[imod]) {
	vthreshmean=vthreshmean+trim[ich];
	vthreshSTDev=vthreshSTDev+trim[ich]*trim[ich];
	nvalid++;
      }	
    }
      
    if (nvalid>0) {
      vthreshmean=vthreshmean/nvalid;
  //commented out by dhanya      vthreshSTDev=sqrt((vthreshSTDev/nvalid)-vthreshmean*vthreshmean);
    } else {
      vthreshmean=thrmi[imod];
      vthreshSTDev=nthr*thrstep;
      printf("No valid channel for module %d\n",imod);
      retval=FAIL;
    }

#ifdef DEBUGOUT
    printf("module= %d nvalid = %d mean=%f RMS=%f\n",imod, nvalid, vthreshmean,vthreshSTDev);
#endif
    // *vthresh=round(vthreshmean-nsigma*vthreshSTDev);
    thr=(int)(vthreshmean-nsigma*vthreshSTDev);
    if (thr<0 || thr>(DAC_DR-1)) {
      thr=thrmi[imod]/2;
      printf("Can't find correct threshold for module %d\n",imod);
      retval=FAIL;
    }
//commented out by dhanya      initDACbyIndexDACU(VTHRESH,thr,imod);
#ifdef VERBOSE
    printf("vthresh=%d  \n",thr);
#endif 
    c=CVTRIM-2.*nsigma*vthreshSTDev/63.;
 //commented out by dhanya     thr=(int)((-b-sqrt(b*b-4*a*c))/(2*a));
    if (thr<500 || thr>(DAC_DR-1)) {
      thr=750;
      printf("Can't find correct trimbit size for module %d\n",imod);
      retval=FAIL;
    }
      
    //commented out by dhanya    initDACbyIndexDACU(VTRIM,thr,imod);
    
#ifdef VERBOSE
    printf("vtrim=%d  \n",thr);
#endif 

  }
  free(trim);
  free(thrmi);
  free(thrma);

#endif
  return retval;
}





int trim_with_level(int countlim, int im) {
  int ich, itrim, ichan, ichip, imod;
  u_int32_t *scan;
  int *inttrim;
  int modma, modmi, nm;
  int retval=OK;
  int *fifodata;
  sls_detector_channel myChan;
  printf("trimming module number %d", im);


#ifdef MCB_FUNCS
  setFrames(1);
  // setNMod(getNModBoard());

  if (im==ALLMOD){
    modmi=0;
    modma=nModX;
  } else {
    modmi=im;
    modma=im+1;
  } 
  nm=modma-modmi;
  
  inttrim=malloc(sizeof(int)*nChips*nChans*nModX);
  printf("countlim=%d\n",countlim);
  for (ich=0; ich<nChans*nChips*nModX; ich++) 
    inttrim[ich]=-1;

  for (itrim=0; itrim<TRIM_DR+1; itrim++) { 
    fifoReset(); 
    printf("Trimbit %d\n",itrim);  
    myChan.chan=-1;
    myChan.chip=-1;
    myChan.module=ALLMOD;
    myChan.reg=COMPARATOR_ENABLE|(itrim<<TRIMBIT_OFF);
    initChannelbyNumber(myChan);

    /*
      setCSregister(im); 
      setSSregister(im);
      initChannel(itrim,0,0,1,0,0,ALLMOD);
      setDynamicRange(32);
    */
    setCSregister(ALLMOD); 
    setSSregister(ALLMOD);
    counterClear(ALLMOD);
    clearSSregister(ALLMOD); 
    usleep(500);
    startStateMachine();
    while (runBusy()) {
    }
    usleep(500);

    fifodata=fifo_read_event();
    scan=decode_data(fifodata);
    for (imod=modmi; imod<modma; imod++) {
      for (ichan=0; ichan<nChans*nChips; ichan++) {
	ich=ichan+imod*nChans*nChips;
	if (inttrim[ich]==-1) {
	  if (scan[ich]>countlim){
	    inttrim[ich]=itrim;
	    if (scan[ich]>2*countlim && itrim>0) {
	    //if (scan[ich]>2*countlim || itrim==0) {
	      inttrim[ich]=itrim-1;
	    }
#ifdef VERBOSE
	      printf("Channel %d trimbit %d counted %d (%08x) countlim %d\n",ich,itrim,scan[ich],fifodata[ich],countlim);
#endif
	  } 
	}
#ifdef VERBOSE
	/*	  else  
	  printf("Channel %d trimbit %d counted %d countlim %d\n",ich,itrim,scan[ich],countlim);*/
#endif
      }
    }
    free(scan);
  }

  for (imod=modmi; imod<modma; imod++) {
    clearCSregister(imod); 
    firstChip(im); 
    for (ichip=0; ichip<nChips; ichip++) {
      clearSSregister(imod); 
      for (ichan=0; ichan<nChans; ichan++) {
	nextStrip(imod);
	ich=ichan+imod*nChans*nChips+ichip*nChans;
	if (*(inttrim+ich)==-1) {
	  *(inttrim+ich)=TRIM_DR;
	  //	  printf("could not trim channel %d chip %d module %d - set to %d\n", ichan, ichip, imod, *(inttrim+ich) );
	  retval=FAIL;
	}
#ifdef VERBOSE 
	//	else
	//  printf("channel %d trimbit %d\n",ich,*(inttrim+ich) );
#endif
	initChannel(inttrim[ich],0,0,1,0,0,imod);
      }
      nextChip(imod); 
    }
  }
  free(inttrim);
  
#endif
  return retval;
}


#define ELEM_SWAP(a,b) { register int t=(a);(a)=(b);(b)=t; }
#define median(a,n) kth_smallest(a,n,(((n)&1)?((n)/2):(((n)/2)-1)))


int kth_smallest(int *a, int n, int k)
{
    register int i,j,l,m ;
    register double x ;

    l=0 ; m=n-1 ;
    while (l<m) {
        x=a[k] ;
        i=l ;
        j=m ;
        do {
            while (a[i]<x) i++ ;
            while (x<a[j]) j-- ;
            if (i<=j) {
                ELEM_SWAP(a[i],a[j]) ;
                i++ ; j-- ;
            }
        } while (i<=j) ;
        if (j<k) l=i ;
        if (k<i) m=j ;
    }
    return a[k] ;
}



int ave(int *a, int n)
{
  int av=0,i;
  for (i=0; i<n; i++)
    av=av+((double)*(a+i))/((double)n);
  return av;
}


int choose_vthresh() {

  int retval=OK;
#ifdef MCB_FUNCS
  int imod,  ichan;
  u_int32_t  *scan, *scan1;
  int olddiff[nModX], direction[nModX];
  int med[nModX], med1[nModX], diff, media;
  int change_flag=1;
  int iteration=0;
  int maxiterations=10;
  int vthreshmean=0;
  int vthresh;
  int im=ALLMOD;
  int modma, modmi, nm;
  int *fifodata;

  setFrames(1);
  // setNMod(getNModBoard());

  if (im==ALLMOD){
    modmi=0;
    modma=nModX;
  } else {
    modmi=im;
    modma=im+1;
  } 
  nm=modma-modmi;
  


    setDynamicRange(32);

  setCSregister(ALLMOD);
  setSSregister(ALLMOD);
  counterClear(ALLMOD);
  clearSSregister(ALLMOD); 
  usleep(500); 
  startStateMachine();
  while (runBusy()) {
    //printf(".");
  }
  usleep(500);
  
    fifodata=fifo_read_event();
    scan=decode_data(fifodata);
    //
    scan1=decode_data(fifodata);


  for (imod=modmi; imod<modma; imod++) {
    //
    med[imod]=median(scan1+imod*nChans*nChips,nChans*nChips);
    med1[imod]=med[imod];
    //commented out by dhanya    vthreshmean=vthreshmean+getDACbyIndexDACU(VTHRESH,imod);
    olddiff[imod]=0xffffff;
    direction[imod]=0;
    printf("Median of module %d=%d\n",imod,med[imod]);
  }
  vthreshmean=vthreshmean/nm;
  //media=median(scan,nChans*nChips*nModX);
  //printf("Median overall=%d\n",media);
  media=median(med1+modmi,nm);
  printf("Median of modules=%d\n",media);
  free(scan);
  free(scan1);


  while(change_flag && iteration<maxiterations) {

    setDynamicRange(32);
    fifoReset(); 
    setCSregister(ALLMOD);
    setSSregister(ALLMOD);
    counterClear(ALLMOD);
    clearSSregister(ALLMOD); 
    usleep(500); 
    startStateMachine();
    while (runBusy()) {
    }
    usleep(500);

    fifodata=fifo_read_event();
    scan=decode_data(fifodata);
    //
    scan1=decode_data(fifodata);

    change_flag=0;
    printf("Vthresh iteration %3d 0f %3d\n",iteration, maxiterations);
    for (ichan=modmi; ichan<modma; ichan++) {
      med[ichan]=median(scan1+ichan*nChans*nChips,nChans*nChips);
      med1[imod]=med[imod];
      media=median(med1+modmi,nm);

      diff=med[ichan]-media;
      if (direction[ichan]==0) {
	if (diff>0) 
	  direction[ichan]=1;
	else
	  direction[ichan]=-1;
      }
   //commented out by dhanya     vthresh=getDACbyIndexDACU(VTHRESH,imod);
      if ( direction[ichan]!=-3) { 
	if (abs(diff)>abs(olddiff[ichan])) {
	  vthresh=vthresh-direction[ichan];
	  if (vthresh>(DAC_DR-1)) {
	    vthresh=(DAC_DR-1);
	    printf("can't equalize threshold for module %d\n", ichan);
	    retval=FAIL;
	  }
	  if (vthresh<0) {
	    vthresh=0;
	    printf("can't equalize threshold for module %d\n", ichan);
	    retval=FAIL;
	  }
	  direction[ichan]=-3;
	} else {
	  vthresh=vthresh+direction[ichan];
	  olddiff[ichan]=diff;
	  change_flag=1;
	}
//commented out by dhanya  	initDACbyIndex(VTHRESH,vthresh, ichan);
      }
    }
    iteration++;
    free(scan);
    free(scan1);
  }
#endif
  return retval;
}





int trim_with_median(int stop, int im) {
 
 
  int retval=OK;

#ifdef MCB_FUNCS
  int  ichan, imod, ichip, ich;
  u_int32_t  *scan, *scan1;
  int *olddiff, *direction;
  int med, diff;
  int change_flag=1;
  int iteration=0;
  int me[nModX], me1[nModX];
  int modma, modmi, nm;
  int trim;
  int *fifodata;

  setFrames(1);
  // setNMod(getNModBoard());

  if (im==ALLMOD){
    modmi=0;
    modma=nModX;
  } else {
    modmi=im;
    modma=im+1;
  } 
  nm=modma-modmi;

  olddiff=malloc(4*nModX*nChips*nChans);
  direction=malloc(4*nModX*nChips*nChans);
  for (imod=modmi; imod<modma; imod++) {  
    for (ichip=0; ichip<nChips; ichip++) {
      for (ich=0; ich<nChans; ich++) {
	ichan=imod*nChips*nChans+ichip*nChans+ich;
	direction[ichan]=0;
	olddiff[ichan]=0x0fffffff;
      }
    }
  }
  /********
  fifoReset(); 
  setCSregister(ALLMOD);
  setSSregister(ALLMOD);
  counterClear(ALLMOD);
  clearSSregister(ALLMOD); 
  usleep(500); 
  startStateMachine();
  while (runBusy()) {
  }
  usleep(500);
  scan=decode_data(fifo_read_event());
  for (imod=modmi; imod<modma; imod++) {
    me[imod]=median(scan+imod*nChans*nChips,nChans*nChips);
    printf("Median of module %d=%d\n",imod,me[imod]);
  }
  med=median(me,nm);
  printf("median is %d\n",med);
  free(scan);
  **************/
  while(change_flag && iteration<stop) {

    setDynamicRange(32);
    fifoReset(); 
    setCSregister(ALLMOD);
    setSSregister(ALLMOD);
    counterClear(ALLMOD);
    clearSSregister(ALLMOD); 
    usleep(500); 
    startStateMachine();
    while (runBusy()) {
    }
    usleep(500);
    fifodata=fifo_read_event();
    scan=decode_data(fifodata);
    scan1=decode_data(fifodata);



    /********* calculates median every time ***********/

    for (imod=modmi; imod<modma; imod++) {
      me[imod]=median(scan1+imod*nChans*nChips,nChans*nChips);
      me1[imod]=me[imod];
      printf("Median of module %d=%d\n",imod,me[imod]);
    }
    med=median(me1,nm);
    printf("median is %d\n",med);

    change_flag=0;
    printf("Trimbits iteration %d of %d\n",iteration, stop);
    for (imod=modmi; imod<modma; imod++) {
      for (ichip=0; ichip<nChips; ichip++) {
	selChip(ichip,imod);
	clearSSregister(imod);
	for (ich=0; ich<nChans; ich++) {
	  ichan=imod*nChips*nChans+ichip*nChans+ich;
	  nextStrip(imod);
	  diff=scan[ichan]-me[imod];
	  if (direction[ichan]==0) {
	    if (diff>0) {
	      direction[ichan]=1;
	    } else {
	      direction[ichan]=-1;
	    }
	  } 
	  if ( direction[ichan]!=-3) { 
	    if (abs(diff)>abs(olddiff[ichan])) {
	      trim=getTrimbit(imod,ichip,ich)+direction[ichan];
	      printf("%d old diff %d < new diff %d   %d - trimbit %d\n",ichan, olddiff[ichan], diff, direction[ichan], trim);
	      direction[ichan]=-3;
	    } else {
	      trim=getTrimbit(imod,ichip,ich)-direction[ichan];
	      olddiff[ichan]=diff;
	      change_flag=1;
	    }
	    if (trim>TRIM_DR) {
	      trim=63;
	      printf("can't trim channel %d chip %d module %d to trim %d\n",ich, ichip, imod, trim);
	      retval=FAIL;
	    }
	    if (trim<0) {
	      printf("can't trim channel %d chip %d module %d to trim %d\n",ich, ichip, imod, trim);
	      trim=0;
	      retval=FAIL;
	    }
	    initChannel(trim,0,0,1,0,0,imod);
	  }
	}
      }
    }
    iteration++;
    free(scan);
    free(scan1);
  }
  free(olddiff);
  free(direction);
#endif
  return retval;
}
