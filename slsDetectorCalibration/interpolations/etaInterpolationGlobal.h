#ifndef ETA_INTERPOLATION_GLOBAL_H
#define ETA_INTERPOLATION_GLOBAL_H


#include "etaInterpolationBase.h"

class etaInterpolationGlobal : public etaInterpolationBase{
 public:
 globalEtaInterpolation(int nx=400, int ny=400, int ns=25, int nb=-1, double emin=1, double emax=0) : etaInterpolationBase(nx,ny,ns, nb, emin,emax){};
  


  virtual void prepareInterpolation(int &ok)
  {
   ok=1;  
#ifdef MYROOT1   
   if (hhx) delete hhx;
   if (hhy) delete hhy;

   hhx=new TH2D("hhx","hhx",heta->GetNbinsX(),heta->GetXaxis()->GetXmin(),heta->GetXaxis()->GetXmax(), heta->GetNbinsY(),heta->GetYaxis()->GetXmin(),heta->GetYaxis()->GetXmax()); 
   hhy=new TH2D("hhy","hhy",heta->GetNbinsX(),heta->GetXaxis()->GetXmin(),heta->GetXaxis()->GetXmax(), heta->GetNbinsY(),heta->GetYaxis()->GetXmin(),heta->GetYaxis()->GetXmax()); 
   
#endif


   ///*Eta Distribution Rebinning*///
   double bsize=1./nSubPixels; //precision
   // cout<<"nPixelsX = "<<nPixelsX<<" nPixelsY = "<<nPixelsY<<" nSubPixels = "<<nSubPixels<<endl;
   double tot_eta=0;
   for (int ip=0; ip<nbeta*nbeta; ip++)
     tot_eta+=heta[ip];
   cout << "total eta entries is :"<< tot_eta << endl;   
   if (tot_eta<=0) {ok=0; return;};


   double hx[nbeta]; //projection x
   double hy[nbeta]; //projection y

   for (int ibx=0; ibx<nbeta; ibx++) {
     for (int iby=0; iby<nbeta; iby++) {
       hx[ibx]=hx[ibx]+heta[ibx+iby*nbeta];
       hy[iby]=hx[iby]+heta[ibx+iby*nbeta];
     }
   }
  double hix[nbeta]; //integral of projection x
  double hiy[nbeta]; //integral of projection y
  hix[0]=hx[0];
  hiy[0]=hy[0];
  
  for (int ib=1; ib<nbeta; ib++) {
    hix[ib]=hix[ib-1]+hx[ib];
    hiy[ib]=hiy[ib-1]+hx[ib];
  }

  int ib=0;
   for (int ibx=0; ibx<nbeta; ibx++) {
     if (hix[ibx]>(ib+1)*tot_eta*bsize) ib++;
     for (int iby=0; iby<nbeta; iby++) {
#ifdef MYROOT1  
       hhx->SetBinContent(ibx+1,iby+1,ib);
#endif
#ifndef MYROOT1  
       hhx[ibx+iby*nbeta]=ib; 
#endif
     }
   }
   ib=0;
   for (int iby=0; iby<nbeta; iby++) {
     if (hiy[iby]>(ib+1)*tot_eta*bsize) ib++;
     for (int ibx=0; ibx<nbeta; ibx++) {
#ifdef MYROOT1 
       hhy->SetBinContent(ibx+1,iby+1,ib); 
#endif
#ifndef MYROOT1  
       hhy[ibx+iby*nbeta]=ib; 
#endif
     }
   }

   return ;
  };

};

#endif
