#include "energyCalibration.h"

#ifdef __CINT
#define MYROOT
#endif


#ifdef MYROOT
#include <TMath.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TGraphErrors.h>
#endif

#include <iostream>

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))
#define ELEM_SWAP(a,b) { register int t=(a);(a)=(b);(b)=t; }


using namespace std;

#ifdef MYROOT

Double_t energyCalibrationFunctions::pedestal(Double_t *x, Double_t *par) { 
    return par[0]-par[1]*sign*x[0];                                      
}


Double_t energyCalibrationFunctions::gaussChargeSharing(Double_t *x, Double_t *par) { 
  Double_t f, arg=0;
  if (par[3]!=0) arg=sign*(x[0]-par[2])/par[3];
    f=TMath::Exp(-1*arg*arg/2.); 
    f=f+par[5]/2.*(TMath::Erfc(arg/(TMath::Sqrt(2.)))); 
    return par[4]*f+pedestal(x,par);                                       
}

Double_t energyCalibrationFunctions::gaussChargeSharingPixel(Double_t *x, Double_t *par) { 
  Double_t f;
  if (par[3]<=0 || par[2]*(*x)<=0 || par[5]<0 || par[4]<=0) return 0;

  Double_t pp[3];

  pp[0]=0;
  pp[1]=par[2];
  pp[2]=par[3];
  

  f=(par[5]-par[6]*(TMath::Log(*x/par[2])))*erfBox(x,pp);
  f+=par[4]*TMath::Gaus(*x, par[2], par[3], kTRUE);
  return f+pedestal(x,par);                                       
}

Double_t energyCalibrationFunctions::erfBox(Double_t *z, Double_t *par) {

  
  
  Double_t m=par[0];
  Double_t M=par[1];
  
  if (par[0]>par[1]) {
    m=par[1];
    M=par[0];
  }
  
  if (m==M)
    return 0;


  if (par[2]<=0) {
    if (*z>=m && *z<=M)
      return 1./(M-m);
    else
      return 0;

  }

  return (TMath::Erfc((z[0]-M)/par[2])-TMath::Erfc((z[0]-m)/par[2]))*0.5/(M-m);

}


// basic erf function
Double_t energyCalibrationFunctions::erfFunction(Double_t *x, Double_t *par) {	
  double arg=0;
  if (par[1]!=0) arg=(par[0]-x[0])/par[1];
  return ((par[2]/2.*(1+TMath::Erf(sign*arg/(TMath::Sqrt(2))))));  
};
  

Double_t energyCalibrationFunctions::erfFunctionChargeSharing(Double_t *x, Double_t *par) {	       
  Double_t f;		
  
  f=erfFunction(x, par+2)*(1+par[5]*(par[2]-x[0]))+par[0]-par[1]*x[0]*sign;		
  return f;								
};
  
  
Double_t energyCalibrationFunctions::erfFuncFluo(Double_t *x, Double_t *par) {
  Double_t f;							       
  f=erfFunctionChargeSharing(x, par)+erfFunction(x, par+6)*(1+par[9]*(par[6]-x[0])); 
  return f;							
};
#endif

double energyCalibrationFunctions::median(double *x, int n){
  // sorts x into xmed array and returns median 
  // n is number of values already in the xmed array
  double xmed[n];
  int k,i,j;

  for (i=0; i<n; i++) {
    k=0;
    for (j=0; j<n; j++) {
      if(*(x+i)>*(x+j))
	k++;
      if (*(x+i)==*(x+j)) {
	if (i>j) 
	  k++;
      }
    }
    xmed[k]=*(x+i);
  }
  k=n/2;
  return xmed[k];
}


int energyCalibrationFunctions::quick_select(int arr[], int n){
    int low, high ;
    int median;
    int middle, ll, hh;

    low = 0 ; high = n-1 ; median = (low + high) / 2;
    for (;;) {
        if (high <= low) /* One element only */
            return arr[median] ;

        if (high == low + 1) {  /* Two elements only */
            if (arr[low] > arr[high])
                ELEM_SWAP(arr[low], arr[high]) ;
            return arr[median] ;
        }

    /* Find median of low, middle and high items; swap into position low */
    middle = (low + high) / 2;
    if (arr[middle] > arr[high])    ELEM_SWAP(arr[middle], arr[high]) ;
    if (arr[low] > arr[high])       ELEM_SWAP(arr[low], arr[high]) ;
    if (arr[middle] > arr[low])     ELEM_SWAP(arr[middle], arr[low]) ;

    /* Swap low item (now in position middle) into position (low+1) */
    ELEM_SWAP(arr[middle], arr[low+1]) ;

    /* Nibble from each end towards middle, swapping items when stuck */
    ll = low + 1;
    hh = high;
    for (;;) {
        do ll++; while (arr[low] > arr[ll]) ;
        do hh--; while (arr[hh]  > arr[low]) ;

        if (hh < ll)
        break;

        ELEM_SWAP(arr[ll], arr[hh]) ;
    }

    /* Swap middle item (in position low) back into correct position */
    ELEM_SWAP(arr[low], arr[hh]) ;

    /* Re-set active partition */
    if (hh <= median)
        low = ll;
        if (hh >= median)
        high = hh - 1;
    }
}

int energyCalibrationFunctions::kth_smallest(int *a, int n, int k){
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



#ifdef MYROOT
Double_t energyCalibrationFunctions::spectrum(Double_t *x, Double_t *par) {
  return gaussChargeSharing(x,par);	
}

Double_t energyCalibrationFunctions::spectrumPixel(Double_t *x, Double_t *par) {
  return gaussChargeSharingPixel(x,par);	
}


Double_t energyCalibrationFunctions::scurve(Double_t *x, Double_t *par) {
  return erfFunctionChargeSharing(x,par);
}


Double_t energyCalibrationFunctions::scurveFluo(Double_t *x, Double_t *par) {
  return erfFuncFluo(x,par);						
}
#endif

energyCalibration::energyCalibration() : 
#ifdef MYROOT	
					 fit_min(-1),	
					 fit_max(-1),	
					 bg_offset(-1),	
					 bg_slope(-1),	
					 flex(-1),
					 noise(-1),
					 ampl(-1),
					 cs_slope(-1),
					 fscurve(NULL),
					 fspectrum(NULL),
#endif
					 funcs(NULL),
					 plot_flag(1), // fit parameters output to screen
					 cs_flag(1)
{

#ifdef MYROOT	
  funcs=new energyCalibrationFunctions();
  
  fscurve=new TF1("fscurve",funcs,&energyCalibrationFunctions::scurve,0,1000,6,"energyCalibrationFunctions","scurve");	
  fscurve->SetParNames("Background Offset","Background Slope","Inflection Point","Noise RMS", "Number of Photons","Charge Sharing Slope");

  fspectrum=new TF1("fspectrum",funcs,&energyCalibrationFunctions::spectrum,0,1000,6,"energyCalibrationFunctions","spectrum");	
  fspectrum->SetParNames("Background Pedestal","Background slope", "Peak position","Noise RMS", "Number of Photons","Charge Sharing Pedestal");

  fspixel=new TF1("fspixel",funcs,&energyCalibrationFunctions::spectrumPixel,0,1000,7,"energyCalibrationFunctions","spectrumPixel");	
  fspixel->SetParNames("Background Pedestal","Background slope", "Peak position","Noise RMS", "Number of Photons","Charge Sharing Pedestal","Corner");

#endif


}



void energyCalibration::fixParameter(int ip, Double_t val){
  
  fscurve->FixParameter(ip, val);
  fspectrum->FixParameter(ip, val);
}


void energyCalibration::releaseParameter(int ip){

  fscurve->ReleaseParameter(ip);
  fspectrum->ReleaseParameter(ip);
}







energyCalibration::~energyCalibration(){ 
#ifdef MYROOT
  delete fscurve;
  delete fspectrum;
#endif
  
}

#ifdef MYROOT





TH1F* energyCalibration::createMedianHistogram(TH2F* h2, int ch0, int nch, int direction) {

  if (h2==NULL || nch==0)
    return NULL;

  double *x=new double[nch];
  TH1F *h1=NULL;
  
  double val=-1;

  if (direction==0) {
    h1=new TH1F("median","Median",h2->GetYaxis()->GetNbins(),h2->GetYaxis()->GetXmin(),h2->GetYaxis()->GetXmax());
    for (int ib=0; ib<h1->GetXaxis()->GetNbins(); ib++) {
      for (int ich=0; ich<nch; ich++) {
	x[ich]=h2->GetBinContent(ch0+ich+1,ib+1);
      }
      val=energyCalibrationFunctions::median(x, nch);
      h1->SetBinContent(ib+1,val);
    }
  } else if (direction==1) {   
    h1=new TH1F("median","Median",h2->GetXaxis()->GetNbins(),h2->GetXaxis()->GetXmin(),h2->GetXaxis()->GetXmax());
    for (int ib=0; ib<h1->GetYaxis()->GetNbins(); ib++) {
      for (int ich=0; ich<nch; ich++) {
	x[ich]=h2->GetBinContent(ib+1,ch0+ich+1);
      }
      val=energyCalibrationFunctions::median(x, nch);
      h1->SetBinContent(ib+1,val);
    }
  }
  delete [] x;

  return h1;

}














void energyCalibration::setStartParameters(Double_t *par){	
  bg_offset=par[0];						
  bg_slope=par[1];						
  flex=par[2];							
  noise=par[3];							
  ampl=par[4];							
  cs_slope=par[5];						
}


void energyCalibration::getStartParameters(Double_t *par){	
  par[0]=bg_offset;
  par[1]=bg_slope;
  par[2]=flex;
  par[3]=noise;
  par[4]=ampl;				
  par[5]=cs_slope;				
}

#endif
int energyCalibration::setChargeSharing(int p) {
  if (p>=0) {
    cs_flag=p; 
#ifdef MYROOT  
    if (p) {
      fscurve->ReleaseParameter(5);
      fspectrum->ReleaseParameter(1);
    } else {
      fscurve->FixParameter(5,0);
      fspectrum->FixParameter(1,0);
    }
#endif
  } 
  
  return cs_flag;
}

 
#ifdef MYROOT  
void energyCalibration::initFitFunction(TF1 *fun, TH1 *h1) {

 Double_t min=fit_min, max=fit_max; 

 Double_t mypar[6];
  
 if (max==-1) 
    max=h1->GetXaxis()->GetXmax();
 
  if (min==-1) 
    min=h1->GetXaxis()->GetXmin();

  
  if (bg_offset==-1)
    mypar[0]=0;
  else
    mypar[0]=bg_offset;


  if (bg_slope==-1)
    mypar[1]=0;
  else
    mypar[1]=bg_slope;


  if (flex==-1)
    mypar[2]=(min+max)/2.;
  else
    mypar[2]=flex;


  if (noise==-1)
    mypar[3]=0.1;
  else
    mypar[3]=noise;

  if (ampl==-1)
    mypar[4]=h1->GetBinContent(h1->GetXaxis()->FindBin(0.5*(max+min)));
  else
    mypar[4]=ampl;

  if (cs_slope==-1)
    mypar[5]=0;
  else
    mypar[5]=cs_slope;
 
  fun->SetParameters(mypar);

  fun->SetRange(min,max);

}


TF1* energyCalibration::fitFunction(TF1 *fun, TH1 *h1, Double_t *mypar, Double_t *emypar) {


  TF1* fitfun;

  char fname[100];

  strcpy(fname, fun->GetName());

  if (plot_flag) {
    h1->Fit(fname,"R0Q");
  } else
    h1->Fit(fname,"R0Q");


  fitfun= h1->GetFunction(fname);
  fitfun->GetParameters(mypar);
  for (int ip=0; ip<6; ip++) {
    emypar[ip]=fitfun->GetParError(ip);
  }
  return fitfun;
}

TF1* energyCalibration::fitSCurve(TH1 *h1, Double_t *mypar, Double_t *emypar) {
  initFitFunction(fscurve,h1);
  return fitFunction(fscurve, h1, mypar, emypar);
}





TF1* energyCalibration::fitSpectrum(TH1 *h1, Double_t *mypar, Double_t *emypar) {
  initFitFunction(fspectrum,h1);
  return fitFunction(fspectrum, h1, mypar, emypar);
}



TF1* energyCalibration::fitSpectrumPixel(TH1 *h1, Double_t *mypar, Double_t *emypar) {
  initFitFunction(fspixel,h1);
  return fitFunction(fspixel, h1, mypar, emypar);
}



TGraphErrors* energyCalibration::linearCalibration(int nscan, Double_t *en, Double_t *een, Double_t *fl, Double_t *efl, Double_t &gain, Double_t &off, Double_t &egain, Double_t &eoff) { 
 
  TGraphErrors *gr;
 
  Double_t  mypar[2];
 
  gr = new TGraphErrors(nscan,en,fl,een,efl);

  if (plot_flag) {
    gr->Fit("pol1");
    gr->SetMarkerStyle(20);
  } else
    gr->Fit("pol1","0Q");
        
  TF1 *fitfun= gr->GetFunction("pol1"); 
  fitfun->GetParameters(mypar);

  egain=fitfun->GetParError(1);
  eoff=fitfun->GetParError(0);

  gain=funcs->setScanSign()*mypar[1];
  
  off=mypar[0]; 
  
  return gr;
}


TGraphErrors* energyCalibration::calibrate(int nscan, Double_t *en, Double_t *een, TH1F **h1, Double_t &gain, Double_t &off, Double_t &egain, Double_t &eoff, int integral) {

  TH1F *h;

  Double_t mypar[6], emypar[6];
  Double_t fl[nscan], efl[nscan];
  

  for (int ien=0; ien<nscan; ien++) {
    h=h1[ien];
    if (integral)
      fitSCurve(h,mypar,emypar);
    else
      fitSpectrum(h,mypar,emypar);

    fl[ien]=mypar[2];
    efl[ien]=emypar[2];
  }
  return linearCalibration(nscan,en,een,fl,efl,gain,off, egain, eoff);

}

#endif




