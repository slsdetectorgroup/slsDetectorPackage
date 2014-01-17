#include "postProcessingFuncs.h"
#include "angleConversionConstant.h"

//#define VERBOSE

postProcessingFuncs::postProcessingFuncs(int *nModules,int *chPerMod,int modMask[],int badCh[], double ffcoeff[], double fferr[], double* t, int *dir, double angRadius[], double angOffset[], double angCentre[], double* to, double* bs, double *sX, double *sY):
  nMods(0), chansPerMod(NULL), moduleMask(NULL), badChannelMask(NULL), ffCoeff(NULL), ffErr(NULL), tDead(0), angDir(1), angConv(NULL), totalOffset(0), binSize(0), sampleX(0), sampleY(0), totalChans(0), nBins(0), mp(NULL), mv(NULL), me(NULL), mm(NULL)
{
  initDataset(nModules, chPerMod,modMask,badCh, ffcoeff, fferr, t, dir, angRadius, angOffset, angCentre, to, bs, sX, sY);

}

int postProcessingFuncs::initDataset() {

  //  cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA init dataset " << endl;

  if (nBins) {
    mp=new double[nBins];
    mv=new double[nBins];
    me=new double[nBins];
    mm=new int[nBins];
    resetMerging(mp,mv,me,mm, nBins);
    // cout << "nbins " << nBins <<  endl;
  } else {
    mv=new double[totalChans];
    me=new double[totalChans];
    // cout << "nchans " << totalChans << endl;
  }
  totalI0=0;

  return 0;

}

int postProcessingFuncs::finalizeDataset(double *ang, double *val, double *err, int *np) {
  
  // cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA finalize dataset " << endl;

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


  // cout << "delete mp " <<endl;
  if (mp) {
    delete [] mp;
    mp=NULL;
  }
  // cout << "delete mv " <<endl;
  if (mv) {
    delete [] mv;
    mv=NULL;
  }
  // cout << "delete me " <<endl;
  if (me) {
    delete [] me;
    me=NULL;
  }
  // cout << "delete mm " <<endl;
  if (mm) {
    delete [] mm;
    mm=NULL;

  }


  return 0;

}

int postProcessingFuncs::addFrame(double *data, double *pos, double *I0, double *expTime, const char *filename, double *var) {


  double p1, vin, ein, vout, eout;
  double  e=0.;
  double i0;

  int imod=0, ch0=0;

  int chlast=chansPerMod[0]-1;
  int nchmod=chansPerMod[0];



  if (I0>0) {
    i0=*I0; 
    totalI0+=i0;
  } else
    i0=-1;

//  if (badChannelMask)
//    cout << "---------------- Discarding bad chans " << endl;

  // cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA add frame " << i0 << endl;

  for (int ich=0; ich<totalChans; ich++) {
    

    if (ich>chlast) {
      //  cout << *pos << " " << moduleMask[imod] << endl;
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
      //#ifdef VERBOSE
      //    cout << "ppFuncs ratecorrect" << endl;
    //#endif
      rateCorrect(vin, ein, vout, eout, tDead, *expTime);
      vin=vout;
      ein=eout;
    }
      //ffcorrect
    
    if (ffCoeff) {
      //#ifdef VERBOSE
      //      cout << "ppFuncs ffcorrect" << endl;
      //#endif
      if (ffErr)
	e=ffErr[ich];
      else
	e=0;
      flatFieldCorrect(vin, ein, vout, eout, ffCoeff[ich], e);
    }


    //i0correct
    if (i0>0) {
      //#ifdef VERBOSE
      // cout << "ppFuncs i0 norm" << endl;
      //#endif
      vout/=i0;
      eout/=i0;
    }
      
    if (badChannelMask) {
      //#ifdef VERBOSE
      //      cout << "ppFuncs badchans" << endl;
      //#endif
      if (badChannelMask[ich]) {
	//	cout << "------------------ Discarding channel " << ich << endl;
	continue;
      }
    }
    if (nBins) {
      //angconv
      
// #ifdef VERBOSE
//       cout << "ppFuncs angconv" << endl;
// #endif
//       //check module mask?!?!?!?
      

      p1=convertAngle(*pos,ich-ch0,angConv[imod],moduleMask[imod],totalOffset,0,angDir);

// #ifdef VERBOSE
//   cout << "************************** ppFuncs merge" << endl;
// #endif
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
  return 0;
}





int postProcessingFuncs::initDataset(int *nModules,int *chPerMod,int modMask[],int badCh[], double ffcoeff[], double fferr[], double* t, int *dir, double angRadius[], double angOffset[], double angCenter[], double* to, double* bs, double *sX, double *sY) {

  // cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA init dataset XXXXXX " << endl;
  //#ifdef VERBOSE
  // cout << "delete pointers " << endl;
  //#endif

  deletePointers();
 

  //#ifdef VERBOSE
    // cout << "nmod " << endl;
  //#endif

  if (nModules)
    nMods=*nModules;
  else
    nMods=0;


  //#ifdef VERBOSE
  //cout << nMods << endl;
  //#endif
#ifdef VERBOSE
  cout << "tdead " << endl;
#endif

  if (t)
    tDead=*t;
  else
    tDead=0;

#ifdef VERBOSE
  cout << tDead << endl;
#endif
#ifdef VERBOSE
  cout << "toffset " << endl;
#endif

  if (to)
    totalOffset=*to;
  else
    totalOffset=0;
#ifdef VERBOSE
  cout << totalOffset << endl;
#endif


  //#ifdef VERBOSE
    // cout << "binsize " << endl;
  //#endif
  if (bs)
    binSize=*bs;
  else
    binSize=0;


  //#ifdef VERBOSE
    // cout << binSize << endl;
  //#endif
#ifdef VERBOSE
  cout << "samplex " << endl;
#endif
  if (sX)
    sampleX=*sX;
  else
    sampleX=0;

#ifdef VERBOSE
  cout << sampleX  << endl;
#endif
#ifdef VERBOSE
  cout << "sampley " << endl;
#endif
  if (sY)
    sampleY=*sY;
  else 
    sampleY=0;

#ifdef VERBOSE
  cout << sampleY  << endl;
#endif
  //#ifdef VERBOSE
    // cout << "angdir " << endl;
  //#endif
  if (dir)
    angDir=*dir;
  else
    angDir=1;

  //#ifdef VERBOSE
    // cout << angDir  << endl;
  //#endif
  totalChans=0;


 
  chansPerMod=new int [nMods];
  
 
  moduleMask=new int [nMods];
  
  nBins=0;
  if (angRadius && angOffset && angCenter && (binSize>0)) {
    // cout << "??????? creating angConv"<< endl;
    angConv=new angleConversionConstant*[nMods];
    nBins=(int)(360./binSize)+1;
  }
  //#ifdef VERBOSE
  //cout << "nBins " << nBins << endl;
  //#endif
  for (int im=0; im<nMods; im++) {
    //#ifdef VERBOSE
    //cout << "MODULE "<< im << endl;
    //#endif
    chansPerMod[im]=chPerMod[im];
    //cout <<  chansPerMod[im] << endl;
    moduleMask[im]=modMask[im];
    //cout <<  modMask[im] << endl;
    totalChans+=chansPerMod[im];
    //cout << totalChans << endl;
    if (angConv) {
      //cout << "??????? angConv"<< endl;
      angConv[im]=new angleConversionConstant(angCenter[im], angRadius[im], angOffset[im], 0);
      //cout << angCenter[im] << " " << chansPerMod[im] << " " << angRadius[im] << " " << angOffset[im] << " " << moduleMask[im] << endl;
    } 
    //else cout << "no ang conv " << endl;
  }
  
  // cout << "finished modules loop" << endl;
  
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

    // cout << " init ff " << ich <<  " " << ffCoeff[ich] << ffcoeff[ich] << endl;

  }
  //cout << "init dataset finished " << endl;
  return 0;

}
  



void postProcessingFuncs::deletePointers() {

  delete [] chansPerMod;
  chansPerMod=NULL;
  
  delete [] moduleMask;
  moduleMask=NULL;


  if (badChannelMask) {
    delete [] badChannelMask;
    badChannelMask=NULL;
  }
  
  if (ffCoeff) {
    delete [] ffCoeff;
    ffCoeff=NULL;
  }
  if (ffErr) {
    delete [] ffErr;
    ffErr=NULL;
  }
  if (angConv) {
    for (int im=0; im<nMods; im++) {
      if (angConv[im])
	delete angConv[im];
    }
    // cout << "deleting angConv "<< angConv  << endl;
    delete [] angConv;
    angConv=NULL;
  }
  // cout << "angConv pointer " << angConv << endl;

}





postProcessingFuncs::~postProcessingFuncs(){							

  deletePointers();
}






int postProcessingFuncs::flatFieldCorrect(double datain, double errin, double &dataout, double &errout, double ffcoefficient, double fferr){
  double e;
  dataout=datain*ffcoefficient;

  //  cout << datain << " " << ffcoefficient << " " << dataout << endl;
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

  // cout << "Claculate flat field " << endl;

  if (chPerMod==NULL)
    return -1;
  // if (moduleMask==NULL)
  //  return -1;
  if (ffData==NULL)
    return -1;

  if (ffErr==NULL)
    return -1;


  //  cout << *nModules << " pp chpm0 " << chPerMod[0] << endl;
  int totch=0;
  int nm= *nModules;
  for (int im=0; im<nm; im++) {
    totch+=chPerMod[im];
    //  cout << im << " " << totch << endl;
  }
  //  cout << "tot ch " << totch << endl;
  xmed=new double[totch];

  for (int ich=0; ich<totch; ich++) {
    //   cout << ich << " " << totch << endl;
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
    //#ifdef VERBOSE
    //   std::cout<< "Flat field median is " << xmed[nmed/2] << " calculated using "<< nmed << " points" << std::endl;
    //#endif

    
    //    cout << "checking bad channel mask " << endl;
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
      cout << 	ich << " " << ffData[ich] << " " << ffCoeff[ich] << endl;
    }

  }
      cout << "done " << endl;

  delete [] xmed;
  xmed=NULL;

  return 0;

}






