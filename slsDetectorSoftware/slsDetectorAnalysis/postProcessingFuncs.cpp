#include "postProcessingFuncs.h"
#include "angleConversionConstant.h"


postProcessingFuncs::postProcessingFuncs(int *nModules,int *chPerMod,int modMask[],int badCh[], double ffcoeff[], double fferr[], double* t, int *dir, double angRadius[], double angOffset[], double angCentre[], double* to, double* bs, double *sX, double *sY):
  nMods(0), chansPerMod(NULL), moduleMask(NULL), badChannelMask(NULL), ffCoeff(NULL), ffErr(NULL), tDead(0), angDir(1), angConv(NULL), totalOffset(0), binSize(0), sampleX(0), sampleY(0), totalChans(0), nBins(0), mp(NULL), mv(NULL), me(NULL), mm(NULL)
{
  initDataset(nModules, chPerMod,modMask,badCh, ffcoeff, fferr, t, dir, angRadius, angOffset, angCentre, to, bs, sX, sY);

}

int postProcessingFuncs::initDataset() {

  if (nBins) {
    mp=new double[nBins];
    mv=new double[nBins];
    me=new double[nBins];
    mm=new int[nBins];
    resetMerging(mp,mv,me,mm, nBins);
  } else {
    mv=new double[totalChans];
    me=new double[totalChans];
  }
  totalI0=0;

  return 0;

}

int postProcessingFuncs::finalizeDataset(double *ang, double *val, double *err, int *np) {
  
  if (nBins) 
    *np=finalizeMerging(mp,mv,me,mm,nBins);
  else
    *np=totalChans;



  if (totalI0<=0)
    totalI0=1.;


  for (int ip=0; ip<(*np); ip++) {

    if (ang) 
      if (mp)
	ang[ip]=mp[ip];

    if (mv)
      val[ip]=mv[ip]*totalI0;

    if (me)
      err[ip]=me[ip]*totalI0;

  }



  if (mp)
    delete [] mp;
  if (mv)
    delete [] mv;
  if (me)
    delete [] me;
  if (mm)
    delete [] mm;


  return 0;

}

int postProcessingFuncs::addFrame(double *data, double *pos, double *I0, double *expTime, const char *filename, double *var) {


  double p1, vin, ein, vout, eout;
  double  e=0.;
  double i0=*I0;
  int imod=0, ch0=0;

  int chlast=chansPerMod[0]-1;
  int nchmod=chansPerMod[0];

  if (i0>0)
    totalI0+=i0;

  for (int ich=0; ich<totalChans; ich++) {
    

    if (ich>chlast) {
      imod++;
      ch0=chlast+1;
      nchmod=chansPerMod[imod];
      chlast=ch0+nchmod-1;
    }
  
    vin=data[ich];
    ein=0;
    vout=data[ich];
    if (vout>=0)
      eout=sqrt(vout);
    else
      eout=0;


    if (tDead) {
#ifdef VERBOSE
    cout << "ppFuncs ratecorrect" << endl;
#endif
      rateCorrect(vin, ein, vout, eout, tDead, *expTime);
      vin=vout;
      ein=eout;
    }
      //ffcorrect
    
    if (ffCoeff) {
#ifdef VERBOSE
      cout << "ppFuncs ffcorrect" << endl;
#endif
      if (ffErr)
	e=ffErr[ich];
      else
	e=0;
      flatFieldCorrect(vin, ein, vout, eout, ffCoeff[ich], e);
    }


    //i0correct
    if (i0>0) {
#ifdef VERBOSE
      cout << "ppFuncs i0 norm" << endl;
#endif
      vout/=i0;
      eout/=i0;
    }
      
    if (badChannelMask) {
#ifdef VERBOSE
      cout << "ppFuncs badchans" << endl;
#endif
      if (badChannelMask[ich])
	continue;
    }
    if (nBins) {
      //angconv
      
#ifdef VERBOSE
      cout << "ppFuncs angconv" << endl;
#endif
      //check module mask?!?!?!?
      

      p1=convertAngle(*pos,ich-ch0,angConv[imod],moduleMask[imod],totalOffset,0,angDir);

#ifdef VERBOSE
      cout << "ppFuncs merge" << endl;
#endif
      addPointToMerging(p1,vout,eout,mp,mv,me,mm, binSize, nBins);

   
    } else {
#ifdef VERBOSE
      cout << "ppFuncs merge" << endl;
#endif
      //mp[ich]=ich;
      mv[ich]+=vout;
      me[ich]+=eout*eout;
    }
  }
  
}





int postProcessingFuncs::initDataset(int *nModules,int *chPerMod,int modMask[],int badCh[], double ffcoeff[], double fferr[], double* t, int *dir, double angRadius[], double angOffset[], double angCenter[], double* to, double* bs, double *sX, double *sY) {

#ifdef VERBOSE
  cout << "delete pointers " << endl;
#endif

  deletePointers();
 

#ifdef VERBOSE
  cout << "nmod " << endl;
#endif

  if (nModules)
    nMods=*nModules;
  else
    nMods=0;
#ifdef VERBOSE
  cout << "tdead " << endl;
#endif

  if (t)
    tDead=*t;
  else
    t=0;

#ifdef VERBOSE
  cout << "toffset " << endl;
#endif

  if (to)
    totalOffset=*to;
  else
    to=0;


#ifdef VERBOSE
  cout << "binsize " << endl;
#endif
  if (bs)
    binSize=*bs;
  else
    binSize=0;

#ifdef VERBOSE
  cout << "samplex " << endl;
#endif
  if (sX)
    sampleX=*sX;
  else
    sampleX=0;

#ifdef VERBOSE
  cout << "sampley " << endl;
#endif
  if (sY)
    sampleY=*sY;
  else 
    sampleY=0;
  
#ifdef VERBOSE
  cout << "angdir " << endl;
#endif
  if (dir)
    angDir=*dir;
  else
    angDir=1;

  totalChans=0;


 
  chansPerMod=new int [nMods];
  
 
  moduleMask=new int [nMods];
  
  nBins=0;
  if (angRadius && angOffset && angCenter && (binSize>0)) {
    angConv=new angleConversionConstant*[nMods];
    nBins=360./binSize+1;
  }

  for (int im=0; im<nMods; im++) {
#ifdef VERBOSE
    cout << "MODULE "<< im << endl;
#endif
    chansPerMod[im]=chPerMod[im];
    moduleMask[im]=modMask[im];
    totalChans+=chansPerMod[im];

    if (angConv) {
      angConv[im]=new angleConversionConstant(angCenter[im], angRadius[im], angOffset[im], 0);
    }

  }

  
#ifdef VERBOSE
  cout << "badchans " << endl;
#endif
  if (badCh)
    badChannelMask= new int[totalChans];

 
#ifdef VERBOSE
  cout << "ffcoeff " << endl;
#endif
  if (ffcoeff) 
    ffCoeff=new double[totalChans];


#ifdef VERBOSE
  cout << "fferr " << endl;
#endif
  if (fferr) 
    ffErr=new double[totalChans];
  

  for (int ich=0; ich<totalChans; ich++) {
    if (badChannelMask)
      badChannelMask[ich]=badCh[ich];
    if (ffCoeff)
      ffCoeff[ich]=ffcoeff[ich];
    if (ffErr)
      ffErr[ich]=fferr[ich];
  }

  return 0;

}
  



void postProcessingFuncs::deletePointers() {

  delete [] chansPerMod;
  
  delete [] moduleMask;
  if (badChannelMask)
    delete [] badChannelMask;
  
  if (ffCoeff)
    delete [] ffCoeff;
  
  if (ffErr)
    delete [] ffErr;

    if (angConv) {
      for (int im=0; im<nMods; im++) {
	if (angConv[im])
	  delete angConv[im];
      }
      delete [] angConv;
    }


}





postProcessingFuncs::~postProcessingFuncs(){							

  deletePointers();
}






int postProcessingFuncs::flatFieldCorrect(double datain, double errin, double &dataout, double &errout, double ffcoefficient, double fferr){
  double e;

  dataout=datain*ffcoefficient;

  if (errin==0 && datain>=0) 
    e=sqrt(datain);
  else
    e=errin;
  
  if (dataout>0)
    errout=sqrt(e*ffcoefficient*e*ffcoefficient+datain*fferr*datain*fferr);
  else
    errout=1.0;
  
  return 0;
};


 int postProcessingFuncs::rateCorrect(double datain, double errin, double &dataout, double &errout, double tau, double t){

   // double data;
   double e;
 
   dataout=(datain*exp(tau*datain/t));
   
   if (errin==0 && datain>=0) 
     e=sqrt(datain);
   else
     e=errin;
   
   if (dataout>0)
     errout=e*dataout*sqrt((1/(datain*datain)+tau*tau/(t*t)));
   else 
     errout=1.;
   return 0;

};


int postProcessingFuncs::calculateFlatField(int* nModules, int *chPerMod, int *moduleMask, int *badChannelMask, double *ffData, double *ffCoeff, double *ffErr) {
  int nmed=0, im=0;
  double *xmed;

  if (chPerMod==NULL)
    return -1;
  // if (moduleMask==NULL)
  //  return -1;
  if (ffData==NULL)
    return -1;

  if (ffErr==NULL)
    return -1;


  int totch=0;
  for (int im=0; im<*nModules; im++) {
    totch+=chPerMod[im];
  }

  xmed=new double[totch];

  for (int ich=0; ich<totch; ich++) {

    if (badChannelMask)
	if (badChannelMask[ich])
	  continue;
    // calculate median if ffData is positive and channel is good

    if (ffData[ich]>0) {
      im=0;
      while ((im<nmed) && (xmed[im]<ffData[ich])) 
	im++;
      for (int i=nmed; i>im; i--) 
	xmed[i]=xmed[i-1];
      
      xmed[im]=ffData[ich];
      nmed++;
      
    }
    
    

  }
  
  
  if (nmed>1 && xmed[nmed/2]>0) {
#ifdef VERBOSE
    std::cout<< "Flat field median is " << xmed[nmed/2] << " calculated using "<< nmed << " points" << std::endl;
#endif

    
    
    for (int ich=0; ich<totch; ich++) {
      if (badChannelMask)
	if (badChannelMask[ich]) {
	  ffCoeff[ich]=0.;
	  ffErr[ich]=1.;
	}
      
      
      
      if (ffData[ich]>0) {
	ffCoeff[ich]=xmed[nmed/2]/ffData[ich];
	ffErr[ich]=ffCoeff[ich]*sqrt(ffData[ich])/ffData[ich];
      } else {
	ffCoeff[ich]=0.;
	ffErr[ich]=1.;
      }

    }

  }

  delete [] xmed;

  return 0;

}






